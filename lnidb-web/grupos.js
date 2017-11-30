
function rmgroup(url) {
    if (confirm("Confirma a remoção deste grupo ? Esta operação não poderá ser desfeita.")) {
	var x = window.location.href;
	window.location = window.location.href + url;
    }
}

function cleangroup(url) {
    if (confirm("Confirma a remoção de todos os estudos do grupo ? Esta operação não poderá ser desfeita.")) {
	var x = window.location.href;
	window.location = window.location.href + url;
    }
}

function newgroup() {
    document.getElementById('newgroupbox').style.visibility = "visible";
    document.getElementById('groupform1').focus();
}

function newgroupcancel() {
    document.getElementById('newgroupbox').style.visibility = "hidden";
}

function grantuser() {
    document.getElementById('grantbox').style.visibility = "visible";
}

function revokeuser() {
    document.getElementById('revokebox').style.visibility = "visible";
}

function cancelgrant() {
    document.getElementById('grantbox').style.visibility = "hidden";
}

function cancelrevoke() {
    document.getElementById('revokebox').style.visibility = "hidden";
}

