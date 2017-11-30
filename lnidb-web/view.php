<?php
include('lnidb.inc');
startup();

if (!isset($_GET['s'])) fatal_error("Parâmetros ausentes","back");

$sid = $_GET['s'];

print "<html><title>DIVA</title><body>\n";

$code = diva_applet_code($sid);

if (strlen($code) == 0) {
  print "<b>Falha na consulta ao banco de dados:</b> " . pg_last_error() ."<br>";
} else {
  print $code;
  print "</body></html>\n";
}
cleanup();

?>
