<?php

function grupos_main() {
  global $_GET;
  global $_SESSION;

  if (!isset($_SESSION['userid'])) fatal_error("Função disponível apenas para usuários logados.",menu_link('pac'));
  $uid = $_SESSION['userid'];

  if (isset($_GET['g'])) {
    grupos_detail($_GET['g']);
    return;
  }
  if (isset($_GET['s']))
    $_SESSION['g.sort'] = $_GET['s'];
  $sort = 0;

  if (isset($_SESSION['g.sort']))
    $sort = $_SESSION['g.sort'];

  $link = menu_link('gru');
  $sortdesc = array("Título","Data,Criador,Título","Criador,Data,Título",
		    "Permissões,Criador,Data,Título","Permissões,Data,Criador,Título");  
  $sortsql = array("3","5 desc,8,3","8,5 desc,3","6 desc,8,5 desc,3","6 desc,5 desc,8,3");
  $nsort = 5;
  if ($sort < 0 || $sort >= $nsort) $sort = 0;

  print "<script language=\"JavaScript\" src=\"grupos.js\"></script>\n";
  print "<div class=\"regular\">\n";
  print "<h3>Grupos de Estudos</h3>";

  print "<a href=\"javascript:newgroup()\">[Adicionar Novo Grupo de Estudos]</a><br>";

  // new group dialog
  print "<div class=\"dialogbox\" id=\"newgroupbox\"><form method=\"post\" action=\"grunew2.php\">\n";
  print "<h2>Novo Grupo de Estudos</h2>";
  
  print "<input type=hidden name=\"uid\" value=\"$uid\">\n";
  print "<table border=0 cellpadding=4 cellspacing=4>\n";
  print "<tr valign=center><td>Título:</td><td><input type=\"text\" id=\"groupform1\" name=\"title\" size=30></td></tr>\n";
  print "<tr valign=center><td>Descrição:</td><td><input type=\"text\" name=\"desc\" size=60></td></tr>\n";
  print "<tr valign=center><td>Permissões/Visibilidade:</td><td><select name=\"public\">";
  print "<option value=\"2\" selected>Colaborativo (todos podem ver e modificar)</option>";
  print "<option value=\"1\">Público (todos podem ver, apenas criador pode modificar)</option>";
  print "<option value=\"0\">Privado (controle individual decidido pelo criador)</option></select>";
  
  print "</td></tr>\n";
  print "<tr><td colspan=2 align=center><input type=submit value=\"Adicionar\">&nbsp;&nbsp;<input type=button value=\"Cancelar\" onClick=\"javascript:newgroupcancel()\"></td></tr></table></div>\n";
  // end of new group dialog

  $sql = "select g.id,g.owner,g.title,g.description,to_char(g.created,'YYYY-MM-DD'),g.public,u.username,u.fullname from groups g, users u where u.id=g.owner and (g.owner=$uid or g.public >= 1) union select g.id,g.owner,g.title,g.description,to_char(g.created,'YYYY-MM-DD'),g.public,u.username,u.fullname from groups g, users u, gvisible v where g.public=0 and g.owner <> $uid and u.id=g.owner and v.gid=g.id and v.uid=$uid order by $sortsql[$sort];";
  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),menu_link('gru'));
  $n = pg_num_rows($q);

  if ($n > 0) {
    print "<br>$n grupo(s) de estudos visíveis. Clique no título para exibir detalhes.<br><br>\n";

    print "<span style=\"color:#cc8833; font-weight:bold\">Ordenar por</span> ";
    for($i=0;$i<$nsort;$i++)
      print "<a class=" . ($i==$sort ? 'csort' : 'sort') . " href=\"$link&s=$i\">[$sortdesc[$i]]</a> ";
    print "<br><br>\n";

  } else {
    print "<br>Nenhum grupo de estudos visível.<br><br>\n";
  }
  print "<table cellpadding=2 cellspacing=2><tr class=title><td class=s>#</td>\n";

  print "<td class=s>Título</td>";
  print "<td class=s>Tamanho</td>";
  print "<td class=s>Criação</td>";
  print "<td class=s>Permissões</td></tr>";

  for($i=0;$i<$n;$i++) {
    print "<tr class=\"" . ($i%2 == 0 ? "even" : "odd") . "\">\n";
    $row = pg_fetch_row($q,$i);
    $j = $i + 1;

    $s1 = scalar_query("select count(sid) from groupstudies where gid=$row[0];");
    if ($s1 > 0)
      $s2 = scalar_query("select sum(s.bytes) from groupstudies gs, studies s where gs.gid=$row[0] and gs.sid=s.id;");
    else
      $s2 = 0;
    $s2 = human_size($s2);
    
    print "<td class=s>$j</td>";
    print "<td class=s><a href=\"$link&g=$row[0]\">$row[2]</a></td>";
    print "<td class=s>$s1 estudo(s) ($s2)</td>";
    print "<td class=s>$row[4] por <b>$row[6]</b> ($row[7])</td>";
    print "<td class=s>";

    switch($row[5]) {
    case 0:

      if ($row[1] == $uid) {
	print "<img src=\"keygold.png\" alt=\"Privado (Próprio)\"> ";
	$v = scalar_query("select count(uid) from gvisible where gid=$row[0] and change=0;");
	$e = scalar_query("select count(uid) from gvisible where gid=$row[0] and change=1;");
	print "Privado (próprio), +$v usuário(s) podem ver, +$e usuário(s) podem ver e modificar.";
      } else {
	$v = scalar_query("select count(uid) from gvisible where gid=$row[0] and uid=$uid and change=1;");
	if ($v == 1) {
	  print "<img src=\"keysilver.png\" alt=\"Privado (Alheio)\"> ";
	  print "Privado (alheio). Você pode ver e modificar o grupo.";
	} else {
	  print "<img src=\"keybronze.png\" alt=\"Privado (Alheio)\"> ";
	  print "Privado (alheio). Você pode ver o grupo sem modificá-lo.";
	}
      }
      break;

    case 1:
	print "<img border=0 src=\"public.png\" alt=\"Público\"> Público: todos podem ver, criador pode modificar.";
	break;
    case 2:
	print "<img border=0 src=\"publicplus.png\" alt=\"Colaborativo\"> Colaborativo: todos podem ver e modificá-lo.";
	break;

    }
    print "</td>\n";

    print "</tr>\n";
  }

  print "</table>\n";

  print "<br><div class=\"chartkey\"><b>Legenda de Permissões</b><br>";
  print "<img src=\"publicplus.png\"> Colaborativo: todos podem ver, todos podem modificar.</img><br>";
  print "<img src=\"public.png\"> Público: todos podem ver, apenas o criador pode modificar.</img><br>";
  print "<img src=\"keygold.png\"> Privado Próprio: grupo privado, você é o dono e controla que usuários têm permissão para ver e/ou modificar o grupo.</img><br>";
  print "<img src=\"keysilver.png\"> Privado Alheio Modificável: grupo privado alheio, e você tem permissão para vê-lo e modificá-lo.</img><br>";
  print "<img src=\"keybronze.png\"> Privado Alheio Não-Modificável: grupo privado alheio, e você tem permissão apenas para vê-lo.</img></div>";


  print "</div>\n";

}

function grupos_detail($gid) {
  global $_GET;
  global $_SESSION;
  
  if (!isset($_SESSION['userid'])) fatal_error("Função disponível apenas para usuários logados.",menu_link('pac'));
  $uid = $_SESSION['userid'];

  // study sorting
  if (isset($_GET['s'])) {
    $sort = $_GET['s'];
    $_SESSION['g.sort'] = $sort;
  } else if (isset($_SESSION['g.sort'])) {
    $sort = $_SESSION['g.sort'];
  } else {
    $sort = 0;
  }

  if (!isset($_GET['g'])) {
    grupos_main();
    return;
  }

  $link = menu_link('gru');
  $linkp = menu_link('pac');
  $gid = $_GET['g'];

  // operations
  if (isset($_GET['o'])) {
    $op = $_GET['o'];
    if ($op == 'rm') {
      grupos_rm();
      return;
    }
    if ($op == 'clrgrp') {
      grupos_clr();
      return;
    }
  }

  // check permissions
  if (!grupos_can_view($uid,$gid))
    fatal_error("Você não tem permissão para ver este grupo de estudos (uid=$uid, gid=$gid).",menu_link('gru'));

  print "<script language=\"JavaScript\" src=\"grupos.js\"></script>\n";
  print "<div class=\"regular\">";

  $sql = "select g.id,g.title,g.description,g.public,g.owner,to_char(g.created,'YYYY-MM-DD'),u.username,u.fullname from groups g, users u where g.owner=u.id and g.id=$gid;";
  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),menu_link('gru'));
  $n = pg_num_rows($q);
  if ($n!=1) fatal_error("Resultado Inesperado.",menu_link('gru'));
  $row = pg_fetch_row($q,0);

  print "<a href=\"$link\">&lt; Lista de Grupos de Estudos</a><br>";
  print "<h3>Detalhe de Grupo de Estudos</h3>";
  print "<table cellspacing=4 cellpadding=4>";
  print "<td>Título:</td><td><b>$row[1]</b></td></tr>";
  print "<td>Descrição:</td><td><b>$row[2]</b></td></tr>";
  print "<td>Criação:</td><td>Em <i>$row[5]</i> por <b>$row[6]</b> ($row[7])</td></tr>";
  print "<td>Visibilidade:</td><td>";

  $perm = -1;
  $pd  = array("Privado Próprio", "Público", "Colaborativo","Privado Alheio Modificável","Privado Alheio Não-Modificável");
  $pdi = array("keygold.png","public.png","publicplus.png","keysilver.png","keybronze.png");

  if ($uid == $row[4] || $row[3] > 0) {
    print "<img src=\"${pdi[$row[3]]}\"> ${pd[$row[3]]}";
    $perm = $row[3];
  } else {
    $v = scalar_query("select count(uid) from gvisible where gid=$row[0] and
 uid=$uid and change=1;");
    if ($v==1) {
      print "<img src=\"$pdi[3]\"> $pd[3]";
      $perm = 3;
    } else {
      print "<img src=\"$pdi[4]\"> $pd[4]";
      $perm = 4;
    }
  }

  print "</td></tr>";

  if ($perm == 0 || $perm == 3 || $perm == 4) {
    print "<tr><td>Usuários que podem <span style=\"color:#880088\">ver</span> e <span style=\"color:#880088; text-decoration:underline\">editar</span>:</td><td>";

    $q2 = pg_query("select v.uid,v.change,u.username,u.fullname from gvisible v, users u where v.gid=$gid and v.uid=u.id order by u.username;");
    if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),menu_link('gru'));
    $n2 = pg_num_rows($q2);
    for($i=0;$i<$n2;$i++) {
      $r2 = pg_fetch_row($q2,$i);
      if ($r2[1] == 1) print "<span style=\"color:#880088; text-decoration:underline;\">"; else print "<span style=\"color: #880088\">";
      print "<b>$r2[2]</b> ($r2[3])</span>";
      if ($i < $n2-1) print ", ";
    }
    print " <i><small>$n2 usuário(s)</small></i></td></tr>";
  }

  print "</table>";

  print "<h4>Operações</h4><ul><table class=\"gop\"><tr class=gop>\n";
  if ($uid == $row[4]) print "<td class=\"admgop\">Operações Exclusivas do Criador</td>"; 
  print "<td class=\"reggop\">Operações Regulares</td></tr><tr class=gop>\n";

  if ($uid == $row[4]) {
    print "<td class=\"admgop\">";
    print "<a href=\"javascript:rmgroup('&o=rm');\" title=\"Remover Grupo\"><img src=\"groupdel.png\" border=0></a>&nbsp;";

    if ($perm == 0) {
      print "<a href=\"javascript:grantuser();\" title=\"Adicionar Permissão a Usuário\"><img src=\"useradd.png\" border=0></a>&nbsp;";
      print "<a href=\"javascript:revokeuser();\" title=\"Remover Permissão de Usuário\"><img src=\"userdel.png\" border=0></a>";
      print "</td>";
    }

  } // group owner operations

  print "<td class=\"reggop\">";
  $canmodify = ($uid == $row[4] || $perm==0 || $perm==2 || $perm==3);
  $ns = scalar_query("select count(sid) from groupstudies where gid=$gid;");
  if ($canmodify) {
    if ($ns > 0)
      print "<a href=\"javascript:cleangroup('&o=clrgrp')\" title=\"Remover Todos os Estudos ($ns) deste Grupo\"><img src=\"cleangroup.png\" border=0></a>&nbsp;";
    else
      print "<img src=\"cleangroupd.png\" border=0 title=\"(Remover Todos os Estudos)\">&nbsp;";
  }

  if ($ns > 0)
    print "<a href=\"gruedit.php?gid=$gid&op=tobasket\" title=\"Copiar Estudos do Grupo ($ns) para a Cesta\"><img src=\"tobasket.png\" border=0></a>&nbsp;";
  else
    print "<img src=\"tobasketd.png\" border=0 title=\"(Copiar Estudos do Grupo para a Cesta)\">&nbsp;";

  if ($canmodify) {
    $nb = scalar_query("select count(distinct s.id) from studies s, filestudies fs, basket b where s.id=fs.sid and fs.fid=b.fid and b.uid=$uid;");
    if ($nb > 0)
      print "<a href=\"gruedit.php?gid=$gid&op=frombasket\" title=\"Adicionar Estudos da Cesta ($nb) ao Grupo\"><img src=\"frombasket.png\" border=0></a>&nbsp;";
    else
      print "<img src=\"frombasketd.png\" border=0 title=\"(Adicionar Estudos da Cesta)\">&nbsp;";
  }

  print "</td></tr></table>";
  print "</ul>\n";

  // create the hidden divs outside the ops table, if needed
  if ($uid == $row[4] && $perm==0) {
    // grant div
    print "<div id=\"grantbox\" class=\"dialogbox\"><h2>Adicionar Permissão a Usuário</h2>";
    print "<form action=\"gruedit.php\" method=POST><table cellspacing=4 cellpadding=4>";      
    print "<tr><td>Usuário:</td><td><select name=\"tuid\">";
    
    $q2 = pg_query("select id,username,fullname from users where id>10 and id!=$uid and admlevel>=0 except select u.id,u.username,u.fullname from users u, gvisible v where u.id=v.uid and v.gid=$gid order by 3;");
    if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),menu_link('gru'));
    $n2 = pg_num_rows($q2);
    for($i=0;$i<$n2;$i++) {
      $r2 = pg_fetch_row($q2,$i);
      print "<option value=\"$r2[0]\">$r2[2] ($r2[1])</option>";
    }
    print "</select></td></tr>\n";
    print "<tr><td colspan=2><input type=checkbox name=\"change\">Permitir Modificações (Adicionar ou remover estudos do grupo)</input></td></tr>\n";
    
    print "<tr><td colspan=2 align=center><input type=hidden name=\"gid\" value=\"$gid\"><input type=hidden name=\"op\" value=\"grant\">";
    print "<input type=submit value=\"Adicionar\">&nbsp;<input type=button value=\"Cancelar\" onClick=\"cancelgrant()\"></form></td></tr></table></div>";
    
    // revoke div
    print "<div id=\"revokebox\" class=\"dialogbox\"><h2>Remover Permissão de Usuário</h2>";
    print "<form action=\"gruedit.php\" method=POST><table cellspacing=4 cellpadding=4>";      
    print "<tr><td>Usuário:</td><td>";
    
    $q2 = pg_query("select u.id,u.username,u.fullname from users u, gvisible v where u.id=v.uid and v.gid=$gid order by 3;");
    if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),menu_link('gru'));
    $n2 = pg_num_rows($q2);
    
    if ($n2 > 0)
      print "<select name=\"tuid\">";
    else
      print "Não há usuários para remover.";
    
    for($i=0;$i<$n2;$i++) {
      $r2 = pg_fetch_row($q2,$i);
      print "<option value=\"$r2[0]\">$r2[2] ($r2[1])</option>";
    }
    if ($n2 >0) print "</select>";
    
    print "</td></tr>\n";
    
    print "<tr><td colspan=2 align=center><input type=hidden name=\"gid\" value=\"$gid\"><input type=hidden name=\"op\" value=\"revoke\">";
    if ($n2 > 0)
      print "<input type=submit value=\"Remover\">&nbsp;";
    print "<input type=button value=\"Cancelar\" onClick=\"cancelrevoke()\"></form></td></tr></table></div>";
  }
  
  // estudos
  print "<h4>Estudos ($ns)</h4>";
  print "<div id=\"divabox\" class=\"fixdiva\"></div>\n";
  print "<div id=\"nojavabox\" class=\"fixnojava\"></div>\n";
  print "<script language=\"JavaScript\" src=\"deployJava.js\"></script>\n";
  print "<script language=\"JavaScript\" src=\"diva.js\"></script>\n";

  $sorts = array("p.age,p.gender,p.name","2","p.name");

  $sql = "select s.id, date_trunc('second',s.scantime),p.id,p.name,p.patcode,p.age,p.gender,s.exam_desc,s.series_desc,s.cols,s.rows,s.slices,round((s.thickness)::numeric,2),round((s.xspacing)::numeric,2),round((s.yspacing)::numeric,2),round((s.zspacing)::numeric,2),s.files,s.storage,sc.maker,sc.model,sc.location,s.oaxi,s.osag,s.ocor,s.bytes from studies s,patstudies ps,patients p,groupstudies gs,scanners sc where gs.gid=$gid and s.id=gs.sid and ps.pid=p.id and ps.sid=s.id and s.scanner=sc.id order by ";

  //$sql .= $sorts[$sort] . " limit 100;"; // TODO
  $sql .= $sorts[$sort] . ";"; // TODO

  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  $n = pg_num_rows($q);

  print "<table cellpadding=2 cellspacing=2><tr class=title><td class=s>#</td>\n";

  print "<td class=s><a class=w href=\"$link&g=$gid&s=1\">Data</a></td>\n";
  print "<td class=s><a class=w href=\"$link&g=$gid&s=2\">Paciente</a> (HC)</td>\n";
  print "<td class=s><a class=w href=\"$link&g=$gid&s=0\">Idade/Sexo</a></td>\n";
  print "<td class=s>Exame / Série</td>\n";
  print "<td class=s>Dimensões</td>\n";
  print "<td class=s>Disco/Tamanho</td>\n";
  print "<td class=s>Scanner</td></tr>\n";

  // 0 sid, date, pid, name, patcode
  // 5 age, gender, exam, series, cols
  // 10 rows, slices, thickness, x, y
  // 15 z, files, storage, maker, model
  // 20 location, oaxi, osag, ocor, bytes
  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q, $i);
    $nr = $i + 1;
    print "<tr class=\"" . ($i%2 == 0 ? "even" : "odd") . "\">\n";
    print "<td class=s align=right>$nr</td>";
    print "<td class=s>$row[1]</td>";
    print "<td class=s>$row[3] ($row[4])<br>";

    print "<a title=\"Detalhes do Paciente\" href=\"$linkp&v=$row[2]&e=0\"><img border=0 src=\"bdetail16.png\"></a>\n";    
    
    print "<a href=\"javascript:void(0)\"><img border=0 onClick=\"javascript:diva($row[0])\" title=\"Visualizar Estudo\" src=\"bview.png\"></a>&nbsp;";

    if ($row[16] == 1)
      print "<a href=\"getfile.php?s=$row[0]\"><img border=0 title=\"Download\" src=\"bdownload.png\"></a>&nbsp;";
    else
      print "<img src=\"null.png\" width=16 height=2>&nbsp;";

    
    // basket
    $bsql = "select distinct fs.sid from filestudies fs, basket b where b.uid=$uid and b.fid=fs.fid;";
    $bq = pg_query($bsql);
    $bsid = pg_fetch_all_columns($bq,0);

    // basket buttons
    $inbasket = 0;
    for($j=0;$j<count($bsid);$j++) {
      if ($bsid[$j] == $row[0]) {
	$inbasket = 1;
	break;
      }
    }
    
    if ($inbasket != 0) {
      print "<a title=\"Remover Estudo da Cesta\" href=\"basket.php?o=2&s=$row[0]\"><img border=0 src=\"bminus.png\"></a>\n";
      print "<img border=0 src=\"baddd.png\">\n";
    } else {
      print "<img border=0 src=\"bminusd.png\">\n";
      print "<a title=\"Adicionar Estudo à Cesta\" href=\"basket.php?o=1&s=$row[0]\"><img border=0 src=\"badd.png\"></a>\n";
    }

    if (grupos_can_change($uid,$gid)) {
      print "<img src=\"null.png\" width=20 height=5><a title=\"Remover Estudo do Grupo\" href=\"gruedit.php?gid=$gid&op=dropstudy&sid=$row[0]\"><img border=0 src=\"bredx.png\"></a>";
    }

    print "</td>";

    print "<td class=s>$row[5]/$row[6]</td>";
    print "<td class=s>$row[7]<br>$row[8]</td>";

    $osum = $row[21] + $row[22] + $row[23];
    $ori = '';
    if ($row[21] != 0) { $ori .= "Ax/"; }
    if ($row[22] != 0) { $ori .= "Sag/"; }
    if ($row[23] != 0) { $ori .= "Cor/"; }
    if ($osum > 0) {
      $ori = substr($ori,0,-1);
    } else {
      $ori = 'n/a';
    }
    $dim = "<span style=\"color:#880088;\">$ori</span> ${row[9]}x${row[10]}x${row[11]}<br>${row[13]}x${row[14]}x${row[15]} mm<br>thickness ${row[12]} mm";

    print "<td class=s>$dim</td>";
    $hs = human_size($row[24]);
    print "<td class=s>$row[17]<br>$row[16] arquivo(s) ($hs)</td>";
    print "<td class=s>$row[17]<br>$row[18] $row[19] $row[20]</td></tr>";
  }

  print "</table>\n";

  print "<h4>Estatísticas</h4><ul>\n";
  grupos_stats($gid);
  print "</ul>";
  print "</div><br><br>";

}

function grupos_rm() {
  global $_GET;
  global $_SESSION;

  if (!isset($_SESSION['userid'])) fatal_error("Função disponível apenas para usuários logados.",menu_link('pac'));
  $uid = $_SESSION['userid'];
  $gid = $_GET['g'];
  $link = menu_link('gru');

  if (scalar_query("select count(id) from groups where id=$gid and owner=$uid;") != 1)
    fatal_error("Apenas o criador do Grupo de Estudos pode removê-lo.",$link);

  $q = pg_query("delete from groupstudies where gid=$gid; delete from gvisible where gid=$gid; delete from groups where id=$gid;");
  if ($q == FALSE)
    fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), menu_link('gru'));
  else
    notify_user("Grupo Removido.",menu_link('gru'));
}

function grupos_clr() {
  global $_GET;
  global $_SESSION;

  if (!isset($_SESSION['userid'])) fatal_error("Função disponível apenas para usuários logados.",menu_link('pac'));
  $uid = $_SESSION['userid'];
  $gid = $_GET['g'];
  $link = menu_link('gru');

  if (!grupos_can_change($uid,$gid))
    fatal_error("Você não tem permissão para modificar este grupo de estudos.",$link);

  $q = pg_query("delete from groupstudies where gid=$gid;");
  if ($q == FALSE)
    fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  else {
    $n = pg_affected_rows($q);
    notify_user("$n estudos removidos do grupo.","back");
  }

}

function grupos_can_change($u,$g) {
  $n = scalar_query("select count(id) from groups where id=$g and (public=2 or owner=$u);") +
    scalar_query("select count(gid) from gvisible where gid=$g and uid=$u and change=1;");
  return($n > 0);
}

function grupos_can_view($u,$g) {
  $n = scalar_query("select count(id) from groups where id=$g and (public>0 or owner=$u);") +
    scalar_query("select count(gid) from gvisible where gid=$g and uid=$u;");
  return($n > 0);
}

function grupos_stats($gid) {
  $sql = "select distinct p.id,p.age,p.gender from patients p, groupstudies gs, patstudies ps where gs.gid=$gid and gs.sid=ps.sid and p.id=ps.pid;";
  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");  
  $n = pg_num_rows($q);

  $male  = array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
  $fem   = array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
  $other = 0;

  $agem = array();
  $agef = array();
  $ageo = array();
  $agea = array();

  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q,$i);
    if ($row[2] == 'O' || $row[1] < 0) {
      array_push($ageo, $row[1]);
      array_push($agea, $row[1]);
      $other++;
      continue;
    }
    $j = intval($row[1] / 5);
    if ($j>=20) $j=19;
    if ($row[2] == 'M') {
      $male[$j]++;
      array_push($agem, $row[1]);
      array_push($agea, $row[1]);
    } else {
      $fem[$j]++;
      array_push($agef, $row[1]);
      array_push($agea, $row[1]);
    }
  }

  $m1 = $m2 = $m3 = $m4 = 0;

  for($i=0;$i<20;$i++) {
    $m3 += $male[$i];
    $m4 += $fem[$i];
    if ($male[$i] > $m1) { $m1 = $male[$i]; }
    if ($fem[$i] > $m2) { $m2 = $fem[$i]; }
  }

  if ($m1 < $m2) $m1 = $m2; else $m2 = $m1;
  
  $total = $m3+$m4+$other;
  print "<table class=gstat>\n";
  
  print "<tr class=gstat1><td class=gstatlabel>Homens</td>";
  for($i=0;$i<20;$i++) {
    if ($m1==0) $h=0; else $h = intval( (50.0*$male[$i])/$m1 );
    print "<td class=gstatup><img src=\"blue.png\" height=$h width=16><br>$male[$i]</td>";
  }
  print "</tr><tr class=gstat2><td class=gstatlabel>Idade</td>";

  for($i=0;$i<20;$i++) {
    $l1 = 5*$i;
    $l2 = $l1 + 4;
    if ($i==19) $range= "${l1}+"; else $range = "${l1}-${l2}";
    print "<td class=gstatmid>$range</td>";
  }

  print "<tr class=gstat3><td class=gstatlabel>Mulheres</td>";
  for($i=0;$i<20;$i++) {
    if ($m1==0) $h=0; else $h = intval( (50.0*$fem[$i])/$m1 );
    print "<td class=gstatdown>$fem[$i]<br><img src=\"pink.png\" height=$h width=16></td>";
  }
  print "</tr></table><br>";
  $s1 = g_mean($agem); $s2 = g_stdev($agem); $s1 = round($s1, 2); $s2 = round($s2, 2);
  print("<img src=\"blue.png\" width=10 height=10> Homens: <b>$m3</b> (&mu;=$s1, &sigma;=$s2)<br>\n");
  $s1 = g_mean($agef); $s2 = g_stdev($agef); $s1 = round($s1, 2); $s2 = round($s2, 2);
  print("<img src=\"pink.png\" width=10 height=10> Mulheres: <b>$m4</b> (&mu;=$s1, &sigma;=$s2)<br>\n");
  $s1 = g_mean($ageo); $s2 = g_stdev($ageo); $s1 = round($s1, 2); $s2 = round($s2, 2);
  print("<img src=\"null.png\" width=10 height=10> Outros: <b>$other</b> (&mu;=$s1, &sigma;=$s2)<br>\n");
  $s1 = g_mean($agea); $s2 = g_stdev($agea); $s1 = round($s1, 2); $s2 = round($s2, 2);
  print("<img src=\"null.png\" width=10 height=10> Total: <b>$total</b> (&mu;=$s1, &sigma;=$s2)<br>\n");
}

function g_mean($a) {
  if (count($a) == 0) return 0.0;
  return( ((float) array_sum($a)) / count($a) );
}

function g_stdev($a) {
  if (count($a) == 0) return 0.0;
  $mean = g_mean($a);
  $var = 0.0;
  foreach ($a as $i)
    $var += pow($i - $mean, 2);
  $var /= ( count($a) - 1 );
  return (float) sqrt($var);
}



?>