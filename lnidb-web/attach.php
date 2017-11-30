<?php
include('lnidb.inc');

startup();

if (!isset($_GET['o'])) fatal_error("Parâmetros ausentes.","back");
$op = $_GET['o'];

if (!isset($_SESSION['userid']) || !isset($_SESSION['admlevel'])) {
  fatal_error("Função não disponível para usuários deslogados.","back");
}

// new attach, p=pat id
if ($op == 1) {
  if (!isset($_GET['p'])) fatal_error("Parâmetros ausentes.","back");
  $pid = $_GET['p'];

  $base = "/home/lnidb/anexos";
  $up = opendir($base);
  if (!$up) fatal_error("Erro ao acessar diretório de importação no servidor.");
  $items = array();
  while( ($item = readdir($up)) !== false ) {
    if (preg_match("/^[A-Za-z0-9_.-]/",$item) && is_file($base . '/' . $item)) {
      array_push($items,htmlentities($item));
    }
  }
  closedir($up);
  sort($items);

  page_start("LNIDB - Novo Anexo em Paciente");
  page_top(1,1);

  print "<div class=\"regular\"><form method=\"post\" action=\"attach2.php\">\n";
  print "<h3>Novo Anexo</h3>";

  $q = pg_query("select name, age, gender from patients where id=$pid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  $row = pg_fetch_row($q,0);
  print "Paciente: $row[0] ($row[1]/$row[2])<br>";

  print "<input type=hidden name=\"pid\" value=\"$pid\">\n";
  print "<table border=0 cellpadding=4 cellspacing=4>\n";
  print "<tr valign=top><td>Descrição:<br><textarea rows=4 cols=40 name=\"text\"></textarea></td></tr>\n";

  if (count($items) > 0) {
    print "<tr><td>Arquivo: <select name=\"file\">\n";

    foreach($items as $x) {
      print "<option value=\"$x\">$x</option>\n";
    }

    print "</select></td></tr>\n";

  } else {
    print "<tr><td width=500><font color=red><b>Não há arquivos no diretório de importação.</b></font><br>";
    print "Envie o arquivo por FTP, SCP ou SFTP para o diretório <b>/home/lnidb/anexos</b> com a conta <b>anexo</b> no servidor e clique no botão <i>Atualizar</i> abaixo. Os nomes de arquivos devem conter apenas letras, dígitos, sublinha, hífen ou ponto ([A-Za-z0-9_.-])</td></tr>\n";
  }

  print "<tr><td align=right>";
  if (count($items) > 0)
    print "<input type=submit value=\"Adicionar\">&nbsp;&nbsp;";
  print "<input type=button value=\"Atualizar\" onClick=\"javascript:history.go(0)\">&nbsp;&nbsp;<input type=button value=\"Cancelar\" onClick=\"javascript:history.go(-1)\"></td></tr></table></div>\n";

}

// delete attach, a=attach id
if ($op == 2) {
  if (!isset($_GET['a'])) fatal_error("Parâmetros ausentes.","back");
  $aid = $_GET['a'];
  $file = scalar_query("select path from attachments where id=$aid;");
  if (preg_match('/^.*\\/(.+)$/',$file,$tmp)) {
    $file = $tmp[1];
  }
  $name = scalar_query("select p.name from patients p, patattachs pa where p.id=pa.pid and pa.aid=$aid;");

  page_start("LNIDB - Remover Anexo (Confirmação)");
  page_top(1,1);

  ask_confirmation("Você realmente deseja remover o arquivo <b>$file</b> anexo ao paciente <b>$name</b> ?<br>A remoção será permanente e não poderá ser revertida.","attach.php?o=3&a=$aid","back");
}

// delete attach (confirmation given), a=attach id
if ($op == 3) {
  if (!isset($_GET['a'])) fatal_error("Parâmetros ausentes.","back");
  $aid = $_GET['a'];

  page_start("LNIDB - Remover Anexo");
  page_top(1,1);
  $uid = $_SESSION['userid'];

  $sql = "insert into tasks (id,state,task,src,options,creator,started) values (nextval('task_id'),0,'remove-attach','$aid','',$uid,'now');";
  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php?m=0");

  notify_user("Remoção de anexo enfileirada.<br>O estado da tarefa pode ser verificado em <b>Utilidades</b> &rarr; <b>Listar Tarefas</b>","back2");
}

cleanup();

?>
