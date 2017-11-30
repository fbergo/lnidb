<script type="text/javascript">

function distclick() {
	 var histodiv = document.getElementById("distdiv");
	 var arrowimg = document.getElementById("distarrow");
	 if (histodiv.style.visibility != "visible") {
	    histodiv.style.visibility = "visible";
	    arrowimg.src = "bleft.png";
	 } else {
	    histodiv.style.visibility = "hidden";
	    arrowimg.src = "bright.png";
	 }
}

</script>
