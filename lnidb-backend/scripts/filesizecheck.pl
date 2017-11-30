#!/usr/bin/perl

use DBI;
use File::Basename;
use Cwd;
use POSIX;

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");

my $disk = $ARGV[0];

$sql = "select id,path,size from files where path ~ '.*$disk.*' order by id;";
$q = $db->prepare($sql);
$q->execute();
while(@row = $q->fetchrow_array ) {
    $rs = (stat($row[1]))[7];
    
    print "id=$row[0] path=$row[1] ";
    if ($rs == $row[2]) {
	print "OK ($rs)\n";
    } else {
	print "mismatch (file $rs db $row[2])\n";
    }
}
$q->finish;

