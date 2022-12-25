#!/usr/bin/perl

use strict;
use warnings;

my %conversion = (
    '2' => 2,
    '1' => 1,
    '0' => 0,
    '-' => -1,
    '=' => -2
    );

sub parse_snafu {
    my ($snafu) = @_;

    my $factor = 1;
    my $number = 0;
    for (my $i = length($snafu) - 1; $i >= 0; $i--) {
	my $digit = substr($snafu, $i, 1);
	$number += $conversion{$digit} * $factor;
	$factor *= 5;
    }
    return $number;
}

sub print_as_snafu {
    my ($n) = @_;

    my $snafu = "2";
    while (parse_snafu($snafu) < $n) {
	$snafu .= "2";
    }
    for (my $i = 0; $i < length($snafu); $i++) {
	foreach my $let ("1", "0", "-", "=") {
	    my $alt_snafu = $snafu;
	    substr($alt_snafu, $i, 1, "$let");
	    if (parse_snafu($alt_snafu) < $n) {
		last;
	    }
	    $snafu = $alt_snafu;
	}
    }
    return $snafu;
}

sub main {
    printf("%20s  %20s\n", "Decimal", "SNAFU");
    my $sum = 0;
    while (my $snafu = <STDIN>) {
	chomp($snafu);
	my $number = parse_snafu($snafu);
	printf("%20d  %20s (%s)\n", $number, $snafu, print_as_snafu($number));
	$sum += $number;
    }
    printf("\nSUM: %d (%s)\n", $sum, print_as_snafu($sum));
}

main() if !caller();
