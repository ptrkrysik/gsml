//////////////////////////////////////////////////////////////////////////////
//
//  Class  : TableMap
//  Desc   : 
//  Author : George Zhao
//  Hist   : 06/17/1999 Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

/**
 * based on Sun TableMap. -gz
 * @version 1.4 12/17/97
 * @author Philip Milne */

package com.jetcell.MibWM;

import javax.swing.table.*;
import javax.swing.event.TableModelListener;
import javax.swing.event.TableModelEvent;

public class TableMap extends DefaultTableModel implements TableModelListener
{
    protected TableModel model;

    public TableModel  getModel() {
        return model;
    }

    public void  setModel(TableModel model) {
        this.model = model;
        model.addTableModelListener(this);
    }

    // By default, Implement TableModel by forwarding all messages
    // to the model.

    public Object getValueAt(int aRow, int aColumn) {
        return model.getValueAt(aRow, aColumn);
    }

    public void setValueAt(Object aValue, int aRow, int aColumn) {
        model.setValueAt(aValue, aRow, aColumn);
    }

    public int getRowCount() {
        return (model == null) ? 0 : model.getRowCount();
    }

    public int getColumnCount() {
        return (model == null) ? 0 : model.getColumnCount();
    }

    public String getColumnName(int aColumn) {
        return model.getColumnName(aColumn);
    }

    public Class getColumnClass(int aColumn)
    {
        if  (model.getColumnCount() == 0)
           return model.getColumnClass(aColumn);

        Object value = model.getValueAt(0,aColumn);
        return value == null ? Object.class : value.getClass();
    }

    public boolean isCellEditable(int row, int column) {
         return model.isCellEditable(row, column);
    }

    public void addColumn(Object col)
    {
        if ( model instanceof DefaultTableModel )
            ( (DefaultTableModel)model).addColumn( col );
        else
            throw new RuntimeException("ERROR: Subclass of DefaultTableModel required.");

    }


    public void addRow(Object rowData[])
    {
        ( (DefaultTableModel)model).addRow( rowData );
    }

    public void removeRow(int rowIndex)
    {
        ( (DefaultTableModel)model).removeRow( rowIndex );
    }

    //
    // Implementation of the TableModelListener interface,
    //

    // By default forward all events to all the listeners.
    public void tableChanged(TableModelEvent e) {
        fireTableChanged(e);
    }
    
    public void insertRow(int row, Object rowData[])
    {
        ( (DefaultTableModel)model).insertRow(row, rowData);
    }
}
//$History:$