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
// File        : MCHCommand.cpp
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description :  
//
// *******************************************************************

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "MCHCommand.h"
#include "MCHTask.h"
#include "logging\vcmodules.h"
#include "logging\vclogging.h"


// *******************************************************************
// MCHCommand
// *******************************************************************


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHCommand::MCHCommand
**
**    PURPOSE: MCHCommand constructor.
**
**    INPUT PARAMETERS: ln - long name for command
**                      sn - short name for command
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
MCHCommand::MCHCommand(char *ln, char *sn)
{
   longName = shortName = commResponse = 0;

   // Make sure the name is not not null and save it.
   if (ln)
   {
       longName = new char(strlen(ln) + 1);
       strcpy (longName, ln);
   }

   // Make sure the name is not not null and save it.
   if (sn)
   {
       shortName = new char(strlen(sn) + 1);
       strcpy (shortName, sn);
   }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHCommand::~MCHCommand
**
**    PURPOSE: MCHCommand destructor.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
MCHCommand::~MCHCommand()
{
   // Free up memory.
   if (longName) delete [] longName;
   if (shortName) delete [] shortName;
   if (commResponse) delete [] commResponse;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHCommand::ClearCommandResponse
**
**    PURPOSE: ClearCommandResponse frees memory allocated for the command
**       response.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void MCHCommand::ClearCommandResponse(void)
{
   DBG_FUNC("MCHCommand::ClearCommandResponse",MAINT_COMM_HDLR);
   DBG_ENTER();
   
   if (commResponse) delete [] commResponse;
   commResponse = 0;
   
   DBG_LEAVE();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHCommand::CommandResponseAppend
**
**    PURPOSE: CommandResponseAppend appends the given string to the current
**       response string.
**
**    INPUT PARAMETERS: printf style input
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void MCHCommand::CommandResponseAppend(char *format, ...)
{
   DBG_FUNC("MCHCommand::CommandResponseAppend",MAINT_COMM_HDLR);
   DBG_ENTER();

   char buff[4096];
   buff[0] = 0;
   va_list ap;
   va_start(ap, format);
   vsprintf(buff, format, ap);
   va_end(ap);

   char *temp = new char[strlen(commResponse) + strlen(buff) + 1];
   temp[0] = 0;
   if (commResponse)
      strcpy(temp, commResponse);
   strcat(temp, buff);
   delete [] commResponse;
   commResponse = temp;
   
   DBG_LEAVE();
}



// *******************************************************************
// MCHHelp
// *******************************************************************


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHHelp::MCHHelp
**
**    PURPOSE: MCHHelp constructor.
**
**    INPUT PARAMETERS: ln - long name for command
**                      sn - short name for command
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
MCHHelp::MCHHelp(char *ln, char *sn) : MCHCommand(ln, sn)
{}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHHelp::ExecuteCommand
**
**    PURPOSE: Execute the maintenance command handler help function.
**
**    INPUT PARAMETERS: parms - parameter for command in ASCII
**
**    RETURN VALUE(S): status indicating success or failure
**
**----------------------------------------------------------------------------*/
int MCHHelp::ExecuteCommand(char *parms)
{
   DBG_FUNC("MCHHelp::ExecuteCommand",MAINT_COMM_HDLR);
   DBG_ENTER();

   DBG_TRACE("MCH : %s %s\n", longName, parms);
   
   CommandResponseAppend("The following maintenance commands are available:\n");
   
   if (reqSrc == TERMINAL_REQ)
   {
      CommandResponseAppend("MaintCmd\n");
   }
    
   for (int type = 0; type < MCH_MAX_COMMANDS; type++)
   {
       CommandResponseAppend(MCHTask::theMCHTask->mchCommands[type]->ShowHelp());
   }
   
   DBG_LEAVE();
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHHelp::ShowHelp
**
**    PURPOSE: Display help information for the help command.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): return help information in ASCII
**
**----------------------------------------------------------------------------*/
char *MCHHelp::ShowHelp(void)
{
   DBG_FUNC("MCHHelp::ShowHelp",MAINT_COMM_HDLR);
   DBG_ENTER();
   
   return("\t\"MCHHelp\" or \"help\"\n"
     "\t\tDisplay this help message\n");
     
   DBG_LEAVE();
}



// *******************************************************************
// MCHDiagCommand
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHDiagCommand::GetDiagTestNum
**
**    PURPOSE: This function converts the first parameter found in the parameter
**     list from ASCII to an integer which represents the test number.
**
**    INPUT PARAMETERS: parms - parameter list
**
**    RETURN VALUE(S): test number or -1 if an error
**
**----------------------------------------------------------------------------*/
int MCHDiagCommand::GetDiagTestNum(char *parms)
{
   DBG_FUNC("MCHDiagCommand::GetDiagTestNum",MAINT_COMM_HDLR);
   DBG_ENTER();
 
   int testNum = -1;
   char testNumString[MCH_MAX_COMMAND_LINE_LEN];
   char diagNameString[MCH_MAX_COMMAND_LINE_LEN];
   
   sscanf(parms, "%s", &testNumString);
   
   if (testNumString)
   {
      // Convert all letters in the name to upper case
      for (int i = 0; i <= strlen(testNumString); i++)
      {
         testNumString[i] = toupper(testNumString[i]);
      }
   
      for (int diagGrp = 0; diagGrp < DIAG_LAST_DIAG_GROUP>>DIAG_GROUP_NUM_BITS; diagGrp++)
      {
         for (int diagNum = 1; DiagNames[diagGrp][diagNum]; diagNum++)
         {
            // Convert all letters in the name to upper case
            for (i = 0; i <= strlen(DiagNames[diagGrp][diagNum]); i++)
            {
               diagNameString[i] = toupper(DiagNames[diagGrp][diagNum][i]);
            }
            
            if (!strcmp(testNumString, diagNameString))
            {
               testNum = (diagGrp<<DIAG_GROUP_NUM_BITS) + diagNum;
               DBG_LEAVE();
               return testNum;
            }
         }
      } 
   } 
     
   DBG_LEAVE();
   return testNum;
}




// *******************************************************************
// MCHStartDiag
// *******************************************************************


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHStartDiag::MCHStartDiag
**
**    PURPOSE: MCHStartDiag constructor.
**
**    INPUT PARAMETERS: ln - long name for command
**                      sn - short name for command
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
MCHStartDiag::MCHStartDiag(char *ln, char *sn) : MCHDiagCommand(ln, sn)
{}




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHStartDiag::ExecuteCommand
**
**    PURPOSE: Execute the maintenance command handler start diagnostic function.
**
**    INPUT PARAMETERS: parms - parameter for command in ASCII
**
**    RETURN VALUE(S): status indicating success or failure
**
**----------------------------------------------------------------------------*/
int MCHStartDiag::ExecuteCommand(char *parms)
{
   DBG_FUNC("MCHStartDiag::ExecuteCommand",MAINT_COMM_HDLR);
   DBG_ENTER();
 
   DBG_TRACE("MCH : %s %s\n", longName, parms);
 
   int testNum = GetDiagTestNum(parms);
   
   if ((testNum >= 0) && 
       (testNum>>DIAG_GROUP_NUM_BITS < DIAG_LAST_DIAG_GROUP>>DIAG_GROUP_NUM_BITS))
   {
      // Search the list of active diagnostics given the test number.
      Diagnostic *newDiag = MCHTask::theMCHTask->FindDiagnostic(testNum);
   
      if (newDiag)
      {
         CommandResponseAppend("Test currently in progress for testNum 0x%x\n", testNum);
         DBG_LEAVE();
         return ERROR;        
      }
              
      // Create a the new diagnostic which corresponds to the requested test.  If a value
      // of zero is returned then the diag was not created.
      if (MCHTask::theMCHTask->AllDiagnostics[testNum>>DIAG_GROUP_NUM_BITS])
      {
         newDiag = 
            MCHTask::theMCHTask->AllDiagnostics[testNum>>DIAG_GROUP_NUM_BITS](testNum, reqSrc, parms);
      }

      // If the diagnostic was created then call the RunDiagnosic function.
      // A value of zero indicates the diagnostic was not created.  If RunDiagnostic was 
      // successful then add the diagnostic to the list of currently active diagnostics.
      // If RunDiagnostic was not successful then delete the diagnostic since it was 
      // unable to run properly.
      if (newDiag)
      {
         if (newDiag->RunDiagnostic() == OK)
         {
            lstAdd(&MCHTask::theMCHTask->activeDiags, newDiag);
            CommandResponseAppend("Initiating test 0x%x\n", testNum);
         }
         else
         {
            delete newDiag;
         }
      }
      else
      {
         CommandResponseAppend("StartDiag: Invalid DiagName\n");       
      }
      
   }
   else
   {
      CommandResponseAppend("DiagName not specified or invalid DiagName\n");
   } 
   
   DBG_LEAVE();
   return OK; 
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHStartDiag::ShowHelp
**
**    PURPOSE: Display help information for the StartDiag command.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): return help information in ASCII
**
**----------------------------------------------------------------------------*/
char *MCHStartDiag::ShowHelp(void)
{
   DBG_FUNC("MCHStartDiag::ShowHelp",MAINT_COMM_HDLR);
   DBG_ENTER();
 
   DBG_LEAVE();
   return("\t\"StartDiag or sttd DiagName <parm1> ... <parmN>\"\n"
      "\t\tExecute diagnostic\n");
}


// *******************************************************************
// MCHStopDiag
// *******************************************************************


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHStopDiag::MCHStopDiag
**
**    PURPOSE: MCHStopDiag constructor.
**
**    INPUT PARAMETERS: ln - long name for command
**                      sn - short name for command
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
MCHStopDiag::MCHStopDiag(char *ln, char *sn) : MCHDiagCommand(ln, sn)
{}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHStopDiag::ExecuteCommand
**
**    PURPOSE: Execute the maintenance command handler stop diagnostic function.
**
**    INPUT PARAMETERS: parms - parameter for command in ASCII
**
**    RETURN VALUE(S): status indicating success or failure
**
**----------------------------------------------------------------------------*/
int MCHStopDiag::ExecuteCommand(char *parms)
{
   DBG_FUNC("MCHStopDiag::ExecuteCommand",MAINT_COMM_HDLR);
   DBG_ENTER();
   
   DBG_TRACE("MCH : %s %s\n", longName, parms);
   
   int testNum = GetDiagTestNum(parms);
   
   if (testNum >= 0)
   {
      // Search the list of active diagnostics given the test number.
      Diagnostic *diag = MCHTask::theMCHTask->FindDiagnostic(testNum);

      // If an active diagnostic was found then attempt to perform the stop test function,
      // otherwise send a NACK message back to requester.
      if (diag)
      {
         // If this diagnostic has a message queue then it may be stopped.
         if (diag->diagQId)
         {
            DiagnosticMsgType diagMsg;
            diagMsg.func = DIAG_TEST_STOP_TEST;
            
            int status = msgQSend(diag->diagQId, 
                     (char *)&diagMsg, 
                     sizeof(DiagnosticMsgType), 
                     WAIT_FOREVER, 
                     MSG_PRI_NORMAL);
                                                                       
                                         
            if (status != OK)
            {
               DBG_ERROR("unable to send message %d\n", status);
               CommandResponseAppend("Unable to stop test 0x%x\n", testNum);
            }
            else
            {
               CommandResponseAppend("Stopping test 0x%x\n", testNum);
            }
         }
         // Since there is no message queue the diagnostic cannot be stopped so send back
         // a NACK message to the requester.
         else
         {
            CommandResponseAppend("Bad message queue.  Can't stop test\n");
         }
      }
      // No diagostic active with the test number.
      else
      {
         CommandResponseAppend("No active test for specified DiagName\n");
      }
   }
   else
   {
       CommandResponseAppend("DiagName not specified\n"); 
   }
   
   DBG_LEAVE();
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHStopDiag::ShowHelp
**
**    PURPOSE: Display help information for the StopDiag command.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): return help information in ASCII
**
**----------------------------------------------------------------------------*/
char *MCHStopDiag::ShowHelp(void)
{
   DBG_FUNC("MCHStopDiag::ShowHelp",MAINT_COMM_HDLR);
   DBG_ENTER();
  
   DBG_LEAVE();
   return ("\t\"StopDiag or stpd DiagName\"\n"
      "\t\tStop diagnostic\n");
}


// *******************************************************************
// MCHGetDiagResult
// *******************************************************************


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHGetDiagResult::MCHGetDiagResult
**
**    PURPOSE: MCHGetDiagResult constructor.
**
**    INPUT PARAMETERS: ln - long name for command
**                      sn - short name for command
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
MCHGetDiagResult::MCHGetDiagResult(char *ln, char *sn) : MCHDiagCommand(ln, sn)
{}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHGetDiagResult::ExecuteCommand
**
**    PURPOSE: Execute the maintenance command handler get diagnostic result
**       function.
**
**    INPUT PARAMETERS: parms - parameter for command in ASCII
**
**    RETURN VALUE(S): status indicating success or failure
**
**----------------------------------------------------------------------------*/
int MCHGetDiagResult::ExecuteCommand(char *parms)
{
   DBG_FUNC("MCHGetDiagResult::ExecuteCommand",MAINT_COMM_HDLR);
   DBG_ENTER();
  
   DBG_TRACE("MCH : %s %s\n", longName, parms);
  
   CommandResponseAppend("MCHGetDiagResult not implemented yet!\n");
   DBG_LEAVE();
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHGetDiagResult::ShowHelp
**
**    PURPOSE: Display help information for the GetDiagResult command.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): return help information in ASCII
**
**----------------------------------------------------------------------------*/
char *MCHGetDiagResult::ShowHelp(void)
{
   DBG_FUNC("MCHGetDiagResult::ShowHelp",MAINT_COMM_HDLR);
   DBG_ENTER();
   
   DBG_LEAVE();
   return ("\t\"GetDiagResult or gdr DiagName <parm1> ... <parmN>\"\n"
      "\t\tGet results of latest diagnostic run\n");
}



// *******************************************************************
// MCHGetDiagHelp
// *******************************************************************


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHGetDiagHelp::MCHGetDiagHelp
**
**    PURPOSE: MCHGetDiagHelp constructor.
**
**    INPUT PARAMETERS: ln - long name for command
**                      sn - short name for command
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
MCHGetDiagHelp::MCHGetDiagHelp(char *ln, char *sn) : MCHDiagCommand(ln, sn)
{}




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHGetDiagHelp::ExecuteCommand
**
**    PURPOSE: Execute the maintenance command handler get diagnostic help 
**       function.
**
**    INPUT PARAMETERS: parms - parameter for command in ASCII
**
**    RETURN VALUE(S): status indicating success or failure
**
**----------------------------------------------------------------------------*/
int MCHGetDiagHelp::ExecuteCommand(char *parms)
{
   DBG_FUNC("MCHGetDiagHelp::ExecuteCommand",MAINT_COMM_HDLR);
   DBG_ENTER();
  
   DBG_TRACE("MCH : %s %s\n", longName, parms);
  
   int testNum = GetDiagTestNum(parms);
   
   if ((testNum >= 0) && 
       (testNum>>DIAG_GROUP_NUM_BITS < DIAG_LAST_DIAG_GROUP>>DIAG_GROUP_NUM_BITS))
   {    
      // Create a the new diagnostic which corresponds to the requested test.  If a value
      // of zero is returned then the diag was not created.
      Diagnostic *newDiag = 0;
      if (MCHTask::theMCHTask->AllDiagnostics[testNum>>DIAG_GROUP_NUM_BITS])
      {
         newDiag = 
            MCHTask::theMCHTask->AllDiagnostics[testNum>>DIAG_GROUP_NUM_BITS](testNum, reqSrc, parms);
      }

      // Display help information.
      if (newDiag)
      {
         CommandResponseAppend(newDiag->GetDiagnosticHelp());
         delete newDiag;
      }
      else
      {
         CommandResponseAppend("StartDiag: Invalid diagName\n");       
      }
   }
   else
   {
      // Display a list of all available diagnostic.
      for (int diagGrp = 0; diagGrp < DIAG_LAST_DIAG_GROUP>>DIAG_GROUP_NUM_BITS; diagGrp++)
      {
         CommandResponseAppend("%s\n", DiagNames[diagGrp][0]);
         for (int diagNum = 1; DiagNames[diagGrp][diagNum]; diagNum++)
         {
            CommandResponseAppend("\t0x%x - %s\n", (diagGrp<<DIAG_GROUP_NUM_BITS) + diagNum,
               DiagNames[diagGrp][diagNum]);
         }
      }
   } 
   
   DBG_LEAVE();
   return OK; 
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MCHGetDiagHelp::ShowHelp
**
**    PURPOSE: Display help information for the GetDiagHelp command.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): return help information in ASCII
**
**----------------------------------------------------------------------------*/
char *MCHGetDiagHelp::ShowHelp(void)
{
   DBG_FUNC("MCHGetDiagHelp::ShowHelp",MAINT_COMM_HDLR);
   DBG_ENTER();
  
   DBG_LEAVE();
   return ("\t\"GetDiagHelp or gdh DiagName\"\n"
     "\t\tGet diagnostic help (No DiagName for list of all diags)\n");
}
