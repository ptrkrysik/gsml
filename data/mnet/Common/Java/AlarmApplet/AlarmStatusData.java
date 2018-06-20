/*
 * AlarmStatusData.java
 * Kevin Lim
 * 03/20/00
 */

import java.util.StringTokenizer;

public class AlarmStatusData 
{
	final static byte	ALARM_SRC_TYPE_VC	= (byte)128;
	final static byte	ALARM_SRC_TYPE_VB	= (byte)129;
 
    String      source_type;
    String      source_ip;
    String      source_name;
    String      critical;
    String      major;
    String      minor;

    public AlarmStatusData( String source_type, String source_ip, String source_name, 
						String critical, String major, String minor)
    {
        this.source_type  = source_type;
        this.source_ip    = source_ip;
        this.source_name  = source_name;
        this.critical     = critical;
        this.major        = major;
        this.minor        = minor;
    }

    public AlarmStatusData(byte[] msg){
		int i = AlarmTCPThread.ALARM_HEAD_SRC_TYPE_POS;
		extractSrcType (msg, i);
		i = AlarmTCPThread.ALARM_MSG_HEADER_LEN;
		extractSrcIP   (msg, i);
		i += AlarmTCPThread.ALARM_MSG_SRC_IP_SIZE;
		extractSrcName (msg, i);
		i += AlarmTCPThread.ALARM_MSG_SRC_NAME_SIZE;
		extractCriCount(msg, i);
		i += AlarmTCPThread.ALARM_MSG_CRITICAL_SIZE;
		extractMajCount(msg, i);
		i += AlarmTCPThread.ALARM_MSG_MAJOR_SIZE;
		extractMinCount(msg, i);

    } 

	public byte[] getSrcIPByte(){
		byte[] ip = new byte[AlarmTCPThread.ALARM_MSG_SRC_IP_SIZE];

		StringTokenizer st = new StringTokenizer(source_ip, ".");
		if(st.countTokens() != 4){
			System.out.println("Error on getSrcIPByte: ip token count");
			return null;
		}	
		else{
			int i = 0;
     		while (st.hasMoreTokens()) {
				ip[i++] = (byte) Integer.parseInt(st.nextToken());
     		}
		}
		return ip;
	}

	public byte getSrcTypeByte(){
		byte type = (byte)0xFF;
		if(source_type.equals("ViperCell"))
			type = AlarmStatusData.ALARM_SRC_TYPE_VC;
		else if(source_type.equals("ViperBase"))
			type = AlarmStatusData.ALARM_SRC_TYPE_VB;
		return type;
	}

	public int extractSrcType(byte[] msg, int i){
		switch(msg[i]){
			case AlarmStatusData.ALARM_SRC_TYPE_VC:
				this.source_type = new String("ViperCell");
				break;
			case AlarmStatusData.ALARM_SRC_TYPE_VB:
				this.source_type = new String("ViperBase");
				break;
			default:
				this.source_type = new String("Unknown");
		}
		return (int)msg[i];
	}

	public void extractSrcIP(byte[] msg, int i)	{
		int h1 = (int)msg[i]   & 0x00FF;
		int h2 = (int)msg[i+1] & 0x00FF;
		int h3 = (int)msg[i+2] & 0x00FF;
		int h4 = (int)msg[i+3] & 0x00FF;
		this.source_ip = ""  + (new Integer(h1)).toString() +
						 "." + (new Integer(h2)).toString() +
						 "." + (new Integer(h3)).toString() +
						 "." + (new Integer(h4)).toString();
	}

	public void extractSrcName(byte[] msg, int i){
		byte[] name = new byte[AlarmTCPThread.ALARM_MSG_SRC_NAME_SIZE];
		for(int j=0; j<AlarmTCPThread.ALARM_MSG_SRC_NAME_SIZE; j++)
			name[j] = msg[i+j];
		this.source_name = (new String(name)).trim();
	}

	public void extractCriCount(byte[] msg, int i){
		this.critical = (new Integer((int)msg[i] & 0x00FF)).toString();
	}

	public void extractMajCount(byte[] msg, int i){
		this.major = (new Integer((int)msg[i] & 0x00FF)).toString();
	}

	public void extractMinCount(byte[] msg, int i){
		this.minor = (new Integer((int)msg[i] & 0x00FF)).toString();
	}
	
	public void dump(){
        System.out.println("*****************************");
        System.out.println("Source Type   : " + source_type);
        System.out.println("Source IP     : " + source_ip);
        System.out.println("Source Name   : " + source_name);
        System.out.println("Critical Count: " + critical);
        System.out.println("Major Count   : " + major);
        System.out.println("Minor Count   : " + minor);
        System.out.println("*****************************");
    }

    public static void main(String[] argv){
       	byte[] msg = { (byte)128, 1, 0, 48, 10, 1, 2, (byte)250, 
				   (byte)'V', (byte)'i', (byte)'p', (byte)'e', (byte)'r', (byte)'1', 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3 }; 
        AlarmStatusData data = new AlarmStatusData(msg);
        data.dump();        
    }
    
}

