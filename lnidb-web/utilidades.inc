<?php

function left_menu_item($text, $itemid, $selid, $lastsubitem=0) {
  if ($lastsubitem == 0) $lastsubitem = $itemid;
  $x = (($selid >= $itemid && $selid <= $lastsubitem) ?  1 : 0);
  print "<div class=\"lefti${x}\"><a class=\"top\" href=\"index.php?f=$itemid\">$text</a></div>\n";
}

function view_selector_item($text, $base, $cssclass, $varname, $curval, $itemval) {
  $x = ($curval == $itemval) ? 1 : 0;
  print "<td class=\"${cssclass}${x}\"><a class=\"top\" href=\"${base}&${varname}=$itemval\">$text</a></td>\n";
}

function util_histogram($arr, $w, $h) {
  $max = 1;
  for($i=0;$i<count($arr);$i++)
    if ($arr[$i] > $max) $max = $arr[$i];
  print "<div class=\"histogram\">";
  for($i=0;$i<count($arr);$i++) {
    $y = ($h * $arr[$i]) / $max;
    print "<img width=\"$w\" height=\"$y\" src=\"red.png\">";
  }
  print "</div>\n";
}

function util_main() {
  global $_SESSION, $_GET;
  if (!isset($_SESSION['admlevel']) || !isset($_SESSION['userid']))
    fatal_error("Acesso restrito a usuários logados.","index.php");

  $link = menu_link('uti');

  $admlevel = $_SESSION['admlevel'];
  $f=0;
  if (isset($_GET['f']))
    $f = $_GET['f'];

  print "<div class=\"fixleft1\">\n";
  print "<h2>Utilidades</h2>\n";
  left_menu_item("Alterar Senha", 1, $f);
  left_menu_item("Listar Tarefas", 2, $f);

  if ($admlevel >= 2) {
    left_menu_item("Administrar Usuários", 21, $f, 25);
    left_menu_item("Log de Edições", 26, $f);
    left_menu_item("Histórico de Logins", 28, $f);
    left_menu_item("Diagnóstico", 27, $f);
  }

  if ($admlevel >= 1) {
    left_menu_item("Importar Dados", 11, $f, 12);
  }

  print "</div>\n";

  if ($f>=10 && $admlevel<1) fatal_error("Você não tem permissão para acessar esta função.","index.php");
  if ($f>=20 && $admlevel<2) fatal_error("Você não tem permissão para acessar esta função.","index.php");

  // change password
  if ($f==1) {
    print "<div class=\"regular2\"><form name=\"pwdform\" action=\"util_passwd.php\" method=\"post\">\n";
    print "<table border=0 cellspacing=4 cellpadding=4>\n";
    print "<tr><td>Senha Atual</td><td><input type=password name=\"oldpw\" size=20</td></tr>\n";
    print "<tr><td>Senha Nova</td><td><input type=password name=\"newpw1\" size=20</td></tr>\n";
    print "<tr><td>Senha Nova (confirmar)</td><td><input type=password name=\"newpw2\" size=20</td></tr>\n";
    print "<tr><td></td><td align=right><input type=submit value=\"Enviar\"></td></tr></table></form></div>\n";
  }

  // list tasks
  if ($f==2) {
    print "<div class=\"regular2\"><h3>Lista de Tarefas</h3>\n";
    $w = 0; $p = 1; $t = 'Todos';
    if (isset($_GET['w'])) $w = $_GET['w'];
    if (isset($_GET['p'])) $p = $_GET['p'];
    if (isset($_GET['t'])) $t = $_GET['t'];
    
    print "<table border=0 cellpadding=0 cellspacing=0><tr>\n";
    print "<td width=50>Estado:</td>";
    view_selector_item("Todas","$link&f=2&t=$t","sel","w",$w,0);
    view_selector_item("Não Terminadas","$link&f=2&t=$t","sel","w",$w,1);
    view_selector_item("Terminadas","$link&f=2&t=$t","sel","w",$w,2);
    view_selector_item("Bem-Sucedidas","$link&f=2&t=$t","sel","w",$w,3);
    view_selector_item("Mal-Sucedidas","$link&f=2&t=$t","sel","w",$w,4);
    print "</tr></table><br>\n";

    print "<table border=0 cellpadding=0 cellspacing=0><tr>\n";
    print "<td width=50>Tipo:</td>";
    $sql = "select distinct task from tasks order by 1;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $n = pg_num_rows($q);
    $types = array();
    for($i=0;$i<$n;$i++) {
      array_push($types,pg_fetch_result($q,$i,0));
    }

    view_selector_item("Todos","$link&f=2&w=$w","selt","t",$t,'Todos');
    for($i=0;$i<count($types);$i++) {
      view_selector_item($types[$i],"$link&f=2&w=$w","selt","t",$t,$types[$i]);
    }

    print "</tr></table>\n";

    if ($w == 0) $r = 't.state >= 0';
    if ($w == 1) $r = '(t.state = 0 or t.state = 4)';
    if ($w == 2) $r = 't.state >= 2';
    if ($w == 3) $r = 't.state = 2';
    if ($w == 4) $r = 't.state = 3';

    if ($t != 'Todos') $r = "$r and t.task = '$t'";

    $sql = "select count(t.id) from tasks t where $r;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $n = pg_fetch_result($q,0,0);
    $npages = (int) ceil($n / 200);
    page_selector($p,$npages,$n,"$link&f=2&w=$w&t=$t");

    $offset = ($p-1) * 200;    
    $sql = "select t.id, t.state, t.message, t.task, t.src, t.options, date_trunc('second',started), date_trunc('second',t.ended), u.username, u.fullname, t.creator from tasks t, users u where $r and t.creator=u.id order by t.started desc limit 200 offset $offset;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $n = pg_num_rows($q);
    if ($n > 0) {
      print "<table border=0 cellspacing=1 cellpadding=4><tr class=\"title\"><td>Situação</td><td>Início</td><td>Fim</td><td>Tarefa</td><td>Usuário</td><td>Mensagem</td></tr>\n";
      
      $estados = array("Pendente","Executando","Completa (Sucesso)","Completa (Falha)","Esperando (Noturno)");
      for($i=0;$i<$n;$i++) {
	$row = pg_fetch_row($q,$i);
	$x = "task$row[1]";
	print "<tr class=\"$x\"><td>${estados[$row[1]]}<br>";

	if ($row[10] == $_SESSION['userid'] && $row[1]==3) {
	  print "<a href=\"util_taskredo.php?i=$row[0]\"><img border=0 title=\"Tentar novamente\" src=\"bredo16.png\"></a>";
	}
	if (($row[10] == $_SESSION['userid'] || $_SESSION['admlevel']>=2) && ($row[1]==0 || $row[1]==4)) {
	  print "<a href=\"util_taskdel.php?i=$row[0]\"><img border=0 title=\"Cancelar\" src=\"bdel16.png\"></a>";
	}

	print "</td><td>$row[6]</td><td>$row[7]</td>";
	print "<td>${row[3]}($row[4],$row[5])</td><td>$row[9] ($row[8])</td><td>$row[2]</td></tr>\n";
      }

      print "</table>\n";
      page_selector($p,$npages,$n,"$link&f=2&w=$w&t=$t");
    }
    print "</div>\n";
  }

  // import data
  if ($f==11) {
    $base = "/home/lnidb/importacao";
    $up = opendir($base);
    if (!$up) fatal_error("Erro ao acessar diretório de importação no servidor.");
    $items = array();
    while( ($item = readdir($up)) !== false ) {
      if (preg_match("/^[A-Za-z0-9_]/",$item) && is_dir($base . '/' . $item)) {
	array_push($items,htmlentities($item));
      }
    }
    closedir($up);
    sort($items);

    print "<div class=\"regular2\"><h3>Importação DICOM</h3>\n";

//    print "<div class=\"warning\">Estagiários: favor não importar nenhum disco até segunda, 9/5/2011, ou este aviso ter sido removido. Estou realizando uma atualização longa no banco de dados e é preferível não importar nenhum estudo até que ela termine.<br><br> -- Felipe Bergo, 5/5/2011.</div>\n";

    print "<a href=\"$link&f=12\">Instruções</a><br><br>\n";

    if (count($items)>0) {
      print "<form name=\"impform\" action=\"util_import.php\" method=\"post\">\n";
      print "<table border=0 cellspacing=4 cellpadding=4>\n";
      print "<tr><td>Diretório:</td><td><select name=\"dir\">\n";
    
      foreach($items as $x) {
	print "<option value=\"$x\">$x</option>\n";
      }
      print "</select></td></tr>\n";
      print "<tr><td>Identificação do disco/storage:</td><td><input type=text size=30 name=\"storage\"></td></tr>\n";
      print "<tr><td>Adiar importação até o período noturno ?</td><td><input type=checkbox name=\"delay\"></td></tr>\n";

      print "<tr><td></td><td align=right><input type=submit value=\"Importar\"></td></tr></table></form></div>\n";
    } else {
      print "<b><font color=red>Não há diretórios disponíveis para importação.</font></b> <a href=\"$link&f=11\">Atualizar</a><br></div>\n";
    }
  }

  if ($f==12) {
    print "<div class=\"regular2\"><h3>Instruções de Importação DICOM</h3>\n";
    print "1. Conecte por FTP no servidor <b>araponga</b> (143.106.129.42) usando login=<b>lnidb</b> e senha=<b>lnidb</b>.<br><br>\n";
    print "2. Entre no diretório <b>importacao</b> e crie um novo diretório lá dentro. O nome deve ser único e não deve conter nada além de letras ou dígitos.<br><br>\n";
    print "3. Envie os arquivos (conteúdo de CD/DVD DICOM) para o diretório criado. Se houver subdiretórios, não há problema.<br><br>\n";
    print "4. O diretório estará disponível para importação na opção \"Importar Dados\" à esquerda. Aguarde o término da cópia dos arquivos (FTP) para solicitar a importação.<br>\n";
    print "<br><a href=\"$link&f=11\">Voltar</a><br></div>\n";
  }

  // user adm - main listing
  if ($f==21) {
    print "<div class=\"regular2\">\n";
    print "<a href=\"$link&f=22\">Novo Usuário</a><br><br>\n";
    $sql = "select id, username, fullname, admlevel, to_char(lastlogin, 'YYYY-MM-DD HH24:MI') from users where id >= 10 order by admlevel desc, username asc;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
    $n = pg_num_rows($q);
    print "<table border=0 cellspacing=1 cellpadding=4>\n";
    print "<tr class=\"title\"><td>Usuário</td><td>Nome</td><td>Nível</td><td>Último Login</td><td>Operações</td></tr>\n";
    for($i=0;$i<$n;$i++) {
      $row = pg_fetch_row($q, $i);
      print "<tr class=\"" . (($i % 2) ? "uodd" : "ueven") . $row[3] . "\">";
      print "<td>$row[1]</td><td>$row[2]</td><td align=right>$row[3]</td><td class=\"plain\">$row[4]</td><td>\n";
      img_link("bdetail16.png","Mostrar Detalhes","$link&f=23&i=$row[0]");
      img_link("bedit16.png","Editar","$link&f=24&i=$row[0]");
      if ($row[0] != $_SESSION['userid'])
	img_link("bdel16.png","Remover","$link&f=25&i=$row[0]");
      print "</td></tr>\n";
    }
    print "</table><p><i>Níveis/permissões:</i><ul>";
    print "<font color=\"#006600\"><b>2: administrador</b></font><br>";
    print "<font color=\"#0000aa\"><b>1: pode importar dados/diretórios</b></font><br>";
    print "0: usuário regular<br>";
    print "<font color=\"#ff0000\"><i>-2: usuário desativado</i></font><br>";
    print "</ul><b>Total:</b> $n usuário(s).</div>\n";
  }

  // new user
  if ($f==22) {
    print "<div class=\"regular2\"<h2>Novo Usuário</h2>\n";
    print "<form action=\"util_adduser.php\" method=\"post\"><table border=0 cellspacing=2 cellpadding=2>\n";
    print "<tr><td>Usuário:</td><td><input type=text size=\"20\" name=\"username\"></td></tr>\n";
    print "<tr><td>Nome:</td><td><input type=text size=\"40\" name=\"fullname\"></td></tr>\n";
    print "<tr><td>Nível:</td><td><input type=text size=\"5\" name=\"admlevel\" value=\"0\"> <small><i>(0=regular, 1=permite importação de dados, 2=administrador)</i></small></td></tr>\n";
    print "<tr><td>Senha:</td><td><input type=password size=\"20\" name=\"password\"></td></tr>\n";
    print "<tr><td></td><td align=right><input type=submit value=\"Criar\">&nbsp;<input type=button value=\"Cancelar\" onClick=\"history.go(-1)\"></td></tr></form></div>\n";
  }

  // user details
  if ($f==23) {
    if (!isset($_GET['i'])) fatal_error("Requisição incorreta.","back");
    $qid = pg_escape_string($_GET['i']);
    $sql = "select username, fullname, admlevel, date_trunc('second',lastlogin), date_trunc('second',created) from users where id=$qid;";
    $q = pg_query($sql);
    if ($q == FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
    if (pg_num_rows($q) < 1) fatal_error("Usuário não encontrado.","back");
    $row = pg_fetch_row($q,0);
    print "<div class=\"regular2\"><h2>Detalhes de Usuário</h2>\n";
    print "Usuário: <b>$row[0]</b><br>Nome: <b>$row[1]</b><br>Nível: </b>$row[2]</b><br>\n";
    print "Criado em: <b>$row[4]</b><br>Último login: <b>$row[3]</b><br><br>\n";
    print "<a href=\"$link&f=24&i=$qid\">Editar</a><br>";
    print "<a href=\"javascript:history.go(-1);\">Voltar</a></div>\n";
  }

  // edit user
  if ($f==24) {
    if (!isset($_GET['i'])) fatal_error("Requisição incorreta.","back");
    $qid = pg_escape_string($_GET['i']);
    $sql = "select username, fullname, admlevel from users where id=$qid;";
    $q = pg_query($sql);
    if ($q == FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
    if (pg_num_rows($q) < 1) fatal_error("Usuário não encontrado.","back");
    $row = pg_fetch_row($q,0);
    print "<div class=\"regular2\"<h2>Editar Usuário</h2>\n";
    print "<form action=\"util_edituser.php\" method=\"post\"><table border=0 cellspacing=2 cellpadding=2>\n";
    print "<input type=hidden name=\"id\" value=\"$qid\">\n";
    print "<tr><td>Usuário:</td><td><input type=text size=\"20\" name=\"username\" value=\"$row[0]\"></td></tr>\n";
    print "<tr><td>Nome:</td><td><input type=text size=\"40\" name=\"fullname\" value=\"$row[1]\"></td></tr>\n";
    print "<tr><td>Nível:</td><td><input type=text size=\"5\" name=\"admlevel\" value=\"$row[2]\"></td></tr>\n";
    print "<tr><td>Senha:</td><td><input type=password size=\"20\" name=\"password\"></td></tr>\n";
    print "<tr><td></td><td><i><small>Deixe em branco para manter a senha atual inalterada.<br>Senhas com menos de 4 carateres bloqueam a conta.</small></i></td></tr>\n";
    print "<tr><td></td><td align=right><input type=submit value=\"Alterar\">&nbsp;<input type=button value=\"Cancelar\" onClick=\"history.go(-1)\"></td></tr></form></div>\n";
  }

  // delete user
  if ($f==25) {
    if (!isset($_GET['i'])) fatal_error("Requisição incorreta.","back");
    $qid = pg_escape_string($_GET['i']);
    if ($qid == $_SESSION['userid']) fatal_error("Você não pode se auto-deletar.");
    // TODO: verificar se o id do usuario eh referenciado em outras tabelas do BD.
    // Se for, negar a remocao.
    $sql = "select username, fullname from users where id=$qid;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
    if (pg_num_rows($q) < 1) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
    $row = pg_fetch_row($q,0);
    ask_confirmation("Você realmente deseja remover a conta do usuário <b>$row[0] ($row[1])</b> ?","util_deluser.php?i=$qid","back");
  }

  // login log
  if ($f==28) {
    print "<div class=\"regular2\"><h3>Histórico de Logins</h3>\n";
    $p = 1;
    if (isset($_GET['p'])) $p = $_GET['p'];
    $n = scalar_query("select count(username) from loginlog;");
    $npages = (int) ceil($n / 200);
    page_selector($p,$npages,$n,"$link&f=28");
    $offset = ($p-1) * 200;    
    $sql = "select to_char(time,'YYYY-MM-DD HH24:MI:SS'), username, ipaddr, agent, success from loginlog order by time desc limit 200 offset $offset;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $n = pg_num_rows($q);
    if ($n > 0) {
      print "<table border=0 cellspacing=1 cellpadding=4><tr class=\"title\"><td>Data</td><td>Usuário</td><td>IP</td><td>Agente</td><td>Sucesso</td></tr>\n";
      
      for($i=0;$i<$n;$i++) {
	$row = pg_fetch_row($q,$i);
	$sv = $row[4] == 't' ? 'Sim' : 'Não';
	print "<tr class=\"" . (($row[4] == 't') ? "greenlog" : "redlog") . "\">";
	print "<td class=s>$row[0]</td><td class=s>$row[1]</td><td class=s>$row[2]</td><td class=s>$row[3]</td><td class=s>$sv</td></tr>\n";
      }

      print "</table>\n";
      page_selector($p,$npages,$n,"$link&f=28");
    }
    print "</div>\n";
  }

  // edit log
  if ($f==26) {
    print "<div class=\"regular2\"><h3>Log de Edições</h3>\n";
    $p = 1;
    if (isset($_GET['p'])) $p = $_GET['p'];
    
    $sql = "select count(uid) from editlog;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $n = pg_fetch_result($q,0,0);
    $npages = (int) ceil($n / 200);
    page_selector($p,$npages,$n,"$link&f=26");

    $offset = ($p-1) * 200;    
    $sql = "select e.uid, date_trunc('seconds',e.edited), message, u.username, u.fullname from editlog e, users u where u.id=e.uid order by e.edited desc limit 200 offset $offset;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $n = pg_num_rows($q);
    if ($n > 0) {
      print "<table border=0 cellspacing=1 cellpadding=4><tr class=\"title\"><td>Data</td><td>Usuário</td><td>Edição</td></tr>\n";
      
      for($i=0;$i<$n;$i++) {
	$row = pg_fetch_row($q,$i);
	print "<tr class=\"" . (($i % 2) ? "odd" : "even") . "\">";
	print "<td class=s>$row[1]</td><td class=s>$row[4] ($row[3])</td><td class=s>$row[2]</td></tr>\n";
      }

      print "</table>\n";
      page_selector($p,$npages,$n,"$link&f=26");
    }
    print "</div>\n";
  }

  // maintenance
  if ($f==27) {
    print "<div class=\"regular2\"><h3>Diagnóstico</h3>\n";
    
    print "<b>Utilização de Disco na Partição do Banco de Dados / Sistema:</b><ul><pre>";
    runfile("/bin/df -h /var/lib/pgsql");
    print "</pre></ul><b>Utilização de Disco na Partição de Arquivos DICOM, Anexos e Pacotes de usuários:</b><ul><pre>";
    runfile("/bin/df -h /home/lnidb2");
    print "</pre></ul>\n";

    print "<b>Estatísticas do Banco de Dados:</b><ul>";

    $vnfiles = $nfiles = scalar_query("select count(id) from files;");
    if ($nfiles<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $nfiles = number_format($nfiles, 0, '.', ' ');

    $n100 = $f100 = scalar_query("select count(id) from files where dlcount >= 100;");
    if ($f100<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $f100 = number_format($f100, 0, '.', ' ');

    $p100 = (100.0 * $n100) / $vnfiles;
    $p100 = number_format($p100, 4, '.', ' ');

    $n10 = $f10 = scalar_query("select count(id) from files where dlcount >= 10;");
    if ($f10<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $f10 = number_format($f10, 0, '.', ' ');

    $p10 = (100.0 * $n10) / $vnfiles;
    $p10 = number_format($p10, 4, '.', ' ');

    $n1 = $f1 = scalar_query("select count(id) from files where dlcount >= 1;");
    if ($f1<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $f1 = number_format($f1, 0, '.', ' ');

    $p1 = (100.0 * $n1) / $vnfiles;
    $p1 = number_format($p1, 4, '.', ' ');

    $total = scalar_query("select sum(bytes) from studies;");
    if ($total<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $hsize = human_size($total);
    $gtotal = number_format($total, 0, '.', ' ');

    print "<li>$nfiles arquivo(s) DICOM totalizando $hsize ($gtotal bytes).</li>\n";
    print "<ul>$f1 arquivo(s) ($p1%) com 1 ou mais downloads/visualizações.<br>\n";
    print "$f10 arquivo(s) ($p10%) com 10 ou mais downloads/visualizações.<br>\n";
    print "$f100 arquivo(s) ($p100%) com 100 ou mais downloads/visualizações.</ul>\n";

    // attachs

    $vnattach = $nattach = scalar_query("select count(id) from attachments;");
    if ($nattach<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $nattach = number_format($nattach, 0, '.', ' ');

    $an100 = $af100 = scalar_query("select count(id) from attachments where dlcount >= 100;");
    if ($af100<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $af100 = number_format($af100, 0, '.', ' ');

    $ap100 = (100.0 * $an100) / $vnattach;
    $ap100 = number_format($ap100, 4, '.', ' ');

    $an10 = $af10 = scalar_query("select count(id) from attachments where dlcount >= 10;");
    if ($af10<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $af10 = number_format($af10, 0, '.', ' ');

    $ap10 = (100.0 * $an10) / $vnattach;
    $ap10 = number_format($ap10, 4, '.', ' ');

    $an1 = $af1 = scalar_query("select count(id) from attachments where dlcount >= 1;");
    if ($af1<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $af1 = number_format($af1, 0, '.', ' ');

    $ap1 = (100.0 * $an1) / $vnattach;
    $ap1 = number_format($ap1, 4, '.', ' ');

    $atotal = scalar_query("select sum(bytes) from attachments;");
    if ($atotal<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $ahsize = human_size($atotal);
    $agtotal = number_format($atotal, 0, '.', ' ');

    print "<li>$nattach arquivo(s) anexo(s) totalizando $ahsize ($agtotal bytes).</li>\n";
    print "<ul>$af1 arquivo(s) ($ap1%) com 1 ou mais downloads.<br>\n";
    print "$af10 arquivo(s) ($ap10%) com 10 ou mais downloads.<br>\n";
    print "$af100 arquivo(s) ($ap100%) com 100 ou mais downloads.</ul>\n";

    // studies / patients

    $nstudies = scalar_query("select count(id) from studies;");
    $nnstudies = $nstudies;
    if ($nstudies<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $nstudies = number_format($nstudies, 0, '.', ' ');

    $npat = scalar_query("select count(id) from patients;");
    if ($npat<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $npat = number_format($npat, 0, '.', ' ');

    $comtotal = scalar_query("select count(id) from comments;");
    if ($comtotal<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $comtotal = number_format($comtotal, 0, '.', ' ');

    $compublic = scalar_query("select count(id) from comments where public=1;");
    if ($compublic<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $compublic = number_format($compublic, 0, '.', ' ');

    print "<li>$nstudies estudos e $npat pacientes.</li>\n";
    print "<li>$comtotal comentários ($compublic públicos).</li>\n";

    print "</ul><b>Pacotes:</b><ul>\n";

    $npack = scalar_query("select count(tid) from wrapped;");
    if ($npack<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");

    $packsize = scalar_query("select sum(size) from wrapped;");
    if ($packsize<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $hpacksize = human_size($packsize);
    $packsize = number_format($packsize, 0, '.', ' ');

    $nold = scalar_query("select count(tid) from wrapped where now()-wraptime > '10 days';");
    if ($nold<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");

    $oldsize = scalar_query("select sum(size) from wrapped where now()-wraptime > '10 days';");
    if ($oldsize<0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $holdsize = human_size($oldsize);
    $oldsize = number_format($oldsize, 0, '.', ' ');

    print "<li>$hpacksize ($packsize bytes) em $npack pacote(s).</li>\n";
    print "<li>$holdsize ($oldsize bytes) em $nold pacote(s) gerado(s) há mais de 10 dias.";

    if ($nold > 0) {
      print " <a href=\"util_purgeold.php\">(Remover pacotes antigos)</a>";
    }
    print "</li></ul>\n";

    print "<b>Qualidade dos Estudos</b><ul>\n";
    $q0 = scalar_query("select count(id) from studies where artifact=0;");
    $q1 = scalar_query("select count(id) from studies where artifact=1;");
    $q2 = scalar_query("select count(id) from studies where artifact=2;");
    $q3 = scalar_query("select count(id) from studies where artifact=3;");

    $pq0 = (100.0 * $q0) / $nnstudies;
    $pq1 = (100.0 * $q1) / $nnstudies;
    $pq2 = (100.0 * $q2) / $nnstudies;
    $pq3 = (100.0 * $q3) / $nnstudies;
    $pq0 = number_format($pq0, 2, '.', ' ');
    $pq1 = number_format($pq1, 2, '.', ' ');
    $pq2 = number_format($pq2, 2, '.', ' ');
    $pq3 = number_format($pq3, 2, '.', ' ');

    print "<li><img src=\"art0.png\"> Sem informação: $q0 ($pq0%)</li>";
    print "<li><img src=\"art1.png\"> Sem artefato: $q1 ($pq1%)</li>";
    print "<li><img src=\"art2.png\"> Artefato leve: $q2 ($pq2%)</li>";
    print "<li><img src=\"art3.png\"> Artefato sério: $q3 ($pq3%)</li></ul>";

    print "<b>Pacotes - Top 10:</b><ol>\n";

    $sql = "select u.username, u.fullname, w.size, to_char(w.wraptime,'YYYY-MM-DD'), w.path, to_char(now()-w.wraptime,'DD') from users u, wrapped w where u.id=w.owner order by w.size desc limit 10;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "index.php");
    $n = pg_num_rows($q);
    for($i=0;$i<$n;$i++) {
      $row = pg_fetch_row($q,$i);
      $spath = $row[4];
      $hsize = human_size($row[2]);
      if (preg_match('/.*\\/(.*)$/',$row[4],$rpath)) {
	$spath = $rpath[1];
      }
      print "<li>Pacote <font color=#000088><i>$spath</i></font> [ <font color=#008800><b>$row[0]</b></font> ] <b>$row[3]</b> (<b>$row[5] dias</b> atrás), <b><font color=#880088>$hsize</font></b>.</li>\n";
    }
    print "</ol>\n";

    // file importing evolution
    print "<b>Importação de Arquivos x Tempo:</b><ul>\n";    
    print "<iframe width=800 height=300 frameborder=0 src=\"util_importgraph.php\"></iframe>\n";
    print "</ul>\n";

    // studies / patients x time
    $first = scalar_query("select to_char(scantime,'YYYY-MM') from studies order by scantime limit 1;");
    $last  = scalar_query("select to_char(scantime,'YYYY-MM') from studies order by scantime desc limit 1;");
    if ($first < 0 || $last < 0) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");

    $m1 = 1;
    $y1 = 2008;
    $m2 = 1;
    $y2 = 2012;
    if (preg_match('/(\\d{4})-(\\d{2})/',$first,$tmp)) {
      $m1 = $tmp[2];
      $y1 = $tmp[1];
    }
    if (preg_match('/(\\d{4})-(\\d{2})/',$last,$tmp)) {
      $m2 = $tmp[2];
      $y2 = $tmp[1];
    }
    $m1 = 1;
    $m2 = 12;

    $sbm = array();
    $m = $m1;
    $y = $y1;
    $mc = 0;

    do {
      $nm = $m+1;
      $ny = $y;
      if ($nm == 13) {$nm = 1; $ny++; }
      $v = scalar_query("select count(id) from studies where scantime >= '$y-$m-1 00:00:00' and scantime < '$ny-$nm-1 00:00:00';");
      if ($v < 0) $v = 0;
      array_push($sbm, $v);
      $mc++;
      $m = $nm;
      $y = $ny;
    } while($y <= $y2);

    $mv = max($sbm);
    print "<table border=0 cellpadding=2 cellspacing=2>\n";
    print "<tr><td colspan=14 class=diagtitle>Estudos x Tempo</td></tr>\n";
    print "<tr><td></td><td><b>Jan</b></td><td><b>Fev</b></td><td><b>Mar</b></td><td><b>Abr</b></td><td><b>Mai</b></td><td><b>Jun</b></td><td><b>Jul</b></td><td><b>Ago</b></td><td><b>Set</b></td><td><b>Out</b></td><td><b>Nov</b></td><td><b>Dez</b></td></tr>\n";
    $m = $m1;
    $y = $y1;
    $cw = array();
    $colors = array("#ffffff","#ffdddd",
		    "#ffbbbb","#ffaaaa",
		    "#ff8888","#ff6666",
		    "#ff5555","#ff4444",
		    "#ff3333","#ff2222");
		    
    for($i=0;$i<$mc;$i++) {
      if ($m==1) {
	print "<tr valign=bottom><td><b>$y</b></td>";
	$yt = 0;
      }
      
      $w = round((10 * $sbm[$i]) / $mv); 
      if ($w < 0) $w=0; if ($w>9) $w=9;
      print "<td bgcolor=\"$colors[$w]\" align=center>$sbm[$i]</td>\n";
      $yt += $sbm[$i];
      array_push($cw,$sbm[$i]);

      if ($m==12) {
	print "<td>";
	util_histogram($cw,5,13);
	print "</td>";
	$cw = array();
	print "<td>Total Anual: $yt</td></tr>\n";
      }
      $m++;
      if ($m==13) { $m=1; $y++; }
    }

    // pacientes x tempo

    $sbm = array();
    $m = $m1;
    $y = $y1;
    $mc = 0;

    do {
      $nm = $m+1;
      $ny = $y;
      if ($nm == 13) {$nm = 1; $ny++; }
      $v = scalar_query("select count(distinct ps.pid) from patstudies ps, studies s where s.scantime >= '$y-$m-1 00:00:00' and s.scantime < '$ny-$nm-1 00:00:00' and s.id=ps.sid;");
      if ($v < 0) $v = 0;
      array_push($sbm, $v);
      $mc++;
      $m = $nm;
      $y = $ny;
    } while($y <= $y2);

    $mv = max($sbm);
    print "<tr><td colspan=14>&nbsp;</td></tr>\n";
    print "<tr><td colspan=14 class=diagtitle><b><font color=white>Pacientes x Tempo</font></b></td></tr>\n";
    print "<tr><td></td><td><b>Jan</b></td><td><b>Fev</b></td><td><b>Mar</b></td><td><b>Abr</b></td><td><b>Mai</b></td><td><b>Jun</b></td><td><b>Jul</b></td><td><b>Ago</b></td><td><b>Set</b></td><td><b>Out</b></td><td><b>Nov</b></td><td><b>Dez</b></td></tr>\n";
    $m = $m1;
    $y = $y1;
    $cw = array();
    for($i=0;$i<$mc;$i++) {
      if ($m==1) {
	print "<tr valign=bottom><td><b>$y</b></td>";
	$yt = 0;
      }
      
      $w = round((10 * $sbm[$i]) / $mv); 
      if ($w < 0) $w=0; if ($w>9) $w=9;
      print "<td bgcolor=\"$colors[$w]\" align=center>$sbm[$i]</td>\n";
      $yt += $sbm[$i];
      array_push($cw,$sbm[$i]);

      if ($m==12) {
	print "<td>";
	util_histogram($cw,5,13);
	print "</td>";
	$cw = array();
	print "<td>Total Anual: $yt</td></tr>\n";
      }
      $m++;
      if ($m==13) { $m=1; $y++; }
    }

    print "</table>\n";

  }


}

function runfile($src) {
  $md = popen($src,"r");
  if ($md) {
    while($s = fgets($md)) {
      print "$s";
    }
    pclose($md);
  } else {
    print "Erro de execução: $src\n";
  }
}


?>
