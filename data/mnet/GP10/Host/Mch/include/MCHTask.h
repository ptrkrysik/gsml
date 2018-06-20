#ifndef MCHTask_H
#define MCHTask_H


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

#include <vxworks.h>
#include <lstLib.h>
#include <msgQLib.h>
#include <stdarg.h>
#include "Os/JCTask.h"
#include "Diagnostic.h"


// Forward declarations.
class MCHCommand;

// These are the available functions to be sent to the maintenance
// command handler.
typedef enum {
   MCH_START_DIAG,
   MCH_STOP_DIAG,
   MCH_GET_RESULT,
   MCH_GET_DIAG_HELP,
   MCH_HELP,
   MCH_MAX_COMMANDS
} MCHCommandType;



// *******************************************************************
// MCHCommand
// *******************************************************************

class MCHTask
{
public:

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
   static int MCHLoop(void);

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
   static int InitMCH(void);

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DiagnosticManager::FindDiagnostic
   **
   **    PURPOSE: Search the list of currently active diagnostics given
   **      test number.
   **
   **    INPUT PARAMETERS: testNum - diagnostic test number
   **
   **    RETURN VALUE(S): Diagnostic * - pointer to the new diagnostic if found
   **                                    0 if no diagnostic was found
   **
   **----------------------------------------------------------------------------*/
   Diagnostic *FindDiagnostic(int testNum);
   
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
   void SendResponse(const char *resp, ReqSourceType reqSrc);



   static MCHTask *theMCHTask;
   MCHCommand *mchCommands[MCH_MAX_COMMANDS];
   LIST activeDiags;
   Diagnostic *(*AllDiagnostics[DIAG_LAST_DIAG_GROUP >> DIAG_GROUP_NUM_BITS])(int testNum, ReqSourceType reqSrc, char *parms);

private:
   static BOOL isOneCreated;

   JCTask MCHTaskObj;

   // The only way to create the MCHTask is to call InitMCH. 
   MCHTask();   
    
};


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
Diagnostic *cdcDiags(int testNum, ReqSourceType reqSrc, char *parms); 
Diagnostic *dspDiags(int testNum, ReqSourceType reqSrc, char *parms); 
Diagnostic *rfDiags(int testNum, ReqSourceType reqSrc, char *parms); 
Diagnostic *clkDiags(int testNum, ReqSourceType reqSrc, char *parms); 
Diagnostic *gpsDiags(int testNum, ReqSourceType reqSrc, char *parms); 
Diagnostic *systemDiags(int testNum, ReqSourceType reqSrc, char *parms); 



#endif