import java.awt.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import java.io.*;
import org.apache.http.*;
import org.apache.http.client.*;
import org.apache.http.client.fluent.*;
import org.apache.http.client.methods.*;
import org.apache.http.impl.client.*;
import org.apache.http.util.*;
import org.apache.commons.lang3.*;

public class QueryWindow extends JFrame implements ActionListener, ListSelectionListener {

    JTabbedPane tabs;
    JComponent[] fpat;
    QTableModel  tpat;
    PatQuery     qpat;
    JLabel       status;
    ViewerConfig appconfig;

    final int patw[] = { 30, 110, 40, 50, 120, 120, 100 };
    
    public QueryWindow() {
        super("LNIDB Viewer");

        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setPreferredSize(new Dimension(1000,800));
	setIconImage(new ImageIcon("viewer_icon.png").getImage());

	appconfig = new ViewerConfig();
	qpat = new PatQuery();

        JPanel mp = new BLPanel();
        getContentPane().add(mp);

        tabs = new JTabbedPane();
        mp.add(tabs,BorderLayout.CENTER);

	{
	    status = new JLabel("LNIDB Viewer versão " + ViewerMain.VERSION);
	    status.setFont(new Font(Font.SANS_SERIF, Font.PLAIN, 12));
	    JPanel lp = new JPanel();
	    lp.setLayout(new BoxLayout(lp, BoxLayout.X_AXIS));
	    lp.setBorder(BorderFactory.createEmptyBorder(4,4,4,4));
	    lp.add(status);
	    lp.add(Box.createHorizontalGlue());


	    JButton cfg = new JButton(new ImageIcon("gear.png"));
	    cfg.setActionCommand("config");
	    cfg.addActionListener(this);
	    cfg.setToolTipText("Configurações");

	    lp.add(cfg);

	    mp.add(BorderLayout.SOUTH, lp);	    
	}

        JPanel pat = new BLPanel();
        JPanel stu = new BLPanel();

        tabs.addTab("Pacientes", pat);
        tabs.addTab("Estudos", stu);

        JPanel p1 = new QPanel(false,10);
        pat.add(BorderLayout.WEST, p1);

        // pat pane
        fpat = new JComponent[19];

        fpat[0] = addField("Nome:", p1);
        fpat[1] = addField("HC/Pat-Id:", p1);
        fpat[2] = addField("Idade:", p1);
        fpat[2].setToolTipText("Exemplos: 30, 40-50");

        {
            JPanel h1 = new QPanel(true,2,6);
            JLabel l;
            String[] vgender = { "Qualquer", "M/F", "M", "F", "O" };
            h1.add(l = new JLabel("Sexo:"));
            l.setForeground(Color.WHITE);
            h1.add(Box.createRigidArea(new Dimension(4,1)));
            h1.add(fpat[3] = new JComboBox(vgender));
            fpat[3].setBorder(BorderFactory.createLineBorder(Color.BLACK));
            fpat[3].setMaximumSize(fpat[3].getPreferredSize());
            p1.add(h1);
        }

        fpat[4] = addField("Scanner:", p1);
        fpat[5] = addField("Comentários:", p1);
        fpat[5].setToolTipText("Formato: valor[,valor...]");
        fpat[6] = addField("Anexos:", p1);
        fpat[6].setToolTipText("Formato: valor[,valor...]");

        {
            JPanel h1 = new QPanel(true,2,6);
            JLabel l;
            String[] vsort = { "Nome do Paciente", "HC", "Sexo/Idade" };
            h1.add(l = new JLabel("Ordenação:"));
            l.setForeground(Color.WHITE);
            h1.add(Box.createRigidArea(new Dimension(4,1)));
            h1.add(fpat[17] = new JComboBox(vsort));
            fpat[17].setBorder(BorderFactory.createLineBorder(Color.BLACK));
            fpat[17].setMaximumSize(fpat[17].getPreferredSize());
            p1.add(h1);
        }

        {
            JPanel h1 = new QPanel(true,2,6);

            h1.add(Box.createHorizontalGlue());
            h1.add(fpat[7] = new JButton("Limpar"));
            h1.add(Box.createRigidArea(new Dimension(8,1)));
            h1.add(fpat[8] = new JButton("Atualizar"));
            p1.add(Box.createRigidArea(new Dimension(4,10)));
            p1.add(h1);

            ((JButton)fpat[7]).setActionCommand("pat.clear");
            ((JButton)fpat[8]).setActionCommand("pat.search");
            ((JButton)fpat[7]).addActionListener(this);
            ((JButton)fpat[8]).addActionListener(this);
        }

        // result browser
        p1 = new QPanel(false,10);
        p1.setBackground(Color.WHITE);
        pat.add(BorderLayout.CENTER, p1);
        {
            JPanel h1 = new QPanel(true,8,2);
            h1.setBackground(Color.WHITE);

            fpat[9] = new QLabel("Nenhuma Consulta.");
	    fpat[18] = new JButton("Abrir");
            h1.add(fpat[9]);
            h1.add(Box.createHorizontalGlue());
            h1.add(fpat[18]);

            p1.add(h1);

	    ((JButton)fpat[18]).setActionCommand("pat.open");
	    ((JButton)fpat[18]).addActionListener(this);
	    fpat[18].setEnabled(false);
        }
        {
            JPanel h1 = new QPanel(true,8,2);
            Font s10 = new Font(Font.SANS_SERIF, Font.PLAIN, 10);
            int i;
            String[] vcmd = {  "pat.first", "pat.prev", "pat.gotopage", "pat.next", "pat.last" };
            h1.setBackground(Color.WHITE);

            fpat[10] = new QLabel("0 itens em 0 páginas.");
            h1.add(fpat[10]);
            h1.add(Box.createHorizontalGlue());
            fpat[11] = new JButton("Primeira");
            fpat[12] = new JButton("Anterior");
            fpat[13] = new JComboBox();
            fpat[13].setMaximumSize(fpat[13].getPreferredSize());
            fpat[14] = new JButton("Próxima");
            fpat[15] = new JButton("Última");
            for(i=11;i<=15;i++) {
                h1.add(Box.createRigidArea(new Dimension(8,1)));
                fpat[i].setFont(s10);
                h1.add(fpat[i]);
                if (i!=13) { 
                    ((JButton)fpat[i]).setActionCommand( vcmd[i-11] );
                    ((JButton)fpat[i]).addActionListener(this);
                } else {
                    ((JComboBox)fpat[i]).setActionCommand( vcmd[i-11] );
                    ((JComboBox)fpat[i]).addActionListener(this);
                }
		fpat[i].setEnabled(false);
            }

            p1.add(h1);
	}
        {
            tpat = new QTableModel(7,100);
            tpat.setColumnName(0,"#");
            tpat.setColumnName(1,"Paciente");
            tpat.setColumnName(2,"HC");
            tpat.setColumnName(3,"Idade/Sexo");
            tpat.setColumnName(4,"Scanners");
            tpat.setColumnName(5,"Comentários");
            tpat.setColumnName(6,"Anexos");

	    /*
            int i;
            for(i=0;i<200;i++) {
                tpat.appendRow();
                tpat.setValueAt(""+(i+1),i,0);
                tpat.setValueAt("Fulano de Tal",i,1);
                tpat.setValueAt("123456",i,2);
                tpat.setValueAt("30/M",i,3);
                tpat.setValueAt("Philips",i,4);
                tpat.setValueAt("",i,5);
                tpat.setValueAt("",i,6);
            }
	    */

            JTable t = new JTable(tpat);
	    t.setFont(new Font(Font.SANS_SERIF,Font.PLAIN,10));
            fpat[16] = t;
            JScrollPane sp = new JScrollPane(t);
	    t.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
            t.setFillsViewportHeight(true);
            t.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
            tableFit(t,patw);
	    t.getSelectionModel().addListSelectionListener(this);

            p1.add(sp);

	    t.addMouseListener(new MouseAdapter() {
		    public void mousePressed(MouseEvent me) {
			JTable table =(JTable) me.getSource();
			int row = table.rowAtPoint(me.getPoint());
			if (me.getClickCount() == 2) patOpen(row);
		    }
		});
        }

        pack();
        setVisible(true);
    }

    private void patOpen(int row) {
	int id = qpat.vpat.elementAt(row).id;
        tabs.addTab("P/"+id+": "+qpat.vpat.elementAt(row).name, new PatDetailPane(id));
	tabs.setSelectedIndex(tabs.getTabCount()-1);
    }

    JComponent addField(String label, JComponent parent) {
        JComponent h1 = new QPanel(true,2,6);
        JComponent r;
        JLabel l;
        h1.add(l = new JLabel(label));
        l.setForeground(Color.WHITE);
        h1.add(Box.createRigidArea(new Dimension(4,1)));
        h1.add(r = new QText(15));
        parent.add(h1);
        return r;
    }

    public void itemStateChanged(ItemEvent ie) {	

	System.out.println( ((String) ie.getItem()) + " : " + ie.getStateChange() );

	if (ie.getItemSelectable() == (ItemSelectable) fpat[13] && ie.getStateChange() == ItemEvent.SELECTED) {
	    int npg = Integer.parseInt( (String) ie.getItem() );
	    System.out.println("goto "+npg+", cur="+qpat.page);
	    if (qpat.page>0 && npg != qpat.page) {
		qpat.page = npg;
		patSearch2();
	    }
	}
    }

    public void valueChanged(ListSelectionEvent lse) {
	int sel;

	sel = ((JTable)fpat[16]).getSelectedRow();
	((JButton)fpat[18]).setEnabled(sel >= 0);
    }

    public void actionPerformed(ActionEvent ae) {
        String acmd = ae.getActionCommand();

        if (acmd.equals("config")) {
	    JDialog dlg = new ConfigDialog(this, appconfig);
        } else if (acmd.equals("pat.clear")) {
            patClear();
        } else if (acmd.equals("pat.search")) {
            patSearch();
        } else if (acmd.equals("pat.gotopage")) {
	    try {
		int npg = Integer.parseInt( (String) ((JComboBox)fpat[13]).getSelectedItem() );
		if (qpat.page > 0 && npg != qpat.page) {
		    qpat.page = npg;
		    patSearch2();
		}
	    } catch(Exception e) { return; }
        } else if (acmd.equals("pat.first")) {
	    if (qpat.page != 1) {
		qpat.page = 1;
		patSearch2();
	    }
        } else if (acmd.equals("pat.prev")) {
	    if (qpat.page > 1) {
		qpat.page--;
		patSearch2();
	    }
        } else if (acmd.equals("pat.next")) {
	    if (qpat.page < qpat.npages) {
		qpat.page++;
		patSearch2();
	    }
        } else if (acmd.equals("pat.last")) {
	    if (qpat.page < qpat.npages) {
		qpat.page = qpat.npages;
		patSearch2();
	    }
        } else if (acmd.equals("pat.open")) {
	    patOpen( ((JTable)fpat[16]).getSelectedRow() );
        } else {
            System.out.println("action performed: "+ae.getActionCommand());
        }

    }

    private void patSearch() {
	qpat.reset();
	qpat.name     = StringUtils.trimToEmpty(((JTextField)fpat[0]).getText());
	qpat.hc       = StringUtils.trimToEmpty(((JTextField)fpat[1]).getText());
	qpat.age      = StringUtils.trimToEmpty(((JTextField)fpat[2]).getText());
	qpat.gender   = ((JComboBox)fpat[3]).getSelectedIndex();
	qpat.scanner  = StringUtils.trimToEmpty(((JTextField)fpat[4]).getText());
	qpat.comments = StringUtils.trimToEmpty(((JTextField)fpat[5]).getText());
	qpat.attach   = StringUtils.trimToEmpty(((JTextField)fpat[6]).getText());
	qpat.page     = 1;
	qpat.reqsort  = 1 + ((JComboBox)fpat[17]).getSelectedIndex();

	patSearch2();
    }

    private void patSearch2() {	
        try {
	    int i;

	    for(i=11;i<=15;i++)
		fpat[i].setEnabled( false );
	    fpat[18].setEnabled(false);

	    status.setText("Enviando consulta...");
	    Content c = 
		Request.
		Post( appconfig.BaseURL + "zpatquery.php" ).
		bodyForm(Form.form().
			 add("patient",qpat.name).
			 add("patcode",qpat.hc).
			 add("age",    qpat.age).
			 add("gender", ""+qpat.gender).
			 add("scanner", ""+qpat.scanner).
			 add("comments", ""+qpat.comments).
			 add("attach", ""+qpat.attach).
			 add("page", ""+qpat.page).
			 add("sort", ""+qpat.reqsort).
			 build()).execute().returnContent();

	    status.setText("Formatando resultados...");
	    String[] lines = StringUtils.split(c.asString(),"\r\n");
	    qpat.vpat.clear();
	    PatResult pr = new PatResult();
	    for(i=0;i<lines.length;i++) {
		//System.out.println("vpat size="+qpat.vpat.size()+" isempty="+pr.isEmpty());
		//System.out.println("parsing [" + lines[i] + "]");

		if (lines[i].length()<3 || lines[i].charAt(0) == '#') continue;
		KeyValue kv = new KeyValue(lines[i]);
		//System.out.println("key="+kv.key+", value="+kv.value);

		if (kv.eq("id")) {
		    //System.out.println("id found");
		    if (!pr.isEmpty())
			qpat.vpat.add(pr);
		    pr = new PatResult();
		    pr.id = kv.intValue();
		    continue;
		}

		if (kv.eq("seq"))      { pr.seq      = kv.intValue();  continue; }
		if (kv.eq("nome"))     { pr.name     = kv.value;       continue; }
		if (kv.eq("hc"))       { pr.hc       = kv.value;       continue; }
		if (kv.eq("age"))      { pr.age      = kv.intValue();  continue; }
		if (kv.eq("gender"))   { pr.gender   = kv.value;       continue; }
		if (kv.eq("scanner"))  { pr.scanner  = kv.value;       continue; }
		if (kv.eq("comments")) { pr.comments = StringEscapeUtils.unescapeHtml4(kv.value); continue; }
		if (kv.eq("attachs"))  { pr.attach   = kv.intValue();  continue; }
		
		if (kv.eq("querydesc")) { qpat.qdesc  = kv.value;      continue; }
		if (kv.eq("sortby"))    { qpat.sortby = kv.value;      continue; }
		if (kv.eq("npages"))    { qpat.npages = kv.intValue(); continue; }
		if (kv.eq("page"))      { qpat.page   = kv.intValue(); continue; }
		if (kv.eq("ntotal"))    { qpat.total  = kv.intValue(); continue; }
		if (kv.eq("error"))     { status.setText("Erro: "+kv.value); return; }

	    }
	    if (!pr.isEmpty())
		qpat.vpat.add(pr);
	    pr = null;

	    // header + paging
	    ((JLabel)fpat[9]).setText("<html>Consulta: <span color=#008800>"+qpat.qdesc+"</span>, ordenado por <span color=#008800>"+qpat.sortby+"</span></html>");

            ((JLabel)fpat[10]).setText(""+qpat.total+" "+PTBR.maybePlural("item",qpat.total)+" em "+
				       qpat.npages+" "+PTBR.maybePlural("página",qpat.npages)+".");
	    ((JComboBox)fpat[13]).removeActionListener(this);
	    ((JComboBox)fpat[13]).removeAllItems();
	    for(i=1;i<=qpat.npages;i++)
		((JComboBox)fpat[13]).addItem(""+i);
	    ((JComboBox)fpat[13]).setSelectedIndex(qpat.page-1);
	    ((JComboBox)fpat[13]).addActionListener(this);

	    fpat[11].setEnabled( qpat.page > 1 );
	    fpat[12].setEnabled( qpat.page > 1 );
	    fpat[13].setEnabled( qpat.npages > 1 );
	    fpat[14].setEnabled( qpat.page < qpat.npages );
	    fpat[15].setEnabled( qpat.page < qpat.npages );

	    // table
	    tpat.clear();
	    for(i=0;i<qpat.vpat.size();i++) {
		pr = qpat.vpat.elementAt(i);
		tpat.appendRow();
		tpat.setValueAt(""+pr.seq,i,0);
		tpat.setValueAt(pr.name,i,1);
		tpat.setValueAt(pr.hc,i,2);
		tpat.setValueAt(""+pr.age+"/"+pr.gender,i,3);
		tpat.setValueAt(pr.scanner,i,4);
		tpat.setValueAt(pr.comments,i,5);
		tpat.setValueAt(""+pr.attach,i,6);
	    }
            tableFit((JTable)fpat[16],patw);


	    status.setText("Consulta concluída.");


        } catch(Exception e) { 	    
	    status.setText("Erro na consulta (Servidor/Rede).");
	    e.printStackTrace();
	}

    }

    private void patClear() {
        ((JTextField)fpat[0]).setText("");
        ((JTextField)fpat[1]).setText("");
        ((JTextField)fpat[2]).setText("");
        ((JComboBox)fpat[3]).setSelectedIndex(0);
        ((JTextField)fpat[4]).setText("");
        ((JTextField)fpat[5]).setText("");
        ((JTextField)fpat[6]).setText("");
        ((JComboBox)fpat[17]).setSelectedIndex(0);
    }

    private void tableFit(JTable t, int[] minwidths) {
        int i,j,c,r,w,hw;
        c = t.getModel().getColumnCount();
        r = t.getModel().getRowCount();
        for(i=0;i<c;i++) {
            w = 10;
            for(j=0;j<r;j++) {
                Dimension d = t.prepareRenderer(t.getCellRenderer(j,i),j,i).getPreferredSize();
                d.width += 10;
                if (d.width > w) w = d.width;
            }
            TableCellRenderer tcr = t.getTableHeader().getColumnModel().getColumn(i).getHeaderRenderer();
            if (tcr==null) tcr = t.getTableHeader().getDefaultRenderer();
            hw = 10 + tcr.getTableCellRendererComponent(t, t.getTableHeader().getColumnModel().getColumn(i).getHeaderValue(), false, false, -1, i).getPreferredSize().width; // \o/ Because... JAVA!
            if (hw > w) w = hw;
            if (minwidths!=null && w < minwidths[i]) w = minwidths[i];
            t.getColumnModel().getColumn(i).setPreferredWidth(w);
        }
    }

    // inner classes

    class PatDetailPane extends JPanel {
	
	int id;
	String nome,hc,birth,gender;
	Vector<String> vs;
	Vector<Comment> vc;
	Vector<Study> vstu;
	int age;
	PatDetailRenderer pdr;

	public PatDetailPane(int patid) {
	    super();
	    id = patid;
	    vs = new Vector<String>();
	    vc = new Vector<Comment>();
	    vstu = new Vector<Study>();

	    setLayout(new BorderLayout());
	    pdr = new PatDetailRenderer();
	    JScrollPane sp = new JScrollPane(pdr);
	    add(BorderLayout.CENTER, sp);

	    retrieve();
	}

	private void retrieve() {
	    try {
		int i;

		status.setText("Enviando consulta...");
		Content c = 
		    Request.
		    Post( appconfig.BaseURL + "zpatdetail.php" ).
		    bodyForm(Form.form().
			     add("id",""+id).
			     build()).execute().returnContent();

		status.setText("Formatando resultados...");
		String[] lines = StringUtils.split(c.asString(),"\r\n");

		Comment nc = new Comment();
		Study ns = new Study();

		for(i=0;i<lines.length;i++) {
		    //System.out.println(lines[i]);
		    if (lines[i].length()<3 || lines[i].charAt(0) == '#') continue;
		    KeyValue kv = new KeyValue(lines[i]);
		    
		    if (kv.eq("nome"))     { nome   = kv.value;    continue; }
		    if (kv.eq("hc"))       { hc     = kv.value;    continue; }
		    if (kv.eq("birth"))    { birth  = kv.value;    continue; }
		    if (kv.eq("age"))      { age    = kv.intValue(); continue; }
		    if (kv.eq("gender"))   { gender = kv.value;    continue; }
		    if (kv.eq("error"))    { status.setText("Erro: "+kv.value); return; }

		    if (kv.eq("scanner"))  { vs.add(kv.value); continue; }

		    if (kv.eq("cpublic"))  { 
			if (!nc.empty()) { vc.add(nc); nc=new Comment(); }
			nc.pub = kv.intValue(); continue;
		    }
		    if (kv.eq("cauthor"))  { nc.author = kv.value; continue; }
		    if (kv.eq("cdate"))    { nc.date = kv.value; continue; }
		    if (kv.eq("ctext"))    { nc.text = kv.value; continue; }

		    if (kv.eq("sid"))  { 
			if (!ns.empty()) { vstu.add(ns); ns=new Study(); }
			ns.id = kv.intValue(); continue;
		    }
		    if (kv.eq("sdate"))    { ns.date = kv.value; continue; }
		    if (kv.eq("sexam"))    { ns.exam = kv.value; continue; }
		    if (kv.eq("sseries"))  { ns.series = kv.value; continue; }
		    if (kv.eq("sori"))     { ns.ori = kv.value; continue; }
		    if (kv.eq("sdim"))     { ns.dim = kv.value; continue; }
		    if (kv.eq("spdim"))    { ns.pdim = kv.value; continue; }
		    if (kv.eq("sthick"))   { ns.thick = kv.value; continue; }
		    if (kv.eq("sfiles"))   { ns.files = kv.intValue(); continue; }
		    if (kv.eq("sdisk"))    { ns.disk = kv.value; continue; }
		    if (kv.eq("sscanner")) { ns.scanner = kv.value; continue; }
		    if (kv.eq("squality")) { ns.quality = kv.intValue(); continue; }

		}
		if (!nc.empty()) vc.add(nc);
		if (!ns.empty()) vstu.add(ns);
		
		status.setText("Consulta concluída.");
		pdr.measure();
		revalidate();
		repaint();

	    } catch(Exception e) { 	    
		status.setText("Erro na consulta (Servidor/Rede).");
		e.printStackTrace();
	    }
	} // retrieve()
	
	class PatDetailRenderer extends JPanel {
	    
	    Font f1,f2,f3,f4;
	    Dimension d;
	    final String[] ct = {"Data/Hora","Exame","Série","Dimensões","Scanner","Armazenamento","Qualidade","Operações"};
	    Color[] scolor;

	    public PatDetailRenderer() {
		f1 = new Font(Font.SANS_SERIF,Font.PLAIN,12);
		f2 = new Font(Font.SANS_SERIF,Font.BOLD,14);
		f3 = new Font(Font.SANS_SERIF,Font.BOLD,12);
		f4 = new Font(Font.SANS_SERIF,Font.PLAIN,10);
		scolor = new Color[8];
		scolor[0] = Color.ORANGE;
		scolor[1] = Color.GREEN;
		scolor[2] = Color.BLUE;
		scolor[3] = Color.MAGENTA;
		scolor[4] = Color.YELLOW;
		scolor[5] = Color.RED;
		scolor[6] = Color.CYAN;
		scolor[7] = Color.PINK;

		measure();
	    }

	    public void measure() {
		FontMetrics fm3,fm2,fm4,fm1;
		int h1,h2,h4;
		int w=0,h=0,i,j;

		fm1 = getFontMetrics(f1);
		fm3 = getFontMetrics(f3);
		fm2 = getFontMetrics(f2);
		fm4 = getFontMetrics(f4);
		h1 = fm3.getHeight();
		h2 = fm2.getHeight();
		h4 = fm4.getHeight();

		h += 10 + 4*h1 + 20 + h2 + 10 + h1*vs.size() + 10 + h2 + 10 + 2*h1*vc.size() + 10 + h2 + 20 + h1 + (3*h4*(vstu.size()+1));

		// w
		int[] cw, rw;
		cw = new int[8];
		rw = new int[8];
		for(i=0;i<8;i++) cw[i] = fm3.stringWidth(ct[i])+10;
		for(i=0;i<vstu.size();i++) {
		    Study s = vstu.elementAt(i);
		    rw[0] = fm4.stringWidth(s.date);
		    rw[1] = fm4.stringWidth(s.exam);
		    rw[2] = fm4.stringWidth(s.series);
		    rw[3] = Math.max(Math.max(fm4.stringWidth(s.ori + " " +s.dim),
					      fm4.stringWidth(s.pdim)),
				     fm4.stringWidth("thickness "+s.thick));
		    rw[4] = 12;
		    rw[5] = Math.max(fm4.stringWidth(s.disk), fm4.stringWidth(""+s.files+" arquivo(s)"));
		    rw[6] = 16;
		    rw[7] = 32;
		    for(j=0;j<8;j++)
			cw[j] = Math.max(cw[j],rw[j]+4);
		}
		for(j=0;j<8;j++)
		    cw[j]+=5;
		w = 60;
		for(j=0;j<8;j++) w += cw[j];
		w = Math.max(800,w);

		d = new Dimension(w,h);
	    }

	    public void validate() {
		measure();
	    }

	    public Dimension getMinimumSize()   { return d; }
	    public Dimension getPreferredSize() { return d; }
	    public Dimension getMaximumSize()   { return d; }

	    public void paint(Graphics g) {
		Dimension cd = getSize(null);
		FontMetrics fm1,fm2,fm3,fm4;
		int h1,h2,h3,h4,w,x,y,i,j;

		((Graphics2D)g).setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING,
						 RenderingHints.VALUE_TEXT_ANTIALIAS_LCD_HRGB);

		fm1 = getFontMetrics(f1);
		fm2 = getFontMetrics(f2);
		fm3 = getFontMetrics(f3);
		fm4 = getFontMetrics(f4);
		h1 = fm1.getHeight();
		h2 = fm2.getHeight();
		h3 = fm3.getHeight();
		h4 = fm4.getHeight();

		g.setColor(Color.WHITE);
		g.fillRect(0,0,cd.width,cd.height);
		g.setColor(Color.BLACK);

		g.setFont(f3);
		g.drawString("Nome:",10,10+h3);
		g.drawString("HC:",10,10+2*h3);
		g.drawString("Nascimento:",10,10+3*h3);
		g.drawString("Idade/Sexo:",10,10+4*h3);

		w = fm3.stringWidth("Nascimento:") + 10;
		g.setFont(f1);
		try {
		    g.drawString(nome,10+w,10+h3);
		    g.drawString(hc,10+w,10+2*h3);
		    g.drawString(birth,10+w,10+3*h3);
		    g.drawString(""+age+"/"+gender,10+w,10+4*h3);
		} catch(Exception e) {
		    return;
		}
		y = 10+4*h3+20+h2;
		g.setFont(f2);
		g.drawString("Scanners (" + vs.size() + ")",10,y);
		g.setFont(f1);
		y+=10;
		for(i=0;i<vs.size();i++) {
		    y += h3;
		    g.setColor(scolor[i%8]);
		    g.fillOval(30,y-11,11,11);
		    g.setColor(Color.BLACK);
		    g.drawOval(30,y-11,11,11);
		    g.drawString(vs.elementAt(i),30+15,y);
		}
		y += 10+h2;
		g.setFont(f2);
		g.drawString("Comentários (" + vc.size() + ")",10,y);
		g.setFont(f1);
		y+=10;
		for(i=0;i<vc.size();i++) {
		    y += 2*h3;
		    Comment c = vc.elementAt(i);
		    g.setColor(Color.BLACK);
		    g.drawString(c.text,30,y-h3);
		    g.setColor(Color.BLUE);
		    g.drawString(c.date + " - " + c.author + (c.pub != 0 ? " [público]":" [privado]"),30,y);
		}
		y += 10+h2;
		g.setColor(Color.BLACK);
		g.setFont(f2);
		g.drawString("Estudos (" + vstu.size() + ")",10,y);
		g.setFont(f1);
		y+=10+h3+10;

		int[] cw, rw;
		cw = new int[8];
		rw = new int[8];
		for(i=0;i<8;i++) cw[i] = 10+fm3.stringWidth(ct[i]);
		for(i=0;i<vstu.size();i++) {
		    Study s = vstu.elementAt(i);
		    rw[0] = fm4.stringWidth(s.date);
		    rw[1] = fm4.stringWidth(s.exam);
		    rw[2] = fm4.stringWidth(s.series);
		    rw[3] = Math.max(Math.max(fm4.stringWidth(s.ori + " " +s.dim),
					      fm4.stringWidth(s.pdim)),
				     fm4.stringWidth("thickness "+s.thick));
		    rw[4] = 12;
		    rw[5] = Math.max(fm4.stringWidth(s.disk), fm4.stringWidth(""+s.files+" arquivo(s)"));
		    rw[6] = 16;
		    rw[7] = 32;

		    for(j=0;j<8;j++)
			cw[j] = Math.max(cw[j],rw[j]+4);
		}
		for(j=0;j<8;j++)
		    cw[j]+=5;

		Color[] tbg = new Color[2];
		tbg[0] = new Color(0xdddddd);
		tbg[1] = new Color(0xbbbbbb);
		x=30;
		int scw=0;
		Color blue = new Color(0x000088);
		g.setFont(f3);
		for(i=0;i<8;i++) {
		    g.setColor(blue);
		    g.fillRect(x,y-h3-10,cw[i],h3+10);
		    g.setColor(Color.WHITE);
		    g.drawRect(x,y-h3-10,cw[i],h3+11);
		    g.drawString(ct[i],x+5,y-5);
		    x += cw[i];
		    scw += cw[i];
		}
		y += 3*h4;
		g.setFont(f4);
		for(j=0;j<vstu.size();j++) {
		    Study s = vstu.elementAt(j);
		    x=30;
		    g.setColor(tbg[j%2]);
		    g.fillRect(x,y-3*h4,scw,3*h4);
		    g.setColor(Color.BLACK);
		    y-=2;
		    g.drawString(s.date,x+5,y-h4); x+=cw[0];
		    g.drawString(s.exam,x+5,y-h4); x+=cw[1];
		    g.drawString(s.series,x+5,y-h4); x+=cw[2];

		    g.drawString(s.ori + " "+s.dim,x+5,y-2*h4); 
		    g.drawString(s.pdim,x+5,y-h4); 
		    g.drawString("thickness "+s.thick,x+5,y); 
		    x+=cw[3];

		    g.setColor(scolor[i%8]);
		    g.fillOval(x+cw[4]/2-5,y-h3-11,11,11);
		    g.setColor(Color.BLACK);
		    g.drawOval(x+cw[4]/2-5,y-h3-11,11,11);
		    x+=cw[4];
		    
		    g.drawString(s.disk,x+5,y-2*h4+h4/2);
		    g.drawString(""+s.files+" arquivo(s)",x+5,y-1*h4+h4/2);
		    x+=cw[5];
		    y += 2;
		    g.setColor(Color.WHITE); x=30;
		    for(i=0;i<8;i++) {
			g.drawRect(x,y-3*h4,cw[i],3*h4+1);
			x += cw[i];
		    }
		    y += 3*h4;
		} // for j
	    } // paint
	} // PatDetailRenderer

	
    } // PatDetailPane


} // QueryWindow

class Comment {
    public int pub;
    public String text,author,date;

    public Comment() {
	pub=0;
	text="";
	author="";
	date="";
    }

    public boolean empty() {
	return(text.length()==0);
    }
}

class Study {

    public String date,exam,series,ori,dim,pdim,thick,disk,scanner;
    public int id, files, quality;

    public Study() {
	id = -1;
	files=quality=0;
	date="";
	exam="";
	series="";
	ori="";
	dim="";
	pdim="";
	thick="";
	disk="";
	scanner="";
    }

    public boolean empty() { return(id<0); }

}