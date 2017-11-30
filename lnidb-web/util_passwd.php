<?php
include('lnidb.inc');

startup();

if (!isset($_POST['oldpw']) || !isset($_POST['newpw1']) || !isset($_POST['newpw2'])) {
  fatal_error("Dados de formulário ausentes.","index.php");
}
if (!isset($_SESSION['userid'])) {
  fatal_error("Função não disponível para usuários deslogados.","index.php");
}

$id = $_SESSION['userid'];
$oldpw  = pg_escape_string($_POST['oldpw']);
$newpw1 = pg_escape_string($_POST['newpw1']);
$newpw2 = pg_escape_string($_POST['newpw2']);

$coldpw = pg_escape_string(crypt($_POST['oldpw'],'db'));
$cnewpw1 = pg_escape_string(crypt($_POST['newpw1'],'db'));

if ($newpw1 != $newpw2) {
  fatal_error("A nova senha e sua confirmação não conferem.","index.php?m=4&f=1");
}

if (strlen($newpw1) < 4) {
  fatal_error("A nova senha deve ter pelo menos 4 caracteres.","index.php?m=4&f=1");
}

$sql = "select id from users where id=$id and (password='$oldpw' or password='$coldpw');";
$q = pg_query($sql);
if ($q==FALSE) {
  fatal_error("Erro na consulta ao banco de dados: " . pg_last_error(),"index.php");
}
$n = pg_num_rows($q);
if ($n==0) {
  fatal_error("A senha atual está incorreta.","index.php?m=4&f=1");
}

$sql = "update users set password='$cnewpw1' where id=$id;";
$q = pg_query($sql);
$n = pg_affected_rows($q);

if ($n==0) {
  fatal_error("Erro inesperado na atualização do banco de dados: " . pg_last_error(),"index.php");
} else {
  notify_user("Senha alterada com sucesso.","index.php");
}

// never executed 
cleanup();
goto_index();
?>