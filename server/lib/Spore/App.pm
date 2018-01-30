use strict;
use warnings;

package Spore::App;
use Spore::Utils;
use Spore::Scanner;
use Spore::Scanner::Request;
use Spore::Scanner::Response;
use Spore::API;
use Data::Dumper;
use DBI::Sugar;
use Time::HiRes;
use JSON;
use Text::Unidecode;
use MIME::Base64;
use DateTime;

sub new {
    my ($class, %args) = @_;
    $args{instance}//=0;

    my $self = bless {
        %args,
        scanner => Spore::Scanner->new(%args),
        api => Spore::API->new(%args),
    }, $class;
    $self->{JSON} //= JSON->new()->utf8(1);
    $self->{JSON_DB} //= JSON->new()->utf8(0);

    return $self;
}

sub json_enc {
    my ($self, $obj) = @_;
    return $self->{JSON}->encode($obj);
}
sub json_dec {
    my ($self, $json) = @_;
    return unless $json;
    return $self->{JSON}->decode($json);
}
sub epoch2iso {
    my ($self, $epoch) = @_;
    return unless $epoch;
    return {
        epoch => $epoch,
        iso8601 => DateTime->from_epoch(epoch => $epoch)->iso8601(),
    };
}

sub arduino {
    my ($self, $uri, $raw_data, $headers) = @_;
    #print hexdump($raw_data);
    my $req = Spore::Scanner::Request->new($raw_data, $uri, $headers, $self->{scanner_hook});
    #warn Dumper($req);

    my $res = $self->{scanner}->process($req, $self);

    #warn Dumper($res);
    return $res->encode();
}

sub api {
    my ($self, $path, $raw_data, $headers) = @_;

    my (undef, $method, @arguments) = split /\//, $path;

    $method =~ m{^\w+$} or die "400 invalid api method: '$method'";

    my $m = "api_$method";
    $self->{api}->can($m) or die "404 api method not found: '$method'";

    my $out = TX {
        return $self->{api}->$m(@arguments);
    };
    warn Dumper($path, $out)."...";
    return $out // {};
}

1;
