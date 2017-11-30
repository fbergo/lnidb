<?php

$pat      = '';
$patcode  = '';
$age      = '';
$gender   = '1';
$comments = '';
$scanner  = '';
$sort = 1;
$page = 1;

function pacientes_parse_session() {
  global $_GET, $_SESSION;
  global $pat, $patcode, $age, $gender, $scanner, $comments, $sort, $page, $attach;
  
  if (isset($_SESSION['p.patient']))  { $pat      = $_SESSION['p.patient']; }
  if (isset($_SESSION['p.patcode']))  { $patcode  = $_SESSION['p.patcode']; }
  if (isset($_SESSION['p.age']))      { $age      = $_SESSION['p.age']; }
  if (isset($_SESSION['p.gender']))   { $gender   = $_SESSION['p.gender']; }
  if (isset($_SESSION['p.sort']))     { $sort     = $_SESSION['p.sort']; }
  if (isset($_SESSION['p.scanner']))  { $scanner  = $_SESSION['p.scanner']; }
  if (isset($_SESSION['p.comments'])) { $comments = $_SESSION['p.comments']; }
  if (isset($_SESSION['p.attach']))   { $attach   = $_SESSION['p.attach']; }

  if (isset($_GET['p'])) { $page = $_GET['p']; } else { $page = 1; }
  if (isset($_GET['s'])) { $sort = $_GET['s']; $_SESSION['p.sort'] = $sort; }

}

function pacientes_detail() {
 global $_GET, $_SESSION;

 $id   = $_GET['v'];
 $edit = $_GET['e'];
  if (isset($_SESSION['userid']))
    $uid = $_SESSION['userid'];
  else
    $uid = -1;

  $link = menu_link('pac');

  $admlevel = 0;
  if (isset($_SESSION['admlevel'])) $admlevel = $_SESSION['admlevel'];

  // diva div
  print "<div id=\"divabox\" class=\"fixdiva\"></div>\n";
  print "<div id=\"nojavabox\" class=\"fixnojava\"></div>\n";

  print "<div class=\"regular\">";
  if ($edit) print "<h3>Editar Paciente</h3>"; else print "<h3>Detalhes de Paciente</h3>";

  $q = pg_query("select name, patcode, birth, age, gender from patients where id=$id;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  $row = pg_fetch_row($q,0);

  if ($edit) {
    if ($uid < 0) fatal_error("Você não tem permissão para usar esta função.","back");

    $q = pg_query("select name,patcode,birth,age,gender from patients where id=$id;");
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
    $row = pg_fetch_row($q);

    print "<table border=0 cellspacing=4 cellpadding=2>";
    print "<form action=\"patedit.php\" method=\"post\"><input type=hidden name=\"id\" value=\"$id\">\n";
    print "<tr><td>Nome:</td><td><input type=text name=\"name\" value=\"$row[0]\" size=30></td></tr>\n";
    print "<tr><td>HC/Pat-Id:</td><td><input type=text name=\"patcode\" value=\"$row[1]\" size=15></td></tr>\n";
    print "<tr><td>Nascimento:</td><td><input type=text name=\"birth\" value=\"$row[2]\" size=12></td></tr>\n";
    print "<tr><td>Sexo:</td><td><input type=text name=\"gender\" value=\"${row[4]}\" size=3></td></tr>\n";
    print "<tr><td>Idade:</td><td><b>$row[3]</b> (será recomputada a partir da data dos<br>exames e de nascimento, se esta for alterada)</td></tr>\n";
    print "<tr><td align=right colspan=2>";
    if ($uid > 0)
      print "<input type=submit value=\"Aplicar\">&nbsp;&nbsp;";
    print "<input type=button value=\"Voltar\" onClick=\"javascript:history.go(-1)\">";
    print "</td></tr></table><br><br>\n";
    print "<font color=red><b>Atenção!</b> Se os novos dados coincidirem com outro paciente existente, <br>os ";
    print "registros serão unificados e a operação não poderá ser desfeita.</font>\n";
    print "</table>\n";

  } else {
    print "<table border=0 cellspacing=4 cellpadding=2>";
    print "<tr><td>Nome:</td><td>$row[0]</td></tr>\n";
    print "<tr><td>HC/Pat-Id:</td><td>$row[1]</td></tr>\n";
    print "<tr><td>Nascimento:</td><td>$row[2]</td></tr>\n";
    print "<tr><td>Idade/Sexo:</td><td>$row[3]/$row[4]</td></tr>\n";
    print "<tr><td colspan=2>";
    if ($uid > 0)
      print "<a href=\"$link&v=${id}&e=1\">[Alterar Dados]</a> ";
    print "<a href=\"javascript:history.go(-1)\">[Voltar]</a>";
    print "</td></tr>\n";
    print "</table>\n";
    
    // scanners
    $q = pg_query("select s.maker, s.model, s.location from scanners s, patscanners ps where ps.pid=$id and ps.sid=s.id order by s.maker,s.model,s.location;");
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
    $n = pg_num_rows($q);
    print "<h4>Scanners ($n)</h4>";
    print "<ul>";
    for($i=0;$i<$n;$i++) {
      $row = pg_fetch_row($q,$i);
      print "$row[0] $row[1] $row[2]<br>\n";
    }
    print "</ul>";
    
    // comentarios
    
    $sql  = "select c.id, c.text, c.owner, date_trunc('second',c.created), c.public, u.username, u.fullname, c.created from comments c, users u where c.pid=$id and c.owner=u.id and (c.public=1 or c.owner=$uid) ";
    $sql .= "union select c.id, c.text, c.owner, date_trunc('second',c.created), c.public, u.username, u.fullname, c.created from comments c, users u, cvisible v where c.pid=$id and c.owner=u.id and c.public=0 and c.owner<>$uid and v.cid=c.id and v.uid=$uid ";
    $sql .= " order by 8;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
    $n = pg_num_rows($q);

    print "<h4>Comentários ($n)</h4>";
    print "<ul>\n";
    if ($uid > 0) print "<a href=\"patcom.php?o=3&p=$id\">[Novo Comentário]</a><br><br>";
    print "<table border=0 cellspacing=1 cellpadding=2>";

    for($i=0;$i<$n;$i++) {
      $row = pg_fetch_row($q,$i);
      print "<tr class=\"odd\" valign=top>";
      print "<td class=s>";
      if ($row[4] == 1)
	print "<img title=\"Comentário Público\" src=\"public.png\">";

      if ($row[2] == $uid) {

	print "<img title=\"Autor: $row[6] ($row[5])\" src=\"user_green.png\">";
	img_link_right("bdel16.png","Deletar","patcom.php?i=$row[0]&o=2");

	if ($row[4] == 0) {
	  print "<a href=\"patcom.php?i=$row[0]&o=1\">Visibilidade: ";
	  $q2 = pg_query("select count(uid) from cvisible where cid=$row[0];");
	  if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
	  $row2 = pg_fetch_row($q2,0);
	  print "$row2[0] usuário(s)</a>";
	}


      } elseif ($row[4] == 1) {
	print "<img title=\"Autor: $row[6] ($row[5])\" src=\"user_gray.png\">";
      } else {
	print "<img title=\"Autor: $row[6] ($row[5])\" src=\"user_shared.png\">";
      }
      print "<br><i>$row[5]</i><br>$row[3]";

      print "</td><td class=s>$row[1]</td></tr>\n";
    }
    print "</table></ul>\n";

    // estudos
    $sql = "select s.id, date_trunc('second',s.scantime),s.exam_desc,s.series_desc,s.cols,s.rows,s.slices,round((s.thickness)::numeric,2),round((s.xspacing)::numeric,2),round((s.yspacing)::numeric,2),round((s.zspacing)::numeric,2),s.files,s.storage,sc.maker,sc.model,sc.location,s.oaxi,s.osag,s.ocor,s.artifact from studies s,patstudies ps,scanners sc ";
    $sql .= "where s.id=ps.sid and ps.pid=$id and s.scanner=sc.id order by s.scantime;";
    $q = pg_query($sql);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
    $n = pg_num_rows($q);

    print "<script language=\"JavaScript\" src=\"deployJava.js\"></script>\n";
    print "<script language=\"JavaScript\" src=\"diva.js\"></script>\n";
    print "<h4>Estudos ($n)</h4>";

    print "<ul>";

    if ($uid > 0 && $n>0) {
      print "<a href=\"basket.php?o=1&p=$id\">[Adicionar todos estes estudos à Cesta]</a><br>";
      print "<a href=\"basket.php?o=2&p=$id\">[Remover todos estes estudos da Cesta]</a><br><br>";

      $bsql = "select distinct fs.sid from filestudies fs, basket b where b.uid=$uid and b.fid=fs.fid;";
      $bq = pg_query($bsql);
      $bsid = pg_fetch_all_columns($bq,0);

      $bsql = "select aid from basket2 where uid=$uid;";
      $bq = pg_query($bsql);
      $atid = pg_fetch_all_columns($bq,0);
    }

    print "<table border=0 cellspacing=1 cellpadding=2>";
    print "<tr class=title><td>Data/Hora</td><td>Exame</td><td>Série</td><td>Dimensões</td><td>Scanner</td><td>Armazenamento</td><td>Qualidade</td><td>Operações</td></tr>\n";
    for($i=0;$i<$n;$i++) {
      $row = pg_fetch_row($q,$i);
      $oe = (($i % 2 == 0) ? 'even' : 'odd');
      print "<tr class=$oe><td class=s>$row[1]</td><td class=s>$row[2]</td>";
      $osum = $row[16] + $row[17] +  $row[18];
      $ori = '';
      if ($row[16] != 0) { $ori .= "Ax/"; }
      if ($row[17] != 0) { $ori .= "Sag/"; }
      if ($row[18] != 0) { $ori .= "Cor/"; }      
      if ($osum > 0) {
	$ori = substr($ori,0,-1);
      } else {
	$ori = 'n/a';
      }
      $dim = "<font color=#880088>$ori</font> ${row[4]}x${row[5]}x${row[6]}<br>${row[8]}x${row[9]}x${row[10]} mm<br>thickness ${row[7]} mm";
      print "<td class=s>$row[3]</td><td class=s>$dim</td><td class=s>$row[13]<br>$row[14]<br>$row[15]</td>";
      print "<td class=s>$row[12]<br>$row[11] arquivo(s)</td>";

      print "<td class=s align=center>" . artifact_icon($row[19]) . "</td><td class=s>";

      //img_link("bdetail16.png","Detalhes","index.php?m=1&i=$row[0]");

      print "<a href=\"javascript:void(0)\"><img border=0 onClick=\"javascript:diva($row[0])\" title=\"Visualizar Estudo\" src=\"bview.png\"></a>&nbsp;";
      //img_link_blank("bview.png","Visualizar Estudo","view.php?s=$row[0]");
      if ($row[11] == 1 && $uid > 0) {
	img_link("bdownload.png","Download","getfile.php?s=$row[0]");
      }
      if ($uid > 0) {
	$inbasket = 0;
	for($j=0;$j<count($bsid);$j++) {
	  if ($bsid[$j] == $row[0]) {
	    $inbasket = 1;
	    break;
	  }
	}

	if ($inbasket != 0) {
	  img_link("bminus.png","Retirar da Cesta","basket.php?o=2&s=$row[0]");
	  img_nolink("baddd.png","");
	} else {
	  img_nolink("bminusd.png","");
	  img_link("badd.png","Adicionar à Cesta","basket.php?o=1&s=$row[0]");
	}
      }
      print "</td></tr>\n";
    }
  }
  print "</table>";

  print "<br><b>Obs:</b> O campo de qualidade dos estudos pode ser modificado apenas por administradores do sistema.<br>";
  if ($admlevel == 2) {
    print "Para modificar a qualidade de um ou mais estudos, adicione um ou mais estudos à cesta e use a opção <i>Alterar Marcadores de Qualidade</i> na listagem da <b>Cesta</b>.<br>";
  }


  print "</ul>\n";

  // anexos
  $sql = "select a.id,a.description,a.path,a.bytes,a.creator,u.username,u.fullname,to_char(a.created,'YYYY-MM-DD HH24:MI'),a.dlcount from attachments a, patattachs pa, users u where pa.pid=$id and pa.aid=a.id and a.creator=u.id order by id;";
  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  $n = pg_num_rows($q);

  print "<h4>Anexos ($n)</h4><ul>\n";
  if ($uid > 0) print "<a href=\"attach.php?o=1&p=$id\">[Novo Anexo]</a><br><br>";

  if ($n>0) {
    print "<table border=0 cellspacing=1 cellpadding=2>";
    print "<tr class=title><td>Arquivo</td><td>Descrição</td><td>Tamanho</td><td>Criação</td><td>Downloads</td><td>Operações</td></tr>\n";
    
    for($i=0;$i<$n;$i++) {
      $row = pg_fetch_row($q,$i);
      $oe = (($i % 2 == 0) ? 'even' : 'odd');
      
     if ($uid > 0) {
       $inbasket = 0;
       for($j=0;$j<count($atid);$j++) {
	 if ($atid[$j] == $row[0]) {
	   $inbasket = 1;
	   break;
	 }
       }
     }

     if (preg_match('/.*\\/(.+)$/',$row[2],$tmp)) {
       $row[2] = $tmp[1];
     }
     
     $sz = human_size($row[3]);
     
     print "<tr><td>$row[2]</td><td>$row[1]</td><td>$sz</td>";
     print "<td><font color=#880088>$row[7]</font> por <font color=#008800>$row[6] ($row[5])</font></td>";
     print "<td align=right>$row[8]</td>";
     print "<td>";
     
     img_link("bdownload.png","Download","getfile.php?a=$row[0]");
     if ($uid == $row[4] || $admlevel == 2)
       img_link("bdel16.png","Remover Anexo","attach.php?o=2&a=$row[0]");

     if ($uid > 0) {
       if ($inbasket != 0) {
	 img_link("bminus.png","Retirar da Cesta","basket.php?o=2&a=$row[0]");
	 img_nolink("baddd.png","");
       } else {
	 img_nolink("bminusd.png","");
	 img_link("badd.png","Adicionar à Cesta","basket.php?o=1&a=$row[0]");
       }       
     }
     
     print "</td></tr>\n";
    }

    print "</table>\n";
  }

  print "</ul><br><br><br>\n";

  print "</div>\n";
}

function compute_patperiod($pid) {
    $q3 = pg_query("select distinct s.scantime from studies s, patstudies ps, patients p where ps.sid=s.id and ps.pid=$pid order by 1;");
    if ($q3==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $n3 = pg_num_rows($q3);
    if ($n3 != 0) {
       $d1 = pg_fetch_result($q3,0,0);
       $d2 = pg_fetch_result($q3,$n3-1,0);
       $r = pg_query("insert into patperiod (pid,first,last) values ($pid,'$d1','$d2');");
       if ($r == FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    }
}

function pacientes_main() {
  global $_GET, $_SESSION;
  global $pat, $patcode, $age, $gender, $scanner, $comments, $sort, $page, $attach;

  if (isset($_GET['v']) && isset($_GET['e'])) {
    pacientes_detail();
    return;
  }

  $link = menu_link('pac');

  $print = 0;
  if (isset($_GET['print'])) $print = 1;

  pacientes_parse_session();
  if (isset($_SESSION['userid']))
    $uid = $_SESSION['userid'];
  else
    $uid = -1;

  // build query
  // TODO: fazer union na busca por comentarios para pegar comentarios privados compartilhados
  $sql1 = "select distinct p.id,p.name,p.patcode,p.age,p.gender from patients p";
  if (strlen($scanner) > 0)  $sql1 .= ",patscanners ps, scanners sc";
  if (strlen($comments) > 0) $sql1 .= ",comments c";
  if (strlen($attach) > 0)   $sql1 .= ",attachments a, patattachs pa";

  $sql2 = "select count(distinct p.id) from patients p";
  if (strlen($scanner) > 0)  $sql2 .= ",patscanners ps, scanners sc";
  if (strlen($comments) > 0) $sql2 .= ",comments c";
  if (strlen($attach) > 0)   $sql2 .= ",attachments a, patattachs pa";

  $sqlr = " where ";
  if (strlen($scanner) > 0)  $sqlr .= "ps.pid=p.id and ps.sid=sc.id and ";
  if (strlen($comments) > 0) $sqlr .= "c.pid=p.id and (c.public=1 or c.owner=$uid) and ";
  if (strlen($attach) > 0)   $sqlr .= "p.id=pa.pid and a.id=pa.aid and ";

  $nc = 0;
  $hrb = '';
  if (strlen($pat)>0) { 
    $sqlr .= "p.name ~* '.*${pat}.*'"; 
    $nc++; $hrb .= "Nome($pat),"; 
  }
  if (strlen($patcode)>0) {
    if ($nc>0) $sqlr .= " and ";
    $sqlr .= "p.patcode ~* '.*${patcode}.*'";
    $nc++; $hrb .= "HC($patcode),";
  }
  if (strlen($age)>0) {
    if (preg_match("/([0-9]+)-([0-9]+)/",$age,$rage)) {
      if ($nc>0) $sqlr .= " and ";
      $sqlr .= "p.age >= $rage[1] and p.age <= $rage[2]";
      $hrb .= "Idade(${rage[1]}-$rage[2]),";
      $nc++;
    } elseif (preg_match("/([0-9]+)/",$age,$rage)) {
      if ($nc>0) $sqlr .= " and ";
      $sqlr .= "p.age = $rage[1]";
      $hrb .= "Idade(${rage[1]}),";
      $nc++;
    }
  }
  switch($gender) {
  case 2: if ($nc>0) $sqlr .= " and "; $sqlr .= "(p.gender = 'M' or p.gender = 'F')"; $nc++; 
    $hrb .= "Sexo(MF),";
    break;
  case 3: if ($nc>0) $sqlr .= " and "; $sqlr .= "p.gender = 'M'"; $nc++; 
    $hrb .= "Sexo(M),";
    break;
  case 4: if ($nc>0) $sqlr .= " and "; $sqlr .= "p.gender = 'F'"; $nc++; 
    $hrb .= "Sexo(F),";
    break;
  case 5: if ($nc>0) $sqlr .= " and "; $sqlr .= "p.gender = 'O'"; $nc++;
    $hrb .= "Sexo(O),";
    break;
  }
  if (strlen($scanner)>0) { if ($nc>0) $sqlr .= " and ";
    $sqlr .= "(sc.maker || ' ' || sc.model || ' ' || sc.location) ~* '.*$scanner.*'";
    $nc++;
    $hrb .= "Scanner($scanner),";
  }
  if (strlen($comments)>0)  { if ($nc>0) $sqlr .= " and "; 
    $rcom = preg_split("/,/",$comments);
    for($i=0;$i<count($rcom);$i++) {
      $nc++;
      if ($i > 0) $sqlr .= " and ";
      $sqlr .= "c.text ~* '.*${rcom[$i]}.*'";
    }
    $hrb .= "Comentários($comments),";
  }
  // search attachment descriptions
  if (strlen($attach)>0)  { if ($nc>0) $sqlr .= " and "; 
    $ratt = preg_split("/,/",$attach);
    for($i=0;$i<count($ratt);$i++) {
      $nc++;
      if ($i > 0) $sqlr .= " and ";
      $sqlr .= "a.description ~* '.*${ratt[$i]}.*'";
    }
    $hrb .= "Anexos($attach),";
  }

  if ($nc == 0) { $sqlr = ' '; $hrb = "Sem restrições"; }
  
  $hrb = preg_replace("/,$/","",$hrb);

  // calc paging
  $q = pg_query($sql2 . $sqlr . ";");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
  $total = pg_fetch_result($q,0,0);
  $offset = ($page - 1) * 200;
  $first  = 1 + $offset;
  $last   = $first + 199;
  if ($last > $total) $last = $total;
  $npages = floor($total/200);
  if ($total % 200 != 0) $npages++;
  if ($total == 0) $first = 0;

  // sorting
  $sp = '';
  if ($sort == 2) { $sp = 'p.patcode,'; }
  if ($sort == 3) { $sp = 'p.gender,p.age,'; }
  
  if (!$print)
    $sql3 = " order by ${sp} p.name limit 200 offset $offset;";
  else
    $sql3 = " order by ${sp} p.name;";

  $ostr = array('','Nome do paciente','HC','Idade/Sexo');
  $op = $ostr[ $sort % 4 ];

  // the left form
  if (!$print) {
    print "<div class=\"fixleft2\">";  
    print "<form method=POST action=\"pacientes.php\">\n";
    print "<font size=1><b>Nome:</b></font><br> <input type=text name=\"patient\" value=\"$pat\" size=15><br>\n";
    print "<font size=1><b>HC/Pat-ID:</b></font><br> <input type=text name=\"patcode\" value=\"$patcode\" size=15><br>\n";
    print "<font size=1><b>Idade:</b></font><br> <input type=text name=\"age\" value=\"$age\" size=10 title=\"Exemplos: 30, 40-50\"><br><br>\n";
    print "<font size=1><b>Sexo:</b></font> <select name=\"gender\">\n";
    
    $genders = array("X","Qualquer","M ou F","M","F","O");
    for($i=1;$i<=5;$i++) {
      print "<option ";
      if ($gender==$i) print "selected ";
      print "value=\"$i\">$genders[$i]</option>\n";
    }
    print "</select><br><br>\n";
    print "<font size=1><b>Scanner:</b></font><br> <input type=text name=\"scanner\" value=\"$scanner\" size=15><br>\n";
    print "<font size=1><b>Comentários:</b></font><br> <input type=text name=\"comments\" value=\"$comments\" size=15 title=\"Formato: valor[,valor,...]\"><br>\n";
    print "<font size=1><b>Anexos (descrição):</b></font><br> <input type=text name=\"attach\" value=\"$attach\" size=15 title=\"Formato: valor[,valor,...]\"><br>\n";
    print "<br>\n";
    print "<input type=submit value=\"Atualizar\"></form><br>\n";
    
    print "<form method=POST action=\"pacientes.php\">\n";  
    print "<input type=hidden name=\"patient\" value=\"\">\n";
    print "<input type=hidden name=\"patcode\" value=\"\">\n";
    print "<input type=hidden name=\"age\" value=\"\">\n";
    print "<input type=hidden name=\"gender\" value=\"1\">\n";
    print "<input type=hidden name=\"scanner\" value=\"\">\n";
    print "<input type=hidden name=\"comments\" value=\"\">\n";
    print "<input type=hidden name=\"attach\" value=\"\">\n";
    print "<input type=hidden name=\"sort\" value=\"1\">\n";
    print "<input type=submit value=\"Limpar\">\n";
    print "</form></div>\n";
  }

  // patient distribution
  if (!$print) {
    readfile("pac_distribution.js");
    print "<div class=\"histoctl\">Distribuição <img id=\"distarrow\" title=\"Exibir Distribuição\" align=right src=\"bright.png\" onClick=\"javascript:distclick();\"></div>\n";
    print "<div id=\"distdiv\" class=\"fixedhisto\"><center><b>Distribução</b><br><small>(Idade e Sexo)</small></center>\n";
    pacientes_distribution($sql1 . $sqlr . ';');
    print "</div>";
  }
  
  // main list
  if (!$print)
    print "<div class=\"regular2\">\n";
  else
    print "<div class=\"print\"><b>LNIDB - Pacientes</b><br>\n";

  print "Consulta realizada: <font color=#008800><i>$hrb</i></font>, ";
  print "ordenada por <font color=#008800><i>$op</i></font>\n";

  if (!$print)  {
    print "<div class=\"pbutton\"><a target=\"_blank\" href=\"$link&print=1\"><img src=\"print24.png\" border=0 title=\"Versão para Impressão\"></a></div><br>\n";
    page_selector($page, $npages, $total, $link);
  } else {
    print "<br>$total resultado(s).<br><br>\n";
  }
  
  print "<table cellpadding=2 cellspacing=2><tr class=title><td class=s>#</td>\n";

  if (!$print) {
    print "<td class=s><a class=w href=\"$link&s=1\">Paciente</a> (<a class=w href=\"$link&s=2\">HC</a>)</td>\n";
    print "<td class=s><a class=w href=\"$link&s=3\">Idade/Sexo</a></td>\n";
  } else {
    print "<td class=s>Paciente (HC)</td>\n";
    print "<td class=s>Idade/Sexo</td>\n";
  }

  print "<td class=s>Período</td>\n";
  print "<td class=s>Scanners</td>\n";
  print "<td class=s>Comentários</td>\n";
  print "<td class=s>Anexos</td></tr>\n";
  
  $q = pg_query($sql1 . $sqlr . $sql3);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
  $n = pg_num_rows($q);
  
  for($i=0;$i<$n;$i++) {
    $nr = $i + 1 + $offset;
    $row = pg_fetch_row($q,$i);
    if (!$print)
      print "<tr class=\"" . ($i%2 == 0 ? "even" : "odd") . "\">\n";
    else
      print "<tr class=\"" . ($i%2 == 0 ? "even2" : "odd2") . "\">\n";
    print "<td class=s align=right>$nr</td><td class=s>$row[1] ";
    if (!$print)
      print "<a title=\"Detalhes\" href=\"$link&v=$row[0]&e=0\"><img border=0 src=\"bdetail16.png\"></a> ";
    print "($row[2])</td>\n";
    print "<td class=s align=right>$row[3]/$row[4]</td>\n";

    // datas
    print "<td class=s>";

    $q3 = pg_query("select to_char(pp.first, 'YYYY-MM-DD'), to_char(pp.last, 'YYYY-MM-DD') from patperiod pp where pp.pid=$row[0];");
    if ($q3==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    if (pg_num_rows($q3) == 0) {
       compute_patperiod($row[0]);
       $q3 = pg_query("select to_char(pp.first, 'YYYY-MM-DD'), to_char(pp.last, 'YYYY-MM-DD') from patperiod pp where pp.pid=$row[0];");
       if ($q3==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    }
    
    if (pg_num_rows($q3) == 0) {
       $d1 = 'n/a';
    } else {
       $d1 = pg_fetch_result($q3,0,0);
       $d2 = pg_fetch_result($q3,0,1);
       if ($d1 !== $d2) { $d1 = "$d1 a $d2"; }
    }

    print "$d1</td>\n";
    
    // scanners
    print "<td class=s>";
    $q2 = pg_query("select s.id, s.maker, s.model, s.location from scanners s, patscanners ps where ps.pid=$row[0] and ps.sid=s.id order by s.maker,s.model,s.location;");
    if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $n2 = pg_num_rows($q2);
    
    for($j=0;$j<$n2;$j++) {
      $row2 = pg_fetch_row($q2,$j);
      print "$row2[1] $row2[2] $row2[3]<br>";
    }
    print "</td>\n";
    
    // comments
    print "<td class=s>";
    
    $sql = "select c.text, c.created from comments c where c.pid=$row[0] and (c.public=1 or c.owner=$uid) union ";
    $sql .= "select c.text, c.created from comments c, cvisible v where c.pid=$row[0] and c.public=0 and c.owner<>$uid and v.cid=c.id and v.uid=$uid ";
    $sql .= "order by 2;";

    $q2 = pg_query($sql);
    if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $n2 = pg_num_rows($q2);
    
    for($j=0;$j<$n2;$j++) {
      $row2 = pg_fetch_row($q2,$j);
      print "$row2[0] &#9642;<br>";
    }
    print "</td>\n";

    // attachs
    $na = scalar_query("select count(aid) from patattachs where pid=$row[0];");
    print "<td align=right>";
    if ($na == 0) print "<font color=red>";
    print "$na";
    if ($na == 0) print "</font>";
    print "</td>";

    print "</tr>\n";

  }

  print "</table>\n";
  print "</div>\n";
}

function pacientes_distribution($sql) {
  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");  
  $n = pg_num_rows($q);

  $male  = array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
  $fem   = array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
  $other = 0;

  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q,$i);
    if ($row[4] == 'O' || $row[3] < 0) {
      $other++;
      continue;
    }
    $j = intval($row[3] / 5);
    if ($j>=20) $j=19;
    if ($row[4] == 'M')
      $male[$j]++;
    else
      $fem[$j]++;
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
  print "<table border=0 cellpadding=0 cellspacing=0>\n";
  
  for($i=0;$i<20;$i++) {
    $l1 = 5*$i;
    $l2 = $l1 + 4;
    if ($i==19) $range= "${l1}+"; else $range = "${l1}-${l2}";

    print "<tr valign=center><td align=right>";
    if ($m1==0) $h=0; else $h = intval( (50.0*$male[$i])/$m1 );
    print "$male[$i]&nbsp;<img src=\"blue.png\" width=$h height=10></td>";
    print "<td align=center><font size=\"1\">&nbsp;${range}&nbsp;</font></td>";
    if ($m1==0) $h=0; else $h = intval( (50.0*$fem[$i])/$m1 );
    print "<td align=left><img src=\"pink.png\" width=$h height=10>&nbsp;$fem[$i]</td></tr>\n";
  }
  
  print("<tr><td colspan=2><img src=\"blue.png\" width=10 height=10> Homens:</td><td align=right>$m3</td></tr>\n");
  print("<tr><td colspan=2><img src=\"pink.png\" width=10 height=10> Mulheres:</td><td align=right>$m4</td></tr>\n");
  print("<tr><td colspan=2>Outros:</td><td align=right>$other</td></tr>\n");
  print("<tr><td colspan=2><b>Total:</b></td><td align=right>$total</td></tr>\n");  
  print("</table>\n");
}

?>
