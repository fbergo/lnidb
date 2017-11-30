#!/usr/bin/perl

use DBI;

my $db          = DBI->connect("dbi:Pg:dbname=lnidb;user=lnidb");
my $wrapdir     = '/home/lnidb2/basket';

# check if all files in the db table are present

print "Verificando integridade de pacotes listados no banco de dados\n";

$sql = 'select w.path,w.size,u.username from wrapped w, users u where u.id=w.owner order by 1;';
$q = $db->prepare($sql);
$q->execute();

$n = $q->rows;
print "$n pacotes\n";

$p = 0;
$a = 0;
$w = 0;

while(@row = $q->fetchrow_array) {
    if (! -f $row[0] ) {
	$a++;
	print "$row[0] ($row[1]) ausente\n";
	next;
    }
    @s = stat($row[0]);
    if ($s[7] != $row[1]) {
	$w++;
	print "$row[0] ($row[1]): tamanho inconsistente (db=$row[1], arquivo local $s[7])\n";
	next;
    }
    $p++;
}

print "Presentes: $p\n";
print "Inconsistentes: $w\n";
print "Ausentes: $a\n\n";

# check for stray files

print "Verificando arquivos não referenciados em $wrapdir\n";
opendir(WD,$wrapdir) or die('oops');

sub human_size;
sub killdir;
sub killfile;

@arq = readdir(WD);
closedir(WD);

$f = 0;
$d = 0;
$e = 0;
$ft = 0;
@straydirs = ();
@strayfiles = ();


@arq = sort @arq;
foreach $i (@arq) {
    $fp = "$wrapdir/$i";
    next if ($i =~ /^\./);
    if (-d $fp) {
	print "diretório perdido: $i\n";
	push @straydirs, $fp;
	$d++;
	next;
    }
    $sql = "select tid from wrapped where path = '$fp';";
    $q = $db->prepare($sql);
    $q->execute();
    if ($q->rows == 0) {
	@s = stat($fp);
	$hs = human_size($s[7]);
	$ft += $s[7];
	print "arquivo perdido: $i ($hs)\n";
	push @strayfiles, $fp;
	$f++;
	next;
    } else {
	$e++;
    }
}

$fths = human_size($ft);

print "Diretórios perdidos: $d\n";
print "Arquivos perdidos: $f ($fths)\n";
print "Arquivos referenciados: $e\n";


if (@straydirs) {
    print "\nRemover $d diretórios perdidos (s/N) ? ";
    $x = <>;
    if ($x =~ /^s/i) {
	foreach (@straydirs) {
	    killdir($_);
	}
    }
}

if (@strayfiles) {
    print "\nRemover $f arquivos perdidos (s/N) ? ";
    $x = <>;
    if ($x =~ /^s/i) {
	foreach (@strayfiles) {
	    killfile($_);
	}
    }
}

sub killdir {
    my ($d) = @_;
    my @c;
    my $f;
    my $fp;
    opendir(TD, $d) or return 0;
    print "..removendo diretório $d...\n";
    @c = readdir(TD);
    closedir(TD);
    @c = sort @c;
    foreach $f (@c) {
	next if ($f eq '.' || $f eq '..');
	$fp = "$d/$f";
	if (-d $fp) {
	    killdir($fp);
	    next;
	}
	if (-f $fp) {
	    killfile($fp);
	    next;
	}
    }
    opendir(TD, $d) or return 0;
    @c = readdir(TD);
    closedir(TD);
    if (@c > 2) {
	print "..remoção de $d falhou, ainda contem arquivos.\n";
	return 0;
    } else {
	if (rmdir($d)) {
	    print "..remoção de $d : sucesso\n";
	} else {
	    print "..remoção de $d : falhou\n";
	}
	return 1;
    }    
}

sub killfile {
    my ($f) = @_;
    my $c;

    $c = unlink($f);
    print "..removendo arquivo $f: ";
    if ($c==1) { print "(sucesso)\n"; } else { print "(falhou)\n"; }
    return $c;
}

sub human_size {
    my ($val) = @_;
    my @units = ("B","KB","MB","GB","TB","PB","Gazillions");
    my $c = 0;
    
    while($c<6 && $val > 5000) {
	$c++;
	$val /= 1024.0;
    }
    $val = int($val);
    return "$val $units[$c]";
}
