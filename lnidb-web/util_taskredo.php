<?php
include('lnidb.inc');

startup();

if (!isset($_GET['i'])) {
  fatal_error("Dados de formulário ausentes.","index.php");
}
if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","index.php");
}

$qid = $_GET['i'];
$myid = $_SESSION['userid'];

$sql = "select id,task,src,options from tasks where id=$qid and creator=$myid;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
if (pg_num_rows($q) != 1) fatal_error("Você não tem permissão para usar esta função.","back");

$row = pg_fetch_row($q,0);

$sql = "insert into tasks (id,state,message,task,src,options,creator,started) values ";
$sql .= "(nextval('task_id'),0,'','$row[1]','$row[2]','$row[3]',$myid,'now');";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
$n = pg_affected_rows($q);
if ($n != 1) fatal_error("Erro inesperado ao reagendar tarefa: " . pg_last_error(),"back");
notify_user("Tarefa reagendada.","back");

cleanup();
?>
