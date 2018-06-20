//////////////////////////////////////////////////////////////////////////////
//
//  Class  : AlarmTableSorter
//  Desc   : table sorter for Order explorer request table.
//  Author : George Zhao
//  Hist   : June 18, 1999
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import java.awt.*;
import java.util.*;
import javax.swing.table.*;



class AlarmTableSorter extends TableSorter
{
    public AlarmTableSorter (TableModel model)
    {
        super (model);
    }

    public int compareRowsByColumn(int row1, int row2, int column)
    {
        TableModel data = model;

        // Check for nulls

        Object o1 = data.getValueAt(row1, column);
        Object o2 = data.getValueAt(row2, column);

        // If both values are null return 0
        if (o1 == null && o2 == null) {
            return 0;
        }
        else if (o1 == null) { // Define null less than everything.
            return -1;
        }
        else if (o2 == null) {
            return 1;
        }

        try {
            if (column == AlarmTableModel.ALARMID ) {
                
                float id1 = (new Float (((String)data.getValueAt(row1, AlarmTableModel.ALARMID)).trim())).floatValue();
                float id2 = (new Float (((String)data.getValueAt(row2, AlarmTableModel.ALARMID)).trim())).floatValue();
                

                if (id1 < id2)
                    return -1;
                else if (id1 > id2)
                    return 1;
                else 
                    return 0;                
            }
            else if (column == AlarmTableModel.READ)
            {
                boolean b1 = ((Boolean) data.getValueAt(row1, column)).booleanValue();
                boolean b2 = ((Boolean) data.getValueAt(row2, column)).booleanValue();
                
                if(b1 && !b2) return -1;
                else if(!b1 && b2) return 1;
                else    return 0;
            }                
            else if (column == AlarmTableModel.ACK)
            {
                boolean b1 = ((Boolean) data.getValueAt(row1, column)).booleanValue();
                boolean b2 = ((Boolean) data.getValueAt(row2, column)).booleanValue();
                
                if(b1 && !b2) return -1;
                else if(!b1 && b2) return 1;
                else    return 0;
            }
            else {
                String s1   = (String)data.getValueAt(row1, column);
                String s2   = (String)data.getValueAt(row2, column);
                int result = s1.compareTo(s2);

                if (result < 0)
                    return -1;
                else if (result > 0)
                    return 1;
                else return 0;
            }
        }
        catch (NumberFormatException e)
        {
            System.out.println(e);
        }
        
        return 0;
    }
}
//$History:$
