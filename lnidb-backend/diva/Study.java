import java.util.*;

public class Study {

    public int id, files, size;
    public String date,exam,series,dimension,voxelSize,thickness;
    public String scannerMaker,scannerModel,scannerLocation;
    public String orientation,fids;

    public Study() {
	id=files=size=-1;
    }

    public String date() {
	StringTokenizer t = new StringTokenizer(date, " \r\n\t");
	return(t.nextToken());
    }

    public String time() {
	StringTokenizer t = new StringTokenizer(date, " \r\n\t");
	t.nextToken();
	return(t.nextToken());
    }
}
