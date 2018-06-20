#ifndef MCHIntf_H
#define MCHIntf_H

/* ******************************************************************/
/*                                                                  */
/* (c) Copyright Cisco 2000                                         */
/* All Rights Reserved                                              */
/*                                                                  */
/* ******************************************************************/

/* *******************************************************************/

/* Version     : 1.0                                                 */
/* Status      : Under development                                   */
/* File        : MCHIntf.h                                           */
/* Author(s)   : Tim Olson                                           */
/* Create Date : 6/29/99                                             */
/* Description :                                                     */

/* *******************************************************************/

#include "Mch/MCHDefs.h"

/* Delete diagnostic message sent by a diagnostic which has terminated.  */
class Diagnostic;
typedef struct {
    Diagnostic *diag;
} DeleteDiagMsgType;

typedef struct {
   MCHFuncType func;
   union {
      char commLine[MCH_MAX_COMMAND_LINE_LEN];
      DeleteDiagMsgType delDiagMsg;
      DiagnosticMsgType diagMsg;
   };
} MCHMessageType;


/* Indicate what was the source of the maintenance request. */
typedef enum {
   NETWORK_REQ,
   TERMINAL_REQ
} ReqSourceType;

extern void DiagSendMsg(char *msg, int msgLen, int func);
extern void MaintCmdTcpServer(char *commLine, short size, unsigned long clientId);




#endif  /* MCHIntf_H  */
