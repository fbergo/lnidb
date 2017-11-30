#!/usr/bin/perl

# $Id: gendercheck.pl,v 1.1 2013/10/04 11:14:17 bergo Exp $

use DBI;
use File::Basename;
use Cwd;
use POSIX;

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");
my $dicomroot   = '/home/lnidb2/dicom';

$sql = "select distinct name from patients order by name;";
$q = $db->prepare($sql);
$q->execute();

$n = $q->rows;
@fullnames = ();
for($i=0;$i<$n;$i++) {
    @row = $q->fetchrow_array;
    push @fullnames, $row[0];
}
$q->finish;

$nf = @fullnames;

print "$nf fullnames\n";

%seen = ();
@names = ();
foreach (@fullnames) {
    if (/^([A-Z]+)\s+/) {
	unless ($seen{$1}) {
	    push @names, $1;
	    $seen{$1} = 1;
	}
    }
}

$nn = @names;
print "$nn first names\n";

%male = ();
%female = ();

print "counting...\n";
foreach (@names) {
    $sql = "select count(id) from patients where gender = 'M' and name ~* '^$_ ';";
    $q = $db->prepare($sql);
    $q->execute();
    @row = $q->fetchrow_array;    
    $male{$_} = $row[0];
    $q->finish;

    $sql = "select count(id) from patients where gender = 'F' and name ~* '^$_ ';";
    $q = $db->prepare($sql);
    $q->execute();
    @row = $q->fetchrow_array;    
    $female{$_} = $row[0];
    $q->finish;
}

foreach (@names) {
    next if ($male{$_} == 0 || $female{$_} == 0 || $male{$_} == $female{$_});
    if ($male{$_} > $female{$_}) {
	$outcast = 'F';
    } else {
	$outcast = 'M';
    }
    print "Found outcasts for '$_' (M=$male{$_} F=$female{$_}):\n";
    $sql = "select name, age, gender from patients where gender = '$outcast' and name ~* '^$_ ';";
    $q = $db->prepare($sql);
    $q->execute();
    $n = $q->rows;
    for($i=0;$i<$n;$i++) {
	@row = $q->fetchrow_array;
	print "$row[0], $row[1], $row[2]\n";
    }
    $q->finish;
    print "\n";
}

%total = ();

foreach (keys %male) {
    $total{$_} = $male{$_} + $female{$_};
}

sub desc {
    $total{$b} <=> $total{$a};
}

print "Most frequent first names:\n";
foreach (sort desc (keys(%total))) {
    last if ($total{$_} < 5);
    print "$_ ($total{$_})\n";
}

