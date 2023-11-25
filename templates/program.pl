#!/usr/bin/perl

use strict;
use warnings;

use Getopt::Long;
use Pod::Usage;

my %options = (
    debug   => 0,
    dryrun  => 0,
    verbose => 0,
    help    => 0,
    manual  => 0
);

sub parse_options()
{
    GetOptions("help|h"    => \$options{help},
               "manual|m"  => \$options{manual},
               "verbose|v" => \$options{verbose},
               "dryrun|n"  => \$options{dryrun},
               "debug|d"   => \$options{debug}) ||
        pod2usage(-verbose => 0,
                  -message => " ", # Force a line-break
                  -exitval => 1);
    if ($options{help}) {
        pod2usage(-verbose => 1,
                  -exitval => 0);
    } elsif ($options{manual}) {
        pod2usage(-verbose => 2,
                  -exitval => 0);
    }
    if (@ARGV > 0) {
        pod2usage(-verbose => 1,
                  -message => "Too many parameters given.\n");
    }
}

sub main()
{
    parse_options();
    while (my $line = <STDIN>) {
	chomp($line);
	if ($options{debug}) {
	    printf("DEBUG: Line received: %s\n", $line);
	}
    }
    
}

main() if !caller();

__END__

=pod

=head1 NAME

programZ - Program for AoC YYYY puzzles; Day X, part Z

=head1 SYNOPSIS

programZ.pl [-d|-debug|-n|--dryrun|-m|--manual|-v|--verbose]

=head1 DESCRIPTION

This program is used for one of the AoC YYYY puzzles; Day X; part Z.

=head1 OPTIONS

=over 5

=item B<--debug>, B<-d>

Enable debugging output.

=item B<--dryrun>, B<-n>

Request that the program operates in dryrun (noop) mode.

=item B<--manual>, B<-m>

Provide built-in (unix) manual style documentation.

=item B<--verbose>, B<-v>

Enable verbose output.

=back

=cut
