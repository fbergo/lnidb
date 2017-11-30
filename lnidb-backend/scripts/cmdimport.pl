#!/usr/bin/perl

# $Id: cmdimport.pl,v 1.1 2013/10/04 11:14:17 bergo Exp $

use DBI;
use File::Basename;
use Cwd;
use POSIX;

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");
my $import_src  = "/home/lnidb/importacao";
my $import_dest = "/home/lnidb2/dicom";

($dir,$storage) = @ARGV;

if (! -d "${import_src}/$dir") { die("diretorio inexistente"); }
if (length($dir)==0 || length($storage)==0) { die("parametros ausentes, cmdimport.pl dir storage"); }

$sql = "insert into tasks (id,state,task,src,options,started,creator) values (nextval('task_id'),0,'import-dicom','$dir','$storage','now',10);";
$db->do($sql);

