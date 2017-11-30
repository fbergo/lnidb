
public class Volume {

    public int W,H,D,N,WH;
    public double dx,dy,dz;
    public short data[];

    public Volume(int w,int h,int d) {
	int i;
	dx=dy=dz=1.0;
	W=w; H=h; D=d;
	WH = W*H;
	N = W*H*D;
	data = new short[N];
	for(i=0;i<N;i++)
	    data[i] = 0;
    }

    public void set(int x,int y,int z,short val) {
	data[ x + y*W + z*WH ] = val;
    }

    public short get(int x,int y,int z) {
	return data[ x + y*W + z*WH ];
    }

    public int address(int x,int y,int z) {
	return(x+y*W+z*WH);
    }

    public boolean valid(int x,int y,int z) {
	return((x>=0)&&(x<W)&&(y>=0)&&(y<H)&&(z>=0)&&(z<=D));
    }

    public int xOf(int a) { return((a%WH) % W ); }
    public int yOf(int a) { return((a%WH) / W ); }
    public int zOf(int a) { return(a/WH); }

    public double maxres() {
	if (dx <= dy && dx <= dz) return dx;
	if (dy <= dx && dy <= dz) return dy;
	return dz;
    }
    
    public int interpW(double res) { return( (int) Math.ceil( W*(dx/res) ) ); }
    public int interpH(double res) { return( (int) Math.ceil( H*(dy/res) ) ); }
    public int interpD(double res) { return( (int) Math.ceil( D*(dz/res) ) ); }


    public short maximum() {
	int i;
	short m = data[0];
	for(i=1;i<N;i++) if (data[i] > m) m = data[i];
	return m;
    }

    public short minimum() {
	int i;
	short m = data[0];
	for(i=1;i<N;i++) if (data[i] < m) m = data[i];
	return m;
    }

};

