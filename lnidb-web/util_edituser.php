<?php
include('lnidb.inc');

startup();

if (!isset($_POST['username']) || !isset($_POST['fullname']) || !isset($_POST['password']) || !isset($_POST['admlevel']) || !isset($_POST['id']) ) {
  fatal_error("Dados de formulário ausentes.","index.php");
}
if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","index.php");
}
if ($_SESSION['admlevel'] < 2) {
  fatal_error("Você não tem permissão para usar esta função.","index.php");
}

$uid = $_SESSION['userid'];
$qid = pg_escape_string($_POST['id']);
$username = pg_escape_string($_POST['username']);
$fullname = pg_escape_string($_POST['fullname']);
$admlevel = pg_escape_string($_POST['admlevel']);
$password = pg_escape_string($_POST['password']);
$cpassword = pg_escape_string(crypt($_POST['password'],'db'));

if ($qid == $_SESSION['userid'] && $admlevel < $_SESSION['admlevel']) {
  fatal_error("Você não pode rebaixar seu próprio nível administrativo.","back");
}

if ( ($admlevel < 0 || $admlevel > 2) && ($admlevel != -2) ) {
  fatal_error("Nível administrativo inválido. Apenas -2, 0, 1 e 2 são permitidos.","back");
}

if (strlen($username) < 2) {
  fatal_error("O nome do usuário deve ter pelo menos 2 caracteres.","back");
}

if (htmlentities($username) != $username) {
  fatal_error("O nome do usuário não pode conter caracteres de formatação HTML.","back");
}

$sql = "select id from users where id <> $qid and username = '$username';";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
$n = pg_num_rows($q);
if ($n > 0) fatal_error("Já existe outra conta com o nome de usuário informado.","back");

$sql = "update users set username='$username', admlevel=$admlevel, fullname='$fullname' where id=$qid;";
if (strlen($password) > 0)
  $sql .= "update users set password='$cpassword' where id=$qid;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back2");
$n = pg_affected_rows($q);
if ($n < 1) fatal_error("Erro inesperado ao aplicar as modificações: " . pg_last_error(),"back2");
notify_user("Alterações realizadas com sucesso.","back2");

cleanup();
?>
