import java.util.*;

public class ScanInfo {

    public String name;
    public String birth;
    public String scandate;
    public String gender;
    public String hc;
    public String exam;
    public String series;
    public String scanner1;
    public String scanner2;
    public float  thickness;
    // coeffs of 0020:0037, explained in page 275/sec C.7.6.2.1.1 of part 3 of
    // the DICOM standard
    public float[] xcos, ycos; 
    public String ox,oy,oz;
    public Vector<ReferenceField> ref;

    public int    age;

    public long loadSizeBytes;
    public long loadTimeMillis;
    public double loadRateMBs;

    public ScanInfo() {
	name = birth = scandate = gender = hc = exam = series = scanner1 = scanner2 = null;
	thickness = 0.0f;
	age=0;
	xcos = new float[3];
	ycos = new float[3];
	xcos[0] = xcos[1] = xcos[2] = 0.0f;
	ycos[0] = ycos[1] = ycos[2] = 0.0f;
	ox="??";
	oy="??";
	oz="??";
	ref = new Vector<ReferenceField>();
	initReference();
	resetLoadStats();
    }

    public void resetLoadStats() {
	loadSizeBytes = 0;
	loadTimeMillis = 0;
	loadRateMBs = 0.0;
    }

    public void computeLoadRate() {
	if (loadTimeMillis <= 0 || loadSizeBytes == 0)
	    loadRateMBs = 0.0;
	else
	    loadRateMBs = (((double)loadSizeBytes) / 1000000.0) / (((double)loadTimeMillis) / 1000.0);
    }

    private void initReference() {

	// pg 349, DICOM part 3
	ref.add(new ReferenceField(0x0010,0x0010,"Patient Name"));
	ref.add(new ReferenceField(0x0010,0x0020,"Patient ID"));
	ref.add(new ReferenceField(0x0010,0x0021,"Issuer of Patient ID"));
	ref.add(new ReferenceField(0x0010,0x0030,"Patient's Birth Date"));
	ref.add(new ReferenceField(0x0010,0x0040,"Patient's Gender"));

	ref.add(new ReferenceField(0x0028,0x0100,"Bits Allocated"));
	ref.add(new ReferenceField(0x0028,0x0101,"Bits Stored"));
	ref.add(new ReferenceField(0x0028,0x0102,"High Bit"));
	ref.add(new ReferenceField(0x0028,0x0103,"Pixel Representation"));
	ref.add(new ReferenceField(0x0028,0x0030,"Pixel Spacing"));
	ref.add(new ReferenceField(0x0028,0x1052,"Rescale Intercept"));
	ref.add(new ReferenceField(0x0028,0x1053,"Rescale Slope"));
	ref.add(new ReferenceField(0x0020,0x0037,"Image Orientation (Patient)"));
	ref.add(new ReferenceField(0x0020,0x0032,"Image Position (Patient)"));

	ref.add(new ReferenceField(0x0008,0x0020,"Study Date"));
	ref.add(new ReferenceField(0x0008,0x0030,"Study Time"));
	ref.add(new ReferenceField(0x0032,0x1050,"Study Completion Date"));
	ref.add(new ReferenceField(0x0032,0x1051,"Study Completion Time"));
	ref.add(new ReferenceField(0x0020,0x1000,"Series in Study"));
	ref.add(new ReferenceField(0x0020,0x1004,"Acquisitions in Study"));

	ref.add(new ReferenceField(0x0008,0x0021,"Series Date"));
	ref.add(new ReferenceField(0x0008,0x0031,"Series Time"));
	ref.add(new ReferenceField(0x0008,0x0022,"Acquisition Date"));
	ref.add(new ReferenceField(0x0008,0x0032,"Acquisition Time"));

	ref.add(new ReferenceField(0x0008,0x0060,"Modality"));
	ref.add(new ReferenceField(0x0008,0x1030,"Study Description"));
	ref.add(new ReferenceField(0x0008,0x103e,"Series Description"));
	ref.add(new ReferenceField(0x0018,0x1030,"Protocol Name"));

	ref.add(new ReferenceField(0x0018,0x0020,"Scanning Sequence"));
	ref.add(new ReferenceField(0x0018,0x0021,"Sequence Variant"));
	ref.add(new ReferenceField(0x0018,0x0022,"Scan Options"));
	ref.add(new ReferenceField(0x0018,0x0050,"Slice Thickness"));
	ref.add(new ReferenceField(0x0018,0x0080,"TR (Repetition Time)"));
	ref.add(new ReferenceField(0x0018,0x0081,"TE (Echo Time)"));
	ref.add(new ReferenceField(0x0018,0x0082,"Inversion Time"));
	ref.add(new ReferenceField(0x0018,0x0083,"Number of Averages"));
	ref.add(new ReferenceField(0x0018,0x0084,"Imaging Frequency (Hz)"));
	ref.add(new ReferenceField(0x0018,0x0085,"Imaged Nucleus"));
	ref.add(new ReferenceField(0x0018,0x0087,"Magnetic Field Strength"));
	ref.add(new ReferenceField(0x0018,0x0088,"Spacing Between Slices"));
	ref.add(new ReferenceField(0x0018,0x0089,"Number of Phase Encoding Steps"));
	ref.add(new ReferenceField(0x0018,0x0093,"Percent Sampling"));
	ref.add(new ReferenceField(0x0018,0x0094,"Percent Phase FOV"));
	ref.add(new ReferenceField(0x0018,0x0095,"Pixel Bandwidth"));
	ref.add(new ReferenceField(0x0018,0x1250,"Receive Coil Name"));
	ref.add(new ReferenceField(0x0018,0x1251,"Transmit Coil Name"));
	ref.add(new ReferenceField(0x0018,0x1310,"Acquisition Matrix"));
	ref.add(new ReferenceField(0x0018,0x1314,"Flip Angle"));
	ref.add(new ReferenceField(0x0018,0x1316,"SAR (W/Kg)"));
	ref.add(new ReferenceField(0x0018,0x1315,"Variable Flip Angle"));
	ref.add(new ReferenceField(0x0018,0x1318,"dB/dt"));
	ref.add(new ReferenceField(0x0020,0x0110,"Temporal Resolution"));
	// C.7-8 Equipment Module
	ref.add(new ReferenceField(0x0008,0x0070,"Manufacturer"));
	ref.add(new ReferenceField(0x0008,0x0080,"Institution Name"));
	ref.add(new ReferenceField(0x0008,0x0081,"Institution Address"));
	ref.add(new ReferenceField(0x0008,0x1010,"Station Name"));
	ref.add(new ReferenceField(0x0008,0x1040,"Institutional Department Name"));
	ref.add(new ReferenceField(0x0008,0x1090,"Manufacturer's Model Name"));
	ref.add(new ReferenceField(0x0018,0x1000,"Device Serial Number"));
	ref.add(new ReferenceField(0x0018,0x1020,"Software Versions"));
	ref.add(new ReferenceField(0x0018,0x1050,"Inherent Spatial Resolution"));
	ref.add(new ReferenceField(0x0018,0x1200,"Date of Last Calibration"));
	ref.add(new ReferenceField(0x0018,0x1201,"Time of Last Calibration"));

    }

    public void captureReference(DicomFile df) {
	Iterator<DicomField> i;
	i = df.mfields.iterator();
	while(i.hasNext()) {
	    captureReference(i.next());
	}
    }

    public void captureReference(DicomField df) {
	int i;
	for(i=0;i<ref.size();i++)
	    if (ref.get(i).match(df)) {
		ref.get(i).setValue(df);
		return;
	    }
    }

    private int roundcos(float x) {
	if (Math.abs(x) < Math.cos(50.0 * Math.PI / 180.0)) return 0;
	if (Math.abs(x) > Math.cos(40.0 * Math.PI / 180.0)) {
	    if (x < 0.0) return -1; else return 1;
	}
	return 2;
    }

    public boolean orientationDefined() {
	return((!ox.equals("??")) && (!oy.equals("??")) && (!oz.equals("??")));
    }

    public boolean orientationDefined(int o) {
	if (o==0) return(!ox.equals("??"));
	if (o==1) return(!oy.equals("??"));
	if (o==2) return(!oz.equals("??"));
	return false;
    }

    public void computeOrientation() {
	int [] ex, ey, ez;
	int i;

	ex = new int[3];
	ey = new int[3];
	ez = new int[3];

	for(i=0;i<3;i++) {
	    ex[i] = roundcos(xcos[i]);
	    ey[i] = roundcos(ycos[i]);
	}

	ox = "??";
	oy = "??";
	oz = "??";

	if (ex[0]==0 && ex[1]==1  && ex[2]==0)  ox = "AP";
	if (ex[0]==0 && ex[1]==-1 && ex[2]==0)  ox = "PA";
	if (ex[0]==1  && ex[1]==0  && ex[2]==0) ox = "RL";
	if (ex[0]==-1 && ex[1]==0 && ex[2]==0)  ox = "LR";
	if (ex[0]==0 && ex[1]==0  && ex[2]==1)  ox = "FH";
	if (ex[0]==0 && ex[1]==0 && ex[2]==-1)  ox = "HF";

	if (ey[0]==0 && ey[1]==1  && ey[2]==0)  oy = "AP";
	if (ey[0]==0 && ey[1]==-1 && ey[2]==0)  oy = "PA";
	if (ey[0]==1  && ey[1]==0  && ey[2]==0) oy = "RL";
	if (ey[0]==-1 && ey[1]==0 && ey[2]==0)  oy = "LR";
	if (ey[0]==0 && ey[1]==0  && ey[2]==1)  oy = "FH";
	if (ey[0]==0 && ey[1]==0 && ey[2]==-1)  oy = "HF";
	
	// cross product
	ez[0] = ex[1]*ey[2] - ex[2]*ey[1];
	ez[1] = ex[2]*ey[0] - ex[0]*ey[2];
	ez[2] = ex[0]*ey[1] - ex[1]*ey[0];

	if (ez[0]==0 && ez[1]==1  && ez[2]==0)  oz = "AP";
	if (ez[0]==0 && ez[1]==-1 && ez[2]==0)  oz = "PA";
	if (ez[0]==1  && ez[1]==0  && ez[2]==0) oz = "RL";
	if (ez[0]==-1 && ez[1]==0 && ez[2]==0)  oz = "LR";
	if (ez[0]==0 && ez[1]==0  && ez[2]==1)  oz = "FH";
	if (ez[0]==0 && ez[1]==0 && ez[2]==-1)  oz = "HF";
    }

    public void datecomp() {
	int by=-1,bm=0,bd=0, sy=-1,sm=0,sd=0;

	if (birth.length() == 8) {
	    birth = new String(birth.substring(0,4) + "." + birth.substring(4,6) + "." + birth.substring(6,8));
	} else if (birth.length() == 10) {
	    birth = new String(birth.substring(0,4) + "." + birth.substring(5,7) + "." + birth.substring(8,10));
	}

	if (birth.length() >= 10) {
	    by = Integer.parseInt( birth.substring(0,4) );
	    bm = Integer.parseInt( birth.substring(5,7) );
	    bd = Integer.parseInt( birth.substring(8,10) );
	}

	if (scandate.length() == 15) {
	    scandate = new String(scandate.substring(0,4) + "." + scandate.substring(4,6) + "." + scandate.substring(6,8) + " " + scandate.substring(9,11) + ":" + scandate.substring(11,13) + ":" + scandate.substring(13,15));
	} else if (scandate.length() == 19) {
	    scandate = new String(scandate.substring(0,4) + "." + scandate.substring(5,7) + "." + scandate.substring(8,10) + " " + scandate.substring(11,13) + ":" + scandate.substring(14,16) + ":" + scandate.substring(17,19));
	}

	if (scandate.length() >= 19) {
	    sy = Integer.parseInt( scandate.substring(0,4) );
	    sm = Integer.parseInt( scandate.substring(5,7) );
	    sd = Integer.parseInt( scandate.substring(8,10) );
	}

	if (sy > 0 && by > 0) {
	    age = sy - by;
	    if (sm < bm || (sm == bm && sd < bd)) age--;
	}

    }

}