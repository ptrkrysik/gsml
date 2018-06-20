//////////////////////////////////////////////////////////////////////////////
//
//  Class  : TextComponentPopupMenu
//  Desc   : Specialized popup menu for subclasses of JTextComponent
//  Author : George Zhao
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

/**
 * Custom popup menu for subclasses of JTextComponent. Includes cut,copy,paste,
 * paste recent, and delete.
 *
 * @see VDDSTextField
 * @version 	1.0
 * @author      Semir Patel
 */

public class TextComponentPopupMenu extends JPopupMenu implements ActionListener
{
    /**
     * Maximum number of elements in Paste Recent submenu
     */
    static final int RECENT_LIST_SIZE = 4;


    /**
     * Popup menu items
     */
    static JMenuItem  cutMenuItem;
    static JMenuItem  copyMenuItem;
    static JMenuItem  pasteMenuItem;
    static JMenu      pasteRecentMenu;
    static JMenuItem  deleteMenuItem;
    static JMenuItem  descriptionMenuItem;

    static boolean iconsLoaded = false;
    
    /**
     * JTextComponent that invokes this popup menu
     */
    static JTextComponent invoker;


    /**
     * Starting and ending selection indices for the text component
     */
    static int start;
    static int end;

	/**
	 *	shared instance of popup menu 
	 */
	protected static TextComponentPopupMenu popupMenu = new TextComponentPopupMenu();


    private TextComponentPopupMenu()
    {
        cutMenuItem = Util.makeMenuItem("Cut",'t');
        cutMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        cutMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
        cutMenuItem.addActionListener(this);
        add(cutMenuItem);

        copyMenuItem = Util.makeMenuItem("Copy",'C');
        copyMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        copyMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
        copyMenuItem.addActionListener(this);
        add(copyMenuItem);

        pasteMenuItem = Util.makeMenuItem("Paste",'P');
        pasteMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        pasteMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
        pasteMenuItem.addActionListener(this);
        add(pasteMenuItem);

        pasteRecentMenu = new JMenu("Paste Recent");
        pasteRecentMenu.setHorizontalTextPosition(SwingConstants.RIGHT);
        pasteRecentMenu.setHorizontalAlignment(SwingConstants.LEFT);
        pasteRecentMenu.registerKeyboardAction(null, KeyStroke.getKeyStroke('R',java.awt.Event.ALT_MASK,false),
                                               JComponent.WHEN_IN_FOCUSED_WINDOW);
        add(pasteRecentMenu);

        deleteMenuItem = Util.makeMenuItem("Delete",'D');
        deleteMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        deleteMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
        deleteMenuItem.addActionListener(this);
        add(deleteMenuItem);
        
        addSeparator();
        descriptionMenuItem = Util.makeMenuItem("Description", 'P');
        descriptionMenuItem.setHorizontalTextPosition(SwingConstants.RIGHT);
        descriptionMenuItem.setHorizontalAlignment(SwingConstants.LEFT);
        descriptionMenuItem.addActionListener(this);
        add(descriptionMenuItem);
    }


	/**
	 *	accessor for shared instance of popup menu
	 */
	public static TextComponentPopupMenu getPopupMenu()
	{
		return popupMenu;
	}

    void loadIcons()
    {
        cutMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("cut.gif")));
        copyMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("copy.gif")));
        pasteMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("paste.gif")));
        pasteRecentMenu.setIcon(new ImageIcon(ImageLoader.loadImage("pasterecent.gif")));
        deleteMenuItem.setIcon(new ImageIcon(ImageLoader.loadImage("delete.gif")));
    }

    /**
     * Action listener for menu items
     */
    public void actionPerformed(ActionEvent e)
    {
        if (e.getSource() == cutMenuItem)
        {
            invoker.select(start,end);
            TextClipboard.setText(invoker.getSelectedText());
            invoker.replaceSelection("");
        }
        else if (e.getSource() == copyMenuItem)
        {
            invoker.select(start,end);
            TextClipboard.setText(invoker.getSelectedText());
        }
        else if (e.getSource() == pasteMenuItem)
        {
            invoker.replaceSelection(TextClipboard.getText());
        }
        else if (e.getSource() == deleteMenuItem)
        {
            invoker.select(start,end);
            invoker.replaceSelection("");
        }
        else if (e.getSource() == descriptionMenuItem)
        {
            /*
            JOptionPane.showMessageDialog(Util.browser,
            "Description of this field, range, detail meaning etc goes here.",
    		"Description", JOptionPane.INFORMATION_MESSAGE);
    		*/
    	}
        else if (e.getSource() instanceof JMenuItem)
        {
            invoker.select(start,end);
            invoker.replaceSelection(((JMenuItem)e.getSource()).getText());
        }
        
    }

    /**
     * Shows popup menu at specified coords with text component selection info
     * @param   c       Invoker
     * @param   x       Absolute x location
     * @param   y       Absolute y location
     * @param   start   Start index of selection
     * @param   end     End index of selection
     */
    public void show(Component c, int x, int y, int start, int end)
    {
        this.start = start;
        this.end   = end;
        show(c,x,y);
    }

    public void show(Component c, int x, int y)
    {
        if (iconsLoaded == false)
        {
            loadIcons();
            iconsLoaded = true;
        }
        
        if (c instanceof JTextComponent)
        {
            invoker = (JTextComponent)c;
            int len = invoker.getText().length();
            if (!invoker.isEditable())
            {
                cutMenuItem.setEnabled(false);
                pasteMenuItem.setEnabled(false);
                pasteRecentMenu.setEnabled(false);
                deleteMenuItem.setEnabled(false);
                
                if (len > 0)
                    copyMenuItem.setEnabled(true);
                else
                    copyMenuItem.setEnabled(false);
            }
            else
            {
                if ((len > 0) && (start != end))
                {
                    cutMenuItem.setEnabled(true);
                    copyMenuItem.setEnabled(true);
                    deleteMenuItem.setEnabled(true);
                }
                else
                {
                    cutMenuItem.setEnabled(false);
                    copyMenuItem.setEnabled(false);
                    deleteMenuItem.setEnabled(false);
                }

                pasteMenuItem.setEnabled(true);
                
                if (pasteRecentMenu.getMenuComponentCount() > 0)
                    pasteRecentMenu.setEnabled(true);
                else
                    pasteRecentMenu.setEnabled(false);
            }

			super.show(invoker,x,y);
        }
    }

    /**
     * Adds entry to Paste Recent menu. List works like FIFO queue.
     * @param   s   String to add to the paste recent menu
     */
    void addToMostRecentList(String s)
    {
        int cnt = pasteRecentMenu.getItemCount();

        for (int i=0; i<cnt; i++)
            if (pasteRecentMenu.getItem(i).getText().equals(s))
                return;

        if (cnt < RECENT_LIST_SIZE)
        {
            JMenuItem menuItem = Util.makeMenuItem();
            pasteRecentMenu.add(menuItem);
            menuItem.addActionListener(this);
            cnt = pasteRecentMenu.getMenuComponentCount();

            if (cnt == 1)
                pasteRecentMenu.getItem(0).setText(s);
            else
            {
                for (int i=cnt-1; i>=1; i--)
                    pasteRecentMenu.getItem(i).setText(pasteRecentMenu.getItem(i-1).getText());
                pasteRecentMenu.getItem(0).setText(s);
            }

        }
        else
        {
            for (int i=cnt-1; i>=1; i--)
                pasteRecentMenu.getItem(i).setText(pasteRecentMenu.getItem(i-1).getText());
            pasteRecentMenu.getItem(0).setText(s);
        }
    }

    public void setVisible(boolean b)
    {
        /**
         * Workaround for swing 0.7 bug.
         * Get rid of visible submenu if showing
         */
        if (b == false)
           pasteRecentMenu.setPopupMenuVisible(false);        
           
        super.setVisible(b);
    }

}
//$History:$