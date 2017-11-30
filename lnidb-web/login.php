<?php
include('lnidb.inc');

startup();
page_start("LNI DB - Login");
page_top(0,0);

print "<div class=\"darkbg\"></div>\n";
print "<div class=\"dlg\"><h2>Login</h2><form name=\"loginform\" action=\"login2.php\" method=\"post\">\n";
print "<table border=0 cellpadding=0 cellspacing=4><tr><td align=right>Login</td>\n";
print "<td><input type=text name=\"username\" size=20></td></tr><tr><td>Senha</td>\n";
print "<td><input type=password name=\"password\" size=20</td></tr></table>\n";
print "<p align=right><input type=submit value=\"Entrar\">&nbsp;<input type=button value=\"Cancelar\" onClick=\"window.location.href='index.php'\"></p>\n";
print "</form></div>\n";

page_end();
cleanup();
?>

