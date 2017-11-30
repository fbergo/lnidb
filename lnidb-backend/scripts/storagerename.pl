#!/usr/bin/perl

# $Id: storagerename.pl,v 1.1 2013/10/04 11:14:18 bergo Exp $

use DBI;
use File::Basename;
use Cwd;
use POSIX;

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");
my $dicomroot   = '/home/lnidb2/dicom';

if (@ARGV != 2) {
    print "usage: storagerename.pl from to\n";
    return 1;
}

($from,$to) = @ARGV;

if ($to eq '') {
    print "error: destination name cannot be empty.\n";
    return 1;
}

$sql = "select count(id) from studies where storage='$to';";
$q = $db->prepare($sql);
$q->execute();

@row = $q->fetchrow_array;

if ($row[0] != 0) {
    print "error: destination name already exists.\n";
    return 1;
}

print "renaming [$from] to [$to]...\n";

print "updating studies table (1/3)...\n";
$sql = "update studies set storage='$to' where storage='$from';";
$db->do($sql);

print "updating files table (2/3)...\n";
$sql = "update files set path = replace(path,'$dicomroot/$from/','$dicomroot/$to/') where path ~ '^$dicomroot/$from';";
$db->do($sql);

print "renaming directory (3/3)...\n";
system("/bin/mv $dicomroot/$from $dicomroot/$to");

$sql = "insert into tasks (id,state,message,task,src,options,creator,started,ended) values (nextval('task_id'),2,'','storage-rename','$from','$to',10,'now','now');";
$db->do($sql);

print "finished.\n";
