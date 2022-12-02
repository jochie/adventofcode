#!/usr/bin/perl

my $score = 0;

# A and X = rock (1 point)
# B and Y = paper (2 points)
# C and Z = scissors (3 points)
# 3 points for a draw
# 6 points for winning
#
# rock > scissors
# scissors > paper
# paper > rock
my %id = (
    'A' => 0,
    'X' => 0,
    'B' => 1,
    'Y' => 1,
    'C' => 2,
    'Z' => 2
    );

my %value = (
    0 => 1, # rock
    1 => 2, # paper
    2 => 3, # scissors
    );

sub score {
    my ($p1, $p2) = @_;

    $p1 = $id{$p1};
    $p2 = $id{$p2};
    if ($p1 == $p2) {
	# draw
	return 3 + $value{$p2};
    }
    if ($p2 == 0 && $p1 == 2 ||
	$p2 == 2 && $p1 == 1 ||
	$p2 == 1 && $p1 == 0) {
	return 6 + $value{$p2};
    }
    return $value{$p2};
}

my $sum = 0;
while (<>) {
    chomp;
    my ($a, $b) = split(/ /, $_);
    my $score = score($a, $b);
    print "$a vs $b => $score\n";
    $sum += $score;
}
print "Sum: $sum\n";

__DATA__
A Y
B X
C Z
