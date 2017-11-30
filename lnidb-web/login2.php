<?php
include('lnidb.inc');

startup();

if (!isset($_POST['username']) || !isset($_POST['password'])) {
  fatal_error("Dados de formulário ausentes.","index.php");
}

$username = pg_escape_string($_POST['username']);
$password = pg_escape_string($_POST['password']);
$cpassword = pg_escape_string(crypt($_POST['password'],'db'));


$sql = "select id, username, fullname, admlevel from users where username='$username' and (password='$password' or password='$cpassword') and id>=10 and length(password)>=4 and admlevel>=0;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Erro na consulta ao banco de dados: " . pg_last_error(),"index.php");
$n = pg_num_rows($q);
if ($n==0) {
  $ip = '0.0.0.0'; if (isset($_SERVER['REMOTE_ADDR']))     { $ip = $_SERVER['REMOTE_ADDR']; }
  $ua = 'unknown'; if (isset($_SERVER['HTTP_USER_AGENT'])) { $ua = $_SERVER['HTTP_USER_AGENT']; }
  $ip = pg_escape_string($ip);
  $ua = pg_escape_string($ua);
  pg_query("insert into loginlog (username,time,ipaddr,agent,success) values ('$username','now','$ip','$ua',FALSE);");
  fatal_error("Nome de usuário ou senha incorretos.","login.php");
}

$row = pg_fetch_row($q,0);
$_SESSION['userid'] = $row[0];
$_SESSION['username'] = $row[1];
$_SESSION['fullname'] = $row[2];
$_SESSION['admlevel'] = $row[3];

$sql = "update users set lastlogin='now' where id=$row[0];";
pg_query($sql);

$ip = '0.0.0.0'; if (isset($_SERVER['REMOTE_ADDR']))     { $ip = $_SERVER['REMOTE_ADDR']; }
$ua = 'unknown'; if (isset($_SERVER['HTTP_USER_AGENT'])) { $ua = $_SERVER['HTTP_USER_AGENT']; }
$ip = pg_escape_string($ip);
$ua = pg_escape_string($ua);
pg_query("insert into loginlog (username,time,ipaddr,agent,success) values ('$username','now','$ip','$ua',TRUE);");

cleanup();
goto_index();
?>
