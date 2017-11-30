import java.awt.*;
import javax.swing.*;

public class QPassword extends JPasswordField {

    public QPassword(int size) {
	super(size);
	setMaximumSize(new Dimension(Integer.MAX_VALUE, getPreferredSize().height) );
	setBorder(BorderFactory.createLineBorder(Color.BLACK));
    }

}

