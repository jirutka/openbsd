use strict;
use warnings;

our %args = (
    client => {
	func => \&http_client,
	path => "a/b/c/d/e/f/gindex.html",
	loggrep => [
	    qr/403 Forbidden/,
	    qr/Server: OpenBSD relayd/,
	    qr/Connection: close/,
	],
	httpnok => 1,
    },
    relayd => {
	protocol => [ "http",
	    'return error',
	    'url filter digest 0ac8ccfc03317891ae2820de10ee2167d31ebd16 log',
	],
	loggrep => {
	    qr/rejecting request \(403 Forbidden\)/ => 1,
	    qr/\[0ac8ccfc03317891ae2820de10ee2167d31ebd16:/ => 1,
	},
    },
    server => {
	noserver => 1,
	nocheck => 1,
    },
);

1;
