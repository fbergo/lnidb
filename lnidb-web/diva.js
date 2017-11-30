function diva(sid) {
    var box  = document.getElementById('divabox');;

    if (deployJava.versionCheck("1.6.0") || deployJava.versionCheck("1.7.0") || deployJava.versionCheck("1.8.0") ) {
	box.innerHTML = "<br><b>Carregando Visualizador</b><br>";
	box.innerHTML += "<iframe seamless width=50 height=1 frameborder=0 src=\"view.php?s=" + sid + "\"></iframe>";
	box.style.visibility = "visible";
	setTimeout("hidediva()",5000);
    } else {
	nojava();
    }
}

function hidediva() {
    document.getElementById('divabox').style.visibility = "hidden";
}

var njto = null;

function nojava() {
    var box  = document.getElementById('nojavabox');
    box.innerHTML = "<b>Impossível Visualizar: Plugin Java Ausente ou Desabilitado.</b><br><br><a class=\"njlink\" href=\"javascript:hidenojava()\">Fechar</a>";
    box.style.visibility = "visible";
    njto = setTimeout("hidenojava()",10000);
}

function hidenojava() {
    if (njto!=null)
	clearTimeout(njto);
    njto = null;
    document.getElementById('nojavabox').style.visibility = "hidden";
}
