import java.io.*;
import java.net.*;
import java.util.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.awt.geom.*;
import javax.swing.*;
import javax.swing.border.*;

public class DicomDetailsWindow extends JFrame implements ItemListener {

    ScanInfo info;
    JCheckBox unique;
    JTextPane tp;

    public DicomDetailsWindow(ScanInfo si) {
	super("LNIDB/DIVA Applet");

	JPanel p,s;
	JScrollPane sp;
	int i;

	info = si;
	setSize(640,480);
	setDefaultCloseOperation(DISPOSE_ON_CLOSE);

	ClassLoader cldr = this.getClass().getClassLoader();
	URL url = cldr.getResource("diva.png");
	setIconImage(new ImageIcon(url).getImage());

	p = new JPanel();
	p.setLayout(new BorderLayout());
	tp = new AATextPane();
	tp.setEditable(false);
	sp = new JScrollPane(tp);
	sp.getViewport().setBackground(new Color(0x135d6c));
	tp.setBackground(new Color(0x135d6c));
	getContentPane().add(p);

	p.add(sp,BorderLayout.CENTER);
	s = new JPanel();
	s.setLayout(new FlowLayout(FlowLayout.LEFT));
	p.add(s,BorderLayout.SOUTH);
	unique = new AACheckBox("Colapsar Valores Repetidos",true);
	unique.addItemListener(this);
	s.add(unique);
       
	update();
	setVisible(true);

	setTitle("DIVA - Detalhes de ["+info.name+": "+info.series+"]");
    }

    public void itemStateChanged(ItemEvent e) {
	update();
    }

    void update() {
	StringBuffer b;
	int i;
	tp.setContentType("text/html");
	b = new StringBuffer();
	b.append("<style type=\"text/css\">body { font-family: sans-serif; font-size: 100%; background-color: #135d6c; color: white; } td.dicomtag { color: #ddffff; } td.fieldname { color: #00ffff; white-space: nowrap; } td.value { color: #ffff44; }</style><table cellpadding=2 cellpacing=2>");
	for(i=0;i<info.ref.size();i++)
	    if (!info.ref.get(i).empty())
		b.append(info.ref.get(i).htmlRow(!unique.isSelected()));
	b.append("</table>");
	tp.setText(b.toString());
	tp.setCaretPosition(0);
    }
}

class AACheckBox extends JCheckBox {
    public AACheckBox(String a, boolean b) {
	super(a,b);
    }
    public void paintComponent(Graphics g) {
	Graphics2D graphics2d = (Graphics2D) g;
	graphics2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				    RenderingHints.VALUE_ANTIALIAS_ON);
	super.paintComponent(g);
    }
}

class AATextPane extends JTextPane {
    public void paintComponent(Graphics g) {
	Graphics2D graphics2d = (Graphics2D) g;
	graphics2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				    RenderingHints.VALUE_ANTIALIAS_ON);
	super.paintComponent(g);
    }
}
