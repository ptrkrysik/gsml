/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: bbdata.h
*
* Description:   
*   This file contains references to the global structures and variables
*   used in the Layer 1 Baseband Processing routines. It includes a file
*   containing the defined constants, enumerations and type definitions
*   that support the same Layer 1 Baseband Processing routines. An item
*   is included in this header file if it is used by multiple baseband 
*   processing functions.
*
*   NOTE: INGLOB and EXGLOB specify internal and external DSP memory
*         allocation, respectively. They are defined in dsptypes.h
*         and included here through bbtypes.h. 
*
*****************************************************************************/
#ifndef _BBDATA_H_
#define _BBDATA_H_  /* include once only */

#include "bbtypes.h"

/*
* Declare the global UL and DL channel data buffers.
*/
INGLOB t_DLCCHBuff      g_DLCCHData;
INGLOB t_ULCCHBuff      g_ULCCHData;
INGLOB t_DLTCHBuff      g_DLTCHData;
INGLOB t_ULTCHBuff      g_ULTCHData;
INGLOB t_ULRACHBuff     g_ULRACHData;
INGLOB t_ULCCHFireBuff  g_ULCCHFireData;

/*
* Declare the global DL Coded data buffer.  This is where modulator
* input is stored
*/
INGLOB t_DLCodedBuf g_DLCodedData;

/*
* Declare the global DL burst "bits" data buffers.  This is where
* modulator input comes from and where burst builder output is stored.
*/
INGLOB t_DLBurstBuf g_DLBurstData;
 
/*
* Declare the global UL Burst data buffer.  This is where demod
* output is stored
*/
INGLOB t_ULBurstBuf g_ULBurstData;
 
/*
* Declare the global DL transmission buffers.  This is where modulator
* output is stored
*/
#pragma DATA_ALIGN(g_DLBBData, 4);
INGLOB t_DLBBData g_DLBBData;    

/*
* Declare the global UL receive buffers.  This is where demod
* input is stored
*/
#pragma DATA_ALIGN(g_ULBBData, 4);
INGLOB t_ULBBData g_ULBBData;

INGLOB t_ULComplexSamp *ulBurstBufPtr[4];        
INGLOB t_DLComplexSamp *dlBurstBufPtr[2];   

/*
* Declare DC offset storage for each TCH and each SDCCH plus one all-carrier offset 
*/
INGLOB t_LongComplexSamp g_DcOffset[NUM_RFS_PER_DSP][NUM_TN_PER_RF+NUM_SDCCH_SUBCHANS+1];

/*
  Declare the DC offset store for the GROUP 7 DC offsets 
*/
 
INGLOB  t_LongComplexSamp g_Dc_7_Offset[NUM_RFS_PER_DSP][NUM_SDCCH8_SUBCHANS];
 
/*
* Declare GPRS codec bits per coding class and associated flags 
*/
INGLOB Uint16  g_GPRSCodecBits[GPRS_CODEC_TYPE];  
INGLOB Uint8   g_GPRSCodecFlags[GPRS_CODEC_TYPE];                  

/*
* Global variables and buffers
*/
INGLOB t_DLCodedBuf4 g_intBuf4Deep;
INGLOB t_ULBurstBuf4 g_deintBuf4Deep;
INGLOB t_DLCodedBuf8 g_intBuf8Deep;
INGLOB t_ULBurstBuf8 g_deintBuf8Deep;

INGLOB UChar g_codecBsic[6];

/*
* TCH and RACH masking thresholds
*/
EXGLOB Uint8   g_TchBerThresh;   /* BER thresh for perceptual voice masking */ 
EXGLOB Int16   g_TchMetricThresh;      /* Viterbi metric thresh for masking */
EXGLOB Uint8   g_RachSnrThresh;        /* SNR thresh for masking */
EXGLOB Int16   g_RachMetricThresh;     /* Viterbi metric thresh for masking */


#endif  /* end of include once only */

                
                    
  
