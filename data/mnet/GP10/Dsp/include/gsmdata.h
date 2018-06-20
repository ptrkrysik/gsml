/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: gsmdata.h
*
* Description:   
*   This file contains references to the global structures and variables
*   used in GSM Layer 1 processing. It includes a file containing the
*   defined constants, enumerations and type definitions that support
*   the same GSM Layer 1 processing.
*
*   NOTE: INGLOB and EXGLOB specify internal and external DSP memory
*         allocation, respectively. They are defined in dsptypes.h
*         and included here through gsmtypes.h. 
*
*****************************************************************************/
#ifndef _GSMDATA_H_
#define _GSMDATA_H_  /* include once only */

#include "dsp/dsphmsg.h"
#include "gsmtypes.h"
                          
EXGLOB t_frameCounts g_frameCounts;

EXGLOB t_GsmFrameNum sysFrameNum;
EXGLOB t_GsmFrameNum setFrameNum;
EXGLOB t_GsmFrameNum localFrameNum[NUM_RFS_PER_DSP];

EXGLOB t_frmSync g_frmSync;
                 
EXGLOB tbfStruct g_tbfInfo[NUM_RFS_PER_DSP][TBF_NUMBER_MAX];  

EXGLOB dlTchFrameStruct g_dlTchFrame[NUM_TS_PER_RF];

EXGLOB burstProcStruc g_burstProc;

EXGLOB dlSidFrameStruct g_dlSidFrame[NUM_TS_PER_RF];
                    
/*
* Function lookup tables for different GSM Groups
*/
EXGLOB Uint32 dlSacchIdleTbl[8];  
EXGLOB Uint32 ulSacchIdleTbl[8];  

EXGLOB Uint32 dlGroupTable0[1];
EXGLOB Uint32 ulGroupTable0[1];
 
EXGLOB Uint32 dlGroupTableI[T2_FRAMES];
EXGLOB Uint32 ulGroupTableI[T2_FRAMES];  
           
EXGLOB Uint32 dlGroupTableIV[1];
EXGLOB Uint32 ulGroupTableIV[1];
            
EXGLOB Uint32 dlGroupTableV[T3_FRAMES];  
EXGLOB Uint32 ulGroupTableV[T3_FRAMES]; 

EXGLOB Uint32 dlGroupTableVII[T3_FRAMES];
EXGLOB Uint32 ulGroupTableVII[T3_FRAMES];

EXGLOB Uint32 dlGroupTableXI[GPRS_MULTI_FRAMES];
EXGLOB Uint32 ulGroupTableXI[GPRS_MULTI_FRAMES];


EXGLOB Uint32 dlGroupTableXIII[GPRS_MULTI_FRAMES/4];
EXGLOB Uint32 ulGroupTableXIII[GPRS_MULTI_FRAMES/4];

EXGLOB Uint32 dlPtcchTable[8];
EXGLOB Uint32 ulPtcchTable[2];

EXGLOB Uint32 dlPbcchTable[4];
EXGLOB Uint32 dlPpchTable[4];

EXGLOB t_group11_params  g11Params;
EXGLOB Uint8  g_PrachBits; /*  # of bits for the PRACH message  (8 or 11) */




/* Define the BBInfo for each RF */
EXGLOB t_BBInfo g_BBInfo[NUM_RFS_PER_DSP];
/* New 01/22/01...encrypt bits onchip to save CPU cycles */
INGLOB UChar tchUlEncryptStream[NUM_RFS_PER_DSP][NUM_TS_PER_RF][NUM_ENCRYPT_BITS];  /*To be applied to UL */
INGLOB UChar sdcchUlEncryptStream[NUM_SDCCH_SUBCHANS][NUM_ENCRYPT_BITS];  /*To be applied to UL */
INGLOB UChar sdcch8UlEncryptStream[NUM_SDCCH8_SUBCHANS][NUM_ENCRYPT_BITS];  /*To be applied to UL */


/*
* CCCH/DCCH related signal buffers, assume mapped to only one time slot
*/   
EXGLOB t_SCHSigBufs    g_dlSigBufsSCH[1][1];
EXGLOB t_CBCHQueue     g_dlSigBufsCBCH[2];      /* one normal, one for extended SMSCB */
EXGLOB t_sysInfoQueue  g_dlSigBufsSYSINFO[1][NUM_SYSINFO_BUFS];



EXGLOB t_DLSigBufs     g_dlSigBufsSACCH[1][1][NUM_SACCH_SUBCHANS];
EXGLOB t_DLSigBufs     g_dlSigBufsSDCCH[1][1][NUM_SDCCH_SUBCHANS];
EXGLOB t_DLSigBufs     g_dlSigBufsSACCH8[1][1][NUM_SACCH8_SUBCHANS];
EXGLOB t_DLSigBufs     g_dlSigBufsSDCCH8[1][1][NUM_SDCCH8_SUBCHANS];

EXGLOB t_DLSigBufs     g_dlSigBufsPTCCH[NUM_RFS_PER_DSP][NUM_TS_PER_RF];

EXGLOB t_DLPPCHBufs    g_dlSigBufsPPCH[NUM_RFS_PER_DSP][NUM_PPCH_SUBCHANS];
 
/* These two variables are associated with the GPRS rateTransition() */
EXGLOB Uint8 g_rateChangeCheck;
EXGLOB Uint8 g_RATE_TRAN_ON;
   
/*
* TCH related Signal buffers for all TRXs and Time slots
*/                                                                   
EXGLOB t_DLSigBufs  g_dlSigBufsFACCH[NUM_RFS_PER_DSP][NUM_TS_PER_RF];
EXGLOB t_DLSigBufs  g_dlSigBufsTchSACCH[NUM_RFS_PER_DSP][NUM_TS_PER_RF];


/*
* P and N buffers (gsm 05.08) for power control and handover.
*/
EXGLOB t_PNbufs   g_PNbufs[NUM_TN_PER_RF];
EXGLOB Uint16     g_rxQual2ber_LookUpTbl[8];
EXGLOB Uint8      g_IntRprtCntr;  /* Counter for interference reports to host */

/*  Definition of group 11 radio block types */

EXGLOB  Uint8          G11_dlRadioBlkType[MAX_RADIO_BLOCKS];
EXGLOB  Uint8          G11_ulRadioBlkType[MAX_RADIO_BLOCKS];
EXGLOB  Uint8          g_dyn_prach_blk;



/* Control whether to send dummy burst for inactive slots */
EXGLOB Uint8      g_IdleFrameMode;

#ifdef ALLOCATE_MEMORY_EXTERNAL /*-------------------------------------------*/ 

Uint8 dcchFiller[23] = { 0x03, 0x03, 0x01, 0x2b, 0x2b, 0x2b, 
                         0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
                         0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
                         0x2b, 0x2b, 0x2b, 0x2b, 0x2b};
Uint8 bcchFiller[23] = { 0x01, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
                         0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
                         0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
                         0x2b, 0x2b, 0x2b, 0x2b, 0x2b};

#else  /*--------------------------------------------------------------------*/

extern Uint8 dcchFiller[23];
extern Uint8 bcchFiller[23];

#endif /*--------------------------------------------------------------------*/


#endif  /* end of include once only */
