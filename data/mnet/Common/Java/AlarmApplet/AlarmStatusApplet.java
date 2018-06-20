/*
 * AlarmStatusApplet.java
 * Kevin Lim
 * 03/20/00
 */

import com.jetcell.MibWM.*;
import java.applet.Applet;
import java.awt.*;
import java.awt.event.*;
import java.util.Calendar;
import java.lang.String;
import javax.swing.*;
import javax.swing.text.Document;
import javax.swing.event.*;import java.net.*; 
import java.net.UnknownHostException;

public class AlarmStatusApplet extends JApplet{
	
    public static String ip = null;
    public static int port = 11798; /*11800;*/
    AlarmStatusPanel alarmPanel   = null;
    JFrame f                = null;
    static AlarmTCPThread commObj = null;

    public static boolean isDebug = false;

    public void init(){
    		Util.applet = this;
    	   String newin = getParameter("NEWWINDOW");
        ip = getParameter("HOST");
        port = Integer.parseInt(getParameter("PORT"));
		
		createUIPanel(newin);	
	}	
	// changes by Shrinivas : the code under this method were under the above init() method
	// earlier. Function split was done so that the code could run both as an applet and an application
	protected void createUIPanel(String newin){
		alarmPanel = new AlarmStatusPanel();	
		if (f != null){
            f.setVisible(true);
        }
        else {
     	if (newin != null){  
                System.out.println("NEWWINDOW VALUE = " +newin);
     		if(newin.equals("FALSE") || newin.equals("false")){
                    getContentPane().setLayout(new BorderLayout());
                    getContentPane().add(alarmPanel.createPanel(this),BorderLayout.CENTER);
                }
                else {
                    f = new JFrame("Alarm Status List for " + ip);
                    f.setIconImage(Toolkit.getDefaultToolkit().getImage("../Images/app_icon.gif"));
                    f.getContentPane().setLayout(new BorderLayout());
                    f.getContentPane().add(alarmPanel.createPanel(this));
                    f.setBounds(0,0, 720,400);
                    f.setVisible(true);
                }	
            }
            else {
                System.out.println("Please set NEWWINDOW parameter to TRUE or FALSE");
                System.out.println("Assuming - no new window");
                getContentPane().setLayout(new BorderLayout());
                getContentPane().add(alarmPanel.createPanel(this),BorderLayout.CENTER);
            }
        }

        commObj = new AlarmTCPThread(ip, port);
        commObj.startReadThread(alarmPanel);
    }
    
  public void start(){
        System.out.println("Applet started");
    }
    
    public void stop(){
        System.out.println("Applet stopped");
	    if (commObj != null){
       	  commObj.finishReadThread();
       	  commObj = null;
			}
    }
    
    public void destroy(){
	   System.out.println("Destroy method has been called");
    }
    
	public void sendRemoveReq(AlarmStatusData data){
		commObj.sendRemoveReq(data);
	}

	public void sendUpdateReq(){
		commObj.sendUpdateReq();
	}
   
	public static void logDM(String message){
		if(isDebug){
			System.out.println(message);
		}
	}
	
	public static void stopAPM1(){
		if(commObj != null)
			commObj.finishReadThread();
	}
	
	public static void startAPM1(String ipStr,boolean debug,String user){
		
		isDebug = debug;
		final JFrame frame = new JFrame();
    	
    	try {
    		InetAddress.getByName(ipStr);
    	}
    	catch (UnknownHostException ex) {
    		JOptionPane.showMessageDialog(frame,"Invalid Host ID:","Error",JOptionPane.ERROR_MESSAGE);
            return;        
    	}
		ip = ipStr;
		final JApplet applet = new AlarmStatusApplet();
		((AlarmStatusApplet)applet).createUIPanel("false");
		AlarmTCPThread.user = user;
		frame.setContentPane(applet.getContentPane());
		frame.setBounds(0,0, 720,400);
		frame.setTitle("Alarm Status List for " + ip);
		frame.setIconImage(Toolkit.getDefaultToolkit().getImage("../Images/app_icon.gif"));
		Dimension d = new JFrame().getToolkit().getScreenSize();
		frame.setLocation(d.width/10,d.height/10);
        	frame.setVisible(true);
		frame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		
		frame.addWindowListener(new WindowAdapter()	{
			public void windowClosed(WindowEvent e){
                		applet.stop();
                    }
				});
		
	}
}

