#!/usr/bin/perl

# $Id: storagedel.pl,v 1.1 2013/10/04 11:14:18 bergo Exp $

use DBI;
use File::Basename;
use Cwd;
use POSIX;

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");
my $dicomroot   = '/home/lnidb2/dicom';

sub del_storage;
sub del_directory;

foreach $x (@ARGV) {
    del_storage($x);
}

# patient cleanup

$npat = 0;
$sql = "select id,name from patients;";
$q = $db->prepare($sql);
$q->execute();

$n = $q->rows();

for($i=0;$i<$n;$i++) {
    @row = $q->fetchrow_array;
    $sql = "select count(pid) from patstudies where pid=$row[0];";
    $r = $db->prepare($sql);
    $r->execute();
    @r2 = $r->fetchrow_array;
    if ($r2[0] == 0) {
	$npat++;
	$sql = "begin; delete from patscanners where pid=$row[0]; delete from cvisible using comments c where cid=c.id and c.pid=$row[0]; ";
	$sql .= "delete from comments where pid=$row[0]; delete from patients where id=$row[0]; commit; ";
	$db->do($sql);
	print "..cleanup $row[1]\n";
    }
    $r->finish();
}
print "$npat patients removed.\n";

sub del_storage {
    my ($storage) = @_;

    print "removing storage [$storage]\n";

    $sql  = "begin; ";
    $sql .= "select f.id into tempsdel from files f, filestudies fs, studies s where f.id=fs.fid and fs.sid=s.id and s.storage='$storage'; ";
    $sql .= "delete from filestudies x using studies s where x.sid=s.id and s.storage='$storage'; ";
    $sql .= "delete from patstudies x using studies s where x.sid=s.id and s.storage='$storage'; ";
    $sql .= "delete from files x using tempsdel t where t.id=x.id; ";
    $sql .= "delete from studies where storage='$storage'; ";
    $sql .= "drop table tempsdel; ";
    $sql .= "commit; ";

    $rows = $db->do($sql);
    print "..affected rows = $rows\n";

    my $x = 0;
    if (-d "$dicomroot/$storage") {
	my $dir = "$dicomroot/$storage";
	print "..removing dicom files: $dir\n";
	$x = del_directory($dir);
	print ".. $x items removed\n";
    } else {
	print ".. dicom directory missing, nothing to remove.\n";
    }

    $sql = "insert into tasks (id,state,message,task,src,options,creator,started,ended) values (nextval('task_id'),2,'$x arquivos removidos','storage-delete','$storage','',10,'now','now');";
    $db->do($sql);
}

sub del_directory {
    my ($dir) = @_;

    unless(opendir(DIR,$dir)) {
	print "** error reading directory $dir.\n";
	return 0;
    }
    
    my $count = 0;
    my @files = readdir(DIR);
    closedir(DIR);
    foreach $i (@files) {
	next if ($i eq '.' || $i eq '..');
	my $full = "$dir/$i";
	if (-d $full) {
	    $count += del_directory($full);
	    rmdir($full);
	    print ".. removed directory $full\n";
	    $count++;
	} else {
	    unlink($full);
	    $count++;
	    print ".. removed file $full\n";
	}
    }

    print ".. removed directory $dir\n";
    rmdir($dir);
    $count++;
    return $count;
}
