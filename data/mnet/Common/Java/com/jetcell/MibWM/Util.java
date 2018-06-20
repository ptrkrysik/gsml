//////////////////////////////////////////////////////////////////////////////
//
//  Class  : Util
//  Desc   : Utility class for MIB Web Management
//  Author : George Zhao    
//  History: April 22, 1999
//
//////////////////////////////////////////////////////////////////////////////

package  com.jetcell.MibWM;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.text.*;
import javax.swing.event.*;

import java.applet.*;
import java.util.*;


public abstract class Util
{
	public static TitledBorder titleBorder;
	static JPanel p;
    public static Applet applet = null;
    
    //undo
    public static boolean debug = false;
    
    public static final int    BORDER_INSET = 5;

    public static final Border EMPTY_BORDER_STYLE = new EmptyBorder(BORDER_INSET, BORDER_INSET, BORDER_INSET, BORDER_INSET);
    public static final Border BORDER_STYLE = new CompoundBorder(new EmptyBorder(BORDER_INSET, BORDER_INSET, BORDER_INSET, BORDER_INSET),
                                  new CompoundBorder(new EtchedBorder(Color.gray, Color.white),
                                  new EmptyBorder(BORDER_INSET, BORDER_INSET, BORDER_INSET, BORDER_INSET)));

    static final int BUTTON_TOP         = 1;
    static final int BUTTON_BOTTOM      = 1;
    static final int BUTTON_LEFT        = 1;
    static final int BUTTON_RIGHT       = 1;
    /**
     * The default number of characters for text field.
     */
    public  static int TEXTFIELD_NUM_CHARS = 12;

    /**
     * The default number of characters for text field.
     */
    public  static int DATEFIELD_NUM_CHARS = 10;

    /**
     * The default number of characters for text field.
     */
    public  static int TIMEFIELD_NUM_CHARS = 12;

    /**
     * Creates a popup menu with items in an array of string.
     * No action listener is to be added for this method.
     * @param      items  an array of String for the menu items
     */
    public static PopupMenu makePopupMenu(String[] items)
    {
        PopupMenu popup = new PopupMenu();
        for (int i=0; i < items.length; i++) {
            popup.add(new MenuItem(items[i]));
        }
        MenuItem mItem = new MenuItem("Copy");
        popup.add(mItem);
        return popup;
    }

    /**
     * Creates a menu with a name of your choice, items in an array of strings, acttionCommands being
     * items with spaces trimmed, and an action listener for all items.
     * @param   menuName    the name of the menu
     * @param   items       an array of String for the items
     * @param   listener    action listener
     */
    public static JMenu makeMenu(String menuName, char hotkey,  String[] items, ActionListener listener)
    {
        JMenu menu = makeMenu(menuName, hotkey);
        JMenu submenu = null;
        JMenuItem menuItem = null;
        String itemString;
        String queryName;
        String submenuItems;
        int comma;

        for (int i=0; i < items.length; i++) {
            //parse the items string
            int index;
            if ( (index = items[i].indexOf(":")) >= 0 ) {
                //has submenu
                queryName = items[i].substring(0,index).trim();
                submenu = makeMenu( queryName );
                submenuItems = items[i].substring(index+1).trim();
                while ( submenuItems.length() > 0 ) {
                    comma = submenuItems.indexOf(",");
                    if ( comma >= 0 ) {
                        itemString = submenuItems.substring(0, comma).trim();
                        submenuItems = submenuItems.substring(comma+1).trim();
                    } else {
                        itemString = submenuItems;
                        submenuItems = "";
                    }
                    menuItem = makeMenuItem(itemString);
                    menuItem.setActionCommand( trimSpaces(queryName + ":" + itemString) );
                    if ( listener != null ) {
                        menuItem.addActionListener( listener );
                    }
                    submenu.add(menuItem);
                }
                menu.add( submenu );
            } else { //no submenu
                menuItem = makeMenuItem(items[i]);
                menuItem.setActionCommand( trimSpaces(items[i]) );
                if ( listener != null ) {
                    menuItem.addActionListener( listener );
                }
                menu.add( menuItem );
            }
        }
        return menu;
    }

    /**
     * Creates a menu with a name of your choice, items and action commands in an array of strings,
     * and an action listener for all items.
     * @param   menuName    the name of the menu
     * @param   items       an array of String for the items
     * @param   commands    action commands in an array of strings
     * @param   listener    action listener
     */
    public static JMenu makeMenu(String menuName, char hotkey, String[] items, String[] commands, ActionListener listener)
    {
        JMenu menu = makeMenu(menuName, hotkey);
        int index;
        for (int i=0; i < items.length; i++) {
            if ( (index = items[i].indexOf("_") ) >= 0 ) {
                JMenu subMenu = makeMenu(items[i].substring(0, index));
                JMenuItem item = makeMenuItem(items[i].substring(index+1) );
                if ( listener != null ) {
                    item.addActionListener( listener );
                }
	            subMenu.add(item);
	            menu.add(subMenu);
   		    } else {
                JMenuItem menuItem = makeMenuItem(items[i]);
                menuItem.setActionCommand( commands[i] );
                if ( listener != null )
                    menuItem.addActionListener( listener );
                menu.add( menuItem );
            }
        }
        return menu;
    }

    public static JMenu makeMenu(String name)
    {
        JMenu menu = new JMenu(name);
        return menu;
    }

    public static JMenu makeMenu (String name, char mnemonic)
    {
        JMenu menu = new JMenu(name);
        menu.setMnemonic(mnemonic);

        return menu;
    }



    /**
     * Creates a Swing label with preferred size and Style Guide conforming attributes
     * @param      text    the text of Label
     */
    public static JLabel makeLabel(String text)
    {
        JLabel label = new JLabel(text,JLabel.RIGHT);
   		//label.setSize(label.getPreferredSize());
   		label.setVerticalAlignment(SwingConstants.CENTER);
        return label;
    }

    /**
     * Creates a label with alignment
     * @param      text    the text of Label
     * @alignment  alignment horizontal alignment - JLabel.{LEFT,RIGHT,CENTER}
     */
    public static JLabel makeLabel(String text, int alignment)
    {
        JLabel label = makeLabel(text);
        label.setHorizontalAlignment(alignment);
        return label;
    }

    public static JLabel makeLabel(String text, char hotkey)
    {
        JLabel label = makeLabel(text);
        label.setDisplayedMnemonic(hotkey);
        return label;
    }

    public static JLabel makeLabel(String text, char hotkey, int alignment)
    {
        JLabel label = makeLabel(text, alignment);
        label.setDisplayedMnemonic(hotkey);
        return label;
    }

	public static JLabel makeLabel(String text, char hotkey, Component target)
	{
		JLabel label = makeLabel(text, hotkey);
		label.setLabelFor(target);
		return label;
	}

	public static JLabel makeLabel(String text, char hotkey, Component target, int alignment)
	{
		JLabel label = makeLabel(text, hotkey, target);
		label.setHorizontalAlignment(alignment);
		return label;
	}

    public static JTextField makeTextField()
    {
        return makeTextField(TEXTFIELD_NUM_CHARS);
    }

    /**
     * Creates an non-editable Swing text field for holding numChars characters
     * @param   numChars holding numChars characters in width
     */
    public static JTextField makeTextField(int numChars)
    {
        return makeTextField("", numChars, false);
    }

    /**
     * Creates a non-editable Swing text field with content text and default width
     * @param   text     the text of field
     */
    public static JTextField makeTextField(String text)
    {
        return makeTextField(text, TEXTFIELD_NUM_CHARS, false);
    }

    /**
     * Creates a Swing text field with intial content text, numChars in width, editable as set
     * @param   text     the text of field
     * @param   numChars holding numChars characters for width
     * @param   editable to set editable or not
     */
    public static JTextField makeTextField(String text, int numChars, boolean editable)
    {
        WMTextField textField = new WMTextField( text, numChars );
        textField.setEditable(editable);
        return textField;
    }

    /**
     * Creates a Swing text field with intial content text, numChars in width, editable as set
     * @param   dateDocument    the date formating document
     * @param   text            the text of field
     * @param   numChars        holding numChars characters for width
     * @param   editable        to set editable or not
     */
	/*
    public static JTextField makeTextField( Document dateDocument, String text, int numChars, boolean editable)
    {
        JTextField textField = new WMTextField( dateDocument, text, numChars);
        textField.setEditable(editable);
        return textField;
    }
	*/

    /**
     * Creates a Swing non-editable text field with intial content text, numChars in width
     * @param   text     the text of field
     * @param   numChars holding numChars characters for width
     */
    public static JTextField makeTextField(String text, int numChars)
    {
        return makeTextField(text, numChars, false);
    }

    /**
     * Creates a Swing text field with editability settable
     * @param   editable to set editable or not
     */
    public static JTextField makeTextField(boolean editable)
    {
        return makeTextField("", TEXTFIELD_NUM_CHARS, editable);
    }
    
    public static JTextField makeTextField(boolean editable, String markup)
    {
        WMTextField field= (WMTextField) makeTextField("", TEXTFIELD_NUM_CHARS, editable);
        field.setMagicMarkup(markup);
        return field;
    }

    /**
     * Creates a Swing text field with editability settable and with numChars in width
     * @param   numChars holding numChars characters for width
     * @param   editable to set editable or not
     */
    public static JTextField makeTextField(int numChars, boolean editable)
    {
        return makeTextField("", numChars, editable);
    }


    /**
     * Creates a Swing button
     * @param   text  the text on the button
     */
    public static JButton makeButton(String text)
    {
        JButton button = new JButton(text);
        button.setMargin(new Insets(BUTTON_TOP,BUTTON_BOTTOM,BUTTON_LEFT,BUTTON_RIGHT));
        return button;
    }

    /**
     * Creates a Swing button with shortcut
     * @param   text   the text on the button
     * @param   hotkey mnemonic character (underlined)
     */
    public static JButton makeButton(String text, char hotkey)
    {
        JButton button = makeButton(text);
        button.registerKeyboardAction(null, KeyStroke.getKeyStroke(hotkey,java.awt.Event.ALT_MASK,false),
                                      JComponent.WHEN_IN_FOCUSED_WINDOW);
        button.setMnemonic(hotkey);
        return button;
    }

    /**
     * Creates a Swing button with shortcut and tooltip
     * @param   text   the text on the button
     * @param   hotkey mnemonic character (underlined)
     * @param   tooltip text for the button's tooltip
     */
    public static JButton makeButton(String text, char hotkey, String tooltip)
    {
        JButton button = makeButton(text);
        button.setMnemonic(hotkey);
        button.setToolTipText(tooltip);
        return button;
    }


    /**
     * Creates a Swing button with shortcut, tooltip, and listener
     * @param   text   the text on the button
     * @param   hotkey mnemonic character (underlined)
     * @param   tooltip text for the button's tooltip
	 * @param   listener actionlistener
     */
	public static JButton makeButton(String text, char hotkey, String tooltip, ActionListener listener)
	{
		JButton button = makeButton(text, hotkey, tooltip);
		button.addActionListener(listener);
		return button;
	}

    /**
     * Create a JButton with Icon
     * @param text  the text on the button
     * @param icon  the image on the button
     */
    public static JButton makeButton(String text, ImageIcon icon)
    {
        JButton button = new JButton(text, icon);
        button.setMargin(new Insets(BUTTON_TOP,BUTTON_BOTTOM,BUTTON_LEFT,BUTTON_RIGHT));
        return button;
    }

    /**
     * create JButton with icon and hot key
     */
    public static JButton makeButton(String text, ImageIcon icon, char hotkey)
    {
        JButton button = makeButton(text, icon);
        button.setMnemonic(hotkey);
        return button;
    }

    /**
     * create JButton with icon, hotkey and tooltip
     * @param text
     * @param icon
     * @param hotkey
     * @param tooltip
     */
    public static JButton makeButton(String text, ImageIcon icon, char hotkey, String tooltip)
    {
        JButton button = makeButton(text, icon, hotkey);
        button.setToolTipText(tooltip);
        return button;
    }

    /**
     * create JButton with icon, hotkey and tooltip, and listener
     * @param text
     * @param icon
     * @param hotkey
     * @param tooltip
	 * @param listener 
     */
    public static JButton makeButton(String text, ImageIcon icon, char hotkey, String tooltip, ActionListener listener)
    {
        JButton button = makeButton(text, icon, hotkey, tooltip);
        button.addActionListener(listener);
        return button;
    }


    /**
     * create JButton with icon and tooltip
     */
    public static JButton makeButton(ImageIcon icon, String tooltip)
    {
        JButton button = new JButton(icon);
        button.setMargin(new Insets(BUTTON_TOP,BUTTON_BOTTOM,BUTTON_LEFT,BUTTON_RIGHT));
        button.setToolTipText(tooltip);
        return button;
    }

    /**
     * Creates a Swing panel
     */
    public static JPanel makePanel()
    {
        JPanel p = new JPanel();
        return p;
    }

	/**
	 * Creates a panel  with layout
	 */
	public static JPanel makePanel(LayoutManager layout)
	{
		JPanel p = makePanel();
		p.setLayout(layout);
		return p;
	}

    /**
     * Creates a Swing bordered panel
     * @param   title  the title of the border
     */
    public static JPanel makeBorderPanel(String title)
    {
        p = makePanel();
        p.setBorder(BORDER_STYLE);
		titleBorder = new TitledBorder(p.getBorder(), title);
		p.setBorder(titleBorder);
        return p;
    }
	public static void setBorderTitle(String title){
		System.out.println("setting title" + title);
		titleBorder.setTitle(title);
		p.validate();
	}
	
	public static String getBorderTitle(){
		return titleBorder.getTitle();
    }

    /**
     * Creates a Swing bordered panel with layout
     * @param   title  the title of the border
	 * @param   layout layout
     */
    public static JPanel makeBorderPanel(String title, LayoutManager layout)
    {
        JPanel p = makeBorderPanel(title);
		p.setLayout(layout);
        return p;
    }


    /**
     * Creates a Swing bordered panel with no title
     */
    public static JPanel makeBorderPanel()
    {
        return makeBorderPanel("");
    }


    /**
     * Creates a Swing bordered panel with NO border
     * Why this ?  to add the space of border to line up with other bordered panels
     */
    public static JPanel makeEmptyBorderPanel()
    {
        JPanel p = makePanel();
        p.setBorder(EMPTY_BORDER_STYLE);
        return p;
    }

    /**
     * Creates a Swing bordered panel with NO border
     * Is this different from makeEmptyBorderPanel ?
     * To be answered....
     */
    public static JPanel makeHugeEmptyBorderPanel()
    {
        JPanel p = makePanel();
        p.setBorder(new CompoundBorder(EMPTY_BORDER_STYLE,
                                       EMPTY_BORDER_STYLE));
        return p;
    }

    /**
     * Remove all space characters in a given string.
     * @param   s    the string to be trimmed
     */
    public static String trimSpaces(String s)
    {
        String r="";
        if ( s == null )
            return null;

        for (int i=0; i < s.length(); i++)
            if ( s.charAt(i) != ' ' )
                r = r + s.charAt(i);

        return r;
    }


    /**
     * Makes a JRadioButton showing text with key as the accelerator
     * @param text the text of the button
     * @param key  the accelerator key
     */
    public static JRadioButton makeRadioButton(String text, char key)
    {
        JRadioButton rb = new JRadioButton(text);
		rb.setMnemonic(key);
        return rb;
    }


    /**
     * Makes a JRadioButton showing text without key accelerator
     * @param text the text of the button
     */
    public static JRadioButton makeRadioButton(String text)
    {
        JRadioButton rb = new JRadioButton(text);
        return rb;
    }
    
    public static JTextArea makeTextArea(int r, int c, boolean editable)
    {
        JTextArea area = new JTextArea(r,c);
        area.setEditable(editable);
        
        return area;
    }

    
    public static JTextArea makeTextArea()
    {
        return makeTextArea("");
    }

    /**
     * Makes a text area with a populated string
     * @param text populate area with string
     */
    public static JTextArea makeTextArea(String s)
    {
        return new JTextArea(s);
    }
    
    public static JComboBox makeComboBox(boolean editable)
    {
        JComboBox cb = new JComboBox();
        cb.setEditable( editable );
        return cb;
    }


    /**
     * Makes a JComboBox with an array of String for items with default size
     * @param items an array of strings
     * @param selectedItemIndex the index of the item to be selected by default
     * @param editable editable or not
     */
    public static JComboBox makeComboBox(String[] items, int selectedItemIndex,
                                         boolean editable)
    {
        return makeComboBox(items, selectedItemIndex, editable, null);
    }

    /**
     * Makes a JComboBox with an array of String for items
     * @param items an array of strings
     * @param selectedItemIndex the index of the item to be selected by default
     * @param editable editable or not
     * @param dimension the dimension of the box
     */
    public static JComboBox makeComboBox(String[] items, int selectedItemIndex,
                                         boolean editable, Dimension dimension)
    {
        JComboBox cb = new JComboBox();
        for (int i=0; i < items.length; i++)
            cb.addItem(items[i]);

        cb.setEditable( editable );
        cb.setSelectedItem( items[selectedItemIndex] );
        if ( dimension != null )
            cb.setPreferredSize( dimension );

        return cb;
    }

    /**
     * Makes an empty menu item.
     */
    public static JMenuItem makeMenuItem()
    {
        return makeMenuItem("");
    }

    /**
     * Makes a menu item with an icon.
     * @param icon  image icon
     */
    public static JMenuItem makeMenuItem(Icon icon)
    {
        JMenuItem m = makeMenuItem();
        m.setIcon(icon);
        return m;
    }

    /**
     * Makes a menu item with a label and icon.
     * @param text  menu label
     * @param icon  image icon
     */
    public static JMenuItem makeMenuItem(String text,Icon icon)
    {
        JMenuItem m = makeMenuItem(text);
        m.setIcon(icon);
        return m;
    }

    /**
     * Makes a menu item with a label and shortcut key.
     * @param text  menu label
     * @param mnemonic underlined character in label
     */
    public static JMenuItem makeMenuItem(String text, char mnemonic)
    {
        JMenuItem m = makeMenuItem(text);
        m.setMnemonic(mnemonic);
        m.registerKeyboardAction(null, KeyStroke.getKeyStroke(mnemonic,java.awt.Event.ALT_MASK,false),
                                 JComponent.WHEN_IN_FOCUSED_WINDOW);
        return m;
    }

    /**
     * Makes a menu item with a label
     * @param text menu label
     */
    public static JMenuItem makeMenuItem(String text)
    {
        return new JMenuItem(text);
    }

    public static JMenuItem makeMenuItem(String text, Icon icon, char mnemonic)
    {
        JMenuItem m = new JMenuItem(icon);
        m.setText(text);
        m.setMnemonic(mnemonic);

        return m;
    }


	/**
	 * Makes a text area that looks like a multiline 
	 * wrapping label that word wraps
	 * @param initial value of textarea
	 */
	public static JTextArea makeWrappingLabel(String s)
	{
		JTextArea ta = new JTextArea(s);
		ta.setLineWrap(true);
		//ta.setWrapStyleWord(true);
		ta.setOpaque(false);
		ta.setEditable(false);
		ta.setBorder(new EmptyBorder(0,0,0,0));	
		ta.setSelectionColor(UIManager.getDefaults().getColor("Label.background")); 
		ta.setFont(UIManager.getDefaults().getFont("Label.font"));
		return ta;
	}

    static public void constrain(Component component, Container container, GridBagConstraints gbc,
                  int grid_x, int grid_y, int grid_width, int grid_height,
                  int fill, int anchor, double weight_x, double weight_y,
                  int top, int left, int bottom, int right)
    {
        gbc.gridx = grid_x;
        gbc.gridy = grid_y;
        gbc.gridwidth = grid_width;
        gbc.gridheight = grid_height;
        gbc.fill = fill;
        gbc.anchor = anchor;
        gbc.weightx = weight_x;
        gbc.weighty = weight_y;
        if (top+bottom+left+right > 0)
            gbc.insets = new Insets(top, left, bottom, right);

        ((GridBagLayout)container.getLayout()).setConstraints(component, gbc);
        container.add(component);
    }
    
    static public void showDialog(JFrame frame, String message, String title, int type)
	{
        JOptionPane.showMessageDialog(frame, message,  title, type);
	}
    
    static public void showDialog(String message, String title)
	{
	    showDialog(message, title, JOptionPane.INFORMATION_MESSAGE);
	}
    
    static public void showDialog(String message, String title, int type)
	{
	    showDialog(new JFrame(), message, title, type);
	}
	
	static public void showDialog(String message)
	{
	    showDialog(message, "");
	}
	
	static public void showDialog(Exception e)
	{
	    showDialog(e.toString(), "Exception");
	}
	
	static public String average(Vector v)
	{
	    int sum =0;
	    for(int i=0; i< v.size(); i++)
	    {
	        try {
	            sum += Integer.parseInt((String) v.elementAt(i));
	        }
	        catch(Exception e) {}
	        
	    }
	    
	    return String.valueOf(sum/v.size());
	}
	
	static public boolean IsDigit(String s)
	{
	    for (int i=0; i< s.length(); i++)
        {
            if (!Character.isDigit(s.charAt(i)))
                return false;
        }
        return true;
    }
}
//$History:$
