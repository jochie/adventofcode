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
# print("Max: $max\n");

@list = sort { $b <=> $a } @list;
# print(join(" ", @list).$/);
my $top3 = $list[0] + $list[1] + $list[2];
print("$top3\n");
