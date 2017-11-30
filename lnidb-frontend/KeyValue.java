import org.apache.commons.lang3.*;

public class KeyValue {

    public String key, value;

    public KeyValue() { key=""; value=""; }

    public KeyValue(String k, String v) { key = k; value = v; }

    public KeyValue(String x) {
	int s = x.indexOf('=');
	if (s < 0) {
	    key="";
	    value="";
	} else {
	    key   = StringUtils.trim( x.substring(0,s) );
	    value = StringUtils.trim( x.substring(s+1) );
	}
    }

    public boolean eq(String k) { return(key.equals(k)); }

    public int intValue() {
	return(Integer.parseInt(value));
    }

    public double doubleValue() {
	return(Double.parseDouble(value));
    }

}


