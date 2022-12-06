#!/usr/bin/perl

use strict;
use warnings;

my $line = <>;
chomp($line);

for (my $i = 0; $i < length($line) - 3; $i++) {
    my $four = substr($line, $i, 4);
    print "Checking at $i: $four\n";
    my %set = map { $_ => 1 } (split('', $four));
    if (scalar(keys %set) == 4) {
	print "Four unique characters at $i: $four\n";
	print "Marker at ".($i + 4)."\n";
	exit;
    }
}
