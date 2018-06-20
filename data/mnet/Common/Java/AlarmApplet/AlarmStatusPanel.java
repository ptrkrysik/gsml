/*
 * AlarmStatusPanel.java
 * Kevin Lim
 * 03/20/00
 */


import javax.swing.*;
import javax.swing.border.*;
import javax.swing.table.*;
import java.awt.event.*;
import javax.swing.text.Document;
import javax.swing.event.*;
import java.net.UnknownHostException;
import java.awt.*;
import java.util.*;

public class AlarmStatusPanel{
    static String host      = null;
    AlarmStatusTable alarmStatusTable   = null;
   	JLabel	statLabel = null;
    
	public AlarmStatusPanel(){
    }
    
	public JPanel createPanel(JApplet applet){
		JPanel panel = new JPanel();
		panel.setBorder(new EmptyBorder(5, 5, 5, 5));
        panel.setLayout(new BorderLayout());
        AlarmStatusTableModel model = new AlarmStatusTableModel();
	    alarmStatusTable = new AlarmStatusTable(model, applet);        
        JScrollPane scroller= new JScrollPane(  alarmStatusTable, 
                                                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED ,
                                                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);   
        panel.add(scroller, BorderLayout.CENTER);
		statLabel = new JLabel("Server Status");
        panel.add(statLabel, BorderLayout.SOUTH);
        return panel;
    }
    
  public void addEvent(AlarmStatusData alarm){
        alarmStatusTable.setAlarmStatusData(alarm);           
    }

	public void dispStatus(String status){
		statLabel.setText(status);
	}

	public void clearEvents()	{
        alarmStatusTable.RemoveAll();
	}
}

