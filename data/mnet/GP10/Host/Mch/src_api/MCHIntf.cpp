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
// File        : MCHIntf.cpp
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description : 
//
// *******************************************************************
#include <msgQLib.h>
#include <string.h>
#include <stdio.h>

#include "MCHTask.h"
#include "MCHInit.h"
#include "logging\vcmodules.h"
#include "logging\vclogging.h"



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DiagSendMSReportMsg
**
**    PURPOSE: Send the DSP report to the Maintenance Command Handler. 
**
**    INPUT PARAMETERS: sndMsg - message to send to diagnostics
**                      msgLen - length of the message
**                      func - function code to send to diagnostics
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DiagSendMsg(char *sndMsg, int msgLen, int func)
{
   DBG_FUNC("DiagSendMSReportMsg",MAINT_COMM_HDLR);
   DBG_ENTER();
   
   int rtnStat;
       
   // Create the message to send to the maintenance command handler.
   MCHMessageType msg;
   bzero ((char *)&msg, sizeof(MCHMessageType));

   if (msgLen > MAX_DIAGNOSTIC_MSG)
   {
      msgLen = MAX_DIAGNOSTIC_MSG;
   }
   
   // Fill in message.
   msg.func = MCH_DIAG_SPECIFIC_MSG;
   msg.diagMsg.func = func;
   msg.diagMsg.msgLen = msgLen;
   bcopy(sndMsg, msg.diagMsg.msgBody, msgLen);

   // Send message to maintenance command handler.
   if ((rtnStat = msgQSend (MCHMsgQId,
                            (char *)&msg,
                            sizeof(MCHMessageType),
                            NO_WAIT,
                            MSG_PRI_NORMAL)) == ERROR)
   {
      DBG_WARNING ("Error sending message %x\n", rtnStat);
   }
   DBG_LEAVE();
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MaintCmd
**
**    PURPOSE:  MaintCmd should be used from the shell to initiate maintenance
**       functionality.  All parameters are passed in as a string.  The
**       diagnostics will parse the strings as required.
**
**    INPUT PARAMETERS: commLine - maintenance command in ASCII
**
**    RETURN VALUE(S): status - OK for success, ERROR for failure
**
**----------------------------------------------------------------------------*/

int MaintCmd(char *commLine)
{
   DBG_FUNC("MaintCmd",MAINT_COMM_HDLR);
   DBG_ENTER();

   int rtnStat = OK;
       
   // Create the message to send to the maintenance command handler.
   MCHMessageType msg;
   bzero ((char *)&msg, sizeof(MCHMessageType));

   // Let the maintenance command handler know the request was from a
   // terminal type device.
   msg.func = MCH_TERMINAL_REQ;

   // If command is not NULL then copy the command into the message.
   if (commLine) 
   {
      strncpy (msg.commLine, commLine, MCH_MAX_COMMAND_LEN);
   } 

   // Send message to maintenance command handler.
   if ((rtnStat = msgQSend (MCHMsgQId,
                            (char *)&msg,
                            sizeof(MCHMessageType),
                            WAIT_FOREVER,
                            MSG_PRI_NORMAL)) == ERROR)
   {
      DBG_WARNING ("Error sending message %x\n", rtnStat);
   }
    
   DBG_LEAVE();   
   return rtnStat;
}




/*----------------------------------------------------------------------------**
**
**    METHOD NAME: MaintCmdTcpServer
**
**    PURPOSE:  MaintCmd should be used from the shell to initiate maintenance
**       functionality.  All parameters are passed in as a string.  The
**       diagnostics will parse the strings as required.
**
**    INPUT PARAMETERS: commLine - maintenance command in ASCII
**                      size - size of command line
**                      clientId - unique id for sending client
**
**    RETURN VALUE(S): status - OK for success, ERROR for failure
**
**----------------------------------------------------------------------------*/

void MaintCmdTcpServer(char *commLine, short size, unsigned long clientId)
{
   DBG_FUNC("MaintCmdTcpServer",MAINT_COMM_HDLR);
   DBG_ENTER();

   int rtnStat = OK;
       
   // Create the message to send to the maintenance command handler.
   MCHMessageType msg;
   bzero ((char *)&msg, sizeof(MCHMessageType));

   // Let the maintenance command handler know the request was from a
   // network type device.
   msg.func = MCH_NETWORK_REQ;

   // If command is not NULL then copy the command into the message.
   if (commLine) 
   {
      strncpy (msg.commLine, commLine, min(MCH_MAX_COMMAND_LEN, size));
   } 

   // Send message to maintenance command handler.
   if ((rtnStat = msgQSend (MCHMsgQId,
                            (char *)&msg,
                            sizeof(MCHMessageType),
                            WAIT_FOREVER,
                            MSG_PRI_NORMAL)) == ERROR)
   {
      DBG_WARNING ("Error sending message %x\n", rtnStat);
   }
    
   DBG_LEAVE();   
}
