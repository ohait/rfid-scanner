use strict;
use warnings;

use lib 'lib';
use DBI::Sugar;
use Spore::ILS;
use Spore::App;

# setup the ILS adaptor
$::ILS = Spore::ILS->new(
    api => 'https://intra.deichman.no/api/v1/location',
    decorator => sub {
        my $cn_loc = $_->{cn_location}//''; $cn_loc =~ s{,}{;};
        $cn_loc =~ s{AVdvd}{DVD};
        $cn_loc =~ s{AVvid}{DVD};
        $cn_loc =~ s{AVmul}{Spill};
        $cn_loc =~ s{(\S+) DVD}{DVD $1};
        $cn_loc =~ s{(\S+) Spill}{Spill $1};

        my $cn_dew = $_->{cn_dewey}||''; $cn_dew =~ s{,}{;}; $cn_dew =~ s{^ +}{};
        my $cn_aut = $_->{cn_author}//''; $cn_aut =~ s{,}{;};
        my $cn_ext = '';
        $cn_aut = substr($cn_aut, 0, 3) if length($cn_aut)>3;
        $cn_loc =~ s{(T|b|u| |^)(q|r\+?|qr\+?)}{$1} and $cn_ext .="$2"; # TODO
        $cn_loc =~ s{\s+}{ }g; $cn_loc =~ s{^ }{}; $cn_loc =~ s{ $}{};
        my $cn = join(',', $cn_loc, $cn_dew, $cn_aut, $cn_ext);
        return (
            callnumber => $cn,
            title => $_->{title} // '',
            author => $_->{author} // '',
            copynumber => $_->{copynumber},
            biblionumber => $_->{biblionumber} // $_->{biblionum},
        );
    },
);
$::APP = Spore::App->new(
    ILS => $::ILS,
    scanner_hook => sub {
        my ($record) = @_;
        if ($record->{barcode}) {
            $record->{barcode} =~ s{^10(03010\d+)$}{$1} and warn "H[(10) $record->{barcode}]";
        }
        if ($record->{item_id}) {
            $record->{item_id} =~ s{^10(03010\d+)$}{$1} and warn "H[(10) $record->{item_id}]";
        }
        return $record;
    },
);

my $db = 'rfid';
my $user = 'rfid';
my $pass = 'rfid';

DBI::Sugar::factory {
    my $dbh = DBI->connect("dbi:mysql:$db", $user, $pass, {
        RaiseError => 1, # die on errors
        mysql_enable_utf8mb4 => 1, # connect with full utf8 support *and* send a SET NAMES
    });
    $dbh->{mysql_use_result} = 1; # force using data while it's coming, instead of storing the whole results
    return $dbh, SELECT_TIME => sub {
        my ($q, $b, $dt) = @_;
        print Dumper(\@_) if $dt > 0.5; # warn if query takes longer than 500ms
    };
};


1;
