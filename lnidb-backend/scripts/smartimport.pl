#!/usr/bin/perl

#
# uso: como lnidb,
# cd importacao
# wherever/linkdir.pl /home/backup/dirs
# wherever/smartimport.pl *
#

use DBI;
use File::Basename;

if (@ARGV < 1) { die("syntax: smartimport.pl [-d] dir [dir ...]"); }

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");
my $import_dest = "/home/lnidb2/dicom";

my $delay = 0;

foreach $x (@ARGV) {
    if ($x eq '-d') {
	$delay = 1;
	next;
    }

    $base = basename($x);

    if ($base =~ /^DISCO(\d+)([A-Za-z]*)$/) {
	$n = $1;
	$s = $2;
	while (length($n) < 4) { $n = '0'.$n; }
	$storage = "backup${n}${s}";
	$storage =~ tr/A-Z/a-z/;
    } elsif ($base =~ /^backup(\d+)([A-Za-z]*)$/) {
	$n = $1;
	$s = $2;
	while (length($n) < 4) { $n = '0'.$n; }
	$storage = "backup${n}${s}";
	$storage =~ tr/A-Z/a-z/;
    } elsif ($base =~ /^BACKUP(\d+)([A-Za-z]*)$/) {
	$n = $1;
	$s = $2;
	while (length($n) < 4) { $n = '0'.$n; }
	$storage = "backup${n}${s}";
	$storage =~ tr/A-Z/a-z/;
    } elsif ($base =~ /^PQ_(\d+)_([ABab])/) {
	$n = $1;
	$s = $2;
	$n = sprintf("%02d",$n);
	$storage = "pq${n}${s}";
	$storage =~ tr/A-Z/a-z/;
    } elsif ($base =~ /^UCP_(\d+)_([ABab])/) {
	$n = $1;
	$s = $2;
	$n = sprintf("%03d",$n);
	$storage = "ucp${n}${s}";
	$storage =~ tr/A-Z/a-z/;
    } elsif ($base =~ /^ucp(\d+)([ABab])/) {
	$n = $1;
	$s = $2;
	$n = sprintf("%03d",$n);
	$storage = "ucp${n}${s}";
	$storage =~ tr/A-Z/a-z/;
    } else {
	print "no match for $base, skipping\n";
	next;
    }
    print "$base to be imported as $storage\n";

    $dstate = ($delay==0 ? 0 : 4);
    
    $sql = "insert into tasks (id,state,task,src,options,started,creator) values (nextval('task_id'),$dstate,'import-dicom','$base','$storage','now',10);";
    $db->do($sql);

}
