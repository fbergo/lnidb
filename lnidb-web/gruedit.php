<?php
include('lnidb.inc');
include('grupos.inc');

startup();

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","back");
}

$uid = $_SESSION['userid'];

if (isset($_POST['gid']) && isset($_POST['op'])) {
  $op  = $_POST['op'];
  $gid = $_POST['gid'];
} else if (isset($_GET['gid']) && isset($_GET['op'])) {
  $op  = $_GET['op'];
  $gid = $_GET['gid'];
} else
  fatal_error("Parâmetros ausentes.","back");

// grant visibility to an user (POST)
if ($op == 'grant') {
  if (!isset($_POST['tuid']))
    fatal_error("Parâmetros ausentes.","back");
  $v = scalar_query("select count(id) from groups where id=$gid and owner=$uid;");
  if ($v != 1)
    fatal_error("Apenas o criador do grupo de estudos pode realizar esta operação.","back");

  $tuid   = $_POST['tuid'];
  if (!isset($_POST['change'])) $change = 'off'; else $change = $_POST['change'];
  $change = ( ($change == 'on') ? 1 : 0 );
  
  $q = pg_query("delete from gvisible where gid=$gid and uid=$tuid; insert into gvisible (gid,uid,change) values ($gid,$tuid,$change);");
  if ($q==FALSE)
    fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
  else
    notify_user("Permissão Adicionada.","back");  
}

// revoke visibility from an user (POST)
if ($op == 'revoke') {
  if (!isset($_POST['tuid']))
    fatal_error("Parâmetros ausentes.","back");
  $v = scalar_query("select count(id) from groups where id=$gid and owner=$uid;");
  if ($v != 1)
    fatal_error("Apenas o criador do grupo de estudos pode realizar esta operação.","back");

  $tuid   = $_POST['tuid'];
  
  $q = pg_query("delete from gvisible where gid=$gid and uid=$tuid;");
  if ($q==FALSE)
    fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
  else
    notify_user("Permissão Removida.","back");  
}

// add studies from basket to group (GET)
if ($op == 'frombasket') {
  if (!grupos_can_change($uid,$gid))
    fatal_error("Você não tem permissão para modificar este grupo de estudos.","back");

  $sql = "insert into groupstudies (gid,sid) select distinct $gid, s.id from studies s, filestudies fs, basket b where s.id=fs.sid and fs.fid=b.fid and b.uid=$uid except select gid, sid from groupstudies where gid=$gid;";
  $q = pg_query($sql);
 if ($q==FALSE)
   fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
 else {
   $n = pg_affected_rows($q);
   notify_user("$n estudo(s) adicionado(s) ao grupo.","back");
 }

}

// copy studies from group to basket (GET)
if ($op == 'tobasket') {
  if (!grupos_can_view($uid,$gid))
    fatal_error("Você não tem permissão para acessar este grupo de estudos.","back");

  $sql = "insert into basket (uid,fid) select $uid, f.id from files f, filestudies fs, groupstudies gs where f.id=fs.fid and fs.sid=gs.sid and gs.gid=$gid except select uid,fid from basket where uid=$uid;";

  $q = pg_query($sql);
  if ($q==FALSE)
    fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
  else {
    $n = pg_affected_rows($q);
    notify_user("$n arquivo(s) adicionado(s) à cesta.","back");
  }
}

// remove study from group (GET)
if ($op == 'dropstudy') {
  if (!isset($_GET['sid']))
    fatal_error("Parâmetros ausentes.","back");
  if (!grupos_can_change($uid,$gid))
    fatal_error("Você não tem permissão para modificar este grupo de estudos.","back");
  $sid = $_GET['sid'];
  $q = pg_query("delete from groupstudies where gid=$gid and sid=$sid;");
  if ($q==FALSE)
    fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
  else
    notify_user("Estudo removido do grupo.","back");
}


cleanup();

?>