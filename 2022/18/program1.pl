#!/usr/bin/perl

# Boiling Boulders

use strict;
use warnings;

my @max = (0, 0, 0);
my @data = ();

my $sides = 0;
my %grid = ();

sub grid_key {
    my ($x, $y, $z) = @_;

    return "$x,$y,$z";
}

while (<STDIN>) {
    chomp;
    my @row = split(/,/);
    push @data, \@row;
    for (my $i = 0; $i < 3; $i++) {
	if ($row[$i] > $max[$i]) {
	    $max[$i] = $row[$i];
	}
    }
    $grid{grid_key(@row)} = 1;
    $sides += 6;
}

sub solo_cube {
    my ($x, $y, $z) = @_;

    return 0 if defined($grid{grid_key($x, $y, $z - 1)});
    return 0 if defined($grid{grid_key($x, $y, $z + 1)});
    return 0 if defined($grid{grid_key($x, $y - 1, $z)});
    return 0 if defined($grid{grid_key($x, $y + 1, $z)});
    return 0 if defined($grid{grid_key($x - 1, $y, $z)});
    return 0 if defined($grid{grid_key($x + 1, $y, $z)});
    
}

for (my $x = 0; $x <= $max[0]; $x++) {
    for (my $y = 0; $y <= $max[1]; $y++) {
	for (my $z = 0; $z <= $max[2]; $z++) {
	    if (defined($grid{grid_key($x,$y,$z)})) {
		print "Occupied: $x,$y,$z\n";
		if (defined($grid{grid_key($x + 1, $y, $z)})) {
		    printf("Touching with (%d,%d,%d)\n", $x + 1, $y, $z);
		    $sides -= 2;
		}
		if (defined($grid{grid_key($x, $y + 1, $z)})) {
		    printf("Touching with (%d,%d,%d)\n", $x, $y + 1, $z);
		    $sides -= 2;
		}
		if (defined($grid{grid_key($x, $y, $z + 1)})) {
		    printf("Touching with (%d,%d,%d)\n", $x, $y, $z + 1);
		    $sides -= 2;
		}
		# Sanity check... doesn't seem to be the case though.
		if (solo_cube($x, $y, $z)) {
		    print "Free floating cube at ($x,$y,$z)?\n";
		    $sides -= 6;
		}
	    }
	}
    }
}

print "@max\n";
print "sides: $sides\n";
