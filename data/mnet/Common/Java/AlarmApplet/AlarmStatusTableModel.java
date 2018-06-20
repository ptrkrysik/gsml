/*
 * AlarmStatusTableModel.java
 * Kevin Lim
 * 03/20/00
 */



import javax.swing.*;
import javax.swing.table.*;
import java.awt.event.*;
import java.awt.*;
import java.util.*;

public class AlarmStatusTableModel extends DefaultTableModel

 
{
    final static int SRC_IP         = 0;
    final static int SRC_NAME       = 1;
    final static int CRITICAL       = 2;
    final static int MAJOR          = 3;
    final static int MINOR          = 4;
    
    static String ColumnTitle[]     = {"Source IP", "Source Name", "Critical Alarm", "Major Alarm", "Minor Alarm"};
    final static int NUM_COL        = ColumnTitle.length;
    
    public AlarmStatusTableModel() 
    {
        super(ColumnTitle, 0);  
    }  
}


