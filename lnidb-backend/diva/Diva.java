import java.awt.*;
import java.util.*;
import javax.swing.*;

public class Diva extends JApplet {

    JFrame f;
    public static int BufferDebug = 0; 

    public static String METHOD="n/a", FIDS="";
    public static int W=-1,H=-1,D=-1,SIZE=-1,SID=-1;

    public static int StudyCount=0;
    public static String PATNAME="",PATCODE="",PATAGEGENDER="";
    public static Study [] STUDIES;

    public void init() {
	String fids, url, dim, size, par;
	StringTokenizer t;
	int i,w,h,d,len=1;
	fids = getParameter("fids");
	url  = getParameter("method");
	dim  = getParameter("dim");
	size = getParameter("size");
	t = new StringTokenizer(dim," \r\t\n");
	w = Integer.parseInt(t.nextToken());
	h = Integer.parseInt(t.nextToken());
	d = Integer.parseInt(t.nextToken());
	if (size != null)
	    len = Integer.parseInt(size);

	par = getParameter("bufferdebug");
	if (par!=null) Diva.BufferDebug = Integer.parseInt(par);

	Diva.METHOD = url;
	Diva.FIDS   = fids;
	Diva.W = w;
	Diva.H = h;
	Diva.D = d;
	Diva.SIZE = len;
	Diva.SID  = intParameter("sid");

	Diva.StudyCount   = intParameter("studyn");
	Diva.PATNAME      = getParameter("patname");
	Diva.PATCODE      = getParameter("patcode");
	Diva.PATAGEGENDER = getParameter("patagegender");

	Diva.STUDIES = new Study[Diva.StudyCount];
	for(i=0;i<Diva.StudyCount;i++) {
	    STUDIES[i] = new Study();
	    STUDIES[i].id         = intParameter("studyid"+i);
	    STUDIES[i].date       = getParameter("studydate"+i);
	    STUDIES[i].exam       = getParameter("studyexam"+i);
	    STUDIES[i].series     = getParameter("studyseries"+i);
	    STUDIES[i].files      = intParameter("studyfiles"+i);
	    STUDIES[i].dimension  = getParameter("studydim"+i);
	    STUDIES[i].voxelSize  = getParameter("studyvox"+i);
	    STUDIES[i].thickness  = getParameter("studythk"+i);
	    STUDIES[i].scannerMaker     = getParameter("scannermaker"+i);
	    STUDIES[i].scannerModel     = getParameter("scannermodel"+i);
	    STUDIES[i].scannerLocation  = getParameter("scannerlocation"+i);
	    STUDIES[i].size        = intParameter("studysize"+i);
	    STUDIES[i].orientation = getParameter("studyorient"+i);
	    STUDIES[i].fids        = getParameter("studyfids"+i);
	}

	f = new DivaWindow(this, fids, url, w, h, d, len, Diva.SID);
    }

    private int intParameter(String s) {
	if (getParameter(s) == null) return 0;
	return(Integer.parseInt(getParameter(s)));
    }

    public void paint(Graphics g) {
	g.setColor(Color.WHITE);
	g.fillRect(0,0,100,100);
    }

}