#!/usr/bin/perl

use strict;
use warnings;

$| = 1;

my $cycle = 0;
my $busy = 0;
my $xreg = 1;
my $addx = 0;

my @rows = ("_"x40, "_"x40, "_"x40, "_"x40, "_"x40, "_"x40);
my $row = -1;
my $col = -1;

sub draw_sprite {
    my $min_sprite = $xreg - 1;
    my $max_sprite = $xreg + 1;
    my $chr = ($min_sprite <= $col && $col <= $max_sprite) ? "#" : ".";
    substr($rows[$row], $col, 1, $chr);
}

while (1) {
    $cycle++;
    if ($cycle % 40 == 1) {
	$row++;
    }
    $col = ($col + 1) % 40;
    if (!$busy) {
	# Fetch next instruction
	my $cmd = <>;
	last if !defined($cmd);
	chomp($cmd);
	if ($cmd =~ /^addx (-?\d+)$/) {
	    $busy = 2;
	    $addx = $1;
	}
    }
    draw_sprite();
    if ($busy > 0) {
	$busy--;
	if ($busy == 0) {
	    $xreg += $addx;
	}
    }
}

print join("\n", @rows)."\n";
