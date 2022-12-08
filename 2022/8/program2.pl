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

sub scenic_score {
    my ($row, $col) = @_;

    my $tree_height = $grid[$row]->[$col];

    # Travel left, right, up, and down, from the starting point.

    my $score = 1;
    my $partial = 0;
    for (my $r = $row - 1; $r >= 0; $r--) {
	$partial++;
	last if $grid[$r]->[$col] >= $tree_height;
    }
    $score *= $partial;
    $partial = 0;
    for (my $r = $row + 1; $r < $height; $r++) {
	$partial++;
	last if $grid[$r]->[$col] >= $tree_height;
    }
    $score *= $partial;
    $partial = 0;
    for (my $c = $col - 1; $c >= 0; $c--) {
	$partial++;
	last if $grid[$row]->[$c] >= $tree_height;
    }
    $score *= $partial;
    $partial = 0;
    for (my $c = $col + 1; $c < $width; $c++) {
	$partial++;
	last if $grid[$row]->[$c] >= $tree_height;
    }
    $score *= $partial;
    return $score;
}

my $max_score = 0;
for (my $row = 1; $row < $height - 1; $row++) {
    for (my $col = 1; $col < $width - 1; $col++) {
	my $new_score = scenic_score($row, $col);
	if ($new_score > $max_score) {
	    $max_score = $new_score;
	}
    }
}
print("maximum scenic score: $max_score\n");
