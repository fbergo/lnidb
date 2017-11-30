#!/usr/bin/perl

# merges patient records when birthdate has been fixed on pq but ucp was imported later

use DBI;
use File::Basename;
use Cwd;
use POSIX;

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");
my $n = 0;

sub maybedo;
sub sqlsafe;

$dry = 0;
$dry = 1 if ($ARGV[0] eq 'dry');

$sql = "select a.id,a.name,a.birth,b.id,b.name,b.birth from patients a, patients b where a.name=b.name and a.patcode=b.patcode and a.birth<>b.birth and a.gender=b.gender and b.birth = '1900-01-01';";

$q = $db->prepare($sql);
$q->execute();

print "*** dry run!\n" if ($dry);

while( @row = $q->fetchrow_array ) {

    $n++;
    print "merging ($row[3],$row[4],$row[5]) to ($row[0],$row[1],$row[2])\n";
    $row[1] = sqlsafe($row[1]);
    $row[4] = sqlsafe($row[4]);

    $aid = $row[0];
    $bid = $row[3];

    $sql = "select sid from patscanners where pid=$bid except select sid from patscanners where pid=$aid;";
    $q2 = $db->prepare($sql);
    $q2->execute();

    # remove all references of b.id
    maybedo("begin;",$dry);

    # update all references from b.id to a.id
    maybedo("update patstudies set pid=$aid where pid=$bid;",$dry);
    maybedo("update patattachs set pid=$aid where pid=$bid;",$dry);
    maybedo("update comments set pid=$aid where pid=$bid;",$dry);

    while( @sid = $q2->fetchrow_array ) {
	maybedo("update patscanners set pid=$aid where pid=$bid and sid=$sid[0];",$dry);
    }

    maybedo("delete from patscanners where pid=$bid;",$dry);
    
    # remove duplicate
    maybedo("delete from patients where id=$bid;",$dry);

    # add edit log
    maybedo("insert into editlog (uid,edited,message) values (10,'now','Paciente duplicado corrigido por script ucpmerge.pl ($row[3],$row[4],$row[5] => $row[0],$row[1],$row[2])');",$dry);
    maybedo("commit;",$dry);
}

print "pacientes afetados: $n\n";

sub maybedo {
    my($cmd,$d) = @_;
    if ($d == 0) {
	$db->do($cmd);
    } else {
	print "dry run: $cmd\n";
    }
}

sub sqlsafe {
    my ($x) = @_;
    $x =~ s/\n//g;
    $x =~ s/\r//g;
    $x =~ s/\'/\'\'/g;
    return $x;
}
