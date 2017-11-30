#!/usr/bin/perl

# $Id: perldbr.pl,v 1.1 2012/10/09 20:16:59 bergo Exp $

my $dicom2scn = "dicom2scn";

foreach $item (@ARGV) {

    unless (-e $item) {
	print "** perldbr.pl: file not found $item\n";
	next;
    }

    unless (open(DUMP,"$dicom2scn -d $item|")) {
	print "** perldbr.pl: unable to read $item\n";
	next;
    }

    $patient = '';
    $seq1 = '';
    $seq2 = '';
    $date = '';
    $time = '';
    $order = '';
    $mf = '';

    while($_=<DUMP>) {
	$patient = $1 if ($patient=='' && /^0010:0010.*\[(.*)\]/);
	$seq1    = $1 if ($seq1=='' && /^0008:103e.*\[(.*)\]/);
	$seq2    = $1 if ($seq2=='' && /^0008:1030.*\[(.*)\]/);
	$date    = $1 if ($date=='' && /^0008:0020.*\[(.*)\]/);
	$time    = $1 if ($time=='' && /^0008:0030.*\[(.*)\]/);
	$order   = $1 if ($order=='' && /^0020:0013.*\[(.*)\]/);
	$mf      = $1 if ($mf=='' && /^0028:0008.*\[(.*)\]/);
    }
    close(DUMP);

#    print "pat=[$patient]\n";
#    print "s1=[$seq1]\n";
#    print "s2=[$seq2]\n";
#    print "date=[$date]\n";
#    print "time=[$time]\n";
#    print "order=[$order]\n";
#    print "mf=[$mf]\n";

    $seq1 = $seq2 if (length($seq1) == '');

    if (length($patient)==0 || length($seq1)==0 || length($date)==0 || length($time)==0) {
	print "**perldbr.pl: missing mandatory field (patient/seq/date/time) on $item\n";
	next;
    }

    # canonize

    $date =~ s/[^0-9]//g;
    if (length($date)==8) {
	$date = substr($date,0,4) . '-' . substr($date,4,2) . '-' . substr($date,6,2);
    }

    $time =~ s/[^0-9]//g;
    if (length($time)==6) {
	$time = substr($time,0,4);
    }
    if (length($time)==4) {
	$time = substr($time,0,2) . '-' . substr($time,2,2);
    }

    $patient =~ s/[ \t\r\n\.]/_/g;
    $patient =~ s/[^A-Za-z0-9_]//g;
    $patient =~ s/^_+//;
    $patient =~ s/_+$//;

    $seq1 =~ s/[ \t\r\n\.]/_/g;
    $seq1 =~ s/[^A-Za-z0-9_]//g;
    $seq1 =~ s/^_+//;
    $seq1 =~ s/_+$//;

    $newname = "$date-$time-$patient-$seq1";

    if ($mf > 1) {
	$newname .= "-ENH";
    }

    $pos = '';
    if (length($order) > 0) {
	$pos = sprintf("%04d",$order);
    }

    $dest = $newname . '-' . $pos;
    next if ($dest eq $item);

    $i = 0;
    while (-e $dest && $dest ne $item) {
	$i++;
	$dest = "$newname-S$i-$pos";
    }

#    print "name1: $item\n";
#    print "name2: $dest\n";
    
    next if ($item eq $dest);

    unless (rename($item,$dest)) {
	print "** perldbr.pl: rename $item => $dest failed.\n";
    }

}

