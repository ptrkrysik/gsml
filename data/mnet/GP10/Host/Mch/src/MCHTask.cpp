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
// File        : MCHTask.cpp
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description :  
//
// *******************************************************************
#include <ctype.h>
#include <stdio.h>

#include "MCHTask.h"
#include "MCH/MCHIntf.h"
#include "MCHInit.h"
#include "MCHConfig.h"
#include "MCHCommand.h"
#include "CdcDiags.h"
#include "DspDiags.h"
#include "SysDiags.h"
#include "logging/vclogging.h"
#include "logging/vcmodules.h"
#include "MnetModuleId.h"
#include "TcpServer/TcpSrvApi.h"
#include "GP10OsTune.h"


// Flag used to indicate that the one and only maintenance command handler
// has been created.
BOOL MCHTask::isOneCreated = FALSE;


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHTask::MCHTask
**
**    PURPOSE: MCHTask constructor.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
MCHTask::MCHTask() : MCHTaskObj(MCH_TASK_NAME)
{
   // Create MCHCommand objects for each of the known maintenance commands.
   mchCommands[MCH_START_DIAG] = new MCHStartDiag("STARTDIAG", "STTD");
   mchCommands[MCH_STOP_DIAG] = new MCHStopDiag("STOPDIAG", "STPD");
   mchCommands[MCH_GET_RESULT] = new MCHGetDiagResult("GETDIAGRESULT", "GDR");
   mchCommands[MCH_GET_DIAG_HELP] = new MCHGetDiagHelp("GETDIAGHELP", "GDH");
   mchCommands[MCH_HELP] = new MCHHelp("MCHHELP", "HELP");

   // Initialize the list of active diagnostice (begins empty).
   lstInit(&activeDiags);
   
   // Initialize the array of diagnostic creation routines.
   AllDiagnostics[DIAG_CDC_DIAGNOSTICS >> DIAG_GROUP_NUM_BITS] = cdcDiags;
   AllDiagnostics[DIAG_DSP_DIAGNOSTICS >> DIAG_GROUP_NUM_BITS] = dspDiags;
   AllDiagnostics[DIAG_RF_DIAGNOSTICS >> DIAG_GROUP_NUM_BITS] = rfDiags;
   AllDiagnostics[DIAG_CLK_DIAGNOSTICS >> DIAG_GROUP_NUM_BITS] = clkDiags;
   AllDiagnostics[DIAG_GPS_DIAGNOSTICS >> DIAG_GROUP_NUM_BITS] = gpsDiags;
   AllDiagnostics[DIAG_SYSTEM_DIAGNOSTICS >> DIAG_GROUP_NUM_BITS] = systemDiags;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: SysCommand_MCH
**
**    PURPOSE: SysCommand_MCH is called anytime the system needs to notify
**       a module about a system event.  
**
**    INPUT PARAMETERS: action - type of system command
**
**    RETURN VALUE(S): OK
**
**----------------------------------------------------------------------------*/
int SysCommand_MCH(T_SYS_CMD action)
{
   DBG_FUNC("MCHTask::InitMCH",MAINT_COMM_HDLR);
   DBG_ENTER();
   
   switch(action){
      case SYS_SHUTDOWN:
         DBG_TRACE("[MCH] Received system shutdown notification\n");
         break;
      case SYS_START:
         DBG_TRACE("[MCH] Received system start notification\n");
         if (MCHTask::InitMCH() != OK)
         {
            DBG_ERROR("Unable to initialize MCH properly\n");
         }
         break;
         
      default:
         DBG_TRACE("[MCH] Unknown system command received\n");
   }
   
   DBG_LEAVE();
   return OK;
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHTask::InitMCH
**
**    PURPOSE: InitMCH function provides the basic initialization for the
**      maintenance command handler in the BTS.  A single instance of MCHTask
**      is created.  The single instance is referenced by static member pointer 
**      theMCHTask.
**
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/

int MCHTask::InitMCH(void)
{
   DBG_FUNC("MCHTask::InitMCH",MAINT_COMM_HDLR);
   DBG_ENTER();

   // Create the Maintenance Command Handler message queue.
   MCHMsgQId = msgQCreate(MCH_MAX_MSGS, // number of messages
      MCH_MAX_MSG_LENGTH,                 // size of each message
      MSG_Q_PRIORITY                  // priority of the queue
      );


   // check message queue creation result
   if(MCHMsgQId == NULL)
   {
      // message queue creation failure. No point to continue. 
      DBG_LEAVE();
      return ERROR;
   }

   // Create the one and only MCHTask object.
   if (isOneCreated == FALSE)
   {
      theMCHTask = new MCHTask();
      isOneCreated = TRUE;
   }

   if (theMCHTask->MCHTaskObj.JCTaskSpawn(MCH_TASK_PRIORITY,
             MCH_TASK_OPTION,
             MCH_TASK_STACK_SIZE, 
             (FUNCPTR) MCHTask::MCHLoop,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
             MODULE_MCH, JC_NON_CRITICAL_TASK ) == ERROR)
   {
      DBG_ERROR("MCH: Unable to spawn task\n");
      DBG_LEAVE();
      return ERROR;
   }
   
   DBG_LEAVE();
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: CallMCHLoop
**
**    PURPOSE: This is a wrapper function used to call MCHLoop.  This is needed
**     so that a simple extern declaration is sufficient to call MCHLoop.  If
**     MCHLoop were call directly the entire MCHTask class would need to be
**     included in the root task scope.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int CallMCHLoop(void)
{
   return MCHTask::MCHLoop();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHTask::MCHLoop
**
**    PURPOSE: MCHLoop is the main message processing loop for the maintenance
**      command handler.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int MCHTask::MCHLoop( void )
{
   DBG_FUNC("MCHTask::MCHLoop",MAINT_COMM_HDLR);
   DBG_ENTER();
   
   ReqSourceType reqSrc;
   MCHMessageType rcvMsg;
   MCHCommand *pMCHComm;
   int rtnStat;
   char commandName[MCH_MAX_COMMAND_LEN], *commandParms;

   // Register with TCP server.
   if (TcpSrvRegHand(kMchGpName, MaintCmdTcpServer) != TRUE)
   {
      DBG_WARNING("MCH: Unable to register with TCP server\n");
   }

   // Maintenance Command Handler main loop.
   theMCHTask->MCHTaskObj.JCTaskEnterLoop();
   while (TRUE)
   {
      if ((rtnStat = msgQReceive(MCHMsgQId,
         (char *) &rcvMsg, 
         MCH_MAX_MSG_LENGTH, 
         WAIT_FOREVER)) == ERROR)
      {
         DBG_WARNING("Failed msgQReceive! error %x\n", rtnStat);
      }

      switch (rcvMsg.func)
      {
         case MCH_NETWORK_REQ :
         {
            reqSrc = NETWORK_REQ;
            sscanf(rcvMsg.commLine, "%s", commandName);
            
            commandParms = strstr(rcvMsg.commLine, commandName) + strlen(commandName) + 1;
            // Convert all letters in the request to upper case
            for (int i = 0; i <= strlen(commandName); i++)
            {
               commandName[i] = toupper(commandName[i]);
            }
         }
         break;

         case MCH_TERMINAL_REQ :
         {
            reqSrc = TERMINAL_REQ;
            
            sscanf(rcvMsg.commLine, "%s", commandName);
            
            commandParms = strstr(rcvMsg.commLine, commandName) + strlen(commandName) + 1;
            // Convert all letters in the request to upper case
            for (int i = 0; i <= strlen(commandName); i++)
            {
               commandName[i] = toupper(commandName[i]);
            }
         }
         break;
         
         case MCH_DELETE_DIAGNOSTIC :
         {
            // Remove diagnostic from the active list and delete the diag.
            lstDelete(&theMCHTask->activeDiags, rcvMsg.delDiagMsg.diag);
            delete rcvMsg.delDiagMsg.diag;
            continue;
         }
         break;
         
         case MCH_DIAG_SPECIFIC_MSG :
         {
            // This message is for an active diagnostic so send it off to
            // all active diagnostics.  The diagnostics that are not expecting
            // this message will just drop it.
            NODE *diag = lstFirst(&theMCHTask->activeDiags);
            while (diag)
            {
               if ((rtnStat = msgQSend (((Diagnostic *)diag)->diagQId,
                               (char *)&rcvMsg.diagMsg,
                               sizeof(DiagnosticMsgType),
                               NO_WAIT,
                               MSG_PRI_NORMAL)) == ERROR)
               {
                  DBG_WARNING ("Error sending message %x\n", rtnStat);
               }
               diag = lstNext(diag);
            }
            continue;
         }
         break;

         default :
            DBG_WARNING("Invalid message received %d\n", rcvMsg.func);
            continue;
      }

      // Loop through the known message types and execute it if found.
      for (int type = 0; type < MCH_MAX_COMMANDS; type++)
      {
         if (theMCHTask->mchCommands[type])
         {
            if (theMCHTask->mchCommands[type]->isCommand(commandName))
            {
               theMCHTask->mchCommands[type]->reqSrc = reqSrc;
               theMCHTask->mchCommands[type]->ClearCommandResponse();
               theMCHTask->mchCommands[type]->ExecuteCommand(commandParms);
               pMCHComm = theMCHTask->mchCommands[type];
               break;
            }
         }
      }
      
      // If a valid command was not entered then display help.
      if (type == MCH_MAX_COMMANDS)
      {
         theMCHTask->mchCommands[MCH_HELP]->ClearCommandResponse();
         theMCHTask->mchCommands[MCH_HELP]->CommandResponseAppend(
            "Unknown maintenance command %s\n", rcvMsg.commLine);
         theMCHTask->mchCommands[MCH_HELP]->ExecuteCommand(commandParms);
         pMCHComm = theMCHTask->mchCommands[MCH_HELP];
      }
     
      theMCHTask->SendResponse(pMCHComm->GetCommandResponse(), reqSrc);
   }
   theMCHTask->MCHTaskObj.JCTaskNormExit();
   DBG_LEAVE();
   return OK;
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DiagnosticManager::FindDiagnostic
**
**    PURPOSE: Search the list of currently active diagnostics given class_id,
**      object_instance, and test number.
**
**    INPUT PARAMETERS: testNum - test number of diagnostic
**
**    RETURN VALUE(S): Diagnostic * - pointer to the new diagnostic if found
**                                    0 if no diagnostic was found
**
**----------------------------------------------------------------------------*/
Diagnostic *MCHTask::FindDiagnostic(int testNum) 
{
   DBG_FUNC("MCHTask::FindDiagnostic",MAINT_COMM_HDLR);
   DBG_ENTER();

   // Now search the list of active diagnostics for this object class to see if
   // one exists with the given instance and test information.
   NODE *diag = lstFirst(&activeDiags);
   while (diag)
   {
      if (((Diagnostic *)diag)->isAMatch(testNum))
      {
         break;
      }
      diag = lstNext(diag);
   }
   DBG_LEAVE();
   return (Diagnostic *)diag;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DiagnosticManager::SendResponse
**
**    PURPOSE: Send the string back to the desired source.
**
**    INPUT PARAMETERS: resp - string to send
**                      reqSrc - who to respond to
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void MCHTask::SendResponse(const char *resp, ReqSourceType reqSrc)
{
   DBG_FUNC("MCHTask::SendResponse",MAINT_COMM_HDLR);
   DBG_ENTER();

   switch (reqSrc)
   {
      case NETWORK_REQ:
      {
         // Send back to the network using the TCP server.
         if (TcpSrvSendResponse(resp, strlen(resp), kMchGpName) != TRUE)
         {
            DBG_WARNING("MCH: Unable to send response to TCP server\n");
         }
      }
      break;
      case TERMINAL_REQ:
      {
         printf("%s", resp);
      }
      break;
   }
   DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: Create diagnostic functions.
**
**    PURPOSE: Each of the following functions are included in the table of functions
**      to create diagnostics.  Each function compares the requested diagnostic test
**      number with the known test numbers and creates an instance of the diagnostic 
**      if the test is valid.
**
**    INPUT PARAMETERS: testNum - test number.
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII.
**
**    RETURN VALUE(S): Diagnostic * - pointer to the new diagnostic if valid
**                                    0 if no diagnostic was created
**
**----------------------------------------------------------------------------*/
Diagnostic *cdcDiags(int testNum, ReqSourceType reqSrc, char *parms)
{
   switch (testNum) {
      case DIAG_HPI_MEMORY_TEST :
         return new HPIMemoryTest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_I2C_LOOPBACK_TEST :
         return new I2CLoopbackTest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;              
      case DIAG_DSP_MEM_TEST :
         return new DSPMemTest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_CF_TEST :
         return new CFTest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;    
      case DIAG_FPGA_TEST:
         return new FPGATest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_I2C_TEST:
         return new I2CTest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;
      default :
      {
         return 0;
      }
   }
} 



Diagnostic *dspDiags(int testNum, ReqSourceType reqSrc, char *parms)
{
   switch (testNum) {
      case DIAG_DSP_LOOPBACK :
         return new DSPLoopbackTest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_DSP_MS_REPORT :
         return new DSPMSReport(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_DSP_GPRS_MS_REPORT :
         return new DSPGPRSMSReport(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_DSP_HO_REPORT :
         return new DSPHOReport(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_DSP_DL_REPORT :
         return new DSPDLReport(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_DSP_HPI_ECHO_TEST :
         return new HPIEchoTest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_DSP_BBLOOPBACK :
         return new DSPBBLoopbackTest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_DSP_EXTMEM :
         return new DSPExtMemTest(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;
      case DIAG_DSP_IF_REPORT :
         return new DSPIFReport(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      default :
      {
         return 0;
      }
   }
} 


Diagnostic *rfDiags(int testNum, ReqSourceType reqSrc, char *parms)
{
   switch (testNum) {
      default :
      {
         return 0;
      }
   }
} 


Diagnostic *clkDiags(int testNum, ReqSourceType reqSrc, char *parms)
{
   switch (testNum) {
      default :
      {
         return 0;
      }
   }
} 


Diagnostic *gpsDiags(int testNum, ReqSourceType reqSrc, char *parms)
{
   switch (testNum) {
      default :
      {
         return 0;
      }
   }
} 


Diagnostic *systemDiags(int testNum, ReqSourceType reqSrc, char *parms)
{
   switch (testNum) {
      case DIAG_SYSTEM_NETWORK_TEST_SERVER :
         return new NetworkTestServer(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      case DIAG_SYSTEM_NETWORK_TEST_TRANSMIT :
         return new NetworkTestTransmit(
            DiagNames[testNum>>DIAG_GROUP_NUM_BITS][testNum & DIAG_TEST_NUM_MASK], 
               testNum, reqSrc, parms);
         break;       
      default :
      {
         return 0;
      }
   }
} 
