#!/usr/bin/perl

######################################################################
# INCOMPLETE
#
# This works for the "sample", but gets the wrong answer for the real
# input that I was given. I'm sure I'm on the right track, but I'm
# putting this to rest now and am moving on. :-)
######################################################################

# Oh hello, Tetris

use strict;
use warnings;
use JSON::XS;

$| = 1;

my $DEBUG = 0;

my $line = <STDIN>;
chomp($line);
my $jet_total = length($line);

my $verbose = 0;
my $chamber_height_init = 4;
my $chamber_width = 7;
my @chamber = ();
for (my $i = 0; $i < $chamber_height_init; $i++) {
    push @chamber, '.'x$chamber_width;
}
my $truncated = 0;
my $rock = -1;
my $falling = 0;

# Rock 0:
#
# ..@@@@.
#
# Rock 1:
#
# ...@...
# ..@@@..
# ...@...
#
# Rock 2:
#
# ....@..
# ....@..
# ..@@@..
#
# Rock 3:
#
# ..@...
# ..@...
# ..@...
# ..@...
#
# Rock 4:
#
# ..@@...
# ..@@...

my %masks = (
    0 => ['@@@@'],
    1 => [' @ ',
	  '@@@',
	  ' @ '],
    2 => ['  @',
	  '  @',
	  '@@@'],
    3 => ['@',
	  '@',
	  '@',
	  '@'],
    4 => ['@@',
	  '@@']
    );
my @mask = ();
my $height;
my ($rock_x, $rock_y, $rock_height, $rock_width);

sub chamber_space {
    my $space = 0;
    foreach my $line (@chamber) {
	last if $line =~ /#/;
	$space++;
    }
    return $space;
}

sub chamber_code {
    my $code = "";
    my $empty = 1;
    for (my $y = 0; $y < @chamber; $y++) {
	my $line = $chamber[$y];
	my $byte = 0;
	my $factor = 1;
	for (my $x = 0; $x < $chamber_width; $x++) {
	    if (substr($line, $x, 1) eq '#') {
		$byte += $factor;
	    }
	    $factor *= 2;
	}
	$empty = 0 if $empty && $byte > 0;
	$code .= sprintf("%02x", $byte) if !$empty;
    }
    return $code;
}

sub display_chamber {
    for (my $y = 0; $y < @chamber; $y++) {
	my $line = $chamber[$y];
	if (@mask > 0) {
	    if ($y >= $rock_y && $y < $rock_y + $height) {
		my $y_offset = $y - $rock_y;

		for (my $x = 0; $x < $chamber_width; $x++) {
		    if ($x >= $rock_x && $x < $rock_x + length($mask[$y_offset]) &&
			substr($mask[$y_offset], $x - $rock_x, 1) eq '@') {
			substr($line, $x, 1, '@');
		    }
		}
	    }
	}
	print("|$line|\n");
    }
    print("`-------'\n");
}

# Return 0 if there is no overlap, I guess
sub chamber_overlap {
    my ($rock_x, $rock_y) = @_;

    for (my $y = 0; $y < @chamber; $y++) {
	my $line = $chamber[$y];
	if ($y >= $rock_y && $y < $rock_y + $height) {
	    my $y_offset = $y - $rock_y;
	    for (my $x = 0; $x < $chamber_width; $x++) {
		if ($x >= $rock_x && $x < $rock_x + length($mask[$y_offset]) &&
		    substr($mask[$y_offset], $x - $rock_x, 1) eq '@' &&
		    substr($line, $x, 1) eq '#') {
		    return 1;
		}
	    }
	}
    }
    return 0;
}

sub solidify_rock {
    for (my $y = 0; $y < $rock_height; $y++) {
	for (my $x = 0; $x < $rock_width; $x++) {
	    if (substr($mask[$y], $x, 1) eq '@') {
		substr($chamber[$y + $rock_y], $x + $rock_x, 1, '#');
	    }
	}
    }
}

sub truncate_chamber {
    for (my $y = 0; $y < @chamber - 1; $y++) {
	my $blocked = 1;
	for (my $x = 0; $x < 7; $x++) {
	    if (substr($chamber[$y],     $x, 1) ne '#' &&
		substr($chamber[$y + 1], $x, 1) ne '#') {
		$blocked = 0;
		last;
	    }
	}
	if ($blocked == 1) {
	    my $keep = $y + 2;
	    my $to_truncate = scalar(@chamber) - $keep;
	    $truncated += scalar(@chamber) - $keep;
	    splice(@chamber, $keep);
	}
    }
}

my $total = 1_000_000_000_000;

print("rocks wanted: $total\n");
print("We'll run until we see a repeat combination of:\n");
print("- (truncated) chamber\n");
print("- last rock to drop\n");
print("- last jet pattern position\n");
print("and then extrapolate from there\n");
print("\n");

my %codes_seen = ();

my %heights = ();
my $last_height = 0;
my $started = time();
my $rocks = 0;
my $i = 0;
my $iteration = 0;
my $duplicate_code;

sub stack_height {
    return ;
}

while (1) {
    if (!$falling) {
	truncate_chamber();

	if (1) {
	    $iteration++;
	    my $height = scalar(@chamber) - chamber_space() + $truncated;
	    my $code = sprintf("%d/%d/%s", $i, $rock, chamber_code());
	    my $diff = $height - $last_height;
	    $last_height = $height;
	    printf("Making a note of the height after %7d rocks: %7d (diff: %5d) CODE: %s\n",
		   $rocks, $height, $diff, $code) if $DEBUG;
	    $heights{$iteration} = $diff;
	    if (defined($codes_seen{$code})) {
		print("Repeat spotted!\n");
		$duplicate_code = $code;
		last;
	    } else {
		$codes_seen{$code} = $iteration;
	    }
	}
	$rock = ($rock + 1) % 5;
	$rocks++;
	@mask = @{ $masks{$rock} };
	$height = scalar(@mask);
	my $space = chamber_space();
	my $needed = $height + 3 - $space;
	while ($needed > 0) {
	    @chamber = ('.'x$chamber_width, @chamber);
	    $needed--;
	}
	while ($needed < 0) {
	    shift @chamber;
	    $needed++;
	}
	$falling = 1;
	$rock_y = 0;
	$rock_x = 2;
	$rock_width = length($mask[0]);
	$rock_height = scalar(@mask);
    }
    # display_chamber();
    my $jet = substr($line, $i, 1);
    if ($jet eq '>') {
	# To the right, if possible
	if ($rock_x + $rock_width < $chamber_width &&
	    chamber_overlap($rock_x + 1, $rock_y) == 0) {
	    $rock_x++;
	}
    } elsif ($jet eq '<') {
	# To the left, if possible
	if ($rock_x > 0 &&
	    chamber_overlap($rock_x - 1, $rock_y) == 0) {
	    $rock_x--;
	}
    } else {
	die "Unexpected jet direction: $jet\n";
    }
    if ($falling) {
	if ($rock_y + $rock_height == scalar(@chamber)) {
	    solidify_rock();
	    @mask = ();
	    $falling = 0;
	} else {
	    if (chamber_overlap($rock_x, $rock_y + 1) == 0) {
		$rock_y++;
	    } else {
		solidify_rock();
		@mask = ();
		$falling = 0;
	    }
	}
    }
    # Repeat the jet pattern over and over
    $i = ($i + 1) % length($line);
}
printf("Chamber height: %d; Space: %d; Truncated rows: %d\nHeight of structure: %d\n",
       scalar(@chamber), chamber_space(), $truncated,
       scalar(@chamber) - chamber_space() + $truncated);

my $structure = scalar(@chamber) - chamber_space() + $truncated;

printf("EXTRAPOLATION TIME:\n");
if ($DEBUG) {
    printf("\nHEIGHTS:\n%s", new JSON::XS->pretty(1)->encode(\%heights));
    printf("\nCODES SEEN:\n%s", new JSON::XS->pretty(1)->encode(\%codes_seen));
}
printf("Repeat code was %s; at rock %d, first hit was %d\n", $duplicate_code, $iteration, $codes_seen{$duplicate_code});
my $pattern = $iteration - $codes_seen{$duplicate_code};
my $remaining = $total - $rocks;
my $diff_total = 0;
printf("Summarize from %d to %d?\n", $codes_seen{$duplicate_code} + 1, $iteration);
for (my $iter = $codes_seen{$duplicate_code} + 1; $iter <= $iteration; $iter++) {
    $diff_total += $heights{$iter};
}
printf("Height increase for the loop of %d rocks is %d\n", $pattern, $diff_total);
printf("We're still %d rotations away from %d\n",
       $remaining, $total);
printf("That's %d full repeats of the pattern of %d; and then %d more steps.\n",
       int($remaining / $pattern), $pattern, $remaining % $pattern);

printf("That should create a structure of %d + %d * %d = %d; plus remaining %d steps' worth\n",
       $structure, $diff_total, int($remaining / $pattern),
       $structure + $diff_total * int($remaining / $pattern),
       $remaining % $pattern);
$structure += $diff_total * int($remaining / $pattern);
for (my $iter = 0; $iter < $remaining % $pattern; $iter++) {
    $structure += $heights{$codes_seen{$duplicate_code} + 1 + $iter};
}
printf("Final answer: %d\n", $structure);
