#!/usr/bin/perl

use strict;
use warnings;

my $line = 0;
my %positions = ();

$positions{"500,0"} = "+";

my $min_x = 500;
my $max_x = 500;
my $max_y = 0;

while (<>) {
    $line++;
    chomp;
    my @parts = split(/ -> /);
    print "LINE $line\n";
    my $part = shift @parts;
    my ($x1, $y1) = split(/,/, $part);
    if ($x1 < $min_x) {
	$min_x = $x1;
    }
    if ($x1 > $max_x) {
	$max_x = $x1;
    }
    if ($y1 > $max_y) {
	$max_y = $y1;
    }
    foreach my $part (@parts) {
	my ($x2, $y2) = split(/,/, $part);
	if ($x2 < $min_x) {
	    $min_x = $x2;
	}
	if ($x2 > $max_x) {
	    $max_x = $x2;
	}
	if ($y2 > $max_y) {
	    $max_y = $y2;
	}
	if ($x1 == $x2) {
	    print "  ($x1,$y1 -> $y2)\n";
	    if ($y1 > $y2) {
		for (my $y = $y2; $y <= $y1; $y++) {
		    $positions{"$x1,$y"} = "#";
		}
	    } else {
		for (my $y = $y1; $y <= $y2; $y++) {
		    $positions{"$x1,$y"} = "#";
		}
	    }
	} elsif ($y1 == $y2) {
	    print "  ($x1 -> $x2,$y1)\n";
	    if ($x1 > $x2) {
		for (my $x = $x2; $x <= $x1; $x++) {
		    $positions{"$x,$y1"} = "#";
		}
	    } else {
		for (my $x = $x1; $x <= $x2; $x++) {
		    $positions{"$x,$y1"} = "#";
		}
	    }
	} else {
	    die "What? $_\n";
	}
	($x1, $y1) = ($x2, $y2);
    }
}

print "min_x / max_x = $min_x / $max_x\n";
print "max_y = $max_y\n";

sub show_grid {
    for (my $y = 0; $y <= $max_y; $y++) {
	my $line = "";
	for (my $x = $min_x; $x <= $max_x; $x++) {
	    $line .= $positions{"$x,$y"} || ".";
	}
	print "$line\n";
    }
}

# Return 1 if it came to rest, 0 if it fell into the "abyss"
sub simulate_grain {
    my $g_x = 500;
    my $g_y = 0;

    while (1) {
	if ($g_y > $max_y) {
	    # This grain has fallen beyond the lowest point, a.k.a the abyss
	    return 0;
	}
	# Can it drop one straight down?
	if (!defined($positions{sprintf("%d,%d", $g_x, $g_y + 1)})) {
	    # Yes, nothing is there.
	    $g_y++;
	    next;
	}
	if (!defined($positions{sprintf("%d,%d", $g_x - 1, $g_y + 1)})) {
	    # Diagonal to the left is available.
	    $g_y++;
	    $g_x--;
	    if ($g_x < $min_x) {
		$min_x = $g_x;
	    }
	    next;
	}
	if (!defined($positions{sprintf("%d,%d", $g_x + 1, $g_y + 1)})) {
	    # Diagonal to the right is available.
	    $g_y++;
	    $g_x++;
	    if ($g_x > $max_x) {
		$max_x = $g_x;
	    }
	    next;
	}
	# None available, the grain stays here.
	$positions{"$g_x,$g_y"} = "o";
	return 1;
    }
}

print "Starting point:\n";
show_grid();
my $grains = 0;
while (1) {
    if (!simulate_grain()) {
	print "Grain after $grains grains dropped into the abyss.\n";
	last;
    }
    print "\n";
    $grains++;
    print "A total of $grains grains of sand...\n";
    show_grid();
}
