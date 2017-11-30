<?php

$date     = '';
$pat      = '';
$patcode  = '';
$age      = '';
$gender   = '1';
$scanner  = '';
$exam     = '';
$storage  = '';
$series   = '';
$artifact = '1';
$sort = 1;
$page = 1;

function estudos_parse_session() {
  global $_GET, $_SESSION;
  global $pat, $patcode, $age, $gender, $scanner, $exam, $series, $storage, $sort, $page, $date, $artifact;
  
  if (isset($_SESSION['s.date']))     { $date     = $_SESSION['s.date']; }
  if (isset($_SESSION['s.patient']))  { $pat      = $_SESSION['s.patient']; }
  if (isset($_SESSION['s.patcode']))  { $patcode  = $_SESSION['s.patcode']; }
  if (isset($_SESSION['s.age']))      { $age      = $_SESSION['s.age']; }
  if (isset($_SESSION['s.gender']))   { $gender   = $_SESSION['s.gender']; }
  if (isset($_SESSION['s.sort']))     { $sort     = $_SESSION['s.sort']; }
  if (isset($_SESSION['s.scanner']))  { $scanner  = $_SESSION['s.scanner']; }
  if (isset($_SESSION['s.exam']))     { $exam     = $_SESSION['s.exam']; }
  if (isset($_SESSION['s.series']))   { $series   = $_SESSION['s.series']; }
  if (isset($_SESSION['s.storage']))  { $storage  = $_SESSION['s.storage']; }
  if (isset($_SESSION['s.artifact'])) { $artifact = $_SESSION['s.artifact']; }

  if (isset($_GET['p'])) { $page = $_GET['p']; } else { $page = 1; }
  if (isset($_GET['s'])) { $sort = $_GET['s']; $_SESSION['s.sort'] = $sort; }
}

function estudos_main() {
  global $_GET, $_SESSION;
  global $pat, $patcode, $age, $gender, $scanner, $exam, $series, $storage, $sort, $page, $date, $artifact;
  $pagesize = 50;

  $tstart = microtime(true);

  estudos_parse_session();
  if (isset($_SESSION['userid']))
    $uid = $_SESSION['userid'];
  else
    $uid = -1;

  if (isset($_GET['add']))
    $tobasket = $_GET['add'];
  else
    $tobasket = 0;

  $print = 0;
  if (isset($_GET['print'])) $print=1;

  // build query
  // TODO: union para comentarios compartilhados
  $sql1  = "select distinct s.id,p.name,p.patcode,p.age,p.gender,"; // 0-4
  $sql1 .= "to_char(s.scantime,'YYYY-MM-DD HH24:MI'),s.exam_desc,s.series_desc,s.storage,s.files,"; // 5-9
  $sql1 .= "s.cols,s.rows,s.slices,round((s.xspacing)::numeric,2),round((s.yspacing)::numeric,2),"; // 10-14
  $sql1 .= "round((s.zspacing)::numeric,2),round((s.thickness)::numeric,2),sc.maker,sc.model,sc.location,"; // 15-19
  $sql1 .= "p.id,s.scantime,s.bytes,s.oaxi,s.osag,s.ocor,s.artifact "; // 20-22, 23-25, 26
  $sql1 .= "from studies s, patstudies ps, patients p, scanners sc";

  $sql2  = "select count(distinct s.id) ";
  $sql2 .= "from studies s, patstudies ps, patients p, scanners sc";

  $sql1tobasket = "insert into basket (uid,fid) select distinct $uid,fs.fid from studies s, patstudies ps, patients p, scanners sc,filestudies fs";

  $sqlr  = " where ";
  $sqlr .= "ps.pid=p.id and ps.sid=s.id and sc.id=s.scanner and ";

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
  if (strlen($date)>0) {
    if (preg_match("/([0-9]{4}-[0-9]{2}-[0-9]{2})\\s+([0-9]{4}-[0-9]{2}-[0-9]{2})/",$date,$rdate)) {
      if ($nc>0) $sqlr .= " and ";
      $sqlr .= "s.scantime >= '$rdate[1] 00:00:00' and scantime <= '$rdate[2] 23:59:59'";
      $nc++;
      $hrb .= "Data(${rdate[1]}-$rdate[2]),";
    } elseif (preg_match("/([0-9]{4}-[0-9]{2}-[0-9]{2})/",$date,$rdate)) {
      if ($nc>0) $sqlr .= " and ";
      $sqlr .= "s.scantime >= '$rdate[1] 00:00:00' and scantime <= '$rdate[1] 23:59:59'";
      $hrb .= "Data(${rdate[1]}),";
      $nc++;
    }
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
  if (strlen($exam)>0) { if ($nc>0) $sqlr .= " and ";
    $sqlr .= "s.exam_desc ~* '$exam'";
    $nc++;
    $hrb .= "Exame($exam),";
  }
  if (strlen($series)>0) { if ($nc>0) $sqlr .= " and ";
    $sqlr .= "s.series_desc ~* '$series'";
    $nc++;
    $hrb .= "Série($series),";
  }
  if (strlen($storage)>0) { if ($nc>0) $sqlr .= " and ";
    $sqlr .= "s.storage ~* '$storage'";
    $nc++;
    $hrb .= "Disco($storage),";
  }
  switch($artifact) {
  case 2: if ($nc>0) $sqlr .= " and "; $sqlr .= "(s.artifact=1)"; $nc++; 
    $hrb .= "Qualidade(boa),";
    break;
  case 3: if ($nc>0) $sqlr .= " and "; $sqlr .= "(s.artifact=0)"; $nc++; 
    $hrb .= "Qualidade(sem info),";
    break;
  case 4: if ($nc>0) $sqlr .= " and "; $sqlr .= "(s.artifact=2)"; $nc++; 
    $hrb .= "Qualidade(artefato menor),";
    break;
  case 5: if ($nc>0) $sqlr .= " and "; $sqlr .= "(s.artifact=3)"; $nc++; 
    $hrb .= "Qualidade(artefato sério),";
    break;
  case 6: if ($nc>0) $sqlr .= " and "; $sqlr .= "(s.artifact=2 or s.artifact=3)"; $nc++; 
    $hrb .= "Qualidade(algum artefato),";
    break;
  case 7: if ($nc>0) $sqlr .= " and "; $sqlr .= "(s.artifact=0 or s.artifact=1)"; $nc++; 
    $hrb .= "Qualidade(boa ou sem info),";
    break;
  case 8: if ($nc>0) $sqlr .= " and "; $sqlr .= "(s.artifact<>3)"; $nc++; 
    $hrb .= "Qualidade(sem artefato sério),";
    break;
  }

  if ($nc == 0) { 
    if (preg_match('/(.*)\\s+and\\s*$/',$sqlr,$tmp))
      $sqlr = $tmp[1];
    $hrb = "Sem restrições"; 
  }
  
  $hrb = preg_replace("/,$/","",$hrb);

  // calc paging
  $tq1 = microtime(true);
  $q = pg_query($sql2 . $sqlr . ";");
  $tq1 = microtime(true) - $tq1;
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
  $total = pg_fetch_result($q,0,0);
  $offset = ($page - 1) * $pagesize;
  $first  = 1 + $offset;
  $last   = $first + $pagesize - 1;
  if ($last > $total) $last = $total;
  $npages = floor($total/$pagesize);
  if ($total % $pagesize != 0) $npages++;
  if ($total == 0) $first = 0;

  // sorting
  $sp = '';
  if ($sort == 2) { $sp = 'p.name,'; }
  if ($sort == 3) { $sp = 'p.patcode,'; }
  if ($sort == 4) { $sp = 'p.gender,p.age,'; }
  if ($sort == 5) { $sp = 's.exam_desc,'; }
  if ($sort == 6) { $sp = 's.series_desc,'; }
  if ($sort == 7) { $sp = 's.storage,'; }
  if ($sort == 8) { $sp = 'sc.model,'; }

  if (!$print)
    $sql3 = " order by ${sp} s.scantime limit $pagesize offset $offset;";
  else
    $sql3 = " order by ${sp} s.scantime;";

  $ostr = array('','Data','Nome do paciente','HC','Idade/Sexo','Exame','Série','Disco','Scanner');
  $op = $ostr[ $sort % 9 ];

  print "<script language=\"JavaScript\" src=\"deployJava.js\"></script>\n";
  print "<script language=\"JavaScript\" src=\"diva.js\"></script>\n";

  // the left form
  if (!$print) {
    print "<div class=\"fixleft1\">";  
    print "<form method=POST action=\"estudos.php\">\n";
    print "<font size=1><b>Nome:</b></font><br> <input type=text name=\"patient\" value=\"$pat\" size=15><br>\n";
    print "<font size=1><b>HC/Pat-ID:</b></font><br> <input type=text name=\"patcode\" value=\"$patcode\" size=15><br>\n";
    print "<font size=1><b>Idade:</b></font><br> <input type=text name=\"age\" value=\"$age\" size=10 title=\"Exemplos: 30, 40-50\"><br>\n";
    print "<font size=1><b>Data:</b></font><br> <input type=text name=\"date\" value=\"$date\" size=15 title=\"Formato: AAAA-MM-DD [AAAA-MM-DD]\"><br><br>\n";
    print "<font size=1><b>Sexo:</b></font> <select name=\"gender\">\n";
    $genders = array("X","Qualquer","M ou F","M","F","O");
    for($i=1;$i<=5;$i++) {
      print "<option ";
      if ($gender==$i) print "selected ";
      print "value=\"$i\">$genders[$i]</option>\n";
    }
    print "</select><br><br>\n";
    
    print "<font size=1><b>Exame:</b></font><br> <input type=text name=\"exam\" value=\"$exam\" size=15><br>\n";
    print "<font size=1><b>Série:</b></font><br> <input type=text name=\"series\" value=\"$series\" size=15><br>\n";
    print "<font size=1><b>Disco:</b></font><br> <input type=text name=\"storage\" value=\"$storage\" size=15><br>\n";
    
    print "<font size=1><b>Scanner:</b></font><br> <input type=text name=\"scanner\" value=\"$scanner\" size=15><br>\n";

    print "<font size=1><b>Qualidade:</b></font> <select name=\"artifact\">\n";
    $artvals = array("X","Qualquer","Boa","Sem info","Artefato menor","Artefato sério", "Algum artefato", "Boa ou sem info", "Sem artefato sério");
    for($i=1;$i<=8;$i++) {
      print "<option ";
      if ($artifact==$i) print "selected ";
      print "value=\"$i\">$artvals[$i]</option>\n";
    }
    print "</select><br><br>\n";

    print "<br>\n";
    print "<input type=submit value=\"Atualizar\"></form><br>\n";
    
    print "<form method=POST action=\"estudos.php\">\n";  
    print "<input type=hidden name=\"patient\" value=\"\">\n";
    print "<input type=hidden name=\"patcode\" value=\"\">\n";
    print "<input type=hidden name=\"age\" value=\"\">\n";
    print "<input type=hidden name=\"date\" value=\"\">\n";
    print "<input type=hidden name=\"gender\" value=\"1\">\n";
    print "<input type=hidden name=\"exam\" value=\"\">\n";
    print "<input type=hidden name=\"series\" value=\"\">\n";
    print "<input type=hidden name=\"storage\" value=\"\">\n";
    print "<input type=hidden name=\"scanner\" value=\"\">\n";
    print "<input type=hidden name=\"artifact\" value=\"1\">\n";
    print "<input type=hidden name=\"sort\" value=\"1\">\n";
    print "<input type=submit value=\"Limpar\">\n";
    print "</form></div>\n";
  }

  // diva div
  print "<div id=\"divabox\" class=\"fixdiva\"></div>\n";
  print "<div id=\"nojavabox\" class=\"fixnojava\"></div>\n";

  // study distribution (todo)

  // main list
  if (!$print)
    print "<div class=\"regular2\">\n";
  else
    print "<div class=\"print\"><b>LNIDB - Estudos</b><br>\n";

  print "Consulta realizada: <font color=#008800><i>$hrb</i></font>, ";
  print "ordenada por <font color=#008800><i>$op</i></font>\n";

  $link  = menu_link('est');
  $linkp = menu_link('pac');

  if (!$print) {
    print "<div class=\"pbutton\"><a target=\"_blank\" href=\"$link&print=1\"><img src=\"print24.png\" border=0 title=\"Versão para Impressão\"></a></div>";
    if ($uid >= 0)
      print "<div class=\"pbutton\"><a href=\"$link&add=1\"><img src=\"badd24.png\" border=0 title=\"Adicionar Todos os Resultados à Cesta\"></a></div>";
    print "<br>\n";
	page_selector($page, $npages, $total, menu_link('est'));
  } else {
    print "<br>$total resultado(s).<br><br>\n";
  }
  
  print "<table cellpadding=2 cellspacing=2><tr class=title><td class=s>#</td>\n";

  if (!$print) {
    print "<td class=s><a class=w href=\"$link&s=1\">Data</a></td>\n";
    print "<td class=s><a class=w href=\"$link&s=2\">Paciente</a> (<a class=w href=\"$link&s=3\">HC</a>)</td>\n";
    print "<td class=s><a class=w href=\"$link&s=4\">Idade/Sexo</a></td>\n";
    print "<td class=s><a class=w href=\"$link&s=5\">Exame</a> / ";
    print "<a class=w href=\"$link&s=6\">Série</a></td>\n";
    print "<td class=s>Dimensões</td>\n";
    print "<td class=s><a class=w href=\"$link&s=7\">Disco</a>/Tamanho</td>\n";
    print "<td class=s><a class=w href=\"$link&s=8\">Scanner</a></td>\n";
    print "<td class=s>Qualidade</td>\n";
    print "<td class=s>Comentários</td></tr>\n";
  } else {
    print "<td class=s>Data</td>\n";
    print "<td class=s>Paciente (HC)</td>\n";
    print "<td class=s>Idade/Sexo</td>\n";
    print "<td class=s>Exame / ";
    print "Série</td>\n";
    print "<td class=s>Dimensões</td>\n";
    print "<td class=s>Disco/Tamanho</td>\n";
    print "<td class=s>Scanner</td>\n";
    print "<td class=s>Qualidade</td>\n";
    print "<td class=s>Comentários</td></tr>\n";
  }

  // add all to basket

  if ($uid >= 0 && $tobasket > 0) {
    $sqlrtobasket = $sqlr . " and fs.sid = s.id";
    $sql3tobasket = ";";

    $bq = pg_query($sql1tobasket . $sqlrtobasket . $sql3tobasket);
    if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $bn = pg_affected_rows($bq);
    print "<script language=\"JavaScript\">history.go(-1);</script>\n";
    exit(0);
  }

  //  print "</table> Query=[" . $sql1 . $sqlr . $sql3 . "]<br></div>";
  //  return;

  $tq2 = microtime(true);
  $q = pg_query($sql1 . $sqlr . $sql3);
  $tq2 = microtime(true) - $tq2;
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
  $n = pg_num_rows($q);

  $tq3 = microtime(true);
  if ($uid > 0) {
    $bsql = "select distinct fs.sid from filestudies fs, basket b where b.uid=$uid and b.fid=fs.fid;";
    $bq = pg_query($bsql);
    $bsid = pg_fetch_all_columns($bq,0);
  }
  $tq3 = microtime(true) - $tq3;

  for($i=0;$i<$n;$i++) {
    $nr = $i + 1 + $offset;
    $row = pg_fetch_row($q,$i);

    $nfiles = $row[9];

    if (!$print)
      print "<tr class=\"" . ($i%2 == 0 ? "even" : "odd") . "\">\n";
    else
      print "<tr class=\"" . ($i%2 == 0 ? "even2" : "odd2") . "\">\n";
    print "<td class=s align=right>$nr</td>";
    print "<td class=s>$row[5]</td>";
    print "<td class=s>$row[1] ($row[2])";
    if (!$print) {
      print "<br><a title=\"Detalhes do Paciente\" href=\"$linkp&v=$row[20]&e=0\"><img border=0 src=\"bdetail16.png\"></a>\n";    
    
      print "<a href=\"javascript:void(0)\"><img border=0 onClick=\"javascript:diva($row[0])\" title=\"Visualizar Estudo\" src=\"bview.png\"></a>&nbsp;";
      //print "<a target=\"_blank\" href=\"view.php?s=$row[0]\"><img border=0 title=\"Visualizar Estudo\" src=\"bview.png\"></a>&nbsp;";
      if ($nfiles == 1 && $uid > 0) {
	print "<a href=\"getfile.php?s=$row[0]\"><img border=0 title=\"Download\" src=\"bdownload.png\"></a>&nbsp;";
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
	  print "<a title=\"Remover Estudo da Cesta\" href=\"basket.php?o=2&s=$row[0]\"><img border=0 src=\"bminus.png\"></a>\n";
	  print "<img border=0 src=\"baddd.png\">\n";
	} else {
	  print "<img border=0 src=\"bminusd.png\">\n";
	  print "<a title=\"Adicionar Estudo à Cesta\" href=\"basket.php?o=1&s=$row[0]\"><img border=0 src=\"badd.png\"></a>\n";
	}

      }
    }

    print "</td>";
    print "<td class=s align=right>$row[3]/$row[4]</td>\n";
    if (!$print)
      print "<td class=s>$row[6]<br>$row[7]</td>\n";
    else
      print "<td class=s>$row[6] / $row[7]</td>\n";

    $osum = $row[23] + $row[24] + $row[25];
    $ori = '';

    if ($row[23] != 0) { $ori .= "Ax/"; }
    if ($row[24] != 0) { $ori .= "Sag/"; }
    if ($row[25] != 0) { $ori .= "Cor/"; }

    if ($osum > 0) {
      $ori = substr($ori,0,-1);
    } else {
      $ori = 'n/a';
    }

    if (!$print)
      $dim = "<font color=#880088>$ori</font> ${row[10]}x${row[11]}x${row[12]}<br>${row[13]}x${row[14]}x${row[15]} mm<br>thickness ${row[16]} mm";
    else
      $dim = "$ori ${row[10]}x${row[11]}x${row[12]}, ${row[13]}x${row[14]}x${row[15]} mm<br>, thk ${row[16]} mm";
    print "<td class=s>$dim</td>\n";

    $tam = human_size($row[22]);

    if (!$print)
      print "<td class=s>$row[8]<br>$nfiles arquivo(s) ($tam)</td>";
    else
      print "<td class=s>$row[8], $nfiles arquivo(s) ($tam)</td>";
    print "<td class=s>$row[17] $row[18] $row[19]</td>";

    // qualidade / artefatos
    print "<td class=s align=center>" . artifact_icon($row[26]) . "</td>";

    // comments

    // comentarios publicos e proprios (no paciente)
    $q2 = pg_query("select count(id) from comments where pid=$row[20] and (public=1 or owner=$uid);");
    if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $r2 = pg_fetch_row($q2,0);
    $ncom = $r2[0];

    // comentarios compartilhados
    $q2 = pg_query("select count(c.id) from comments c, cvisible v where c.id=v.cid and c.public=0 and c.owner<>$uid and v.uid=$uid;");
    if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"index.php");
    $r2 = pg_fetch_row($q2,0);
    $ncom += $r2[0];

    print "<td align=right class=s>$ncom</td>";
    print "</tr>";
  }

  print "</table>";

  $tstart = microtime(true) - $tstart;
  $tstart = number_format($tstart, 4, '.', ' ');
  $tq1 = number_format($tq1, 4, '.', ' ');
  $tq2 = number_format($tq2, 4, '.', ' ');
  $tq3 = number_format($tq3, 4, '.', ' ');

  print "<span class=\"debuginfo\">Tempo de consulta: $tstart (q1=$tq1, q2=$tq2, q3=$tq3)</span><br>";

  print "</div>\n";


}


?>