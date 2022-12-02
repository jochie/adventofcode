#!/usr/bin/perl

my $max = -1;
my $sum = 0;
my $elf = 0;
my @list = ();
while (<>) {
    chomp;
    # Separator
    if (/^$/) {
	if ($sum > 0) {
	    $elf++;
	    # print("Elf $elf: $sum\n");
	    if ($sum > $max) {
		$max = $sum;
	    }
	    push @list, $sum;
	}
	$sum = 0;
    } else {
	$sum += $_;
    }
}

if ($sum > 0) {
    $elf++;
    # print("Elf $elf: $sum\n");
    if ($sum > $max) {
	$max = $sum;
    }
    push @list, $sum;
}
print("$max\n");
