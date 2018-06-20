#ifndef _PM_COMMON_H_
#define _PM_COMMON_H_

#include "pm\pmdefs.h"

const int PM_MAX_INTERVAL   = 2;
const int PM_MAX_RECORDS    = 1500;

#define PM_ONE_SECOND               1
#define PM_ONE_MINUTE               (PM_ONE_SECOND*60)
#define PM_SECONDS_IN_15_MIN        900
#define PM_SECONDS_IN_30_MIN        1800
#define PM_SECONDS_IN_45_MIN        2700
#define PM_SECONDS_IN_HOUR          3600


#ifdef UNIT_TEST_PMCLASS
typedef int     CauseCode;
#endif

/* Bind options */
#define DEFAULT_PORT_ADDR        1
#define FIXED_PORT_LOCAL_ADDR    2
#define GET_PORT_AND_LOCAL_ADDR  3

/* Defaults */
#define     kANY_PORT         0
#define FIXED_PORT             1045

#define kMaxRecvUdpMsgSize	2048

typedef struct socket				OS_SPECIFIC_SOCKET_SOCKET;
typedef struct socket				OS_SPECIFIC_DGRAM_SOCKET;

extern unsigned long GetLocalHostAddress();
extern OS_SPECIFIC_SOCKET_HANDLE	SOCKET_UdpCreate();
extern bool SOCKET_UdpBind(OS_SPECIFIC_SOCKET_HANDLE sock, ubyte2 port);

extern bool SOCKET_UdpReceive(OS_SPECIFIC_DGRAM_HANDLE sock, char *msg, ubyte4 *msgSize, ubyte4 *peerAddr);
extern bool SOCKET_UdpSendTo(OS_SPECIFIC_DGRAM_HANDLE sock, struct sockaddr_in *pAddrTo, char *message, ubyte4 msglen);
extern bool SOCKET_Close( OS_SPECIFIC_SOCKET_HANDLE sock );

/*
 * The PM_BaseObject class declaration
 */
class PM_BaseObject {
    private:
	    /*
	     * base Performance-Management class
	     * - provide critical section
         */
        SEM_ID      m_semaphore;
    public:
	    bool takeSemaphore(void);
	    bool giveSemaphore(void);
        /*
         * Constructor for the object
         */
        PM_BaseObject()
        {
            m_semaphore = semBCreate(SEM_Q_PRIORITY,SEM_FULL);
        };

        /*
         * Destructor for the object
         */
        ~PM_BaseObject()
        {
            if (m_semaphore != NULL)
                semDelete(m_semaphore);
        }

};

int pm_map_cause_to_index(CauseCode cause);
CauseCode pm_map_index_to_cause(int index);
char * pm_map_cause_code_to_str(CauseCode cause);

/*
 * The PM_SimpleCounter class declaration
 * 1-minute interval period
 * total of 15 completed intervals (last 15 minutes) plus
 * 1 current interval (accumulating)
 */
class PM_SimpleCounter: public PM_BaseObject
{
    public:
	    void reset(void);       /* reset counter to 0 (both value and maxValue) */
	    bool increment(void);   /* increment the counter value by 1 */
        bool increment(int x);  /* increment the counter value by x */
	    bool decrement(void);   /* decrement the counter value by 1 */
	    int getValue(void);	    /* obtain the current value of the counter for
		                         * the past PM_MAX_INTERVAL-1 (15) completed intervals (sum)
		                         */
		bool setValue(int new_value);

	    int getMaxValue(void);  /* obtain the maximum value of the counter for
		                         * the past PM_MAX_INTERVAL-1 (15) completed intervals (accumulated max)
                                 */

	    void advanceInterval(void);/* move to the next measurement interval (called by PM task) */

        /* Constructor for the class */
        PM_SimpleCounter()
        {
            currentInterval = 0;
        };

private:
	    int value[PM_MAX_INTERVAL];		/* maintain the current value for each interval */
	    int maxValue[PM_MAX_INTERVAL];	/* maintain the max value for each interval */
	    int currentInterval;			/* index of the current interval (cyclical) */
};




/*
 * The PM_PerCauseCounter class declaration
 */
class PM_PerCauseCounter: public PM_BaseObject
{
    public:
	    void reset(void);	                /* reset counters to 0 (both value and maxValue) */
	    bool increment(CauseCode cause);	/* increment the counter value by 1 */
	    bool decrement(CauseCode cause);	/* decrement the counter value by 1 */
	    int getValue(CauseCode cause);      /* obtain the current value of the counter for
		                                     * the past PM_MAX_INTERVAL-1 (15) completed intervals (sum)
                                             */

	    void advanceInterval(void);         /* move to the next measurement interval (called by PM task) */

    private:
	    PM_SimpleCounter perCauseCounter[PM_MAX_CAUSES];
		/* each per-cause counter is a simpleCounter */
};



/*
 * The PM_StatisticCounter class declaration
 */
class PM_StatisticCounter: public PM_BaseObject
{
    public:
	    void reset(void);                   /* reset the entire records[] array by
		                                     * removing all records, and set the numberRecords to 0
                                             */

	    bool addRecord(int value);		    /* insert a record into the record array
		                                     * return true if successful
		                                     * return false if out of record entry (newest records will be discarded)
                                             */

	    int getMeanValue(void);             /* obtain the arithmatic-mean of all the records for
                                		     * the past PM_MAX_INTERVAL-1 (15) completed intervals
                                             */
		int getMaxValue();

	    void advanceInterval(void); 	    /* move to the next measurement interval (called by PM task) */

        PM_StatisticCounter()
        {
            reset();
        }

    private:
	    int numRecords[PM_MAX_INTERVAL];
	    int records[PM_MAX_INTERVAL][PM_MAX_RECORDS];
	    int currentInterval;			    /* index of the current interval (cyclical) */
        /*
         * Constructor for the class
         */

};



/*
 * The PM_AccumTimeCounter class declaration
 * 1-minute interval period
 * total of 15 completed intervals (last 15 minutes) plus
 * 1 current interval (accumulating)
 */
class PM_AccumTimeCounter: public PM_BaseObject
{
    public:
	    void reset(void);       /* reset counter to 0 (both value and maxValue) */
	    void start();			/* stat accumulating time */
        void stop();			/* stop accumulating time */
	    int	 getValue(void);	/* obtain the current value of the counter for
		                         * the current interval
		                         */
	    void advanceInterval(void);/* move to the next measurement interval (called by PM task) */

        /* Constructor for the class */
        PM_AccumTimeCounter()
        {
            currentInterval = 0;
        };

private:
	    int accumValue[PM_MAX_INTERVAL];	/* maintain the current accumulated value for each interval */
	    int lastStart[PM_MAX_INTERVAL];		/* maintain the tick value when the timer was last started */
	    bool state[PM_MAX_INTERVAL];			/* maintain the on-off state of the timer */
	    int currentInterval;				/* index of the current interval (cyclical) */
};

#endif
