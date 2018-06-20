// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : Diagnostic.h
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description : 
//
// *******************************************************************

#include <taskLib.h>
#include <stdio.h>
#include <string.h>
#include "Diagnostic.h"
#include "MCHInit.h"
#include "logging/vclogging.h"
#include "logging/vcmodules.h"
#include "oam_api.h"
#include "TcpServer/TcpSrvApi.h"

// *******************************************************************
// Diagnostic
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Diagnostic constructor
**
**    PURPOSE: Initialize paramters for diagnostic base class.
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
Diagnostic::Diagnostic(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : diagQId(0), instanceNum(0), reqSrc(reqSrc), testNumber(testNum),
   isTimerCreated(FALSE)
{
   if (parms)
   {
       strncpy (parameters, parms, MCH_MAX_COMMAND_LINE_LEN);
   }
   
   if (testName)
   {
       diagName = new char[strlen(testName) + 1];
       strcpy (diagName, testName);
   }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Diagnostic destructor
**
**    PURPOSE: Delete any data members allocated from free store.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
Diagnostic::~Diagnostic()
{
   delete [] diagName;
   
   // Delete timer.
   if (isTimerCreated) 
   {
      timer_delete(durationTimer);
   }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Diagnostic::SendDeleteMsg
**
**    PURPOSE: Send a DIAG_DELETE_DIAGNOSTIC message to the MCHTask.
**      This message causes the MCHTask to remove the diagnostic from
**      the list of active diagnostics and to delete this instance of the
**      diagnostic.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void Diagnostic::SendDeleteMsg(void)
{
	DBG_FUNC("Diagnostic::SendDeleteMsg",DIAG_TEST);
	DBG_ENTER();

   int status;
   MCHMessageType delDiagMsg;

   // Initialize the delete diagnostic message.
   delDiagMsg.func = MCH_DELETE_DIAGNOSTIC;
   delDiagMsg.delDiagMsg.diag = this;

   // Send delete diagnostic message to the diagnostic manager.
   status = msgQSend(MCHMsgQId, 
                     (char *)&delDiagMsg, 
                     sizeof(MCHMessageType), 
                     WAIT_FOREVER, 
                     MSG_PRI_NORMAL);
                   
   if (status != OK)
   {
       DBG_WARNING("unable to send message %x\n", errno);
   }
   DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DiagnosticManager::InternalStopTest
**
**    PURPOSE: Diagnostics which determine their own completion must call this
**       function to terminate the test.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void Diagnostic::InternalStopTest(void)
{
	DBG_FUNC("Diagnostic::InternalStopTest",DIAG_TEST);
	DBG_ENTER();
   
   int status;
   DiagnosticMsgType diagMsg;

   // Initialize the delete diagnostic message.
   diagMsg.func = DIAG_TEST_INTERNAL_STOP_TEST;

   // Send internal stop diagnostic message to self.
   status = msgQSend(diagQId, 
                     (char *)&diagMsg, 
                     sizeof(DiagnosticMsgType), 
                     WAIT_FOREVER, 
                     MSG_PRI_NORMAL);
                   
   if (status != OK)
   {
       DBG_WARNING("unable to send message %x\n", errno);
   }
   
   DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DiagnosticManager::SendResultData
**
**    PURPOSE: Send result data back to the original test requester.  This
**       can be intermediate data or a final result.
**
**    INPUT PARAMETERS: format - format string as if a printf
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void Diagnostic::SendResultData(char *format, ...)
{
	DBG_FUNC("Diagnostic::SendResultData",DIAG_TEST);
	DBG_ENTER();
   
   va_list ap;
   va_start(ap, format);
   
   switch (reqSrc)
   {
      case NETWORK_REQ:
      {
         // Break down the result data into null terminated strings that are
         // of the size expected by OAM.
         char buf[2048];
         
         // Put result string into a buffer.
         vsprintf(buf, format, ap);
                  
         // Send back to the network using the TCP server.
         if (TcpSrvSendResponse(buf, strlen(buf), kMchGpName) != TRUE)
         {
            DBG_WARNING("Diagnostic: Unable to send response to TCP server\n");
         }
      }
      break;
      case TERMINAL_REQ:
      {
         vprintf(format, ap);
      }
      break;
   }
   
   va_end(ap);
   
   DBG_LEAVE();
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Diagnostic::TimerExpired
**
**    PURPOSE: Duration timer has expired so send a message to the diagnostic to
**      terminate the diagnostic.
**
**    INPUT PARAMETERS: timerId - Id of the duration timer for this diagnostic
**                      diagPtr - pointer to instance of diagnostic (this pointer)
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void Diagnostic::TimerExpired(timer_t timerId, int diagPtr)
{
	DBG_FUNC("Diagnostic::TimerExpired",DIAG_TEST);
	DBG_ENTER();

   // Cast the integer paramter to be a pointer to the instance of diagnostic.
   Diagnostic * myThis = (Diagnostic *)diagPtr;

   int rtnStatus;
   DiagnosticMsgType diagMsg;
   diagMsg.func = DIAG_TEST_INTERNAL_STOP_TEST;

   // Send a message to the diagnostic to end the current test.
   rtnStatus = msgQSend(myThis->diagQId, 
                     (char *)&diagMsg, 
                     sizeof(DiagnosticMsgType), 
                     WAIT_FOREVER, 
                     MSG_PRI_NORMAL);
   if (rtnStatus != OK)
   {
      DBG_WARNING("unable to send message %d", rtnStatus);
   }
   DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Diagnostic::SetTimer
**
**    PURPOSE: Connect a function to the duration timer and set the seconds value
**      to the specified number of seconds.
**
**    INPUT PARAMETERS: seconds - number of seconds to set the duration timer
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void Diagnostic::SetTimer(UINT seconds)
{
	DBG_FUNC("Diagnostic::SetTimer",DIAG_TEST);
	DBG_ENTER();
   
   if (!isTimerCreated)
   {
      // Create a timer for use by the diagnostic.
      if (timer_create(CLOCK_REALTIME, NULL, &durationTimer) == -1)
      {
         DBG_WARNING("Unable to create duration timer %d", errno);
      }
      else
      {
         isTimerCreated = TRUE;
      }
   }
   
   if (timer_connect(durationTimer, (VOIDFUNCPTR)Diagnostic::TimerExpired, (int)this) == -1)
   {
       DBG_WARNING("error connecting to duration timer");
   }
       
   itimerspec timeout;
   timeout.it_interval.tv_nsec = 0;
   timeout.it_interval.tv_sec = 0;
   timeout.it_value.tv_nsec = 0;
   timeout.it_value.tv_sec = seconds;
       
   if (timer_settime(durationTimer, CLOCK_REALTIME, &timeout, NULL) == -1)
   {
      DBG_WARNING("error setting to duration timer");
   }
   DBG_LEAVE();
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Diagnostic::RunDiagnostic
**
**    PURPOSE: RunDiagnostic initializes the necessary resources to run the
**      specified diagnostic.  A queue and a task are created to
**      be able to run the diagnostic and respond to messages.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S):  an indication of whether the test was initiated
**
**----------------------------------------------------------------------------*/
int Diagnostic::RunDiagnostic(void)
{
	DBG_FUNC("Diagnostic::RunDiagnostic",DIAG_TEST);
	DBG_ENTER();

   int rtnStatus; 

   // Create a message queue for this diagnostic.  
   diagQId = msgQCreate( 
       DIAG_TEST_MAX_MSGS,
       sizeof(DiagnosticMsgType),
       MSG_Q_PRIORITY);
       
   if ( !diagQId )
   {
      DBG_WARNING("Unable to create Queue %d", errno);
      DBG_LEAVE();
      return ERROR;
   }

   // Spawn a task to run this diagnostic.  The function called is a static function
   // of diagnostic.  Since the object needs an instance to access data
   // members and non-static functions, the this pointer is passed as an argument.
   rtnStatus = taskSpawn(diagName, DIAGNOSTIC_TEST_TASK_PRI, 0,
             DIAGNOSTIC_TEST_STACK_SIZE, 
             (FUNCPTR) Diagnostic::DiagnosticMsgLoop,
             (int)this, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

   if ( rtnStatus == ERROR )
   {
      DBG_WARNING("Unable to create Task %d", rtnStatus);
      msgQDelete(diagQId);
      DBG_LEAVE();
      return ERROR;
   }

   DBG_LEAVE();
   return OK;
}




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Diagnostic::DiagnosticMsgLoop
**
**    PURPOSE: Execute the diagnostic.  After initiating the execution of the
**      diagnostic, loop forever and wait for messages.  Valid messages are
**      DIAG_TEST_STOP_TEST, DIAG_TEST_SEND_TEST_REPORT and
**      DIAG_TEST_INTERNAL_STOP_TEST.
**
**      The flow is like this:
**          call InitiateDiagnostic (overloaded by derived class)
**          Loop for messages
**              process common messages
**              call ProcessMessage (overloaded by derived class)
**          call TerminateDiagnostic (overloaded by derived class) 
**          send DELETE_DIAGNOSTIC message          
**          delete message queue
**          end task
**
**      InitiateDiagnostic - should be overloaded by diagnostic to start diagnostic
**      ProcessMessage - should be overloaded only if derived diagnostic test needs
**          to process messages specific to itself.
**      TerminateDiagnostic - should be overloaded to handle test cleanup and test
**          report generation if appropriate.
**
**    INPUT PARAMETERS: argThis - pointer to the instance of diagnostic
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void Diagnostic::DiagnosticMsgLoop(int argThis)
{
	DBG_FUNC("Diagnostic::DiagnosticMsgLoop",DIAG_TEST);
	DBG_ENTER();
   
   // Cast the integer parameter to be a pointer to the instance of diagnostic.
   Diagnostic * myThis = (Diagnostic *)argThis;


   // Begin execution of the diagnostic.  If something goes wrong then send a
   // NACK back to the requester and delete this instance of diagnostic.
   if (myThis->InitiateDiagnostic() != OK)
   {
      DBG_WARNING("Unable to enable diagnostic");
      DBG_LEAVE();
      // Let the MCH task know to delete this instance of diag.
      myThis->SendDeleteMsg();
      msgQDelete(myThis->diagQId);
      DBG_LEAVE();
      return;
   }


   // Loop here for messages: DIAG_STOP_TEST.
   int rtnStatus;

   DiagnosticMsgType diagMsg;


   int done = FALSE;
   while (!done)
   {
      rtnStatus = msgQReceive( 
         myThis->diagQId, 
         (char *)&diagMsg, 
         sizeof(DiagnosticMsgType),
         WAIT_FOREVER );

      if ( rtnStatus == ERROR )
      {
         DBG_WARNING("error from sw_msgq_receive %d\n", errno);
         break;
      }

      // Based on the receive function perform desired action. 
      switch ( diagMsg.func )
      {
         case DIAG_TEST_STOP_TEST:
         {
            done = TRUE;
            break;
         }

         case DIAG_TEST_INTERNAL_STOP_TEST:
         {
            done = TRUE;
            break;
         }

         default:
         {
            done = myThis->ProcessMessage(diagMsg);
         }
      }  /***  switch ( RcvFunc )  ***/
   }  /***  task loop  ***/


   // Terminate diagnostic
   if (myThis->TerminateDiagnostic() != OK)
   {
       DBG_WARNING("Unable to disable diagnostic");
   }

   // Let the MCH task know to delete this instance of diag.
   myThis->SendDeleteMsg();
   
   // Delete the message queue.
   msgQDelete(myThis->diagQId);
   DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Diagnostic::ProcessMessage
**
**    PURPOSE: ProcessMessage should be overloaded by a derived diagnostic to
**      process any messages specific to that diagostic.  
**
**    INPUT PARAMETERS: diagMsg - message sent to diagnostic
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int Diagnostic::ProcessMessage(DiagnosticMsgType diagMsg)
{
	DBG_FUNC("Diagnostic::ProcessMessage",DIAG_TEST);
	DBG_ENTER();
   

   DBG_LEAVE();
   return CONTINUE_TEST;  
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Diagnostic::isAMatch
**
**    PURPOSE: Test if the diagnostic matches the specified test number.
**
**    INPUT PARAMETERS: testNum - test number
**
**    RETURN VALUE(S): 0 - if NOT a match
**                     1 - if a match
**
**----------------------------------------------------------------------------*/
char Diagnostic::isAMatch(int testNum)
{
   return  (testNum==testNumber);
}



char **DiagNames[DIAG_LAST_DIAG_GROUP>>DIAG_GROUP_NUM_BITS] = 
{
   CDCDiagNames,
   DSPDiagNames,
   RFDiagNames,
   CLKDiagNames,
   GPSDiagNames,
   SystemDiagNames
};

char *CDCDiagNames[DIAG_DIAGS_PER_GROUP] =
{
   "CDC Diagnostics",
   "HPIMemoryAccess",
   "I2CLoopback",
   "DSPMemTest",
   "CFTest",
   "FPGATest",
   "I2CTest"
};

char *DSPDiagNames[DIAG_DIAGS_PER_GROUP] =
{
   "DSP Diagnostics",
   "DSPLoopback",
   "MSReport",
   "HOReport",
   "DLReport",
   "HPIEchoTest",
   "DSPBBLBTest",
   "DSPExtMemTest",
   "IFReport",
   "GPRSMsReport"
};

char *RFDiagNames[DIAG_DIAGS_PER_GROUP] =
{
   "RF Diagnostics",
};

char *CLKDiagNames[DIAG_DIAGS_PER_GROUP] =
{
   "CLK Diagnostics",
};

char *GPSDiagNames[DIAG_DIAGS_PER_GROUP] =
{
   "GPS Diagnostics",
};


char *SystemDiagNames[DIAG_DIAGS_PER_GROUP] =
{
   "System Diagnostics",
   "NetworkTestServer",
   "NetworkTestTransmit"
};
