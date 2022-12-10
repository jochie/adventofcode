#!/usr/bin/perl

use strict;
use warnings;

$| = 1;

my $cycle = 0;
my $busy = 0;
my $xreg = 1;
my $addx = 0;

my $sum = 0;

while (1) {
    $cycle++;
    if (!$busy) {
	my $cmd = <>;
	last if !defined($cmd);
	chomp($cmd);
	if ($cmd =~ /^addx (-?\d+)$/) {
	    $busy = 2;
	    $addx = $1;
	}
    }
    if ($cycle >= 20 && ($cycle - 20) % 40 == 0) {
	print "-- $xreg * $cycle = ".($xreg * $cycle)."\n";
	$sum += $xreg * $cycle;
    }
    if ($busy > 0) {
	$busy--;
	if ($busy == 0) {
	    $xreg += $addx;
	}
    }
}


print "SUM: $sum\n";

