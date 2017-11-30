import java.io.*;
import java.util.*;

public class DicomField {

    public int group, key;
    public byte[] value;
    boolean endianflip;

    public DicomField() {
	group   = 0;
	key     = 0;
        value   = null;
	endianflip = false;
    }

    public DicomField(boolean flip) {
	group   = 0;
	key     = 0;
        value   = null;
	endianflip = flip;
    }

    public DicomField(int g,int k, boolean flip) {
	group = g;
	key   = k;
	value = null;
	endianflip = flip;
    }

    public void setValue(byte[] b) {
	int i;
	value = new byte[b.length];
	for(i=0;i<value.length;i++) value[i] = b[i];
    }

    public boolean match(int g,int k) {
	return(g==group && k==key);
    }

    public int length() {
	if (value == null) return 0; else return(value.length);
    }

    public void discard() {
	value = null;
    }

    public String toString() {
	String s;
	if (value == null) return ("NULL");
	try {
	    int i;
	    i = value.length;
	    while(i>1 && value[i-1]==0)
		i--;
	    s = new String(value, 0, i, "US-ASCII");
	} catch(UnsupportedEncodingException e) {
	    s = "ERROR";
	}
	return s;
    }

    public int toInteger() {
	if (value == null) return 0;
	StringTokenizer st = new StringTokenizer(toString(), " \t\r\n\\/");
	if (st.hasMoreTokens())	    
	    return(Integer.parseInt(st.nextToken()));
	else
	    return 0;
    }

    public float toFloat() {
	if (value == null) return 0.0f;
	StringTokenizer st = new StringTokenizer(toString(), " \t\r\n\\/");
	if (st.hasMoreTokens())	    
	    return(Float.parseFloat(st.nextToken()));
	else
	    return 0;
    }

    public static int parse16(byte[] b, int offset, boolean eflip) {
	int v;
	if (eflip)
	    v = (0x000000ff & (int)b[offset+1]) | ((0x000000ff & (int)b[offset]) << 8);
	else
	    v = (0x000000ff & (int)b[offset]) | ((0x000000ff & (int)b[offset+1]) << 8);
	return v;
    }

    public static int parse32(byte[] b, int offset, boolean eflip) {
	int v;
	if (eflip)
	    v = (0x000000ff & (int)b[offset+3]) | ((0x000000ff & (int)b[offset+2]) << 8) |
		((0x000000ff & (int)b[offset+1]) << 16) | ((0x000000ff & (int)b[offset]) << 24);
	else
	    v = (0x000000ff & (int)b[offset]) | ((0x000000ff & (int)b[offset+1]) << 8) |
		((0x000000ff & (int)b[offset+2]) << 16) | ((0x000000ff & (int)b[offset+3]) << 24);
	return v;
    }

    public int parse16(byte[] b, int offset) {
	int v;
	if (endianflip)
	    v = (0x000000ff & (int)b[offset+1]) | ((0x000000ff & (int)b[offset]) << 8);
	else
	    v = (0x000000ff & (int)b[offset]) | ((0x000000ff & (int)b[offset+1]) << 8);
	return v;
    }

    public int parse32(byte[] b, int offset) {
	int v;
	if (endianflip)
	    v = (0x000000ff & (int)b[offset+3]) | ((0x000000ff & (int)b[offset+2]) << 8) |
		((0x000000ff & (int)b[offset+1]) << 16) | ((0x000000ff & (int)b[offset]) << 24);
	else
	    v = (0x000000ff & (int)b[offset]) | ((0x000000ff & (int)b[offset+1]) << 8) |
		((0x000000ff & (int)b[offset+2]) << 16) | ((0x000000ff & (int)b[offset+3]) << 24);
	return v;
    }


}

