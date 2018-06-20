//////////////////////////////////////////////////////////////////////////////
//
//  Class  : TablePopupMenu
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

public class TablePopupMenu extends JPopupMenu implements ActionListener
{
	static JMenuItem copyCellMenuItem;
	static JMenuItem copyRowMenuItem;
	static JMenuItem copyColumnMenuItem;
	static JMenuItem copyTableMenuItem;

    static boolean iconsLoaded = false;
    static JTable invoker ; 

	static int x;
	static int y;

	protected static TablePopupMenu popupMenu = new TablePopupMenu();


	/**	
	 *	You cannot instantiate this class. Use getPopupMenu() to get
     *  a shared instance!
	 */
    protected TablePopupMenu()
    {
        copyCellMenuItem = Util.makeMenuItem("Copy Cell",'C');
        copyCellMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        copyCellMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
        copyCellMenuItem.addActionListener(this);
        add(copyCellMenuItem);

		copyRowMenuItem = Util.makeMenuItem("Copy Row", 'R');
        copyRowMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        copyRowMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		copyRowMenuItem.addActionListener(this);
		add(copyRowMenuItem);

		copyColumnMenuItem = Util.makeMenuItem("Copy Column", 'L');
        copyColumnMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        copyColumnMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		copyColumnMenuItem.addActionListener(this);
		add(copyColumnMenuItem);

		copyTableMenuItem = Util.makeMenuItem("Copy Table", 'T');
		copyTableMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
		copyTableMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
		copyTableMenuItem.addActionListener(this);
		add(copyTableMenuItem);
    }


	/**
	 *	returns shared instance of popupMenu
	 */
	public static TablePopupMenu getPopupMenu()
	{
		return popupMenu;
	}


	/**
	 *	update to use difference icons later!
	 */
    void loadIcons()
    {
        if(Util.applet == null)
        {
            copyCellMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("copy.gif")));
		    copyRowMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("copy.gif")));
		    copyColumnMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("copy.gif")));
		    copyTableMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("copy.gif")));
	    }
	}


	/**
	 *	update to use shared clipboard access later
	 */
    public void actionPerformed(ActionEvent e)
    {
		Object obj = e.getSource();

		Point p = new Point(x,y);
		int row = invoker.rowAtPoint(p);
		int col = invoker.columnAtPoint(p);

		String selection = null;

        if (obj == copyCellMenuItem)
        {
			selection = invoker.getValueAt(row,col).toString();
        }
		else if (obj == copyRowMenuItem)
		{
			for(int i=0; i < invoker.getColumnCount(); i++)
			{
				Object cell = invoker.getValueAt(row,i);
				if (cell != null)
					selection += cell.toString() + " ";
			}
		}
		else if (obj == copyColumnMenuItem)
		{
			for(int i=0; i < invoker.getRowCount(); i++)
			{
				Object cell = invoker.getValueAt(i,col);
				if (cell != null)
					selection += cell.toString() + " ";
			}
		}
		else if (obj == copyTableMenuItem)
		{
			for(int i=0; i < invoker.getRowCount(); i++)
			{
				for(int j=0; j < invoker.getColumnCount(); j++)
				{
					Object cell = invoker.getValueAt(i,j);
					if (cell != null)
						selection += cell.toString() + " ";
				}

				selection += "\n";
			}
		}


		TextClipboard.setText(selection);

    }


	/**
	 *	force JTable only later
	 */
    public void show(Component c, int x, int y)
    {
		invoker = (JTable)c;

        if (iconsLoaded == false)
        {
            loadIcons();
            iconsLoaded = true;
        }

		Point p = new Point(x,y);
		int row = invoker.rowAtPoint(p);
		int col = invoker.columnAtPoint(p);

		if ((row >= 0) && (col >= 0))
			enableMenu();
		else
		{
			disableMenu();
			return;
		}

		this.x = x;
		this.y = y;
		super.show(c,x,y);
    }


	void disableMenu()
	{
		copyCellMenuItem.setEnabled(false);
		copyRowMenuItem.setEnabled(false);
		copyColumnMenuItem.setEnabled(false);
		copyTableMenuItem.setEnabled(false);
	}


	void enableMenu()
	{
		copyCellMenuItem.setEnabled(true);
		copyRowMenuItem.setEnabled(true);
		copyColumnMenuItem.setEnabled(true);
		copyTableMenuItem.setEnabled(true);
	}
}

//$History:$