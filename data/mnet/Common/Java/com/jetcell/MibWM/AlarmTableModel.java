//////////////////////////////////////////////////////////////////////////////
//
//  Class  : AlarmTableModel
//  Desc   : 
//  Author : George Zhao
//  Hist   : 06/17/1999 Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import javax.swing.*;
import javax.swing.table.*;
import java.awt.event.*;
import java.awt.*;
import java.util.*;

public class AlarmTableModel extends DefaultTableModel

 
{
    final static int INDEX          = 6;
    final static int READ           = 0;
    final static int ALARMID        = 1;
    final static int TIMESTAMP      = 2;
    final static int SEVERITY       = 3;
    final static int OBJECT         = 4;
    final static int SUBJECT        = 5;
    final static int MESSAGE        = 6;
    final static int ACK            = 7;
    
    
    static String ColumnTitle[]     = {"Read", "Alarm ID", "Time Stamp", "Severity", "Object", "Message Subject", "","Ack"};
    final static int NUM_COL        = ColumnTitle.length;
    
    public AlarmTableModel() 
    {
        super(ColumnTitle, 0);  
    }  
}


//$History:$
