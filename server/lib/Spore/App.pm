use strict;
use warnings;

package Spore::App;
use Spore::Utils;
use Spore::Scanner;
use Spore::Scanner::Request;
use Spore::Scanner::Response;
use Spore::API;
use Data::Dumper;
use DBI::Sugar;
use Time::HiRes;
use JSON;
use Text::Unidecode;
use MIME::Base64;
use DateTime;
use Carp qw/confess/;

sub new {
    my ($class, %args) = @_;
    $args{instance}//=0;

    my $self = bless {
        %args,
        scanner => Spore::Scanner->new(%args),
        api => Spore::API->new(%args),
    }, $class;
    $self->{JSON} //= JSON->new()->utf8(1);
    $self->{JSON_DB} //= JSON->new()->utf8(0);

    return $self;
}

sub json_enc {
    my ($self, $obj) = @_;
    return undef unless defined $obj;
    return eval { $self->{JSON}->encode($obj); } // confess($@);
}
sub json_dec {
    my ($self, $json) = @_;
    return unless $json;
    return $self->{JSON}->decode($json);
}
sub epoch2iso {
    my ($self, $epoch) = @_;
    return unless $epoch;
    return {
        epoch => $epoch,
        iso8601 => DateTime->from_epoch(epoch => $epoch)->iso8601(),
    };
}

sub arduino {
    my ($self, $uri, $raw_data, $req) = @_;
    #print hexdump($raw_data);

    # parse the request into a scanner object (it's good to have this as middleman, make the next easier to test)
    my $sreq = Spore::Scanner::Request->new($raw_data, $uri, $req->headers, $self->{scanner_hook});
    #warn Dumper($req);

    # perform the main logic tasks
    my $res = $self->{scanner}->process($sreq, $self);
    #warn Dumper($res);

    # send the instructions back to the scanner
    return $res->encode();
}

sub api {
    my ($self, $path, $raw_data, $req) = @_;

    my (undef, $method, @arguments) = split /\//, $path;

    $method or die "404 api method missing";
    $method =~ m{^\w+$} or die "404 api method not found: '$method'";

    my $m = "api_$method";
    $self->{api}->can($m) or die "404 api method not found: '$method'";

    my $out = TX {
        local $_ = $req;
        return $self->{api}->$m(@arguments);
    };
    warn Dumper($path, $out)."...";
    return $out // {};
}

sub hub {
    my ($self, $path, $raw_data, $req) = @_;
# assuming /hub/in

    my $list = $self->json_dec($raw_data);
    $list = [$list] unless 'ARRAY' eq ref $list;

    # expand the lists with the custom decorator
    my @records;
    for my $msg (@$list) {
        $msg->{client_IP} or die "missing IP"; # Should we use the remote addr?
        my $record = $self->{hub_hook}->($msg);
        next unless $record;
        push @records, $record;
    }

    # sync with ILS
    $self->{ILS}->sync(@records);

    TX {
        $self->append_activity($_) for @records;
    };

    return {
        count => scalar(@$list),
        at => $self->epoch2iso(time()),
    };
}

# given an activity record, fills the db tables properly
sub append_activity {
    my ($self, $record) = @_;
    #warn Dumper($record);
    my $supplier = $record->{item_supplier} or die "missing item supplier";
    my $id = $record->{item_id};

    my $rfid = $record->{rfid};
	my $dev = $record->{dev};
    if ($rfid) {
        my $loc = $record->{loc}//$record->{shelf};
        my %data;
        $data{item_supplier} = $supplier;
        $data{item_id} = $id;
        $data{data} = $record->{data};

        if ($loc =~ m{\.(pub|stack)\.}) {
            $data{ploc} = $loc;
            $data{ploc_at} = Time::HiRes::time();
            $data{ploc_dev} = $dev;
        } else {
            $data{tloc} = $loc;
            $data{tloc_at} = Time::HiRes::time();
            $data{tloc_dev} = $dev;
        }

        UPSERT tags => {
            instance => $self->{instance},
            rfid => $rfid,
        } => {
            %data
        } => {};
    }

    if ($id) {
        UPSERT items => {
            instance => $self->{instance},
            item_supplier => $supplier,
            item_id => $id,
        } => {
            product_id => $record->{product_id},
            json => $self->json_enc($record->{meta}),
            last_seen => Time::HiRes::time(),
        } => {
        };

        #warn "DB[".$self->json_enc($record->{meta})."]";
    }
    # FULLTEXT index
    DELETE items_idx => {
        instance => $self->{instance},
        item_supplier => $supplier,
        item_id => $id,
    };
    if (my $ft = $record->{ftext}) {
        for my $word (split /\s+/, $ft) {
            next unless $word =~ m{\w\w};
            my $w2 = $word;
            $w2 =~ s{\W}{}g;
            my %words = map { lc($_) => 1 } $word, unidecode($word), $w2, unidecode($w2);
            eval {
                INSERT items_idx => {
                    instance => $self->{instance},
                    item_supplier => $supplier,
                    item_id => $id,
                    word => $_,
                };
            } for keys %words;
        }
    }
    my $actions = $record->{actions}//[];
    $actions = [$actions] unless ref $actions;

    INSERT history => {
        instance => $self->{instance},
        dev => $dev,
        item_supplier => $supplier,
        item_id => $id,
        rfid => $rfid,
        at => Time::HiRes::time(),
        actions => join(", ", @$actions),
        loc => $record->{loc},
    };
}

1;
