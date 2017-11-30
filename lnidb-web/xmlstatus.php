<?php
include('lnidb.inc');

if (isset($_GET['u'])) {
  $uid = $_GET['u'];
} else {
  exit(0);
}

xmlstartup();

print "<xml>\n";
print "<uid>$uid</uid>\n";
$t = scalar_query("select to_char(now(),'HH24:MI:SS');");
print "<now>$t</now>";

// tarefas concluidas na última hora, tarefas executando e pendentes
$sql = "select id,state,creator,message,task,src,options from tasks where creator=$uid and ( ((state=2 or state=3) and (now()-ended < '1 hour')) or state < 2 ) order by started desc;";
$q = pg_query($sql);
$n = pg_num_rows($q);

print "<count>$n</count>\n";

for($i=0;$i<$n;$i++) {
  $row = pg_fetch_row($q,$i);
  for($j=0;$j<7;$j++) {
    if ($row[$j] == '')
      $row[$j] = '-';
    $row[$j] = htmlspecialchars($row[$j]);
  }
  print "<item><id>$row[0]</id><state>$row[1]</state><msg>$row[3]</msg>";
  print "<task>$row[4]</task><src>$row[5]</src><options>$row[6]</options>";

  if ($row[1] == 0) {
    $np = scalar_query("select count(id) from tasks where state < 2 and id < $row[0];");
    print "<behind>$np</behind>";
  }
  $t = scalar_query("select to_char(started,'HH24:MI:SS') from tasks where id=$row[0];");
  print "<started>$t</started>";
  if ($row[1] == 2 || $row[1] == 3) {
    $t = scalar_query("select to_char(ended,'HH24:MI:SS') from tasks where id=$row[0];");
    print "<ended>$t</ended>";
  }
  print "</item>\n";

}

$nb = scalar_query("select count(fid) from basket where uid=$uid;") + 
  scalar_query("select count(aid) from basket2 where uid=$uid;");
$np = scalar_query("select count(tid) from wrapped where owner=$uid;");
print "<basket>$nb</basket><packages>$np</packages>\n";

print "</xml>\n";
cleanup();

?>
