#!/usr/bin/perl

use strict;
use warnings;

# Trying to do this efficiently
#
# @numbers contains the current set of numbers, rearranged by the algorithm
# @indices keeps track of where each number is now, given the original index
# @reverse tells us which original-index entry is at a current index

my @numbers = ();
my @indices = ();
my @reverse = ();

my $zero_index;

my $i = 0;
while (<STDIN>) {
    chomp;
    push @numbers, $_;
    push @indices, $i;
    push @reverse, $i;
    if ($_ eq '0') {
	$zero_index = $i;
    }
    $i++;
}
my $total = $i;

sub dump_numbers {
    print("\n");
    for (my $i = 0; $i < $total; $i++) {
	printf("#%d: %d (originally #%d) -- the original is at %d\n", $i, $numbers[$i], $reverse[$i], $indices[$i]);
    }
}

# dump_numbers();

sub swap_forward {
    my ($i) = @_;

    my $index = $indices[$i];
    my $number = $numbers[$index];

    # printf("swap_forward(%d) => %d at %d\n", $i, $number, $index);

    if ($index > $total - 2) {
	for (my $j = 0; $j < $total - 2; $j++) {
	    swap_reverse($i);
	}
    } else {
	my $next = $index + 1;

	# Which entry is at the next spot
	my $next_index = $reverse[$next];
	my $next_number = $numbers[$next];
	# printf("Which original-index is at %d: %d (value: %d)\n", $next, $next_index, $next_number);

	# Swap the numbers
	$numbers[$index]   = $next_number;
	$numbers[$next] = $number;

	# Where is the ${i}th number now
	$indices[$i] = $next;
	$indices[$next_index] = $index;

	# What 
    	$reverse[$next] = $i;
	$reverse[$index] = $next_index;
    }

    # dump_numbers();
    # sleep(1);
}

sub swap_reverse {
    my ($i) = @_;

    my $index = $indices[$i];
    my $number = $numbers[$indices[$i]];

    # printf("swap_reverse($i) => $number at $index\n");

    if ($index < 2) {
	for (my $j = 0; $j < $total - 2; $j++) {
	    swap_forward($i);
	}
    } else {
	my $next = $index - 1;


	# Which entry is at the next spot
	my $next_index = $reverse[$next];
	my $next_number = $numbers[$next];
	# printf("Which original-index is at %d: %d (value: %d)\n", $next, $next_index, $next_number);

	# Swap the numbers
	$numbers[$index]   = $next_number;
	$numbers[$next] = $number;

	# Where is the ${i}th number now
	$indices[$i] = $next;
	$indices[$next_index] = $index;

	# What 
    	$reverse[$next] = $i;
	$reverse[$index] = $next_index;
    }
}

for (my $i = 0; $i < $total; $i++) {
    my $index = $indices[$i];
    my $number = $numbers[$indices[$i]];
    # printf("\nNumber %d (at offset %d) is %d\n", $i, $index, $number);
    next if $number == 0;
    if ($number > 0) {
	for (my $j = 0; $j < $number; $j++) {
	    swap_forward($i);
	}
	# dump_numbers();
    } else {
	for (my $j = 0; $j < -$number; $j++) {
	    swap_reverse($i);
	}
	# dump_numbers();
    }
    # sleep(1);
    # dump_numbers();
}

print "The original index of zero was: $zero_index\n";
my $new_zero_index = $indices[$zero_index];
print "The current index of zero is: $new_zero_index\n";
my $offset_1000 = ($new_zero_index + 1000) % $total;
print "The 1000th number after 0 would be at ($new_zero_index + 1000) % $total = $offset_1000\n";
print "That number would be $numbers[$offset_1000]?\n";

my $offset_2000 = ($new_zero_index + 2000) % $total;
print "The 1000th number after 0 would be at ($new_zero_index + 2000) % $total = $offset_2000\n";
print "That number would be $numbers[$offset_2000]?\n";

my $offset_3000 = ($new_zero_index + 3000) % $total;
print "The 1000th number after 0 would be at ($new_zero_index + 3000) % $total = $offset_3000\n";
print "That number would be $numbers[$offset_3000]?\n";

print "\n";
printf("Sum of these numbers is: %d\n",
       $numbers[$offset_1000] + $numbers[$offset_2000] + $numbers[$offset_3000]);
