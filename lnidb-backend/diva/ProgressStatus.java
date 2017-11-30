
public class ProgressStatus {

    private int total, done;

    public ProgressStatus() {
	total = 1;
	done = 0;
    }

    public ProgressStatus(int pTotal) {
	total = pTotal;
	done = 0;
    }

    public void setDone(int pDone) {
	done = pDone;
	if (done > total) done = total;
	if (done < 0) done = 0;
    }

    public void incrementDone(int pDone) {
	done += pDone;
	if (done > total) done = total;
    }

    public void setTotal(int pTotal) {
	total = pTotal;
	if (total < 1) total = 1;
	if (done > total) done = total;
    }
    
    public int getDone() {
	return done;
    }

    public int getTotal() {
	return total;
    }

    public double getRatio() {
	return ( ((double)done) / ((double)total) );
    }

    public double getPercentRatio() {
	return (100.0 * getRatio());
    }
}

