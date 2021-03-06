#	$OpenBSD: src/regress/usr.sbin/relayd/funcs.pl,v 1.10 2014/05/02 14:10:03 andre Exp $

# Copyright (c) 2010-2013 Alexander Bluhm <bluhm@openbsd.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

use strict;
use warnings;
#no warnings 'experimental::smartmatch';
use feature 'switch';
use Errno;
use Digest::MD5;
use Socket;
use Socket6;
use IO::Socket;
use IO::Socket::INET6;

sub find_ports {
	my %args = @_;
	my $num    = delete $args{num}    // 1;
	my $domain = delete $args{domain} // AF_INET;
	my $addr   = delete $args{addr}   // "127.0.0.1";

	my @sockets = (1..$num);
	foreach my $s (@sockets) {
		$s = IO::Socket::INET6->new(
		    Proto  => "tcp",
		    Domain => $domain,
		    $addr ? (LocalAddr => $addr) : (),
		) or die "find_ports: create and bind socket failed: $!";
	}
	my @ports = map { $_->sockport() } @sockets;

	return @ports;
}

########################################################################
# Client funcs
########################################################################

sub write_char {
	my $self = shift;
	my $len = shift // $self->{len} // 251;
	my $sleep = $self->{sleep};

	my $ctx = Digest::MD5->new();
	my $char = '0';
	for (my $i = 1; $i < $len; $i++) {
		$ctx->add($char);
		print $char
		    or die ref($self), " print failed: $!";
		given ($char) {
			when(/9/)	{ $char = 'A' }
			when(/Z/)	{ $char = 'a' }
			when(/z/)	{ $char = "\n" }
			when(/\n/)	{ print STDERR "."; $char = '0' }
			default		{ $char++ }
		}
		if ($self->{sleep}) {
			IO::Handle::flush(\*STDOUT);
			sleep $self->{sleep};
		}
	}
	if ($len) {
		$char = "\n";
		$ctx->add($char);
		print $char
		    or die ref($self), " print failed: $!";
		print STDERR ".\n";
	}
	IO::Handle::flush(\*STDOUT);

	print STDERR "LEN: ", $len, "\n";
	print STDERR "MD5: ", $ctx->hexdigest, "\n";
}

sub http_client {
	my $self = shift;
	my @lengths = @{$self->{lengths} || [ shift // $self->{len} // 251 ]};
	my $vers = $self->{lengths} ? "1.1" : "1.0";
	my $method = $self->{method} || "GET";
	my %header = %{$self->{header} || {}};
	my $cookie = $self->{cookie} || "";

	foreach my $len (@lengths) {
		# encode the requested length or chunks into the url
		my $path = ref($len) eq 'ARRAY' ? join("/", @$len) : $len;
		# overwrite path with custom path
		if (defined($self->{path})) {
			$path = $self->{path};
		}
		my @request = ("$method /$path HTTP/$vers");
		push @request, "Host: foo.bar" unless defined $header{Host};
		push @request, "Content-Length: $len"
		    if $vers eq "1.1" && $method eq "PUT" &&
		    !defined $header{'Content-Length'};
		push @request, "$_: $header{$_}" foreach sort keys %header;
		push @request, "Cookie: $cookie"
		    if $cookie ne "";
		push @request, "";
		print STDERR map { ">>> $_\n" } @request;
		print map { "$_\r\n" } @request;
		write_char($self, $len) if $method eq "PUT";
		IO::Handle::flush(\*STDOUT);
		# XXX client shutdown seems to be broken in relayd
		#shutdown(\*STDOUT, SHUT_WR)
		#    or die ref($self), " shutdown write failed: $!"
		#    if $vers ne "1.1";

		my $chunked = 0;
		{
			local $/ = "\r\n";
			local $_ = <STDIN>;
			defined
			    or die ref($self), " missing http $len response";
			chomp;
			print STDERR "<<< $_\n";
			m{^HTTP/$vers 200 OK$}
			    or die ref($self), " http response not ok"
			    unless $self->{httpnok};
			while (<STDIN>) {
				chomp;
				print STDERR "<<< $_\n";
				last if /^$/;
				if (/^Content-Length: (.*)/) {
					$1 == $len or die ref($self),
					    " bad content length $1";
				}
				if (/^Transfer-Encoding: chunked$/) {
					$chunked = 1;
				}
			}
		}
		if ($chunked) {
			read_chunked($self);
		} else {
			read_char($self, $vers eq "1.1" ? $len : undef)
			    if $method eq "GET";
		}
	}
}

sub read_chunked {
	my $self = shift;

	for (;;) {
		my $len;
		{
			local $/ = "\r\n";
			local $_ = <STDIN>;
			defined or die ref($self), " missing chunk size";
			chomp;
			print STDERR "<<< $_\n";
			/^[[:xdigit:]]+$/
			    or die ref($self), " chunk size not hex: $_";
			$len = hex;
		}
		last unless $len > 0;
		read_char($self, $len);
		{
			local $/ = "\r\n";
			local $_ = <STDIN>;
			defined or die ref($self), " missing chunk data end";
			chomp;
			print STDERR "<<< $_\n";
			/^$/ or die ref($self), " no chunk data end: $_";
		}
	}
	{
		local $/ = "\r\n";
		while (<STDIN>) {
			chomp;
			print STDERR "<<< $_\n";
			last if /^$/;
		}
		defined or die ref($self), " missing chunk trailer";
	}
}

sub errignore {
	$SIG{PIPE} = 'IGNORE';
	$SIG{__DIE__} = sub {
		die @_ if $^S;
		warn "Error ignored";
		warn @_;
		IO::Handle::flush(\*STDERR);
		POSIX::_exit(0);
	};
}

########################################################################
# Server funcs
########################################################################

sub read_char {
	my $self = shift;
	my $max = shift // $self->{max};

	my $ctx = Digest::MD5->new();
	my $len = 0;
	if (defined($max) && $max == 0) {
		print STDERR "Max\n";
	} else {
		while (<STDIN>) {
			$len += length($_);
			$ctx->add($_);
			print STDERR ".";
			if (defined($max) && $len >= $max) {
				print STDERR "\nMax";
				last;
			}
		}
		print STDERR "\n";
	}

	print STDERR "LEN: ", $len, "\n";
	print STDERR "MD5: ", $ctx->hexdigest, "\n";
}

sub http_server {
	my $self = shift;
	my %header = %{$self->{header} || {}};
	my $cookie = $self->{cookie} || "";

	my($method, $url, $vers);
	do {
		my $len;
		{
			local $/ = "\r\n";
			local $_ = <STDIN>;
			return unless defined $_;
			chomp;
			print STDERR "<<< $_\n";
			($method, $url, $vers) = m{^(\w+) (.*) HTTP/(1\.[01])$}
			    or die ref($self), " http request not ok";
			$method =~ /^(GET|PUT)$/
			    or die ref($self), " unknown method: $method";
			($len, my @chunks) = $url =~ /(\d+)/g;
			$len = [ $len, @chunks ] if @chunks;
			while (<STDIN>) {
				chomp;
				print STDERR "<<< $_\n";
				last if /^$/;
				if ($method eq "PUT" &&
				    /^Content-Length: (.*)/) {
					$1 == $len or die ref($self),
					    " bad content length $1";
				}
				if ($cookie eq "" &&
				    /^Cookie: (.*)/) {
				    $cookie = $1;
				}
			}
		}
		# XXX reading to EOF does not work with relayd
		#read_char($self, $vers eq "1.1" ? $len : undef)
		read_char($self, $len)
		    if $method eq "PUT";

		my @response = ("HTTP/$vers 200 OK");
		$len = defined($len) ? $len : scalar(split /|/,$url);
		if (ref($len) eq 'ARRAY') {
			push @response, "Transfer-Encoding: chunked"
			    if $vers eq "1.1";
		} else {
			push @response, "Content-Length: $len"
			    if $vers eq "1.1" && $method eq "GET";
		}
		push @response, "$_: $header{$_} " foreach sort keys %header;
		push @response, "Set-Cookie: $cookie"
		    if $cookie ne "";
		push @response, "";

		print STDERR map { ">>> $_\n" } @response;
		print map { "$_\r\n" } @response;

		if (ref($len) eq 'ARRAY') {
			write_chunked($self, @$len);
		} else {
			write_char($self, $len) if $method eq "GET";
		}
		IO::Handle::flush(\*STDOUT);
	} while ($vers eq "1.1");
}

sub write_chunked {
	my $self = shift;
	my @chunks = @_;

	foreach my $len (@chunks) {
		printf STDERR ">>> %x\n", $len;
		printf "%x\r\n", $len;
		write_char($self, $len);
		printf STDERR ">>> \n";
		print "\r\n";
	}
	my @trailer = ("0", "X-Chunk-Trailer: @chunks", "");
	print STDERR map { ">>> $_\n" } @trailer;
	print map { "$_\r\n" } @trailer;
}

########################################################################
# Script funcs
########################################################################

sub check_logs {
	my ($c, $r, $s, %args) = @_;

	return if $args{nocheck};

	check_len($c, $r, $s, %args);
	check_md5($c, $r, $s, %args);
	check_loggrep($c, $r, $s, %args);
}

sub check_len {
	my ($c, $r, $s, %args) = @_;

	my @clen = $c->loggrep(qr/^LEN: /) or die "no client len"
	    unless $args{client}{nocheck};
	my @slen = $s->loggrep(qr/^LEN: /) or die "no server len"
	    unless $args{server}{nocheck};
	!@clen || !@slen || @clen ~~ @slen
	    or die "client: @clen", "server: @slen", "len mismatch";
	!defined($args{len}) || !$clen[0] || $clen[0] eq "LEN: $args{len}\n"
	    or die "client: $clen[0]", "len $args{len} expected";
	!defined($args{len}) || !$slen[0] || $slen[0] eq "LEN: $args{len}\n"
	    or die "server: $slen[0]", "len $args{len} expected";
	my @lengths = map { ref eq 'ARRAY' ? @$_ : $_ }
	    @{$args{lengths} || []};
	foreach my $len (@lengths) {
		unless ($args{client}{nocheck}) {
			my $clen = shift @clen;
			$clen eq "LEN: $len\n"
			    or die "client: $clen", "len $len expected";
		}
		unless ($args{server}{nocheck}) {
			my $slen = shift @slen;
			$slen eq "LEN: $len\n"
			    or die "server: $slen", "len $len expected";
		}
	}
}

sub check_md5 {
	my ($c, $r, $s, %args) = @_;

	my $cmd5 = $c->loggrep(qr/^MD5: /) unless $args{client}{nocheck};
	my $smd5 = $s->loggrep(qr/^MD5: /) unless $args{server}{nocheck};
	!$cmd5 || !$smd5 || ref($args{md5}) eq 'ARRAY' || $cmd5 eq $smd5
	    or die "client: $cmd5", "server: $smd5", "md5 mismatch";
	my $md5 = ref($args{md5}) eq 'ARRAY' ?
	    join('|', @{$args{md5}}) : $args{md5};
	!$md5 || !$cmd5 || $cmd5 =~ /^MD5: ($md5)$/
	    or die "client: $cmd5", "md5 $md5 expected";
	!$md5 || !$smd5 || $smd5 =~ /^MD5: ($md5)$/
	    or die "server: $smd5", "md5 $md5 expected";
}

sub check_loggrep {
	my ($c, $r, $s, %args) = @_;

	my %name2proc = (client => $c, relayd => $r, server => $s);
	foreach my $name (qw(client relayd server)) {
		my $p = $name2proc{$name}
		    or next;
		my $pattern = $args{$name}{loggrep} or next;
		$pattern = [ $pattern ] unless ref($pattern) eq 'ARRAY';
		foreach my $pat (@$pattern) {
			if (ref($pat) eq 'HASH') {
				while (my($re, $num) = each %$pat) {
					my @matches = $p->loggrep($re);
					@matches == $num
					    or die "$name matches @matches: ",
					    "$re => $num";
				}
			} else {
				$p->loggrep($pat)
				    or die "$name log missing pattern: $pat";
			}
		}
	}
}

1;
