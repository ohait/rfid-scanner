#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

require "./Config.pm";


#use Carp::Always;
use Carp;
use Plack::Request;
use Data::Dumper;
use DBI;
use HTML::Entities;
use POSIX qw/strftime/;
use Date::Parse;
use Time::HiRes;
use JSON; our $JSON = JSON->new()->utf8(1)->pretty(1)->canonical(1);
use DBI::Sugar;

use lib 'lib';

TX {
    my (undef, $ct) = SELECT_ROW "count(1) FROM items" => [];
    print STDERR "$ct items in the database\n";
};

sub dispatch {
    my ($req) = @_;
    my ($route, $path) = $req->path =~ m{^/(\w+)(.*)};

    if ($route eq 'arduino') {
        return $::APP->arduino($req->uri, $req->raw_body, $req);
    }
    if ($route eq 'api') {
        return $::APP->api($path, $req->raw_body, $req);
    }
    if ($route eq 'hub') {
        return $::APP->hub($path, $req->raw_body, $req);
    }
    my $f = $req->path;
    $f =~ m{(^|\/)\.} and die "404 Invalid path '$f'";
    warn Dumper("static/$f", -d "static/$f");
    $f .= "index.html" if -f "static/$f/index.html";
    -r "static/$f" or die "404 Not Found '$f'";

    open my $fh, '<:raw', "static/$f" or die "403 Forbidden: $!";
    my $body = do { local $/, <$fh> }; # slurp
    close $fh;
    my $ctype = 'application/octet-stream';
    $ctype = 'text/plain' if $f =~ m{\.txt};
    $ctype = 'text/html' if $f =~ m{\.html};
    $ctype = 'text/css' if $f =~ m{\.css};
    $ctype = 'application/javascript' if $f =~ m{\.js};

    my $r = $req->new_response(200);
    $r->header("Cache-Control" => "private");
    $r->header("Access-Control-Allow-Origin", "*");
    $r->header("Content-Type" => $ctype);
    $r->body($body);

    return $r;
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
            $r->header("Cache-Control" => "private");
            $r->header("Access-Control-Allow-Origin", "*");
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
