<?php
include('lnidb.inc');

startup();

if (!isset($_GET['i'])) {
  fatal_error("Dados de formulário ausentes.","index.php");
}
if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","index.php");
}
if ($_SESSION['admlevel'] < 2) {
  fatal_error("Você não tem permissão para usar esta função.","index.php");
}

$uid = $_SESSION['userid'];
$qid = $_GET['i'];

// reference errors

$sql = "select id from tasks where creator=$qid and state>0;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados.","back2");
if (pg_num_rows($q) > 0) fatal_error("Usuários que realizaram tarefas não podem ser removidos.");

$sql = "select id from files where creator=$qid;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados.","back2");
if (pg_num_rows($q) > 0) fatal_error("Usuários que importaram arquivos não podem ser removidos.");

$sql = "select id from attachments where creator=$qid;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados.","back2");
if (pg_num_rows($q) > 0) fatal_error("Usuários que importaram arquivos não podem ser removidos.");

$sql = "select id from comments where owner=$qid;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados.","back2");
if (pg_num_rows($q) > 0) fatal_error("Usuários que fizeram comentários não podem ser removidos.");

$sql = "select uid from editlog where uid=$qid;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados.","back2");
if (pg_num_rows($q) > 0) fatal_error("Usuários que fizeram edições não podem ser removidos.");

// delete pending tasks from this user
$sql = "delete from tasks where creator=$qid;";
$q = pg_query($sql);

// turn any wrapped packages from the deleted user to the admin doing the removal
// (should not be possible as it would require a finished task from this user)
$sql = "update wrapped set owner=$uid where owner=$qid;";
$q = pg_query($sql);

// delete the user
$sql = "delete from users where id=$qid;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados.","back2");
$n = pg_affected_rows($q);
if ($n != 1) fatal_error("Erro inesperado ao remover usuário.","back2");
notify_user("Usuário removido com sucesso.","back2");

cleanup();
?>
