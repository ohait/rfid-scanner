use strict;
use warnings;

package Spore::Scanner;

use Spore::Utils;
use Spore::Scanner::Request;
use Spore::Scanner::Response;
use Data::Dumper;
use DBI::Sugar;
use Time::HiRes;
use JSON;
use Text::Unidecode;

sub new {
    my ($type, %args) = @_;
    return bless { %args }, $type;
}

sub process {
    my ($self, $req, $app) = @_;
    my $dev = $req->{dev} or die "no device";

#    my $shelf = $req->{init_shelf};
#    for my $record (@{$req->{records}}) {
#        if ($record->{type} eq 'shelf') {
#            $shelf = $record->{shelf};
#        }
#        else {
#            $record->{shelf} //= $shelf;
#        }
#    }

    $app->{ILS}->sync(@{$req->{records}});

    my $res = Spore::Scanner::Response->new();

    my $last;
    TX {
        my $last;
        for my $record (@{$req->{records}})
        {
            my $rfid = $record->{rfid};
            if ($rfid) {
                $last = $record if $record->{meta}->{biblionumber};
                my %tag = SELECT_ROW "* FROM tags WHERE rfid = ?" => [$record->{rfid}];
                # expand 'branchname' to 'branchname.previous.shelf'
                if ($tag{ploc} and $record->{dest} and $tag{ploc} =~ m{^\Q$record->{dest}}) {
                    warn "dest '$record->{dest}' => '$tag{ploc}'";
                    $record->{dest} = $tag{ploc};
                }
                elsif (my $dloc = $record->{meta}->{default_loc}) {
                    $record->{dest} = $dloc;
                }
            }

            $self->append($record, $dev, $res);

            if (my $dest = $record->{dest} and my $shelf = $record->{shelf}) {
                warn "[$shelf]~[$dest]";
                if ($shelf !~ m{^\Q$dest}) {
                    $res->append(PIMG => $record);
                }
            }
        }
        if (!$res->messages()) {
            $last and $res->append(IMG => $last);
        }
    };

    return $res;
}

sub append {
    my ($self, $record, $dev, $res, $app) = @_;
    $app //= Spore::App->new();
#warn Dumper($record)."...";
    my $rfid = $record->{rfid};
    my $action;
    my $type;
    if ($rfid) {
        my $shelf = $record->{shelf};
        my %data;
        $data{item_supplier} = $record->{item_supplier};
        $data{item_id} = $record->{item_id};
        $data{data} = $record->{data};

        if ($shelf =~ m{\.(pub|stack)\.}) {
            $data{ploc} = $shelf;
            $data{ploc_at} = Time::HiRes::time();
            $data{ploc_dev} = $dev;
        } else {
            $data{tloc} = $shelf;
            $data{tloc_at} = Time::HiRes::time();
            $data{tloc_dev} = $dev;
        }

        my (undef, $new_data) = SELECT_ROW "new_data FROM tags WHERE rfid = ?" => [$rfid];
        if (defined $new_data) {
            if ($new_data eq $record->{data}) {
                $data{new_data} = undef;
            } else {
                $action = 'WRT';
                $res->append(WRT => $new_data);
            }
        }

        UPSERT tags => {
            rfid => $rfid,
        } => {
            %data
        } => {};

        if ($record->{item_id}) {
            UPSERT items => {
                item_supplier => $record->{item_supplier},
                item_id => $record->{item_id},
            } => {
                product_id => $record->{product_id},
                json => $app->json_enc($record->{meta}),
                last_seen => Time::HiRes::time(),
            } => {
            };

            #warn "DB[".$app->json_enc($record->{meta})."]";
        }

        # FULLTEXT index
        DELETE items_idx => {
            item_supplier => $record->{item_supplier},
            item_id => $record->{item_id},
        };
        if (my $ft = $record->{ftext}) {
            for my $word (split /\s+/, $ft) {
                next unless $word =~ m{\w\w};
                my $w2 = $word;
                $w2 =~ s{\W}{}g;
                my %words = map { lc($_) => 1 } $word, unidecode($word), $w2, unidecode($w2);
                eval {
                    INSERT items_idx => {
                        item_supplier => $record->{item_supplier},
                        item_id => $record->{item_id},
                        word => $_,
                    };
                } for keys %words;
            }
        }
    }

    INSERT history => {
        dev => $dev,
        rfid => $rfid,
        at => Time::HiRes::time(),
        action => $action,
        type => $type,
        shelf => $record->{shelf},
    };
}

1;
