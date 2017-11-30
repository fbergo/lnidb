<?php
include('lnidb.inc');
include('utilidades.inc');
include('discos.inc');
include('cesta.inc');
include('pacientes.inc');
include('estudos.inc');
include('grupos.inc');

startup();

if (isset($_GET['reset'])) {
  pacientes_reset_session();
  estudos_reset_session();
}

if (isset($_GET['m'])) {
  $menu = $_GET['m'];
  if ($menu>=0 && $menu<=6)
    $_SESSION['prevmenu'] = $menu;
} elseif (isset($_SESSION['prevmenu'])) {
  $menu = $_SESSION['prevmenu'];
} else {
  $menu = 0;
}

page_start("LNI DB");

if (!isset($_GET['print'])) page_top();

if ($menu == 0) pacientes_main();
if ($menu == 1) estudos_main();
if ($menu == 2) grupos_main();
if ($menu == 3) discos_main();
if ($menu == 4) cesta_main();
if ($menu == 5) util_main();

if ($menu == 6) {
  print "<div class=\"ajuda\">";
  readfile("ajuda.html");
  print "</div>\n";
}

page_end();
cleanup();

?>

