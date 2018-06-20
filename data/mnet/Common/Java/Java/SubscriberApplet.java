//////////////////////////////////////////////////////////////////////////////
//
//  Class  : SubscriberApplet
//  Desc   : 
//  Author : George Zhao
//  Hist   : 08/25/1999 Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

package Java;

import java.awt.event.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.event.*;
import java.util.Hashtable;
import java.awt.*;
import com.jetcell.MibWM.*;
import com.jetcell.MibWM.ViperBase.*;


public class SubscriberApplet extends JApplet 
{
    
    
    public void init()
    {  
        try {
            String host = getParameter("HOST");
            int port = Integer.parseInt(getParameter("PORT"));            
            //test. -begin
            Subscriber sub = new Subscriber(host, port);
            //Subscriber sub = new Subscriber();
            // test. -end.
            getContentPane().setLayout(new BorderLayout());
	        getContentPane().setBackground(Color.white);
	        setBackground(Color.white);
	        getContentPane().add(((Subscriber)sub).mainPanel ,BorderLayout.CENTER);
	    }
	    catch(Exception e)
	    {
	        System.out.println("Exception : " +e.toString());
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
    }
}
//$History:$