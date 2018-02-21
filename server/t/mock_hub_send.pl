use strict;
use warnings;
use Data::Dumper;

use lib 'lib/';
use Spore::Utils;
use LWP::UserAgent;
use HTTP::Request;

my $ua = LWP::UserAgent->new();

my $req = HTTP::Request->new(POST => 'http://127.0.0.1:5000/hub/', [], <<"EOD");
{
    "barcode":   "03011006590001",
    "client_IP": "10.172.41.69",
    "sender":    "rfidhub",
    "sip_message_type": "MsgReqCheckin"
}
EOD
my $res = $ua->request($req);

print Dumper($res);
