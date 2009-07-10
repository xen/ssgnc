#! /usr/bin/perl -T

use strict;
use warnings;

my $data_dir = shift @ARGV
	or die "Usage: - DataDir [BinPath]\n";
my $bin_path = shift @ARGV || "ssgnc-search";

## The following script generates a CGI perl script.

print << 'END_OF_FIRST_PART';
#! /usr/bin/perl

use strict;
use warnings;

use CGI;

## Settings.
END_OF_FIRST_PART

print << "END_OF_SETTINGS";
my \$data_dir = \"$data_dir\";
my \$bin_path = \"$bin_path\";
END_OF_SETTINGS

print << 'END_OF_LATTER_PART';

## Gets parameters from a CGI request.
my $cgi = CGI->new;
my $query = $cgi->param("q") || $cgi->param("query") || "";
my $order = $cgi->param("t") || $cgi->param("order") || "UNORDERED";
my $min_freq = $cgi->param("f") || $cgi->param("min_freq") || "0";
my $max_results = $cgi->param("m") || $cgi->param("max_results") || "100";
my $n_range = $cgi->param("n") || $cgi->param("n_range") || "1-100";
my $content_type = $cgi->param("c") || $cgi->param("content_type") || "open";

## Checks keywords.
if ((lc $min_freq) eq "all") { $min_freq = 0; }
if ((lc $max_results) eq "all") { $max_results = 2147483647; }
if ((lc $n_range) eq "all") { $n_range = "1-"; }

## Removes single quotes from parameters.
$query =~ s/'//g;
$order =~ s/'//g;
$min_freq =~ s/'//g;
$max_results =~ s/'//g;
$n_range =~ s/'//g;

## Prints an HTTP response header.
if ($content_type eq "save")
{
	print("Content-Type: text/download; name=\"ngram.txt\"\n");
	print("Content-Disposition: attachment; filename=\"ngram.txt\"\n\n");
}
else
{
	print("Content-Type: text/plain; charset=utf-8\n\n");
}

## Executes a command to search n-grams.
open(CMD, "| $bin_path -d \'$data_dir\' -o \'$order\'"
	. " -r \'$max_results\' -f \'$min_freq\' -n \'$n_range\'");
print(CMD "$query");
close(CMD);

exit
END_OF_LATTER_PART
