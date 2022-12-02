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
    'B' => 1,
    'C' => 2
    );

my %value = (
    0 => 1, # rock
    1 => 2, # paper
    2 => 3, # scissors
    );

my %losers = (
    0 => 2,
    1 => 0,
    2 => 1,
    );

my %winners = (
    0 => 1,
    1 => 2,
    2 => 0,
    );

sub score {
    my ($p1, $p2) = @_;

    if ($p2 eq "X") {
	# need to lose
	$p2 = $losers{$id{$p1}};
	return $value{$p2};
    }
    if ($p2 eq 'Y') {
	# need to draw
	return 3 + $value{$id{$p1}};
    }
    # Should be $p2 eq 'Z' 
    # need to win
    $p2 = $winners{$id{$p1}};
    return 6 + $value{$p2};
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
