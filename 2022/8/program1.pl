#!/usr/bin/perl

use strict;
use warnings;

my @grid = ();

while (<>) {
    chomp;
    push @grid, [ split(//, $_) ];
}

my $height = scalar(@grid);
my $width  = scalar(@{ $grid[0] });
my $edge_trees = ($height - 1 + $width - 1) * 2;

sub is_visible {
    my ($row, $col) = @_;
    
    my $tree_height = $grid[$row]->[$col];

    # Check left, right, above, and below the starting point.
    # It's visible unless something blocks it.

    my $visible = 1;
    for (my $r = 0; $r < $row; $r++) {
	if ($grid[$r]->[$col] >= $tree_height) {
	    $visible = 0;
	    last;
	}
    }
    return 1 if $visible;
    $visible = 1;
    for (my $r = $row + 1; $r < $height; $r++) {
	if ($grid[$r]->[$col] >= $tree_height) {
	    $visible = 0;
	    last;
	}
    }
    return 1 if $visible;
    $visible = 1;
    for (my $c = 0; $c < $col; $c++) {
	if ($grid[$row]->[$c] >= $tree_height) {
	    $visible = 0;
	    last;
	}
    }
    return 1 if $visible;
    $visible = 1;
    for (my $c = $col + 1; $c < $width; $c++) {
	if ($grid[$row]->[$c] >= $tree_height) {
	    $visible = 0;
	    last;
	}
    }
    return $visible;
}

my $visible_trees = 0;
for (my $row = 1; $row < $height - 1; $row++) {
    for (my $col = 1; $col < $width - 1; $col++) {
	$visible_trees++ if is_visible($row, $col);
    }
}
printf("Total: %d + %d = %d\n", $edge_trees, $visible_trees, $edge_trees + $visible_trees);
