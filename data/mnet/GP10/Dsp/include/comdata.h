/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: comdata.h
*
* Description:
*   This file contains references to the global structures and variables
*   used for DSP HPI communication. It includes a file containing the
*   defined constants, enumerations and type definitions that support HPI.
*
*   NOTE: INGLOB and EXGLOB specify internal and external DSP memory
*         allocation, respectively. They are defined in dsptypes.h
*         and included here through comtypes.h. 
*
******************************************************************************/
#ifndef _COMDATA_H_
#define _COMDATA_H_  /* include once only */

#include "comtypes.h"

/*
 * DSP Rx HPI counter and buffer. These are referenced extern here since
 * their memory is allocated in hpirx.c and assigned to Rx HPI memory.
 */                           
extern volatile Int32 HostMsgNumWord;    
extern Uint8          rxMsgBuf[0x200];

/*
 * DSP Tx HPI counter and buffer. These are referenced extern here since
 * their memory is allocated in hpitx.c and assigned to Tx HPI memory.
 */                           
extern volatile Int32 DspMsgNumWord;
extern Uint8          txMsgBuf[0x200];

/*
 * DSP HOST PORT buffer pointer
 */                           
INGLOB volatile cmdStruc *hpiRxMsgPtr;  
INGLOB volatile rspStruc *hpiTxMsgPtr;  
                                               
/*
 * Message queue for HPI
 */                              
INGLOB qCmdStruc *FastHmsgQue;  
INGLOB qCmdStruc *HmsgQue;    

INGLOB qRspStruc *FastRspQue; 
INGLOB qRspStruc *RspQue;  
                                 
INGLOB Uint32 TimerIsrCount;

INGLOB Uint16 DspId;

#endif  /* end of include once only */
