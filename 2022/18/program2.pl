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

sub calculate_surface {
    my $sides = 0;
    for (my $x = 0; $x <= $max[0]; $x++) {
	for (my $y = 0; $y <= $max[1]; $y++) {
	    for (my $z = 0; $z <= $max[2]; $z++) {
		if (defined($grid{grid_key($x,$y,$z)})) {
		    $sides += 6;
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
    return $sides;
}

print "@max\n";

# First fill the grid, which means the only unreachable spots should be the
# air pockets
sub fill_grid {
    # Find an unoccupied spot on the outside, to start from

    # For simplification
    for (my $x = 0; $x <= $max[0]; $x++) {
	for (my $y = 0; $y <= $max[1]; $y++) {
	    next if defined($grid{grid_key($x, $y, 0)});

	    fill_grid_iter($x, $y, 0);
	    return;
	}
    }
}

sub fill_grid_iter {
    my ($x, $y, $z) = @_;

    my $filled = 0;
    my @coords = ([$x, $y, $z]);
    while (@coords > 0) {
	my @new_coords = ();
	foreach my $coord (@coords) {
	    # Fill that spot
	    my ($x, $y, $z) = @$coord;

	    # Did someone get here first?
	    if (defined($grid{grid_key($x, $y, $z)})) {
		next;
	    }
	    # printf("Filling (%d,%d,%d)\n", $x, $y, $z);
	    $grid{grid_key($x, $y, $z)} = 1;
	    $filled++;

	    push @new_coords, [ $x - 1, $y, $z ] if $x > 0       && !defined($grid{grid_key($x - 1, $y, $z)});
	    push @new_coords, [ $x + 1, $y, $z ] if $x < $max[0] && !defined($grid{grid_key($x + 1, $y, $z)});

	    push @new_coords, [ $x, $y - 1, $z ] if $y > 0       && !defined($grid{grid_key($x, $y - 1, $z)});
	    push @new_coords, [ $x, $y + 1, $z ] if $y < $max[1] && !defined($grid{grid_key($x, $y + 1, $z)});

	    push @new_coords, [ $x, $y, $z - 1 ] if $z > 0       && !defined($grid{grid_key($x, $y, $z - 1)});
	    push @new_coords, [ $x, $y, $z + 1 ] if $z < $max[2] && !defined($grid{grid_key($x, $y, $z + 1)});
	}
	@coords = @new_coords;
    }
    printf("Filled %d coordinates in the grid\n", $filled);
}

sub reverse_grid {
    for (my $x = 0; $x <= $max[0]; $x++) {
	for (my $y = 0; $y <= $max[1]; $y++) {
	    for (my $z = 0; $z <= $max[2]; $z++) {
		if (defined($grid{grid_key($x, $y, $z)})) {
		    delete $grid{grid_key($x, $y, $z)};
		} else {
		    $grid{grid_key($x, $y, $z)} = 1;
		}
	    }
	}
    }
}

# Part 1
printf("cubes: %d\n", scalar(@data));
my $total_sides = calculate_surface();
printf("sides: %d\n", $total_sides);

# Part 2
fill_grid();

# Part 3
reverse_grid();

my $inner_sides = calculate_surface();
printf("inner sides: %d\n", $inner_sides);

printf("Exterior sides then is: %d\n", $total_sides - $inner_sides);
