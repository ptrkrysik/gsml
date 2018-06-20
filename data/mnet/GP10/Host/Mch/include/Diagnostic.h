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
#ifndef Diagnostic_H
#define Diagnostic_H

#include <lstLib.h>
#include <timers.h>
#include <msgQLib.h>
#include "MCH/MCHIntf.h"

#define CONTINUE_TEST   0
#define STOP_TEST       1

#define DIAG_GROUP_NUM_BITS    8
#define DIAG_DIAGS_PER_GROUP   (1<<DIAG_GROUP_NUM_BITS)
#define DIAG_TEST_NUM_MASK     (DIAG_DIAGS_PER_GROUP - 1)

enum DIAG_TEST_BUMBERS {
   DIAG_CDC_DIAGNOSTICS        = 0x000,
   DIAG_HPI_MEMORY_TEST,
   DIAG_I2C_LOOPBACK_TEST,
   DIAG_DSP_MEM_TEST,
   DIAG_CF_TEST,
   DIAG_FPGA_TEST,
   DIAG_I2C_TEST,
   
   DIAG_DSP_DIAGNOSTICS        = 0x100,
   DIAG_DSP_LOOPBACK,
   DIAG_DSP_MS_REPORT,
   DIAG_DSP_HO_REPORT,
   DIAG_DSP_DL_REPORT,
   DIAG_DSP_HPI_ECHO_TEST, 
   DIAG_DSP_BBLOOPBACK,
   DIAG_DSP_EXTMEM,
   DIAG_DSP_IF_REPORT, 
   DIAG_DSP_GPRS_MS_REPORT,
   
   DIAG_RF_DIAGNOSTICS         = 0x200,
   
   DIAG_CLK_DIAGNOSTICS        = 0x300,
   
   DIAG_GPS_DIAGNOSTICS        = 0x400,
   
   DIAG_SYSTEM_DIAGNOSTICS     = 0x500,
   DIAG_SYSTEM_NETWORK_TEST_SERVER,
   DIAG_SYSTEM_NETWORK_TEST_TRANSMIT,
   
   DIAG_LAST_DIAG_GROUP        = 0x600
};

extern char **DiagNames[DIAG_LAST_DIAG_GROUP>>DIAG_GROUP_NUM_BITS];
extern char *CDCDiagNames[DIAG_DIAGS_PER_GROUP];
extern char *DSPDiagNames[DIAG_DIAGS_PER_GROUP];
extern char *RFDiagNames[DIAG_DIAGS_PER_GROUP];
extern char *CLKDiagNames[DIAG_DIAGS_PER_GROUP];
extern char *GPSDiagNames[DIAG_DIAGS_PER_GROUP];
extern char *SystemDiagNames[DIAG_DIAGS_PER_GROUP];

#define DIAG_TEST_MAX_MSGS                 100
#define DIAGNOSTIC_TEST_TASK_PRI           200
#define DIAGNOSTIC_TEST_STACK_SIZE         10000


// *******************************************************************
// Diagnostic
// *******************************************************************


class Diagnostic : public NODE
{
public:

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
   Diagnostic(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   virtual ~Diagnostic();



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
   int Diagnostic::RunDiagnostic(void);
   
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: Diagnostic::GetDiagnosticHelp
   **
   **    PURPOSE: GetDiagnosticHelp must be overloaded by the derived diagnostic
   **      class.  The diagnostic should display useful help information.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): 
   **
   **----------------------------------------------------------------------------*/
   virtual char *GetDiagnosticHelp(void) = 0;   
   


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
   static void TimerExpired(timer_t timerId, int diagPtr);
   
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
   static void DiagnosticMsgLoop(int argThis);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: Diagnostic::isAMatch
   **
   **    PURPOSE: Test if the diagnostic matches the specified object_instance
   **      and test number.
   **
   **    INPUT PARAMETERS: testNum - test number
   **
   **    RETURN VALUE(S): 0 - if NOT a match
   **                     1 - if a match
   **
   **----------------------------------------------------------------------------*/
   char isAMatch(int testNum);
   
   

   MSG_Q_ID diagQId;
    
protected:

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
   void SetTimer(UINT seconds);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DiagnosticManager::SendResponse
   **
   **    PURPOSE: Send the string back to the desired source.
   **
   **    INPUT PARAMETERS: resp - string to send
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   void SendResponse(const char *resp);
   
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
   void SendResultData(char *format, ...);
   
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
   void InternalStopTest(void);
   
        

   timer_t durationTimer;
   int isTimerCreated;
   char instanceNum;               
   char *diagName;
   int testNumber;
   ReqSourceType reqSrc; 
   char parameters[MCH_MAX_COMMAND_LINE_LEN];                
    
private:
   
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
   void SendDeleteMsg(void);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: Diagnostic::InitiateDiagnostic
   **
   **    PURPOSE: InitiateDiagnostic must be overloaded by the derived diagnostic
   **      class.  This is where the test is initiated.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S):  OK if able to initiate diagnostic or ERROR if failed
   **
   **----------------------------------------------------------------------------*/
   virtual int InitiateDiagnostic(void) = 0;   

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: Diagnostic::TerminateDiagnostic
   **
   **    PURPOSE: TerminateDiagnostic must be overloaded by the derived diagnostic
   **      class.  This is where cleanup of the diagnostic should take place.  Also
   **      test reporting should occur here.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S):  OK if able to terminate diagnostic or ERROR if failed
   **
   **----------------------------------------------------------------------------*/
   virtual int TerminateDiagnostic(void) = 0;

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
   virtual int ProcessMessage(DiagnosticMsgType diagMsg);


};
#endif Diagnostic_H
