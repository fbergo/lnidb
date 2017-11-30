<?php
session_start();

$pars = array('patient', 'patcode', 'age', 'gender', 'scanner', 'comments','attach');
foreach($pars as $x) {
  if (isset($_POST[$x])) { $_SESSION['p.' . $x] = pg_escape_string($_POST[$x]); }
}

?>
<html>
<script language="JavaScript">

  function goindex() {
    var x = window.location.href;
    window.location = x.replace('pacientes.php','index.php?m=0');
  }

</script>
<body onload="goindex()"></body></html>
