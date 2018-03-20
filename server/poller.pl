#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

print "started at ".gmtime()." [$$]\n";

my $DEV = "ILS";

use lib 'lib';
BEGIN { require "./Config.pm"; };

$SIG{ALRM} = sub { die "TIMEOUT"; };
alarm(50); # it will finish long before

use LWP::UserAgent;
use POSIX qw/strftime/;
use Time::Piece;
use URI;
use Data::Dumper;

sub process {
    my (@records) = @_;
    #@records = @records[0..5] if @records > 5;

    my @list;
    for my $record (@records) {
        $record->{item_id} //= $record->{barcode} or next;
        eval { $record->{at} = Time::Piece->strptime($record->{at}, "%Y-%m-%d %T")->epoch; } or die "[$record->{at}] => $@";
        my @actions = ();
        if (my $field = $record->{field}) {
            my $old = $record->{oldval};
            my $new = $record->{newval};
            push @actions, defined $new ? "$field: $new" : "$field";
            if (defined $old) {
                push @actions, "was: $old";
            }
        }
        $record->{actions} = \@actions;
        push @list, $record;
    }
    @records = @list;

    $_->{dev} = $DEV for @records;

    $::APP->{ILS}->sync(@records);
    #print Dumper(\@records);

    TX {
        $::APP->append_activity($_) for @records;
    };
    print "appended ".scalar(@records)." entries in the log\n";
}

TX {

    my $UA = LWP::UserAgent->new("spore-poller/0.1");
    my $uri = URI->new($::APP->{poller}) or die "you must specify a {poller} api point in Config.pm";

    my (undef, $last) = SELECT_ROW "at FROM history WHERE instance = ? AND dev = ? ORDER BY at DESC LIMIT 1" => [$::APP->{instance}, $DEV];
    if ($last) {
        $uri->query_form(from => strftime("%Y-%m-%d %T", gmtime($last)));
    }
    my $req = HTTP::Request->new(GET => $uri);
    my $res = $UA->request($req);
    $res->is_success or die "FAILED: "+$res->status_line;
    my $data = $::APP->json_dec($res->content) or die "missing data";
    $data->{log} or die "no log entry";
    process(@{$data->{log}});
};

print "finished at ".gmtime()." [$$]\n";
