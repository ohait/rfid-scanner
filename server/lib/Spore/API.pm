use strict;
use warnings;

package Spore::API;
use Spore::Utils;
use Spore::Scanner;
use Spore::Scanner::Request;
use Spore::Scanner::Response;
use Data::Dumper;
use DBI::Sugar;
use Time::HiRes;
use JSON;
use Text::Unidecode;
use MIME::Base64;
use DateTime;

sub new {
    my ($class, %args) = @_;

    my $self = bless {
        %args,
    }, $class;
    $self->{JSON} //= JSON->new()->utf8(1);
    $self->{JSON_DB} //= JSON->new()->utf8(0);

    return $self;
}

sub json_enc {
    my ($self, $obj) = @_;
    return $self->{JSON}->encode($obj);
}
sub json_dec {
    my ($self, $json) = @_;
    return undef unless $json;
    return $self->{JSON}->decode($json);
}
sub epoch2iso {
    my ($self, $epoch) = @_;
    return undef unless $epoch;
    return {
        epoch => $epoch,
        iso8601 => DateTime->from_epoch(epoch => $epoch)->iso8601(),
    };
}

sub api_shelves {
    my ($self, $root) = @_;
    $root =~ m{^[\w\.]*$} or die "400 Invalid shelf: '$root'";

    my $out = {
        at => $self->epoch2iso(Time::HiRes::time()),
        results => [],
    };
    SELECT "ploc, count(distinct(item_id)) ct FROM tags WHERE instance = ? GROUP BY ploc"
    => [$self->{instance}] => sub {
        push @{$out->{results}}, {
            loc => ($_{ploc}//''),
            tags => $_{ct},
        };
    };

    return $out;
}

sub api_item {
    my ($self, $item_supplier, $item_id) = @_;
    $item_supplier or die "400 Missing item_supplier";
    $item_id or die "400 Missing item_id";
    my $out = {
        item_supplier => $item_supplier,
        item_id => $item_id,
        at => $self->epoch2iso(Time::HiRes::time()),
        response => undef,
    };
    my %row = SELECT_ROW "* FROM items WHERE instance = ? AND item_supplier = ? AND item_id = ?"
        => [$self->{instance}, $item_supplier, $item_id];

    if (not %row) {
        # AUTOVIVIFY?
    }

    %row or die "404 Not Found: item('$item_supplier', '$item_id')";

    $out->{response} = {
        item_supplier => $row{item_supplier},
        item_id => $row{item_id},
        product_id => $row{product_id},
        last_seen => $self->epoch2iso($row{last_seen}),
        tags => [],
        meta => $self->json_dec($row{json}),
    };

    SELECT "* FROM tags WHERE instance = ? AND item_supplier = ? AND item_id = ?"
    => [$self->{instance}, $item_supplier, $item_id] => sub {
        my $history = [];
        push @{$out->{response}->{tags}}, {
            rfid => $_{rfid},
            permanent => {
                loc => $_{ploc},
                at => $self->epoch2iso($_{ploc_at}),
                dev => $_{ploc_dev},
            },
            temporary => {
                loc => $_{tloc},
                at => $self->epoch2iso($_{tloc_at}),
                dev => $_{tloc_dev},
            },
            #last_seen => $_{last_at},
            data => {
                base64 => encode_base64($_{data}, ''),
            },
        };

    };

    #for my $tag (@{$out->{response}->{tags}}) {
    #    SELECT "* FROM history WHERE instance = ? AND rfid = ? ORDER BY at DESC limit 1000"
    #    => [$self->{instance}, $tag->{rfid}] => sub {
    #        push @{$tag->{history}}, {
    #            dev => $_{dev},
    #            at => $_{at},
    #            action => $_{action},
    #            type => $_{type},
    #            loc => $_{shelf},
    #        };
    #    };
    #}
    return $out;
}

1;
