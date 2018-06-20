import java.io.Serializable;

public class FilterObject implements Serializable{

	// filter criteria
	public int NUM_ALARMS;
	public int TIME;
	public String SEVERITY;
	public int ACKNOWLEDGE;
	
	public static int NONE = 0;
	
	// Alarm number constants
	public static int N1 = 2;
	public static int N2 = 4;
	public static int N3 = 6;
	public static int N4 = 8;

	// Severtiy constants
	public static String CRITICAL = "CRITICAL";
	public static String MAJOR = "MAJOR";
	public static String MINOR = "MINOR";
	public static String WARNING = "WARNING";
	public static String NULL = "NULL";

	// Ack constants
	public static int NO_ACK = 1;
	public static int ALL_ACK = 2;
	
	// Time stamp constants
	public static int MIN_15 = 1;
	public static int HR_1 = 2;
	public static int HR_12 = 3;
	public static int HR_24 = 4;
}
