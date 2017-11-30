<?php
include('lnidb.inc');
startup();

if ( (!isset($_POST['id'])) || (!isset($_POST['name'])) || (!isset($_POST['patcode'])) || (!isset($_POST['birth'])) || (!isset($_POST['gender'])) ) fatal_error("Parâmetros ausentes.","back");

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel']))
  fatal_error("Função não disponível para usuários deslogados.","back");

$uid   = $_SESSION['userid'];
$login = $_SESSION['username'];
$id        = $_POST['id'];
$name      = pg_escape_string($_POST['name']);
$patcode   = pg_escape_string($_POST['patcode']);
$birth     = pg_escape_string($_POST['birth']);
$gender    = $_POST['gender'];

if (preg_match('/^\\d{4}-\\d{2}-\\d{2}$/',$birth) == 0) fatal_error("Formato incorreto de data: $birth","back");
if (preg_match('/^([MFO])$/',$gender,$tmp) == 0) fatal_error("Formato incorreto para sexo: $gender","back");

$gender = $tmp[1];

$q = pg_query("select age(s.scantime,'$birth') from studies s, patstudies ps where s.id=ps.sid and ps.pid=$id limit 1;");
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");

$row = pg_fetch_row($q,0);
if (preg_match("/^(\\d+)\\s+(\\w)/",$row[0],$tmp) == 0) fatal_error("Erro inesperado ao computar idade.","back");

if ($tmp[2] == 'y')
  $age = $tmp[1];
else
  $age = 0;

$q = pg_query("select name,patcode,birth,gender from patients where id=$id;");
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
$old = pg_fetch_row($q,0);
$old[0] = pg_escape_string($old[0]);
$old[1] = pg_escape_string($old[1]);

$q = pg_query("select id from patients where id<>$id and name='$name' and patcode='$patcode' and birth='$birth' and age=$age and gender='$gender';");
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");

if (pg_num_rows($q) > 1) fatal_error("Inconsistência no banco de dados, múltiplos pacientes com mesmos dados.","back");

if (pg_num_rows($q) == 1) {
  $row = pg_fetch_row($q,0);
  $twin = $row[0];

  $sql =  "delete from patscanners ps1 using patscanners ps2 where ps1.sid=ps2.sid and ps1.pid=$twin and ps2.pid=$id; ";
  $sql .= "update patscanners set pid=$id where pid=$twin; ";
  $sql .= "update patstudies set pid=$id where pid=$twin; ";
  $sql .= "update patattachs set pid=$id where pid=$twin; ";
  $sql .= "update comments set pid=$id where pid=$twin; ";
  $sql .= "delete from patients where id=$twin; ";
  $sql .= "delete from patperiod where pid=$twin or pid=$id; ";
  $sql .= "update patients set name='$name',patcode='$patcode',birth='$birth',age=$age,gender='$gender' where id=$id;";
  $sql .= "insert into editlog (uid,edited,message) values ($uid,'now','Paciente editado id=$id ($old[0],$old[1],$old[2],$old[3]) => ($name,$patcode,$birth,$gender). Paciente $twin removido por fusão.');";
  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  notify_user("Alterações aplicadas, e outro registro foi unificado a este.","back2");
} else {
  $sql  = "update patients set name='$name',patcode='$patcode',birth='$birth',age=$age,gender='$gender' where id=$id; ";
  $sql .= "insert into editlog (uid,edited,message) values ($uid,'now','Paciente editado id=$id ($old[0],$old[1],$old[2],$old[3]) => ($name,$patcode,$birth,$gender).');";
  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  notify_user("Alterações aplicadas, e não houve unificação com outro registro.","back2");
}

cleanup();

?>