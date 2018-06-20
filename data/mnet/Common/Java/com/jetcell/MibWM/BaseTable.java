//////////////////////////////////////////////////////////////////////////////
//
//  Class  : BaseTable
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
import javax.swing.border.*;
import javax.swing.table.*;
import javax.swing.event.*;

public class BaseTable extends JTable
{
    static final int KEY_UP   = 1;
    static final int KEY_DOWN = 2;
    static final int KEY_HOME = 3;
    static final int KEY_END  = 4;
    static final int KEY_PGUP = 5;
    static final int KEY_PGDN = 6;

    JTableHeader tableHeader;
    TableModel model;
    TableSorter sorter;
	static JPopupMenu popupMenu;

	public BaseTable()
	{
		super();
		modTable();
	}

    public BaseTable(Object[][] data, Object[] cols)
    {
        super(data,cols);
        modTable();

    }

    public BaseTable(TableModel dm)
    {
        super();
        model = dm;
        modTable();
    }


	public BaseTable(TableModel dm, TableColumnModel cm)
	{
		super(dm, cm);
        model = dm;
		modTable();
	}


	public BaseTable(TableModel dm, TableColumnModel cm, ListSelectionModel sm)
	{
		super(dm, cm, sm);
        model = dm;
		modTable();
	}


	public BaseTable(int numRows, int numColumns)
	{
		super(numRows, numColumns);
		modTable();
	}


	public BaseTable(Vector rowData,  Vector columnNames)
	{
		super(rowData, columnNames);
		modTable();
	}

	public void setPopupMenu (JPopupMenu popupMenu)
	{
	    this.popupMenu = popupMenu;
	}
	
	public JPopupMenu getPopupMenu()
	{
	    return popupMenu;
	}


    void modTable()
    {
        addKeyboardHooks();
        addHeaderHooks();

		popupMenu = TablePopupMenu.getPopupMenu();
		add(popupMenu);
        addMouseListener(new MouseHandler() );
    }


    void addHeaderHooks()
    {
        if( model != null )
        {
            sorter = new TableSorter( model );
            sorter.addMouseListenerToHeaderInTable( this );
            super.setModel( sorter );
        }

    }

    public boolean isCellEditable(int i, int j)
    {
        return false;
    }

    public void makeCurrentRowVisible()
    {
        scrollRectToVisible(getCellRect(getSelectedRow(),0 , true));
    }

	public void selectFirstRow()
	{
		getSelectionModel().setSelectionInterval(0,0);
	}

    void addKeyboardHooks()
    {
        //debug("adding table hooks");
        int criteria;
        criteria = WHEN_IN_FOCUSED_WINDOW;
        //criteria = WHEN_FOCUSED;
        registerKeyboardAction(new TableKeyboardHandler(KEY_UP)  , KeyStroke.getKeyStroke(KeyEvent.VK_UP,0), criteria);
        registerKeyboardAction(new TableKeyboardHandler(KEY_DOWN), KeyStroke.getKeyStroke(KeyEvent.VK_DOWN,0), criteria);
        registerKeyboardAction(new TableKeyboardHandler(KEY_HOME), KeyStroke.getKeyStroke(KeyEvent.VK_HOME,0), criteria);
        registerKeyboardAction(new TableKeyboardHandler(KEY_END) , KeyStroke.getKeyStroke(KeyEvent.VK_END,0), criteria);
        registerKeyboardAction(new TableKeyboardHandler(KEY_PGUP), KeyStroke.getKeyStroke(KeyEvent.VK_PAGE_UP,0), criteria);
        registerKeyboardAction(new TableKeyboardHandler(KEY_PGDN), KeyStroke.getKeyStroke(KeyEvent.VK_PAGE_DOWN,0), criteria);
    }

    class TableKeyboardHandler extends AbstractAction
    {
        int key;
        int pageInc = 5;	// fix later to found out number of visible rows!
        int row;
        int lastRow;

        public TableKeyboardHandler(int key)
        {
            this.key = key;
        }

        public void actionPerformed(ActionEvent e)
        {
            switch(key)
            {
                case KEY_UP :
                    //debug("Up arrow");
            		row = getSelectedRow();
            		if (row <= 0)
            		    getToolkit().beep();
            		else
            		{
            		    row=row-1;
            		    setRowSelectionInterval (row,row);
            		    updateUI();
            		}
                    break;

                case KEY_DOWN :
                    //debug("Down arrow");
            		row = getSelectedRow();
            		if ((row == getRowCount()-1) || (row < 0))
            		    getToolkit().beep();
            		else
            		{
            		    row++;
            		    setRowSelectionInterval (row,row);
            		    updateUI();
            		}
                    break;

                case KEY_HOME :
                    //debug("Home key");
            		row = getSelectedRow();
            		if (row <= 0)
            		    getToolkit().beep();
            		else
            		{
            		    setRowSelectionInterval (0,0);
            		    updateUI();
            		}
                    break;

                case KEY_END :
                    //debug("End key");
            		lastRow = getRowCount()-1;
            		row = getSelectedRow();
            		if ((row == lastRow) || (row < 0))
            		    getToolkit().beep();
            		else
            		{
            		    setRowSelectionInterval (lastRow,lastRow);
            		    updateUI();
            		}
                    break;

                case KEY_PGUP :
                    //debug("Page up");
            		row = getSelectedRow();
            		if (row <= 0)
            		    getToolkit().beep();
            		else if (row-pageInc <= 0)
            		{
            		    setRowSelectionInterval (0,0);
            		    updateUI();
            		}
            		else
            		{
            		    row=row-pageInc;
            		    setRowSelectionInterval (row,row);
            		    updateUI();
            		}
                    break;

                case KEY_PGDN :
                    //debug("Page down");
            		int lastRow = getRowCount()-1;
            		row = getSelectedRow();
            		if ((row == lastRow) || (row < 0))
            		    getToolkit().beep();
            		else if (row+pageInc >= lastRow)
            		{
            		    setRowSelectionInterval (lastRow,lastRow);
            		    updateUI();
            		}
            		else
            		{
            		    row+=pageInc;
            		    setRowSelectionInterval (row,row);
            		    updateUI();
            		}
                    break;

                default :
                    System.out.println("Key not handled " + key);
                    break;
            }

            makeCurrentRowVisible();
        }

    }


    class MouseHandler extends MouseAdapter
    {
        public void mouseReleased(MouseEvent e)
        {
            if ( e.isPopupTrigger())
            {
                popupMenu.show(BaseTable.this, e.getX(), e.getY());
            }
        }
    }

	public void setScrollPane(JScrollPane p)
	{
		// only here for compatibility with drop1 code. remove later!
	}

    public void setColumnWidth(int columnIndex, int minWidth, int maxWidth)
    {
        TableColumn column = (TableColumn) getColumn( getColumnName( columnIndex ) );

        column.setMinWidth(minWidth);
        column.setMaxWidth(maxWidth);
    }

    public String getDataFromTable( int col )
    {
        if ( getRowCount() > 0 )
        {
            int row = getSelectedRow();

            if (row < 0)
            {
                setRowSelectionInterval(0,0);
                row = getSelectedRow();
            }

            return (String) getModel().getValueAt( row, col );
        }
        else
            return "";
    }

    public void setColumnHeaderTitle( int column, String value )
    {
    	DefaultTableColumnModel cm = (DefaultTableColumnModel)getColumnModel();
		TableColumn tc = cm.getColumn( column );
		tc.setHeaderValue( value );
	}


}
//$History:$