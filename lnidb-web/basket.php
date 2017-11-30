<?php
include('lnidb.inc');
startup();

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","back");
}

if (!isset($_GET['o']) && !isset($_POST['o'])) fatal_error("Parâmetros ausentes.","back");
if (isset($_GET['o']) && !isset($_GET['p']) && !isset($_GET['s']) && !isset($_GET['a']))
  fatal_error("Parâmetros ausentes.","back");
if (isset($_POST['o']) && !isset($_POST['sidtext']))
  fatal_error("Parâmetros ausentes.","back");

if (isset($_POST['o'])) {
  $o = $_POST['o'];
  $sidtext = $_POST['sidtext'];
} else {
  $o = $_GET['o'];
  $p = $s = $a = -1;
  if (isset($_GET['p'])) $p = $_GET['p'];
  if (isset($_GET['s'])) $s = $_GET['s'];
  if (isset($_GET['a'])) $a = $_GET['a'];
}
$uid = $_SESSION['userid'];

$before = scalar_query("select count(fid) from basket where uid=$uid;") +
  scalar_query("select count(aid) from basket2 where uid=$uid;");

// add files
if ($o==1) {
  if ($p > 0) basket_add_patient($p);
  if ($s > 0) basket_add_study($s);
  if ($a > 0) basket_add_attach($a);
}
// remove files
if ($o==2) {
  if ($p > 0) basket_del_patient($p);
  if ($s > 0) basket_del_study($s);
  if ($a > 0) basket_del_attach($a);
}

// remove all
if ($o==3) {
  basket_del_all();
}

// remove wrapped package
if ($o==4 && $s > 0) {
  $q = pg_query("insert into tasks (id,state,task,src,creator,started) values (nextval('task_id'),0,'remove-package','$s',$uid,'now');");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  notify_user("Remoção de pacote enfileirada.","back");
  exit(0);
}

// remove wrapped packages (all)
if ($o==4 && $s==0) {
  $q = pg_query("insert into tasks (id,state,task,src,creator,started) values (nextval('task_id'),0,'remove-package','all',$uid,'now');");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  notify_user("Remoção de pacotes enfileirada.","back");
  exit(0);
}

// add files from sid list
if ($o==5) {
  $lines = preg_split("/[\\r\\n]+/",$sidtext);
  for($i=0;$i<count($lines);$i++) {
    if (preg_match('/^\\#/',$lines[$i])) next;
    $sids = preg_split('/,/',$lines[$i]);
    for($j=0;$j<count($sids);$j++)
      if (preg_match('/^\\d+$/',$sids[$j]))
	basket_add_study($sids[$j]);
  }
}

$after = scalar_query("select count(fid) from basket where uid=$uid;") +
  scalar_query("select count(aid) from basket2 where uid=$uid;");

if ($after > $before) {
  $d = $after-$before;
  notify_user("Operação concluída: $d arquivo(s) adicionado(s) à cesta.","back");
} elseif ($after < $before) {
  $d = $before-$after;
  notify_user("Operação concluída: $d arquivo(s) removido(s) da cesta.","back");
} else {
  notify_user("Operação concluída: nenhuma alteração na cesta.","back");
}

function basket_add_patient($pid) {
  $q = pg_query("select s.id from studies s, patstudies ps where s.id=ps.sid and ps.pid=$pid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  $n = pg_num_rows($q);
  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q,$i);
    basket_add_study($row[0]);
  }
}

function basket_del_patient($pid) {
  $q = pg_query("select s.id from studies s, patstudies ps where s.id=ps.sid and ps.pid=$pid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  $n = pg_num_rows($q);
  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q,$i);
    basket_del_study($row[0]);
  }
}

function basket_add_study($sid) {
  global $uid;
  $q = pg_query("insert into basket (uid,fid) select $uid,fid from filestudies where sid=$sid except select uid,fid from basket where uid=$uid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
}

function basket_add_attach($aid) {
  global $uid;
  $q = pg_query("insert into basket2 (uid,aid) values ($uid,$aid);");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
}

function basket_del_study($sid) {
  global $uid;
  $q = pg_query("delete from basket using filestudies where basket.uid=$uid and basket.fid=filestudies.fid and filestudies.sid=$sid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
}

function basket_del_attach($aid) {
  global $uid;
  $q = pg_query("delete from basket2 where uid=$uid and aid=$aid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
}

function basket_del_all() {
  global $uid;
  $q = pg_query("delete from basket where uid=$uid; delete from basket2 where uid=$uid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
}

?>

