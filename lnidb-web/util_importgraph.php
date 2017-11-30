<?php
include('lnidb.inc');
include('svg.inc');

function svg_error() {
  text("Erro de banco de dados.",10,20,"Sans",12,0x000000,0,0);
  print "</svg>\n";
  exit(0);
}

function get_fcount($d1,$d2) {
  $now = date('Y-m-01');
  $n = scalar_query("select val from statcache where d1='$d1' and d2='$d2' and what=0;");
  //syslog(LOG_WARNING,"get_fcount $d1 $d2 now=$now n=$n");
  if ($n < 0 || $now == $d2 || $now == $d1) {
    $n = scalar_query("select count(id) from files where created >= '$d1' and created < '$d2';");
    //syslog(LOG_WARNING,"get_fcount failed n=$n");
    $q = pg_query("delete from statcache where d1='$d1' and d2='$d2' and what=0;");
    $q = pg_query("insert into statcache(d1,d2,what,val) values ('$d1','$d2',0,$n);");
  }
  return $n;
}

function get_bcount($d1,$d2) {
  $now = date('Y-m-01');
  $b = scalar_query("select val from statcache where d1='$d1' and d2='$d2' and what=1;");
  //syslog(LOG_WARNING,"get_bcount $d1 $d2 now=$now b=$b");
  if ($b < 0 || $now == $d2 || $now == $d1) {
    $b = scalar_query("select sum(size) from files where created >= '$d1' and created < '$d2';");
    if ($b == '') $b = 0;
    //syslog(LOG_WARNING,"get_bcount failed b=$b");
    $q = pg_query("delete from statcache where d1='$d1' and d2='$d2' and what=1;");
    $q = pg_query("insert into statcache(d1,d2,what,val) values ('$d1','$d2',1,$b);");
  }
  return $b;
}

startup();

header('Content-Type: image/svg+xml'); 
header("Content-Disposition: inline; filename=dugraph.svg");
print "<?xml version=\"1.0\" standalone=\"no\"?>\n";




?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg width="100%" height="100%" version="1.1" xmlns="http://www.w3.org/2000/svg">
<?php

$width = 700;

rect(0,0,$width,250,0xffffff,0x000000,1);
line(0,100,$width,100,0x00000,1);
line(0,125,$width,125,0x00000,1);
line(0,225,$width,225,0x00000,1);

$first  = scalar_query("select to_char(created,'YYYY-MM') from files order by created limit 1;");
$last   = scalar_query("select to_char(created,'YYYY-MM') from files order by created desc limit 1;");
$total  = scalar_query("select count(id) from files;");
$totalb = scalar_query("select sum(size) from files;");

if ($first < 0 || $last < 0 || $total < 0) svg_error();

$rtotal = $total;
$rtotalb = $totalb;
$total += $total / 10;
$totalb += $totalb / 10;

if (preg_match('/(\\d{4})-(\\d{2})/',$first,$tmp)) {
  $m1 = $tmp[2];
  $y1 = $tmp[1];
}
if (preg_match('/(\\d{4})-(\\d{2})/',$last,$tmp)) {
  $m2 = $tmp[2];
  $y2 = $tmp[1];
}

$m2++;
if ($m2==13) { $m2=1; $y2++; }

$ms = array();
$ys = array();
$h1 = array();
$h2 = array();

$m = $m1;
$y = $y1;

while(1) {
  array_push($ms,$m);
  array_push($ys,$y);
  if ($m == $m2 && $y == $y2) break;
  $m++;
  if ($m==13) { $m=1; $y++; }
}

$nmon = count($ys);
$x = 0;
$p = 0;
$y0 = 0;
$y1 = 0;
$x0 = 0;
$px = 0;
$pn = 0;
$prev = 0;
$prevb = 0;
$mn = array("NUL","Jan","Fev","Mar","Abr","Mai","Jun","Jul","Ago","Set","Out","Nov","Dez");

for($i=0;$i<$nmon;$i++) {

  $x = ($i * $width) / ($nmon - 1);
  $mx = $width / ($nmon - 1);

  line($x,1,$x,252,0x808080,1);
  if ($ms[$i] == 1 || $i==0)
    text("$ys[$i]",$x,285,'Sans',10,0x404040,1,0);
  vtext($mn[$ms[$i]],$x+2*$mx/3,270,'Sans',8,0x404040,0,0);


  if ($i>0) {
    $d1 = sprintf("%04d-%02d-01",$ys[$i-1],$ms[$i-1]);
    $d2 = sprintf("%04d-%02d-01",$ys[$i],$ms[$i]);
  } else {
    $d1 = '1900-01-01';
    $d2 = sprintf("%0.4d-%0.2d-01",$ys[$i],$ms[$i]);
  }

  $n = get_fcount($d1,$d2);
  $b = get_bcount($d1,$d2);

  array_push($h1, $n);
  array_push($h2, $b);

  $cur = $prev + $n;
  $curb = $prevb + $b;
  $prev = $cur;
  $prevb = $curb;
  
  $y = 100 - ((100 * $cur) / $total);
  marker($x,$y,0xff0000,0,10.0);
  if ($x != $x0)
    line($x0,$y0,$x,$y,0x880000,1);
  $y0 = $y;

  $y = 225 - ((100 * $curb) / $totalb);
  marker($x,$y,0x00ff00,0,10.0);
  if ($x != $x0)
    line($x0,$y1,$x,$y,0x008800,1);

  $x0 = $x;
  $y1 = $y;
}

text("$rtotal arquivos",$width+10,$y0,'Sans',8,0x000000,0,1);
$hsb = human_size($rtotalb);
text("$hsb",$width+10,$y1,'Sans',8,0x000000,0,1);

$mh1 = $h1[0];
$mh2 = $h2[0];
for($i=0;$i<count($h1);$i++) {
  if ($h1[$i] > $mh1) $mh1 = $h1[$i];
  if ($h2[$i] > $mh2) $mh2 = $h2[$i];
}

for($i=1;$i<count($h1);$i++) {
  $x  = (($i-1) * $width) / ($nmon - 1);
  $x2 = ((($i) * $width) / ($nmon - 1)) - 1;

  rect($x+1,100,$x2-$x-1,25,0xffffff,0,0);
  $y = 125 - ((25 * $h1[$i]) / $mh1);
  rect($x,$y,$x2-$x+1,125-$y,0xcc4444,0,0);

  $v = $h1[$i];
  if ($v > 0)
    $lv = log($v,10);
  else
    $lv = 0;
  if ($lv >= 9) { $v /= 1000000000; $v = round($v,0); $v = "${v}G"; }
  else if ($lv >= 6) { $v /= 1000000;    $v = round($v,0); $v = "${v}M"; }
  else if ($lv >= 4) { $v /= 1000;       $v = round($v,0); $v = "${v}K"; }
  else if ($lv >= 3) { $v /= 1000;       $v = round($v,1); $v = "${v}K"; }

  vtext("$v",($x+$x2)/2,120,'Sans',8,0x220000,0,0);

  rect($x,225,$x2-$x,25,0xffffff,0,0);
  $y = 250 - ((25 * $h2[$i]) / $mh2);
  rect($x,$y,$x2-$x+1,250-$y,0x448844,0,0);
  
  $hr = human_size($h2[$i]);
  vtext("$hr",($x+$x2)/2,245,'Sans',8,0x002200,0,0);
}

print "</svg>\n";

cleanup();
?>

