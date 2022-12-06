#!/usr/bin/perl

use strict;
use warnings;

my $line = <>;
chomp($line);

for (my $i = 0; $i < length($line) - 3; $i++) {
    my $fourteen = substr($line, $i, 14);
    print "Checking at $i: $fourteen\n";
    my %set = map { $_ => 1 } (split('', $fourteen));
    if (scalar(keys %set) == 14) {
	print "Fourteen unique characters at $i: $fourteen\n";
	print "Marker at ".($i + 14)."\n";
	exit;
    }
}
