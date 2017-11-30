
function updateTicker(uid) {

    var t = document.getElementById('ticker');
    var ta = document.getElementById('tickerarrow');
    var n, x, html='';

    xhr = new XMLHttpRequest();
    xhr.open("GET","xmlstatus.php?u="+uid,false);
    xhr.send();
    xml = xhr.responseXML;

    if (xml == null) {

	t.innerHTML = "resposta nula, status=" + xhr.status + ", text=" + xhr.statusText;
	t.style.visibility = "visible";
	ta.style.visibility = "visible";
	n = 0;

    } else {

	x = xml.getElementsByTagName('item');
	n = x.length;

	if (n!=0)
	    html += "<span style=\"color: yellow\">Suas tarefas recentes ou pendentes:</span><br>";

	now = (xml.getElementsByTagName("now")[0].childNodes[0].nodeValue);

	for(i=0;i<n;i++) {

	    if (i > 4) {
		html += "<span class=\"tickermore\">...e mais " + (n-5) + " tarefa(s)</span><br>";
		break;
	    }

	    id      = (x[i].getElementsByTagName("id")[0].childNodes[0].nodeValue);
	    state   = (x[i].getElementsByTagName("state")[0].childNodes[0].nodeValue);
	    task    = (x[i].getElementsByTagName("task")[0].childNodes[0].nodeValue);
	    src     = (x[i].getElementsByTagName("src")[0].childNodes[0].nodeValue);
	    options = (x[i].getElementsByTagName("options")[0].childNodes[0].nodeValue);
	    started = (x[i].getElementsByTagName("started")[0].childNodes[0].nodeValue);

	    if (task == 'wrap-basket')         task = 'Empacotar cesta';
	    if (task == 'remove-package')      task = 'Remover pacotes';
	    if (task == 'import-dicom')        task = 'Importar DICOM';
	    if (task == 'attach-file')         task = 'Anexar arquivo';
	    if (task == 'purge-old-packages')  task = 'Remover pacotes antigos';
	    if (task == 'remove-attach')       task = 'Remover anexo';
	    if (task == 'storage-delete')      task = 'Remover disco';
	    if (task == 'storage-rename')      task = 'Renomear disco';
	    
	    switch(state) {
	    case '0': // pending
		behind = (x[i].getElementsByTagName("behind")[0].childNodes[0].nodeValue);
		html += now + " <span class=\"ticker" + state + "\">Pendente</span> "+task+" ("+src+","+options+") atrás de <b>"+behind+"</b> outra(s) tarefa(s).<br>";
		break;
	    case '1': // executing
		html += now + " <span class=\"ticker" + state + "\">Executando</span> "+task+" ("+src+","+options+")<br>";
		break;
	    case '2': // done, success
		ended = (x[i].getElementsByTagName("ended")[0].childNodes[0].nodeValue);
		html += ended + " <span class=\"ticker" + state + "\">Completa</span> "+task+" ("+src+","+options+")<br>";
		break;
	    case '3': // done, failed
		ended = (x[i].getElementsByTagName("ended")[0].childNodes[0].nodeValue);
		html += ended + " <span class=\"ticker" + state + "\">Falha</span> "+task+" ("+src+","+options+")<br>";
		break;
	    }

	}

	if (n == 0) {
	    t.style.visibility = "hidden";
	    ta.style.visibility = "hidden";
	    t.innerHTML = '';
	} else {
	    t.innerHTML = html;
	    t.style.visibility = "visible";
	    ta.style.visibility = "visible";

	    if (ta.innerHTML.indexOf('adown') != -1)
		t.style.visibility = 'hidden';
	}


	nb = (xml.getElementsByTagName('basket')[0].childNodes[0].nodeValue);
	np = (xml.getElementsByTagName('packages')[0].childNodes[0].nodeValue);
	updateBasket(nb,np);
    }

    setTimeout("updateTicker("+uid+")",5000);

}

function updateBasket(nb, np) {
    var x = document.getElementById('numcesta');
    var y = document.getElementById('numpacotes');
    var c = document.getElementById('conteudocesta');
    var html;

    if (x.innerHTML != nb || y.innerHTML != np) {
	x.innerHTML = nb;
	y.innerHTML = np;
	if (c != null)
	    c.innerHTML = "<span style=\"color:red; font-weight: bold\">Conteúdo e pacotes modificados, recarregue a a página.</span><br>";

    }

}

function flipTicker() {
    var x = document.getElementById('tickerarrow');
    var y = document.getElementById('ticker');
    
    html = x.innerHTML;
    
    if (html.indexOf('aup') != -1) {
	x.innerHTML = "<img border=0 src=\"adown9.png\" onclick=\"javascript:flipTicker()\">";
	y.style.visibility = 'hidden';
    } else {
	x.innerHTML = "<img border=0 src=\"aup9.png\" onclick=\"javascript:flipTicker()\">";
	y.style.visibility = 'visible';
    }

}