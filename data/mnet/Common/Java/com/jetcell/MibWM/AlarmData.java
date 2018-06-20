//////////////////////////////////////////////////////////////////////////////
//
//  Class  : AlarmData
//  Desc   : 
//  Author : George Zhao
//  Hist   : 06/17/1999 Initial Creation.
//
//////////////////////////////////////////////////////////////////////////////

package com.jetcell.MibWM;

import javax.swing.*;
import javax.swing.table.*;
import java.awt.event.*;
import java.text.*;
import java.awt.*;
import java.util.*;
public class AlarmData 
{
    static  int AlarmIndex = 0;
    
    String      alarmId;
    String      timestamp;
    String      severity;
    String      objectId;
    String      object;
    String      errorNo;
    String      subject;
    String      message;
    Integer     alarmIndex;
    String      optional1;
    String      optional2;
    String 			acknowledge;
    String 			recordNumber;
	String			srcIp;
    static int  NUM_REQUIRED_FIELDS = 8;
    static int  NUM_OPTIONAL_FIELDS = 3;
    static int  OP1                 = 6;
    static int  OP2                 = 7;
    static int  SRCIP				= 8; 
    static int  ACK 				= 9;
    static int  REC_NUM				= 10;
    static final String OP1Field    = "%1";
    static final String OP2Field    = "%2";
    static final String OPModule    = "%module";
    static String[] AlarmSeverity   = { "Critical",
                                        "Major",
                                        "Minor",
                                        "Information"
                                       };

    public static SimpleDateFormat formatter = new SimpleDateFormat( "yyyy/MM/dd HH:mm:ss z" );
    
    public static SimpleDateFormat myFormatter = new SimpleDateFormat("dd.MM.yyyy/HH:mm:ss");

    static final char        dilim = '|';
    
    public AlarmData( String alarmId, String timestamp, String severity,
                       String objectId, String object, String subject, String message)
    {
        this.alarmId    = alarmId;
        this.timestamp  = timestamp;
        this.severity   = severity;
        this.objectId   = objectId;
        this.object     = object;
        this.subject    = subject;
        this.message    = message;
    }

    public AlarmData(String event) throws Exception
    {
        // event are in the format of:
        // eventId|timestamp|severity|objectid|object name|event no |optional 1 |optional 2 | optional 3
        String[] values = new String[NUM_REQUIRED_FIELDS+NUM_OPTIONAL_FIELDS];
            
        
        int index   = 0;
        int pos     = 0;
        int i       = 0;
        boolean end = false;        
        while(!end)
        {
            index = event.indexOf(dilim, pos);            
            if (index == -1) 
            {
                index = event.length();
                end = true;
                continue;
            }
            values[i++] = event.substring(pos,index);     
            pos = index+1;
        }  
        
		try {
            alarmId         = values[0].trim();
            
            timestamp       = formatter.format(new Date(1000*Long.parseLong(Util.trimSpaces(values[1]))));
            //timestamp = Util.trimSpaces(values[1]);
            if(Util.debug)
            {
                System.out.println("values[1] " + values[1]);
                System.out.println("Util.trimSpaces(values[1])) " + Util.trimSpaces(values[1]));
                System.out.println("Long.parseLong(Util.trimSpaces(values[1])) " + Long.parseLong(Util.trimSpaces(values[1])));
                Date d = new Date(1000*Long.parseLong(Util.trimSpaces(values[1])));
                System.out.println("date " + d.toString());
                System.out.println("date.gettime() "+d.getTime());
            }
            
            severity        = AlarmSeverity[Integer.parseInt(Util.trimSpaces(values[2]))];
            objectId        = values[3].trim();
            object          = values[4];
            errorNo         = values[5].trim();
            int error = Integer.parseInt(errorNo);
            Vector alarmMsg = AlarmMessageMap.Get(error);
            subject = (String) alarmMsg.elementAt(AlarmMessageMap.SUBJECT);
            message = (String) alarmMsg.elementAt(AlarmMessageMap.MESSAGE);
            alarmIndex   = new Integer(AlarmIndex++);
            subject = Replace(subject, object, OPModule);
            message = Replace(message, object, OPModule);

            optional1 = values[OP1].trim();
            optional2 = values[OP2].trim();
            srcIp = values[SRCIP].trim();
            acknowledge = values[ACK].trim();
            recordNumber = values[REC_NUM].trim();

            if(values[OP1] != null)
            {
                optional1 = values[OP1];
                subject = Replace(subject, optional1, OP1Field);
                message = Replace(message, optional1, OP1Field);
            }
                
            if(values[OP2] != null) 
            {
                optional2 = values[OP2];
                subject = Replace(subject, optional2, OP2Field);
                message = Replace(message, optional2, OP2Field);
            }
        }
        catch(Exception e)
        {
            System.out.println("Invalid error No. " +errorNo);
            e.printStackTrace();
            throw e;
        }       
    } 


    public Object getValueAt(int column) 
    {
        switch (column) 
		{
            case AlarmTableModel.ALARMID:
                return alarmId;
            case AlarmTableModel.TIMESTAMP:
                return timestamp;            
            case AlarmTableModel.SEVERITY:
                return severity;
            case AlarmTableModel.OBJECT:
                return object;
            case AlarmTableModel.SUBJECT:
                return subject;
            default:
                return "Unknown";
        }        
    }  
    
    public void dump()
    {
        if(Util.debug)
        {
            System.out.println("Alarm ID  : " +alarmId);
            System.out.println("Timestamp : " +timestamp);
            System.out.println("Severity  : " +severity);
            System.out.println("Object ID : " +objectId);
            System.out.println("Object    : " +object);
            System.out.println("Error No  : " +errorNo);
            System.out.println("Subject   : " +subject);
            System.out.println("Message   : " +message);
            System.out.println("Optional1 : " +optional1);
            System.out.println("Optional2 : " +optional2);
            System.out.println("Index:      " +alarmIndex);
        }
    }
    
    String Replace(String field, String opfield, String op)
    {
        String message = null;
        int pos = field.indexOf(op);
        if(pos != -1) message = field.substring(0, pos) + opfield + field.substring(pos+op.length(), field.length());
        else message = field;
        return message;
    }
    
    public static void main(String[] argv)
    {
        // alarmId|timestamp|severity|objectId|objectName|error code|op 1| op 2
        String event1="12323|10:23:34 1999|Critical|123|RRM|0|op1|op2";
        String event2="12323|10:23:34 1999|Critical|123|RRM|1|op1";
        String event3="12323|10:23:34 1999|Critical|123|RRM|2";
        
        try {
            AlarmData data = new AlarmData(event1);
            data.dump();        
            data = new AlarmData(event2);
            data.dump();
            data = new AlarmData(event3);
            data.dump();
        }
        catch(Exception e) {}
    }
    
}

//$History:$
