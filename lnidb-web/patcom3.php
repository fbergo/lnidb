<?php
include('lnidb.inc');
startup();

// modifica cvisible de um comentario
// a principio, valido para qq tipo de comentario

if ( (!isset($_POST['cid'])) || (!isset($_POST['add'])) || (!isset($_POST['del'])) ) fatal_error("Parâmetros ausentes.","back");

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","back");
}
$cid  = $_POST['cid'];
$add  = $_POST['add'];
$del  = $_POST['del'];
$uid = $_SESSION['userid'];

$n1=$n2=0;
$a1 = preg_split("/[\s,]+/",$add,-1,PREG_SPLIT_NO_EMPTY);
$a2 = preg_split("/[\s,]+/",$del,-1,PREG_SPLIT_NO_EMPTY);
if ($a1) $n1 = count($a1);
if ($a2) $n2 = count($a2);

$m1 = 0;
$m2 = 0;

$q = pg_query("select id from comments where id=$cid and public=0 and owner=$uid;");
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
if (pg_num_rows($q) != 1) fatal_error("Inconsistência na operação, cancelada.", "back");

for($i=0;$i<$n1;$i++) {
  $a1[$i] = pg_escape_string($a1[$i]);
  $q = pg_query("select id from users where username='${a1[$i]}';");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  if (pg_num_rows($q) != 1) continue;
  $row = pg_fetch_row($q,0);

  $q = pg_query("insert into cvisible (cid,uid) values ($cid, $row[0]);");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  if (pg_affected_rows($q) == 1) $m1++;
}

for($i=0;$i<$n2;$i++) {
  $a2[$i] = pg_escape_string($a2[$i]);
  $q = pg_query("select id from users where username='${a2[$i]}';");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  if (pg_num_rows($q) != 1) continue;
  $row = pg_fetch_row($q,0);

  $q = pg_query("delete from cvisible where cid=$cid and uid=$row[0];");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  if (pg_affected_rows($q) == 1) $m2++;
}

notify_user("Visibilidade adicionada para $m1 usuário(s) ($n1 solicitado(s)).<br>Visibilidade removida para $m2 usuário(s) ($n2 solicitado(s)).<br>","back");

cleanup();

?>
