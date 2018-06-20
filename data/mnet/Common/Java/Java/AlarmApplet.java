//////////////////////////////////////////////////////////////////////////////
//
//  Class  : AlarmApplet
//  Desc   : 
//  Author : George Zhao
//  Hist   : 06/17/1999 Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

package Java;

import java.applet.Applet;
import java.awt.*;
import java.util.Calendar;
import java.lang.String;
import javax.swing.*;
import javax.swing.text.Document;
import javax.swing.event.*;
import java.net.UnknownHostException;
import com.jetcell.MibWM.*;

public class AlarmApplet extends JApplet implements DocumentListener
{
    public static String ip = null;
    JTextField eventField   = null;    
    AlarmPanel alarmPanel   = null;
    JFrame f                = null;
    AlarmCommThread commObj;
   	public final static short TCP_SERVER_PORT = 10977;


    
    public void init()
    {
        Util.applet = this;
        Util.debug = true;
        eventField = new JTextField();
        Document doc = eventField.getDocument();
        doc.addDocumentListener(this);
        
        /*
        setBrokerOptions();
        Broker.addMagicMarkup("QSEvent", "java.lang.String", false);
        RlBeanProperty beanProp = Broker.wrapBeanProperty(eventField, "text", java.lang.String.class);
        Broker.bindMarkupToBeanProperty("QSEvent", "", null, beanProp, true, false, true, null);
        */
        
        if(getParameter("TEST") != null)
        {
            String event1="1111|123456789|0|123|RRM|0|op1|op2";
            String event2="1111|4321999|1|123|RRM|1|op1";
            String event3="1111|3432213|3|123|RRM|2";
            eventField.setText(event1);    
            eventField.setText(event2); 
            eventField.setText(event3); 
        } else {
            commObj = new AlarmCommThread(getCodeBase().getHost(), TCP_SERVER_PORT);
            commObj.startReadThread(eventField);
        }
        
        alarmPanel = new AlarmPanel();
        String newin = getParameter("NEWINDOW");
        if (f != null) 
        {
            f.setVisible(true);
        }
        else {
            if (newin != null)
            {  
                System.out.println("NEWINDOW VALUE = " +newin);
                if(newin.equals("FALSE") || newin.equals("false"))
                {
                    getContentPane().setLayout(new BorderLayout());
                    getContentPane().setBackground(Color.white);
                    setBackground(Color.white);
                    getContentPane().add(alarmPanel.createPanel(),BorderLayout.CENTER);
                }
                else {
                    f = new JFrame("Alarm List for ViperCell: "+ip);
                    f.getContentPane().setLayout(new BorderLayout());
                    f.getContentPane().add(alarmPanel.createPanel());
                    f.setBounds(0,0, 720,400);
                    f.setVisible(true);
                }	
            }
            else {
                System.out.println("Please set NEWINDOW parameter to TRUE or FALSE");
            }
        }
        
        try
        {
            getRootPane().putClientProperty("defeatSystemEventQueueCheck", Boolean.TRUE);
            UIManager.setLookAndFeel( "com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
            SwingUtilities.updateComponentTreeUI(this);
        }
        catch(Exception ex)	
        { 
            //Util.showDialog(f, ex.toString(), "Error", JOptionPane.ERROR_MESSAGE); 
        }  
    }
    
    public void start()
    {
    }
    
    public void stop()
    {
    }
    
    public void destroy()
    {
       commObj.finishReadThread();
	   System.out.println("Destroy method has been called");
    }
    
    public void changedUpdate(DocumentEvent e)
    {
    }
    
    public void insertUpdate(DocumentEvent e)
    {
        /*
        System.out.println("insertUpdate");
        java.util.Random r = new java.util.Random();
        Calendar d = Calendar.getInstance();
        */
        String alarmId = eventField.getText();
        System.out.println("Receiving Alarm: "+alarmId);
        
        // for test only. -gz
        /*
        int eventId = Integer.parseInt(alarmId.substring(alarmId.indexOf('r')+2, alarmId.length()));
        
        String severity = null; 
        
        try {
        d.add(Calendar.SECOND, java.lang.Math.abs(r.nextInt()));
        if (eventId-eventId/3*3 == 0) 
        severity = new String("Critical");
        else if (eventId-eventId/3*3 == 1)
        severity = new String("Minor");
        else if (eventId - eventId/3*3 == 2)
        severity = new String("Info");
        String date =    d.getTime().toString();
        String object = "Object_"+String.valueOf(java.lang.Math.abs(r.nextInt()));
        String message = eventField.getText();
        
        //AlarmData alarm = new AlarmData(String.valueOf(eventId), date, severity, null, object, message, null);
        
        AlarmData alarm = new AlarmData(alarmId);
        alarmPanel.addEvent(alarm);
        Thread.sleep(1000);
        System.out.println("Event: " +alarmId);
        }
        catch(Exception ex)
        {
        System.out.println(ex);
        ex.printStackTrace();
        }
        */
        
        if(!alarmId.startsWith("Event Number"))
        {
            try {
                AlarmData alarm = new AlarmData(alarmId);
                alarmPanel.addEvent(alarm);
            }
            catch(Exception ex) {}
        }
    }
    
    public void removeUpdate(DocumentEvent e)
    {
    }
}
//$History:$