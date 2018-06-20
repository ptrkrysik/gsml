#ifndef _MSGPROC_H_
#define _MSGPROC_H_

// *******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : msgProc.h
// Author(s)   : Igal Gutkin
// Create Date : 6/21/00
// Description :  
//
// *******************************************************************



/* CDR Message common header */
typedef struct 
{
    ubyte4 msgType;  // reserved for the future use
    ubyte4 dataLen;
} ClientInterfaceMsgHeader_t;

#define clientMsgHdrLen  (sizeof(ClientInterfaceMsgHeader_t))


typedef struct 
{
    char logData [CDR_MAX_DATA_LENGTH];
} ClientCdrData_t;


typedef struct 
{
    ClientInterfaceMsgHeader_t hdr;
    ClientCdrData_t            msg;
} ClientMsg_t;


#endif /* _MSGPROC_H_ */
