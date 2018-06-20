//////////////////////////////////////////////////////////////////////////////
//
//  Class  : AlarmTable
//  Desc   : 
//  Author : George Zhao
//  Hist   : 06/17/1999 Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.event.*;
import java.util.Hashtable;
import java.awt.*;
import java.io.*;
import java.awt.image.*;
import java.net.*;
import AlarmTCPThread;

public class AlarmTable extends  JTable {
    
    DefaultTableModel model;
    JPopupMenu popupMenu = null;
    
    final int [] columnMinWidth = {25, 30, 150, 50, 80, 250, 0,25};
    final int [] columnMaxWidth = {40, 60, 200, 100, 150, 800, 0,40};
    
    AlarmTableSorter sorter     = null;
    Hashtable alarmHashTable    = null;
    DetailFrame detailFrame     = new DetailFrame();
    String viperCellIp = null;
    
    public AlarmTable(DefaultTableModel model) 
	{
        super(model);        
        popupMenu = AlarmTablePopupMenu.getPopupMenu();
                
        sorter = new AlarmTableSorter(model);
        super.setModel( sorter );
        addMouseListener(new MouseHandler(this));
        alarmHashTable = new Hashtable();        
        for (int i=0; i < model.getColumnCount(); i++ )
            setColumnWidth(i, columnMinWidth[i], columnMaxWidth[i]);
        TableColumnModel colModel = getColumnModel();
        colModel.getColumn(AlarmTableModel.READ).setCellRenderer(new ReadCellRenderer());
        colModel.getColumn(AlarmTableModel.SEVERITY).setCellRenderer(new SeverityRenderer());
        colModel.getColumn(AlarmTableModel.ACK).setCellRenderer(new ReadCellRenderer());
        
        setRowSelectionAllowed(true);   
        setColumnSelectionAllowed(false);
        setSelectionMode( ListSelectionModel.SINGLE_SELECTION );
        setShowGrid(true); 
        setAutoResizeMode(JTable.AUTO_RESIZE_ALL_COLUMNS);        
        setShowVerticalLines(false);
        setGridColor(new Color(153,153,255)); 
    }

    
    public AlarmTableSorter getSorter(){
    			return this.sorter; 	
    }
    public void setViperCellIp(String viperCell){
    	this.viperCellIp = viperCell;
    }
    
    public void setColumnWidth(int columnIndex, int minWidth, int maxWidth)
    {
        TableColumn column = (TableColumn)getColumn(getColumnName(columnIndex));
        
        column.setMinWidth(minWidth);
        column.setMaxWidth(maxWidth);
    }
    
    public boolean isCellEditable(int i, int j)
    {
        return false;
    }
    
    
	public AlarmData getAlarmData(Integer index)
    {
        if (alarmHashTable != null) 
            return (AlarmData) alarmHashTable.get(index);
        return null;
    }
    
    public void setAlarmData(AlarmData alarmdata){
        if(AlarmMessageMap.readfile){
            alarmHashTable.put(alarmdata.alarmIndex, alarmdata);
    }
    }
    
    public void OpenEvent()
    {
        Integer index = (Integer)getValueAt(getSelectedRow(), AlarmTableModel.INDEX);
        System.out.println("Open Event: " +index);
        AlarmData data = (AlarmData) alarmHashTable.get(index);
        data.dump();
        detailFrame.setData(data);
        Dimension d = new JFrame().getToolkit().getScreenSize();
        detailFrame.setLocation(d.width/4,d.height/3);
        detailFrame.setResizable(false);
        if(Util.applet == null){
        	detailFrame.setIconImage(Toolkit.getDefaultToolkit().getImage("../Images/app_icon.gif"));
        }
        else{
        	String iconPath = Util.applet.getCodeBase().toString();
        	iconPath += "Images/app_icon.gif";
        	URL iconPathURL = null;
        	try{
        		iconPathURL = new URL(iconPath);
        	}
        	catch(MalformedURLException exc){
        		System.out.println("Bad URL.");
        	}
        	System.out.println(iconPathURL);
        	detailFrame.setIconImage(Toolkit.getDefaultToolkit().getImage(iconPathURL));
        }
        //detailFrame.setIconImage(Toolkit.getDefaultToolkit().getImage("../Images/app_icon.gif"));
        detailFrame.setVisible(true);
        setValueAt(new Boolean(true), getSelectedRow(), AlarmTableModel.READ);
    }
    
    public void RemoveAllEvents()
    {
        System.out.println("Remove All");
        while( getRowCount()> 0)
        {            
            ((DefaultTableModel) getModel()).removeRow(0);
        }
    }
    
    public void RemoveInfoEvents()
    {
        System.out.println("Remvoe Info");
        sorter.reallocateIndexes();
        sorter.sortByColumn(AlarmTableModel.ALARMID, false);
        for (int i=0; i< getRowCount(); i++)
        {
            if (((String) getValueAt(i, AlarmTableModel.SEVERITY)).equals("Info"))
            {
                System.out.println("removing row " + i);
                ((DefaultTableModel) getModel()).removeRow(i);
            }
        }
    }
    
    public void RemoveReadEvents()
    {
        System.out.println("RemoveRead");
        sorter.reallocateIndexes();
        sorter.sortByColumn(AlarmTableModel.READ, false);
        for (int i=0; i< getRowCount(); i++)
        {
            if (((Boolean) getValueAt(i, AlarmTableModel.READ)).booleanValue())
            {
                System.out.println("removing row " + i);
                ((DefaultTableModel) getModel()).removeRow(i);
                i--;
            }
        }
    }
    
    public void RemoveEvent()
    {
        String alarmId = (String)getValueAt(getSelectedRow(), AlarmTableModel.INDEX);
        System.out.println("Remove Event: " +alarmId);
        sorter.reallocateIndexes();
        sorter.sortByColumn(AlarmTableModel.ALARMID, false);
        for (int i=0; i< getRowCount(); i++)
        {
            if (((String) getValueAt(i, AlarmTableModel.INDEX)).equals(alarmId))
            {
                System.out.println("removing row " + i);
                ((DefaultTableModel) getModel()).removeRow(i);
                break;
            }
        }
    }
    
    
    public void Refresh(){
        RemoveAllEvents();
        java.net.URL cfg_url = null;
        
        // Shrinivas Feb 28th
        AlarmTCPThread.sendEventReq(viperCellIp);

        /*if(Util.applet == null){
        	AlarmTCPThread.sendEventReq(viperCellIp);
        }
        // original applet code : Refresh logic as it was before
        else{
        try {
            if(Util.applet != null)
                cfg_url = new java.net.URL(Util.applet.getDocumentBase()+"Text/alarm.log");
            else
                cfg_url = new java.net.URL("http://"+AlarmPanel.host+"/Text/alarm.log");
            System.out.println("Open File: " +cfg_url);
            java.io.BufferedReader in = new java.io.BufferedReader(new InputStreamReader(cfg_url.openStream()));
            String line = null;
           while( (line = in.readLine()) != null){	
	            try {
		            System.out.println("Line : " +line);
		            AlarmData alarm = new AlarmData(line);
		            setAlarmData(alarm);
		            Object [] row = new Object[AlarmTableModel.NUM_COL];
                    row[AlarmTableModel.INDEX]      = alarm.alarmIndex;
                    row[AlarmTableModel.READ]       = new Boolean(false);
                    row[AlarmTableModel.ALARMID]    = alarm.alarmId;
                    row[AlarmTableModel.TIMESTAMP]  = alarm.timestamp;
                    row[AlarmTableModel.SEVERITY]   = alarm.severity;
                    row[AlarmTableModel.OBJECT]     = alarm.object;
                    row[AlarmTableModel.SUBJECT]    = alarm.subject;
                    
                    ((DefaultTableModel)getModel()).insertRow(0, row);
                }
                catch(Exception ex) {}
	        }
	    }
	      catch(FileNotFoundException e){
	        System.out.println(e.toString());
	        System.out.println("Cannot open alarm code file Text/alarm.log");
	    }
	      catch(IOException ex){
	        System.out.println("IO Exception: " + ex.toString());
	    }
    	}*/
    	
    	// Shrinivas end
    }

    public void Acknowledge(){
    	Integer index = (Integer)getValueAt(getSelectedRow(), AlarmTableModel.INDEX);
    	AlarmData data = (AlarmData) alarmHashTable.get(index);
    	if(data == null)
    		System.out.println("unable to get Alarm data");
    	AlarmTCPThread.sendAck(data.srcIp,data.alarmId,data.recordNumber);
    	setValueAt(new Boolean(true), getSelectedRow(), AlarmTableModel.ACK);
    }
    
    class MouseHandler extends MouseAdapter 
	{
    	AlarmTable holder = null;
    	public MouseHandler(AlarmTable table){
    		this.holder = table;
    	}
    	public void mouseReleased(MouseEvent e){
            if (e.getClickCount() == 2 ) 
			{
			    System.out.println("Process Double click");
			    OpenEvent();
            }
            
            if ( e.isPopupTrigger()){
            	int selectedRow = rowAtPoint(new Point(e.getX(),e.getY()));
            	setRowSelectionInterval(selectedRow,selectedRow);
            	         	
            	popupMenu.show(holder, e.getX(), e.getY());
            }
        }
    }
    
    class ReadCellRenderer extends JCheckBox implements TableCellRenderer
    {
        Color mHighlightedBackground = (Color) (new Color(0, 0, 255)).darker().darker();
        Color mDefaultBackground = SystemColor.window;
        public ReadCellRenderer()
        {
            setBackground(Color.white);
            setHorizontalAlignment(JLabel.CENTER);
        }            
        
        public Component getTableCellRendererComponent(  JTable table,
                                                        Object value,
                                                        boolean isSelected,
                                                        boolean hasFocus,
                                                        int row,
                                                        int column )
        {
            Boolean b = (Boolean) value;
            
            if(b != null){
                setSelected(b.booleanValue());
              
            }
            
            return this;
        }
    }  
    
    class SeverityRenderer extends JLabel implements TableCellRenderer
    {        
        Color mHighlightedBackground    = (Color) (new Color(0, 0, 255)).darker().darker();
        Color mDefaultBackground        = SystemColor.window;
        Font  defaultFont               = new Font("Helvetica",  Font.BOLD,12);
        Font  empFont                   = new Font("TimesRoman", Font.BOLD,18);
        ImageIcon severityIcon = null;
        ImageIcon criticalIcon = null;
        ImageIcon majorIcon = null;
        ImageIcon minorIcon = null;
        ImageIcon infoIcon = null;

        public SeverityRenderer(){
        		super("");
        		loadIcons();
            setHorizontalAlignment(JLabel.CENTER);
        }
        
        private void loadIcons(){
		String fileSep = System.getProperty("file.separator");
		String codeBase = null;
		String imagePath = "file:" +".."+fileSep+"Images"+fileSep;
		String imageFile = null;
		if(Util.applet != null){
			codeBase = Util.applet.getCodeBase().toString();
			System.out.println(codeBase);
		}
		try{
			if(Util.applet != null){
				imageFile = codeBase + "Images/scritical.gif";
			}
			else{
				imageFile = imagePath+"scritical.gif";
			}
			criticalIcon = new ImageIcon(new URL(imageFile));
			
			if(Util.applet != null){
				imageFile = codeBase + "Images/smajor.gif";
			}
			else
				imageFile = imagePath+"smajor.gif";
				majorIcon = new ImageIcon(new URL(imageFile));
			
			if(Util.applet != null){
				imageFile = codeBase + "Images/sminor.gif";
			}
			else
				imageFile = imagePath+"sminor.gif";
				minorIcon = new ImageIcon(new URL(imageFile));
			
			if(Util.applet != null){
				imageFile = codeBase + "Images/sinformation.gif";
			}
			else
				imageFile = imagePath + "sinformation.gif";
				infoIcon = new ImageIcon(new URL(imageFile));
						}
						catch(MalformedURLException malEx){
							System.out.println("Bad URL");
						}
        }
        
        public Component getTableCellRendererComponent(  JTable table,
                                                        Object value,
                                                        boolean isSelected,
                                                        boolean hasFocus,
                                                        int row,
                                                        int column ){
                                                        	
            String severity = (String) value;
            
            if(value != null){
                if(severity.equals(AlarmData.AlarmSeverity[0])) {                    
      			setIcon(criticalIcon);
     			criticalIcon.setImageObserver(new CellImageObserver(table, row, column));
                }
                else if(severity.equals(AlarmData.AlarmSeverity[1])){
                	setIcon(majorIcon);
     			majorIcon.setImageObserver(new CellImageObserver(table, row, column));
                }
                else if (severity.equals(AlarmData.AlarmSeverity[2])){
                	setIcon(minorIcon);
                	minorIcon.setImageObserver(new CellImageObserver(table, row, column));
                }
                else if (severity.equals(AlarmData.AlarmSeverity[3])){
      			setIcon(infoIcon);
      			infoIcon.setImageObserver(new CellImageObserver(table, row, column));
                }
            }
            
            if (isSelected){
                setBackground(mHighlightedBackground);
            }
            else{
                setBackground(mDefaultBackground);
            }
            
            return this;
        }        
    }    
    
    class CellImageObserver implements ImageObserver {
    	JTable table;
    	int row;
    	int col;
    	
    	CellImageObserver(JTable table,int row, int col) {
    		this.table = table;
    		this.row   = row;
    		this.col   = col;
    	}
    	
    	public boolean imageUpdate(Image img, int flags,int x, int y, int w, int h){
    		if ((flags & (FRAMEBITS|ALLBITS)) != 0) {
    			Rectangle rect = table.getCellRect(row,col,false);
    			table.repaint(rect);
    		}
    		return (flags & (ALLBITS|ABORT)) == 0;
    	}
    }


    class DetailFrame extends JFrame implements ActionListener
    {
        JPanel mainPanel = null;
        final int WIDTH  = 520;
        final int HEIGHT = 360;
        final int rows   = 4;
        final int columns = 40;
        
        JTextField  alarmIdField    = Util.makeTextField(false);
        JTextField  severityField   = Util.makeTextField(false);
        JTextField  timestampField  = Util.makeTextField(false);
        JTextField  objectField     = Util.makeTextField(false);
        JTextField  subjectField    = Util.makeTextField(false);
        JTextArea   messageArea     = Util.makeTextArea(rows, columns, false);
        JButton     closeButton     = new JButton("Close");
        
        public DetailFrame ()
        {            
            mainPanel = createPanel();
            getContentPane().add(mainPanel, BorderLayout.CENTER);		
            pack();
            setSize(getPreferredSize());
            addWindowListener(new WindowAdapter() {
                public void windowClosing(WindowEvent e)
                {
                    close();
                }
            });
            closeButton.addActionListener(this);
            try
		    {		   
                UIManager.setLookAndFeel( "com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
                SwingUtilities.updateComponentTreeUI(this);
            }
            catch(Exception ex)	
            { 
                Util.showDialog(this, ex.toString(), "Error", JOptionPane.ERROR_MESSAGE); 
            }
        }
        
        public JPanel createPanel()
        {
            JPanel panel = Util.makeBorderPanel();
            panel.setLayout(new BorderLayout(0, 0));
            panel.add(makeDetailPanel(), BorderLayout.CENTER);
            panel.add(makeMessagePanel(), BorderLayout.SOUTH);
            return panel;
        }
        
        JPanel makeDetailPanel()
        {
            JPanel panel = Util.makeBorderPanel("Event Data");
            GridBagConstraints gbc;
            GridBagLayout gridbag = new GridBagLayout();
            gbc = new GridBagConstraints();   
            JLabel label;
            panel.setLayout(gridbag);
    		
            label = new JLabel("Event ID  ", JLabel.RIGHT);    
            label.setToolTipText("Alarm ID");
		    Util.constrain(label, panel, gbc, 0,0,1,1,
		                GridBagConstraints.VERTICAL,GridBagConstraints.WEST,
		                0.0,0.0,0,5,5,0);

            Util.constrain(alarmIdField, panel, gbc, 1,0,1,1,
		                GridBagConstraints.HORIZONTAL,GridBagConstraints.WEST,
		                100.0,100.0,0,5,5,0);

            label = new JLabel("Severity  ",JLabel.RIGHT);
            label.setToolTipText("Severity of the Event");
            Util.constrain(label, panel, gbc, 2,0,1,1,
		                GridBagConstraints.VERTICAL,GridBagConstraints.WEST,
		                0.0,0.0,0,5,5,0);

            Util.constrain(severityField, panel, gbc, 3,0,1,1,
		                GridBagConstraints.HORIZONTAL,GridBagConstraints.WEST,
		                100.0,100.0,0,5,5,0);
    		
            label = new JLabel("Time Stamp  ",JLabel.RIGHT);
            label.setToolTipText("Event time stamp");
            Util.constrain(label, panel, gbc, 0,1,1,1,
		                GridBagConstraints.VERTICAL,GridBagConstraints.WEST,
		                0.0,0.0,0,5,5,0);

            Util.constrain(timestampField, panel, gbc, 1,1,1,1,
		                GridBagConstraints.HORIZONTAL,GridBagConstraints.WEST,
		                100.0,100.0,0,5,5,0);
		                
		    label = new JLabel("Object  ",JLabel.RIGHT);
		    label.setToolTipText("Name of the object that generated this event");
            Util.constrain(label, panel, gbc, 2,1,1,1,
		                GridBagConstraints.VERTICAL,GridBagConstraints.WEST,
		                0.0,0.0,0,5,5,0);
            
            Util.constrain(objectField, panel, gbc, 3,1,1,1,
		                GridBagConstraints.HORIZONTAL,GridBagConstraints.WEST,
		                100.0,100.0,0,5,5,0);
            
		    label = new JLabel("Subject  ",JLabel.RIGHT);
		    label.setToolTipText("Subject of the Event");
            Util.constrain(label, panel, gbc, 0,2,1,1,
		                GridBagConstraints.VERTICAL,GridBagConstraints.WEST,
		                0.0,0.0,0,5,5,0);
            
            Util.constrain(subjectField, panel, gbc, 1,2,3,1,
		                GridBagConstraints.HORIZONTAL,GridBagConstraints.WEST,
		                100.0,100.0,0,5,5,0);
    		
		                
            return panel;
        }
        
        JPanel makeMessagePanel()
        {
            JPanel panel = Util.makeEmptyBorderPanel();
            panel.setLayout(new BorderLayout(0,0));
            panel.add(messageAreaPanel(), BorderLayout.CENTER);
            panel.add(makeButtonPanel(), BorderLayout.SOUTH);
            return panel;
        }
        
        JPanel messageAreaPanel()
        {
            JPanel panel = Util.makeBorderPanel("Message");
            panel.setLayout(new BorderLayout(0,0));
            messageArea.setLineWrap(true);
            messageArea.setWrapStyleWord(true);
            JScrollPane scroller = new JScrollPane(messageArea);
            panel.add(scroller, BorderLayout.CENTER);
            return panel;
        }
        
        JPanel makeButtonPanel()
        {
            JPanel panel = new JPanel();
            panel.setLayout(new FlowLayout());
            panel.add(closeButton);
            return panel;
        }
        
        public Dimension getPreferredSize() 
	    {
            return new Dimension(WIDTH, HEIGHT);
        }
        
        public void actionPerformed(ActionEvent event) 
	    {
    	    
            Object obj = event.getSource();
            if (obj == closeButton)
                close();
        }
        
        public void close()
        {
            setVisible(false);
        }
        
        public void setData(AlarmData data)
        {
            setTitle("Event Detail for Event Id: " +data.alarmId);
            alarmIdField.setText(data.alarmId);
            severityField.setText(data.severity);
            timestampField.setText(data.timestamp);
            objectField.setText(data.object);
            subjectField.setText(data.subject);
            messageArea.setText(data.message);
        }
    }       
    
}
//$History:$
