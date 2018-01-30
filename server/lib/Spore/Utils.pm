use strict;
use warnings;

package Spore::Utils;

use base 'Exporter';

our @EXPORT = qw/hexdump/;

use Data::Dumper;

sub hexdump {
    my ($data) = @_;
    my $out = '';
    for (my $i=0; $i<length $data; $i+=16) {
        $out .= sprintf "%3d:", $i;
        for (my $j=$i; $j<$i+16; $j++) {
            if ($j<length $data) {
                $out .= sprintf " %02x", ord(substr($data, $j, 1));
            } else {
                $out .= "   ";
            }
        }
        $out .= " [";
        for (my $j=$i; $j<$i+16 and $j<length $data; $j++) {
            my $h = substr($data, $j, 1);
            $h =~ s{[^\x20-\x7E]}{.}g;
            $out .= $h;
        }
        $out .= "]\n";
    }
    return $out;
}

1;
