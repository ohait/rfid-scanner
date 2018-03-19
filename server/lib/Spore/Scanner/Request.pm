use strict;
use warnings;

package Spore::Scanner::Request;

use Data::Dumper;
use Spore::Utils;

sub _hexdump {
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

sub new {
    my ($type, $data, $uri, $headers, $hook) = @_;
    $data = $data->raw_body if UNIVERSAL::isa($data, 'Plack::Request');
    length($data) >= 20 or die "expected 20+ bytes, got: ".length($data);

    $data =~ m{^\x42\x42} or die "invalid data: magic number missing";
    my $dev = join(':', map { sprintf("%02x", ord($_)); } split //, substr($data, 2, 6));

    my $loc = substr($data, 8, 32);
    $loc =~ s{[\x00].*$}{};
    $data = substr($data, 8+32);

    my $self = bless {
        dev => $dev,
        init_loc => $loc,
        records => [],
    }, $type;

    BLOCK:
    while(length($data)>=9) {
        #print "R HEAD:\n".hexdump(substr($data, 0, 10));
        my $_rfid = substr($data, 0, 8);
        my $rfid = join(':', map { sprintf("%02x", ord($_)); } split //, $_rfid);
        $rfid = undef if $rfid eq '00:00:00:00:00:00:00:00';
        my $flags = ord(substr($data, 8, 1));
        my $len = ord(substr($data, 9, 1));
        my $record_data = substr($data, 10, $len);
        #print "R BODY:\n".hexdump($record_data);
        $data = substr($data, 10+$len);
        #print STDERR "$rfid => (f: $flags) $len bytes\n";

        my $record =  {
            len => $len,
            rfid => $rfid,
            raw_rfid => $_rfid,
            flags => $flags,
            data => $record_data,
        };

        if (my $obj = $self->parse_record($record_data)) {
            $record->{$_} //= $obj->{$_} for keys %$obj;
            if ($obj->{type} eq 'shelf') {
                $loc = $record->{loc};
            } else {
                $record->{loc}//=$loc;
            }
        }
        if ('CODE' eq ref $hook) {
            my @out = $hook->($record);
            warn "expected hash" if scalar grep { ref ne 'HASH' } @out;
            push @{$self->{records}}, @out;
        } else {
            push @{$self->{records}}, $record;
        }
    }

    return $self;
}

sub records {
    my ($self) = @_;
    return @{$self->{records}};
}

# used for S24 standard
sub _crc {
    my ($data) = @_;

    my $poly = 0x1021;
    my $crc = 0xffff;

    for my $ch (split //, $data) {
        my $c = ord($ch);
        #printf "DBG %d '%s' => ", $c, $ch;
        $c <<=8;
        for (my $i=0; $i<8; $i++) {
            my $xor_f = (($crc^$c)&0x8000) != 0;
            $crc <<= 1;
            $xor_f and $crc = $crc ^ $poly;
            $c <<= 1;
        }
        $crc &= 0xffff;
        #printf "[%04x]\n", $crc;
    }
    return pack('v', $crc);
}

sub parse_record {
    my ($self, $data) = @_;
    #warn _hexdump($data);
    if ($data =~ m{^.?(SHELF\#)(?<loc>.*)}) {
        my $loc = $+{loc};
        $loc =~ s{\0}{}g;
        return {
            type => 'shelf',
            loc => $loc,
        };
    }

    # Danish S24 standard
    if ($data =~ m{^
            (?<before>.*?)
            (?<data>
                (?<d1>
                    (?<ver>.) # 4 bit version, 4 bit type (0=>buy, 1=>circ, 2=>no circ, 7=>throw, 8=>patroncard)
                    (?<tot>.)
                    (?<nr>.)
                    (?<barcode>[\d\x00]{16})
                )
                (?<crc>..)
                (?<d2>
                    (?<country>[A-Z][A-Z])
                    #(?<library>[\d\x00]*)
                    (?<library>[\d\x00]{11})
                )
            )(?<after>.*)
        $}x
    ) {
        my %m = %+;
        my $ver = ord($+{ver});
        my $given_crc = unpack('H*', $+{crc});
        my $calc_crc = unpack('H*', _crc($+{d1}.$+{d2}));
        $given_crc eq $calc_crc or die "invalid crc, got: '$given_crc', expected '$calc_crc'";
        my $library = $m{library}; $library =~ s{\0}{}g;
        my $barcode = $m{barcode}; $barcode =~ s{\0}{}g;
        return {
            type => "item",
            vertype => $ver,
            item => [ord($m{nr}),ord($m{tot})],
            item_supplier => $m{country}.":".$library,
            item_id => $barcode,
            barcode => $barcode,
            country => $m{country},
            library => $library,
            _crc => unpack('H*', $m{crc}),
            _trim => {
                before => unpack('H*', $m{before}),
                after => unpack('H*', $m{after}),
            },
            #_data => unpack('H*',$m{data}),
        };
    }

    if ($data =~ m{^.\0+$}) { # only first byte set => empty
        return {
            type => 'empty',
        };
    }

    #Dump($data);
    warn "can't parse ".Dumper($data);
    return {};
}

1;
