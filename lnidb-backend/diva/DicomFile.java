import java.util.*;
import java.util.zip.*;
import java.net.*;
import java.io.*;

public class DicomFile {

    public static boolean LastEndianFlip = false;

    public String pathname;
    public LinkedList<DicomField> fields;
    public LinkedList<DicomField> mfields;
    DicomVR[] vr;
    long fsize, pos;
    boolean pixels;
    boolean endianflip;
    ProgressStatus ps;

    public DicomFile(String path, ProgressStatus pstatus) throws BadDicomException, FileNotFoundException {
	int pdone=0;
	pathname = path;
	initvr();
	fields = new LinkedList<DicomField>();
	mfields = new LinkedList<DicomField>();
	pixels = true;
	endianflip = DicomFile.LastEndianFlip;
	ps = pstatus;
	if (ps != null) pdone = ps.getDone();
	try {
	    if (path.startsWith("http://")) readNet(); else readFile();
	} catch(OutOfMemoryError oom) {
	    endianflip = !endianflip;
	    try {
		if (path.startsWith("http://")) readNet(); else readFile();
		DicomFile.LastEndianFlip = endianflip;
	    } catch(EmptyDicomException ede3) {
		throw(new BadDicomException("No DICOM fields."));
	    }
	} catch(EmptyDicomException ede) {
	    endianflip = !endianflip;
	    if (ps != null) ps.setDone(pdone);
	    try {
		if (path.startsWith("http://")) readNet(); else readFile();
		DicomFile.LastEndianFlip = endianflip;
	    } catch(EmptyDicomException ede2) {
		throw(new BadDicomException("No DICOM fields."));
	    }
	}
    }

    public DicomFile(String path, ProgressStatus pstatus, boolean pix) throws BadDicomException, FileNotFoundException {
	int pdone=0;
	pathname = path;
	initvr();
	fields = new LinkedList<DicomField>();
	mfields = new LinkedList<DicomField>();
	pixels = pix;
	endianflip = DicomFile.LastEndianFlip;
	ps = pstatus;
	if (ps != null) pdone = ps.getDone();
	try {
	    if (path.startsWith("http://")) readNet(); else readFile();
	} catch(EmptyDicomException ede) {
	    endianflip = !endianflip;
	    if (ps!=null) ps.setDone(pdone);
	    try {
		if (path.startsWith("http://")) readNet(); else readFile();
		DicomFile.LastEndianFlip = endianflip;
	    } catch(EmptyDicomException ede2) {
		throw(new BadDicomException("No DICOM fields."));
	    }
	}
    }

    public Iterator<DicomField> iterator() {
	return(fields.iterator());
    }

    public DicomField getField(int group, int key) {
	Iterator<DicomField> i = fields.iterator();
	DicomField f;
	while(i.hasNext()) {
	    f = i.next();
	    if (f.match(group,key)) return f;
	}
	return null;
    }

    // returns the index occurrence of the same field
    public DicomField getMField(int group, int key, int index) {
	Iterator<DicomField> i = mfields.iterator();
	int n=-1;
	DicomField f;
	while(i.hasNext()) {
	    f = i.next();
	    if (f.match(group,key)) n++;
	    if (n==index) return f;
	}
	return null;
    }

    // returns the occurrence of the same field after the index-th occurrence of the stack id field
    public DicomField getSMField(int group, int key, int index) {
	Iterator<DicomField> i = mfields.iterator();
	int n=-1;
	DicomField f;
	while(i.hasNext()) {
	    f = i.next();
	    if (n==index && f.match(group,key)) return f;
	    if (f.match(0x0020,0x9057)) n++;
	}
	return null;
    }

    // counts the number of occurrences of a given field
    public int getFieldCount(int group, int key) {
	int n = 0;
	Iterator<DicomField> i = mfields.iterator();
	DicomField f;
	while(i.hasNext()) {
	    f = i.next();
	    if (f.match(group,key)) n++;
	}
	return n;
    }

    public void cleanUp(int sizelimit) {
	Iterator<DicomField> i = fields.iterator();
	DicomField f;
	while(i.hasNext()) {
	    f = i.next();
	    if (f.length() > sizelimit)
		f.discard();
	}
	i = mfields.iterator();
	while(i.hasNext()) {
	    f = i.next();
	    if (f.length() > sizelimit)
		f.discard();
	}
    }

    private void initvr() {
	vr = new DicomVR[26];
	vr[0] = new DicomVR("AE",16,0,false);
	vr[1] = new DicomVR("AS",4,1,false);
	vr[2] = new DicomVR("AT",4,1,false);
	vr[3] = new DicomVR("CS",16,0,false);

	vr[4] = new DicomVR("DA",8,1,false);
	vr[5] = new DicomVR("DS",16,0,false);
	vr[6] = new DicomVR("DT",26,0,false);
	vr[7] = new DicomVR("FL",4,1,false);

	vr[8] = new DicomVR("FD",8,1,false);
	vr[9] = new DicomVR("IS",12,0,false);
	vr[10] = new DicomVR("LO",64,0,false);
	vr[11] = new DicomVR("LT",10240,0,false);

	vr[12] = new DicomVR("OB",0,0,true);
	vr[13] = new DicomVR("OW",0,0,true);
	vr[14] = new DicomVR("PN",64,0,false);
	vr[15] = new DicomVR("SH",16,0,false);

	vr[16] = new DicomVR("SL",4,1,false);
	vr[17] = new DicomVR("SQ",0,0,true);
	vr[18] = new DicomVR("SS",2,1,false);
	vr[19] = new DicomVR("ST",1024,0,false);

	vr[20] = new DicomVR("TM",16,0,false);
	vr[21] = new DicomVR("UI",64,0,false);
	vr[22] = new DicomVR("UL",4,1,false);
	vr[23] = new DicomVR("UN",0,0,true);

	vr[24] = new DicomVR("US",2,1,false);
	vr[25] = new DicomVR("UT",0,0,true);
    }

    private void readNet() throws BadDicomException, EmptyDicomException, FileNotFoundException {
	DataInputStream dis=null;
	int retriesLeft = 10;
	boolean ok = false;
	//System.out.println("reading " + pathname);

	while(!ok) {
	    try {
		ok=false;
		URL u = new URL(pathname);
		URLConnection conn;
		conn = u.openConnection();
		conn.setConnectTimeout(60000);
		conn.setReadTimeout(60000);
		dis = new DataInputStream(new BufferedInputStream(conn.getInputStream()));
		fsize = 300000000;
		pos = 0;
		
		byte[] gzhdr = new byte[2];
		boolean gzip = false;
		dis.readFully(gzhdr,0,2);

		// gzip header
		if (gzhdr[0] == (byte) 0x1f && gzhdr[1] == (byte) 0x8b) {
		    gzip = true;
		    dis.close();
		    conn = u.openConnection();
		    conn.setConnectTimeout(60000);
		    conn.setReadTimeout(60000);		    
		    dis = new DataInputStream(new BufferedInputStream(new GZIPInputStream(conn.getInputStream())));
		    dis.skipBytes(2);
		}

		byte[] b = new byte[4];
		dis.skipBytes(126);
		dis.readFully(b,0,4);
		
		String hdr = new String(b,"US-ASCII");
		if (!hdr.equals("DICM")) {
		    dis.close();
		    conn = u.openConnection();
		    conn.setConnectTimeout(60000);
		    conn.setReadTimeout(60000);		    
		    if (gzip) {
			dis = new DataInputStream(new BufferedInputStream(new GZIPInputStream(conn.getInputStream())));
		    } else {
			dis = new DataInputStream(new BufferedInputStream(conn.getInputStream()));
		    }
		} else {
		    pos = 128+4;
		    if (ps!=null) ps.incrementDone(128+4);
		}
		
		ok=true;
	    } catch(SocketTimeoutException ste) {
		retriesLeft--;
		if (retriesLeft <= 0)
		    throw(new BadDicomException("Network Timeout Error"));
	    } catch(IOException e) {
		throw(new BadDicomException("Network I/O Error"));
	    }
	}
	if (dis!=null) {
	    read(dis);
	    fsize = pos;
	}
    }

    private void readFile() throws BadDicomException, EmptyDicomException, FileNotFoundException {
	DataInputStream dis;
	try {
	    File f = new File(pathname);
	    fsize = f.length();
	    pos = 0;
	    
	    dis = new DataInputStream(new FileInputStream(pathname));

	    byte[] gzhdr = new byte[2];
	    boolean gzip = false;
	    dis.readFully(gzhdr,0,2);

	    // gzip header
	    if (gzhdr[0] == (byte) 0x1f && gzhdr[1] == (byte) 0x8b) {
		gzip = true;
		fsize = 300000000;
		dis.close();
		dis = new DataInputStream(new GZIPInputStream(new FileInputStream(pathname)));
		dis.skipBytes(2);
	    }

	    byte[] b = new byte[4];
	    dis.skipBytes(126);
	    dis.readFully(b,0,4);
	    
	    String hdr = new String(b,"US-ASCII");
	    if (!hdr.equals("DICM")) {
		dis.close();
		if (gzip) {
		    dis = new DataInputStream(new GZIPInputStream(new FileInputStream(pathname)));
		} else {
		    dis = new DataInputStream(new FileInputStream(pathname));
		}
	    } else {
		pos = 128+4;
		if (ps!=null) ps.incrementDone(128+4);
	    }	    

	} catch(IOException e) {
	    throw(new BadDicomException("IO Error"));
	}
	read(dis);
    }

    public long fileSize() {
	return fsize;
    }

    private void read(DataInputStream dis) throws BadDicomException, EmptyDicomException {
	DicomField df, r;
	long spos;
	do {
	    spos = pos;
	    df = readDicomField(dis);
	    if (ps!=null) ps.incrementDone((int)(pos-spos));
	    if (df!=null) {
		r = getField(df.group, df.key);
		mfields.add(df);
		//System.out.println("field "+df.group+","+df.key);
		if (r!=null)
		    r.value = df.value;
		else
		    fields.add(df);
	    }
	} while(df!=null);

	try {
	    dis.close();
	} catch(IOException e) {
	    throw(new BadDicomException("IO Error"));
	}

	if (fields.isEmpty())
	    throw(new EmptyDicomException("No Fields Read in "+pathname));

    }

    private DicomField readDicomField(DataInputStream dis) throws BadDicomException {
	DicomField df = new DicomField(endianflip);
	byte[] b = new byte[128];
	int i,len,tmp;

	try {
	    dis.readFully(b,0,4);
	    pos += 4;
	    df.group = df.parse16(b,0);
	    df.key   = df.parse16(b,2);
	    dis.readFully(b,0,4);
	    pos += 4;

	    if (!pixels && df.group == 0x7fe0 && df.key == 0x0010) return null;

	    //System.out.printf("read [%04x:%04x]\n",df.group,df.key);

	    String s = new String(b,0,2,"US-ASCII");
	    for(i=0;i<vr.length;i++) {

		if (s.equals(vr[i].id)) {
		    
		    //System.out.println("VR match, i=" + i);

		    if (vr[i].longlen) {
			dis.readFully(b,0,4);
			pos += 4;
			len = df.parse32(b,0);
		    } else {
			len = df.parse16(b,2);
		    }

		    //System.out.println("len=" + len);

		    if (len > (fsize - pos)) return null;

		    if (len <= 0) {
			df.value = new byte[1];
			df.value[0] = 0;
			// ignore SQ
		    } else {
			df.value = new byte[len];
			dis.readFully(df.value, 0, len);
			pos += len;
			// in dicom2scn, len>126 in VR is an error

			if (i==16 || i==22) {
			    tmp = df.parse32(df.value,0);
			    df.value = String.valueOf(tmp).getBytes("US-ASCII");
			}
			if (i==18 || i==24) {
			    tmp = df.parse16(df.value,0);
			    df.value = String.valueOf(tmp).getBytes("US-ASCII");
			}
		    }
		    //System.out.println("got here, value=" + df.toString());
		    return df;
		}
	    } // VR check

	    len = df.parse32(b,0);
	    if (len > (fsize - pos)) return null;
	    if (len <= 0) {
		df.value = new byte[1];
		df.value[0] = 0;
		return df;
	    } else {
		df.value = new byte[len];
		dis.readFully(df.value, 0, len);
		pos += len;
		
		if (df.match(0x18,0x50) || df.match(0x08,0x60) || df.match(0x10,0x40) ||
		    df.match(0x18,0x24) || df.match(0x20,0x1041) || looksAlphanumeric(df.value))
		    return df;
		if (len==2) {
		    tmp = df.parse16(df.value,0);
		    df.value = String.valueOf(tmp).getBytes("US-ASCII");
		}
		if (len==4) {
		    tmp = df.parse32(df.value,0);
		    df.value = String.valueOf(tmp).getBytes("US-ASCII");
		}
		return df;
	    }
	} catch(IOException e) {
	    //e.printStackTrace();
	    return null;
	}
    }

    private boolean looksAlphanumeric(byte[] v) {
	int i;
	for(i=0;i<v.length;i++)
	    if (v[i] < 32 || v[i] > 126) return false;
	return true;
    }

    public String debugString() {
	StringBuffer x;
	Iterator<DicomField> i = fields.iterator();
	DicomField f;
	int fc=0;

	x = new StringBuffer();
	x.append("DicomFile debugString pathname=["+pathname+"] ");
	x.append("pos="+pos+" endianflip="+endianflip+"\n");

	while(i.hasNext()) {
	    f = i.next();
	    x.append(String.format("[%04x:%04x len %d]",f.group,f.key,f.value.length));
	    fc++;
	    if (fc%6 == 0) x.append("\n"); else x.append(" ");  
	}
	x.append("End of debugString.\n");

	return(x.toString());
    }

}
