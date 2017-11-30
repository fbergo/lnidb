<?php
include('lnidb.inc');

startup();

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","back");
}

if (!isset($_POST['uid']) || !isset($_POST['title']) || !isset($_POST['desc']) || !isset($_POST['public'])) {
  fatal_error("Parâmetros ausentes.","back");
}

$pid    = $_POST['pid'];
$title  = $_POST['title'];
$desc   = $_POST['desc'];
$public = $_POST['public'];
$uid    = $_SESSION['userid'];

$title = pg_escape_string(htmlentities($title,ENT_COMPAT,'UTF-8'));
$desc  = pg_escape_string(htmlentities($desc,ENT_COMPAT,'UTF-8'));

$x = scalar_query("select count(id) from groups where owner=$uid and title='$title';");
if ($x != 0) {
  fatal_error("Já existe outro grupo de estudos com o mesmo título, também criado por você.","back");
}

$sql = "insert into groups (id,title,description,public,owner,created) values (nextval('group_id'),'$title','$desc',$public,$uid,'now');";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
if (pg_affected_rows($q) == 0) fatal_error("Erro inesperado ao adicionar comentário.","back");

notify_user("Grupo de estudos adicionado.","back");
cleanup();

?>
