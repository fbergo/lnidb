<?php
include('lnidb.inc');
startup();

if (!isset($_SESSION['userid'])) fatal_error("Função disponível apenas para usuários logados.","back");

if (!isset($_POST['artifact'])) fatal_error("Dados ausentes.","back");

$uid    = $_SESSION['userid'];
$admlevel = $_SESSION['admlevel'];

if ($admlevel < 2) fatal_error("Você não tem permissão para acessar esta função.","back");

$artifact = $_POST['artifact'];

if ($artifact < 0 || $artifact > 3) fatal_error("Dados inválidos no formulário.","back");

$sql = "update studies s set artifact=$artifact from basket b, filestudies fs where s.id=fs.sid and fs.fid=b.fid and b.uid=$uid;";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");

$n = pg_affected_rows($q);

if ($n > 0) {
  $sql = "insert into editlog (uid,edited,message) values ($uid, 'now', 'Marcação de qualidade ($artifact) aplicada a $n estudo(s).');";
  $q = pg_query($sql);
}

notify_user("Marcação realizada, $n estudo(s) afetado(s).","back");

cleanup();

?>
