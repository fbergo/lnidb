#!/usr/bin/perl

use DBI;

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");

if (@ARGV != 1) {
    print "usage: patientsbydisk.pl prefix\n";
    exit 1;
}

$prefix = $ARGV[0];

$sql = "select distinct storage from studies where storage ~* '^$prefix' order by 1;";
$q = $db->prepare($sql);
$q->execute();

@discos = ();

while(@row = $q->fetchrow_array) {
    push @discos, @row;
}

@discos = sort @discos;

foreach $d (@discos) {
    $sql = "select distinct p.name, p.patcode from patients p, studies s, patstudies ps where p.id=ps.pid and s.id=ps.sid and s.storage = '$d' order by 1,2;";
    $r = $db->prepare($sql);
    $r->execute();

    print "Disco: $d\n";
    print "---------------\n";
    

    $i = 1;
    while(@row = $r->fetchrow_array) {
	print "$i - $row[0] ($row[1])\n";
	$i++;
    }

    print "\n";
}


