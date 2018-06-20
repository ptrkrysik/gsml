#ifndef _CDRMSG_H_
#define _CDRMSG_H_

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
// File        : CdrMsg.h
// Author(s)   : Sandeep A.
// Create Date : 9/26/00
// Description :  
//
// *******************************************************************

#include "msgProc.h"


#define INPUT_SIZE                      8192 // Ananth: need to revisit the size
#define REGISTER_MSG_TYPE               (0x10000021)
#define REGISTER_ACK_MSG_TYPE           (0x10000031)

#define FILE_ACK_MSG_TYPE               (0x10000051)
#define STATUS_QUERY_MSG_TYPE           (0x10000061)
#define STATUS_RESPONSE_MSG_TYPE        (0x10000071)
#define IPC_FILE_FULL_MSG_TYPE          (0x10000151)

#define REGISTER_TIMEOUT_MSG_TYPE       (0x10000201)
#define FILE_ACK_TIMEOUT_MSG_TYPE       (0x10000211)
#define STATUS_QUERY_TIMEOUT_MSG_TYPE   (0x10000221)
#define DATA_TIMEOUT_MSG_TYPE           (0x10000231)

// New error codes
#define CDR_STATUS_FILE_TRANSFER_ERROR  5
#define CDR_STATUS_NON_FATAL_SEND_ERROR 6
#define CDR_STATUS_FATAL_SEND_ERROR     7

#define CDR_NAME_MAX_SIZE               32

#define FILE_ERROR                      (~1)


typedef struct
{
    ubyte4 cause;
    sbyte  cdr1Name[CDR_NAME_MAX_SIZE];
} RegisterData_t;

typedef struct
{
    ubyte4  cdr1Id;
} RegisterAckData_t;


typedef struct
{
    // ubyte4  lastValidFileId;
} FileAckData_t;

typedef struct
{
    ubyte4  lastValidFileId;
} StatusResponseData_t;

typedef struct
{
    ubyte4 cdr1Id;
    ubyte4  fileId;
    sbyte4  fileSize;
    ubyte4 fileOpenTime;
    ubyte4 fileCloseTime;
} StatusQueryData_t;

typedef struct CdrControlMsg_t
{
    ClientInterfaceMsgHeader_t hdr;
    union {
        RegisterData_t registerData;
        RegisterAckData_t registerAckData;
        FileAckData_t fileAckData;
        StatusQueryData_t statusQueryData;
        StatusResponseData_t statusResponseData;
    }body;
} CdrControlMsg;

typedef struct
{
    ubyte4 cdr1Id;
    sbyte inmsgPool[INPUT_SIZE - 4];
} FirstData_t;

typedef struct
{
    sbyte inmsgPool[INPUT_SIZE];
} NextData_t;


typedef struct CdrDataMsg_t
{
    union {
        FirstData_t firstData;
        NextData_t  nextData;
    }data;
}CdrDataMsg;

#endif
