#!/usr/bin/perl

# Oh hello, Tetris

use strict;
use warnings;

use constant DEBUG => 0;

$| = 1;
my $clear = qx{tput clear};
my $lines = qx{tput lines};
chomp($lines);

my $line = <STDIN>;
chomp($line);

my $chamber_height_init = 4;
my $chamber_width = 7;
my @chamber = ();
for (my $i = 0; $i < $chamber_height_init; $i++) {
    push @chamber, '.'x$chamber_width;
}

my $rock = -1;
my $falling = 0;
my $rocks = 0;
my $i = 0;

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

sub display_chamber {
    my $prefix = '   ';
    print($clear);
    print("Rock $rocks; Jet $i\n");
    for (my $y = 0; $y < @chamber; $y++) {
	if ($y >= $lines - 4) {
	    # Truncate what's shown
	    print("$prefix:       :\n");
	    return;
	}
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
	print("$prefix|$line|\n");
    }
    print("$prefix`-------'\n");
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

while (1) {
    if (!$falling) {
	$rock = ($rock + 1) % 5;
	$rocks++;
	if ($rocks == 2023) {
	    last;
	}
	@mask = @{ $masks{$rock} };
	$height = scalar(@mask);
	my $space = chamber_space();
	print("rock $rock ($rocks); height $height; space $space\n") if DEBUG;
	my $needed = $height + 3 - $space;
	while ($needed > 0) {
	    @chamber = ('.'x$chamber_width, @chamber);
	    $needed--;
	}
	$falling = 1;
	$rock_y = -$needed;
	$rock_x = 2;
	$rock_width = length($mask[0]);
	$rock_height = scalar(@mask);
    }
    display_chamber();
    my $jet = substr($line, $i, 1);
    print("\nJET: $jet ($i)\n") if DEBUG;
    if ($jet eq '>') {
	# To the right, if possible
	if ($rock_x + $rock_width < $chamber_width) {
	    if (chamber_overlap($rock_x + 1, $rock_y) == 0) {
		print("Moved to the right.\n") if DEBUG;
		$rock_x++;
	    } else {
		print("Blocked by rock from moving to the right.\n") if DEBUG;
	    }
	} else {
	    print("Bump into chamber wall on the right.\n") if DEBUG;
	}
    } elsif ($jet eq '<') {
	# To the left, if possible
	if ($rock_x > 0) {
	    if (chamber_overlap($rock_x - 1, $rock_y) == 0) {
		print("Moved to the left.\n") if DEBUG;
		$rock_x--;
	    } else {
		print("Blocked by rock from moving to the left.\n") if DEBUG;
	    }
	} else {
	    print("Bump into chamber wall on the left.\n") if DEBUG;
	}
    } else {
	die "Unexpected jet direction: $jet\n";
    }
    if ($falling) {
	if ($rock_y + $rock_height == scalar(@chamber)) {
	    print("We hit the bottom\n") if DEBUG;
	    solidify_rock();
	    @mask = ();
	    $falling = 0;
	} else {
	    if (chamber_overlap($rock_x, $rock_y + 1) == 0) {
		print("Moved one row down.\n") if DEBUG;
		$rock_y++;
	    } else {
		print("Running into other rocks below..\n") if DEBUG;
		solidify_rock();
		@mask = ();
		$falling = 0;
	    }
	}
    }
    # Repeat the jet pattern over and over
    $i = ($i + 1) % length($line);

    # Quick partial-second sleep:
    select(undef, undef, undef, 0.05) if !DEBUG;
}
printf("Chamber height: %d; Space: %d -- Height of structure: %d\n",
       scalar(@chamber), chamber_space(), scalar(@chamber) - chamber_space());

# Should be between 2560 and 3414 for actual input.
