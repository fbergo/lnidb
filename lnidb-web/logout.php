<?php
include('lnidb.inc');

startup();

unset($_SESSION['username']);
unset($_SESSION['fullname']);
unset($_SESSION['userid']);
unset($_SESSION['admlevel']);
unset($_SESSION['prevmenu']);

cleanup();
goto_index();
?>
