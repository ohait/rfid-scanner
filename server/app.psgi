#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

use Plack::Request;
use Data::Dumper;
use DBI;
use HTML::Entities;
use POSIX qw/strftime/;
use Date::Parse;
use Time::HiRes;
use Secrets;

use JSON; our $JSON = JSON->new()->utf8(1)->pretty(1)->canonical(1);

use lib 'lib';
use OhaIt::RFIDScanner; our $D = OhaIt::RFIDScanner->new(
    koha_api => $Secrets::KOHA_API,
);

use lib '../../../perl-dbi-sugar/lib/';
use DBI::Sugar;

DBI::Sugar::factory {
    my $dbh = DBI->connect("dbi:SQLite:dbname=rfid.db","","");
    $dbh->{sqlite_unicode} = 1;
    return $dbh;
};

sub epoch2h {
    my ($e) = @_;
    $e||=0;
    $e += 0;
    return 'n/a' unless $e;

    my $now = Time::HiRes::time();
    my $ago = $now-$e;
    if ($ago>86400*7) {
        return localtime($e);
    }
    my $days = int($ago/86400);
    my $h = int($ago/3600)%24;
    my $m = int($ago/60)%60;

    if ($days>1) {
        return $days."d&nbsp;ago";
    }
    if ($days==1) {
        return ($days*24+$h)."h&nbsp;ago";
    }
    if ($h>1) {
        return $h."h&nbsp;ago";
    }
    return ($h*60+$m)."'&nbsp;ago";
}

sub epoch2html {
    my ($e) = @_;
    my $ago = epoch2h($e);
    my $title = $e ? localtime($e) : 'NULL';

    return qq'<abbr title="$title">$ago</abbr>';
}

sub blob2html {
    my ($d) = @_;
    return '<span class="hex">NULL</span>' unless defined $d;
    utf8::downgrade($d);
    $d =~ s{\0+$}{};
    $d =~ s{\n}{<span class="hex">\\n</span>}g;
    $d =~ s{\r}{<span class="hex">\\r</span>}g;
    $d =~ s{\t}{<span class="hex">\\t</span>}g;
    $d =~ s{([^\x20-\x7E])}{sprintf '<span class="hex">%02x</span>', ord $1;}ge;
    $d =~ s{(\d{12,})}{
        my $code = $1;
        my $pre = '';
        $code =~ s{^(10)(0301)}{$2} and $pre = $1;
        qq'$pre<a href="/rfid/$code">$code</a>'
    }ge;
    return $d;
}

sub html_rfid {
    my ($req) = @_;
	my (undef, $code, undef, $extra) = $req->path =~ m<^/rfid(/([\w:]+)(/(.*))?)?> or die;
	my $res = $req->new_response(200);
	my @list;
	my $title = ($code//'');
	TX {
		if (not defined $code) {
			$title = "Devices";
			SELECT "distinct dev FROM history" => [] => sub {
				my $dev = $_{dev};
				print $_{dev}."\n";
				push @list, qq'[<a href="/rfid/$dev">$dev</a>]<br>';
			};
		} elsif ($code =~ m{^((\w\w:){5}\w\w)$} and $extra) { # dev
			$title = "History: device $code";
			push @list, qq[<tr><td>at</td><td>tag</a></td><td>data</td><td>action</td></tr>];
			my $to = '';
			SELECT "* FROM history WHERE dev = ? AND at > ? ORDER BY at LIMIT 1000" => [$code, $extra] => sub {
				$_{data} = blob2html($_{data});
				$_{action} = blob2html($_{action});
				my $ago = epoch2html($_{at});
				$ago = '&uarr;' if $_{at} eq $to;
				$to = $_{at};
				push @list, qq[<tr><td>$ago</td><td><a href="/rfid/$_{rfid}">$_{rfid}</a></td><td>$_{data}</td><td>$_{action}</td></tr>];
			};
		} elsif ($code =~ m{^((\w\w:){5}\w\w)$}) { # dev
            $title = "History: device $code";
            my %days;
            SELECT "round(at/3600)*3600 l, count(1) ct from history group by l"
            => [] => sub {
                my $date = strftime "%F %a", localtime($_{l});
                print "$_{l} => ".strftime("%F %T", localtime($_{l}))." += $_{ct}\n";
                my $c = $days{$date}//={
                    ct => 0,
                    at => str2time(strftime('%F', localtime($_{l}))."T00:00:00"),
                    hat => $date,
                };
                $c->{ct} += $_{ct};
            };
            for my $d (sort { $b->{at} cmp $a->{at} } values %days) {
                push @list, qq[<tr><td><a href="/rfid/$code/$d->{at}">$d->{hat}</a></td><td align=right>$d->{ct}</td></tr>];
            }
            unshift @list, qq[<tr><th>day</th><th>scans count</th></tr>];
		} elsif ($code =~ m{^(\w\w:){7}\w\w$}) { # tag
			$title = "History: TAG $code";
			push @list, qq[<tr><td>at</td><td>device</a></td><td>data</td></tr>];
			SELECT "* FROM history WHERE rfid = ? ORDER BY at DESC LIMIT 100" => [$code] => sub {
				$_{data} = blob2html($_{data});
				push @list, qq[<tr><td>$_{at}</td><td><a href="/rfid/$_{dev}">$_{dev}</a></td><td>$_{data}</td></tr>];
			};
		} elsif ($code =~ m{^(\d{12,16})$}) { # barcode
			$title = "nearby $code";
			my %row;
			while(1) {
				last if %row = SELECT_ROW "* FROM items i left join items_details d ON i.barcode = d.barcode WHERE i.barcode = ? order by last_at desc limit 1" => [$code];
				last if %row = SELECT_ROW "* FROM items_details d left join items i ON d.barcode = i.barcode WHERE d.barcode = ? order by last_at desc limit 1" => [$code];
				$code =~ s{^0}{} or last;
			}

			%row or die "404 Not Found '$code'";
			$row{shelf}//='no&nbsp;shelf';
			$row{$_}//='' for keys %row;
			$row{cn} =~ s{ }{&nbsp;};
			$row{cn}//="<i>$row{cc}</i>";

			my $ago = epoch2html($row{last_at});
			push @list, qq[<tr class="hit"><td>$row{cn}</td><td><a href="/rfid/$row{rfid}">$ago</a></td><td>$row{shelf}</td><td>$row{barcode}</td><td>$row{author}</td><td>$row{title}</td></tr>];

			my %seen;
			SELECT "* FROM items i left join items_details d ON i.barcode = d.barcode
				WHERE last_dev = ? AND last_at <= ? AND i.barcode <> ?
				ORDER BY last_at DESC, i.barcode DESC limit 20"
				=> [$row{last_dev}, $row{last_at}, $row{barcode}] => sub {
					return if $_{barcode} and $seen{$_{barcode}}++;
					return if $_{last_at} < $row{last_at}-600;
					my $shelf = $_{shelf}//'no shelf';
					$_{cn}//="<i>$_{cc}</i>";
					$_{$_}//='' for keys %_;
					my $ago = epoch2html($_{last_at});
					unshift @list, qq[<tr><td>$_{cn}</td><td><a href="/rfid/$_{rfid}">$ago</a></td><td>$shelf</td><td><a href="/rfid/$_{barcode}">$_{barcode}</a></td><td>$_{author}</td><td>$_{title}</td></tr>];
				};

			SELECT "* FROM items i left join items_details d ON i.barcode = d.barcode
				WHERE last_dev = ? AND last_at >= ? AND i.barcode <> ?
				ORDER BY last_at, i.barcode limit 20"
				=> [$row{last_dev}, $row{last_at}, $row{barcode}] => sub {
					return if $_{barcode} and $seen{$_{barcode}}++;
					return if ($_{last_at}//0) > ($row{last_at}//0)+600;
					my $shelf = $_{shelf}//'no shelf';
                    $_{cc} //= '';
					$_{cn}//="<i>$_{cc}</i>";
					$_{$_}//='' for keys %_;
					my $ago = epoch2html($_{last_at});
					push @list, qq[<tr><td>$_{cn}</td><td><a href="/rfid/$_{rfid}">$ago</a></td><td>$shelf</td><td><a href="/rfid/$_{barcode}">$_{barcode}</a></a></td><td>$_{author}</td><td>$_{title}</td></tr>];
				};

			unshift @list, qq[<tr><td>call</td><td>last seen</td><td>shelf</td><td>barcode</td><td width="15%">author</td><td>title</td></tr>];
		} else {
            die "unrecognized '$code'";
		}
	};
	my $list = join "\n", @list;
	my $body = <<"EOD";
<html>
<head>
<title>$title</title>
<style>
body {
    font-family: "Lucida Console", Monaco, monospace;
}
td {
    padding: 4px;
}
tr.hit {
    background-color: #fbb;
}
.hex {
    font-size: 60%;
    border: 1px solid gray;
    margin-right: 1px;
    background-color: gray;
    color: white;
}
</style>
</head>
<body>
<h1><a href="/rfid/">RFID</a> - $title</h1>
<table border=1 cellspacing=0>
$list
</table>
</body>
</html>
EOD
	use Encode;
	$body = encode('UTF-8', $body);
	#utf8::downgrade($body);
	$res->body($body);
	$res->header("Content-Type" => "text/html; charset=utf-8");
	$res->header("Cache-Control" => "private");
	$res->header("Access-Control-Allow-Origin", "*");
	return $res;
}

sub json_rfid {
    my ($req) = @_;
	my $t0 = Time::HiRes::time();
    my $out = {
        at => $t0,
        path => $req->path(),
    };
	my (undef, $code) = $req->path =~ m</rfid(/([\w:]+))?> or die;
	TX {
		if (not defined $code) {
            $out->{type} = "devices_list";
            $out->{results} = [];
			SELECT "distinct dev FROM history" => [] => sub {
				my $dev = $_{dev};
                $out->{results}->push($dev);
			};
		} elsif ($code =~ m{^(\w\w:){5}\w\w$}) { # dev
            $out->{type} = "device_history";
            $out->{results} = [];
			SELECT "* FROM history WHERE dev = ? ORDER BY at DESC LIMIT 1000" => [$code] => sub {
                push @{$out->{results}}, {%_};
			};
		} elsif ($code =~ m{^(\w\w:){7}\w\w$}) { # tag
            $out->{type} = "tag_history";
            $out->{results} = [];
			SELECT "* FROM history WHERE rfid = ? ORDER BY at DESC LIMIT 100" => [$code] => sub {
                push @{$out->{results}}, {%_};
			};
		} elsif ($code =~ m{^(\d{12,16})$}) { # barcode
            $out->{type} = "nearby";
            $out->{before} = [];
            $out->{after} = [];
			my %row;
			while(1) {
				last if %row = SELECT_ROW "* FROM items i left join items_details d ON i.barcode = d.barcode WHERE i.barcode = ? order by last_at desc limit 1" => [$code];
				last if %row = SELECT_ROW "* FROM items_details d left join items i ON d.barcode = i.barcode WHERE d.barcode = ? order by last_at desc limit 1" => [$code];
				$code =~ s{^0}{} or last;
			}

			%row or die "404 Not Found '$code'";
            $out->{entry} = {%row};

			my %seen;
			SELECT "* FROM items i left join items_details d ON i.barcode = d.barcode
				WHERE last_dev = ? AND last_at <= ? AND i.barcode <> ?
				ORDER BY last_at DESC, i.barcode DESC limit 20"
				=> [$row{last_dev}, $row{last_at}, $row{barcode}] => sub {
					return if $_{barcode} and $seen{$_{barcode}}++;
					return if $_{last_at} < $row{last_at}-600;
                    unshift @{$out->{before}}, {%_};
				};

			SELECT "* FROM items i left join items_details d ON i.barcode = d.barcode
				WHERE last_dev = ? AND last_at >= ? AND i.barcode <> ?
				ORDER BY last_at, i.barcode limit 20"
				=> [$row{last_dev}, $row{last_at}, $row{barcode}] => sub {
					return if $_{barcode} and $seen{$_{barcode}}++;
					return if ($_{last_at}//0) > ($row{last_at}//0)+600;
                    push @{$out->{after}}, {%_};
				};

		} else {
            die "unrecognized '$code'";
		}
	};
    return $out;
}

sub hexdump {
    my ($data) = @_;
    for (my $i=0; $i<length $data; $i+=16) {
        printf "%3d:", $i;
        for (my $j=$i; $j<$i+16 and $j<length $data; $j++) {
            printf " %02x", ord(substr($data, $j, 1));
        }
        print " [";
        for (my $j=$i; $j<$i+16 and $j<length $data; $j++) {
            my $h = substr($data, $j, 1);
            $h =~ s{[^\x20-\x7E]}{.}g;
            print $h;
        }
        print "]\n";
    }
}

sub arduino_rfid {
    my ($req) = @_;
	my $t0 = Time::HiRes::time();

    my $data = $req->raw_body();
    hexdump($data);

    #$data ||= "BB123456"."hutl.stack.123B".("\0"x24)."012345678".(chr(15))."03011931218015\0";
    #$data ||= "BB123456"."hutl.stack.123B".("\0"x24)."012345678".(chr(15))."03010664064002\0";

    # TODO add mode-byte, and variable length data
	length($data) >= 40 or die "expected 40+ bytes, got: ".length($data);

    my $dev = join(':', map { sprintf("%02x", ord($_)); } split //, substr($data, 2, 6));
    my $shelf = substr($data, 8, 32);
    $shelf =~ s{\0+$}{};
    $data = substr($data, 40);
    print "DEV $dev => shelf: '$shelf'\n";

    my $body;
    my @items;

	TX {
		BLOCK: while(length($data)>=9) {
			my $_rfid = substr($data, 0, 8);
			my $rfid = join(':', map { sprintf("%02x", ord($_)); } split //, $_rfid);

			my $len = ord(substr($data, 8, 1));
			my $_data = substr($data, 9, $len);
			$data = substr($data, 9+$len);
			print "$rfid => $len bytes\n";

			my %row = SELECT_ROW "* FROM items WHERE rfid = ? LIMIT 1"=>[$rfid];
            if ($len) {
                my (undef, $wrt) = SELECT_ROW "data FROM to_write WHERE rfid = ?" => [$rfid];
                if ($wrt) {
                    my $old = $_data;
                    $old =~ s{\0+$}{};
                    utf8::downgrade($wrt);
                    if ($wrt eq $old) {
                        print "WROTE OK [$rfid]\n";
                        # NOTHING TO WRITE
                        DELETE to_write => { rfid => $rfid };
                    } else {
                        print "TO WRITE [$rfid]\n";
                        my $w = $_rfid.$wrt;
                        $body = "WRT\n".length($w)."\n".$w;
                    }
                }
                my $barcode;
                if (my $obj = $D->parse_data($_data)) {
                    $barcode = $obj->{barcode};
                }
                $barcode and $barcode =~ s{^100301}{0301};

                push @items, {
                    _rfid => $rfid,
                    _data => $_data,
                    rfid => $rfid,
                    barcode => $barcode,
                    loc => $shelf, # this will be sent to koha, and replaced with the expected location
                };

                $barcode and UPSERT items => {
                    rfid => $rfid,
                } => {
                    last_at => $t0,
                    last_dev => $dev,
                    shelf => $shelf,
                    barcode => $barcode,
                } => {};
            }
            else {
                if (%row) {
                    push @items, {
                        _rfid => $_rfid,
                        rfid => $rfid,
                        barcode => $row{barcode},
                        loc => $shelf,
                    };
                } else {
					$body = "READ\n$_rfid" if $len == 0;
                }
            }
        }
    };

    $D->koha_decorate(@items); # this will decorate the list with title, autors and expected locations

    TX {
        for my $item (@items) {
            my $action;
            if ($shelf and $item->{dest} and $shelf !~ m{^\Q$item->{dest}}) {
                $body = $action = join ("\n", PICK => $item->{barcode} => $item->{author} => $item->{title} => $item->{ccode} => $item->{dest} => '');
            }
            if ($item->{barcode}) {
                UPSERT items_details => {
                    barcode => $item->{barcode},
                } => {
                    title => $item->{title},
                    author => $item->{author},
                    cn => $item->{ccode},
                } => {};
            }

            INSERT history => {
                dev => $dev,
                at => $t0,
                rfid => $item->{rfid},
                data => $item->{_data},
                action => $action,
            };

        }

        if (!$body) {
            my ($item) = @items;
            $body = join ("\n", NOOP => map { $item->{$_}//'' } qw/barcode author title ccode dest ./);
        }

    };

	utf8::encode($body) if utf8::is_utf8($body);
	my $res = $req->new_response(200);
	$res->body($body);
	$res->header("Cache-Control" => "private");
	$res->header("Access-Control-Allow-Origin", "*");
	return $res
}

sub dispatch {
    my ($req) = @_;
    return arduino_rfid($req) if $req->path =~ m<^/arduino/rfid(/|$)>;
    return html_rfid($req) if $req->path =~ m<^/rfid(/|$)>;
    return json_rfid($req) if $req->path =~ m<^/json(/|$)>;
    die "404 Not Found ".$req->path;
}


sub {
    my ($req) = Plack::Request->new(@_);
    my $res;
    eval {
        eval {
            $res = dispatch($req);
            1;
        } or do {
            my $err = $@;
            warn $err;
            my $code = 500;
            $err =~ s{^(\d\d\d) }{} and $code = $1;
            $res = $req->new_response($code);
            $res->body($err);
        };
        if (!ref($res)) {
            # plain html?
            my $r = $req->new_response(200);
            $r->header("Content-Type" => "text/html; charset=UTF-8");
            $r->body($res);
            $res = $r;
        }
        elsif (UNIVERSAL::isa($res, 'Plack::Response')) {
            # ALL GOOD
        }
        else {
            # JSON!
            my $r = $req->new_response(200);
            $r->header("Content-Type" => "application/json");
            $r->body($JSON->encode($res));
            $res = $r;
        }
        $res->header(author => "oha[at]oha.it");
        1;
    } or do {
        my $err = $@ // 'Error';
        my $code = 500;
        $err =~ s{^([45]\d\d) ?}{} and $code = $1;
        $res = $req->new_response($code);
        if ($code < 500) {
            $res->header("Cache-Control" => "public, max-age=10, s-maxage=10");
        }
        $res->body("$code $err");
    };
    return $res->finalize;
};
