<?php
include('lnidb.inc');

startup();

if (!isset($_GET['o'])) fatal_error("Parâmetros ausentes.","back");

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","back");
}
$op  = $_GET['o'];
$uid = $_SESSION['userid'];
// 1: sharing 2: delete 3: new

// share/unshare
if ($op == 1) {
  if (!isset($_GET['i'])) fatal_error("Parâmetros ausentes.","back");
  $id = $_GET['i'];
  page_start("LNIDB - Visibilidade de Comentário");
  page_top(1,1);

  print "<div class=\"regular\"><h3>Visibilidade de Comentário</h3>";

  $q = pg_query("select text from comments where id=$id and owner=$uid and public=0;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  if (pg_num_rows($q)==0) fatal_error("Inconsistência na consulta ao banco de dados.", "back");

  $row = pg_fetch_row($q,0);
  print "<b>Comentário:</b> $row[0]<br><br>\n";

  $q = pg_query("select u.username from users u, cvisible v where v.cid=$id and v.uid=u.id order by username;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");

  $n = pg_num_rows($q);
  $ul = '';

  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q,$i);
    $ul .= "${row[0]},";
  }
  $ul = preg_replace('/,$/','.',$ul);
  
  print "<b>Usuários com visibilidade:</b> $ul ($n usuário(s)).<br><br>\n";
  print "<form action=\"patcom3.php\" method=\"post\">";
  print "<input type=hidden name=\"cid\" value=\"$id\">";
  print "<table border=0 cellspacing=4 cellpadding=4>\n";
  print "<tr><td>Dar visibilidade a:<br><small>(nomes de usuários separados por vírgulas)</small></td>";
  print "<td><input type=text size=30 name=\"add\"></td></tr>\n";
  print "<tr><td>Retirar visibilidade de:<br><small>(nomes de usuários separados por vírgulas)</small></td>";
  print "<td><input type=text size=30 name=\"del\"></td></tr>\n";
  print "<tr><td colspan=2 align=right><input type=submit value=\"Aplicar\">&nbsp;&nbsp;<input type=button value=\"Voltar\" onClick=\"javascript:history.go(-1)\"></td></tr></table><br><br>\n";
  print "<a href=\"patcom4.php?i=$id\">[Tornar Público]</a> (uma vez tornado público, um comentário não poderá mais ser privado)<br>";
  print "</div>";
}

// delete comment
if ($op == 2) {
  if (!isset($_GET['i'])) fatal_error("Parâmetros ausentes.","back");
  $id = $_GET['i'];
  $q = pg_query("select owner from comments where id=$id and owner=$uid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  if (pg_num_rows($q) == 0) fatal_error("Você não pode deletar comentários de outros usuários.","back");

  $q = pg_query("delete from cvisible where cid=$id; delete from comments where id=$id;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  if (pg_affected_rows($q) == 0) fatal_error("Resultado inesperado, remoção falhou.", "back");
  notify_user("Comentário removido.","back");
}

// new comment
if ($op == 3) {
  if (!isset($_GET['p'])) fatal_error("Parâmetros ausentes.","back");
  $pid = $_GET['p'];
  page_start("LNIDB - Novo Comentário em Paciente");
  page_top(1,1);
  
  print "<div class=\"regular\"><form method=\"post\" action=\"patcom2.php\">\n";
  print "<h3>Novo Comentário</h3>";

  $q = pg_query("select name, age, gender from patients where id=$pid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  $row = pg_fetch_row($q,0);
  print "Paciente: $row[0] ($row[1]/$row[2])<br>";

  print "<input type=hidden name=\"pid\" value=\"$pid\">\n";
  print "<table border=0 cellpadding=4 cellspacing=4>\n";
  print "<tr valign=top><td>Comentário:<br><textarea rows=4 cols=40 name=\"text\"></textarea></td></tr>\n";
  print "<tr valign=top><td align=right><input type=checkbox name=\"public\" checked=\"checked\"> Comentário Público</td></tr>\n";
  print "<tr><td align=right><input type=submit value=\"Adicionar\">&nbsp;&nbsp;<input type=button value=\"Cancelar\" onClick=\"javascript:history.go(-1)\"></td></tr></table></div>\n";
}

cleanup();

?>
