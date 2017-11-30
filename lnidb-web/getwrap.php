<?php
include('lnidb.inc');
startup();

if (!isset($_SESSION['userid'])) fatal_error("Função disponível apenas para usuários logados.","back");
if (!isset($_GET['w'])) fatal_error("Parâmetros ausentes","back");

$tid = $_GET['w'];
$uid = $_SESSION['userid'];

$q = pg_query("select path, size from wrapped where owner=$uid and tid=$tid;");
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");

if (pg_num_rows($q) != 1) fatal_error("Pacote não encontrado no servidor.","back");

$row = pg_fetch_row($q,0);
if (!is_readable($row[0])) fatal_error("Arquivo ausente ou sem permissão de leitura no servidor.","back");

header("X-Sendfile: $row[0]");

if (preg_match('/\\.zip$/',$row[0]))
    header('Content-Type: application/zip');
elseif (preg_match('/\\.tar\\.gz$/',$row[0])) 
    header('Content-Type: application/x-gzip');
else
  header('Content-Type: application/octet-stream');

header('Content-Disposition: attachment; filename='.basename($row[0]));
set_time_limit(600);

?>
