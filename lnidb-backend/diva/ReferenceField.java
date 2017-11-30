import java.util.*;

public class ReferenceField {

    public int group, key;
    public String name;
    public Vector<String> values;
    public Vector<String> mvalues;

    public ReferenceField(int g,int k,String n) {
	group = g; key = k;
	name = n;
	values = new Vector<String>();
	mvalues = new Vector<String>();
    }

    public boolean match(DicomField df) {
	return (df.group == group && df.key == key);
    }

    public void setValue(DicomField df) {
	int i;
	mvalues.add(df.toString());
	for(i=0;i<values.size();i++)
	    if (values.get(i).equals(df.toString()))
		return;
	values.add(df.toString());
    }

    public boolean empty() {
	return(values.size() == 0);
    }

    public String headerString() {
	return("("+String.format("%04x",group)+","+String.format("%04x",key)+") "+name);
    }

    public String htmlRow(boolean multi) {
	return("<tr valign=top><td class=\"dicomtag\">("+String.format("%04x",group)+","+String.format("%04x",key)+")</td><td class=\"fieldname\">"+name+"</font></td><td class=\"value\">"+valueString(multi)+"</td></tr>");
    }

    public String valueString(boolean multi) {
	StringBuffer b = new StringBuffer();
	int i;
	Vector<String> x;
	if (multi) x = mvalues; else x = values;

	for(i=0;i<x.size();i++) {
	    b.append(x.get(i));
	    if (i<x.size()-1)
		b.append(", ");
	}
	return(b.toString());
    }

}
