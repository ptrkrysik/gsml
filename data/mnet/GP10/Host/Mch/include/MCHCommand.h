#ifndef MCHCommand_H
#define MCHCommand_H


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
// File        : MCHCommand.h
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description :  
//
// *******************************************************************

#include <vxworks.h>
#include <string.h>

#include "MCH/MCHIntf.h"

// *******************************************************************
// MCHCommand
// *******************************************************************

class MCHCommand
{
public:

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
   MCHCommand(char *ln, char *sn);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: MCHCommand::~MCHCommand
   **
   **    PURPOSE: MCHCommand destructor.
   **
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/ 
   virtual ~MCHCommand();
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: MCHCommand::ExecuteCommand
   **
   **    PURPOSE: Each derived class must implement this function.  ExecuteCommand
   **       is the function that performs the given command.
   **
   **    INPUT PARAMETERS: parms
   **
   **    RETURN VALUE(S): status - indicates if command was executed.
   **
   **----------------------------------------------------------------------------*/ 
   virtual int ExecuteCommand(char *parms) = 0;
   
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: MCHCommand::ShowHelp
   **
   **    PURPOSE: Each derived class must implement this function.  ShowHelp
   **       returns a help description for the command.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): help description in ASCII.
   **
   **----------------------------------------------------------------------------*/ 
   virtual char *ShowHelp(void) = 0;
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: MCHCommand::isCommand
   **
   **    PURPOSE: Test to see if specified command name is this command object.
   **
   **    INPUT PARAMETERS: commName - command name in ASCII
   **
   **    RETURN VALUE(S): TRUE - if it is the command
   **                     FALSE - if it is not the command
   **
   **----------------------------------------------------------------------------*/
   BOOL isCommand(char *commName)
   {
      return ((!strcmp(longName, commName) || !strcmp(shortName, commName)));
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
   void ClearCommandResponse(void);
   
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: MCHCommand::CommandResponseAppend
   **
   **    PURPOSE: GetCommandResponse returns a pointer to the command response.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): constant pointer to the response
   **
   **----------------------------------------------------------------------------*/
   const char *GetCommandResponse(void) { return commResponse; }
   
   
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
   void CommandResponseAppend(char *format, ...);
   
   
   // Public data members.
   ReqSourceType reqSrc;
    
protected:

    char *longName;
    char *shortName;
    
private:

    char *commResponse;
    
    // Don't allow a the default constructor action.
    MCHCommand(){}
};



// *******************************************************************
// MCHHelp
// *******************************************************************

class MCHHelp : public MCHCommand
{
public:

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
   MCHHelp(char *ln, char *sn);
    
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
   virtual int ExecuteCommand(char *parms);
   
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
   virtual char *ShowHelp(void);

private:

   // No default constructor allowed.
   MCHHelp() : MCHCommand(0,0){} 
};


// *******************************************************************
// MCHDiagCommand
// *******************************************************************

class MCHDiagCommand : public MCHCommand
{
public:

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: MCHDiagCommand::MCHDiagCommand
   **
   **    PURPOSE: MCHDiagCommand constructor.
   **
   **    INPUT PARAMETERS: ln - long name for command
   **                      sn - short name for command
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   MCHDiagCommand(char *ln, char *sn) : MCHCommand(ln, sn){};
    
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: MCHDiagCommand::ExecuteCommand
   **
   **    PURPOSE: Each derived class must implement this function.  ExecuteCommand
   **       is the function that performs the given command.
   **
   **    INPUT PARAMETERS: parms
   **
   **    RETURN VALUE(S): status - indicates if command was executed.
   **
   **----------------------------------------------------------------------------*/ 
   virtual int ExecuteCommand(char *parms) = 0;
   
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: MCHCommand::ShowHelp
   **
   **    PURPOSE: Each derived class must implement this function.  ShowHelp
   **       returns a help description for the command.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): help description in ASCII.
   **
   **----------------------------------------------------------------------------*/ 
   virtual char *ShowHelp(void) = 0;
    
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
   int GetDiagTestNum(char *parms);
    

private:

    // No default constructor allowed.
    MCHDiagCommand() : MCHCommand(0,0){}
    
    
};


// *******************************************************************
// MCHStartDiag
// *******************************************************************

class MCHStartDiag : public MCHDiagCommand
{
public:

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
   MCHStartDiag(char *ln, char *sn);
    
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
   virtual int ExecuteCommand(char *parms);
   
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
   virtual char *ShowHelp(void);

private:

   // No default constructor allowed.
   MCHStartDiag() : MCHDiagCommand(0,0){}
    
};


// *******************************************************************
// MCHStopDiag
// *******************************************************************

class MCHStopDiag : public MCHDiagCommand
{
public:

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
   MCHStopDiag(char *ln, char *sn);
    
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
   virtual int ExecuteCommand(char *parms);
   
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
   virtual char *ShowHelp(void);

private:

    // No default constructor allowed.
    MCHStopDiag() : MCHDiagCommand(0,0){}
    
    
};


// *******************************************************************
// MCHGetDiagResult
// *******************************************************************

class MCHGetDiagResult : public MCHDiagCommand
{
public:

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
   MCHGetDiagResult(char *ln, char *sn);
    
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
   virtual int ExecuteCommand(char *parms);
   
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
   virtual char *ShowHelp(void);

private:

   // No default constructor allowed.
   MCHGetDiagResult() : MCHDiagCommand(0,0){}
    
    
};


// *******************************************************************
// MCHGetDiagHelp
// *******************************************************************

class MCHGetDiagHelp : public MCHDiagCommand
{
public:

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
   MCHGetDiagHelp(char *ln, char *sn);
    
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
   virtual int ExecuteCommand(char *parms);
   
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
   virtual char *ShowHelp(void);

private:

   // No default constructor allowed.
   MCHGetDiagHelp() : MCHDiagCommand(0,0){}
    
    
};




#endif