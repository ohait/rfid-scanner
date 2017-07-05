use strict;
use warnings;

package OhaIt::RFIDScanner;

use LWP::UserAgent;
use HTTP::Request;
use JSON;
use Data::Dumper;
use Devel::Peek;

sub new {
    my ($class, %args) = @_;
    my $self = bless {
        ua => LWP::UserAgent->new(),
        json => JSON->new()->utf8(1),
        %args,
    }, $class;
    $self->{koha_api} or die "you must specify a koha_api";

    return $self;
}

sub hexdump {
    my ($self, $data, $line_len) = @_;
    $line_len //= 16;
    my $out = '';
    for (my $i=0; $i<length $data; $i+=$line_len) {
        my $line = sprintf "%3d:", $i;
        for (my $j=$i; $j<$i+$line_len and $j<length $data; $j++) {
            $line .= sprintf " %02x", ord(substr($data, $j, 1));
            $line .= " " if $j%8==7;
        }
        $line = substr($line.'    'x$line_len, 0, 5+3*$line_len);
        $out .= $line;
        $out .= " [";
        for (my $j=$i; $j<$i+$line_len and $j<length $data; $j++) {
            my $h = substr($data, $j, 1);
            $h =~ s{[^\x20-\x7E]}{.}g;
            $out .= $h;
            $out .= " " if $j%8==7;
        }
        $out =~ s{ $}{};
        $out .= "]\n";
    }
    return $out;
}

# I can't make Digest::CRC work with this, so I reimplemented it looking at examples
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

sub parse_data {
    my ($self, $data) = @_;

    if ($data =~ m{^(SHELF\#).(?<shelf>[^\0]*)}) {
        return {
            type => 'shelf',
            shelf => $+{shelf},
        };
    }

    if ($data =~
        m{^(?<before>.*?)
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
                (?<library>[\d\x00]{8})
                (?<extra>...)
            )
        )(?<after>.*+)$}x
    ) {
        my $ver = ord($+{ver});
        my $given_crc = unpack('H*', $+{crc});
        my $calc_crc = unpack('H*', _crc($+{d1}.$+{d2}));
        $given_crc eq $calc_crc or die "invalid crc, got: '$given_crc', expected '$calc_crc'";
        my $library = $+{library}; $library =~ s{\x00+$}{};
        my $barcode = $+{barcode}; $barcode =~ s{^\x00+}{}; $barcode =~ s{\x00+$}{};
        return {
            vertype => $ver,
            item => [ord($+{nr}),ord($+{tot})],
            barcode => $+{barcode},
            country => $+{country},
            library => $+{library},
            what => $+{what},
            _crc => unpack('H*', $+{crc}),
            _trim => {
                before => unpack('H*', $+{before}),
                after => unpack('H*', $+{after}),
            },
            _data => unpack('H*',$+{data}),
        };
    }

    Dump($data);
    die "can't parse ".Dumper($data);
}

sub pack_data {
    my ($self, %data) = @_;
    $data{item} //= [1,1];
    $data{country} // die "missing country";
    $data{library} // die "missing library id";
    $data{barcode} // die "missing barcode";
    $data{what}//='10'; # REALLY? :(
    $data{ver} //= 17;

    my @pack = (
        C => $data{ver},
        C => $data{item}->[1],
        C => $data{item}->[0],
        a16 => $data{barcode},
        X => 0, # CRC placeholder
        A2 => $data{country},
        a11 => $data{library},
    );
    my $out = '';
    my $pre = '';
    for (my $i=0; $i<@pack; $i+=2) {
        my $pack = $pack[$i];
        if ($pack eq 'X') {
            $pre = $out;
            $out = '';
            next;
        }
        my $val = $pack[$i+1];
        my $p = pack($pack, $val);
        #print Dumper([$pack,$val,$p]);
        $out .= $p;
    }
    my $crc = _crc($pre.$out);
    print $self->hexdump($pre.$crc.$out);
    return $pre.$crc.$out;
}

# expect a list of hashes, and decorate them with title, author and expected location from koha
sub koha_decorate {
    my ($self, @items) = @_;
    my %items;
    my @list;
    for (@items) {
        my $barcode = $_->{barcode} or next;
        $items{$barcode} = $_;
        #push @list, { barcode => $barcode, loc => $_->{loc} };
        push @list, $barcode;
    }
    return unless keys %items;

    my $req = HTTP::Request->new(POST => $self->{koha_api}, [], $self->{json}->encode(\@list));
    my $res = $self->{ua}->request($req);
    $res->is_success or die "KOHA: ".$res->status_line;
    my $out = $self->{json}->decode($res->content);
    print Dumper(\@list, $out);


    for (@{$out->{items}}) {
        my $barcode = $_->{barcode} or die "missing barcode from koha?";
        my $item = $items{$barcode} or next;
        $item->{title} = $_->{title};
        $item->{author} = $_->{author};
        #$item->{dest} = $_->{loc};
        $item->{ccode} = $_->{ccode};
        my $dest = $_->{loc};
        #$dest = 'xxxx.pickup';
        $dest =~ m{(\w+).*?(\.pickup|\.transfer)?$} and $item->{dest} = "$1$2";
    }
    return $out;
}

1;
