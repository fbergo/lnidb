<?php
include('lnidb.inc');
startup();

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","back");
}

if (!isset($_POST['pid']) || !isset($_POST['text'])) {
  fatal_error("Parâmetros ausentes.","back");
}

if (!isset($_POST['public'])) $_POST['public'] = 'off';

$pid    = $_POST['pid'];
$text   = $_POST['text'];
$public = $_POST['public'];
$uid    = $_SESSION['userid'];

$public = ( ($public == 'on') ? 1 : 0 );

if (preg_match("/[A-Za-z0-9]+/",$text)==0) fatal_error("Comentário vazio não adicionado.","back");

$text = pg_escape_string(htmlentities($text,ENT_COMPAT,'UTF-8'));

$sql = "insert into comments (id,text,owner,public,created,pid) values (nextval('comment_id'),'$text',$uid,$public,'now',$pid);";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
if (pg_affected_rows($q) == 0) fatal_error("Erro inesperado ao adicionar comentário.","back");

notify_user("Comentário adicionado.","back2");

cleanup();

?>
