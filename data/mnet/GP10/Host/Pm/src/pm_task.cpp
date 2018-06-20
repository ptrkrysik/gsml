#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vxWorks.h>
#include <semlib.h>
#include <time.h>
#include <wdlib.h>
#include <sysLib.h>
#include <taskLib.h>
#include <fioLib.h>

#ifndef UNIT_TEST_PMCLASS
#include "vipermib.h"
#endif

#include "defs.h"
#include "apm/include/apmdefs.h"
#include "apm/include/msgproc.h"
#include "pm/pmdefs.h"
#include "pm/pm_class.h"

#include "Os/JCTask.h"
#include "Os/JcModule.h"
#include "Gp10OsTune.h"
#include "MnetModuleId.h"
#include "oam_api.h"

static bool initialized = false;
static unsigned long reqd_interval_in_secs;

extern void pm_reset_all_counters();
extern void pm_advance_interval();
void pm_timer_handler(int arg);


/*
 * Global variables
 */
static WDOG_ID	  g_pm_timer;
static int g_timer_ticks_required;
extern PmSourceToAmsMsg amsMsg;
SEM_ID pmSemId;
VOID pm_task(void);
bool pm_sendDataToAms();

JCTask pmTaskObj("PMTask");  


/*
 * SysCommand_PM
 */
/**************************/    
int SysCommand_PM(T_SYS_CMD action)
{
    DBG_FUNC("SysCommand_PM",PM_LAYER);
    DBG_ENTER();
    switch(action){
        case SYS_SHUTDOWN:
			{
				printf("PM TASK shutdown notification received\n");
				DBG_WARNING("PM TASK shutdown notification received\n");
			}
            break;

        case SYS_START:
			{
				DBG_TRACE("Spawning PM Task now\n");

				if (pmTaskObj.JCTaskSpawn(PM_TASK_PRIORITY, PM_TASK_OPTION, PM_TASK_STACK_SIZE,
				(FUNCPTR) pm_task, 0,0,0,0, 0, 0, 0, 0, 0, 0,MODULE_PM,JC_CRITICAL_TASK) == ERROR){  
					DBG_ERROR("Unable to spawn PM task!!\n");
				}
				else {
					DBG_TRACE("Spawned PM Task successfully!\n");
				}
			}
            break;
    }
    DBG_LEAVE();
    return 0;
}
/***************************/
/*
 * pm_timer_handler
 */
void 
pm_timer_handler(int arg)
{
    DBG_FUNC("pm_timer_handler",PM_LAYER);
    DBG_ENTER();
    
    STATUS status;
    
    if ((status = semGive(pmSemId)) != OK) 
    {
       DBG_ERROR("Error on semGive in PM_TASK! Status(%d)", status);
    }
    DBG_LEAVE();
}


/*
 * pm_task
 */
VOID
pm_task()
{
    DBG_FUNC("pm_task",PM_LAYER);
    DBG_ENTER();

    struct tm *current_time;
    time_t long_time;
    unsigned long current_seconds_passed_in_hour;
    STATUS status;

    time(&long_time);
    current_time = localtime(&long_time);

    current_seconds_passed_in_hour = (current_time->tm_min * 60) + current_time->tm_sec;

	DBG_TRACE("Current secs in hour %lu ; current minute %d; current secs %d\n",
    	current_seconds_passed_in_hour,current_time->tm_min,current_time->tm_sec);

    if ( (current_seconds_passed_in_hour >= 0) && (current_seconds_passed_in_hour < PM_SECONDS_IN_15_MIN) ) {
           reqd_interval_in_secs = (PM_SECONDS_IN_15_MIN - current_seconds_passed_in_hour);
    }
    else if ( (current_seconds_passed_in_hour >= PM_SECONDS_IN_15_MIN) && (current_seconds_passed_in_hour < PM_SECONDS_IN_30_MIN) ) {
           reqd_interval_in_secs = (PM_SECONDS_IN_30_MIN - current_seconds_passed_in_hour);
    }
    else if ( (current_seconds_passed_in_hour >= PM_SECONDS_IN_30_MIN) && (current_seconds_passed_in_hour < PM_SECONDS_IN_45_MIN) ) {
           reqd_interval_in_secs = (PM_SECONDS_IN_45_MIN - current_seconds_passed_in_hour);
    }
    else {
           reqd_interval_in_secs = (PM_SECONDS_IN_HOUR - current_seconds_passed_in_hour);
    }

    g_pm_timer              = wdCreate();
    g_timer_ticks_required  = reqd_interval_in_secs * sysClkRateGet();
	DBG_TRACE("Timer ticks required %lu\n",g_timer_ticks_required);

    initialized = false;

    pmSemId = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
    if (pmSemId == NULL)
    {
          DBG_ERROR("Error creating semaphore for PM_TASK!");
    }

    if(wdStart(g_pm_timer, g_timer_ticks_required, (FUNCPTR)pm_timer_handler, 0)){
          DBG_ERROR("Error creating timer for PM_TASK!");
    }

    /*
    * need to reset all counters for the current interval
    * including reset the current interval to 0
    */
    pm_reset_all_counters();

    /*
    * Since we do not update the MIB, the MIB will still contain the old values
    * (likley the old value before last reboot)
    */

    /*
    * First refresh the MIB values
    pm_update_mib();
    */

    pmTaskObj.JCTaskEnterLoop();   

    while(1)
    {
        if ( (status = semTake(pmSemId, WAIT_FOREVER)) == ERROR)
        {
             DBG_ERROR("Error on semTake in PM_TASK! Status(%d)", status);
        }

        /*
         * update the mib first
         */
        pm_update_mib();    
	pm_sendDataToAms();

	/* 
	 * advance to the next interval
	 */
	pm_advance_interval();

        /*
         *  Now we need to do it every 15 minutes
         */
        reqd_interval_in_secs = PM_SECONDS_IN_15_MIN;
        g_timer_ticks_required  = reqd_interval_in_secs * sysClkRateGet();

		DBG_TRACE("Timer ticks required %lu\n",g_timer_ticks_required);

 	       
	if(wdStart(g_pm_timer, g_timer_ticks_required, (FUNCPTR)pm_timer_handler, 0)){
             DBG_ERROR("Error creating timer for PM_TASK!");
        } 
      
    }
    
    pmTaskObj.JCTaskNormExit();   

    DBG_LEAVE();
    return;
}


/*-----------------------------------------------------------------------*/


static struct sockaddr_in addrTo;
static OS_SPECIFIC_SOCKET_HANDLE sock;

bool
initSocketAndMsgHdr()
{
	DBG_FUNC("initSocketAndMsgHdr", PM_LAYER);
	DBG_ENTER();

	if (!initialized)
	{
		int ip;
		/* 1.  prepare udp socket */
		sock = SOCKET_UdpCreate();

		if (sock == ERROR)
			return false;

		if (SOCKET_UdpBind(sock, (ubyte2) kANY_PORT) != true)
		{
			SOCKET_Close(sock);
			return false;
		}
        
        /* 2. get IP address from MIB */

        if (oam_getMibIntVar(MIB_h323_GKIPAddress, (long *) &ip) != STATUS_OK)
        {
            DBG_ERROR("Unable to get alarm server's IP from the MIB\n");
            return false;
		}

		/* 3. prepare the the server's address */
		memset (&addrTo, 0, sizeof(sockaddr_in));

		addrTo.sin_family = AF_INET;
		addrTo.sin_addr.s_addr = ip;
		addrTo.sin_port = htons((ubyte2)PM_SERVER_PORT);

        initialized = true;
    }
	DBG_LEAVE();
	return true;
}

/*-----------------------------------------------------------------------*/

bool
pm_sendDataToAms()
{

	DBG_FUNC("pm_sendDataToAms", PM_LAYER);
	DBG_ENTER();

	if (!initSocketAndMsgHdr())
	{
		DBG_LEAVE();
		return false;
	}

	//memset(&amsMsg, 0, sizeof(PmSourceToAmsMsg));

	amsMsg.hdr.alarmSrcType = srcTypViperCell;
	amsMsg.hdr.alarmMsgType = PM_msgTypData;
	amsMsg.hdr.alarmMsgLen = sizeof(PmData);

	/* Get local host IP address and timestamp */
	time_t ltime;
	time(&ltime);
	amsMsg.body.pmData.pmSrcIp = GetLocalHostAddress();
    //amsMsg.body.pmData.pmTimestamp = ltime;
	amsMsg.body.pmData.pmTimestamp = htonl(ltime); //convert to network byte order

    SOCKET_UdpSendTo(sock, &addrTo, (char *) &amsMsg, PM_msgLen);
    DBG_LEAVE();
    return true;
}


