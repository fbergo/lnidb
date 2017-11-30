import java.util.*;

public class DivaLoader implements Runnable {

    private String fids, method;
    public boolean done, failed;
    public Volume vol;
    public ScanInfo info;
    public String failreason;
    public StringBuffer livestatus;
    private ProgressStatus ps;

    DivaLoader(String pfids, String pmethod,int w,int h,int d,ProgressStatus pstatus) {
	fids   = pfids;
	method = pmethod;
	done = false;
	failed = false;
	vol = new Volume(w,h,d);
	info = new ScanInfo();
	failreason = "Sem Erro.";
	livestatus = new StringBuffer("DivaLoader iniciado.\n");
	ps = pstatus;
    }

    public void run() {
	StringTokenizer t;
	DicomFile df=null;
	DicomField f;
	int i,j,k,n,a;
	int w,h,d;
	float [] zpos;
	Vector<Float> imul, iadd;
	int n1;
	String nt;
	boolean duplicated = false;
	long t0,t1,fsize;

	livestatus.append("DivaLoader.run() iniciado.");

	t = new StringTokenizer(fids," ,\r\n\t");
	n = t.countTokens();

	livestatus.append(" Número de FIDs: "+n+"\n");

	if (n==0) { 
	    livestatus.append("Abortado (100).\n");
	    failreason = "Lista vazia de arquivos."; 
	    done=failed=true; return; 
	}
	zpos = new float[n];

	iadd = new Vector<Float>();
	imul = new Vector<Float>();
	
	t0 = System.currentTimeMillis();
	try {
	    //System.out.println("n=" + n);
	    for(i=0;i<n;i++) {
		//System.out.println("i=" + i);
		nt = t.nextToken();
		livestatus.append("(" + (i+1) + "/" + n + ") Iniciando carga de "+method+nt);
		df = new DicomFile(method + nt, ps);
		livestatus.append(" [OK]\n");
		info.loadSizeBytes += df.fileSize();

		if (df.getField(0x0010,0x0010) != null && info.name == null)
		    info.name = df.getField(0x0010,0x0010).toString();
		if (df.getField(0x0010,0x0020) != null && info.hc == null)
		    info.hc = df.getField(0x0010,0x0020).toString();
		if (df.getField(0x0010,0x0030) != null && info.birth == null)
		    info.birth = df.getField(0x0010,0x0030).toString();
		if (df.getField(0x0010,0x0040) != null && info.gender == null)
		    info.gender = df.getField(0x0010,0x0040).toString();
		if (df.getField(0x0008,0x0020) != null && info.scandate == null)
		    info.scandate = df.getField(0x0008,0x0020).toString() + " " + df.getField(0x0008,0x0030);
		if (df.getField(0x0008,0x1030) != null && info.exam == null)
		    info.exam = df.getField(0x0008,0x1030).toString();
		if (df.getField(0x0008,0x103e) != null && info.series == null)
		    info.series = df.getField(0x0008,0x103e).toString();
		if (df.getField(0x0018,0x1030) != null && info.series == null)
		    info.series = df.getField(0x0018,0x1030).toString();
		if (df.getField(0x0008,0x0070) != null && info.scanner1 == null) {
		    info.scanner1 = df.getField(0x0008,0x0070).toString();
		    info.scanner2 = df.getField(0x0008,0x1090).toString() + " " + df.getField(0x0008,0x0080).toString();
		}
		if (df.getField(0x0018,0x0050) != null)
		    info.thickness = df.getField(0x0018,0x0050).toFloat();
		if (df.getField(0x0020,0x0037) != null) {
		    StringTokenizer tt = new StringTokenizer(df.getField(0x0020,0x0037).toString(),"\\");
		    info.xcos[0] = Float.parseFloat(tt.nextToken());
		    info.xcos[1] = Float.parseFloat(tt.nextToken());
		    info.xcos[2] = Float.parseFloat(tt.nextToken());
		    info.ycos[0] = Float.parseFloat(tt.nextToken());
		    info.ycos[1] = Float.parseFloat(tt.nextToken());
		    info.ycos[2] = Float.parseFloat(tt.nextToken());
		}
	    
		w = df.getField(0x0028,0x0011).toInteger();
		h = df.getField(0x0028,0x0010).toInteger();
		d = 1;
		zpos[i] = 0.0f;

		//System.out.println("w=" + w);
		//System.out.println("h=" + h);
		//System.out.println("d=" + d);

		if ((f=df.getField(0x0028,0x0008)) != null)
		    d = f.toInteger();
		else if ((f=df.getField(0x0020,0x1041)) != null)
		    zpos[i] = f.toFloat();

		if (d==1) {
		    if (df.getField(0x0028,0x1052)!=null && df.getField(0x0028,0x1053)!=null) {
			iadd.add(df.getField(0x0028,0x1052).toFloat());
			imul.add(df.getField(0x0028,0x1053).toFloat());
		    } else {
			iadd.add(0.0f);
			imul.add(1.0f);
		    }
		}

		// in-stack position
		n1 = df.getFieldCount(0x0020,0x9057);
		if (n1>0 && 
		    (df.getField(0x0028,0x1052) == null ||
		     df.getField(0x0028,0x1053) == null))
		    n1 = 0;

		for(j=0;j<n1;j++) {
		    iadd.add(df.getSMField(0x0028,0x1052,j).toFloat());
		    imul.add(df.getSMField(0x0028,0x1053,j).toFloat());
		}

		// 3D case
		if (d>1 && (w!=vol.W || h!=vol.H || d!=vol.D)) {

		    // 3D case with duplicated files
		    if (df.getField(0x0028,0x0008) != null && vol.W==w && vol.H==h && vol.D == d*n) {
			duplicated = true;
			vol = new Volume(w,h,d);
		    } else {
			livestatus.append("Abortado (101)\n");
			livestatus.append(df.debugString());
			failreason = "Dimensões inconsistentes (3D).";
			done=failed=true;
			return;
		    }
		}
		
		if (d==1 && (w!=vol.W || h!=vol.H)) {
		    livestatus.append("Abortado (102)\n");
		    livestatus.append(df.debugString());
		    failreason = "Dimensões inconsistentes (2D).";
		    done=failed=true;
		    return;
		}

		info.computeOrientation();

		// copy pixel data
		f = df.getField(0x7fe0,0x0010);
		if (f==null) { 
		    livestatus.append("Abortado (103)\n");
		    livestatus.append(df.debugString());
		    failreason = "PixelData (7fe0:0010) ausente."; 
		    done=failed=true; 
		    return;
		}
		a = vol.address(0,0,i);
		for(j=0;j<w*h*d;j++)
		    vol.data[a+j] = (short) f.parse16(f.value,j*2);
		
		info.captureReference(df);

		if (duplicated) break;
	    }

	    // apply intensity scaling

	    if (iadd.size() >= vol.D && imul.size() >= vol.D) {

		for(k=0;k<vol.D;k++)
		    if (iadd.get(k) != 0.0f || imul.get(k) != 1.0f) {
			//System.out.println("applying scale k="+k+" add="+iadd.get(k)+" mul="+imul.get(k));
			for(j=0;j<vol.H;j++)
			    for(i=0;i<vol.W;i++) {
				vol.set(i,j,k, (short) ( iadd.get(k) + imul.get(k) * vol.get(i,j,k) ));
			    }
		    }

	    } else if (iadd.size() == 1 && imul.size() == 1) {

		if (iadd.get(0) != 0.0f || imul.get(0) != 1.0f) {
		    for(k=0;k<vol.D;k++)
			//System.out.println("applying single scale k="+k+" add="+iadd.get(0)+" mul="+imul.get(0));
			for(j=0;j<vol.H;j++)
			    for(i=0;i<vol.W;i++) {
				vol.set(i,j,k, (short) ( iadd.get(0) + imul.get(0) * vol.get(i,j,k) ));
			    }
		    }
	    }

	    // sort slices by zpos (selection sort)
	    if (n>1) {
		for(i=0;i<n;i++) {
		    k = i;
		    for(j=i+1;j<n;j++) 
			if (zpos[j] < zpos[k])
			    k = j;
		    if (k!=i) 
			sliceSwap(zpos,k,i);
		}
	    }

	    // find dx,dy,dz
	    f = df.getField(0x0028,0x0030);
	    if (f!=null) {
		StringTokenizer t2 = new StringTokenizer(f.toString(),"\\");
		vol.dx = Float.parseFloat(t2.nextToken());
		vol.dy = Float.parseFloat(t2.nextToken());
	    }

	    if ((f = df.getField(0x0018,0x0088)) != null) {
		vol.dz = f.toFloat();
	    } else if (n>1) {
		vol.dz = (zpos[n-1] - zpos[0]) / n;
	    }
	    //System.out.println("dx=" + vol.dx + ", dy=" + vol.dy + ", dz=" + vol.dz);
	    info.datecomp();

	    done   = true;
	    failed = false;
	    t1 = System.currentTimeMillis();
	    info.loadTimeMillis = t1 - t0;
	    info.computeLoadRate();

	    livestatus.append("Fim do loop de carga.\n");

	} catch(Exception e) {
	    e.printStackTrace();
	    livestatus.append("Exceção capturada: ["+e.toString()+"]\n");
	    failreason = new String("Exceção: " + e.toString());
	    failed = true;
	    done = true;
	}
    }

    private void sliceSwap(float [] zpos, int a, int b) {
	int i,n,p,q;
	float t;
	short s;
	t = zpos[a];
	zpos[a] = zpos[b];
	zpos[b] = t;
	
	p = vol.address(0,0,a);
	q = vol.address(0,0,b);
	n = vol.WH;

	for(i=0;i<n;i++) {
	    s = vol.data[p+i];
	    vol.data[p+i] = vol.data[q+i];
	    vol.data[q+i] = s;
	}
    }

}
