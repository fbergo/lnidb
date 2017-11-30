<?php
include('lnidb.inc');

startup();

if (!isset($_POST['dir']) || !isset($_POST['storage'])) {
  fatal_error("Dados de formulário ausentes.","index.php");
}
if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","index.php");
}
if ($_SESSION['admlevel'] < 1) {
  fatal_error("Você não tem permissão para usar esta função.","index.php");
}

$dir     = pg_escape_string($_POST['dir']);
$storage = pg_escape_string($_POST['storage']);
$delay = '';
if (isset($_POST['delay'])) $delay = $_POST['delay'];
$uid     = $_SESSION['userid'];

$delay = ( ($delay == 'on') ? 1 : 0 );

$allowed = array('/^backup[0-9]{4}[a-z]{0,1}$/',
		 '/^pq[0-9]{2,4}[a-z]{0,2}$/',
		 '/^ucp[0-9]{3,4}[a-z]{0,2}$/',
		 '/^avulso[0-9]{3,4}[a-z]{0,1}$/',
		 '/^eltf[0-9]{3,4}[a-z]{0,2}$/',
		 '/^controles_[A-Z]$/',
		 '/^mm[0-9]{2,4}[a-z]{0,2}$/',
		 '/^sm[0-9]{2,4}[a-z]{0,2}$/',
		 '/^hl[0-9]{2,4}[a-z]{0,2}$/',
		 '/^xx/');
$ok = 0;
for($i=0;$i<count($allowed);$i++)
  if (preg_match($allowed[$i],$storage)) {
    $ok = 1;
    break;
  }

if ($ok == 0) {
  $msg = "O nome de identificação do disco está fora dos padrões autorizados.<br>Padrões autorizados:<br><br>";
  for($i=0;$i<count($allowed);$i++)
    $msg .= $allowed[$i] . "<br>";
  fatal_error($msg,"index.php");
}

if ($delay != 0) $state = 4; else $state = 0;

$sql = "insert into tasks (id,state,task,src,options,creator,started) values (nextval('task_id'),$state,'import-dicom','$dir','$storage',$uid,'now');";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
$n = pg_affected_rows($q);
if ($n != 1) fatal_error("Erro inesperado ao agendar importação: " . pg_last_error(),"back");
notify_user("Importação agendada com sucesso.","back");

cleanup();
?>

