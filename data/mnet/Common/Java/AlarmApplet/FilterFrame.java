import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import javax.swing.UIManager.*;
import java.net.*;

import com.jetcell.MibWM.*;

 public class FilterFrame extends JFrame{
 	
 	final FilterFrame ff = this;
 
   Container contentPane = null;
   JPanel centerPanel = null;
   JPanel southPanel = null;
   
   // Labels
   JLabel alarms = null;
   JLabel time = null;
   JLabel severity = null;
   JLabel acknowledge = null;
   
   // Combo Boxes
   JComboBox alarmCombo = new JComboBox();   
   JComboBox timeCombo = new JComboBox();   
   JComboBox severityCombo = new JComboBox();   
   JComboBox acknowledgeCombo = new JComboBox();
   
   // Reference to "AlarmPanel"
   AlarmPanel alarmPanel = null;
   
   public static FilterObject filterObject = null;
   public static FilterObject tempFilterObject = null;
   	
   public FilterFrame(){
   	if(Util.applet == null){
   		setIconImage(Toolkit.getDefaultToolkit().getImage("../Images/app_icon.gif"));
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
   		setIconImage(Toolkit.getDefaultToolkit().getImage(iconPathURL));
   	}
   }
   	
   public FilterFrame(AlarmPanel panel){
   	this.alarmPanel = panel;
	if(tempFilterObject == null)
		tempFilterObject = filterObject;
   	try{
   		UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
   	}
   	catch(Exception ex){
   		System.out.println("Error setting Look and Feel for Filter frame");
   	}
   	if(Util.applet == null){
   		setIconImage(Toolkit.getDefaultToolkit().getImage("../Images/app_icon.gif"));
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
   		setIconImage(Toolkit.getDefaultToolkit().getImage(iconPathURL));
   	}
   	JButton ok = new JButton("   OK   ");
   	JButton cancel = new JButton("CANCEL");
	JButton saveDefault = new JButton("Save As Default");
   	contentPane = getContentPane();
    getCenterPanel(tempFilterObject);
   	if(tempFilterObject != null){
   		mapFilterOnFrame(tempFilterObject);
   	}
    southPanel = new JPanel();
    southPanel.setLayout(new FlowLayout(FlowLayout.CENTER));
    southPanel.add(ok); 
    southPanel.add(cancel); 
	if(Util.applet == null){
		southPanel.add(saveDefault);	
	}
    
    contentPane.add(centerPanel,BorderLayout.CENTER);
    contentPane.add(southPanel,BorderLayout.SOUTH);
    setSize(400,200);
    addWindowListener(new windowListener());
    
    ok.addActionListener(new ActionListener(){
    	public void actionPerformed(ActionEvent evt){
    		AlarmStatusApplet.logDM("Handle filter selection");
    		handleFilterSave();
    	}
    	});
    
    cancel.addActionListener(new ActionListener(){
    	public void actionPerformed(ActionEvent evt){
    		dispose();
    		AlarmStatusApplet.logDM("No filter selection");
    	}
    });
    
	saveDefault.addActionListener(new ActionListener(){
		public void actionPerformed(ActionEvent evt){
			handleFilterSave();
			filterObject = tempFilterObject;
			
		}
	});
    
   }
   
   public void handleFilterSave(){

		// alarms selected  
		tempFilterObject = new FilterObject();
		String alarms = (String)alarmCombo.getSelectedItem();
		if(alarms.equals("All")){
			tempFilterObject.NUM_ALARMS = FilterObject.NONE;
    			}
		else{
    				try{
				tempFilterObject.NUM_ALARMS = Integer.parseInt(alarms);
    				}
    				catch(NumberFormatException nfex){
				JOptionPane.showMessageDialog(ff,"Bad Alarm number criteria.Default value assumed. ","Invalid number",JOptionPane.ERROR_MESSAGE);
				tempFilterObject.NUM_ALARMS = FilterObject.NONE;
    			}
    			}
		
		// time selected 
		int timeIndex = timeCombo.getSelectedIndex();
		
		if(timeIndex == 1){
			tempFilterObject.TIME = FilterObject.MIN_15;
    		}
		else if(timeIndex == 2){
			tempFilterObject.TIME = FilterObject.HR_1;
		}
		else if(timeIndex == 3){
			tempFilterObject.TIME = FilterObject.HR_12;
    			}
		else if(timeIndex == 4){
			tempFilterObject.TIME = FilterObject.HR_24;
    			}
    			else
			tempFilterObject.TIME = FilterObject.NONE;
				
		// severity selected 
		int severityIndex = severityCombo.getSelectedIndex();
		
		if(severityIndex == 1){
			tempFilterObject.SEVERITY = FilterObject.CRITICAL;
    		}
		else if(severityIndex == 2){
			tempFilterObject.SEVERITY = FilterObject.MAJOR;
    			}
		else if(severityIndex == 3){
			tempFilterObject.SEVERITY = FilterObject.MINOR;
		}
		else
			tempFilterObject.SEVERITY = FilterObject.NULL;
		
		// acknowledge selected 
		int ackIndex = acknowledgeCombo.getSelectedIndex();
		if(ackIndex == 1){
			tempFilterObject.ACKNOWLEDGE = FilterObject.ALL_ACK;
		}
		else if(ackIndex == 2){
			tempFilterObject.ACKNOWLEDGE = FilterObject.NO_ACK;
		}
		else
			tempFilterObject.ACKNOWLEDGE = FilterObject.NONE;
		this.dispose();
		// Ananth
		/*if (tempFilterObject.NUM_ALARMS == FilterObject.NONE && 
			tempFilterObject.SEVERITY == FilterObject.NULL &&
			tempFilterObject.TIME == FilterObject.NONE &&
			tempFilterObject.ACKNOWLEDGE == FilterObject.NONE){
			Util.setBorderTitle("Alarm List");
    		}
    		else{
			Util.setBorderTitle("Alarm List (Filtered)");
		}*/
		
    		if(alarmPanel!=null){
			alarmPanel.handleFilterEvent(tempFilterObject);
			alarmPanel.getAlarmTable().Refresh();
		}
		else{
			filterObject = tempFilterObject;
    	}	
    	
    	}
   
	private void mapFilterOnFrame(FilterObject tempFilterObject){
	
		if(tempFilterObject.NUM_ALARMS != FilterObject.NONE){
			String alarms = Integer.toString(tempFilterObject.NUM_ALARMS);
			alarmCombo.getEditor().setItem(alarms);			
   		}
   		else{
			alarmCombo.setSelectedIndex(0);
   		}
   		
		if(tempFilterObject.TIME != FilterObject.NONE){
		
			if(tempFilterObject.TIME == FilterObject.MIN_15)
		   		timeCombo.setSelectedIndex(1);
		   	else if(tempFilterObject.TIME == FilterObject.HR_1)
		   		timeCombo.setSelectedIndex(2);
		   	else if(tempFilterObject.TIME == FilterObject.HR_12)
		   		timeCombo.setSelectedIndex(3);
		   	else if(tempFilterObject.TIME == FilterObject.HR_24)
		   		timeCombo.setSelectedIndex(4);
   	}
		else{
			timeCombo.setSelectedIndex(0);
   	}
   	
		if(!tempFilterObject.SEVERITY.equals(FilterObject.NULL)){
			
			if(tempFilterObject.SEVERITY.equals(FilterObject.CRITICAL))
				severityCombo.setSelectedIndex(1);
			else if(tempFilterObject.SEVERITY.equals(FilterObject.MAJOR))
				severityCombo.setSelectedIndex(2);
			else if(tempFilterObject.SEVERITY.equals(FilterObject.MINOR))
				severityCombo.setSelectedIndex(3);
   	}
		else{
			severityCombo.setSelectedIndex(0);
		}
		   	
		if(tempFilterObject.ACKNOWLEDGE != FilterObject.NONE){
		  
		  if(tempFilterObject.ACKNOWLEDGE == FilterObject.ALL_ACK)
		  	acknowledgeCombo.setSelectedIndex(1);
		  else if(tempFilterObject.ACKNOWLEDGE == FilterObject.NO_ACK)
		  	acknowledgeCombo.setSelectedIndex(2);
		}
		else{
			acknowledgeCombo.setSelectedIndex(0);
   	}
   }	
   
   public JPanel getCenterPanel(FilterObject filterObj){
   	
   	centerPanel = Util.makeBorderPanel("Filter Screen");
	centerPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
   	alarms = new JLabel(" Alarms ");
   	alarms.setToolTipText("Check to set Alarm filter / Uncheck to set Default (All)");
   	time = new JLabel(" Time ");
   	time.setToolTipText("Check to set Time filter / Uncheck to set Default(All)");
   	severity = new JLabel(" Severity ");
   	severity.setToolTipText("Check to set Severity filter / Uncheck to set Default (All)");
   	acknowledge = new JLabel(" Acknowledge ");
   	acknowledge.setToolTipText("Check to set Acknowledge filter / Uncheck to set Default (All)");
   	
   	// Alarms
	alarmCombo.setPreferredSize(new Dimension(50,20));
	alarmCombo.setEditable(true);
	alarmCombo.addItem("All");
	for(int i=1;i<=20;i++){
		alarmCombo.addItem(Integer.toString(i));
	}
   	
   	// Time
	timeCombo.addItem("All");
	timeCombo.addItem("15 mins");
	timeCombo.addItem("1 hour");
	timeCombo.addItem("12 hours");
	timeCombo.addItem("24 hours");
   	
   	// Severity
	severityCombo.addItem("All");
	severityCombo.addItem("Critical");
	severityCombo.addItem("Critical-Major");
	severityCombo.addItem("Critical-Minor");

   	// Acknowledge
	acknowledgeCombo.addItem("All");
	acknowledgeCombo.addItem("Acknowledged");
	acknowledgeCombo.addItem("Unacknowledged");
   	
 	// Alarms  	
   	final JPanel p1 = new JPanel();
   	p1.setLayout(new FlowLayout(FlowLayout.LEFT));
	p1.add(alarms);
	p1.add(alarmCombo);
	p1.add(acknowledge);
	p1.add(acknowledgeCombo);
   	
   	
   	// Time
   	final JPanel p2 = new JPanel();
   	p2.setLayout(new FlowLayout(FlowLayout.LEFT));
	p2.add(severity);
	p2.add(severityCombo);
	p2.add(time);
	p2.add(timeCombo);	
   	
   	// Severity
	
   	// Acknowledge
   	
   	
   	centerPanel.add(p1);
   	
   	centerPanel.add(p2);
   	
   	if(filterObj != null){
   		mapFilterOnFrame(filterObj);
   	}
   	return centerPanel;
   }
   
   
   class windowListener extends WindowAdapter{
   	public void windowClosing(WindowEvent evt){
   		dispose();
   		AlarmStatusApplet.logDM("Closing Filter Screen");
   	}
   }
   
 }
