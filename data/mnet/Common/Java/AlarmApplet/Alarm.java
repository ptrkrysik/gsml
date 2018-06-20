

import java.awt.*;
import java.awt.event.*;
import java.lang.String;
import javax.swing.*;
import javax.swing.event.*;
import java.net.UnknownHostException;
import com.jetcell.MibWM.*;
import java.net.*;


public class Alarm{
	public AlarmPanel alarmPanel            = null;
	JFrame f                                = null;
	AlarmTCPThread commObj					= null;
	FilterFrame filterFrame 	 = null;	
	public final static int AM_APPLET_PORT2 = 11798;
	JPanel jpanel = null;
    
	// Shrinivas 23rd Apr 01
	public Alarm(String viperbaseIp, final String vipercellIp){
	// Shrinivas end	
		JButton setFilterButton = new JButton("Set Filter");
		final Dimension d = new JFrame().getToolkit().getScreenSize();
     if (f == null){
			alarmPanel = new AlarmPanel(viperbaseIp);
			jpanel = alarmPanel.createPanel();
			alarmPanel.setViperCellIp(vipercellIp);//passing on viperCellIp to panel
			JPanel southPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
			southPanel.add(setFilterButton);
			
			f = new JFrame("Alarm List for source: "+vipercellIp);
			// Shrinivas 23rd Apr 01
			f.addWindowListener(new WindowAdapter(){
				public void windowClosing(WindowEvent evt){
					AlarmStatusTable.removeAlarmSrcIp(vipercellIp);
					destroy();
				}
			});
			// Shrinivas end
			if(Util.applet == null){
				f.setIconImage(Toolkit.getDefaultToolkit().getImage("../Images/app_icon.gif"));
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
				f.setIconImage(Toolkit.getDefaultToolkit().getImage(iconPathURL));
			}
			
			f.getContentPane().setLayout(new BorderLayout());
			f.getContentPane().add(southPanel,BorderLayout.SOUTH);
			f.getContentPane().add(jpanel,BorderLayout.CENTER);
			f.setBounds(0,0, 720,400);
			f.setLocation(d.width/5,d.height/5);
			f.setVisible(true);
     } 
     else
            f.setVisible(true);

		if (commObj == null)
			commObj = new AlarmTCPThread(viperbaseIp, AM_APPLET_PORT2);
		commObj.startReadThread(alarmPanel, vipercellIp, true);
      
		setFilterButton.addActionListener(new ActionListener(){
      		public void actionPerformed(ActionEvent evt){
      			AlarmStatusApplet.logDM("Launch filter screen");
      			filterFrame = new FilterFrame(alarmPanel);
      			filterFrame.setLocation(d.width/4,d.width/5);
      			if(Util.applet == null){
      				filterFrame.setTitle("Filter for user : " + AlarmTCPThread.user.toUpperCase());
      	}
      			else{
      				filterFrame.setTitle("Filter Screen");
      			}	
      			filterFrame.setResizable(false);
      			filterFrame.setVisible(true);
    }
      });
      
    }

	public void start(String vipercellIp){
	    f.setTitle("Alarm List for source: "+vipercellIp);
	    f.setVisible(true);
		alarmPanel.setViperCellIp(vipercellIp);//passing on viperCellIp to panel
		commObj.startReadThread(alarmPanel, vipercellIp, false);
	}

    public void destroy(){
	   System.out.println("Destroy method has been called");
       commObj.finishReadThread();
    }
}
