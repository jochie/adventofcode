#!/usr/bin/perl

use strict;
use warnings;

$| = 1;
my @sections = ();

sub plot_grid {
    my ($s_x, $s_y, $b_x, $b_y, $checking_x, $checking_y) = @_;

    # The distance between Sensor and Beacon
    my $distance = abs($s_x - $b_x) + abs($s_y - $b_y);

    # How far away is the target line
    my $y_offset = abs($checking_y - $s_y);

    # Too far away to reach the target line
    return if $y_offset > $distance;

    # At the line, how much left/right do we have
    my $left = $distance - $y_offset;
    my $min_x = $s_x - $left;
    my $max_x = $s_x + $left;
    return if $min_x < 0 && $max_x < 0; # completely out of sight on the left
    return if $min_x > $checking_x && $max_x > $checking_x; # completely out of sight on the right
    if ($min_x < 0) {
	$min_x = 0;
    }
    if ($max_x > $checking_x) {
	$max_x = $checking_x;
    }
    push @sections, [ $min_x, $max_x, "#", [ $s_x, $s_y, $b_x, $b_y] ];
}

sub main {
    my $checking = $ARGV[0];
    my @sensors = ();
    while (<STDIN>) {
	chomp;
	if (/^Sensor at x=(-?\d+), y=(-?\d+): closest beacon is at x=(-?\d+), y=(-?\d+)$/) {
	    push @sensors, [ $1, $2, $3, $4 ];
	}
    }
    for (my $y = 0; $y <= $checking; $y++) {
	@sections = ();
	foreach my $sensor (@sensors) {
	    if (0) {
	    # Put the sensor on the map, if relevant
	    if ($sensor->[1] eq $y && $sensor->[0] >= 0 && $sensor->[0] <= $checking) {
		push @sections, [ $sensor->[0], $sensor->[0], "S" ];
	    }
	    # Put the beacon on the map, if relevant
	    if ($sensor->[3] eq $y && $sensor->[2] >= 0 && $sensor->[2] <= $checking) {
		push @sections, [ $sensor->[2], $sensor->[2], "B" ];
	    }
	    }
	    plot_grid(@$sensor, $checking, $y);
	}
	# Sort the sections by their start position
	@sections = sort { $a->[0] <=> $b->[0] } @sections;

	my $x = -1;
	foreach my $section (@sections) {
	    # printf "  %7d :: %d..%d\n", $x, $section->[0], $section->[1];
	    if ($section->[0] > $x + 1) {
		printf "Found a gap at %d,%d\n", $x + 1, $y;
		printf "That means a tuning frequency of %d\n", ($x + 1) * 4000000 + $y;
		print "SECTIONS WERE:\n";
		foreach my $section (@sections) {
		    if (scalar(@$section) > 3) {
			printf("  [%7d..%7d] %s (%s)\n", $section->[0], $section->[1], $section->[2], join(', ', @{ $section->[3] }));
		    } else {
			printf("  [%7d..%7d] %s\n", $section->[0], $section->[1], $section->[2]);
		    }
		}
		exit(1);
	    }
	    # Ignore sections that we've already passed
	    next if $section->[1] <= $x;
	    $x = $section->[1];
	}
	if ($x < $checking) {
	    printf "Found a gap at %d,%d\n", $x + 1, $y;
	    printf "That means a tuning frequency of %d\n", ($x + 1) * 4000000 + $y;
	    print "SECTIONS WERE:\n";
	    foreach my $section (@sections) {
		if (scalar(@$section) > 3) {
		    printf("  [%7d..%7d] %s (%s)\n", $section->[0], $section->[1], $section->[2], join(', ', @{ $section->[3] }));
		} else {
		    printf("  [%7d..%7d] %s\n", $section->[0], $section->[1], $section->[2]);
		}
	    }
	    exit(1);
	}
    }
}

main();
