

public class PatResult {
    public int id, seq, age, attach;
    public String name, hc, gender, scanner, comments;

    public PatResult() {
	id=age=attach=seq=0;
	name="";
	hc="";
	gender="";
	scanner="";
	comments="";
    }

    public boolean isEmpty() {
	return(id==0 && seq==0 && name.length()==0);
    }

}
