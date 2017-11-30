<?php
include('lnidb.inc');

$parnames = array('patient', 'patcode', 'age', 'gender', 'scanner', 'comments','attach','page','sort');
$param = array();
$param['page'] = "1";
$param['sort'] = "1";
foreach($parnames as $x) {
  if (isset($_POST[$x])) { $param[$x] = pg_escape_string($_POST[$x]); }
}
foreach($parnames as $x) {
  if (!isset($param[$x])) { $param[$x] = ''; }
}
$uid = -1; // todo

txtstartup();

// build query
// TODO: fazer union na busca por comentarios para pegar comentarios privados compartilhados
$sql1 = "select distinct p.id,p.name,p.patcode,p.age,p.gender from patients p";
if (strlen($param['scanner']) > 0)  $sql1 .= ",patscanners ps, scanners sc";
if (strlen($param['comments']) > 0) $sql1 .= ",comments c";
if (strlen($param['attach']) > 0)   $sql1 .= ",attachments a, patattachs pa";

$sql2 = "select count(distinct p.id) from patients p";
if (strlen($param['scanner']) > 0)  $sql2 .= ",patscanners ps, scanners sc";
if (strlen($param['comments']) > 0) $sql2 .= ",comments c";
if (strlen($param['attach']) > 0)   $sql2 .= ",attachments a, patattachs pa";

$sqlr = " where ";
if (strlen($param['scanner']) > 0)  $sqlr .= "ps.pid=p.id and ps.sid=sc.id and ";
if (strlen($param['comments']) > 0) $sqlr .= "c.pid=p.id and (c.public=1 or c.owner=$uid) and ";
if (strlen($param['attach']) > 0)   $sqlr .= "p.id=pa.pid and a.id=pa.aid and ";

$nc = 0;
$hrb = '';
if (strlen($param['patient'])>0) {
  $sqlr .= "p.name ~* '.*${param['patient']}.*'";
  $nc++; $hrb .= "Nome(${param['patient']}),";
}
if (strlen($param['patcode'])>0) {
  if ($nc>0) $sqlr .= " and ";
  $sqlr .= "p.patcode ~* '.*${param['patcode']}.*'";
  $nc++; $hrb .= "HC(${param['patcode']}),";
}
if (strlen($param['age'])>0) {
  if (preg_match("/([0-9]+)-([0-9]+)/",$param['age'],$rage)) {
    if ($nc>0) $sqlr .= " and ";
    $sqlr .= "p.age >= $rage[1] and p.age <= $rage[2]";
    $hrb .= "Idade(${rage[1]}-$rage[2]),";
    $nc++;
  } elseif (preg_match("/([0-9]+)/",$param['age'],$rage)) {
    if ($nc>0) $sqlr .= " and ";
    $sqlr .= "p.age = $rage[1]";
    $hrb .= "Idade(${rage[1]}),";
    $nc++;
  }
}
switch($param['gender']) {
case 1: if ($nc>0) $sqlr .= " and "; $sqlr .= "(p.gender = 'M' or p.gender = 'F')"; $nc++;
  $hrb .= "Sexo(MF),";
  break;
case 2: if ($nc>0) $sqlr .= " and "; $sqlr .= "p.gender = 'M'"; $nc++;
  $hrb .= "Sexo(M),";
  break;
case 3: if ($nc>0) $sqlr .= " and "; $sqlr .= "p.gender = 'F'"; $nc++;
  $hrb .= "Sexo(F),";
  break;
case 4: if ($nc>0) $sqlr .= " and "; $sqlr .= "p.gender = 'O'"; $nc++;
  $hrb .= "Sexo(O),";
  break;
}
if (strlen($param['scanner'])>0) { if ($nc>0) $sqlr .= " and ";
  $sqlr .= "(sc.maker || ' ' || sc.model || ' ' || sc.location) ~* '.*${param['scanner']}.*'";
  $nc++;
  $hrb .= "Scanner(${param['scanner']}),";
}
if (strlen($param['comments'])>0)  { if ($nc>0) $sqlr .= " and ";
  $rcom = preg_split("/,/",$param['comments']);
  for($i=0;$i<count($rcom);$i++) {
    $nc++;
    if ($i > 0) $sqlr .= " and ";
    $sqlr .= "c.text ~* '.*${rcom[$i]}.*'";
  }
  $hrb .= "Comentários(${param['comments']}),";
}
// search attachment descriptions
if (strlen($param['attach'])>0)  { if ($nc>0) $sqlr .= " and ";
  $ratt = preg_split("/,/",$param['attach']);
  for($i=0;$i<count($ratt);$i++) {
    $nc++;
    if ($i > 0) $sqlr .= " and ";
    $sqlr .= "a.description ~* '.*${ratt[$i]}.*'";
  }
  $hrb .= "Anexos(${param['attach']}),";
}

if ($nc == 0) { $sqlr = ' '; $hrb = "Sem restrições"; }

$hrb = preg_replace("/,$/","",$hrb);

// calc paging
$q = pg_query($sql2 . $sqlr . ";");
if ($q==FALSE) zzfatal("Falha na consulta ao banco de dados: " . pg_last_error());
$total = pg_fetch_result($q,0,0);
$offset = ($param['page'] - 1) * 200;
$first  = 1 + $offset;
$last   = $first + 199;
if ($last > $total) $last = $total;
$npages = floor($total/200);
if ($total % 200 != 0) $npages++;
if ($total == 0) $first = 0;

// sorting
$sp = '';
if ($param['sort'] == 2) { $sp = 'p.patcode,'; }
if ($param['sort'] == 3) { $sp = 'p.gender,p.age,'; }

// print: no limit/offset
$sql3 = " order by ${sp} p.name limit 200 offset $offset;";

$ostr = array('','Nome do paciente','HC','Sexo/Idade');
$op = $ostr[ $param['sort'] % 4 ];

// main list
print "querydesc=$hrb\n";
print "sortby=$op\n";
print "npages=$npages\n";
print "ntotal=$total\n";
print "page=${param['page']}\n";

$q = pg_query($sql1 . $sqlr . $sql3);
if ($q==FALSE) zzfatal("Falha na consulta ao banco de dados: " . pg_last_error());
$n = pg_num_rows($q);

for($i=0;$i<$n;$i++) {
  $nr = $i + 1 + $offset;
  $row = pg_fetch_row($q,$i);

  print "id=$row[0]\n";
  print "seq=$nr\n";
  print "nome=$row[1]\n";
  print "hc=$row[2]\n";
  print "age=$row[3]\n";
  print "gender=$row[4]\n";

  // scanners
  $q2 = pg_query("select s.id, s.maker, s.model, s.location from scanners s, patscanners ps where ps.pid=$row[0] and ps.sid=s.id order by s.maker,s.model,s.location;");
  if ($q2==FALSE) zzfatal("Falha na consulta ao banco de dados: " . pg_last_error());
  $n2 = pg_num_rows($q2);

  for($j=0;$j<$n2;$j++) {
    $row2 = pg_fetch_row($q2,$j);
    print "scanner=$row2[1] $row2[2] $row2[3]\n";
  }

  // comments
  $sql = "select c.text, c.created from comments c where c.pid=$row[0] and (c.public=1 or c.owner=$uid) union ";
  $sql .= "select c.text, c.created from comments c, cvisible v where c.pid=$row[0] and c.public=0 and c.owner<>$uid and v.cid=c.id and v.uid=$uid ";
  $sql .= "order by 2;";

  $q2 = pg_query($sql);
  if ($q2==FALSE) zzfatal("Falha na consulta ao banco de dados: " . pg_last_error());
  $n2 = pg_num_rows($q2);

  print "comments=";
  for($j=0;$j<$n2;$j++) {
    $row2 = pg_fetch_row($q2,$j);
    print "$row2[0]; ";
  }
  print "\n";

  // attachs
  $na = scalar_query("select count(aid) from patattachs where pid=$row[0];");
  print "attachs=$na\n";
}

cleanup();

?>
