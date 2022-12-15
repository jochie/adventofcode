#!/usr/bin/perl

use strict;
use warnings;

my %grid = ();
my ($min_x, $max_x, $min_y, $max_y);

sub update_limits {
    my ($x, $y) = @_;

    if (!defined($min_x) || $x < $min_x) {
	$min_x = $x;
    }
    if (!defined($max_x) || $x > $max_x) {
	$max_x = $x;
    }
    if (!defined($min_y) || $y < $min_y) {
	$min_y = $y;
    }
    if (!defined($max_y) || $y > $max_y) {
	$max_y = $y;
    }
}

sub plot_grid {
    my ($s_x, $s_y, $b_x, $b_y, $checking) = @_;

    update_limits($s_x, $s_y);
    update_limits($b_x, $b_y);
    my $distance = abs($s_x - $b_x) + abs($s_y - $b_y);
    for (my $y_offset = -$distance; $y_offset <= $distance; $y_offset++) {
	my $left = $distance - abs($y_offset);
	my $y = $s_y + $y_offset;
	# next if $y != $checking;
	for (my $x_offset = -$left; $x_offset <= $left; $x_offset++) {
	    my $x = $s_x + $x_offset;
	    update_limits($x, $y);
	    if (!defined($grid{"$x,$y"})) {
		$grid{"$x,$y"} = "#";
	    }
	}
    }
}

sub draw_grid {
    for (my $y = $min_y; $y <= $max_y; $y++) {
	my $result = sprintf("%3d ", $y);
	for (my $x = $min_x; $x <= $max_x; $x++) {
	    $result .= $grid{"$x,$y"} || ".";
	}
	print("$result\n");
    }
}

sub check_row {
    my ($y) = @_;

    my $markers = 0;
    for (my $x = $min_x; $x <= $max_x; $x++) {
	next if !defined($grid{"$x,$y"});
	$markers += $grid{"$x,$y"} eq '#';
    }
    return $markers;
}

sub main {
    my $checking = $ARGV[0];
    while (<STDIN>) {
	chomp;
	if (/^Sensor at x=(-?\d+), y=(-?\d+): closest beacon is at x=(-?\d+), y=(-?\d+)$/) {
	    $grid{"$1,$2"} = "S";
	    $grid{"$3,$4"} = "B";
	    plot_grid($1, $2, $3, $4, $checking);
	} else {
	    die "Parsing error: $_\n";
	}
    }
    print("How many # markers on row 10: ".check_row($checking)."\n");
    draw_grid();
}

main();
