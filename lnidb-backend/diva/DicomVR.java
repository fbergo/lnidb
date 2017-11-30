
public class DicomVR {
    public String id;
    public int maxl, fixed;
    public boolean longlen;

    DicomVR(String a, int b, int c, boolean d) {
	id = a;
	maxl = b;
	fixed = c;
	longlen = d;
    }
}

