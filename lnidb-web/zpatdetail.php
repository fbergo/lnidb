<?php
include('lnidb.inc');

$id = -1;
if (isset($_POST['id'])) { $id = $_POST['id']; }
$uid = -1; // todo

txtstartup();

$q = pg_query("select name, patcode, birth, age, gender from patients where id=$id;");
if ($q==FALSE) zzfatal("Falha na consulta ao banco de dados: " . pg_last_error());
$row = pg_fetch_row($q,0);

print "nome=$row[0]\n";
print "hc=$row[1]\n";
print "birth=$row[2]\n";
print "age=$row[3]\n";
print "gender=$row[4]\n";

// scanners
$q = pg_query("select s.maker, s.model, s.location from scanners s, patscanners ps where ps.pid=$id and ps.sid=s.id order by s.maker,s.model,s.location;");
if ($q==FALSE) zzfatal("Falha na consulta ao banco de dados: " . pg_last_error());
$n = pg_num_rows($q);
for($i=0;$i<$n;$i++) {
  $row = pg_fetch_row($q,$i);
  print "scanner=$row[0] $row[1] $row[2]\n";
}

// comentarios
    
$sql  = "select c.id, c.text, c.owner, date_trunc('second',c.created), c.public, u.username, u.fullname, c.created from comments c, users u where c.pid=$id and c.owner=u.id and (c.public=1 or c.owner=$uid) ";
$sql .= "union select c.id, c.text, c.owner, date_trunc('second',c.created), c.public, u.username, u.fullname, c.created from comments c, users u, cvisible v where c.pid=$id and c.owner=u.id and c.public=0 and c.owner<>$uid and v.cid=c.id and v.uid=$uid ";
$sql .= " order by 8;";
$q = pg_query($sql);
if ($q==FALSE) zzfatal("Falha na consulta ao banco de dados: " . pg_last_error());
$n = pg_num_rows($q);

for($i=0;$i<$n;$i++) {
  $row = pg_fetch_row($q,$i);
  if ($row[4] == 1)
    print "cpublic=1\n";
  else
    print "cpublic=0\n";

  print "cauthor=$row[6] ($row[5])\n";
  print "cdate=$row[3]\n";
  print "ctext=$row[1]\n";
}

// estudos
$sql = "select s.id, date_trunc('second',s.scantime),s.exam_desc,s.series_desc,s.cols,s.rows,s.slices,round((s.thickness)::numeric,2),round((s.xspacing)::numeric,2),round((s.yspacing)::numeric,2),round((s.zspacing)::numeric,2),s.files,s.storage,sc.maker,sc.model,sc.location,s.oaxi,s.osag,s.ocor,s.artifact from studies s,patstudies ps,scanners sc ";
$sql .= "where s.id=ps.sid and ps.pid=$id and s.scanner=sc.id order by s.scantime;";
$q = pg_query($sql);
if ($q==FALSE) zzfatal("Falha na consulta ao banco de dados: " . pg_last_error());
$n = pg_num_rows($q);

for($i=0;$i<$n;$i++) {
  $row = pg_fetch_row($q,$i);
  $oe = (($i % 2 == 0) ? 'even' : 'odd');

  print "sid=$row[0]\n";
  print "sdate=$row[1]\n";
  print "sexam=$row[2]\n";
  print "sseries=$row[3]\n";

  $osum = $row[16] + $row[17] +  $row[18];
  $ori = '';
  if ($row[16] != 0) { $ori .= "Ax/"; }
  if ($row[17] != 0) { $ori .= "Sag/"; }
  if ($row[18] != 0) { $ori .= "Cor/"; }      
  if ($osum > 0) {
    $ori = substr($ori,0,-1);
  } else {
    $ori = 'n/a';
  }

  print "sori=$ori\n";
  print "sdim=${row[4]}x${row[5]}x${row[6]}\n";
  print "spdim=${row[8]}x${row[9]}x${row[10]} mm\n";
  print "sthick=$row[7] mm\n";
  print "sfiles=$row[11]\n";
  print "sdisk=$row[12]\n";
  print "sscanner=$row[13] $row[14] $row[15]\n";
  print "squality=$row[19]\n";
  // inbasket: todo
}
  
// todo: anexos

cleanup();

?>

