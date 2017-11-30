import java.awt.*;
import javax.swing.*;

public class QLabel extends JLabel {

    public QLabel(String text, int hborder, int vborder) {
	super(text);
	init(hborder, vborder);
    }

    public QLabel(String text) {
	super(text);
	init(8,8);
    }

    void init(int hb, int vb) {
	if (hb > 0 || vb >0)
	    setBorder(BorderFactory.createEmptyBorder(vb, hb, vb, hb));
	setFont(new Font(Font.SANS_SERIF, Font.PLAIN, 12));
    }

}