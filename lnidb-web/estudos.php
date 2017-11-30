<?php
session_start();

$pars = array('patient', 'patcode', 'date', 'age', 'gender', 'exam', 'series', 'storage', 'scanner', 'artifact');
foreach($pars as $x) {
  if (isset($_POST[$x])) { $_SESSION['s.' . $x] = pg_escape_string($_POST[$x]); }
}

?>
<html>
<script language="JavaScript">

  function goindex() {
    var x = window.location.href;
    window.location = x.replace('estudos.php','index.php?m=1');
  }

</script>
<body onload="goindex()"></body></html>
