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

$sql = "select id,task,src,options,creator from tasks where id=$qid;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
if (pg_num_rows($q) != 1) fatal_error("Tarefa inexistente.","back");
$row = pg_fetch_row($q,0);

if ($_SESSION['admlevel'] < 2 && $row[3] != $myid) 
  fatal_error("Você não tem permissão para remover esta tarefa.","back");

$sql = "delete from tasks where id=$qid and (state=0 or state=4);";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
$n = pg_affected_rows($q);
if ($n != 1) fatal_error("Erro inesperado ao cancelar tarefa: " . pg_last_error(),"back");
notify_user("Tarefa cancelada.","back");

cleanup();
?>
