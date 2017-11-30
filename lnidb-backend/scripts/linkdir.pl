#!/usr/bin/perl

use File::Basename;

# linkdir.pl orig [orig orig ...]
# creates a copy of orig directory in . and hard links all files inside. 
# only works if orig has no subdirs.

if (@ARGV < 1) { die("syntax: linkdir.pl orig [...]"); }

foreach $x (@ARGV) {
    print "linking $x\n";
    $base = basename($x);
    mkdir($base);
    system("ln $x/* $base");
}
