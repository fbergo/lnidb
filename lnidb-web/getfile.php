<?php
include('lnidb.inc');
startup();

if (!isset($_GET['s']) && !isset($_GET['f']) && !isset($_GET['a']))
  fatal_error("Parâmetros ausentes","back");

$sid = $fid = $aid = 0;
if (isset($_GET['s'])) $sid = $_GET['s'];
if (isset($_GET['f'])) $fid = $_GET['f'];
if (isset($_GET['a'])) $aid = $_GET['a'];

if ($sid > 0)
  $q = pg_query("select f.id, f.path, f.size from files f, filestudies fs where fs.sid=$sid and fs.fid=f.id;");
else if ($fid > 0)
  $q = pg_query("select f.id, f.path, f.size from files f where f.id=$fid;");
else
  $q = pg_query("select a.id, a.path, a.bytes from attachments a where a.id=$aid;");
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");

if (pg_num_rows($q) != 1) fatal_error("Inconsistência no resultado de consulta.","back");

$row = pg_fetch_row($q,0);
if (!is_readable($row[1])) fatal_error("Arquivo ausente ou sem permissão de leitura no servidor.","back");

if ($sid > 0 || $fid > 0)
  $q = pg_query("update files set dlcount = dlcount + 1, lastdl='now' where id=${row[0]};");
else
  $q = pg_query("update attachments set dlcount = dlcount + 1, lastdl='now' where id=${row[0]};");
cleanup();

header("X-Sendfile: $row[1]");
header('Content-Type: application/octet-stream');
header('Content-Disposition: attachment; filename='.basename($row[1]));
set_time_limit(600);

?>