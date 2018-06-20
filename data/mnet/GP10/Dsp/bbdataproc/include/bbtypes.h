/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: bbtypes.h
*
* Description:   
*   This file contains defined constants, enumerations and type definitions
*   of shared items in support of the Layer 1 Baseband Processing routines.
*   An item is included in this header file if it is used by multiple
*   baseband processing functions.
*
*****************************************************************************/
#ifndef _BBYTPES_H
#define _BBYTPES_H

#include "gsmtypes.h" 
           
#define MAX_SOFT_DECISION     8
#define MODULO_2_MASK         0x0001
#define MODULO_4_MASK         0x0003
#define MODULO_8_MASK         0x0007   

/*
* Define bit lengths for each burst type including
* training sequence length, number of encrypted bits, etc...   
*
*
* Normal Burst
*  -----------------------------------------------------------
* |TB=3|  DATA = 58   | TSEQ = 26 |  DATA = 58   |TB=3|GP=8.25|
*  -----------------------------------------------------------
*
* Access Burst
*  -----------------------------------------------------------
* |TB=8| TSEQ = 41 | DATA = 36 |TB=3|    GP=68.25             |
*  -----------------------------------------------------------
*
* Sync Burst
*  -----------------------------------------------------------
* |TB=3| DATA = 39|     TSEQ = 64   |  DATA = 39 |TB=3|GP=8.25|
*  -----------------------------------------------------------
*
*/
#define NB_NUM_ENCRYPT_BITS_PER_BURST  116                      
#define NB_NUM_ENCRYPT_BITS_PER_SIDE   (NB_NUM_ENCRYPT_BITS_PER_BURST/2)
#define NB_NUM_ENCRYPT_WORDS           (NB_NUM_ENCRYPT_BITS_PER_BURST/16 + 1)  
#define NB_TRAIN_SIZE                  26
#define NB_NUM_TRAIN_SEQ               8

#define SB_NUM_ENCRYPT_BITS_PER_BURST  78
#define SB_NUM_ENCRYPT_BITS_PER_SIDE   (SB_NUM_ENCRYPT_BITS_PER_BURST/2)
#define SB_TRAIN_SIZE                  64

#define AB_NUM_ENCRYPT_BITS_PER_BURST  36
#define AB_TRAIN_SIZE                  41

#define DB_NUM_MIXED_BITS              142
#define FB_NUM_FIXED_BITS              142

#define MAX_NUM_ENCRYPT_BITS_PER_BURST (NB_NUM_ENCRYPT_BITS_PER_BURST) 
#define MAX_NUM_ENCRYPT_BITS_PER_SIDE  (NB_NUM_ENCRYPT_BITS_PER_SIDE)
#define MAX_TSEQ_SIZE                  (SB_TRAIN_SIZE)

#define NB_NUM_TAIL_BITS    3
#define SB_NUM_TAIL_BITS    3
#define AB_NUM_TAIL_BITS    8

#define BURST_SIZE_DL               625/2 + 1

#define NUM_BURSTS_PER_CCH_BLOCK    4 
#define NUM_BURSTS_PER_TCH_BLOCK    8

#define NORMAL_BURST 0
#define ACCESS_BURST 1
           
#define NUM_BITS_TCH_FRAME          260
#define NUM_BITS_CCH_FRAME	         184
#define NUM_BITS_RACH_FRAME         8
#define NUM_BITS_PRACH_FRAME        11
#define NUM_BITS_SCH_FRAME          25
#define FILLFRAME_LEN_START         18
#define FILLFRAME_LEN_END           24

#define NUM_STEAL_FLAGS_PER_FRAME  8

#define NUM_CODED_BITS_TCH_FRAME  378
#define NUM_CODED_BITS_CCH_FRAME  456

#define NUM_BER_BITS_TCH_FRAME    (NUM_CODED_BITS_TCH_FRAME + NUM_STEAL_FLAGS_PER_FRAME)
#define NUM_BER_BITS_FACCH_FRAME  (NUM_CODED_BITS_CCH_FRAME + NUM_STEAL_FLAGS_PER_FRAME)
#define NUM_BER_BITS_SACCH_FRAME  (NUM_CODED_BITS_CCH_FRAME + NUM_STEAL_FLAGS_PER_FRAME)

/* 
* Uplink constants: Define the number of samples per burst
*/   
#define BURST_SIZE_UL_MAX           157  

/*
* Downlink constants. Note: cut sampling rate by 2 to bypass
* McBSP dropping samples at 26 MHz bit clock rate
*/
#define SAMPLES_PER_BIT        2
#define SAMPLES_PER_BIT_13MHz  2
#define LUT_DEPTH              32
#define MOD_NUM_BITS_IN        148
#define NB_BITS_TO_MOD         146
#define AB_BITS_TO_MOD         86
#define RAMP_SAMPLES           (SAMPLES_PER_BIT*5)

/*
* TCH Perceptual Masking Thresholds. BER Thresh is the number of bit errors
* per voice frame above which we mask out the frame. This is set empirically
* to 37/378 = 10% BER. Metric Thresh is the Viterbi metric below which we mask.
*/
#define TCH_BER_THRESH              37
#define TCH_METRIC_THRESH           0x600  
#define RACH_SNR_THRESHOLD          35       /* # RACH tseq bits to match */
#define RACH_METRIC_THRESHOLD       0x80     /* This needs to be tuned */

/*
* GPRS related items
*/
#define MAX_NUM_GPRS_ADD_BITS 168

typedef enum
{
   GPRS_CS_1 = 0,
   GPRS_CS_2 = 1,
   GPRS_CS_3 = 2,
   GPRS_CS_4 = 3,
   GPRS_CODEC_TYPE

} GPRSCodecType; 

/*
*  GSM 5.03
*/            
#define NUM_BITS_GPRS_CS1  NUM_BITS_CCH_FRAME 
#define NUM_BITS_GPRS_CS2  271 
#define NUM_BITS_GPRS_CS3  315  
#define NUM_BITS_GPRS_CS4  431  

/*
*  GPRS Codec Flags on 4 bursts
*/
#define FRAME_FLAGS_CS1  0xFF
//#define FRAME_FLAGS_CS2  0x31
#define FRAME_FLAGS_CS2  0xC8
//#define FRAME_FLAGS_CS3  0x84
#define FRAME_FLAGS_CS3  0x21
//#define FRAME_FLAGS_CS4  0x68
#define FRAME_FLAGS_CS4  0x16
/*
* Type used for DL transmission buffers
*/
typedef struct
{
   t_DLComplexSamp burst0246[BURST_SIZE_DL];
   t_DLComplexSamp burst1357[BURST_SIZE_DL];
} t_DLBBData;  
 
/*
* Type used for UL transmission buffers
*/
typedef struct
{
   t_ULComplexSamp burst04[BURST_SIZE_UL_MAX];
   t_ULComplexSamp burst15[BURST_SIZE_UL_MAX-1];
   t_ULComplexSamp burst26[BURST_SIZE_UL_MAX-1];
   t_ULComplexSamp burst37[BURST_SIZE_UL_MAX-1];
   t_ULComplexSamp toaOffset[3];      /* extra buffer for TOA correction */
} t_ULBBData;  

/*
* Buffer for burst builder output and input to modulator.
*/
typedef struct
{
   Bool tx;   /* Transmission Indicator */
   UChar data[MOD_NUM_BITS_IN+RAMP_SAMPLES];
} t_DLBurstBuf;

typedef struct
{
   Short  toa;       /* Time of Arrival: from -2 (NB_TOA_MIN) to +64 (AB_TOA_MAX) */
   Short  snr;       /* Signal to Noise (plus Interference) Ratio */
   Short  rcvPower;  /* Uplink Signal Power Level */
   /*
   * Soft decisions of the bits
   */
   Char  data[MAX_NUM_ENCRYPT_BITS_PER_BURST]; /* Soft decisions of the bits */
} t_ULBurstBuf;

typedef struct
{
   Int  cchRXBurstIndex[NUM_TN_PER_RF];        /* Interleaver burst index */
   /*
   * Soft decisions of the bits
   */
   Char  data[NUM_TN_PER_RF][NUM_BURSTS_PER_CCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST];
} t_ULBurstBuf4;

typedef struct
{
   Int   tchRXBurstIndex[NUM_TN_PER_RF];        /* Interleaver burst index */
   /*
   * Soft decisions of the bits
   */
   Char  data[NUM_TN_PER_RF][NUM_BURSTS_PER_TCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST];
} t_ULBurstBuf8;
                      
typedef struct
{
   Bool  tx;   /* Transmission Indicator */
   UChar data[NB_NUM_ENCRYPT_BITS_PER_BURST];
} t_DLCodedBuf;

typedef struct
{
   Int   cchTXBurstIndex[NUM_TN_PER_RF];       /* Interleaver burst index */
   Char  data[NUM_TN_PER_RF][NUM_BURSTS_PER_CCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST];
} t_DLCodedBuf4;

typedef struct
{           
   Int   tchTXBurstIndex[NUM_TN_PER_RF];       /* Interleaver burst index */
   Char  txBurstCount[NUM_TN_PER_RF];          /* Burst Transmission Indicator */   
   Char  data[NUM_TN_PER_RF][NUM_BURSTS_PER_TCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST];
} t_DLCodedBuf8;

typedef struct
{           
   Int   tchTXBurstIndex[NUM_TN_PER_RF];       /* Interleaver burst index */
   Char  data[NUM_TN_PER_RF][NUM_BURSTS_PER_CCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST];
   Char  txBurstCount[NUM_TN_PER_RF];          /* Burst Transmission Indicator */   
} t_PDCHCodedBuf4;

typedef union
{
   t_DLCodedBuf8
   t_PDCHCodedBuf4;
}u_DLCodecBuf;

typedef struct
{
   Bool bufferValid;    /* Buffer Valid Flag. Indicates buffer ready */
   Uint8         chCodec;
   Uint8         codecFlags;   
   Char data[NUM_CODED_BITS_CCH_FRAME];
} t_DLCCHBuff;  

typedef union
{
   UShort intval;
   struct
   {
      UInt bufferValid  :1;    /* Buffer Valid Flag. Indicates buffer ready */
      UInt sp           :1;    /* Speech Present flag */
      UInt spare        :14;
   } bitval;
} t_DLTCHWord0;

typedef struct
{
   t_DLTCHWord0  dltchword0;
   Char          data[NUM_BITS_TCH_FRAME+MAX_NUM_GPRS_ADD_BITS+4];
} t_DLTCHBuff;

typedef union
{
   UShort intval;
   struct
   {
      UInt bufferValid   :1;    /* Buffer Valid Flag.  Indicates buffer ready */
      UInt spare         :8;
      UInt bfi           :1;
      UInt spare2        :3;
      UInt fireStatus    :3;    /* Status of Fire Code: 0-no error, */
                                /* 1-error detected, 2-error corrected, 3-reserved */
   } bitval;
} t_ULCCHWord0;

typedef union
{
   UShort intval;
   struct
   {
      UInt bufferValid  :1;    /* Buffer Valid Flag. Indicates buffer ready */
      UInt parityStatus :1;    /* TCH frame CRC bit field */
      UInt bfi          :1;    /* Bad Frame Indicator */
      UInt sid          :2;    /* Silence Frame Indicator */
      UInt mask         :1;    /* Perceptual Masking Indicator */
      UInt spare        :10;
   } bitval;
} t_ULTCHWord0;

typedef union
{
   UShort intval;
   struct
   {
      UInt bufferValid   :1;    /* Buffer Valid Flag.  Indicates buffer ready */
      UInt rachDet       :1;    /* RACH Detection Flag */
      UInt spare         :13;
      UInt bfi           :1;    /* Bad Frame Indication */
   } bitval;
} t_ULRACHWord0;

typedef struct
{
   t_ULCCHWord0   ulcchword0;
   UShort         nErrs;         /* Estimated errors in 1 FACCH frame (by chanDecoder) */
   UShort         metric;        /* Viterbi "Best Path Distance Metric" */
   UShort         T1;            /* Records the TDMA Frame number where frame was */
   UChar          T2;            /* completely received */
   UChar          T3;
   Uint8          chCodec;
   Uint8          codecFlags;   
   UChar          data[NUM_CODED_BITS_CCH_FRAME];
} t_ULCCHBuff;

typedef struct
{
   UInt           errorPatternReg;         /* Fire Decoder Error Pattern Register */
   UInt           errorLocationReg;        /* Fire Decoder Error Location Register */
   UChar          fireStatus;              /* Fire Decoder Processing Status Flag */
   UChar          rfCh;
   UChar          timeslot;            
   UChar          subChan;
   UChar          chType;
   UChar          data[NUM_BITS_CCH_FRAME];
} t_ULCCHFireBuff;

typedef struct
{
   t_ULTCHWord0  ultchword0;
   UShort        nErrs;          /* Estimated errors in 1 TCH frame (by chanDecoder) */
   UShort        metric;         /* Viterbi "Best Path Distance Metric" */
   Char          data[NUM_BITS_TCH_FRAME+MAX_NUM_GPRS_ADD_BITS+4];
} t_ULTCHBuff;

typedef struct
{
   t_ULRACHWord0  ulRACHword0;
   UShort         nErrs;         /* Estimated number of errors by Channel Decoder */
   UShort         metric;        /* Viterbi "Best Path Distance Metric" */
   UShort         T1;            /* Records the TDMA Frame number where frame was */
   UChar          T2;            /* completely received */
   UChar          T3;
   UChar          data[NUM_BITS_PRACH_FRAME];
   UChar          spare[NUM_BITS_CCH_FRAME-NUM_BITS_RACH_FRAME];
} t_ULRACHBuff;


#endif  /* end of include once only */

