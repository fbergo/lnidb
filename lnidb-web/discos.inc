<?php

function discos_main() {
  global $_GET;

  print "<div class=\"regular\">\n";

  $sql = "select distinct regexp_matches(storage, '(^[A-Za-z-]+)') from studies order by 1;";
  $q = pg_query($sql);
  $npref = pg_num_rows($q);
  $pref = array();
  for($i=0;$i<$npref;$i++) {
    $x = pg_fetch_result($q,$i,0);
    if (preg_match('/([A-Za-z-]+)/', $x, $tmp)) $x = $tmp[1];
    array_push($pref, $x);
  }

  $showpref = '';
  if (isset($_GET['r'])) $showpref = $_GET['r']; 
  if ($showpref=='all') $showpref='';

  if ($showpref == '') 
    $sql = "select distinct storage from studies order by storage;";
  else
    $sql = "select distinct storage from studies where storage ~ '^$showpref' order by storage;";

  $q = pg_query($sql);
  if ($q==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
  $n = pg_num_rows($q);

  print "<h3>Lista de Discos</h3><ul>";

  print "Prefixo/Grupo: <form action=\"index.php\"><input type=hidden name=\"m\" value=\"3\">";
  print "<select name=\"r\" onchange=\"form.submit();\">";
  if ($showpref == '')
    print "<option value=\"all\" selected>Todos</option>";
  else
    print "<option value=\"all\">Todos</option>";

  foreach($pref as $x) {
    if ($x == $showpref)
      print "<option value=\"$x\" selected>$x</option>";
    else
      print "<option value=\"$x\">$x</option>";
  }

  print "</select></form><br>\n";

  print "Total: <b>$n</b> discos listados<br>\n";

  $d = pg_fetch_result($q,$n-1,0);
  print "Último disco listado (por nome/número):  <b>$d</b><br>\n";

  $q2 = pg_query("select storage from studies order by id desc limit 1;");
  if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
  $d = pg_fetch_result($q2,0,0);
  print "Último disco (importação mais recente): <b>$d</b><br>\n";

  $q2 = pg_query("select storage, to_char(scantime,'YYYY-MM-DD HH24:MM') from studies order by scantime desc limit 1;");
  if ($q2==FALSE) fatal_error("Falha na consulta ao banco de dados: " . pg_last_error(),"back");
  $d = pg_fetch_result($q2,0,0);
  $st = pg_fetch_result($q2,0,1);
  print "Disco com estudo mais recente: <b>$d</b> (Data do estudo: <i>$st</i>)<br></ul>\n";

  print "<table border=0 cellspacing=4 cellpadding=2>\n";
  
  $ncols = 10;
  $numbers = array();
  $nmin = -1;
  $nmax = -1;
  $alt = 'b';
  $prev = 'none';
  $break = "</tr><tr valign=top>";
  for($i=0,$cpos=0;$i<$n;$i++,$cpos++) {
    if ( ($cpos % $ncols) == 0 ) { print "<tr valign=top>"; }
    $d = pg_fetch_result($q,$i,0);
    $pd = $d;

    if (preg_match('/^([A-Za-z]+)([0-9]+)([A-Za-z]+)$/',$d,$tmp)) {
      if ($tmp[1] != $prev) { 
	if ($prev != 'none') { $cpos=0; print $break; }
	$prev = $tmp[1]; $alt = ($alt == 'a' ? 'b' : 'a'); 
      }

      $pd = "$tmp[1]<b>$tmp[2]</b>$tmp[3]";
    } else if (preg_match('/^([A-Za-z]+)([0-9]+)$/',$d,$tmp)) {
      if ($tmp[1] != $prev) { 
	if ($prev != 'none') { $cpos=0; print $break; }
	$prev = $tmp[1]; $alt = ($alt == 'a' ? 'b' : 'a'); 
      }
      $pd = "$tmp[1]<b>$tmp[2]</b>";
    } else if (preg_match('/^([A-Za-z_]+)([A-Za-z]+)$/',$d,$tmp)) {
      if ($tmp[1] != $prev) { 
	if ($prev != 'none') { $cpos=0; print $break; }
	$prev = $tmp[1]; $alt = ($alt == 'a' ? 'b' : 'a');
      }
      $pd = "$tmp[1]<b>$tmp[2]</b>";
    } else {
      $prev = $pd; $alt = ($alt == 'a' ? 'b' : 'a');
      $cpos=0; print $break;
    }

    print "<td class=\"dkl$alt\">$pd</td>\n";

    if ($showpref != '') {
      if (preg_match('/^[A-Za-z_]+([0-9]+)/',$d,$tmp)) {
	$x = (int) $tmp[1];
	$numbers[$x] = 1;
	if ($x < $nmin || $nmin == -1) $nmin = $x;
	if ($x > $nmax || $nmax == -1) $nmax = $x;
      }
    }

    if ( ($cpos % $ncols) == ($ncols - 1) ) { print "</tr>"; }
  }

  print "</table>\n";

  if ($showpref != '') {


    if ($nmin == -1 || $nmax == -1) {
      print "<ul><table border=0 cellspacing=4 cellpadding=2><tr><td width=500>Numeração: <b>indefinida</b></td></tr></table></ul>\n";
    } else {
      $miss = array();
      for($i=$nmin;$i<=$nmax;$i++)
	if (!isset($numbers[$i]))
	  array_push($miss,$i);
      
      print "<ul><table border=0 cellspacing=4 cellpadding=2><tr><td width=500>Numeração de <b>$nmin</b> a <b>$nmax</b>. Faltam: <font color=red>";
      foreach($miss as $x)
	print "$x, ";
      $c = count($miss);
      print "</font><b>$c</b> itens.</td></tr></table></ul>\n";
      
    }

  }

  print "<br><br></div>\n";
}

?>

