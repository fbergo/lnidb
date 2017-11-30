<?php

function svgcolor($val) {
  $r = ($val >> 16) & 0xff;
  $g = ($val >> 8) & 0xff;
  $b = ($val) & 0xff;
  return("rgb($r,$g,$b)");
}

function rect($x,$y,$w,$h,$fill,$border,$sw) {  
  $fill = svgcolor($fill);
  $border = svgcolor($border);
  $x = round($x,1);
  $y = round($y,1);
  $w = round($w,1);
  $h = round($h,1);
  print "<rect x=\"$x\" y=\"$y\" width=\"$w\" height=\"$h\" style=\"fill:$fill;stroke-width:$sw;stroke:$border\"/>\n";
}

function rtext($text, $x, $y, $font, $size, $color,$bold=0,$italic=0) {
  $color = svgcolor($color);
  $x = round($x,1);
  $y = round($y,1);
  $extra = '';
  if ($bold) { $extra="$extra font-weight=\"bold\""; }
  if ($italic) { $extra="$extra font-style=\"italic\""; }
  print "<text x=\"$x\" y=\"$y\" font-family=\"$font\" font-size=\"$size\" fill=\"$color\" text-anchor=\"end\">$text</text>\n";
}

function vtext($text, $x, $y, $font, $size, $color,$bold=0,$italic=0) {
  $color = svgcolor($color);
  $x = round($x,1);
  $y = round($y,1);
  $extra = '';
  if ($bold) { $extra="$extra font-weight=\"bold\""; }
  if ($italic) { $extra="$extra font-style=\"italic\""; }
  print "<text x=\"$x\" y=\"$y\" transform=\"rotate(270,$x,$y)\" font-family=\"$font\" font-size=\"$size\" fill=\"$color\" text-anchor=\"start\">$text</text>\n";
}

function ctext($text, $x, $y, $font, $size, $color, $bold=0, $italic=0) {
  $color = svgcolor($color);
  $x = round($x,1);
  $y = round($y,1);
  $extra = '';
  if ($bold) { $extra="$extra font-weight=\"bold\""; }
  if ($italic) { $extra="$extra font-style=\"italic\""; }
  print "<text x=\"$x\" y=\"$y\" font-family=\"$font\" font-size=\"$size\" fill=\"$color\" $extra text-anchor=\"middle\">$text</text>\n";
}

function text($text, $x, $y, $font, $size, $color,$bold=0,$italic=0) {
  $color = svgcolor($color);
  $x = round($x,1);
  $y = round($y,1);
  $extra = '';
  if ($bold) { $extra="$extra font-weight=\"bold\""; }
  if ($italic) { $extra="$extra font-style=\"italic\""; }
  print "<text x=\"$x\" y=\"$y\" font-family=\"$font\" font-size=\"$size\" fill=\"$color\">$text</text>\n";
}

function line($x1,$y1,$x2,$y2,$color=-1,$sw=-1) {
  if ($color >= 0) { $color = svgcolor($color); }
  $x1 = round($x1,1);
  $y1 = round($y1,1);
  $x2 = round($x2,1);
  $y2 = round($y2,1);
  if ($sw >= 0) {
    print "<line x1=\"$x1\" y1=\"$y1\" x2=\"$x2\" y2=\"$y2\" style=\"stroke-width:$sw;stroke:$color\"/>\n";
  } else {
    print "<line x1=\"$x1\" y1=\"$y1\" x2=\"$x2\" y2=\"$y2\"/>\n";
  }
}

function style($fill,$border,$sw) {
  $fill = svgcolor($fill);
  $border = svgcolor($border);
  print "<g fill=\"$fill\" stroke=\"$border\" stroke-width=\"$sw\">\n";
}

function circle($x,$y,$r,$fill,$border,$sw) {
  $fill   = svgcolor($fill);
  $border = svgcolor($border);
  $x = round($x,1);
  $y = round($y,1);
  $r = round($r,2);
  print "<circle cx=\"$x\" cy=\"$y\" r=\"$r\" style=\"fill:$fill;stroke-width:$sw;stroke:$border\"/>\n";
}

function translate($x, $y) {
  $x = round($x,1);
  $y = round($y,1);
  print "<g transform=\"translate($x,$y)\">\n";
}

function rotate($ang) {
  print "<g transform=\"rotate($ang)\">\n";
}

function scale($fac) {
  $fac = round($fac,3);
  print "<g transform=\"scale($fac)\">\n";
}

function endtrans() {
  print "</g>\n";
}

function endstyle() {
  print "</g>\n";
}

function regpoly($x, $y, $r, $n, $fill, $border, $sw) {
  $fill = svgcolor($fill);
  $border = svgcolor($border);
  print "<polygon points=\"";
  for($i=0;$i<$n;$i++) {
    $px = round($x + $r * cos( M_PI_2 + $i*2.0*M_PI / $n ), 2);
    $py = round($y - $r * sin( M_PI_2 + $i*2.0*M_PI / $n ), 2);

    print "$px,$py ";
  }
  print "\" style=\"fill:$fill;stroke-width:$sw;stroke:$border\"/>\n";
}

function star($x, $y, $r1, $r2, $n, $fill, $border, $sw) {
  $fill = svgcolor($fill);
  $border = svgcolor($border);
  print "<polygon points=\"";
  for($i=0;$i<$n;$i++) {
    $px = round($x + $r1 * cos( M_PI_2 + $i*2.0*M_PI / $n ), 2);
    $py = round($y - $r1 * sin( M_PI_2 + $i*2.0*M_PI / $n ), 2);
    print "$px,$py ";
    $px = round($x + $r2 * cos( M_PI_2 + ($i+0.5)*2.0*M_PI / $n ), 2);
    $py = round($y - $r2 * sin( M_PI_2 + ($i+0.5)*2.0*M_PI / $n ), 2);
    print "$px,$py ";
  }
  print "\" style=\"fill:$fill;stroke-width:$sw;stroke:$border\"/>\n";  
}

function marker($x,$y,$color,$shape,$binsize) {

  translate($x,$y);
  scale(min($binsize/15.0, 1.0));

  switch($shape % 10) {

  case 0:
    circle(0,0,4,$color,0,0.5);
    break;
  case 1:
    rect(-3,-3,7,7,$color,0,0.5);
    break;
  case 2:
    regpoly(0,0,5,3,$color,0,0.5);
    break;
  case 3:
    regpoly(0,0,6,5,$color,0,0.5);
    break;
  case 4:
    star(0,0,6,3,5,$color,0,0.5);
    break;
  case 5:
    star(0,0,6,3,6,$color,0,0.5);
    break;
  case 6:
    star(0,0,6,3,4,$color,0,0.5);
    break;
  case 7:
    star(0,0,6,3,3,$color,0,0.5);
    break;
  case 8:
    star(0,0,6,3,8,$color,0,0.5);
    break;
  case 9:
    rect(-3,-2,7,5,$color,0,0.5);
    break;
  }

  endtrans();
  endtrans();
}

function human_value($v) {
  $u = 0;
  if ($v == 0) return("0");

  $units = array('KB','MB','GB','TB');
  while ($v > 1024) {
    $v = $v / 1024.0;
    $u++;
  }
  $v = round($v,1);
  return("$v $units[$u]");
}

?>
