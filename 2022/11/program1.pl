#!/usr/bin/perl

use strict;
use warnings;

use JSON::XS;

my %monkeys = ();
my $monkey = undef;
my $data = { inspections => 0 };

while (<>) {
    chomp;
    next if /^\s*$/;

    if (/^Monkey (\d+):/) {
	if (defined($monkey)) {
	    print "Storing for $monkey: ".encode_json($data)."\n";
	    $monkeys{$monkey} = $data;
	    $data = { inspections => 0 };
	}
	$monkey = $1;
	next;
    }
    if (/^\s*Starting items: (.*)$/) {
	$data->{items} = [ split(/, /, $1) ];
    } elsif (/^\s*Operation: new = old (.) (\d+|old)$/) {
	$data->{operation} = [ $1, $2 ];
    } elsif (/^\s*Test: divisible by (\d+)$/) {
	$data->{test} = $1;
    } elsif (/^\s*If true: throw to monkey (\d+)$/i) {
	$data->{true} = $1;
    } elsif (/^\s*If false: throw to monkey (\d+)$/i) {
	$data->{false} = $1;
    } else {
	print "Unrecognized $_\n";
    }
}
if (defined($monkey)) {
    print "Storing for $monkey: ".encode_json($data)."\n";
    $monkeys{$monkey} = $data;
}

#print(encode_json(\%monkeys).$/);
#exit;

my @monkeys = sort keys %monkeys;

for (my $round = 1; $round <= 20; $round++) {
    for $monkey (@monkeys) {
	my $mref = $monkeys{$monkey};
	print "Monkey $monkey:\n";
	foreach my $item (@{ $mref->{items} }) {
	    $mref->{inspections}++;
	    print "  Inspecting item with worry level of $item\n";
	    my ($op, $rhs) = @{ $mref->{operation} };
	    print "    new = old $op $rhs\n";
	    my $old = $item;
	    if ($rhs eq 'old') {
		$rhs = $old;
	    }
	    my $new = ($op eq '+') ? ($old + $rhs) : ($old * $rhs);
	    print "    Worry level is now: $new\n";
	    $new = int($new / 3);
	    print "    Worry level is dividied by 3 to $new\n";
	    if ($new % $mref->{test}) {
		print "    Level is not visible by $mref->{test} -> monkey $mref->{false}\n";
		push @{ $monkeys{$mref->{false}}->{items} }, $new;
	    } else {
		print "    Level is visible by $mref->{test} -> monkey $mref->{true}\n";
		push @{ $monkeys{$mref->{true}}->{items} }, $new;
	    }
	}
	$mref->{items} = [];
    }
    print "ROUND $round:\n";
    for $monkey (@monkeys) {
	print "  Monkey $monkey: ".join(', ', @{ $monkeys{$monkey}->{items} })."\n";
    }
}

my @active = ();
print "INSPECTIONS:\n";
foreach my $monkey (@monkeys) {
    my $val = $monkeys{$monkey}->{inspections};
    print "Monkey $monkey inspected items $val\n";
    push @active, $val;
}
@active = sort { $b <=> $a } @active;
print "Monkey business: $active[0] * $active[1] = ".($active[0] * $active[1])."\n";
