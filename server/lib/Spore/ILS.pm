use strict;
use warnings;

package Spore::ILS;

# module which integrate with a generic ILS api point to share location updates and get item details


#use Fcntl qw/:flock/;
use Time::HiRes;
use LWP::UserAgent;
use HTTP::Request;
use JSON;
use Data::Dumper;
#use Digest::CRC;
use DBI::Sugar;

sub new {
    my ($class, %args) = @_;
    my $self = bless {
        ua => LWP::UserAgent->new(),
        json => JSON->new()->utf8(1)->pretty(1),
        %args,
    }, $class;
    $self->{api} or die "you must specify an {api}";
    return $self;
}


sub sync {
    my ($self, @items) = @_;
    my %items;
    my @req_list;
    for (@items) {
        $_ = { barcode => $_ } unless ref $_;
        $_->{barcode} or next;
        $_->{barcode} =~ s{\0+$}{}; # trim trailing nulls
        my $barcode = $_->{barcode};
        push @{$items{$barcode}//=[]}, $_;
        push @req_list, {
            barcode => $barcode,
            location => ($_->{loc}//$_->{shelf}),
        };
    }
    return unless @req_list;

    my $req = HTTP::Request->new(POST => $self->{api}, [], $self->{json}->encode(\@req_list));
    my $res = $self->{ua}->request($req);
    $res->is_success or die "ILS: ".$res->status_line."\n\t".Dumper($res);
    my $out = $self->{json}->decode($res->content);
    warn Dumper($out)."...";

    for (@{$out->{items}}) {
        next unless $_;
        my $barcode = $_->{barcode} or die "missing barcode from api?";
        for my $item (@{$items{$barcode}}) {
            $item->{homebranch} = $_->{homebranch} // $_->{branch};
            $item->{ftext} = join("\n", grep { defined } $_->{title}, $_->{author}, $_->{barcode}, $_->{biblionumber});
            $item->{product_id} = $_->{biblionumber};
            $item->{dest} = $_->{dest} // $_->{shelf} // $_->{loc};
            my $meta = $item->{meta} = {};
#            $item->{meta}->{copynumber} = $_->{copynumber};
#            $item->{meta}->{title} = $_->{title};
#            $item->{meta}->{author} = $_->{author};
            # TODO HOLDS?
            if ('CODE' eq ref $self->{decorator}) {
                local $_ = {%$_};
                my %meta = $self->{decorator}->();
                for my $k (keys %meta) {
                    $meta->{$k} = $meta{$k};
                }
            }
        }
    }
    return @items;
}

1;
