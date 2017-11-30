<?php

function cesta_main() {
  global $_SESSION;

  if (!isset($_SESSION['userid'])) fatal_error("Função disponível apenas para usuários logados.",menu_link('pac'));
  $uid = $_SESSION['userid'];

  print "<div class=\"regular\">\n";

  print "<h3>Cesta de Arquivos</h3><ul>\n";

  $q = pg_query("select distinct s.id,to_char(s.scantime,'YYYY-MM-DD HH24:MM'),s.exam_desc,s.series_desc,p.name,p.patcode,p.age,p.gender,sc.maker,sc.model,sc.location,p.id,s.scantime,s.cols,s.rows,s.slices,round((s.xspacing)::numeric,2),round((s.yspacing)::numeric,2),round((s.zspacing)::numeric,2),round((s.thickness)::numeric,2),s.artifact from studies s, basket b, filestudies fs, patients p, patstudies ps, scanners sc where s.id=fs.sid and fs.fid=b.fid and b.uid=$uid and p.id=ps.pid and s.id=ps.sid and sc.id=s.scanner order by p.name, s.scantime;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), menu_link('pac'));
  $n = pg_num_rows($q);

  $q3 = pg_query("select a.id,to_char(a.created,'YYYY-MM-DD HH24:MM'),a.description,a.bytes,p.id,p.name,p.patcode,p.age,p.gender,u.username,u.fullname,a.path from attachments a, patattachs pa, patients p, users u, basket2 b where b.uid=$uid and b.aid=a.id and pa.aid=b.aid and pa.pid=p.id and a.creator=u.id order by p.name, a.created;");
  if ($q3==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(), menu_link('pac'));
  $m = pg_num_rows($q3);

  // 0-4 s.id, scantime, exam, series, p.name
  // 5-9 patcode, age, gender, maker, model
  // 10-14 location, p.id, scantime, cols, rows
  // 15-19 slices, dx, dy, dz, thickness
  // 20 artifact

  print "<script language=\"JavaScript\" src=\"cesta.js\"></script>\n";

  if ($n+$m > 0) {
    print "<a href=\"basket.php?o=3&s=0\">[Esvaziar Cesta]</a><br><br>\n";
    print "<a href=\"javascript:packbasket()\">[Empacotar]</a><br><br>\n";
    
    $admlevel = $_SESSION['admlevel'];
    if ($admlevel == 2)
      print "<a href=\"javascript:tagquality()\">[Alterar Marcadores de Qualidade]</a><br><br>\n";

    print "<div class=\"dialogbox\" id=\"qualitybox\"><form id=\"qualityform1\" action=\"quality.php\" method=POST><h2>Alterar Marcadores de Qualidade</h2>Marcar o(s) $n estudos na cesta como:<br><br>";
    print "<input type=radio name=\"artifact\" value=\"0\" checked=\"checked\"><img src=\"art0.png\"> Sem informação</input><br>";
    print "<input type=radio name=\"artifact\" value=\"1\"><img src=\"art1.png\"> Sem artefatos</input><br>";
    print "<input type=radio name=\"artifact\" value=\"2\"><img src=\"art2.png\"> Artefatos leves</input><br>";
    print "<input type=radio name=\"artifact\" value=\"3\"><img src=\"art3.png\"> Artefatos sérios</input><br>";
    print "<br><p>";
    print "<input type=submit value=\"Marcar\">&nbsp;<input type=button value=\"Cancelar\" onClick=\"javascript:tagcancel()\"></p></form>";
    print "<i><small>Artefatos localizados que não impeçam o uso da imagem para todo tipo de pesquisa devem ser marcados como leves.<br>Artefatos que tornem a imagem inutilizável devem ser marcados como sérios.</small></i><br>";
    print "</div>\n";

    print "<div class=\"dialogbox\" id=\"packbox\"><form action=\"wrap.php\" method=POST><h2>Empacotar Cesta</h2><ul>Nome: <input type=text name=\"title\" id=\"packform1\" size=14>";
    print "&nbsp;Formato: <select name=\"format\"><option selected value=\"zip\">ZIP</option><option value=\"targz\">TAR+GZ</option></select></ul>";
    print "<b>Conversões:</b><ul>";
    print "<input type=checkbox name=\"nii\">NIfTI (<font color=#008800><b>.nii</b></font> ou <font color=#008800><b>.nii.gz</b></font>, via <font color=#880000>dcm2nii/mricron</font>)</input>\n";
    print "<ul><input type=checkbox name=\"niigz\">NIfTI comprimido (.nii.gz)</input></ul>\n";
    print "<input type=checkbox name=\"analyze\">Analyze (<font color=#008800><b>.hdr+.img</b></font>, via <font color=#880000>dcm2nii/mricron</font>)</input><br>\n";
    print "<input type=checkbox name=\"minc\">MINC (<font color=#008800><b>.mnc</b></font>, via <font color=#880000>dicom2scn + scn2ana + nii2mnc/MNI</font>)</input><br>\n";
    print "<ul><input type=checkbox name=\"mincsag\">Gerar MINC Sagital</input><br>\n";
    print "<input type=checkbox name=\"minccor\">Gerar MINC Coronal</input><br>\n";
    print "<input type=checkbox name=\"mincaxi\">Gerar MINC Axial</input><br></ul>\n";
    print "<input type=checkbox name=\"scn\">SCN (<font color=#008800><b>.scn</b></font>, via <font color=#880000>dicom2scn</font>)</input><br>\n";
    print "<ul>Tratamento de intensidades negativas [SCN]:<ul>\n";    
    print "<input type=radio name=\"scnneg\" value=\"shift\"><i>Somar</i> deslocamento para que a menor intensidade seja zero</input><br>\n";
    print "<input type=radio name=\"scnneg\" value=\"zero\"><i>Zerar</i> intensidades negativas</input><br>\n";
    print "<input type=radio name=\"scnneg\" value=\"keep\"><i>Manter</i> intensidades negativas</input><br>\n";
    print "<input type=radio name=\"scnneg\" value=\"auto\" checked=\"checked\"><i>Automático</i> <small><i><font color=#880088>(zera negativos em estudos T2 multi-eco, soma offset nos demais)</i></small></font></input>\n";
    print "</ul><input type=checkbox name=\"scnme\">Separar ecos em estudos multi-eco/relaxometria [SCN]</input><br>\n";
    print "<input type=checkbox name=\"scndif\">Separar fases em estudos de difusão [SCN]</input></ul>\n";
    print "<input type=checkbox name=\"nodicom\">Não incluir os arquivos DICOM originais <font color=#880088><i><small>(opção ignorada se não houver conversões)</small></i></font></input>\n";
    print "</ul><br><p align=center>\n";
    print "<input type=submit value=\"Empacotar\">&nbsp;<input type=button value=\"Cancelar\" onClick=\"javascript:packcancel()\"></p></form></div>\n";
  }

  print "<div class=\"dialogbox\" id=\"sidbox\"><form action=\"basket.php\" id=\"sidform\" method=POST><h2>Inserir Lista Numérica de Estudos</h2>";
  print "<small>Entre com IDs de estudos separados por vírgulas.<br>Todo pacote gerado pelo sistema inclui uma lista neste formato no arquivo <i>0001-studylist.txt</i>.</small><br>";
  print "<input type=hidden name=\"o\" value=\"5\"><textarea rows=4 cols=60 id=\"sidtext\" name=\"sidtext\"></textarea><br><input type=submit value=\"Inserir\">&nbsp;<input type=button value=\"Cancelar\" onClick=\"javascript:closesid()\"></form></div>\n";
  print "<a href=\"javascript:insertsid()\">[Inserir Lista Numérica de Estudos]</a><br><br>\n";

  print "</ul><span id=\"conteudocesta\"><h4>Conteúdo</h4><ul>\n";
  
  $totalf = 0;
  $totals = 0;

  $qstart = microtime(true);
  $late = 0;
  $nlate = 0;
  $tpass=0.0;
  print "<table border=0 cellspacing=4 cellpadding=4>\n";
  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q,$i);
    
    $i1 = $i + 1;
    print "<tr valign=top><td>$i1</td><td>";
    img_link("bminus.png","Remover estudo da Cesta","basket.php?o=2&s=$row[0]");
    img_link("bdetail16.png","Detalhes do Paciente",menu_link('pac')."&v=$row[11]&e=0");
    print "</td><td>";
    print "$row[4] ($row[5]), ${row[6]}/${row[7]}&nbsp;" . artifact_icon($row[20]) . "<br><b>$row[1]</b> ($row[8] $row[9] $row[10]): <b>$row[3]</b> ($row[2]).";

    $dim = "${row[13]}x${row[14]}x${row[15]}, ${row[16]}x${row[17]}x${row[18]} mm, thickness ${row[19]} mm";
    print "<br><font color=#888888>$dim</font>";

    if ($late==0 && $i > 200) {
      $qnow = microtime(true);
      if ($qnow - $qstart > 5.0) {
	$late = 1;
	$nlate = $i;
	$tpass = $qnow-$qstart;
      }
    }

    $nfiles = 0;
    $hs = '';
    if (!$late) {
      $q2 = pg_query("select count(b.fid) from basket b, filestudies fs where b.fid=fs.fid and b.uid=$uid and fs.sid=$row[0];");
      if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),menu_link('pac'));
      $r2 = pg_fetch_row($q2,0);
      $nfiles = $r2[0];

      $q2 = pg_query("select sum(f.size) from basket b, filestudies fs, files f where b.fid=fs.fid and b.uid=$uid and fs.sid=$row[0] and f.id=b.fid;");
      if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),menu_link('pac'));
      $r2 = pg_fetch_row($q2,0);
      $hs = human_size($r2[0]);

      $totalf += $nfiles;
      $totals += $r2[0];
      print "<br><b>$nfiles arquivo(s), $hs.</b></td></tr>\n";
    } else {
      print "<br><span class=rederror>Muitos estudos na cesta, contagem de arquivos e tamanho interrompida.</span></td></tr>\n";
    }

  }

  // 0-5 a.id,creation,description,bytes,p.id,p.name
  // 6-11 patcode,age,gender,username,fullname,path
  for($i=0;$i<$m;$i++) {
    $row = pg_fetch_row($q3,$i);

    $file = $row[11];
    if (preg_match('/.*\\/(.+)$/',$file,$tmp))
      $file = $tmp[1];

    print "<tr valign=top><td>";
    img_link("bminus.png","Remover estudo da Cesta","basket.php?o=2&a=$row[0]");
    img_link("bdetail16.png","Detalhes do Paciente",menu_link('pac')."&v=$row[4]&e=0");
    print "</td><td>";
    print "$row[5] ($row[6]), ${row[7]}/${row[8]}<br><b>$row[1]</b> por <font color=#008800>$row[10] ($row[9])</font>: <font color=#000088><b>$file</b></font><br>";
    print "<font color=#880088>$row[2]</font><br>";
    $hs = human_size($row[3]);
    print "<b>1 arquivo, $hs.</b></td></tr>\n";

    $totalf += 1;
    $totals += $row[3];
  }

  print "</table>\n";

  $hs = human_size($totals);
  print "$hs em $totalf arquivo(s) de $n estudo(s) e $m anexo(s).<br>\n";
  if ($late) {
    print "<span class=rederror>Obs: Contagem incompleta por excesso de arquivos na cesta.</span><br>\n";
  }
  $qnow = microtime(true);
  $tq = $qnow - $qstart;
  $tq = number_format($tq, 4, '.', ' ');
  $tpass = number_format($tpass, 4, '.', ' ');
  print "<span class=\"debuginfo\">Tempo de consulta: $tq seg, nlate=$nlate, tpass=$tpass</span><br>\n";

  print "</ul><h4>Pacotes</h4><ul>\n";

  $q = pg_query("select tid,path,size,to_char(wraptime,'YYYY-MM-DD HH24:MI') from wrapped where owner=$uid order by wraptime;");
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),menu_link('pac'));
  $n = pg_num_rows($q);

  if ($n > 0) {
    print "<a href=\"basket.php?o=4&s=0\">[Remover Todos os Pacotes]</a><br><br>\n";
  }

  $totals = 0;
  for($i=0;$i<$n;$i++) {
    $row = pg_fetch_row($q,$i);
    $name = basename($row[1]);
    $hs = human_size($row[2]);
    print "<a href=\"getwrap.php?w=$row[0]\"><img border=0 title=\"Download\" src=\"bdownload.png\"></a>&nbsp;";
    print "<a href=\"basket.php?o=4&s=$row[0]\"><img border=0 title=\"Remover\" src=\"bdel16.png\"></a>&nbsp;";
    print "<b>$name</b>, $hs, finalizado em $row[3]<br>\n";
    $totals += $row[2];
  }

  $hs = human_size($totals);
  print "<br>$hs em $n pacote(s).<br>\n";

  print "</ul></span>\n";
  print "</div>\n";
}

?>

