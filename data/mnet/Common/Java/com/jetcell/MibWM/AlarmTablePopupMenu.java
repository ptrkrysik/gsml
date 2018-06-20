//////////////////////////////////////////////////////////////////////////////
//
//  Class  : AlarmTablePopupMenu
//  Desc   : 
//  Author : George Zhao
//  Hist   : 06/17/1999 Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.table.*;

public class AlarmTablePopupMenu extends JPopupMenu implements ActionListener
{
	static JMenuItem openMenuItem;
	static JMenuItem removeMenuItem;
	static JMenuItem removeAllMenuItem;
	static JMenuItem removeReadMenuItem;
	static JMenuItem removeInfoMenuItem;
	static JMenuItem refreshMenuItem;
	static JMenuItem acknowledgeMenuItem; 

    static boolean iconsLoaded = false;
	protected static AlarmTablePopupMenu popupMenu = new AlarmTablePopupMenu();
    
    static AlarmTable invoker ; 

	static int x;
	static int y;


	/**	
	 *	You cannot instantiate this class. Use getPopupMenu() to get
     *  a shared instance!
	 */
    protected AlarmTablePopupMenu()
    {
        openMenuItem = Util.makeMenuItem("Open Event",'O');
        openMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        openMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
        openMenuItem.addActionListener(this);
        add(openMenuItem);
        addSeparator();
        
        
        /*
        removeMenuItem = Util.makeMenuItem("Remove Event",'R');
        removeMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        removeMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
        removeMenuItem.addActionListener(this);
        add(removeMenuItem);
        addSeparator();

		removeAllMenuItem = Util.makeMenuItem("Remove All Events", 'A');
        removeAllMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        removeAllMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		removeAllMenuItem.addActionListener(this);
		add(removeAllMenuItem);

		removeReadMenuItem = Util.makeMenuItem("Remove Read Events", 'E');
        removeReadMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        removeReadMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		removeReadMenuItem.addActionListener(this);
		add(removeReadMenuItem);

		removeInfoMenuItem = Util.makeMenuItem("Remove Clear Events", 'C');
		removeInfoMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
		removeInfoMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		removeInfoMenuItem.addActionListener(this);
		add(removeInfoMenuItem);*/
		
		// Shrinivas 20th Apr 01
		/*addSeparator();
		refreshMenuItem = Util.makeMenuItem("Refresh", 'R');
		refreshMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
		refreshMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		refreshMenuItem.addActionListener(this);
		add(refreshMenuItem);*/
		// Shrinivas 20th Apr 01

		addSeparator();
		addSeparator();
		acknowledgeMenuItem = Util.makeMenuItem("Acknowledge", 'A');
		acknowledgeMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
		acknowledgeMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		acknowledgeMenuItem.addActionListener(this);
		add(acknowledgeMenuItem);
    }


	/**
	 *	returns shared instance of popupMenu
	 */
	public static AlarmTablePopupMenu getPopupMenu()
	{
		return popupMenu;
	}


	/**
	 *	update to use difference icons later!
	 */
    void loadIcons()
    {
        openMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("opengray.gif")));
		removeAllMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("delete.gif")));
		removeReadMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("delete.gif")));
		removeInfoMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("delete.gif")));
		refreshMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("reset.gif")));
	}


	/**
	 *	update to use shared clipboard access later
	 */
    public void actionPerformed(ActionEvent e)
    {
		Object obj = e.getSource();
		String selection = null;

        if (obj == openMenuItem)
        {
			invoker.OpenEvent();
        }
        else if (obj == removeMenuItem)
        {
            invoker.RemoveEvent();
        }
		else if (obj == removeAllMenuItem)
		{
			invoker.RemoveAllEvents();
		}
		else if (obj == removeReadMenuItem)
		{
			invoker.RemoveReadEvents();
		}
		else if (obj == removeInfoMenuItem)
		{
			invoker.RemoveInfoEvents();
		}
		else if (obj == refreshMenuItem){
		    invoker.Refresh();
		}
		else if (obj == acknowledgeMenuItem){ 
			invoker.Acknowledge();
		}

    }


	/**
	 *	force JTable only later
	 */
    public void show(Component c, int x, int y)
    {
		invoker = (AlarmTable)c;
		/*
        if (iconsLoaded == false)
        {
            loadIcons();
            iconsLoaded = true;
        }
        */
        

		Point p = new Point(x,y);
		int row = invoker.rowAtPoint(p);
		int col = invoker.columnAtPoint(p);
        
        /*
		if ((row >= 0) && (col >= 0))
			enableMenu();
		else
		{
			disableMenu();
			return;
		}
		*/

     DefaultTableModel model = (DefaultTableModel)invoker.getModel();
     Boolean acknowledge = (Boolean) model.getValueAt(row,AlarmTableModel.ACK);
     if(acknowledge.booleanValue()){
     	acknowledgeMenuItem.setEnabled(false);
     }
     else{ 
     	acknowledgeMenuItem.setEnabled(true);
     }
		this.x = x;
		this.y = y;
		super.show(c,x,y);
    }


	void disableMenu()
	{
		openMenuItem.setEnabled(false);
		removeAllMenuItem.setEnabled(false);
		removeReadMenuItem.setEnabled(false);
		removeInfoMenuItem.setEnabled(false);
	}


	void enableMenu()
	{
		openMenuItem.setEnabled(true);
		removeAllMenuItem.setEnabled(true);
		removeReadMenuItem.setEnabled(true);
		removeInfoMenuItem.setEnabled(true);
	}
}

//$History:$
