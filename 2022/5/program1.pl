#!/usr/bin/perl

use JSON::XS;
my @stacks = ();

# Read stack configuration
while (<>) {
    chomp;
    last if /^$/;

    if (substr($_, 1, 1) eq '1') {
	# Last line, with stack numbers, is irrelevant
	next;
    }
    my $stack = 0;
    for (my $i = 1; $i < length($_); $i += 4) {
	if (substr($_, $i, 1) =~ /[A-Z]/) {
	    push @{ $stacks[$stack] }, substr($_, $i, 1);
	}
	$stack++;
    }
}

print(encode_json(\@stacks), "\n");

# Read move instructions and execute them
sub move_from_to {
    my ($total, $from_stack, $to_stack) = @_;

    while ($total > 0) {
	my $val = shift(@{ $stacks[$from_stack-1] });
	unshift(@{ $stacks[$to_stack-1] }, $val);
	$total--;
    }
}

while (<>) {
    chomp;
    if (/^move (\d+) from (\d+) to (\d+)/) {
	move_from_to($1, $2, $3);
    }
}

print(encode_json(\@stacks), "\n");

print("Stack tops:\n");
my $index = 1;
my $result = "";
foreach my $stack (@stacks) {
    print "$index - $stack->[0]\n";
    $result .= $stack->[0];
    $index++;
}

print "Output: $result\n";
