<?php
include('lnidb.inc');
startup();

if (!isset($_GET['i'])) fatal_error("Parâmetros ausentes.","back");

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","back");
}

$i   = $_GET['i'];
$uid = $_SESSION['userid'];

$q = pg_query("select id from comments where id=$i and public=0 and owner=$uid;");
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
if (pg_num_rows($q) != 1) fatal_error("Inconsistência na operação, cancelada.", "back");

$q = pg_query("delete from cvisible where cid=$i; update comments set public=1 where id=$i;");
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
if (pg_affected_rows($q) < 1) fatal_error("Resultado inesperado ao executar operação.", "back");

notify_user("O comentário foi tornado público.","back2");
cleanup();
?>

