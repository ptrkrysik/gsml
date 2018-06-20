/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: diagdata.h
*
* Description:
*   This file contains references to the global structures and variables
*   used for diagnostics and error reporting. It includes a file containing
*   the defined constants, enumerations and type definitions that support 
*   diagnostics and error reporting.
*
*   NOTE: INGLOB and EXGLOB specify internal and external DSP memory
*         allocation, respectively. They are defined in dsptypes.h
*         and included here through diagtypes.h. 
*
******************************************************************************/
#ifndef _DIAGDATA_H_
#define _DIAGDATA_H_  /* include once only */

#include "diagtypes.h"
                                                           
EXGLOB t_diagData g_diagData;
EXGLOB Uint8      g_loopBackMode;
EXGLOB Int32      g_UlBurstLog[156];

EXGLOB Uint16     g_DlUnderflowCount[NUM_RFS_PER_DSP][NUM_TN_PER_RF];
EXGLOB Uint16     g_DlOverflowCount[NUM_RFS_PER_DSP][NUM_TN_PER_RF];
EXGLOB Uint16     g_DlOutOfSequenceCount[NUM_RFS_PER_DSP][NUM_TN_PER_RF];

EXGLOB Int16      g_ErrorEnable; /* for sending messages only once per SF */ 

EXGLOB idleProcStruc g_idleFrameInfo[1][8];

EXGLOB Uint32  rcvdRawIQFrames[8][625];
EXGLOB Uint16  rcvRawIQBufIndex[8];            
EXGLOB Uint16  txRawIQBufIndex[8];  
   

#endif  /* end of include once only */

                  
  