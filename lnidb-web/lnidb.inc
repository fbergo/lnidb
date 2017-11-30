<?php

$html_done = 0;

function page_start($title) {
  global $html_done;
  print "<!DOCTYPE HTML>\n<html><head><title>$title</title>\n";
  print "<link rel=\"stylesheet\" type=\"text/css\" href=\"lnidb.css\">\n";
  print "<link rel=\"icon\" type=\"image/png\" href=\"lnidb_icon32.png\"></head>\n";
  print "<body OnLoad=\"formfocus();\">\n";
  print "<script type=\"text/javascript\">\n  function formfocus(){\n";
  print "  var forms = document.forms || [];\n";
  print "  for(var i = 0; i < forms.length; i++){\n";
  print "  for(var j = 0; j < forms[i].length; j++){\n";
  print "  if(!forms[i][j].readonly != undefined && forms[i][j].type != \"hidden\" && forms[i][j].disabled != true && forms[i][j].style.display != 'none'){\n";
  print "forms[i][j].focus(); return; } } } }\n</script>\n";

  $html_done = 1;
}

function checkbox_value($x) {
  return( ($x == 'on') ? 1 : 0 );
}

function scalar_query($sql) {
  $q = pg_query($sql);
  if ($q==FALSE) return -1;
  if (pg_num_rows($q)==0) return -2;
  $row = pg_fetch_row($q,0);
  return($row[0]);
}

function menu_link($what) {
  $menu   = array("Pacientes","Estudos","Grupos","Discos","Cesta","Utilidades","Ajuda");
  $smenu  = array("pac","est","gru","dis","ces","uti","aju");
  for($i=0;$i<7;$i++)
    if ($what == $menu[$i] || $what == $smenu[$i])
      return ("index.php?m=$i");
  return("index.php");
}

function page_top($showmenu=1, $showuser=1) {
  global $html_done, $_SESSION;
  $menu  = array("Pacientes","Estudos","Grupos","Discos","Cesta","Utilidades","Ajuda");
  $level = array(-1,-1,0,-1,0,0,-1);
  print "<div class=\"fixed1\"><table class=\"top\">\n";
  print "<tr class=\"top\"><td class=\"top\"><img src=\"lnidb-200.png\"></td>\n";
  print "<td width=\"100%\"></td>\n";

  $mylevel = -1;
  if (isset($_SESSION['admlevel'])) {
    $mylevel = $_SESSION['admlevel'];
  }
  $selection = 0;
  if (isset($_SESSION['prevmenu']))
    $selection = $_SESSION['prevmenu'];
  else
    $_SESSION['prevmenu'] = $selection;

  if ($level[$selection] > $mylevel) {
    $selection = 0;
    $_SESSION['prevmenu'] = $selection;
  }
 
  if ($showmenu) {
    if (isset($_SESSION['userid'])) {
      $uid = $_SESSION['userid'];
      $nb = 0;
      $nb = scalar_query("select count(fid) from basket where uid=$uid;") + 
	scalar_query("select count(aid) from basket2 where uid=$uid;");
      $np = scalar_query("select count(tid) from wrapped where owner=$uid;");
      $menu[4] = "Cesta&nbsp;(<span id=\"numcesta\">$nb</span>+<span id=\"numpacotes\">$np</span>)";

    }

    $n = count($menu);
    for($i=0;$i<$n;$i++) {
      if ($mylevel < $level[$i]) continue;
      if ($i==$selection) { $class="top1"; } else { $class="top0"; }
      print "<td class=\"$class\"><a class=\"top\" href=\"index.php?m=$i\">$menu[$i]</a></td>\n";
    }
  }

  print "</tr></table>\n";

  print "<table class=\"top2\"><tr class=\"top2\">\n";
  print "<td class=\"top2x\">Sistema LNIDB &copy; 2010-2013 Laboratório de Neuroimagem - Depto. de Neurologia - FCM - Unicamp</td>\n";
  print "<td class=\"top2\">\n";

  if ($showuser) {
    if (isset($_SESSION['username'])) {
      $username = $_SESSION['username'];
      $fullname = $_SESSION['fullname'];
      print "Logado como <b>$username</b> ($fullname) ";
      print "<a class=\"top2r\" href=\"logout.php\">Deslogar</a>";
    } else {
      print "Você não está logado. ";
      print "<a class=\"top2g\" href=\"login.php\">Logar</a>";
    }
  }

  print "</td></tr></table></div>\n";

  // task ticker
  if ($showmenu && isset($_SESSION['userid'])) {
    $uid = $_SESSION['userid'];
    print "<script language=\"JavaScript\" src=\"ticker.js\"></script>\n";
    print "<div class=\"tickerarrow\" id=\"tickerarrow\"><img src=\"aup9.png\" border=\"0\" onclick=\"javascript:flipTicker()\"></div>";
    print "<div class=\"fixticker\" id=\"ticker\"></div>\n";
    print "<script language=\"JavaScript\">updateTicker($uid);</script>\n";
  }

  $html_done = 2;
}

function page_end() {
  global $html_done;
  print "</body></html>\n";
  $html_done = 4;
}

function fatal_error($msg, $dest='') {
  global $html_done;

  if ($html_done < 1) page_start("LNI DB");
  if ($html_done < 2) page_top(0,0);
  if ($html_done == 2) print "</table></div></table></div>\n";

  if ($html_done < 4) {
    print "<div class=\"darkbg\"></div>\n";
    print "<div class=\"erro\"><h2>Erro</h2>$msg<br><br>\n";
    print "<p align=right><img src=\"null.png\" width=80 height=1>\n";

    if (strlen($dest)>0) 
      $code = "window.location.href='$dest'";
    else
      $code = "history.go(0)";
    if ($dest == "back")
      $code = "history.go(-1)";
    if ($dest == "back2")
      $code = "history.go(-2)";

    print "<input type=\"button\" value=\"Continuar\" onClick=\"$code\">\n";
    print "<input type=\"button\" value=\"Limpar Estado\" onClick=\"window.location.href='index.php?reset=1'\">\n";
    print "</p></div>\n";
    page_end();
    exit(0);
  }
}

function notify_user($msg, $dest='') {
  global $html_done;

  if ($html_done < 1) page_start("LNI DB");
  if ($html_done < 2) page_top(0,0);

  if ($html_done < 4) {
    print "<div class=\"darkbg\"></div>\n";
    print "<div class=\"notify\"><h2>Notificação</h2>$msg<br><br>\n";
    print "<p align=right><img src=\"null.png\" width=80 height=1>\n";

    if (strlen($dest)>0) 
      $code = "window.location.href='$dest'";
    else
      $code = "history.go(0)";
    if ($dest == "back")
      $code = "history.go(-1)";
    if ($dest == "back2")
      $code = "history.go(-2)";

    print "<input type=\"button\" value=\"Continuar\" onClick=\"$code\"></p></div>\n";
    page_end();
    exit(0);
  }
}

function ask_confirmation($msg, $destyes, $destno) {
  global $html_done;

  if ($html_done < 1) page_start("LNI DB");
  if ($html_done < 2) page_top(0,0);

  if ($html_done < 4) {
    print "<div class=\"darkbg\"></div>\n";
    print "<div class=\"confirmation\"><h2>Confirmação</h2>$msg<br><br>\n";
    print "<p align=right><img src=\"null.png\" width=80 height=1>\n";

    $code1 = "window.location.href='$destyes'";
    if ($destyes == "back")
      $code1 = "history.go(-1)";
    if ($destyes == "back2")
      $code1 = "history.go(-2)";

    $code2 = "window.location.href='$destno'";
    if ($destno == "back")
      $code2 = "history.go(-1)";
    if ($destno == "back2")
      $code2 = "history.go(-2)";

    print "<input type=\"button\" value=\"Sim\" onClick=\"$code1\">&nbsp;";
    print "<input type=\"button\" value=\"Não\" onClick=\"$code2\"></p></div>\n";
    page_end();
    exit(0);
  }
}

// connects to database, sets up session
function startup() {
  global $conn, $_SESSION;
  session_start();
  $conn = pg_connect("dbname=lnidb user=lnidb");
  if ($conn == FALSE)
    fatal_error("Falha na conexão com o banco de dados.");
  if (!isset($_SESSION['admlevel'])) {
    $_SESSION['admlevel'] = -1;
  }
  pg_query("set statement_timeout = 15000;");
  header('Content-type: text/html; charset=utf-8');
  date_default_timezone_set("America/Sao_Paulo");
}

function xmlstartup() {
  global $conn, $_SESSION;
  session_start();
  $conn = pg_connect("dbname=lnidb user=lnidb");
  if ($conn == FALSE) return;
  pg_query("set statement_timeout = 15000;");
  header('Content-type: text/xml; charset=utf-8');
  date_default_timezone_set("America/Sao_Paulo");
}

function txtstartup() {
  global $conn, $_SESSION;
  session_start();
  $conn = pg_connect("dbname=lnidb user=lnidb");
  if ($conn == FALSE) return;
  pg_query("set statement_timeout = 15000;");
  header('Content-type: text/plain; charset=utf-8');
  date_default_timezone_set("America/Sao_Paulo");
}


// releases db connection
function cleanup() {
  pg_close();
}

function goto_index() {
  header("Location: index.php");
}

function page_selector($curpage,$totalpages,$totalitems,$base) {

  if ($totalpages==0) return;
  print "<center><table border=0 cellpadding=4 cellspacing=4><tr><td><font color=\"#444444\">$totalitems item(s) em $totalpages página(s):</font></td>\n";

  if ($totalpages > 1 && $curpage > 1)
    print "<td><a href=\"${base}&p=1\">Primeira</a></td>";
  $pa = array();
  for($i=-3;$i<=3;$i++) {
    array_push($pa,$i);
    array_push($pa,$totalpages-$i+1);
  }

  for($i=$curpage-3;$i<=$curpage+3;$i++) {
    if ($i < 1 || $i > $totalpages) continue;
    if ($i==$curpage)
      print "<td class=\"curpage\">$i</td>";
    else
      print "<td><a href=\"${base}&p=$i\">$i</a></td>";
  }

  if ($totalpages > 1 && $curpage < $totalpages)
    print "<td><a href=\"${base}&p=$totalpages\">Última</a></td>\n";

  if ($totalpages > 1)
    print "<td>|</td>\n";

  if ($curpage != 1) { $pp = $curpage-1; print "<td><a href=\"${base}&p=$pp\">Anterior</a></td>"; }
  if ($curpage != $totalpages) { $pp = $curpage+1; print "<td><a href=\"${base}&p=$pp\">Próxima</a></td>"; }

  print "</tr></table></center>";
}

function img_link($img,$title,$link) {
  print "<a href=\"$link\"><img border=0 title=\"$title\" src=\"$img\"></a>&nbsp;";
}

function img_link_right($img,$title,$link) {
  print "<a href=\"$link\"><img align=right border=0 title=\"$title\" src=\"$img\"></a>&nbsp;";
}

function img_link_blank($img,$title,$link) {
  print "<a target=\"_blank\" href=\"$link\"><img border=0 title=\"$title\" src=\"$img\"></a>&nbsp;";
}

function img_nolink($img,$title) {
  print "<img border=0 title=\"$title\" src=\"$img\">&nbsp;";
}

function human_size($val) {
  $units = array("B","KB","MB","GB","TB","PB","Gazillions");
  $c = 0;

  while($c<6 && $val > 5000) {
    $c++;
    $val /= 1024.0;
  }
  $val = round($val);
  return "$val $units[$c]";
}

function pacientes_reset_session() {
  global $_SESSION;
  unset($_SESSION['p.patient']);
  unset($_SESSION['p.patcode']);
  unset($_SESSION['p.age']);
  unset($_SESSION['p.gender']);
  unset($_SESSION['p.sort']);
  unset($_SESSION['p.scanner']);
  unset($_SESSION['p.comments']);
  unset($_SESSION['p.attach']);
}

function estudos_reset_session() {
  global $_SESSION;
  unset($_SESSION['s.date']);
  unset($_SESSION['s.patient']);
  unset($_SESSION['s.patcode']);
  unset($_SESSION['s.age']);
  unset($_SESSION['s.gender']);
  unset($_SESSION['s.sort']);
  unset($_SESSION['s.scanner']);
  unset($_SESSION['s.exam']);
  unset($_SESSION['s.series']);
  unset($_SESSION['s.storage']);
  unset($_SESSION['s.artifact']);
}

// applet_param and diva_applet_code formerly from view.php
function applet_param($name,$value) {
  return("<param name=\"$name\" value=\"$value\">\n");
}

function diva_applet_code($sid) {
  $q = pg_query("select fid from filestudies where sid=$sid order by fid;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), "back");
  
  $n = pg_num_rows($q);
  $fids = '';
  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q,$i);
    $fids .= "$row[0] ";
  }
  $method = "http://143.106.129.42/db/getfile.php?f=";

  $q = pg_query("select cols,rows,slices,bytes from studies where id=$sid;");
  if ($q==FALSE) return '';
  $row = pg_fetch_row($q,0);
  $dim = "$row[0] $row[1] $row[2]";
  $len = $row[3];

  $code = "<applet code=\"Diva\" archive=\"diva.jar\" width=\"10\" height=\"10\">\n";

  $code .= applet_param("permissions","sandbox");
  $code .= applet_param("sid",$sid);
  $code .= applet_param("fids",$fids);
  $code .= applet_param("method",$method);
  $code .= applet_param("dim",$dim);
  $code .= applet_param("size",$len);

  // study list
  $sql1 = "select p.name,p.patcode,p.age,p.gender from patstudies ps,patients p where ps.sid=$sid and ps.pid=p.id;";
  $q = pg_query($sql1);
  if ($q==FALSE) return ''; 
  $patrow = pg_fetch_row($q,0);
  
  $code .= applet_param("patname",$patrow[0]);
  $code .= applet_param("patcode",$patrow[1]);
  $code .= applet_param("patagegender","$patrow[2]/$patrow[3]");
  
  $sql1  = "select s.id,"; // 0
  $sql1 .= "to_char(s.scantime,'YYYY-MM-DD HH24:MI'),s.exam_desc,s.series_desc,s.storage,s.files,"; // 1-5
  $sql1 .= "s.cols,s.rows,s.slices,round((s.xspacing)::numeric,2),round((s.yspacing)::numeric,2),"; // 6-10
  $sql1 .= "round((s.zspacing)::numeric,2),round((s.thickness)::numeric,2),sc.maker,sc.model,sc.location,"; // 11-15
  $sql1 .= "s.scantime,s.bytes,s.oaxi,s.osag,s.ocor "; // 16-17, 18-20
  $sql1 .= "from studies s, studies t, patstudies ps, patstudies pt, scanners sc ";
  $sql1 .= "where t.id=$sid and t.id=pt.sid and s.id=ps.sid and pt.pid=ps.pid and ";
  $sql1 .= "to_char(t.scantime,'YYYY-MM-DD') = to_char(s.scantime,'YYYY-MM-DD') and sc.id=s.scanner ";
  $sql1 .= "order by s.scantime, s.series_desc;";
  $q = pg_query($sql1);
  if ($q==FALSE) return '';
  
  $n = pg_num_rows($q);
  $code .= applet_param("studyn",$n);
  
  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q,$i);
    
    $code .= applet_param("studyid".$i,$row[0]);
    $code .= applet_param("studydate".$i,$row[1]);
    $code .= applet_param("studyexam".$i,$row[2]);
    $code .= applet_param("studyseries".$i,$row[3]);
    $code .= applet_param("studyfiles".$i,$row[5]);
    $code .= applet_param("studydim".$i,"$row[6]x$row[7]x$row[8]");
    $code .= applet_param("studyvox".$i,"$row[9]x$row[10]x$row[11]");
    $code .= applet_param("studythk".$i,$row[12]);
    $code .= applet_param("scannermaker".$i,$row[13]);
    $code .= applet_param("scannermodel".$i,$row[14]);
    $code .= applet_param("scannerlocation".$i,$row[15]);
    $code .= applet_param("studysize".$i,$row[17]);
    
    $osum = $row[18] + $row[19] + $row[20];
    $ori = '';
    
    if ($row[18] != 0) { $ori .= "Ax/"; }
    if ($row[19] != 0) { $ori .= "Sag/"; }
    if ($row[20] != 0) { $ori .= "Cor/"; }
    
    if ($osum > 0) { $ori = substr($ori,0,-1); } else { $ori = 'n/a'; }
    $code .= applet_param("studyorient".$i,$ori);
    
    $fq = pg_query("select fid from filestudies where sid=$row[0] order by fid;");
    if ($fq==FALSE) return '';
    
    $nfids = pg_num_rows($fq);
    $fids = '';
    for($j=0;$j<$nfids;$j++) {
      $frow = pg_fetch_row($fq,$j);
      $fids .= "$frow[0] ";
    }
    
    $code .= applet_param("studyfids".$i,$fids);
  }
  
  $code .= applet_param("java_arguments","-Xmx2048m");
  $code .= "</applet>\n";
  return $code;
}

function artifact_icon($aval) {
  $artpng = array('art0.png','art1.png','art2.png','art3.png');
  $arttxt = array('Sem informação sobre artefatos', 'Sem Artefatos', 'Artefatos Leves', 'Artefatos Sérios');

  if ($aval >= 0 && $aval <= 3) {
    return("<img width=\"16\" height=\"16\" src=\"" . $artpng[$aval] . "\" title=\"" . $arttxt[$aval] . "\">");
  } else return('');
}

function zzfatal($msg) {
  print "error=$msg\n";
  cleanup();
  exit(0);
}

?>
