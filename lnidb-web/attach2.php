<?php
include('lnidb.inc');

startup();

if (!isset($_POST['pid']) || !isset($_POST['text']) || !isset($_POST['file'])) {
  fatal_error("Dados de formulário ausentes.","index.php");
}
if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","index.php");
}
if ($_SESSION['admlevel'] < 1) {
  fatal_error("Você não tem permissão para usar esta função.","index.php");
}

$base = "/home/lnidb/anexos";
$pid     = pg_escape_string($_POST['pid']);
$text    = pg_escape_string($_POST['text']);
$file    = pg_escape_string($_POST['file']);
$uid     = $_SESSION['userid'];

if (!is_file("$base/$file")) {
  fatal_error("Arquivo inválido ou não encontrado.","index.php");
}

$sql = "insert into tasks (id,state,task,src,options,creator,started) values (nextval('task_id'),0,'attach-file','$pid,$file','$text',$uid,'now');";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
$n = pg_affected_rows($q);
if ($n != 1) fatal_error("Erro inesperado ao agendar anexação: " . pg_last_error(),"back");
notify_user("Anexação agendada com sucesso.","back2");

cleanup();
?>

