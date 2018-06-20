// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.0
// File        : simulator.cpp
// Author(s)   : Igal Gutkin
// Create Date : 08/28/01
// Description : Cdr Client simulator 
//
// *******************************************************************

#include "stdafx.h"
#include "Os\JCCTimer.h"
#include "Os\JCMsgQueue.h"
#include "Os\JCTask.h"

#include "CdrModule.h"

#define CDR_DEFAULT_PERIOD  10
#define CDR_SIM_PIPE        "/pipe/cdrSimPipe"

#ifdef __cplusplus
 extern "C" 
 {
    bool _CdrInitTest     ();
    void _CdrCleanTest    ();

    bool _CdrStartTest    (unsigned period = 0, unsigned num = 0);
    void _CdrStopTest     ();

    void _CdrSimCallback  ();
    bool _CdrSimHandle    ();

    void _CdrTestStatus   ();
    void _CdrSimHelp      ();

 }
#endif 

// CDR message resend timer
static JCCTimer   *pCdrSimTimer;
static JCMsgQueue *pCdrSimQueue;
static JCTask     *pCdrSimTask ;

static unsigned uTime      = CDR_DEFAULT_PERIOD;

static int      iMaxTimes  = 0,
                iCount     = 0;

static bool     bCdrSimSet    = false,
                bCdrTestStart = false;

//===================================================================

void _CdrSimHelp ()
{
    printf ("Cdr Client Simulator Commands:\n\
    _CdrTestStatus - print simulation status\n\
    _CdrInitTest   - initialize simulator\n\
    _CdrCleanTest  - remove simulator\n\
    _CdrStartTest  - begin simulation\n\
        Parameters : (period [in 100 mS], number of messages [0 - unlimited]\n\
    _CdrStopTest   - end simulation\n");
}


bool _CdrInitTest ()
{
  bool retVal = false;

    if (bCdrSimSet)
    {
        printf ("CDR simulator is initialized already\n");
        return (true);
    }

    if ((pCdrSimTimer = new JCCTimer((FUNCPTR)_CdrSimCallback)) != NULL)
    {
        if ((pCdrSimQueue = new JCMsgQueue (200          ,  // max num of msgs 
                                           sizeof (int)  ,  // max size of a msg
                                           JC_MSG_Q_FIFO ))   // message queue options
                          != NULL)
        { // create and spawn simulator task
            if (pCdrSimTask = new JCTask ("tCdrSim"))
            {
                if (pCdrSimTask->JCTaskSpawn (100, 0, 5000          , 
                                              (FUNCPTR)_CdrSimHandle,
                                              0, 0, 0, 0, 0         , 
                                              0, 0, 0, 0, 0         ,
                                              CDR_MODULE_ID         ,
                                              JC_NON_CRITICAL_TASK  ) != ERROR)
                {
                    retVal     = true;
                    bCdrSimSet = true;
                }
            }
        }
    }

    if (!retVal)
    {
        printf ("Unable to start CDR Client Simulator\n");
        _CdrCleanTest ();
    }

  return (retVal);
}


void _CdrCleanTest ()
{
    // kill timer
    if (pCdrSimTimer)
    {
        _CdrStopTest ();
        delete pCdrSimTimer;
        pCdrSimTimer = NULL;
    }

    // kill task
    if (pCdrSimTask)
    {
      int tId = pCdrSimTask->GetTaskId();

        taskSuspend (tId);
        taskDelete  (tId);
        delete pCdrSimTask;
        pCdrSimTask = NULL;
    }

    // kill queue
    if (pCdrSimQueue)
    {
        delete pCdrSimQueue;
        pCdrSimQueue = NULL;
    }

    bCdrSimSet = false;
}

// use resolution of 100 mSec
// number of attempts = 0 ==> unlimited till stop is called
bool _CdrStartTest (unsigned period, unsigned num)
{
  bool retVal = false;

    if (bCdrSimSet || (retVal=_CdrInitTest()) )
    {
        if (!bCdrTestStart)
        {
            uTime     = (period) ? period : CDR_DEFAULT_PERIOD;
            iMaxTimes = (int)num;
        
            pCdrSimTimer->setTimerMSec (period);
            bCdrTestStart = true;
            retVal        = true;
            printf ("CDR Client simulation begin\n");
        }
        else
            printf ("CDR Client simulator is already started\n");
    }

  return (retVal);
}


void _CdrStopTest  ()
{
    if (bCdrSimSet)
    {
        pCdrSimTimer->cancelTimer();

        iMaxTimes     = 0;
        iCount        = 0;
        bCdrTestStart = false             ;
        uTime         = CDR_DEFAULT_PERIOD;
    }
}


void _CdrTestStatus ()
{
    if (bCdrSimSet)
    {
        printf ("CDR Client Simulator is %s\n", ((bCdrTestStart) ? "running" : "stoped") );
    }
    else
        printf ("CDR Client Simulator is not initialized\n");
}


//===================================================================

// simulator task handler
bool _CdrSimHandle ()
{
  bool         keepGoing = true;
  CdrMessage_t msg                 ;
  sbyte        szHostName[MAX_PATH];
  ubyte4       uHostIp = 0;
  struct tm    *currTime  ;
  time_t       lTime      ;

  JCMsgQueue  *pReplayQ   ;
  unsigned     inMsgType  ,
               inMsgSize  ,
               inMsgBuf   ;

  MNET_MODULE_ID fromModId;

    // Spin here forever waiting for simulator activations
    while (keepGoing)
    {
        if (pCdrSimQueue->JCMsgQReceive (&pReplayQ        ,
                                         &inMsgType       ,
                                         &fromModId       ,
                                         (char *)&inMsgBuf,
                                         &inMsgSize       , 
                                         WAIT_FOREVER     ) != ERROR)
        {
            if (!uHostIp)
            {
                msg.msgType = CDR_GENERIC_MSG_TYPE;
            
                // get a local host name
                if (gethostname (szHostName, sizeof(szHostName)) != SOC_STATUS_ERROR)
                {
                    uHostIp = htonl (cdrGetLocalIp());
                }
                else
                    *szHostName = '\0';
            }

            /* Obtain coordinated universal time: */
            time (&lTime);
            currTime = gmtime (&lTime);
            sprintf ((char *)msg.record, "Test log number %u [ref. %u] from a virtual client on host %s [%s], %s",
                     iCount, cdrAllocateRef(), szHostName, GenSocket::getIp(uHostIp), asctime(currTime)          );

            cdrSendLog (&msg);
        }
    }
        
  return (true);
}


// timer ISR call back
void _CdrSimCallback ()
{
  static CdrMessage_t msg                 ;
  static sbyte        szHostName[MAX_PATH];
  static ubyte4       uHostIp = 0;

  struct tm          *currTime;
         time_t       lTime   ;

    if (!iMaxTimes || iCount < iMaxTimes)
    {
        ++iCount;
        
        pCdrSimQueue->JCMsgQSend (pCdrSimQueue, 0, CDR_MODULE_ID, (char *)&iCount, sizeof iCount,
                                  JC_NO_WAIT, JC_MSG_PRI_NORMAL);

        pCdrSimTimer->setTimerMSec (uTime, iCount);
    }
    else
    {
        _CdrStopTest ();
    }

}
