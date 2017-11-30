<?php
include('lnidb.inc');

startup();

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","index.php");
}
if ($_SESSION['admlevel'] < 2) {
  fatal_error("Você não tem permissão para usar esta função.","index.php");
}

$uid = $_SESSION['userid'];
$sql = "insert into tasks (id,state,task,src,options,creator,started) values (nextval('task_id'),0,'purge-old-packages','','',$uid,'now');";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
$n = pg_affected_rows($q);
if ($n != 1) fatal_error("Erro inesperado ao agendar remoção: " . pg_last_error(),"back");
notify_user("Remoção agendada com sucesso.","back");

cleanup();
?>
