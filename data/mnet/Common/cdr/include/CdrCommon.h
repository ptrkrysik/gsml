#ifndef __CDRCOMMON_H__
#define __CDRCOMMON_H__

// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CdrCommon.h
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description :  
//
// *******************************************************************

#include "defs.h"

// TCP Port number for CDR client to CDR server level 1 connection
#define AM_MAIN_SERVER_PORT         11790

// TCP Port number for client connection to CDR server level 2
//#define CDR_MAIN_CONTROL_PORT       11792
//#define CDR_MAIN_DATA_PORT          11793
#define CDR_IPC_PORT                11794

#define CDR_HEADER_SIZE         1           // CDR header length

#define CDR_MAX_HEADER_LEN      100
#define CDR_MAX_DATA_LENGTH     1200
#define CDR_MAX_RECORD_LEN      (CDR_MAX_DATA_LENGTH+CDR_MAX_HEADER_LEN)


#define CDR_GENERIC_MSG_TYPE    (0x1A2B3C4DL)

#define CDR_FIELD_DELIMETER     ' '

// CDR Reference definitions
#define CDR_INVALID_REFERENCE   (0)
#define CDR_MIN_REFERENCE       (1)
#define CDR_MAX_REFERENCE       (0xFFFF)

// CDR Version numbers
#define CDR_INVALID_FORMAT_VER  0
#define CDR_FORMAT_VER_1        1
#define CDR_FORMAT_VER_2        2
#define CDR_FORMAT_VER_3        3
#define CDR_FORMAT_VER_4        4
#define CDR_FORMAT_VER_5        5

//Cdr Error codes
#define CDR_STATUS_OK              SOCKET_STATUS_OK
#define CDR_STATUS_MSG_TOO_LONG    SOCKET_STATUS_MSG_TOO_LONG
#define CDR_STATUS_TEMP_SOC_ERR    SOCKET_STATUS_TEMP_SOC_ERR
#define CDR_STATUS_FATAL_SOC_ERR   SOCKET_STATUS_FATAL_SOC_ERR

// Buffer lengths
#define CTRL_SOC_RCV_BUFF_LEN   (0x1000)
#define CTRL_SOC_SND_BUFF_LEN   (0x1000)
#define DATA_SOC_RCV_BUFF_LEN   (0x500)
#define DATA_SOC_SND_BUFF_LEN   (0x10000)

typedef char CdrLogPool_t [CDR_MAX_RECORD_LEN ];
typedef char CdrRecPool_t [CDR_MAX_DATA_LENGTH];


// CDR password filename
#define CDR_PWD_FILE_NAME   "cdrpwd.dat"


typedef enum
{
    SOCK_ERROR          = SOC_STATUS_ERROR,
    SOCK_NORMAL         = 0,
    SOCKET_CLOSED       = 1,
    FILE_FULL           = 2,
    REGISTER_TIMEOUT    = 3,
    STATUS_QUERY_TIMEOUT= 4,
    FILE_ACK_TIMEOUT    = 5,
    DATA_TIMEOUT        = 6,
    SHUTDOWN_CLIENT     = 7
    
} CDR_SOCK_RESULT;

enum RegisterCause
{
    INIT_CAUSE,
    FILE_TRANSFER_CAUSE
};

typedef enum
{
    CURRENT ,
    FINISHED,
    UNACK   ,
    ACK
}CDR_FILE_EXTENSIONS;

typedef enum
{
    DELETE_ENTRY              ,
    ADD_ENTRY                 ,
    WRITE_FILE_CLOSE_TIME     ,
    WRITE_FILE_TRANSFER_TIME  ,
    WRITE_FILE_TRANSFER_STATUS

} CDR_TIMESTAMP_FILE_OP;

struct timeStampRec
{
  char fileName[MAX_PATH-1];
  bool fileTransfer;
  ubyte4 fileOpenTime;
  ubyte4 fileCloseTime;
  ubyte4 fileTransferTime;

};

typedef struct
{
  struct timeStampRec* rec;
  sbyte4 numEntries;

} timeStampStruct;


#endif //__CDRCOMMON_H__
