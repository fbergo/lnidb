import java.io.*;
import java.net.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.awt.geom.*;
import javax.swing.*;
import javax.swing.border.*;

public class DivaWindow extends JFrame implements WindowListener {

    Diva myApp;
    String pfids, pmethod;
    int sid;
    DivaLoader ldr;
    Volume vol;
    ScanInfo info;
    DivaPanel dp;
    int dots;
    int status;
    String failreason;
    ProgressStatus pstatus;
    int reloadIndex;
    JFrame details;
    boolean goodbye;

    static final String build = "2015.01.25/A";

    public DivaWindow(Diva app, String fids, String method,int w,int h, int d, int len, int psid) {
	super("LNIDB/DIVA Applet");
	int ri;

	//System.out.println("v2");

	myApp = app;
	pfids = fids;
	pmethod = method;
	dots = 0;
	vol = null;
	status = 0;
	failreason = "";
	sid = psid;
	pstatus = new ProgressStatus(len);
	reloadIndex = -1;
	goodbye = false;
	details = null;

	setSize(800,600);
        setDefaultCloseOperation(DISPOSE_ON_CLOSE);

	ClassLoader cldr = this.getClass().getClassLoader();
	URL url = cldr.getResource("diva.png");
	setIconImage(new ImageIcon(url).getImage());

        JPanel tp = new JPanel();
        tp.setLayout(new BorderLayout());
        getContentPane().add(tp);

	setFocusTraversalKeys(KeyboardFocusManager.FORWARD_TRAVERSAL_KEYS, null);
	setFocusTraversalKeys(KeyboardFocusManager.BACKWARD_TRAVERSAL_KEYS, null);

	dp = new DivaPanel();
	addKeyListener(dp);
	addWindowListener(this);
        
        tp.add(dp,BorderLayout.CENTER);
	setVisible(true);

	ldr = null;
	try {
	    ldr = new DivaLoader(pfids,pmethod,w,h,d,pstatus);
	    if (ldr == null || ldr.vol == null) {
		failreason = "Erro de alocação em DivaLoader.";
		status = -1;
		repaint();
		return;
	    }
	    Thread t = new Thread(ldr);
	    t.start();
	} catch(Exception e) {
	    failreason = new String("Exceção (2):" + e.toString());
	    status = -1;
	    repaint();
	}

	while(status==0 && !ldr.done) {
	    try { Thread.sleep(100); } catch(Exception e) { }
	    dots = (dots + 1) % 16;
	    repaint();
	}

	if (status == 0 && ldr.done && ldr.failed) {
	    status = -1;
	    failreason = ldr.failreason;
	    vol = null;
	    info = null;
	    repaint();
	}

	if (status == 0 && ldr.done && !ldr.failed) {
	    status = 1;
	    vol = ldr.vol;
	    info = ldr.info;
	    ldr = null;
	    repaint();
	}

	if (info != null)
	    setTitle("DIVA - ["+info.name+": "+info.series+" ]");

	for(;;) {
	    while(reloadIndex < 0 && !goodbye) try { Thread.sleep(300); } catch(Exception e) { }
	    if (goodbye) return;
	    ri = reloadIndex;
	    reloadIndex = -1;
	    loadStudy(ri);
		
	    if (info != null)
		setTitle("DIVA - ["+info.name+": "+info.series+" ]");
	    else
		setTitle("LNIDB/DIVA Applet");
	}
    }

    // new frame, works fine
    private void loadStudy2(int index) {
	JFrame f;
	int w,h,d;
	StringTokenizer k;

	k = new StringTokenizer(Diva.STUDIES[index].dimension," ,;xX\r\t\n");
	w = Integer.parseInt(k.nextToken());
	h = Integer.parseInt(k.nextToken());
	d = Integer.parseInt(k.nextToken());

	f = new DivaWindow(myApp,Diva.STUDIES[index].fids, pmethod, w, h, d, 
			   Diva.STUDIES[index].size, 
			   Diva.STUDIES[index].id);
	
    }

    private void loadStudy(int index) {
	int w,h,d,len;
	StringTokenizer k;

	pfids = Diva.STUDIES[index].fids;
	sid = Diva.STUDIES[index].id;
	len = Diva.STUDIES[index].size;

	k = new StringTokenizer(Diva.STUDIES[index].dimension," ,;xX\r\t\n");
	w = Integer.parseInt(k.nextToken());
	h = Integer.parseInt(k.nextToken());
	d = Integer.parseInt(k.nextToken());

	dp.unprepare();
	dots = 0;
	status = 0;
	vol = null;
	failreason = "";

	pstatus.setTotal(len);
	pstatus.setDone(0);

	ldr = null;

	try {
	    ldr = new DivaLoader(pfids,pmethod,w,h,d,pstatus);
	    if (ldr == null || ldr.vol == null) {
		failreason = "Erro de alocação em DivaLoader.";
		status = -1;
		dp.revalidate();
		dp.repaint();
		return;
	    }
	    Thread t = new Thread(ldr);
	    t.start();
	} catch(Exception e) {
	    failreason = new String("Exceção (2):" + e.toString());
	    status = -1;
	    dp.revalidate();
	    dp.repaint();
	}

	while(status==0 && !ldr.done) {
	    try { Thread.sleep(100); } catch(Exception e) { }
	    dots = (dots + 1) % 16;
	    repaint();
	}

	if (status == 0 && ldr.done && ldr.failed) {
	    status = -1;
	    failreason = ldr.failreason;
	    vol = null;
	    info = null;
	    repaint();
	}

	if (status == 0 && ldr.done && !ldr.failed) {
	    status = 1;
	    vol = ldr.vol;
	    info = ldr.info;
	    ldr = null;
	    repaint();
	}

	if (status != 1) goodbye = true;
    }

    public void windowActivated(WindowEvent e) { }
    public void windowClosed(WindowEvent e) { goodbye = true; }
    public void windowClosing(WindowEvent e) { goodbye = true; }
    public void windowDeactivated(WindowEvent e) { }
    public void windowDeiconified(WindowEvent e) { }
    public void windowIconified(WindowEvent e) { }
    public void windowOpened(WindowEvent e)  { }

    public class DivaPanel extends JPanel 
	implements MouseListener, MouseMotionListener, MouseWheelListener, KeyListener, FocusListener {

	Font f,f2,f3,f4,f5;
	int cx,cy,cz,px,py,panx,pany,rowy,ncols;
	int [] bx, by, bw, bh;
	int sx1, sx2;
	int rpanex, rpanew;
	float zoom, userzoom, maxuserzoom;
	float [] basezoom;
	int wmin, wmax, vmax, vmin;
	boolean prepared, showcursor, interpolate;
	ViewBuffer [] vb;
	ViewBufferCache vbc;
	Image panctl, check, slicectl, colctl, updownctl, detailctl;
	Image [] toolctl, viewctl;
	double rfx, rfy, rfz;
	int panefocus;
	int viewmode, studysel, studyscroll;

	DivaPanel() {
	    int i;
	    sx1=sx2=0;
	    prepared = false;
	    f = new Font(Font.SANS_SERIF, Font.PLAIN, 12);
	    f2 = new Font(Font.SANS_SERIF, Font.PLAIN, 10);
	    f3 = new Font(Font.SANS_SERIF, Font.BOLD, 10);
	    f4 = new Font(Font.SANS_SERIF, Font.ITALIC, 9);
	    f5 = new Font(Font.SANS_SERIF, Font.PLAIN, 10);
	    // 3 panes, window control, cursor, zoom control, pan control, interpolate, slice ctl(3)
	    bx = new int[16];
	    by = new int[16];
	    bw = new int[16];
	    bh = new int[16];
	    vb = new ViewBuffer[3];
	    vb[0] = new ViewBuffer();
	    vb[1] = new ViewBuffer();
	    vb[2] = new ViewBuffer();
	    wmin = 0;
	    vmin = 0;
	    wmax = 4095;
	    userzoom = 1.0f;
	    basezoom = new float[4];
	    basezoom[0] = basezoom[1] = basezoom[2] = basezoom[3] = 1.0f;
	    maxuserzoom = 8.0f;
	    rpanew = 280;
	    panx = pany = rowy = 0;
	    rfx = rfy = rfz = 1.0;
	    showcursor = true;
	    interpolate = false;
	    panefocus = 0;
	    viewmode = 0;
	    ncols = 4;
	    studysel = 0;
	    studyscroll = 0;
	    addKeyListener(this);
	    addMouseListener(this);
	    addMouseWheelListener(this);
	    addMouseMotionListener(this);

	    setFocusable(true);
	    addFocusListener(this);
	    setFocusTraversalKeys(KeyboardFocusManager.FORWARD_TRAVERSAL_KEYS, null);
	    setFocusTraversalKeys(KeyboardFocusManager.BACKWARD_TRAVERSAL_KEYS, null);

	    ClassLoader cldr = this.getClass().getClassLoader();
	    URL url = cldr.getResource("panctl.png");
	    panctl = new ImageIcon(url).getImage();
	    url = cldr.getResource("check.png");
	    check = new ImageIcon(url).getImage();
	    url = cldr.getResource("slicectl.png");
	    slicectl = new ImageIcon(url).getImage();
	    url = cldr.getResource("colctl.png");
	    colctl = new ImageIcon(url).getImage();
	    url = cldr.getResource("updown.png");
	    updownctl = new ImageIcon(url).getImage();
	    url = cldr.getResource("detail.png");
	    detailctl = new ImageIcon(url).getImage();

	    toolctl = new Image[2];
	    viewctl = new Image[4];
	    for(i=0;i<2;i++) {
		url = cldr.getResource("tool" + (i+1) + ".png");
		toolctl[i] = new ImageIcon(url).getImage();
	    }
	    for(i=0;i<=3;i++) {
		url = cldr.getResource("view" + (i+1) + ".png");
		viewctl[i] = new ImageIcon(url).getImage();
	    }
	}

	public void unprepare() {
	    prepared = false;
	}

	public void prepare() {
	    cx = vol.W / 2;
	    cy = vol.H / 2;
	    cz = vol.D / 2;
	    wmin = vmin = (int) vol.minimum();
	    wmax = vmax = (int) vol.maximum();
	    basezoom[0] = basezoom[1] = basezoom[2] = 1.0f;
	    userzoom = 1.0f;
	    prepared = true;
	    viewmode = 0;
	    rowy = 0;
	    panx = 0;
	    pany = 0;
	    if (vol.D < 40) viewmode=2;
	    if (vol.D > 500) viewmode=1;
	    vbc = new ViewBufferCache(128);
	    //System.out.println("wmax="+wmax);
	}

	private int [] lookup;

	public void computeLookup() {
	    int i;
	    lookup = new int[vmax-vmin+1];
	    for(i=vmin;i<=vmax;i++)
		lookup[i-vmin] = pixelvalue((short) i);
	}

	public int pixelvalue(short v) {
	    int x;
	    if (v <= wmin) return 0;
	    if (v >= wmax) {
		//System.out.println("wmax=" + wmax + ",v=" + v);
		return 0xffffff;
	    }
	    if (wmax <= wmin) return 0x808080;
	    x = ((v-wmin)*255)/(wmax-wmin);
	    x = (x<<16)|(x<<8)|x;
	    return x;
	}

	public int pixelvalue_lu(short v) {
	    if (v < vmin || v > vmax) return 0;
	    return(lookup[v-vmin]);
	}

	public void zoomToFit() {
	    int pw,ph;
	    int fw,fh,vw,vh;
	    float zw,zh;
	    double res;
	    int W,H,D;
	    pw = rpanex;
	    ph = getHeight();

	    if (pw < 0) pw = 40;

	    if (viewmode == 0 && interpolate) {
		res = vol.maxres();
		W = vol.interpW(res);
		H = vol.interpH(res);
		D = vol.interpD(res);
		rfx = vol.dx / res;
		rfy = vol.dy / res;
		rfz = vol.dz / res;
	    } else {
		W = vol.W;
		H = vol.H;
		D = vol.D;
		rfx = rfy = rfz = 1.0;
	    }

	    // mode 0
	    fw = fh = 30;
	    vw = W+D;
	    vh = H+D;
	    zw = ((float)  pw-fw) / ((float) vw);
	    zh = ((float)  ph-fh) / ((float) vh);
	    if (zw > zh) basezoom[0] = zh; else basezoom[0] = zw;
	    
	    // mode 1
	    fw = fh = 20;
	    vw = W;
	    vh = H;
	    zw = ((float)  pw-fw) / ((float) vw);
	    zh = ((float)  ph-fh) / ((float) vh);
	    if (zw > zh) basezoom[1] = zh; else basezoom[1] = zw;

	    // mode 2
	    fw = ncols*1 + 2 + 30;
	    fh = 10;
	    vw = ncols*W;
	    vh = H;
	    zw = ((float)  pw-fw) / ((float) vw);
	    zh = ((float)  ph-fh) / ((float) vh);
	    if (zw > zh) basezoom[2] = zh; else basezoom[2] = zw;

	    zoom = basezoom[viewmode]*userzoom;

	    switch(viewmode) {
	    case 0:
		bx[0] = 10; by[0] = 10; 
		bw[0] = (int) (zoom * W); bh[0] = (int) (zoom * H);

		bx[1] = bx[0] + bw[0] + 10; by[1] = 10; 
		bw[1] = (int) (zoom * D); bh[1] = (int) (zoom * H);

		bx[2] = 10; by[2] = by[0] + bh[0] + 10; 
		bw[2] = (int) (zoom * W); bh[2] = (int) (zoom * D);
		break;

	    case 1:
		bx[0] = 10; by[0] = 10; 
		bw[0] = (int) (zoom * W); bh[0] = (int) (zoom * H);

		bx[1] = bx[2] = by[1] = by[2] = 0;
		bw[1] = bw[2] = bh[1] = bh[2] = -1;
		break;

	    case 2:
	    case 3:
		bx[0] = 1; by[0] = 1; 
		bw[0] = (int) (zoom * W); bh[0] = (int) (zoom * H);

		bx[1] = bx[2] = by[1] = by[2] = 0;
		bw[1] = bw[2] = bh[1] = bh[2] = -1;
		break;

	    }

	    bx[0] += panx; bx[1] += panx; bx[2] += panx;
	    by[0] += pany; by[1] += pany; by[2] += pany;
	}

	private void fadeBox(Graphics g, Color outter, Color inner, int len, int x,int y,int w,int h) {
	    float [] fo, fi;
	    float pct;
	    int i;

	    fo = outter.getRGBColorComponents(null);
	    fi = inner.getRGBColorComponents(null);

	    for(i=0;i<len;i++) {
		pct = ((float) (i+1)) / ((float)len);
		g.setColor(new Color( 
				     fi[0]*pct + fo[0]*(1.0f-pct),
				     fi[1]*pct + fo[1]*(1.0f-pct),
				     fi[2]*pct + fo[2]*(1.0f-pct) ));
		g.drawRect(x-i,y-i,w+i*2,h+i*2);
	    }

	}
	
	private int stringWidth(Graphics g, String s) {
	    FontMetrics fm;
	    Rectangle2D r2;
	    fm = g.getFontMetrics(g.getFont());
	    r2 = fm.getStringBounds(s, g);
	    return((int)(r2.getWidth()));
	}

	private int stringHeight(Graphics g, String s) {
	    FontMetrics fm;
	    Rectangle2D r2;
	    fm = g.getFontMetrics(g.getFont());
	    r2 = fm.getStringBounds(s, g);
	    return((int)(r2.getHeight()));
	}

	private String humanSize(int sz) {
	    final String [] HumanUnits = { "", "K", "M", "G", "T", "P" };
	    int u = 0;
	    while (sz > 4000) { sz /= 1024; u++; }
	    if (u > 5) return(new String("Enorme"));
	    return(new String(""+sz+HumanUnits[u]));
	}

	private void openDicomDetails() {
	    if (details != null) {
		details.dispose();
		details = null;
	    }
	    details = new DicomDetailsWindow(info);
	}

	// blues: 0x0e2c33 0x0e4652 0x2c89a0 0x80d1e5
	private void paintMode3(Graphics g) { 
	    int ry, rx, w, h, i;
	    Study s;
	    String tmp;

	    g.setColor(new Color(0x0e2c33));
	    g.fillRect(0,0,rpanex,50);
	    w = getWidth();
	    h = getHeight();

	    g.setColor(new Color(0x0e2c33));
	    g.fillRect(0,50,rpanex,h-50);

	    rx = 10;
	    ry = 15;
	    g.setColor(Color.WHITE);
	    g.drawString(Diva.PATNAME+" (HC "+Diva.PATCODE+"), "+Diva.PATAGEGENDER,rx,ry);

	    ry += 15;
	    if (Diva.StudyCount > 0) {
		g.drawString(Diva.STUDIES[0].scannerMaker+" "+Diva.STUDIES[0].scannerModel+" / "+Diva.STUDIES[0].scannerLocation,10,ry);
		g.setColor(Color.YELLOW);
		ry+=15;
		if (Diva.StudyCount == 1)
		    g.drawString(""+Diva.StudyCount+" série em "+Diva.STUDIES[0].date(),10,ry);
		else
		    g.drawString(""+Diva.StudyCount+" séries em "+Diva.STUDIES[0].date(),10,ry);
	    } else {
		g.setColor(Color.YELLOW);
		g.drawString("Nenhuma série associada foi especificada.",10,ry);
	    }

	    while (50 + 18*(studysel - studyscroll) < 50) studyscroll--;
	    while (50 + 18*(studysel - studyscroll) > h-18) studyscroll++;

	    for(i=0;i<Diva.StudyCount;i++) {
		s = Diva.STUDIES[i];
		rx = 0;
		ry = 50 + 18*(i-studyscroll);
		if (ry > h || ry < 50) continue;

		if (s.id == sid) g.setColor(new Color(0xaaaaaa)); else g.setColor(new Color(0xdddddd));
		if (studysel == i) g.setColor(new Color(0xeeaa44));
		g.fillRect(rx,ry,rpanex,18-1);
		if (studysel == i) {
		    g.setColor(new Color(0xff8800));
		    g.drawRect(rx,ry,rpanex-1,18-2);
		}

		rx = 25;
		g.setFont(f3); g.setColor(Color.BLACK);
		tmp = ""+(i+1);
		g.drawString(tmp,rx-stringWidth(g,tmp),ry+12);
		rx += 5;
		g.setFont(f2); g.setColor(new Color(0x006655));
		g.drawString(s.exam,rx,ry+12);
		rx += stringWidth(g,s.exam) + 5;
		g.setFont(f2); g.setColor(new Color(0x880088));
		g.drawString(s.time(),rx,ry+12);
		rx += stringWidth(g,s.time()) + 5;
		g.setFont(f3); g.setColor(Color.BLACK);
		g.drawString(s.series,rx,ry+12);
		rx += stringWidth(g,s.series) + 5;
		g.setFont(f2); g.setColor(new Color(0x880088));
		g.drawString(s.orientation,rx,ry+12);
		rx += stringWidth(g,s.orientation) + 5;
		g.setFont(f4); g.setColor(new Color(0x444488));
		g.drawString(tmp="("+s.dimension+"@"+s.voxelSize+"/"+s.thickness+")",rx,ry+12);
		rx += stringWidth(g,tmp) + 5;
		g.setFont(f2); g.setColor(new Color(0x880044));
		g.drawString(tmp=humanSize(s.size),rx,ry+12);
		rx += stringWidth(g,tmp) + 5;



	    }
	    g.setFont(f);
	}

	// mosaic
	private void paintMode2(Graphics g) { 
	    int i,j,k,pj,mx,my,rx,ry,w,h,mink,maxk;
	    BufferedImage xy;
	    Image q;
	    w = getWidth();
	    h = getHeight();
	    computeLookup();
	    
	    mink = vol.D; maxk = 0;
	    for(k=0;k<vol.D;k++) {
		mx = k%ncols;
		my = k/ncols;
		rx = panx + bx[0] + mx * (bw[0] + 1);
		ry = by[0] + (my-rowy) * (bh[0] + 1);

		// out of bounds
		if ( (rx > rpanex) || (ry > h) || (rx + bw[0] <= 0) || 
		     (ry + bh[0] <= 0) || (bw[0] <= 0) || (bh[0] <= 0) )
		    continue;
		
		if (k < mink) mink = k;
		if (k > maxk) maxk = k;

		q = vbc.query(bw[0],bh[0],k);
		if (q == null) {
		    xy = new BufferedImage(vol.W,vol.H,BufferedImage.TYPE_INT_RGB);
		    for(j=0;j<vol.H;j++)
			for(i=0;i<vol.W;i++)
			    xy.setRGB(i,j,pixelvalue_lu(vol.get(i,j,k)));
		    q = xy.getScaledInstance(bw[0],bh[0],Image.SCALE_SMOOTH);
		    vbc.store(q,bw[0],bh[0],k);
		}
		g.drawImage(q, rx, ry, null);

		if (showcursor && bw[0] > 30 && bh[0] > 25) {
		    g.setColor(Color.GREEN);
		    if (info.orientationDefined(0)) {
			g.drawString(info.ox.substring(0,1), rx+5,ry+bh[0]/2);
			g.drawString(info.ox.substring(1,2), rx+bw[0]-15,ry+bh[0]/2);
		    }
		    if (info.orientationDefined(1)) {
			g.drawString(info.oy.substring(0,1), rx+bw[0]/2,ry+15);
			g.drawString(info.oy.substring(1,2), rx+bw[0]/2,ry+bh[0]-5);
		    }
		    g.drawString(""+(k+1)+"/"+vol.D,rx+5,ry+15);
		    if (info.orientationDefined(2))
			g.drawString(""+info.oz.substring(0,1)+"\u2192"+info.oz.substring(1,2),rx+5,ry+30);
		}
	    }

	    // scroll bar
	    bx[1] = rpanex - 30 + 4;
	    by[1] = 10;
	    bw[1] = 21;
	    bh[1] = h-20;
	    g.setColor(new Color(0x555555));
	    g.fillRect(bx[1],by[1],bw[1],bh[1]);
	    g.setColor(new Color(0x08b8d8));
	    i = (mink * bh[1]) / vol.D;
	    j = (((maxk+1) * bh[1]) / vol.D) - 1;
	    g.fillRect(bx[1],by[1]+i,bw[1],j-i+1);
	    for(i=0,pj=-10;i<vol.D;i++) {
		if (i < mink || i > maxk) g.setColor(new Color(0x333333)); else g.setColor(new Color(0x02343d));
		j = by[1] + (i*bh[1]) / vol.D;
		if (j-pj>1) {
		    g.drawLine(bx[1],j,bx[1]+ ((i%ncols == 0) ? bw[1] : bw[1]/2),j);
		    pj = j;
		}
		if (Diva.BufferDebug!=0) {
		    if (vbc.has(i)) {
			g.setColor(new Color(0x00ff00));
			g.drawLine(bx[1]+bw[1]/2,j,bx[1]+ bw[1],j);
		    }
		}
	    }
	    g.setColor(Color.BLACK);
	    g.drawRect(bx[1],by[1],bw[1],bh[1]);

	    if (Diva.BufferDebug!=0)
		vbc.print();
	}

	// single pane
	private void paintMode1(Graphics g) { 
	    int i,j;
	    BufferedImage xy;
	    computeLookup();

	    if (bw[0]>0 && bh[0]>0)
		if (!vb[0].valid(bw[0],bh[0],cz)) {
		    xy = new BufferedImage(vol.W,vol.H,BufferedImage.TYPE_INT_RGB);
		    for(j=0;j<vol.H;j++)
			for(i=0;i<vol.W;i++)
			    xy.setRGB(i,j,pixelvalue_lu(vol.get(i,j,cz)));
		    vb[0].set(xy.getScaledInstance(bw[0],bh[0],Image.SCALE_SMOOTH),bw[0],bh[0],cz);
		}

	    if (bw[0] > 0 && bh[0] > 0){
		g.setColor(new Color(0xffff80));
		g.fillRect(bx[0]-4,by[0]-4,
			   vb[0].getWidth() + 8,
			   vb[0].getHeight() + 8);
		
		g.drawImage(vb[0].getImage(), bx[0], by[0], null);
	    }

	    if (showcursor) {
		g.setColor(Color.GREEN);
		i = bx[0] + (int)(cx * zoom * rfx);
		g.drawLine(i,by[0],i,by[0]+bh[0]);
		i = by[0] + (int)(cy * zoom * rfy);
		g.drawLine(bx[0],i,bx[0]+bw[0],i);
		
		g.setColor(Color.GREEN);
		if (info.orientationDefined(0)) {
		    g.drawString(info.ox.substring(0,1), bx[0]+5,by[0]+bh[0]/2);
		    g.drawString(info.ox.substring(1,2), bx[0]+bw[0]-15,by[0]+bh[0]/2);
		}
		if (info.orientationDefined(1)) {
		    g.drawString(info.oy.substring(0,1), bx[0]+bw[0]/2,by[0]+15);
		    g.drawString(info.oy.substring(1,2), bx[0]+bw[0]/2,by[0]+bh[0]-5);
		}
		g.drawString(""+(cz+1)+"/"+vol.D,bx[0]+5,by[0]+15);
		if (info.orientationDefined(2))
		    g.drawString(""+info.oz.substring(0,1)+"\u2192"+info.oz.substring(1,2),bx[0]+5,by[0]+30);
	    }
	}

	// 3 panes
	private void paintMode0(Graphics g) {
	    int i,j;
	    BufferedImage xy,zy,xz;
	    computeLookup();

	    if (bw[0]>0 && bh[0]>0)
		if (!vb[0].valid(bw[0],bh[0],cz)) {
		    xy = new BufferedImage(vol.W,vol.H,BufferedImage.TYPE_INT_RGB);
		    for(j=0;j<vol.H;j++)
			for(i=0;i<vol.W;i++)
			    xy.setRGB(i,j,pixelvalue_lu(vol.get(i,j,cz)));
		    vb[0].set(xy.getScaledInstance(bw[0],bh[0],Image.SCALE_SMOOTH),bw[0],bh[0],cz);
		    //System.out.println("invalid 0");
		}
	    
	    if (bw[1]>0 && bh[1]>0)
		if (!vb[1].valid(bw[1],bh[1],cx)) {
		    zy = new BufferedImage(vol.D,vol.H,BufferedImage.TYPE_INT_RGB);
		    for(j=0;j<vol.H;j++)
			    for(i=0;i<vol.D;i++)
				zy.setRGB(i,j,pixelvalue_lu(vol.get(cx,j,i)));
		    vb[1].set(zy.getScaledInstance(bw[1],bh[1],Image.SCALE_SMOOTH),bw[1],bh[1],cx);
		    //System.out.println("invalid 1");
		    }
	    
	    if (bw[2]>0 && bh[2]>0)
		if (!vb[2].valid(bw[2],bh[2],cy)) {
		    xz = new BufferedImage(vol.W,vol.D,BufferedImage.TYPE_INT_RGB);
		    for(j=0;j<vol.D;j++)
			for(i=0;i<vol.W;i++)
			    xz.setRGB(i,j,pixelvalue_lu(vol.get(i,cy,vol.D-j-1)));
		    vb[2].set(xz.getScaledInstance(bw[2],bh[2],Image.SCALE_SMOOTH),bw[2],bh[2],cy);
		    //System.out.println("invalid 2");
		}
	    
	    if (bw[0] > 0 && bw[1] > 0 && bw[2] > 0 &&
		bh[0] > 0 && bh[1] > 0 && bh[2] > 0) {
		
		//g.setColor(new Color(0x77cfe5));
		g.setColor(new Color(0xffff80));
		g.fillRect(bx[panefocus]-4,by[panefocus]-4,
			   vb[panefocus].getWidth() + 8,
			   vb[panefocus].getHeight() + 8);
		
		g.drawImage(vb[0].getImage(), bx[0], by[0], null);
		g.drawImage(vb[1].getImage(), bx[1], by[1], null);
		g.drawImage(vb[2].getImage(), bx[2], by[2], null);
		
	    }
	    
	    if (showcursor) {
		g.setColor(Color.GREEN);
		i = bx[0] + (int)(cx * zoom * rfx);
		g.drawLine(i,by[0],i,by[0]+bh[0]);
		g.drawLine(i,by[2],i,by[2]+bh[2]);
		i = by[0] + (int)(cy * zoom * rfy);
		g.drawLine(bx[0],i,bx[0]+bw[0],i);
		g.drawLine(bx[1],i,bx[1]+bw[1],i);
		i = (int)(cz * zoom * rfz);
		g.drawLine(bx[2],by[2]+bh[2]-i-1,bx[2]+bw[2],by[2]+bh[2]-i-1);
		g.drawLine(bx[1]+i,by[1],bx[1]+i,by[1]+bh[1]);
		
		g.setColor(Color.GREEN);
		if (info.orientationDefined(0)) {
		    g.drawString(info.ox.substring(0,1), bx[0]+5,by[0]+bh[0]/2);
		    g.drawString(info.ox.substring(1,2), bx[0]+bw[0]-15,by[0]+bh[0]/2);
		    g.drawString(info.ox.substring(0,1), bx[2]+5,by[2]+bh[2]/2);
		    g.drawString(info.ox.substring(1,2), bx[2]+bw[2]-15,by[2]+bh[2]/2);
		}
		if (info.orientationDefined(1)) {
		    g.drawString(info.oy.substring(0,1), bx[0]+bw[0]/2,by[0]+15);
		    g.drawString(info.oy.substring(1,2), bx[0]+bw[0]/2,by[0]+bh[0]-5);
		    g.drawString(info.oy.substring(0,1), bx[1]+bw[1]/2,by[1]+15);
		    g.drawString(info.oy.substring(1,2), bx[1]+bw[1]/2,by[1]+bh[1]-5);
		}
		if (info.orientationDefined(2)) {
		    g.drawString(info.oz.substring(0,1), bx[1]+5,by[1]+bh[1]/2);
		    g.drawString(info.oz.substring(1,2), bx[1]+bw[1]-15,by[1]+bh[1]/2);
		    // inverted
		    g.drawString(info.oz.substring(1,2), bx[2]+bw[2]/2,by[2]+15);
		    g.drawString(info.oz.substring(0,1), bx[2]+bw[2]/2,by[2]+bh[2]-5);
		}
	    }	       		
	}

	public void paint(Graphics g) {
	    int w,h,pp,i,j,k;
	    double res;
	    int rx, ry, tx, tt, tc;
	    Color bgcolor;
	    String tmp;

	    w = getWidth();
	    h = getHeight();
	    
	    //System.out.println("paint status="+status);

	    ((Graphics2D) g).setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING,
					      RenderingHints.VALUE_TEXT_ANTIALIAS_GASP);
	    requestFocusInWindow();

	    bgcolor = new Color(0x2c89a0);
	    g.setColor(bgcolor);

	    g.fillRect(0,0,w,h);
	    g.setFont(f);

	    if (status <= 0 && ldr != null) {
		StringTokenizer st = new StringTokenizer(ldr.livestatus.toString(),"\n");
		//g.setColor(new Color(0x0e4652));
		g.setColor(Color.YELLOW);
		rx = (h/16)-4; if (rx < 1) rx = 1;
		tc = st.countTokens();
		while(tc > rx) { st.nextToken(); tc--; }
		for(ry=20;st.hasMoreTokens();ry+=16) {
		    g.drawString(st.nextToken(), 10, ry);
		}
	    }
	    g.setColor(Color.WHITE);
	    if (status<0) {
		g.drawString("Erro ao carregar as imagens do servidor: " + failreason,10,h-10);
		return;
	    } // status<0 (out of memory or network error)

	    if (status == 0) {
		int bw, perc, tw, th;
		int cx=130, cy=h-45+7, cr=8, cr2 = 2;
		double ang;
		String s;
		Color[] pb;
		g.setColor(Color.WHITE);
		g.fillRect(10,h-45,100,15);

		g.setColor(new Color(0xcc4444));
		bw = (int) (100.0*pstatus.getRatio());
		g.fillRect(10,h-45,bw,15);

		g.setColor(Color.BLACK);
		perc = (int) pstatus.getPercentRatio();
		s = ""+perc+"%";
		tw = stringWidth(g,s);
		th = stringHeight(g,s);
		g.drawString(s,10+(100-tw)/2,(h-45)+th+(15-th)/2);

		g.setColor(Color.BLACK);
		g.drawRect(10,h-45,100,15);

		pb = new Color[4];
		pb[3] = new Color(0xffd900);
		pb[2] = new Color(0xd8ca1e);
		pb[1] = new Color(0x9cb34b);
		pb[0] = new Color(0x6ca170);

		for(i=0;i<8;i++) {
		    ang = - ((dots+i) * (2.0 * Math.PI / 16.0));
		    g.setColor(pb[i/2]);
		    g.fillOval((int) (cx + cr*Math.cos(ang)) - cr2, (int) (cy + cr*Math.sin(ang)) - cr2, 2*cr2, 2*cr2);
		}

		g.setColor(Color.YELLOW);
		g.drawString("Carregando... ",10,h-10);
	    } // status 0 (image loading, no disaster yet)

	    if (status == 1 && vol!=null && info!=null) {
		if (!prepared) prepare();
		rpanex = w - rpanew;
		zoomToFit();

		switch(viewmode) {
		case 0: paintMode0(g); break;
		case 1: paintMode1(g); break;
		case 2: paintMode2(g); break;
		case 3: paintMode3(g); break;
		}

		// view mode
		rx = w - rpanew;
		g.setColor(new Color(0x0e4652));
		g.fillRect(rx,0,rpanew,36);
		
		for(i=0;i<4;i++) {
		    bx[11+i] = rx + 10 + 32*i;
		    by[11+i] = 2;
		    bw[11+i] = 32;
		    bh[11+i] = 32;
		    g.drawImage(toolctl[ viewmode == i ? 1 : 0 ], bx[11+i], by[11+i], null);
		    g.drawImage(viewctl[ i ], bx[11+i], by[11+i], null);
		}

		// patinfo
		if (info.name != null && viewmode < 3) {
		    rx = w - rpanew;
		    ry = 36;
		    g.setColor(new Color(0x135d6c));
		    g.fillRect(rx,ry,rpanew,h-ry);
		    g.setColor(Color.WHITE);
		    if (info.name != null) g.drawString("Nome: " + info.name, rx+10,ry+=17);
		    if (info.hc != null) g.drawString("HC: " + info.hc, rx+10,ry+=17);
		    if (info.scandate != null) g.drawString("Data do Estudo: " + info.scandate, rx+10,ry+=17);
		    if (info.birth != null) g.drawString("Nascimento: " + info.birth, rx+10,ry+=17);
		    if (info.gender != null) g.drawString("Idade/Sexo: " + info.age + "/" + info.gender, rx+10,ry+=17);
		    if (info.scanner1 != null) g.drawString("Scanner: " + info.scanner1, rx+10,ry+=17);
		    if (info.scanner2 != null) g.drawString("  " + info.scanner2, rx+10,ry+=17);
		    if (info.exam != null) g.drawString("Exame: " + info.exam, rx+10,ry+=17);
		    if (info.series != null) g.drawString("Série: " + info.series, rx+10,ry+=17);
		    g.drawString("Dimensões: " + vol.W + " x " + vol.H + " x " + vol.D, rx+10,ry+=17);
		    g.drawString("Voxel: " + r2(vol.dx) + " x " + r2(vol.dy) + " x " + r2(vol.dz), rx+10,ry+=17);
		    g.drawString("Thickness: " + r2(info.thickness), rx+10,ry+=17);

		    bx[15] = w-30; by[15] = ry-17;
		    bw[15] = 21; bh[15] = 21;
		    g.drawImage(detailctl,bx[15],by[15],null);
		}

		// controls (window/zoom/pan)
		// window control
		rx = rpanex;
		ry = 250;
		if (viewmode == 3) ry = 37;
		g.setColor(new Color(0x0e4652));
		g.fillRect(rx,ry,rpanew,h-ry);

		if (viewmode < 3) {
		    g.setColor(Color.WHITE);
		    g.drawString("Intensidade ("+wmin+"\u2192"+wmax+" ["+vmin+"\u2192"+vmax+"])",rx+10,ry+20);

		    bx[3] = rx+20;    by[3] = ry+30;
		    bw[3] = w-rx-30;  bh[3] = 20;

		    tt = ((wmin-vmin) * bw[3]) / (vmax-vmin);
		    tx = ((wmax-vmin) * bw[3]) / (vmax-vmin);
		    sx1 = bx[3] + tt + 2;
		    sx2 = bx[3] + tx + 2;

		    GradientPaint gp = new GradientPaint(sx1, by[3], Color.BLACK,
							 sx2, by[3], Color.WHITE);
		    ((Graphics2D) g).setPaint(gp);
		    ((Graphics2D) g).fill(new Rectangle2D.Double(bx[3],by[3]+4,bw[3],bh[3]-8));
		    
		    g.setColor(Color.BLACK);
		    g.drawRect(bx[3],by[3]+4,bw[3],bh[3]-8);

		    g.setColor(new Color(0xff8800));		    
		    g.fillRect(bx[3] + tt - 2 ,by[3], 5 ,bh[3]);
		    g.fillRect(bx[3] + tx - 2 ,by[3], 5 ,bh[3]);
		    g.setColor(Color.BLACK);
		    g.drawRect(bx[3] + tt - 2 ,by[3], 5 ,bh[3]);
		    g.drawRect(bx[3] + tx - 2 ,by[3], 5 ,bh[3]);
		}

		// series controls
		if (viewmode == 3 && Diva.StudyCount > 0) {
		    int tw,th;
		    g.setColor(Color.WHITE);
		    g.drawString("Seleção: "+(studysel+1)+" ("+Diva.STUDIES[studysel].series+")", rx+10,ry+20);
		    bx[3] = rx+10; by[3] = ry + 25; bw[3] = 21; bh[3] = 40;
		    g.drawImage(updownctl,bx[3],by[3],null);

		    bx[4] = rx+10; by[4] = ry+70; bw[4] = 70; bh[4] = 22;
		    g.setColor(new Color(0xcccccc));
		    g.fillRect(bx[4],by[4],bw[4],bh[4]);
		    g.setColor(Color.BLACK);
		    g.drawRect(bx[4],by[4],bw[4],bh[4]);
		    g.setColor(new Color(0x888888));
		    g.drawRect(bx[4]+1,by[4]+1,bw[4]-2,bh[4]-2);
		    g.setColor(Color.WHITE);
		    g.drawLine(bx[4]+1,by[4]+1,bx[4]+bw[4]-2,by[4]+1);
		    g.drawLine(bx[4]+1,by[4]+1,bx[4]+1,by[4]+bh[4]-2);
		    g.setColor(Color.BLACK);
		    tw = stringWidth(g,"Carregar");
		    th = stringHeight(g,"Carregar");
		    g.drawString("Carregar",bx[4]+(bw[4]-tw)/2,by[4]+bh[4]-(bh[4]-th)/2-2);

		    if (info.loadRateMBs != 0.0) {
			double rr = info.loadRateMBs;
			double mb = info.loadSizeBytes / 1000000.0;
			double mbit = (info.loadRateMBs * 8);
			rr = ((double) ((long) (100.0 * rr))) / 100.0;
			mb = ((double) ((long) (100.0 * mb))) / 100.0;
			mbit = ((double) ((long) (100.0 * mbit))) / 100.0;
			g.setFont(f5);
			g.setColor(Color.YELLOW);
			g.drawString("Última Transferência:",bx[4],h-90);
			g.drawString(""+info.loadSizeBytes+" bytes ("+mb+" MB)",bx[4]+10,h-75);
			g.drawString(""+(info.loadTimeMillis/1000.0)+" segundos",bx[4]+10,h-60);
			g.drawString("Taxa efetiva: "+rr+" MB/s ("+mbit+" Mbps)",bx[4]+10,h-45);
		    }
		}


		// voxel info
		if (viewmode < 2) {
		    g.setColor(Color.WHITE);
		    g.drawString("Voxel(" + cx + "," + cy + "," + cz + ") = " + vol.get(cx,cy,cz),bx[3], by[3] + bh[3] + 20);
		}

		ry += 90;

		// cursor control
		if (viewmode < 3) {
		    bx[4] = rx+10; by[4] = ry; bh[4] = 12; bw[4] = w - bx[4];
		    g.setColor(Color.WHITE);
		    g.fillRect(bx[4],by[4],13,12);
		    g.setColor(Color.BLACK);
		    g.drawRect(bx[4],by[4],13,12);
		
		    if (showcursor)
			g.drawImage(check, bx[4]+2, by[4]+2, null);
		    g.setColor(Color.WHITE);
		    g.drawString("Exibir [C]ursor e Orientação", bx[4] + 20, by[4] + bh[4] - 2);
		}

		// interpolation
		ry += 20;
		bx[7] = rx+10; by[7] = ry; bh[7] = 12; bw[7] = w - bx[4];
		if (viewmode == 0) {
		    g.setColor(Color.WHITE);
		    g.fillRect(bx[7],by[7],13,12);
		    g.setColor(Color.BLACK);
		    g.drawRect(bx[7],by[7],13,12);
		
		    if (interpolate)
			g.drawImage(check, bx[7]+2, by[7]+2, null);
		    g.setColor(Color.WHITE);
		    g.drawString("Exibição [I]sométrica", bx[7] + 20, by[7] + bh[7] - 2);
		}

		// zoom
		ry += 20;
		tx = (int) (zoom * 100.0);

		if (viewmode < 3) {
		    g.setColor(Color.WHITE);
		    g.drawString("Zoom ("+tx+"%)",rx+10,ry+20);
		    
		    bx[5] = rx+20;    by[5] = ry+30;
		    bw[5] = w-rx-30;  bh[5] = 20;
		    
		    g.setColor(Color.GRAY);
		    g.fillRect(bx[5],by[5]+4,bw[5],bh[5]-8);
		    g.setColor(Color.BLACK);
		    g.drawRect(bx[5],by[5]+4,bw[5],bh[5]-8);
		    
		    g.setColor(new Color(0xff8800));
		    tx = (int) (( Math.sqrt(userzoom) * bw[5]) / Math.sqrt(maxuserzoom));
		    g.fillRect(bx[5] + tx - 2 ,by[5], 5 ,bh[5]);
		    g.setColor(Color.BLACK);
		    g.drawRect(bx[5] + tx - 2 ,by[5], 5 ,bh[5]);
		}

		// pan
		ry += 60;

		if (viewmode < 3) {
		    g.setColor(Color.WHITE);
		    switch(viewmode) {
		    case 0:
		    case 1:
			g.drawString("Pan ("+panx+","+pany+")",rx+10,ry+20);
			break;
		    case 2:
			g.drawString("Pan ("+panx+",0), Fatia "+((rowy*ncols)+1),rx+10,ry+20);
			break;
		    }
		    g.drawImage(panctl,rx+20,ry+30,null);
		    bx[6] = rx+20; by[6] = ry+30; bw[6] = 60; bh[6] = 60;
		    
		    // slice ctl
		    bx[8] = rx+140;  by[8] = ry+30;  bw[8] = 60;  bh[8] = 20;
		    bx[9] = rx+210;  by[9] = ry+30;  bw[9] = 60;  bh[9] = 20;
		    bx[10] = rx+140; by[10] = ry+60; bw[10] = 60; bh[10] = 20;
		    
		    switch(viewmode) {
		    case 0:
			g.drawString("Nav. (" + cx + "," + cy + "," + cz + ")",rx+130,ry+20);
			break;
		    case 1:
			g.drawString("Nav. (" + (cz+1) + " / " + vol.D + ")",rx+130,ry+20);
			break;
		    }
		}

		// cols ctl
		if (viewmode == 2) {
		    g.setColor(Color.WHITE);
		    g.drawString("Colunas: "+ncols,rx+170,ry+20);
		    bx[8] = rx+170; by[8] = ry+30; bw[8] = 61; bh[8] = 21;
		    g.drawImage(colctl,bx[8],by[8],null);
		}

		if (viewmode < 2)
		    g.drawImage(slicectl,bx[8],by[8],null);

		if (viewmode == 0) {
		    g.drawImage(slicectl,bx[9],by[9],null);
		    g.drawImage(slicectl,bx[10],by[10],null);
		}
		

	    } // status 1 (image loaded)

	    // version
	    g.setFont(f5);
	    g.setColor(new Color(0x00ffff));
	    tmp = "LNIDB/DIVA";
	    g.drawString(tmp,w-stringWidth(g,tmp)-5,10);
	    tmp = "Build "+build;
	    g.drawString(tmp,w-stringWidth(g,tmp)-5,20);
	    g.setColor(new Color(0x00dddd));
	    tmp = "Unicamp/Lab.Neuroimagem";
	    g.drawString(tmp,w-stringWidth(g,tmp)-5,h-5);
	} // paint

	private double r2(double x) {
	    int ix;
	    ix = (int) (100.0 * x);
	    return(ix/100.0);
	}

	// event handlers

	public void keyPressed(KeyEvent e) {
	    if (status < 1) return;
	    switch(e.getKeyCode()) {
	    case KeyEvent.VK_SPACE:
		panefocus = (panefocus + 1) % 3;
		repaint();
		break;
	    case KeyEvent.VK_RIGHT:
	    case KeyEvent.VK_KP_RIGHT:

		if (viewmode == 0) {
		    switch(panefocus) {
		    case 0: if (cz < vol.D-1) { cz++; repaint(); } break;
		    case 1: if (cx < vol.W-1) { cx++; repaint(); } break;
		    case 2: if (cy < vol.H-1) { cy++; repaint(); } break;
		    }
		} else if (viewmode == 1) {
		    if (cz < vol.D-1) { cz++; repaint(); }
		} else if (viewmode == 2) {
		    panx -= 50;
		    repaint();
		}
		break;
	    case KeyEvent.VK_LEFT:
	    case KeyEvent.VK_KP_LEFT:
		if (viewmode == 0) {
		    switch(panefocus) {
		    case 0: if (cz > 0) { cz--; repaint(); } break;
		    case 1: if (cx > 0) { cx--; repaint(); } break;
		    case 2: if (cy > 0) { cy--; repaint(); } break;
		    }
		} else if (viewmode == 1) {
		    if (cz > 0) { cz--; repaint(); }
		} else if (viewmode == 2) {
		    panx += 50;
		    repaint();
		}
		break;
	    case KeyEvent.VK_UP:
	    case KeyEvent.VK_KP_UP:
		if (viewmode < 2) {
		    windowChanged();
		    wmax -= 100; if (wmax < wmin + 1) wmax = vmin + 1; repaint();
		} else if (viewmode == 2) {
		    rowy--; if (rowy < 0) rowy = 0;
		    repaint();
		} else if (viewmode == 3) {
		    studysel--; if (studysel < 0) studysel = 0;
		    repaint();
		}
		break;
	    case KeyEvent.VK_DOWN:
	    case KeyEvent.VK_KP_DOWN:
		if (viewmode < 2) {
		    windowChanged();
		    wmax += 100; if (wmax > vmax) wmax = vmax; repaint();
		} else if (viewmode == 2) {
		    rowy++;
		    if (rowy * ncols >= vol.D) rowy--;
		    repaint();
		} else if (viewmode == 3) {
		    studysel++; if (studysel >= Diva.StudyCount) studysel = Diva.StudyCount-1;
		    repaint();
		}
		break;
	    case KeyEvent.VK_I:
		if (viewmode == 0) {
		    interpolate = !interpolate;
		    repaint();
		}
		break;
	    case KeyEvent.VK_C:
		if (viewmode < 3) {
		    showcursor = !showcursor;
		    repaint();
		}
		break;
	    }
	}

	private void windowChanged() {
	    int i;
	    for(i=0;i<3;i++) vb[i].invalidate();
	    vbc.clear();
	}

	public void keyReleased(KeyEvent e) { }

	public void keyTyped(KeyEvent e) { }

	public void focusGained(FocusEvent e) {
	    repaint();
	}

	public void focusLost(FocusEvent e) {
	    repaint();
	}

	public void mouseWheelMoved(MouseWheelEvent e) {
	    int y = e.getWheelRotation();
	    if (e.isControlDown()) y*=10;
	    
	    switch(viewmode) {
	    case 0:
		switch(panefocus) {
		case 0: cz += y; if (cz < 0) cz = 0; if (cz >= vol.D) cz=vol.D-1; break;
		case 1: cx += y; if (cx < 0) cx = 0; if (cx >= vol.W) cx=vol.W-1; break;
		case 2: cy += y; if (cy < 0) cy = 0; if (cy >= vol.H) cy=vol.H-1; break;
		}
		break;
	    case 1:
		cz += y;
		if (cz < 0) cz = 0;
		if (cz >= vol.D) cz = vol.D-1;
		break;
	    case 2:
		rowy += y;
		if (rowy < 0) rowy = 0;
		if (rowy * ncols >= vol.D) rowy = (vol.D-1) / ncols;
		break;
	    case 3:
		studysel += y;
		if (studysel < 0) studysel = 0;
		if (studysel >= Diva.StudyCount) studysel = Diva.StudyCount-1;
		break;
	    }
	    repaint();
	}

	public void mouseClicked(MouseEvent e) { 
	    requestFocusInWindow();
	}

	public void mouseEntered(MouseEvent e) { }
	public void mouseExited(MouseEvent e) { }

	public void mousePressed(MouseEvent e) {
	    int i,button=0;
	    int x,y,px,py,pu;

	    if (status < 1) return;

	    if ( (e.getModifiers() & MouseEvent.BUTTON1_MASK) != 0) button = 1;
	    if ( (e.getModifiers() & MouseEvent.BUTTON2_MASK) != 0) button = 2;	    
	    if ( (e.getModifiers() & MouseEvent.BUTTON3_MASK) != 0) button = 3;

	    x = e.getX(); y = e.getY();
	    if (x >= bx[0] && x < bx[0]+bw[0] && y >= by[0] && y <= by[0]+bh[0] && x < rpanex && viewmode < 2) {
		cx = (int) ((x - bx[0]) / (zoom * rfx));
		cy = (int) ((y - by[0]) / (zoom * rfy));
		panefocus = 0;
		repaint();
		return;
	    }
	    if (x >= bx[1] && x < bx[1]+bw[1] && y >= by[1] && y <= by[1]+bh[1] && x < rpanex && viewmode == 0) {
		cz = (int) ((x - bx[1]) / (zoom * rfz));
		cy = (int) ((y - by[1]) / (zoom * rfy));
		panefocus = 1;
		repaint();
		return;
	    }
	    if (x >= bx[2] && x < bx[2]+bw[2] && y >= by[2] && y <= by[2]+bh[2] && x < rpanex && viewmode == 0) {
		cx = (int) ((x - bx[2]) / (zoom * rfx));
		cz = (int) ((y - by[2]) / (zoom * rfz));
		cz = vol.D - cz - 1;
		panefocus = 2;
		repaint();
		return;
	    }

	    // window control: left click/drag sets, right click resets
	    if (x >= bx[3]-5 && x < bx[3]+bw[3]+5 && y >= by[3] && y <= by[3]+bh[3] && viewmode < 3) {

		if (button == 1) {

		    if ( Math.abs(x-sx1) < Math.abs(x-sx2) ) {
			wmin = ((vmax-vmin) * (x - bx[3])) / bw[3];
			wmin += vmin;
			if (wmin >= wmax) wmin = wmax-1;
			if (wmin < vmin) wmin = vmin;
		    } else {
			wmax = ((vmax-vmin) * (x - bx[3])) / bw[3];
			wmax += vmin;
			if (wmax <= wmin) wmax = wmin + 1;
			if (wmax > vmax) wmax = vmax;
		    }


		}
		if (button == 3) {
		    wmax = vmax;
		    wmin = vmin;
		}
		windowChanged();
		repaint();
		return;
	    }

	    // cursor control
	    if (viewmode < 3 && x >= bx[4] && x < bx[4]+bw[4] && y >= by[4] && y <= by[4]+bh[4]) {
		showcursor = !showcursor;
		repaint();
		return;
	    }

	    // dicom details
	    if (viewmode < 3 && x >= bx[15] && x < bx[15]+bw[15] && y >= by[15] && y <= by[15]+bh[15]) {
		openDicomDetails();
		return;
	    }

	    // interpolation control
	    if (viewmode == 0 && x >= bx[7] && x < bx[7]+bw[7] && y >= by[7] && y <= by[7]+bh[7]) {
		interpolate = !interpolate;
		repaint();
		return;
	    }

	    // userzoom control: left click/drag sets, right click resets
	    if (x >= bx[5]-5 && x < bx[5]+bw[5]+5 && y >= by[5] && y <= by[5]+bh[5] && viewmode < 3) {

		if (button == 1) {
		   
		    userzoom = (float) ((Math.sqrt(maxuserzoom )* (x - bx[5])) / ((double)bw[5]));
		    userzoom *= userzoom;
		    if (userzoom > maxuserzoom) userzoom = maxuserzoom;
		    if (userzoom < 0.01f) userzoom = 0.01f;
		}
		if (button == 3) {
		    userzoom = 1.0f;
		}
		repaint();
		return;
	    }

	    // pan, modes 0-1
	    if (x >= bx[6] && x < bx[6]+bw[6] && y >= by[6] && y <= by[6]+bh[6] && viewmode < 2) {
		px = (x-bx[6]) / 20;
		py = (y-by[6]) / 20;
		pu = (int) (30.0*zoom);
		if (pu < 50) pu = 50;
		if ((px==1 && py==1) || button==3) { panx=0; pany=0; }
		if (button == 1) {
		    if (px==0) panx+=pu; if (px==2) panx-=pu;
		    if (py==0) pany+=pu; if (py==2) pany-=pu;
		}
		repaint();
		return;
	    }

	    // pan, mode 2
	    if (x >= bx[6] && x < bx[6]+bw[6] && y >= by[6] && y <= by[6]+bh[6] && viewmode == 2) {
		px = (x-bx[6]) / 20;
		py = (y-by[6]) / 20;
		pu = (int) (30.0*zoom);
		if (pu < 50) pu = 50;
		if ((px==1 && py==1) || button==3) { panx=0; rowy=0; }
		if (button == 1) {
		    if (px==0) panx+=pu; if (px==2) panx-=pu;
		    if (py==0) rowy--; if (py==2) rowy++;
		    if (rowy < 0) rowy = 0;
		    if (rowy * ncols >= vol.D) rowy--;
		}
		repaint();
		return;
	    }

	    // scroll, mode 2
	    if (x >= bx[1] && x < bx[1]+bw[1] && y >= by[1] && y<= by[1]+bh[1] && viewmode == 2) {
		py = (vol.D * (y-by[1])) / bh[1];
		if (py < 0) py = 0;
		if (py >= vol.D) py = vol.D - 1;
		rowy = py / ncols;
		repaint();
		return;
	    }

	    // slice ctl
	    if (viewmode < 2 && x >= bx[8] && x < bx[8]+bw[8] && y >= by[8] && y <= by[8]+bh[8]) {
		px = (x-bx[8]) / 15;
		if (px==0) cz = 0;
		if (px==1) { cz--; if (cz<0) cz=0; }
		if (px==2) { cz++; if (cz>=vol.D) cz=vol.D-1; }
		if (px==3) cz = vol.D - 1;
		repaint();
		return;
	    }
	    // cols ctl
	    if (viewmode == 2 && x >= bx[8] && x < bx[8]+bw[8] && y >= by[8] && y <= by[8]+bh[8]) {
		py = rowy * ncols;
		px = (x-bx[8]) / 20;
		if (px==0) ncols--;
		if (px==1) ncols = 4;
		if (px==2) ncols++;
		if (ncols < 2) ncols = 2;
		if (ncols > 16) ncols = 16;
		rowy = py / ncols;
		if (rowy < 0) rowy = 0;
		while (rowy * ncols >= vol.D) rowy--;
		repaint();
		return;
	    }
	    if (viewmode==0 && x >= bx[9] && x < bx[9]+bw[9] && y >= by[9] && y <= by[9]+bh[9]) {
		px = (x-bx[9]) / 15;
		if (px==0) cx = 0;
		if (px==1) { cx--; if (cx<0) cx=0; }
		if (px==2) { cx++; if (cx>=vol.W) cx=vol.W-1; }
		if (px==3) cx = vol.W - 1;
		repaint();
		return;
	    }
	    if (viewmode==0 && x >= bx[10] && x < bx[10]+bw[10] && y >= by[10] && y <= by[10]+bh[10]) {
		px = (x-bx[10]) / 15;
		if (px==0) cy = 0;
		if (px==1) { cy--; if (cy<0) cy=0; }
		if (px==2) { cy++; if (cy>=vol.H) cy=vol.H-1; }
		if (px==3) cy = vol.H - 1;
		repaint();
		return;
	    }
	    for(i=0;i<4;i++) {
		if (x >= bx[11+i] && x < bx[11+i]+bw[11+i] && y >= by[11+i] && y <= by[11+i]+bh[11+i]) {
		    viewmode = i;
		    repaint();
		    return;
		}
	    }
	    // mode 3, sel up/down
	    if (viewmode==3 && x >= bx[3] && x < bx[3]+bw[3] && y >= by[3] && y <= by[3]+bh[3]) {
		if (y-by[3] <= 20) studysel--; else studysel++;
		if (studysel < 0) studysel = 0;
		if (studysel >= Diva.StudyCount) studysel = Diva.StudyCount - 1;
		repaint();
		return;
	    }
	    // mode 3, load
	    if (viewmode==3 && x >= bx[4] && x < bx[4]+bw[4] && y >= by[4] && y <= by[4]+bh[4]) {
		reloadIndex = studysel;
		return;
	    }
	    // mode 3, select
	    if (viewmode==3 && x < rpanex && y > 50) {
		i = studysel = ((y-50) / 18) + studyscroll;
		if (i < 0 || i >= Diva.StudyCount) return;
		studysel = i;
		repaint();
		return;
	    }

	}

	public void mouseReleased(MouseEvent e) { }
	public void mouseDragged(MouseEvent e) { if ((e.getModifiers() & MouseEvent.BUTTON1_MASK) != 0) mousePressed(e); }
	public void mouseMoved(MouseEvent e) { }

    } // DivaPanel

}

class ViewBuffer {
    private Image img;
    private int w,h,p;

    ViewBuffer() {
	img = null;
	w=h=p=-1;
    }

    public int getWidth() { return w; }
    public int getHeight() { return h; }
    public int getPos() { return p; }

    public void set(Image i,int pw,int ph,int pp) {
	img = i;
	w = pw;
	h = ph;
	p = pp;
    }

    public boolean valid(int width, int height, int pos) {
	return(w==width && h==height && p==pos && img!=null);
    }

    public void invalidate() {
	w = -1;
    }

    public Image getImage() {
	return img;
    }

}

class ViewBufferCache {

    private int N;
    private ViewBuffer [] cache;
    private int        [] free;

    public ViewBufferCache(int cachelen) {
	int i;
	N = cachelen;
	cache = new ViewBuffer[N];
	free = new int[N];
	for(i=0;i<N;i++) {
	    cache[i] = null;
	    free[i] = 1;
	}
    }

    public boolean has(int p) {
	int i;
	for(i=0;i<N;i++)
	    if (free[i] == 0 && cache[i] != null)
		if (cache[i].getPos() == p)
		    return true;
	return false;
    }

    public Image query(int w,int h,int p) {
	int i;
	for(i=0;i<N;i++)
	    if (free[i] == 0 && cache[i] != null)
		if (cache[i].valid(w,h,p)) {
		    if (Diva.BufferDebug > 1)
			System.out.println("hit " + w + "," + h + "," + p);
		    return(cache[i].getImage());
		}
	if (Diva.BufferDebug > 1)
	    System.out.println("miss " + w + "," + h + "," + p);
	return null;
    }

    public void clear() {
	int i;
	for(i=0;i<N;i++) {
	    cache[i] = null;
	    free[i] = 1;
	}
    }

    public void print() {
	int i;
	System.out.println("Cache N="+N);
	for(i=0;i<N;i++) {
	    System.out.print("#"+i+": ");
	    if (free[i] != 0) 
		System.out.print("free/");
	    else
		System.out.print("used/");
	    if (cache[i] == null) { System.out.println("null"); continue; }
	    System.out.println("pos="+cache[i].getPos()+", w="+cache[i].getWidth()+", h="+cache[i].getHeight());
	}
    }

    public void store(Image img, int w, int h, int p) {
	int i,md,mdi;
	if (Diva.BufferDebug > 1)
	    System.out.println("store " + w + "," + h + "," + p);
	// if same pos in cache, replace
	for(i=0;i<N;i++)
	    if (free[i]==0 && cache[i]!=null)
		if (cache[i].getPos() == p) {
		    cache[i].set(img,w,h,p);
		    if (Diva.BufferDebug > 1)
			System.out.println("replaced in slot "+i);
		    return;
		}
	// not in cache, try to store in free slot
	for(i=0;i<N;i++) {
	    if (free[i]!=0) {
		cache[i] = new ViewBuffer();
		cache[i].set(img,w,h,p);
		free[i] = 0;
		if (Diva.BufferDebug > 1)
		    System.out.println("placed in free slot "+i);
		return;
	    }
	}
	// no free slot, discard furthest item 
	md = Math.abs(cache[0].getPos() - p);
	mdi = 0;
	for(i=0;i<N;i++)
	    if (Math.abs(cache[i].getPos() - p) > md) {
		md = Math.abs(cache[i].getPos() - p);
		mdi = i;
	    }
	if (Diva.BufferDebug > 1)
	    System.out.println("slot "+mdi+" (dist="+md+") discarded.");
	cache[mdi] = new ViewBuffer();
	cache[mdi].set(img,w,h,p);
	free[mdi] = 0;
    }
}
