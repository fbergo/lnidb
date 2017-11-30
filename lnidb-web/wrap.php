<?php
include('lnidb.inc');
startup();

if (!isset($_SESSION['userid'])) fatal_error("Função disponível apenas para usuários logados.","back");

if (!isset($_POST['title']) || !isset($_POST['format'])) fatal_error("Dados ausentes.","back");

$uid    = $_SESSION['userid'];
$title  = pg_escape_string($_POST['title']);
$format = $_POST['format'];

$niigz = $nii = $analyze = $minc = $scn = $scnme = $scndif = $scnneg = $nodicom = 0;
$mincaxi = $mincsag = $minccor = 0;
if (isset($_POST['niigz']))   $niigz   = checkbox_value($_POST['niigz']);
if (isset($_POST['nii']))     $nii     = checkbox_value($_POST['nii']);
if (isset($_POST['analyze'])) $analyze = checkbox_value($_POST['analyze']);
if (isset($_POST['minc']))    $minc    = checkbox_value($_POST['minc']);
if (isset($_POST['mincsag'])) $mincsag = checkbox_value($_POST['mincsag']);
if (isset($_POST['minccor'])) $minccor = checkbox_value($_POST['minccor']);
if (isset($_POST['mincaxi'])) $mincaxi = checkbox_value($_POST['mincaxi']);
if (isset($_POST['scn']))     $scn     = checkbox_value($_POST['scn']);
if (isset($_POST['scnme']))   $scnme   = checkbox_value($_POST['scnme']);
if (isset($_POST['scndif']))  $scndif  = checkbox_value($_POST['scndif']);
if (isset($_POST['nodicom'])) $nodicom = checkbox_value($_POST['nodicom']);
if (isset($_POST['scnneg']))  $scnneg  = $_POST['scnneg'];

if ($format != 'zip' && $format != 'targz') fatal_error("Formato inválido.","back");
if (!preg_match('/^[A-Za-z0-9_]+$/',$title)) fatal_error("O nome deve ser alfanumérico","back");

if ($nii)           $format .= ",nii";
if ($nii && $niigz) $format .= ",niigz";
if ($analyze)  $format .= ",hdr";
if ($minc)     $format .= ",mnc";
if ($mincsag)  $format .= ",mncsag";
if ($minccor)  $format .= ",mnccor";
if ($mincaxi)  $format .= ",mncaxi";
if ($minc && !($mincsag || $minccor || $mincaxi)) $format .= ",mncsag";
if ($scn)      $format .= ",scn";
if ($scn && $scnneg) $format .= ",scnneg=$scnneg";
if ($scn && $scnme)  $format .= ",scnme";
if ($scn && $scndif) $format .= ",scndif";
if ($nodicom)  $format .= ",nodicom";

$sql  = "insert into tasks (id,state,task,src,options,creator,started) values (nextval('task_id'),0,'wrap-basket','$title','$format',$uid,'now'); ";
$sql .= "insert into wraptmp (tid,fid) select currval('task_id'), b.fid from basket b where b.uid=$uid; ";
$sql .= "insert into wraptmp2 (tid,aid) select currval('task_id'), b.aid from basket2 b where b.uid=$uid; ";
$sql .= "delete from basket where uid=$uid; ";
$sql .= "delete from basket2 where uid=$uid; ";
$q = pg_query($sql);
if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php?m=0");

notify_user("Cesta enfileirada para empacotamento.<br>O estado da tarefa pode ser verificado em <b>Utilidades</b> &rarr; <b>Listar Tarefas</b>","back");

cleanup();
?>
