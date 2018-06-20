
/*
 * AlarmTCPThread.java
 * Kevin Lim
 * 03/21/00
 */



import java.net.*;
import java.io.*;
import javax.swing.*;
import javax.swing.*;
import javax.swing.text.Document;
import javax.swing.event.*;
import com.jetcell.MibWM.*;

public class AlarmTCPThread extends Thread{
	
	static private boolean FILTER = false;
	private String filterVipercellIp = null;

	static final int MAX_IN_BUF_SIZE			= 4096;
	static final int MAX_MSG_BUF_SIZE			= 1024;
	static final int ALARM_HEAD_SRC_TYPE_POS	= 0;
	static final int ALARM_HEAD_MSG_TYPE_POS	= 1;
	static final int ALARM_HEAD_MSG_SIZE_POS    = 2;
	static final int ALARM_MSG_HEADER_LEN		= 4;
	static final int ALARM_MSG_SRC_IP_SIZE		= 4;
	static final int ALARM_MSG_SRC_NAME_SIZE	= 41;
	static final int ALARM_MSG_CRITICAL_SIZE	= 1;
	static final int ALARM_MSG_MAJOR_SIZE		= 1;
	static final int ALARM_MSG_MINOR_SIZE		= 1;
	
	static final byte ALARM_MSG_TYPE_UPDATE_REQ	= 1;
	static final byte ALARM_MSG_TYPE_UPDATE_RSP = 2;
	static final byte ALARM_MSG_TYPE_REMOVE_REQ = 3;
	static final byte ALARM_MSG_TYPE_EVENT_REQ  = 4;
	static final byte ALARM_MSG_TYPE_EVENT_RSP  = 5;
	static final byte ALARM_MSG_TYPE_ACK        = 6;

	static final byte ALARM_MSG_SRC_TYPE_APPLET = (byte)0xFF;
	
static final int MAX_MODULE_SPECIC_ALARM    = 0xFFFF;
    static final int ALARM_CRITICAL_START_POINT = 0x0001;
    static final int ALARM_MAJOR_START_POINT    = 0x0100;
    static final int ALARM_MINOR_START_POINT    = 0x0800;
    static final int ALARM_INFO_START_POINT     = 0x1000;
    static final int ALARM_CRITICAL             = 0;
    static final int ALARM_MAJOR                = 1;
    static final int ALARM_MINOR                = 2;
    static final int ALARM_INFO                 = 3;
    public static String user 		= null; 	
    
	Socket socket;
	String host;
	int    port;
	int appletVipercellIp = 0;
	int msgVipercellIp = 0;
	AlarmStatusPanel panel = null;
	AlarmPanel alarmPanel = null;
	AlarmData alarmData = null;
	BufferedInputStream		in		= null;
	InputStreamReader		isr		= null;
	public static BufferedOutputStream	out		= null;
	boolean running;

	public AlarmTCPThread(String host, int port) {
		this.host = host;
		this.port = port;
		System.out.println("Connecting to the Server " + host + " at port " + port);
		try {
			socket = new Socket(host, port);
		} catch(IOException e) {
			System.out.println("Could not connect to the Server");
			socket = null;			
		}
		if(socket != null){
			try {    
				in  = new BufferedInputStream(socket.getInputStream());			// reading is done to byte arrays
				out = new BufferedOutputStream(socket.getOutputStream());		// writing is done from byte arrays
				out.flush();
			} catch(IOException e) {
				System.out.println("IO exception in AlarmTCPThread: ");
				close_socket();
			}
		}
	}
	
	public void sendRemoveReq(AlarmStatusData data)
	{
		if(out == null) return;
		try {    
			byte[] msg = {ALARM_MSG_SRC_TYPE_APPLET,
						  ALARM_MSG_TYPE_REMOVE_REQ,
						  0, 5, 0, 0, 0, 0, 0}; 
			int i = AlarmTCPThread.ALARM_MSG_HEADER_LEN;
			byte[] ip_addr = data.getSrcIPByte(); 
			if(ip_addr == null){	
				System.out.println("Error on sendRemoveReq: getSrcIPByte failed");
				return;
			}
			for(int j=0; j<AlarmTCPThread.ALARM_MSG_SRC_IP_SIZE; j++){
				msg[i++] = ip_addr[j];
				System.out.println("ip_addr "+ip_addr[j]);
			}
			byte type = data.getSrcTypeByte();
			if(type == (byte)0xFF){	
				System.out.println("Error on sendRemoveReq: getSrcTypeByte returned unknown");
				return;
			}
			msg[i++] = type;	
			out.write(msg, 0, i);
			out.flush();
		} catch(IOException ioe) {
			System.out.println("IO exception in AlarmTCPThread:send "+ioe);
		}
	}

	public void sendUpdateReq(){
		if(out == null) return;
		try {    
			byte[] msg = {ALARM_MSG_SRC_TYPE_APPLET,
						  ALARM_MSG_TYPE_UPDATE_REQ,
						  0, 0}; 
			out.write(msg, 0, ALARM_MSG_HEADER_LEN);
			out.flush();
		} catch(IOException ioe) {
			System.out.println("IO exception in AlarmTCPThread:send "+ioe);
		}
	}

    // send alarm event msg update request to AlarmServer
	static public void sendEventReq(String vipercellIp){
	    int index = 0;
	    String tmpStr = vipercellIp;
	    String valueStr;
	    Short [] ip = new Short[4];
		if (out == null)
		    return;

        // converting IP string into 4 octets
        for (int i = 0; i < 4; i++){
            index = tmpStr.indexOf(".");
            valueStr = i == 3 ? tmpStr : tmpStr.substring(0, index);
            ip[i] = new Short(valueStr);
            tmpStr = tmpStr.substring(index + 1);
        }
        
		try {    
			byte[] msg = {ALARM_MSG_SRC_TYPE_APPLET,
						  ALARM_MSG_TYPE_EVENT_REQ,
						  0x00, 0x04, // length of data
						  ip[0].byteValue(), ip[1].byteValue(), ip[2].byteValue(),
						  ip[3].byteValue()}; 
			out.write(msg, 0, ALARM_MSG_HEADER_LEN + 4);
			out.flush();
		} 
		catch(IOException ioe) {
			System.out.println("IO exception in AlarmTCPThread:send "+ioe);
		}
		FILTER = false;
	}

	public static void sendAck(String srcIp,String alarmId,String recordNum){
		int index = 0;
		String tmpStr = srcIp;
		String valueStr;
		Short [] ip = new Short[4];
		
		byte aid[] = new byte[4];
		byte rec[] = new byte[4];

		
		// converting IP string into 4 octets
		for (int i = 0; i < 4; i++){
			index = tmpStr.indexOf(".");
			valueStr = i == 3 ? tmpStr : tmpStr.substring(0, index);
			ip[i] = new Short(valueStr);
			tmpStr = tmpStr.substring(index + 1);
		}
		
		// converting alarmID to bytes
		aid = AlarmTCPThread.extractBytes(alarmId);
		rec = AlarmTCPThread.extractBytes(recordNum);
		
		try {    
			byte[] msg = {ALARM_MSG_SRC_TYPE_APPLET,
										ALARM_MSG_TYPE_ACK,
										0x00, 0x0c, // length of data
										ip[0].byteValue(), ip[1].byteValue(), ip[2].byteValue(),
										ip[3].byteValue(),
										aid[3],aid[2],aid[1],aid[0],
										rec[3],rec[2],rec[1],rec[0]}; 
			out.write(msg, 0, msg.length);
			out.flush();
		} 
		catch(IOException ioe) {
			AlarmStatusApplet.logDM("IO exception in AlarmTCPThread: ack "+ioe);
		}
	}
	
	public static byte[] extractBytes(String s){
		
		byte store[] = new byte[4];
		int intValue = Integer.parseInt(s);
		
		store[0] = (byte)(intValue & 0x000000ff);
		store[1] = (byte)((intValue >> 8)  & 0x000000ff);
		store[2] = (byte)((intValue >> 16) & 0x000000ff);
		store[3] = (byte)((intValue >> 24) & 0x000000ff);
		
		return store;
	}

	public void alarmMsgHandler(byte[] msg) {
		int index = AlarmTCPThread.ALARM_HEAD_MSG_TYPE_POS;
		switch(msg[index]){
			case ALARM_MSG_TYPE_UPDATE_RSP:
			    if (panel != null)
			    {
				    AlarmStatusData alarm = new AlarmStatusData(msg);
				    panel.addEvent(alarm);
				}
				break;
			case ALARM_MSG_TYPE_EVENT_RSP:
			    if (alarmPanel != null)
			    {
			        try {
			            alarmData = new AlarmData(parseEventData(msg));
			        }
			        catch(Exception e) {}
			        if (appletVipercellIp == msgVipercellIp){
			          		alarmPanel.addEvent(alarmData);
			        }    
				}
			    break;
			default:
				System.out.println("Unknown message received "+(int)msg[index]);	
				int size = 0x00FF & (int)msg[AlarmTCPThread.ALARM_HEAD_MSG_SIZE_POS];
				size = (size << 8) | (0x00FF & (int)msg[AlarmTCPThread.ALARM_HEAD_MSG_SIZE_POS+1]);
				size += AlarmTCPThread.ALARM_MSG_HEADER_LEN; 
				if(size > AlarmTCPThread.MAX_MSG_BUF_SIZE) size	= AlarmTCPThread.MAX_MSG_BUF_SIZE;
				for(int i=0; i<size; i++) System.out.print(" " + (int)(msg[i] & 0x00FF));
				System.out.println(" ");
		}
	}

	public void close_socket() {
		if(socket != null){
		try {
			socket.close();
			System.out.println("closing socket at third window level...");
			} 
			catch(IOException e) {
			System.out.println("Couldn't close socket " + e);
		}
		}
	}

	public void reconnect() {
		boolean done = false;
		while(!done){
			try {
			    if (panel != null)
				    panel.dispStatus("Attempting to reconnect to the Server...");
				socket = new Socket(host, port);
				done = true;
				if (panel != null)
				    panel.dispStatus("Server on-line");
			} 
			catch(IOException e) {
			    if (panel != null)
				    panel.dispStatus("Server off-line");
			}
			try {
				sleep(1000);
			}
			catch (InterruptedException ie) {
				System.out.println(" Interrupted Excution = " + ie);
			}
		}
		if(socket != null){
			try {    
				in  = new BufferedInputStream(socket.getInputStream());			// reading is done to byte arrays
				out = new BufferedOutputStream(socket.getOutputStream());		// writing is done from byte arrays
				out.flush();
				if (panel != null)	{
				    panel.clearEvents();
				    sendUpdateReq();
				}
			} 
			catch(IOException e) {
				System.out.println("IO exception in AlarmTCPThread: ");
				close_socket();
			}
		}
	}

	public void run() {
		// Shrinivas 01 June 01
		if(socket == null && running ) reconnect();
		byte[] buf = new byte[AlarmTCPThread.MAX_IN_BUF_SIZE];
		byte[] msg = new byte[AlarmTCPThread.MAX_MSG_BUF_SIZE];
		running = true;
		int readBytes;
		int index;
		int msg_size;
		int header_size = AlarmTCPThread.ALARM_MSG_HEADER_LEN;
		while(running){
			try {
				readBytes = in.read(buf, 0, AlarmTCPThread.MAX_IN_BUF_SIZE);
				if(readBytes == -1){
					System.out.println("End of stream reached!");
					close_socket();
					// Shrinivas 01 June 01'
					if(running){
						reconnect();
					}
					// Shrinivas end
				}
				index = 0;
				while ((index + header_size) <= readBytes){	// while there are msg at least size of header
					msg_size = 0x00FF & (int)buf[index+AlarmTCPThread.ALARM_HEAD_MSG_SIZE_POS];
					msg_size = (msg_size << 8) 
							 | (0x00FF & (int)buf[index+AlarmTCPThread.ALARM_HEAD_MSG_SIZE_POS+1]);
					if((header_size + msg_size) <= (readBytes - index)){ // about to copy msg within valid range
						for(int i=0; i<(header_size + msg_size); i++){
							msg[i] = buf[index++];
						}
						AlarmStatusApplet.logDM("Received msg with length: " + msg_size);
						alarmMsgHandler(msg);
					}
					else{
						System.out.println("Length Err. Msg Length specified  : "+msg_size);
						System.out.println("Length Err. Bytes left in read buf: "+(readBytes - index));
						index = readBytes; // finish while
					}
				}
			} 
			catch(IOException e) {
				close_socket();
				if(running != false){
					reconnect();
				}
				System.out.println("Stopping thread.");
			}
		}
	}

	public void startReadThread(AlarmStatusPanel panel){
		this.panel = panel;
		start();
		panel.dispStatus("Server on-line");
		sendUpdateReq();
	}
	
	public void startReadThread(AlarmPanel panel, String vipercellIp, boolean startThread){
	    alarmPanel = panel;
	    alarmPanel.removeAllEvents();
	    setAppletVipercellIp(vipercellIp);
	    if (startThread)
	        start();
	  this.filterVipercellIp = vipercellIp;      
		sendEventReq(vipercellIp);
	}
	
	public void finishReadThread(){
		System.out.println("TCP thread is being shutdown");
		running = false;
		close_socket();
		stop();
	}

	private void setAppletVipercellIp(String vipercellIp){
	    String valueStr;
	    Short octetVal;
	    String tmpStr = vipercellIp;
	    int index = 0;
	    appletVipercellIp = 0;
        for (int i = 0; i < 4; i++)
        {
            index = tmpStr.indexOf(".");
            valueStr = i == 3 ? tmpStr : tmpStr.substring(0, index);
            if (i > 1) // we use only last two octets
            {
                octetVal = new Short(valueStr);
                if (i == 2)
                    appletVipercellIp = octetVal.shortValue() << 8;
                else
                    appletVipercellIp += octetVal.shortValue();
            }
            tmpStr = tmpStr.substring(index + 1);
        }
	}

	private int extractNumber(byte[] msg, int pos){
	    return ((msg[pos] << 24 & 0xff000000) +
	        (msg[pos+1] << 16 & 0xff0000) +
	        (msg[pos+2] << 8 & 0xff00) +
	        (msg[pos+3] & 0xff));
	}
	
	private int extractIp(byte[] msg, int pos){
	    // for now, we are only extracting last 2 octets of an IP address
	    return((msg[pos+2] << 8 & 0xff00) + (msg[pos+3] & 0xff));
	}
	
	public String extractSrcIP(byte[] msg, int i){
		int h1 = (int)msg[i]   & 0x00FF;
		int h2 = (int)msg[i+1] & 0x00FF;
		int h3 = (int)msg[i+2] & 0x00FF;
		int h4 = (int)msg[i+3] & 0x00FF;
		String source_ip = ""  + (new Integer(h1)).toString() +
										 "." + (new Integer(h2)).toString() +
			   "." + (new Integer(h3)).toString() +
			   "." + (new Integer(h4)).toString();
			   
	return source_ip;				   
  }

	private int getSeverity(int errorCode){
	    int severity;
	    
		if (errorCode == 0)
			return ALARM_INFO;

        int tmp = (errorCode & MAX_MODULE_SPECIC_ALARM);
        if (tmp < ALARM_MAJOR_START_POINT)
        {
            severity =  ALARM_CRITICAL;
        } else if (tmp < ALARM_MINOR_START_POINT) {
            severity =  ALARM_MAJOR;
        } else if (tmp < ALARM_INFO_START_POINT) {
            severity = ALARM_MINOR;
        } else {
            severity = ALARM_INFO;
        }
        return severity;
	}
	
	private String parseEventData(byte[] msg)	{
	    int id = extractNumber(msg, 4);
	    int code = extractNumber(msg, 8);
	    int severity = getSeverity(code);
	    int timestamp = extractNumber(msg, 12);
	    int srcIp = extractIp(msg, 16);
		
	    String srcIpString = extractSrcIP(msg, 16);

	    if (0 == appletVipercellIp){
	        appletVipercellIp = srcIp;
	        msgVipercellIp = srcIp;
	    }
	    else
	        msgVipercellIp = srcIp;
	    int srcModule = extractNumber(msg, 64);
	    int opt1 = extractNumber(msg, 68);
	    int opt2 = extractNumber(msg, 72);
	    int ack = extractNumber(msg,76);
	    int recNumber = extractNumber(msg,80);
	    String srcName = new String(msg, 20, 41);
	    srcName = srcName.trim();
	    String eventData = new String(id+"|"+timestamp+"|"+severity+"|"+srcModule+"|"+
	        srcName+"|"+code+"|"+opt1+"|"+opt2+"|"+srcIpString+"|"+ack+"|"+recNumber+"|");
	    return eventData;
	}
}

