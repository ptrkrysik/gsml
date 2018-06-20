/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: comtypes.h
*
* Description:
*   This file contains defined constants, enumerations and type definitions
*   of shared items to support DSP HPI communication.
*
******************************************************************************/
#ifndef _COMTYPES_H_
#define _COMTYPES_H_  /* include once only */

#include "dsp/dsptypes.h"
#include "dsp/dsphmsg.h"

#define MSG_OVERFLOW_MAX_COUNT 0xffffffff
#define QUE_HEAD_SIZE 12

typedef struct 
{           
  Uint32    msgOverflow;
  Int16     msgCount;
  Int16     qSize;
  Int16     qAttrb;   
  Int8      msgInIndex;
  Int8      msgOutIndex;
  msgStruc  **qMsg;
} qRspStruc, qCmdStruc, qStruc;

#endif  /* end of include once only */ 
                            
