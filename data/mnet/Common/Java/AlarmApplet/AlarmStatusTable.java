/*
 * AlarmStatusTable
 * Kevin Lim
 * 03/20/00
 */



import java.awt.event.*;
import javax.swing.*;
import javax.swing.JTable;
import javax.swing.table.*;
import javax.swing.event.*;
import java.util.Hashtable;
import java.awt.*;
import java.io.*;
import java.net.*;
import java.applet.*;
import com.jetcell.MibWM.Util;
// Shrinivas 23rd Apr 01
import java.util.*;
// Shrinivas end
public class AlarmStatusTable extends  JTable {
    
    DefaultTableModel model;
    JPopupMenu popupMenu = null;
    JApplet applet = null;
    // Shrinivas 23rd Apr 01
    //Alarm alarm = null;
    static Vector alarmSrcIpList = null;
    // Shrinivas end
    final int [] columnMinWidth = { 30,  60,  30,  30,  30, 0};
    final int [] columnMaxWidth = {200, 150, 150, 150, 150, 0};
    Hashtable alarmHashTable    = null;
    
    public AlarmStatusTable(DefaultTableModel model, JApplet applet){
        super(model);        
        // Shrinivas 23rd Apr 01
        alarmSrcIpList = new Vector(8,3);
        // Shrinivas end
        popupMenu = AlarmStatusTablePopupMenu.getPopupMenu();
        this.applet = applet;
                
        addMouseListener(new MouseHandler(this));
        alarmHashTable = new Hashtable();        
        for (int i=0; i < model.getColumnCount(); i++ )
            setColumnWidth(i, columnMinWidth[i], columnMaxWidth[i]);
        TableColumnModel colModel = getColumnModel();
        colModel.getColumn(AlarmStatusTableModel.CRITICAL).setCellRenderer(new CountCellRenderer());
        colModel.getColumn(AlarmStatusTableModel.MAJOR).setCellRenderer(new CountCellRenderer());
        colModel.getColumn(AlarmStatusTableModel.MINOR).setCellRenderer(new CountCellRenderer());
        
        setRowSelectionAllowed(true);   
        setColumnSelectionAllowed(false);
        setSelectionMode( ListSelectionModel.SINGLE_SELECTION );
        setShowGrid(true); 
        setAutoResizeMode(JTable.AUTO_RESIZE_ALL_COLUMNS);        
        getTableHeader().setReorderingAllowed(false);
    }

    public void setColumnWidth(int columnIndex, int minWidth, int maxWidth){
        TableColumn column = (TableColumn)getColumn(getColumnName(columnIndex));
        column.setMinWidth(minWidth);
        column.setMaxWidth(maxWidth);
    }
    
    public boolean isCellEditable(int i, int j){
        return false;
    }
    
    public AlarmStatusData getAlarmStatusData(String source_ip){
        if (alarmHashTable != null) 
            return (AlarmStatusData) alarmHashTable.get(source_ip);
        return null;
    }
    
	public int rowAlarmStatusData(String source_ip){
		int row_count = ((DefaultTableModel)getModel()).getRowCount();
		for(int i=0; i<row_count; i++){
        	String ip = (String)getValueAt(i, AlarmStatusTableModel.SRC_IP);
			if(ip.equals(source_ip))
				return i;
		}
		return -1;
	}

	public void setAlarmStatusData(AlarmStatusData alarm){
		int row_index;
		if (getAlarmStatusData(alarm.source_ip) != null){
			alarmHashTable.remove(alarm.source_ip);
        	alarmHashTable.put(alarm.source_ip, alarm);
			row_index = rowAlarmStatusData(alarm.source_ip);
			setValueAt(alarm.source_ip, row_index, AlarmStatusTableModel.SRC_IP);
			setValueAt(alarm.source_name, row_index, AlarmStatusTableModel.SRC_NAME);
			setValueAt(alarm.critical, row_index, AlarmStatusTableModel.CRITICAL);
			setValueAt(alarm.major, row_index, AlarmStatusTableModel.MAJOR);
			setValueAt(alarm.minor, row_index, AlarmStatusTableModel.MINOR);
		}
		else{
        	alarmHashTable.put(alarm.source_ip, alarm);
			row_index = ((DefaultTableModel)getModel()).getRowCount();
			Object [] row = new Object[AlarmStatusTableModel.NUM_COL];
       		row[AlarmStatusTableModel.SRC_IP]      = alarm.source_ip;
       		row[AlarmStatusTableModel.SRC_NAME]    = alarm.source_name; 
       		row[AlarmStatusTableModel.CRITICAL]    = alarm.critical;
       		row[AlarmStatusTableModel.MAJOR]       = alarm.major;
       		row[AlarmStatusTableModel.MINOR]       = alarm.minor;
       		((DefaultTableModel)getModel()).insertRow(row_index, row);
		}
    }
    
    // Shrinivas 23rd Apr 01
    public static void removeAlarmSrcIp(String ip){
    	alarmSrcIpList.remove(ip);
    }
    // Shrinivas end
    public void OpenEvent(){
    	int selectedRow = getSelectedRow();
	if(selectedRow == -1){
		System.out.println("Open Event: no selection made");
		return;
	}
	String ip = (String)getValueAt(selectedRow, AlarmStatusTableModel.SRC_IP);
	//System.out.println("Open Event: " +ip);
	// Shrinivas 23rd Apr 01
	if(alarmSrcIpList.contains(ip)){
		return;
	}
	else{
		alarmSrcIpList.add(ip);
	}
	Alarm alarmList = new Alarm(AlarmStatusApplet.ip, ip);
	/*if (alarm == null){
		alarm = new Alarm(AlarmStatusApplet.ip, ip);
	}
	else
		alarm.start(ip); */
	// Shrinivas end	
   } // end OpenEvent()

   public void Remove(){
		int selectedRow = getSelectedRow();
		if(selectedRow == -1){
			System.out.println("Remove Event: no selection made");
			return;
		}
        String ip = (String)getValueAt(selectedRow, AlarmStatusTableModel.SRC_IP);
        System.out.println("Remove Event: " +ip);
		AlarmStatusData data = getAlarmStatusData(ip);
		if(data == null){ 
			System.out.println("Remove Event: data not found");
		}
	    else{
			alarmHashTable.remove(ip);
			int row_index = rowAlarmStatusData(ip);
			((DefaultTableModel)getModel()).removeRow(row_index);
		}
		((AlarmStatusApplet)applet).sendRemoveReq(data);
	}

	public void RemoveAll(){
		int row_count = ((DefaultTableModel)getModel()).getRowCount();
		while(row_count != 0){
        	String ip = (String)getValueAt(0, AlarmStatusTableModel.SRC_IP);
			alarmHashTable.remove(ip);	
			((DefaultTableModel)getModel()).removeRow(0);
			row_count = ((DefaultTableModel)getModel()).getRowCount();
		}
	}

    public void Refresh(){
		((AlarmStatusApplet)applet).sendUpdateReq();
    }

    
    class MouseHandler extends MouseAdapter{
		public AlarmStatusTable table;

		public MouseHandler(AlarmStatusTable table){
			this.table = table;
		}

        public void mouseReleased(MouseEvent e){
            if (e.getClickCount() == 2 ){
			    OpenEvent();
            }
            
            if ( e.isPopupTrigger()){
            	// changes by Shrinivas on 17th Oct 2k
            	System.out.println("Event : Popup menu");
            	int selectedRow = rowAtPoint(new Point(e.getX(),e.getY()));
            	setRowSelectionInterval(selectedRow,selectedRow);
            	// end of changes
            	popupMenu.show(table, e.getX(), e.getY());
            }
        }
    }

    class CountCellRenderer extends JLabel implements TableCellRenderer{        
        Font  defaultFont               = new Font("Helvetica",  Font.BOLD,12);
        Font  empFont                   = new Font("TimesRoman", Font.BOLD,18);

        public CountCellRenderer(){
            super("Unknown");
            setHorizontalAlignment(JLabel.CENTER);
        }
        
        public Component getTableCellRendererComponent(  JTable table,
                                                        Object value,
                                                        boolean isSelected,
                                                        boolean hasFocus,
                                                        int row,
                                                        int column ){
            setText((String)value);
            
            return this;
        }        
    }    

}

