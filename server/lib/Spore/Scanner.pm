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

    $app->{ILS}->sync(@{$req->{records}});

    my $res = Spore::Scanner::Response->new();

    my $last;
    TX {
        my $last;
        for my $record (@{$req->{records}})
        {
            my $rfid = $record->{rfid};
            if ($rfid) {
                my %tag = SELECT_ROW "* FROM tags WHERE rfid = ?" => [$rfid];

                # expand 'branchname' to 'branchname.previous.shelf' if match
                if ($tag{ploc} and $record->{dest} and $tag{ploc} =~ m{^\Q$record->{dest}}) {
                    $record->{dest} = $tag{ploc};
                }

                $record->{dest} //= $record->{meta}->{default_loc}; # TODO what is this?

                # TODO base64 encode, maybe?
                if (defined $tag{new_data}) {
                    if ($tag{new_data} eq $record->{data}) {
                        UPDATE tags => { rfid => $rfid } => { new_data => undef };
                    } else {
                        $res->append(WRT => $tag{new_data});
                        push @{$record->{actions}//=[]}, 'tag write';
                    }
                }
            }

            # should you PICK the item and move it elsewhere?
            if (my $dest = $record->{dest} and my $loc = $record->{loc}) {
                warn "[$loc]~[$dest]";
                if ($loc !~ m{^\Q$dest}) {
                    $res->append(PIMG => $record);
                    push @{$record->{actions}//=[]}, 'pick';
                }
            }

            warn Dumper($record);
            # store the entries in the DB
            $app->append_activity({%$record, dev => $dev});

            # only show info for the last valid items if nothing else to do
            $last = $record if $record->{meta}->{biblionumber};
        }
        if (!$res->messages()) {
            $last and $res->append(IMG => $last);
        }
    };

    return $res;
}

1;
