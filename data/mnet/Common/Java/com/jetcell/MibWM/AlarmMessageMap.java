//////////////////////////////////////////////////////////////////////////////
//
//  Class  : AlarmMessageMap
//  Desc   : 
//  Author : George Zhao
//  Hist   : 06/17/1999 Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import java.awt.event.*;
import java.awt.*;
import java.util.*;
import java.io.*;
import java.net.URL;
import Java.AlarmApplet;

abstract class AlarmMessageMap 
{
    static String file          = "Text/AlarmCode.English";
    static String Comments      = "//";
    static String delim         = "|";
    static Hashtable alarmTable = null;
    static final int SUBJECT    = 0;
    static final int MESSAGE    = 1;
    static final int VECTOR_SIZE= 2;
    static final int LENGTH     = 3;
    static boolean  readfile = false;
    
    static String[] ErrorCode = {
        "0|subect for alarm code 0| detail message for alarm code 0 wdsdsa fdsafdsafs adfdsa fdsafsad fsaf sadfdsaf sadfdsfsa fds dfsa asfa sfasf asdf sadfasfadf  sa fasf asfa dsaf asasfdsafa a afasdfdsaf dsafds afitsdfsfdsafsdfsafjksa;lkfjsalkfjsaf;lksajfsaklfsa;lkfasjfa;lksjfsadlkfjas;lkfjsfalksfjsa;lkfjsafkljdsfkldsafjdskfjflksajflksfj;safjas;lfjsafjsaklfsajflkdsafjsalkfah optional field %1",
        "1|subect for alarm code 1| detail message for alarm code 1 ddsfsdfsafsfaskf;lsjflsfskdl fjsalkfsaflksafwith optional field %1", 
        "2|subect for alarm code 2| detail message for alarm code 2 with optional field %1"
    };
    
    
    static Vector Get(long errorNo)
    {
        if (alarmTable == null)
        {
            Init();
        }
        
        Vector object = (Vector) alarmTable.get(new Long(errorNo));
        if (object == null)
        {
            object = new Vector();
            object.addElement(new String("No entry for data file for error code " +errorNo));
            object.addElement(new String("No entry for data file for error code " +errorNo));
        }
        return object;
    }
    
    static void Init()
    {        
        alarmTable = new Hashtable();
        URL cfg_url;
        
        try {
            if(Util.applet != null) {
							System.out.println("Running applet");            	
                    System.out.println("codebase : " + Util.applet.getCodeBase());
                    System.out.println("documentbase : " + Util.applet.getDocumentBase());
                    System.out.println("file : " + Util.applet.getCodeBase().getFile());    
                //BufferedReader in = new BufferedReader(new FileReader(new File(Util.applet.getDocumentBase().getFile(), file)));
                cfg_url = new URL(Util.applet.getCodeBase()+file);
                //cfg_url = new URL("http://"+ ((AlarmApplet)Util.applet).ip+"/"+file);
            }
            else {
            	System.out.println("Running application");
            	String sep = System.getProperty("file.separator");
            	String pwd = "file:"+".."+sep+"Text"+sep+"AlarmCode.English";
            	cfg_url = new URL(pwd);
				  }
            System.out.println("Open File: " +cfg_url);
            BufferedReader in = new BufferedReader(new InputStreamReader(cfg_url.openStream()));
            String line = null;
            while( (line = in.readLine()) != null)
	        {	
		        ProcessErrorCode(line);
		        readfile = true;
	        }
	    }
	    catch(FileNotFoundException e)
	    {
	        System.out.println(e.toString());
	        System.out.println("Cannot open alarm code file " + file);
	    }
	    catch(IOException ex)
	    {
	        System.out.println("IO Exception: " + ex.toString());
	    }
	    
	    if(!readfile)
	    {
	        for(int i=0; i< ErrorCode.length; i++)
	        {
	            ProcessErrorCode(ErrorCode[i]);
	        }
	    }
	}     
	
	static void ProcessErrorCode(String line)
	{
	    if(line.startsWith(Comments)) return;
    		    
		StringTokenizer token = new StringTokenizer(line, delim);
		if(token.countTokens() != LENGTH)
		{
		    return;
		}
		            		    
		Long errorNo = new Long(Util.trimSpaces(token.nextToken(delim)));
		Vector  alarmMsg = new Vector(VECTOR_SIZE);
		alarmMsg.insertElementAt(token.nextToken(delim), SUBJECT);
		alarmMsg.insertElementAt(token.nextToken(delim), MESSAGE);
		        
		alarmTable.put(errorNo, alarmMsg);
	}
		   
    static void dump()
    {
        if(Util.debug)
            for (Enumeration e = alarmTable.elements() ; e.hasMoreElements() ;) {
            System.out.println(e.nextElement()); 
            }
    } 
    
}
//$History:$
