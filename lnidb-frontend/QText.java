import java.awt.*;
import javax.swing.*;

public class QText extends JTextField {

    public QText(int size) {
	super(size);
	setMaximumSize(new Dimension(Integer.MAX_VALUE, getPreferredSize().height) );
	setBorder(BorderFactory.createLineBorder(Color.BLACK));
    }

}

