<?php
include('lnidb.inc');

startup();

if (!isset($_POST['username']) || !isset($_POST['fullname']) || !isset($_POST['password']) || !isset($_POST['admlevel'])) {
  fatal_error("Dados de formulário ausentes.","index.php");
}
if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","index.php");
}
if ($_SESSION['admlevel'] < 2) {
  fatal_error("Você não tem permissão para usar esta função.","index.php");
}

$username = pg_escape_string($_POST['username']);
$fullname = pg_escape_string($_POST['fullname']);
$admlevel = pg_escape_string($_POST['admlevel']);
$password = pg_escape_string($_POST['password']);
$cpassword = pg_escape_string(crypt($_POST['password'],'db'));

if ($admlevel < 0 || $admlevel > 2) {
  fatal_error("Nível administrativo inválido. Apenas 0, 1 e 2 são permitidos.","back");
}

if (strlen($username) < 2) {
  fatal_error("O nome do usuário deve ter pelo menos 2 caracteres.","back");
}

if (strlen($password) < 4) {
  fatal_error("A senha deve ter pelo menos 4 caracteres.","back");
}

if (htmlentities($username) != $username) {
  fatal_error("O nome do usuário não pode conter caracteres de formatação HTML.","back");
}

$sql = "select id from users where username = '$username';";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
$n = pg_num_rows($q);
if ($n > 0) fatal_error("Já existe outra conta com o nome de usuário informado.","back");

$sql = "insert into users (id,username,fullname,admlevel,password,created,lastlogin) values (nextval('user_id'), '$username', '$fullname', $admlevel, '$cpassword', 'now', 'now');";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back2");
$n = pg_affected_rows($q);
if ($n != 1) fatal_error("Erro inesperado ao adicionar usuário: " . pg_last_error(),"back2");
notify_user("Usuário adicionado com sucesso.","back2");

cleanup();
?>
