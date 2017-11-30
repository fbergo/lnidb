#!/usr/bin/perl

# $Id: updateorient.pl,v 1.1 2013/10/04 11:14:18 bergo Exp $

use DBI;
use File::Basename;
use Cwd;
use POSIX;

sub dicom_hash;
sub compute_orientations;
sub roundcos;

my $dicom       = "dicom2scn";
my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");

$sql = "select id from studies where oaxi+osag+ocor = 0 order by id;";
$x = $db->prepare($sql);
$x->execute();

$N = $x->rows();
$gone = 0;

for($j=0;$j<$N;$j++) {

    @xx = $x->fetchrow_array;
    $sql = "select f.id,f.path from files f, filestudies fs where f.id=fs.fid and fs.sid=$xx[0];";
    $q = $db->prepare($sql);
    $q->execute();

    $n = $q->rows;

    print "study has $n files\n";

    for($i=0;$i<$n;$i++) {
	@row = $q->fetchrow_array;

	print "processing [id=$row[0] path=$row[1]]\n";
	$ref = dicom_hash($row[1]);
	@orient = compute_orientations($ref);

	$oc = '';
	if ($orient[0]) { $oc .= "oaxi=1,"; }
	if ($orient[1]) { $oc .= "osag=1,"; }
	if ($orient[2]) { $oc .= "ocor=1,"; }

	if ($oc =~ /(.*),$/) {
	    $oc = $1;
	    $sql = "update studies set $oc where id=$xx[0];";
	    print "sql=[$sql]\n";
	    $db->do($sql);
	}
    }

    $gone++;
    $pct =  (100.0 * $gone) / $N;
    $pct = sprintf("%.2f",$pct);
    print "concluido: $gone de $N ($pct %) [id=$xx[0]]\n";
}

print "concluido.\n";

sub dicom_hash {
    my ($file) = @_;
    my %dicom = ();
    open(DCM,"$dicom -d \"$file\"| grep 0020:0037|") or return {};
    while($_ = <DCM>) {
        if (/^(\w{4}:\w{4})\s+\(\d+\)\s+\[(.*?)\s*\]/) {
	    my $k = $1;
	    my $kk = $k;
	    my $n = 0;
	    my $val = $2;
	    while($dicom{$kk}) {
		$n++;
		$kk = "$k$n";
	    }
            $dicom{$kk} = $val;
        }
    }
    close(DCM);
    return \%dicom;
}

sub roundcos {
    my ($x) = @_;
    my $pi = 3.1415926535;
    if (abs($x) < cos(45.5 * $pi / 180.0)) { return 0; }
    if (abs($x) > cos(44.5 * $pi / 180.0)) {
	if ($x < 0.0) { return -1; } else { return 1; }
    }
    return 2;
}

sub compute_orientations {
    my ($ref) = @_;
    my %ldicom = %$ref;
    my $i;
    my ($axi,$sag,$cor) = (0,0,0);
    my $x = '';

    foreach (keys %ldicom) {
	if (/^0020:0037/) {
	    my @val = split( /\\/, $ldicom{$_} );
	    next if (@val != 6);
	    my @rval;
	    for($i=0;$i<6;$i++) {
		$rval[$i] = roundcos($val[$i]);
	    }
	    print "val=($val[0],$val[1],$val[2],$val[3],$val[4],$val[5])\n";
	    print "rval=($rval[0],$rval[1],$rval[2],$rval[3],$rval[4],$rval[5])\n";
	    $axi = 1 if ($rval[0]==0 && $rval[2]==0 && $rval[4]==0 && $rval[5]==0); # ap/rl
	    $axi = 1 if ($rval[1]==0 && $rval[2]==0 && $rval[3]==0 && $rval[5]==0); # rl/ap
	    $sag = 1 if ($rval[0]==0 && $rval[2]==0 && $rval[3]==0 && $rval[4]==0); # ap/fh
	    $sag = 1 if ($rval[0]==0 && $rval[1]==0 && $rval[3]==0 && $rval[5]==0); # fh/ap	    
	    $cor = 1 if ($rval[0]==0 && $rval[1]==0 && $rval[4]==0 && $rval[5]==0); # fh/rl
	    $cor = 1 if ($rval[1]==0 && $rval[2]==0 && $rval[3]==0 && $rval[4]==0); # rl/fh
	}
    }

    print "ori=$axi,$sag,$cor\n";
    return($axi,$sag,$cor);
}

