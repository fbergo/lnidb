import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;

public class ConfigDialog extends JDialog implements ActionListener {

    ViewerConfig cfg;
    JTextField[] field;

    public ConfigDialog(JFrame parent, ViewerConfig _cfg) {
	super(parent,"Configurações",true);
	cfg = _cfg;

	setDefaultCloseOperation(DISPOSE_ON_CLOSE);

	JPanel v = new JPanel();
	v.setLayout(new BoxLayout(v, BoxLayout.Y_AXIS));
	v.setBorder(BorderFactory.createEmptyBorder(16, 16, 16, 16));
	add(v);

	field = new JTextField[3];

	JPanel h = new JPanel();
	h.setLayout(new BoxLayout(h, BoxLayout.X_AXIS));
	h.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
	h.add(new JLabel("URL do Servidor:"));
	h.add(Box.createRigidArea(new Dimension(8,1)));
	h.add(field[0] = new JTextField(30));
	v.add(h);

	h = new JPanel();
	h.setLayout(new BoxLayout(h, BoxLayout.X_AXIS));
	h.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
	h.add(new JLabel("Usuário:"));
	h.add(Box.createRigidArea(new Dimension(8,1)));
	h.add(field[1] = new JTextField(16));
	v.add(h);

	h = new JPanel();
	h.setLayout(new BoxLayout(h, BoxLayout.X_AXIS));
	h.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
	h.add(new JLabel("Senha:"));
	h.add(Box.createRigidArea(new Dimension(8,1)));
	h.add(field[2] = new JPasswordField(16));
	v.add(h);
	
	h = new JPanel();
	h.setLayout(new BoxLayout(h, BoxLayout.X_AXIS));
	h.setBorder(BorderFactory.createEmptyBorder(12, 4, 4, 4));
	h.add(Box.createHorizontalGlue());

	JButton ok,reset,cancel;
	ok     = new JButton("Ok");
	reset  = new JButton("Restaurar Padrões");
	cancel = new JButton("Cancelar");
	ok.setActionCommand("ok");
	reset.setActionCommand("reset");
	cancel.setActionCommand("cancel");
	ok.addActionListener(this);
	reset.addActionListener(this);
	cancel.addActionListener(this);

	h.add(ok);
	h.add(Box.createRigidArea(new Dimension(8,1)));
	h.add(cancel);
	h.add(Box.createRigidArea(new Dimension(8,1)));
	h.add(reset);
	v.add(h);

	field[0].setText(cfg.BaseURL);
	field[1].setText(cfg.User);
	field[2].setText(cfg.Password);

	pack();
	setLocationRelativeTo(parent);
	setVisible(true);
    }

    public void actionPerformed(ActionEvent ae) {

	String cmd = ae.getActionCommand();

	if (cmd.equals("reset")) {
	    ViewerConfig c = new ViewerConfig();
	    field[0].setText(c.BaseURL);
	    field[1].setText(c.User);
	    field[2].setText(c.Password);
	} else if (cmd.equals("cancel")) {
	    setVisible(false);
	} else if (cmd.equals("ok")) {
	    cfg.BaseURL  = field[0].getText();
	    cfg.User     = field[1].getText();
	    cfg.Password = field[2].getText();

	    if (!cfg.BaseURL.endsWith("/"))
		cfg.BaseURL = cfg.BaseURL + "/";

	    setVisible(false);
	}

    }

    
}