import java.util.*;

public class PatQuery {
    public String  name, hc, age, scanner, comments, attach;
    public int gender, page, npages, total, reqsort;
    public String qdesc, sortby;
    public Vector<PatResult> vpat;

    public PatQuery() {
	reset();
    }

    public void reset() {
	name = "";
	hc = "";
	age = "";
	scanner = "";
	comments = "";
	attach = "";
	gender = 0;
	page = npages = total = reqsort = 0;
	vpat = new Vector<PatResult>();
    }

}

