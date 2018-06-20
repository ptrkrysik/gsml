//////////////////////////////////////////////////////////////////////////////
//
//  Class  : WMTextField
//  Desc   : TextField widget for Web Management Browser
//  Author : George Zhao
//  History: April 22, 1999  Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/**
 * Subclass of JTextfield that add a right mouse button triggered
 * popupmenu for cut/copy/paste/delete functionality.
 *
 * @see TextComponentPopupMenu
 * @version 	1.0
 * @author      George Zhao
 */

public class WMTextField extends JTextField implements FocusListener
{
    static TextComponentPopupMenu popupMenu = TextComponentPopupMenu.getPopupMenu();

    /**
     * Previous value is tracked to make sure duplicates don't make it into
     * the paste recent menu.
     */
    String previousValue="";
    String magicMarkup = "";

    public WMTextField()
    {
        super();
        init();
    }

    public WMTextField(String text)
    {
        super(text);
        init();
    }

    public WMTextField(int columns)
    {
        super(columns);
        init();
    }

    public WMTextField(String text, int columns)
    {
        super(text,columns);
        init();
    }

    /**
     * Adds focusListener to textfield
     */
    void init()
    {
        addFocusListener(this);
    		//{{INIT_CONTROLS
		//}}
    }


    public void focusGained(FocusEvent e) {};

    /**
     *  Listens for focusLost event to trigger updates to
     *  the paste recent popup menu
     * @param   e      FocusEvent
     */
    public void focusLost(FocusEvent e)
    {
        String value = getTrimmedText();
        if ((value.length() > 0) && (!value.equals(previousValue)))
        {
            previousValue = value;
            popupMenu.addToMostRecentList(value);
        }
    }
    
    /**
     * Captures right mouse button mouse events to invoke popup menu.
     * @param   e       MouseEvent
     */
    public void processMouseEvent(MouseEvent e)
    {
        if (e.getModifiers() == InputEvent.BUTTON3_MASK)
        {
            if (e.isPopupTrigger())
            {
                popupMenu.show(e.getComponent(),
                               e.getX(),
                               e.getY(),
                               getSelectionStart(),
                               getSelectionEnd());
            }
        }
        else
            super.processMouseEvent(e);
    }

    /**
     * Returns text without  with leading and trailing spaces
     */
    public String getTrimmedText()
    {
        return getText().trim();
    }

    /**
     * Sets the text. Keeps track of previous value.
     */
    public void setText(String s)
    {
        super.setText(s);
        previousValue = s;
        repaint();
    }

    /**
     * Swing bug - force repaint on setEditable() to refresh color
     */
    public void setEditable(boolean b)
    {
        super.setEditable(b);
        if(b) {
            //setBackground(UIManager.getDefaults().getColor("TextField.background"));
			setBackground(Color.white);
        }
        else {
            //setBackground(UIManager.getDefaults().getColor("Label.background"));
			setBackground(Color.lightGray);
        }
		repaint();
    }
    
    public void setMagicMarkup(String markup)
    {
        magicMarkup = markup;
    }
    
    public String getMagicMarkup()
    {
        return magicMarkup;
    }
}

//$History:$