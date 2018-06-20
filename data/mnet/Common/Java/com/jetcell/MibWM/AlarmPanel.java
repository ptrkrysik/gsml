//////////////////////////////////////////////////////////////////////////////
//
//  Class  : AlarmPanel
//  Desc   : 
//  Author : George Zhao
//  Hist   : 04/27/1999 Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import javax.swing.*;
import javax.swing.table.*;
import java.awt.event.*;
import javax.swing.text.Document;
import javax.swing.event.*;
import java.net.UnknownHostException;
import java.awt.*;
import java.util.*;
import rl.clbroker.Broker;
import rl.clbroker.RlBeanProperty;
import rl.clbroker.RlError;

import FilterObject;
import FilterFrame;
import java.text.*;
import AlarmStatusApplet;

public class AlarmPanel implements DocumentListener,Cloneable{
    static String host      = null;
    JPanel mainPanel        = null;
    AlarmTable alarmTable   = null;
    
    FilterObject filterObject = null;
    int alarmFilterCount = 0;
    Date filterDate = null;
    Date alarmDate = null;
    SimpleDateFormat formatter = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss z");
    String viperCellIp = null; // gp10 ip for this panel.

    private static JTextField eventField = null;
    
    public AlarmPanel(){
		filterObject = FilterFrame.filterObject;
        AlarmMessageMap.Init();
        /*
        Thread t = new LoadFileThread();
        t.start();
        */
        if(Util.applet == null)
        {
            System.out.println("Running as application");
            Document doc = eventField.getDocument();
            doc.addDocumentListener(this);
        }
    }
    
    // Instantiated from another applet
    public AlarmPanel(String viperbaseIp){
		filterObject = FilterFrame.filterObject;
        host = viperbaseIp;
        AlarmMessageMap.Init();
    }
    public void setViperCellIp(String viperCell){
    	this.viperCellIp = viperCell;
    		alarmTable.setViperCellIp(viperCell);
    }
    
    public AlarmTable getAlarmTable(){
    		return alarmTable;
    }
    public JPanel createPanel(){
		JPanel panel;
		panel = Util.makeBorderPanel("Alarm List");
		//Ananth
		/*if (filterObject.NUM_ALARMS == FilterObject.NONE && 
			filterObject.SEVERITY == FilterObject.NULL &&
			filterObject.TIME == FilterObject.NONE &&
			filterObject.ACKNOWLEDGE == FilterObject.NONE){
			panel = Util.makeBorderPanel("Alarm List");
		}
		else{
			panel = Util.makeBorderPanel("Alarm List (Filtered)");
		}*/
        panel.setLayout(new BorderLayout());
        AlarmTableModel model = new AlarmTableModel();
	    alarmTable = new AlarmTable(model);        
	alarmTable.setViperCellIp(viperCellIp);
        JScrollPane scroller= new JScrollPane(  alarmTable, 
                                                JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED ,
                                                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);   
        panel.add(scroller, BorderLayout.CENTER);

		if (host == null) // applet is run against GP10 instead of GMC
        	alarmTable.Refresh();
        return panel;
    }
    
    public void removeRows(int start,int end){
    	DefaultTableModel model = (DefaultTableModel)alarmTable.getSorter().getModel();
    	AlarmTableSorter sorter = alarmTable.getSorter();
    	int row = 0;
    	int indexAtRow = 0;
    	AlarmStatusApplet.logDM("Row : " + row + " Index : " + indexAtRow);
    	
    	for(int i = 0;i < (end-start); i++){
    		row = model.getRowCount();
    		indexAtRow = alarmTable.getSorter().getIndexAtRow(row-1);
    		AlarmStatusApplet.logDM("Row : " + row + " Index : " + indexAtRow);
    		sorter.filteredRemoveRow(row-1);    	
    	}
    }

    
    public void handleFilterEvent(FilterObject filter){
    		
    		DefaultTableModel model =  (DefaultTableModel)alarmTable.getModel();
    		AlarmTableSorter sorter = alarmTable.getSorter();
    		this.filterObject = filter;   	
    		int rowCount = model.getRowCount();
    		String severity = null;
    		String date = null;
    		Boolean acknowledge = null;
    	
    	// filtering num of Alarms
    		if(filterObject.NUM_ALARMS != FilterObject.NONE){
    			AlarmStatusApplet.logDM("Filtering num: of Alarms");
    			if(rowCount > filterObject.NUM_ALARMS){
    				AlarmStatusApplet.logDM("Filtering no: of Alarms");
    				removeRows(filterObject.NUM_ALARMS,rowCount);
    		}	
    	}
    	
    		int newRowCount = model.getRowCount();
    		MyCalendar myCalendar = new MyCalendar();
    		filterDate = myCalendar.calculateDate();
    		
    	
    		for(int i=0;i<newRowCount;i++){
    			// filtering Severity
    			if(!filterObject.SEVERITY.equals(FilterObject.NULL)){
    				AlarmStatusApplet.logDM("Filtering severity");
    				severity = (String)model.getValueAt(i,AlarmTableModel.SEVERITY);
					if(filterObject.SEVERITY.equals(FilterObject.CRITICAL)){
						if(!severity.toUpperCase().equals(filterObject.CRITICAL)){
    					sorter.filteredRemoveRow(i);
    					i--;
    					newRowCount--;
    					continue;
    				}	
    			}
					else if(filterObject.SEVERITY.equals(FilterObject.MAJOR)){
						if(!(severity.toUpperCase().equals(filterObject.CRITICAL)
						|| severity.toUpperCase().equals(filterObject.MAJOR))){
							sorter.filteredRemoveRow(i);
    						i--;
    						newRowCount--;
    						continue;
    					}
					}
					else if(filterObject.SEVERITY.equals(FilterObject.MINOR)){
						if(!(severity.toUpperCase().equals(filterObject.CRITICAL)
						|| severity.toUpperCase().equals(filterObject.MAJOR)
						|| severity.toUpperCase().equals(filterObject.MINOR))){
							sorter.filteredRemoveRow(i);
    						i--;
    						newRowCount--;
    						continue;
    					}
					}

    		}
    			
    	// filtering Date/Time 
    	if(!(filterObject.TIME == FilterObject.NONE)){
    				AlarmStatusApplet.logDM("Checking timestamp");
    				date = (String)model.getValueAt(i,AlarmTableModel.TIMESTAMP);
    		try{
    					alarmDate = formatter.parse(date);
    		}
    		catch(ParseException ex){
    			System.out.println("Cudn't parse date ...");
    		}
    		if(!alarmDate.after(filterDate)){
    					sorter.filteredRemoveRow(i);
    					i--;
    					newRowCount--;
    					continue;
    		}
    	}	
    	
    	// filtering Ack
    	if(!(filterObject.ACKNOWLEDGE == FilterObject.NONE)){
    				AlarmStatusApplet.logDM("Filtering ack");
    				acknowledge = (Boolean)model.getValueAt(i,AlarmTableModel.ACK);
    		if(filterObject.ACKNOWLEDGE == FilterObject.NO_ACK){
    					if(acknowledge.booleanValue()){
    						sorter.filteredRemoveRow(i);
    						i--;
    						newRowCount--;
    					}	
    						continue;
    		}
    		else if(filterObject.ACKNOWLEDGE == FilterObject.ALL_ACK){
    			 		if(!acknowledge.booleanValue()){
    			 			sorter.filteredRemoveRow(i);
    			 			i--;
    			 			newRowCount--;
    			 		}	
    			 			continue;
    		}
    	}
    		} // end "for loop"
    	
    //	addEvent(alarm);
    }

    public void addEvent(AlarmData alarm){
		String severity;
		Boolean acknowledge;
		int ack;
		String date;
		DefaultTableModel model = (DefaultTableModel)alarmTable.getModel();
		
		if(filterObject != null){ 
				
		// filtering Severity
		if(!filterObject.SEVERITY.equals(FilterObject.NULL)){
	    	AlarmStatusApplet.logDM("Filtering severity");
	    	severity = alarm.severity;
			if(filterObject.SEVERITY.equals(FilterObject.CRITICAL)){
				if(!severity.toUpperCase().equals(filterObject.CRITICAL)){
					return;
	    		}	
			}
			else if(filterObject.SEVERITY.equals(FilterObject.MAJOR)){
				if(!(severity.toUpperCase().equals(filterObject.CRITICAL)
					|| severity.toUpperCase().equals(filterObject.MAJOR))){
					return;
	    		}
			}
			else if(filterObject.SEVERITY.equals(FilterObject.MINOR)){
				if(!(severity.toUpperCase().equals(filterObject.CRITICAL)
					|| severity.toUpperCase().equals(filterObject.MAJOR)
					|| severity.toUpperCase().equals(filterObject.MINOR))){
					return;
	    		}
			}
		}
		
		// filtering Ack
		ack = Integer.parseInt(alarm.acknowledge);
		if(ack == 0){
			acknowledge = new Boolean(false);
		}
		else{
			acknowledge = new Boolean(true);
		}
    	if(!(filterObject.ACKNOWLEDGE == FilterObject.NONE)){
    		AlarmStatusApplet.logDM("Filtering ack");
    		if(filterObject.ACKNOWLEDGE == FilterObject.NO_ACK){
    			if(acknowledge.booleanValue()){
					return;
    			}	
    		}
    		else if(filterObject.ACKNOWLEDGE == FilterObject.ALL_ACK){
    			if(!acknowledge.booleanValue()){
    				return;   				
    			}	
    		}
    	}
		
	   	// filtering Date/Time 
		MyCalendar myCalendar = new MyCalendar();
		filterDate = myCalendar.calculateDate();
		
    	if(!(filterObject.TIME == FilterObject.NONE)){
    		
			AlarmStatusApplet.logDM("Checking timestamp");
    		date = alarm.timestamp;
    		try{
    			alarmDate = formatter.parse(date);
    		}
    		catch(ParseException ex){
    			System.out.println("Cudn't parse date ...");
    		}
    		if(!alarmDate.after(filterDate)){
    			return;
    		}
    	}	
    	 
    	 // Shrinivas 05 jun 01'
    	 // filtering NUM_ALARMS
    	 if(filterObject.NUM_ALARMS != FilterObject.NONE){
    	 	int rowCount = model.getRowCount();
    	 		AlarmStatusApplet.logDM("Filtering num: of Alarms");
    	 		if(rowCount > filterObject.NUM_ALARMS-1){
    	 			AlarmStatusApplet.logDM("Filtering no: of Alarms");
    	 			removeRows(filterObject.NUM_ALARMS-1,rowCount);
    	 		}	
    	}
    	// Shrinivas end
    	}	 
    	

        Object [] row = new Object[AlarmTableModel.NUM_COL];
        row[AlarmTableModel.INDEX]      = alarm.alarmIndex;
        row[AlarmTableModel.READ]       = new Boolean(false);
        row[AlarmTableModel.ALARMID]    = alarm.alarmId;
        row[AlarmTableModel.TIMESTAMP]  = alarm.timestamp;
        row[AlarmTableModel.SEVERITY]   = alarm.severity;
        row[AlarmTableModel.OBJECT]     = alarm.object;
        row[AlarmTableModel.SUBJECT]    = alarm.subject;
        
        if(Integer.parseInt(alarm.acknowledge) == 0)
        	row[AlarmTableModel.ACK]    = new Boolean(false);
        else
        	row[AlarmTableModel.ACK]    = new Boolean(true);

        ((DefaultTableModel)alarmTable.getModel()).insertRow(0, row);
        alarmTable.setAlarmData(alarm);           
        
    }

	public void removeAllEvents()
	{
		while (((DefaultTableModel)alarmTable.getModel()).getRowCount() > 0)
			((DefaultTableModel)alarmTable.getModel()).removeRow(0);
	}

    static public void main(String[] argv)
	{
	    eventField = new JTextField();
	    host = argv[0];
	    try
        {
            boolean useTCP = true;
            int portNum = 80;
            int maxLen = 4096;
            
            System.out.println("Connect to " + host + "  prot " +portNum);
            Broker.setOptions(host, portNum, maxLen, useTCP);
            
        }
        catch(UnknownHostException e) { e.printStackTrace(); }
        Broker.addMagicMarkup("QSEvent", "java.lang.String", false);
        RlBeanProperty beanProp = Broker.wrapBeanProperty(eventField, "text", java.lang.String.class);
        Broker.bindMarkupToBeanProperty("QSEvent", "", null, beanProp, true, false, true, null);
        Broker.start();
	    JFrame f = new JFrame("ViperCell Event List");
	    f.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e)
            {
                System.exit(1);
            }
        });
	    f.getContentPane().setLayout(new BorderLayout());
	    AlarmPanel base = new AlarmPanel();
	    f.getContentPane().add(base.createPanel());
	    f.setBounds(0,0, 720,400);
	    try
		{		   
            UIManager.setLookAndFeel( "com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
            SwingUtilities.updateComponentTreeUI(f);
        }
        catch(Exception ex)	
        { 
            Util.showDialog(f, ex.toString(), "Error", JOptionPane.ERROR_MESSAGE); 
        }	      
	    f.setVisible(true); 
	    
	    /*
	    Random r = new Random();
	    Calendar d = Calendar.getInstance();
	    
        String severity = null; 
        
	    for(int i=0; i<20; i++)
	    {
	        try {
	            d.add(Calendar.SECOND, java.lang.Math.abs(r.nextInt()));
	            if (i-i/3*3 == 0) 
                    severity = new String("Critical");
                else if (i-i/3 == 1)
                    severity = new String("Minor");
                else if (i - i/3*3 == 2)
                    severity = new String("Info");
                String date =    d.getTime().toString();
                String object = "Object_"+String.valueOf(java.lang.Math.abs(r.nextInt()));
                String message = "  Event messages ....";
                
                AlarmData alarm = new AlarmData(String.valueOf(i), date, severity, null, object, message, null);
	            base.addEvent(alarm);
	            Thread.sleep(10000);
	            System.out.println("Event: " +i);
	        }
	        catch(Exception e)
	        {}	        
	    }
	    */
	    
	    
	}
	
	public void changedUpdate(DocumentEvent e)
    {
    }
    
    public void insertUpdate(DocumentEvent e)
    {
	    String alarmId = eventField.getText();
        alarmId.trim();
	    System.out.println("Receiving Alarm: " + alarmId);	    
	    
	    if(!alarmId.startsWith("Event Number"))
	    {
	        try {
	            AlarmData alarm = new AlarmData(alarmId);
	            addEvent(alarm);
	        }
	        catch(Exception ex) {}
	    }
    }
    
    public void removeUpdate(DocumentEvent e)
    {
    }
	
	class LoadFileThread extends Thread
	{
	    public void run()
	    {
	        AlarmMessageMap.Init();
	    }
	}	

	class MyCalendar extends GregorianCalendar{
		
		public Date calculateDate(){
			
			 MyCalendar myCalen = new MyCalendar();
			 long currentMillis = myCalen.getTimeInMillis();
			 long temp;
			 Date pDate = null;
			 
			 if(filterObject.TIME == FilterObject.MIN_15){
			 	temp = 15 * 60 * 1000;
			 	pDate = new Date(currentMillis - temp);
			 }
			 else if(filterObject.TIME == FilterObject.HR_1){
			 	temp = 60 * 60 * 1000;
			 	pDate = new Date(currentMillis - temp);
			 }
			 else if(filterObject.TIME == FilterObject.HR_12){
			 	temp = (12 * 60 ) * 60 * 1000;
			 	pDate = new Date(currentMillis - temp);
			 }
			 else if(filterObject.TIME == FilterObject.HR_24){
			 	temp = (24 * 60 ) * 60 * 1000;
			 	pDate = new Date(currentMillis - temp);
			 }
			return pDate; 
		}
  }
}

//$History:$
