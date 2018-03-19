use strict;
use warnings;

package Spore::Scanner::Response;
use Data::Dumper;
use POSIX qw/strftime/;
use Imager;
use Text::Unidecode qw/unidecode/;
our @B64 = ('A'..'Z', 'a'..'z', '0'..'9', '+', '/');
our $FONT = Imager::Font->new(file => '04b_03/04B_03__.TTF') or die;
our $FONT2 = Imager::Font->new(file => 'PixelOperator/PixelOperator8.ttf') or die;

sub new {
    my ($type) = @_;

    my $self = bless {
        messages => [],
    }, $type;

    return $self;
}

sub toascii {
    my ($x) = @_;
    $x = unidecode($x);
    return $x;
}

sub tocn {
    my ($x) = @_;
    if ("ARRAY" eq ref $x) {
        $x = join ' ', @$x;
    }
    $x =~ s{,+}{ }g; $x =~ s{^ }{};
    # TODO expand to latin-1? unidecode anything else
    $x =~ s{([^\x00-\x7FåæøÅÆØ])}{
        unidecode($1);
    }ge;
    return $x;
}

sub item_pick {
    my ($info) = @_;
    warn Dumper(IMG => $info)."...";
    my $cn = tocn($info->{meta}->{callnumber} // '?callnum?');
    my $author = toascii($info->{meta}->{author} // '?author?');
    my $title = toascii($info->{meta}->{title} // '?title?');
    my $bibnum = $info->{meta}->{biblionumber}//'';
    my $copynum = $info->{meta}->{copynumber}//'0';
    my $barcode = $info->{barcode}//'';
    my $dest = $info->{dest}//'?dest?';
    $dest =~ s{\.verify\.}{.v.};
    if ($copynum<10) { $copynum = "00$copynum"; }
    elsif ($copynum<100) { $copynum = "0$copynum"; }

    my $img = Imager->new(xsize=>128, ysize=>32);
    $FONT2->align(image => $img, string => $cn,
            size => 8, x => 0, y => 7);
    $FONT->align(image => $img, string => $author,
            size => 8, x => 0, y => 15);
    $FONT->align(image => $img, string => $title,
            size => 8, x => 0, y => 22, valing => 'top');

    $FONT->align(image => $img, string => $dest,
            size => 8, x => 0, y => 30, valing => 'top');
    $FONT->align(image => $img, string => "$bibnum/$copynum",
            size => 8, x => 127, y => 30, valing => 'top', halign=>'right');

    return render_img($img);
}

sub render_img {
    my ($img) = @_;
    my @out;
    for (my $y=0; $y<32; $y+=6) {
        my $s = '';
        for (my $x=0; $x<128; $x++) {
            my $w = 0;
            for my $b (0..5) {
                my $col = $img->getpixel(x=>$x, y=>$y+$b) or next;
                my ($r) = $col->rgba();
                $w += (2**$b) if $r;
            }
            $s .= $B64[$w];
        }
        push @out, $s;
    }
    return @out;
}


sub messages {
    my ($self) = @_;
    return @{$self->{messages}};
}

sub append {
    my ($self, $type, @args) = @_;

    if ($type eq 'IMG') {
        push @{$self->{messages}}, [IMG => item_pick(@args)];
    }
    elsif ($type eq 'PIMG') {
        push @{$self->{messages}}, [PIMG => item_pick(@args)];
    }
    elsif ($type eq 'WRT') {
        my ($w) = @args;
        push @{$self->{messages}}, [WRT => length($w) => $w];
    }
    elsif ($type eq 'SOFTWARE_UPDATE') {
        my ($path) = @args;
        push @{$self->{messages}}, [SOFTWARE_UPDATE => $path];
    }
    else {
        die "invalid type: '$type'";
    }
}

sub encode {
    my ($self) = @_;

    my @list = @{$self->{messages}};

    push @list, [EPOCH => time()];
    my $localtz = strftime("%z", localtime());
    if ($localtz =~ m/([+\-]?\d\d)(\d\d)/) {
        my $localepoch = time() + $1*3600 + $2*60;
        push @list, [LOCAL_EPOCH => $localepoch];
    }

    #print Dumper(\@list);

    my $out = join("\n", map {
        my $x = $_; $x = [$x] unless ref $x;
        join "\n", @$x;
    } @list, "");
    return $out;
}

