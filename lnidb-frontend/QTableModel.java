import java.awt.*;
import javax.swing.*;
import javax.swing.table.*;

public class QTableModel extends AbstractTableModel {
    String[] cnames;
    Object[] data;
    int W,H, maxrows;

    QTableModel(int cols, int _maxrows) {
	int i;
	W = cols;
	H = 0;
	maxrows = _maxrows;
	cnames = new String[W];
	for(i=0;i<W;i++)
	    cnames[i] = "col" + (i+1);
	data = new Object[W*maxrows];
    }

    public void setColumnName(int col, String s) { 
	if (col>=0 && col<W) {
	    cnames[col] = s; 
	    fireTableStructureChanged();
	}
    }

    public int    getColumnCount() { return W; }
    public int    getRowCount() { return H; }
    public String getColumnName(int col) { return( ( col>=0 && col<W ) ? cnames[col] : null); }
    public Object getValueAt(int row, int col) { 
	return( (data!=null && row>=0 && col>=0 && row<H && col<W) ? data[row*W+col] : null);
    }

    public void setValueAt(Object val, int row, int col) { 
	if (data!=null && row>=0 && col>=0 && row<H && col<W) {
	    data[row*W+col] = val;
	    fireTableCellUpdated(row,col);
	}
    }

    public Class  getColumnClass(int c) { return(getValueAt(0,c).getClass()); }

    public void clear() {
	int i;
	for(i=0;i<W*H;i++) data[i]=null;
	H=0;
	fireTableStructureChanged();
    }

    public void appendRow() {
	int i;
	if (maxrows == H) grow(10);
	H++;
	for(i=0;i<W;i++) data[(H-1)*W + i] = null;
	fireTableRowsInserted(H-1,H-1);
    }

    private void grow(int plus) {
	Object[] ndata;
	int i;
	ndata = new Object[ (maxrows+plus) * W ];
	for(i=0;i<maxrows*W;i++)
	    ndata[i] = data[i];
	data = ndata;
	maxrows += plus;
    }

}
