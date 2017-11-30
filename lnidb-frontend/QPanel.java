import java.awt.*;
import javax.swing.*;

public class QPanel extends JPanel {

    public float alignX, alignY;

    public QPanel(boolean horizontal, int hborder, int vborder) {
	super();
	init(horizontal, hborder, vborder);
    }

    public QPanel(boolean horizontal, int border) {
	super();
	init(horizontal, border, border);
    }

    public QPanel(boolean horizontal) {
	super();
	init(horizontal, 8, 8);
    }

    public QPanel() {
	super();
	init(true,8,8);
    }

    void init(boolean horizontal, int hborder, int vborder) {
	setOpaque(true);
	setBackground(new Color(0x2c89a0));
	setLayout(new BoxLayout(this, horizontal ? BoxLayout.X_AXIS : BoxLayout.Y_AXIS));
	if (hborder > 0 || vborder >0)
	    setBorder(BorderFactory.createEmptyBorder(vborder, hborder, vborder, hborder));
	alignX = alignY = 0.0f;
    }

    public float getAlignmentX() { return alignX; }
    public float getAlignmentY() { return alignY; }
}
