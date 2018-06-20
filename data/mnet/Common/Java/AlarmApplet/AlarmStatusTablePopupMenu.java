/*
 * AlarmStatusTablePopupMenu.java
 * Kevin Lim
 * 03/20/00
 */



import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

public class AlarmStatusTablePopupMenu extends JPopupMenu implements ActionListener{
	static JMenuItem openMenuItem;
	static JMenuItem removeMenuItem;
	static JMenuItem refreshMenuItem;
    static boolean iconsLoaded = false;
	protected static AlarmStatusTablePopupMenu popupMenu = new AlarmStatusTablePopupMenu();
    static AlarmStatusTable invoker ; 
	static int x;
	static int y;


	/**	
	 *	You cannot instantiate this class. Use getPopupMenu() to get
     *  a shared instance!
	 */
  protected AlarmStatusTablePopupMenu(){
	
        openMenuItem = new JMenuItem(new String("Open Alarm"));
        openMenuItem.setMnemonic('O');
        openMenuItem.registerKeyboardAction(null, KeyStroke.getKeyStroke('O', Event.ALT_MASK, false),
                                            JComponent.WHEN_IN_FOCUSED_WINDOW);
        openMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        openMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
        openMenuItem.addActionListener(this);
        add(openMenuItem);

        removeMenuItem = new JMenuItem(new String("Remove"));
        removeMenuItem.setMnemonic('m');
        removeMenuItem.registerKeyboardAction(null, KeyStroke.getKeyStroke('m', Event.ALT_MASK, false),
                                            JComponent.WHEN_IN_FOCUSED_WINDOW);
		removeMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
		removeMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		removeMenuItem.addActionListener(this);
		add(removeMenuItem);

		// Shrinivas 20th Apr 01
        /*refreshMenuItem = new JMenuItem(new String("Refresh"));
        refreshMenuItem.setMnemonic('R');
        refreshMenuItem.registerKeyboardAction(null, KeyStroke.getKeyStroke('R', Event.ALT_MASK, false),
                                            JComponent.WHEN_IN_FOCUSED_WINDOW);
		refreshMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
		refreshMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		refreshMenuItem.addActionListener(this);
		add(refreshMenuItem);*/
		// Shrinivas end
    }


	/**
	 *	returns shared instance of popupMenu
	 */
	public static AlarmStatusTablePopupMenu getPopupMenu(){
		return popupMenu;
	}


	/**
	 *	update to use difference icons later!
	 */
	void loadIcons(){
	}


	/**
	 *	update to use shared clipboard access later
	 */
	public void actionPerformed(ActionEvent e){
		Object obj = e.getSource();
		String selection = null;

        if (obj == openMenuItem) {
			invoker.OpenEvent();
        }
		else if (obj == removeMenuItem) {
		    invoker.Remove();
		}
		else if (obj == refreshMenuItem) {
		    invoker.Refresh();
		}
    }


	/**
	 *	force JTable only later
	 */
  public void show(Component c, int x, int y){
		invoker = (AlarmStatusTable)c;

		Point p = new Point(x,y);
		int row = invoker.rowAtPoint(p);
		int col = invoker.columnAtPoint(p);
        
		this.x = x;
		this.y = y;
		super.show(c,x,y);
    }


	void disableMenu()	{
		openMenuItem.setEnabled(false);
		removeMenuItem.setEnabled(false);
		refreshMenuItem.setEnabled(false);
	}


	void enableMenu(){
		openMenuItem.setEnabled(true);
		removeMenuItem.setEnabled(true);
		refreshMenuItem.setEnabled(true);
	}
}

