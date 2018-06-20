/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: gsmtypes.h
*
* Description:   
*   This file contains defined constants, enumerations and type definitions
*   of shared items to support GSM Layer 1 processing.
*
*****************************************************************************/
#ifndef _GSMTYPES_H_
#define _GSMTYPES_H_  /* include once only */

#include "dsp/dsptypes.h"
                          
#define NUM_TS_PER_RF   (8)   /* Number of timeslots per RF   */                                            
#define NUM_RFS_PER_DSP (1)   /* Number of RFs within one DSP */   

#define MS_TXPWR_MAX_DEF   0  /* in 2 dB units, i.e. 3  = 3*2 dB = 6dB */
#define MS_TXPWR_MIN_DEF  15  /* ditto                                 */

#define T1_FRAMES 2048
#define T2_FRAMES 26
#define T3_FRAMES 51

#define FRAME_COUNT_TOO_BIG   (2715648L)  /* Maximum frame number */

#define  R  5   /* R for RACH frame in Group 5 Channel Map Table (burstisr.c) */
               
typedef enum {
   GSM_900 = 0,
   DCS_1800 = 1,
   PCS_1900 = 2
} airInterfaceType;

/*
* Logical channel types
*/
typedef enum {
   IDLE,
   TCHF,
   SACCH,
   SACCH4,    /* last sacch burst in a sequence of 4*/
   SACCH8,    /*  SACCH/C8 type for Configuration 7 */
   SDCCH,     /* first sdcch "     "     "       "  "  */
   SDCCH4,     
   SDCCH8,
   TCHH,
   BCCH,
   PCH,
   AGCH,
   PAGCH = AGCH,
   SCH,
   RACH,
   FACCH,     
   FCCH,
   SACCH_OR_IDLE,  /* Can be idle or SACCH, control code will determine */
   CBCH,
   CBCH_EXT,
   PDCH,
   PDCH4,       /* the last burst of the RLC block */
   PACCH,
   PTCCH,
   PDTCH,
   PRACH,
   PPAGCH,    /* GROUP_11, GROUP_12 ---> Packet access grant */
   PPCH,       /* GROUP_11, GROUP_12 ---> Packet paging       */
   PBCCH
} t_chanSel;

typedef enum {
   CHAN_INACTIVE = 0,   /* The channel is not activated */
   CHAN_ACTIVE          /* The channel is activated */
} t_chanState;

typedef enum {
   IND_SPEECH  = 1,     /* speech */
   IND_DATA    = 2,     /* data */
   IND_SIG     = 3      /*signalling */        
} t_speechDataInd;

typedef enum {
   TYPE_SDCCH  = 1,     /* sdcch */
   TYPE_TCHF   = 8,     /* tch full rate (Bm) */
   TYPE_TCHH   = 9      /* tch half rate (Lm) */        
} t_rateType;

typedef enum {
   VOC_NULL = 0,     /* no active vocoder */
   VOC_GSMF = 1,     /* GSM Full rate */
   VOC_EFR  = 2      /* GSM EFR */        
} t_vocAlgo;

typedef enum {
   RATE_9_6T_12   = 16, /* 9.6/12 kbit/s transparent/non-transparent */
   RATE_4_8T_6    = 17, /* 4.8/6 kbit/s transparent/non-transparent */
   RATE_2_4T      = 18, /* 2.4 kbit/s transparent */
   RATE_1_2T      = 19, /* 1.2 kbit/s transparent */
   RATE__6T       = 20, /* 0.6 kbit/s transparent */
   RATE_1_2D_75UT = 21  /* 1.2 kbits/s downlink, 75 bits/s uplink */
} t_dataRate;

typedef enum {
   NON_TRANSPARENT   = 0,  /* non-transparent data*/
   TRANSPARENT       = 1   /* non-transparent data*/
} t_transMode;

typedef struct {
   UInt     FN;         /* GSM TDMA frame number, 0 - 2715647 (22 bits) */
   UShort   t1;         /* 11 bits, 0 -> 2047  = FN div (26*51) */
   UChar    t2;         /* 5 bits, 0 -> 25 = FN mod 26 */
   UChar    t3;         /* 6 bits, 0 -> 50 = FN mod 51 */
   UChar    TNNumInit;  /* Uplink timeslot number initial value */
   Uchar    ulFNOffset;
} t_frameCounts;

typedef struct {
   Uint32 FN;
   Uint16 t1;
   Uint8  t2;
   Uint8  t3;
} t_GsmFrameNum;

typedef enum {
   SET_SYNC,
   CHECK_SYNC,
   TS_RE_SYNC,   
   OUT_OF_SYNC
} t_frameSyncState;

typedef struct {
   t_frameSyncState state;
   Uint8 newPageState;
   Uint8 newBSPaMfrms;
} t_frmSync;

typedef enum {
   /*
   * Sys Info types for BCCH
   */
   TYPE_8 = 0,
   TYPE_1,
   TYPE_2,   
   TYPE_3,
   TYPE_4,
   TYPE_2_BIS,
   TYPE_2_TER,                                  
   TYPE_7,   
   TYPE_9,
   TYPE_10,
   TYPE_11,            
   TYPE_13,   
   TYPE_14,    
   TYPE_15,   
   /*          
   * Sys Info types for SACCH
   */
   TYPE_5,
   TYPE_6,                                
   TYPE_5_BIS,
   TYPE_5_TER,

   PACKET_TYPE_1,
   PACKET_TYPE_2,
   PACKET_TYPE_3,
   PACKET_TYPE_3BIS,   
   PACKET_TYPE_4,
   PACKET_TYPE_5,
   PACKET_TYPE_13,   
   NUM_SYSINFO_BUFS  /* Defines how many sysinfo buffers exist */
} t_sysInfoBufIndex;



typedef enum {
   SYS_INFO_8 = 0,
   SYS_INFO_1 = 1,   
   SYS_INFO_2 = 2,   
   SYS_INFO_3 = 3,                  
   SYS_INFO_4 = 4,
   SYS_INFO_5 = 5,   
   SYS_INFO_6 = 6,   
   SYS_INFO_7 = 7,   
   SYS_INFO_13 = 13,

   SYS_INFO_2BIS = 0x10,                          
   SYS_INFO_2TER = 0x11,  
   SYS_INFO_5BIS = 0x12,                          
   SYS_INFO_5TER = 0x13,

   /*
   *  GSM 4.60-720  11.2.0.1
   */
   PSI_1 =     0x31,
   PSI_2 =     0x32,
   PSI_3 =     0x33,
   PSI_3BIS =  0x34,
   PSI_4 =     0x35,   
   PSI_5 =     0x36,            
   PSI_13 =    0x37
} t_sysInfoCode;
                                 
#define DL_SIG_Q_DEPTH              2
#define DL_SIG_MSG_SIZE             23 /*TBD */
#define NUM_SDCCH_TS_PER_RF         8
#define NUM_SACCH_TS_PER_RF         8
#define NUM_PCH_TS_PER_RF           1
#define NUM_TN_PER_RF               8  

#define NUM_BCCH_SUBCHANS           NUM_SYSINFO_BUFS


#define NUM_SDCCH_SUBCHANS          4  /* USE 4 for Group V, 8 is the maximum */
#define NUM_SACCH_SUBCHANS          4  /* USE 4 for Group V, 8 is the maximum */
#define NUM_FACCH_SUBCHANS          1
#define NUM_SACCH8_SUBCHANS         8  /* USE 8 for Group VII, 8 is the maximum */
#define NUM_SDCCH8_SUBCHANS         8  /* USE 8 for Group VII, 8 is the maximum */


#define MAX_BS_PA_MFRMS             9
#define PAGE_BLOCKS_G5              3
#define NUM_PAGCH_SUBCHANS          (MAX_BS_PA_MFRMS * PAGE_BLOCKS_G5)

#define BS_PBCCH_BLKS_PLUS_BS_PAG_BLKS_RES  11
#define NUM_PPCH_SUBCHANS         (12-BS_PBCCH_BLKS_PLUS_BS_PAG_BLKS_RES)*64
#define USF_FREE                   0x7  // USF indicates a PRACH in the
                                        // block 


typedef struct {
   Uint8    readIndex;     /* Next buffer to read by the asynch code */
   Uint8    writeIndex;    /* Next buffer to write by the synch code */ 
   Uint8    numMsgsReady;  /* Number of pending messages to read */
   Bool     overflow;      /* Set to TRUE is buffer has been overloaded */
   Bool     valid;         /* Set to true when L3 has initialized */          
   Uint8    data[DL_SIG_Q_DEPTH][DL_SIG_MSG_SIZE];
   Uint8    tag[DL_SIG_Q_DEPTH];
   Uint8    burstType[DL_SIG_Q_DEPTH];     
} t_DLSigBufs;


/*
*   t_DLPPCHBufs is the global pooled resource buffer used 
*   for PCH and AGCH. In the future, PPCH and PPAGCH will use it too.
*/
typedef struct {
   Uint8    msgReady;      /* Pending message */
   Uint8    timeslot;      /* one global buffer for all (P)CCCH timeslots */
   Uint8    logCh;       
   Uint8    pageNum;       /* paging index */
   Bool     valid;         /* Set to true when L3 has initialized */          
   Uint8    data[DL_SIG_MSG_SIZE];
} t_DLPPCHBufs;
    
                             
/*
*  Those message buffers with "RTS Flow control", uses single buffer to save
*  memory
*/    
#define DL_SIG_QUEUE_SIZE 1       
typedef struct {
   Uint8    readIndex;     /* Next buffer to read by the asynch code */
   Uint8    writeIndex;    /* Next buffer to write by the synch code */ 
   Uint8    numMsgsReady;  /* Number of pending messages to read */
   Bool     overflow;      /* Set to TRUE is buffer has been overloaded */
   Bool     valid;         /* Set to true when L3 has initialized */          
   Uint8    data[DL_SIG_QUEUE_SIZE][DL_SIG_MSG_SIZE];
} t_DLSigBuf;

#define SCH_INFO_Q_DEPTH   2  /* SCH downlink message queue size */
#define SCH_INFO_MSG_SIZE  4  /* SCH downlink message buffer size */
                
/*
* downlink SCH buffer
*/
typedef struct {
   UChar   data[SCH_INFO_Q_DEPTH][SCH_INFO_MSG_SIZE];
   UChar   state;           /* Indicates buffer to transmit from */  
   UInt    frameNumUpdate;  /* Indicates the frame number to switch buffers */
   Bool    update;          /* Set to true when an update is scheduled */
   Bool    valid;           /* Set to true when L3 has initialized */   
} t_SCHSigBufs;
                
typedef struct {
   t_SCHSigBufs *dlSigBuf; /* Define the downlink SCH signaling buffers */
} t_SCHInfo;
        
#define SYS_INFO_Q_DEPTH   2
#define SYS_INFO_MSG_SIZE  25

typedef struct {
   Uint32   frameNumUpdate;   /* Indicates the frame number to switch buffers */
   Uint8    data[SYS_INFO_Q_DEPTH][SYS_INFO_MSG_SIZE];
   Uint8    state;            /* Indicates buffer to transmit from */   
   Bool     update;           /* Set to true when an update is scheduled */
   Bool     valid;            /* Set to true when L3 has initialized */          
} t_sysInfoQueue;


typedef struct {
   Uint8    data[4][SYS_INFO_MSG_SIZE];
   Uint8    bufValid;            /* Indicates buffer empty */
} t_CBCHQueue;

#define UL_SIG_Q_DEPTH  2  
#define UL_SIG_MSG_SIZE 23    /* 23 octet message */
     
/*
* Uplink signaling buffers
*/
typedef struct {
   UChar     chanSel;              /* Channel type identifier */
   UChar     subchannel;           /* Subchannel identifier */
   UChar     data[UL_SIG_MSG_SIZE];/* UL frame data */
   UShort    T1;                   /* Frame number T1 */
   Uchar     T2;                   /* Frame number T2 */
   Uchar     T3;                   /* Frame number T3 */
} t_ULSigMsg;

typedef struct {
   t_ULSigMsg  msg[UL_SIG_Q_DEPTH];

   UChar       readIndex;      /* Next buffer to read by the asynch code */
   UChar       writeIndex;     /* Next buffer to write by the synch code */ 
   UChar       numMsgsReady;   /* Number of pending messages to read */
   Bool        overflow;       /* Set to TRUE if buffer has been overloaded */
} t_ULSigBufs;
 
typedef enum {
   SYS_INFO_SACCH = 0,
   DL_DCH_SACCH
}  t_sacchState;


#define NUM_RX_SIGNAL_SAMPLES_MAX 32  /* gsm 05.08 default */

typedef struct {
   Int8     rxLev[NUM_RX_SIGNAL_SAMPLES_MAX];/* Power from uplink (MS) */
   Uint8    count;                           /* for rxLev circular buffer */
   Int8     rxLevAve;                        /* Average uplink power for rxLev array  */
   Int16    rxPwrNormSmooth;      /* normalized, rcv pwr output from smoothing filter */
   Uint16   rxgain;     /* 10-bit result of ul agc used to set ul gain FPGA resgister */
} t_ULSignalStrength;

typedef struct {
   Uint8   rxQual[NUM_RX_SIGNAL_SAMPLES_MAX]; /* uplink RXQUAL of gsm 05.08 */
   Uint8   count;        /* for rxQual[] circular buffer */
   Uint16  ber;          /* calculated value used for reporting, handover purposes */
   Uint16  berAve;       /* Averaged BER using a resolution of 0.01% for voice, 0.001% for GPRS */
   Uint8   rxQualAve;    /* converted from berAve */    
   Uint16  blerAve;      /* Exponentially averaged RLC block errors, GPRS only! */
} t_ULSignalQuality;


typedef struct {
    Uint8   MSPwrSetting;    // !! set from syncmsg.c 
    Int16   rxLevNew; 

    Int8    rxLev[NUM_RX_SIGNAL_SAMPLES_MAX];
    Uint8   count;   
    Int8    rxLevAve;    
} t_DLSignalStrengthSCell;  // Serving Cell

typedef struct {
    Uint8   rxQualNew; 

    Uint8   rxQual[NUM_RX_SIGNAL_SAMPLES_MAX];
    Uint8   count;  
    Uint16  berAve;
    Uint8   rxQualAve;
} t_DLSignalQualitySCell;        // Serving Cell      


/*
*   GPRS Related structures and definitions
*/

typedef struct {
   Int8     toa;          /* accumulation of time-of-arrival of all bursts */
   Int8     toaSub;       /* accumulation of time-of-arrival of sub bursts */
   Int8     toaLastGood;       /* Last toa of a "good" quality burst */
   Int8     toaLastGoodActual; /* Last actual toa of  "good" quality */
   Int16    MSPower;      /* accumulated uplink power (set to Sub or Full, see below) */
   Int16    MSPowerSub;   /* TBD - uplink power on SACCH plus potential SID bursts */
//   Int16    MSPowerFull;  /* TBD - uplink power on all TCH bursts */
   Uint16   bitErrors;     /* accumulation of bit errors on one RLC block */
   Uint16   bitCount;
   Uint16   blockErrors;   /* RLC block errors per RLC block (0% or 100%) */
} t_ULAccum;

typedef struct {
  Uint8  C_VALUE; /* GPRS, the normalized rec'd signal level at the MS (RXLEV units) */
  Uint8  RXQUAL;  /* GPRS, downlink signal quality calculated by the MS */
  Uint8  iLevelTN[NUM_TS_PER_RF];    /* GPRS, interference relative to C value, valid only in pkt. xfer mode */
  Uint16 ber;     /* derived from RXQUAL, used for DL code rate monitoring */
  Uint16 berAve;  /* derived from RXQUAL, used for DL code rate monitoring */
} t_PacketDownlinkAckNackInfo;

#define MAX_TFI 32
#define GPRS_MULTI_FRAMES 52
#define BLOCKS_PER_GPRS_MULTI_FRAMES 12

#define PTCCH_FRAMES_PER_GPRS_MULTI_FRAMES 2 
#define IDLE_FRAMES_PER_GPRS_MULTI_FRAMES  2 
#define PTCCH_FRAMES_PER_TA_MSG            PTCCH_FRAMES_PER_GPRS_MULTI_FRAMES*2
#define HALF_TA_MSG_SIZ                    PTCCH_FRAMES_PER_TA_MSG/2

typedef struct {
    Uint8 blkOrPeriod;
    Uint8 numBlocks;
    Uint8 bitMapLen;
    Uint8 bitMapAlloc[16];   /*1-128 bit map allocation*/
    Uint32 startTime;
} t_fixedMode;


typedef struct {
    Uint32 startTime;
} t_singleBlkMode;

#define SBA_TAG    0xff

typedef struct {
   Uint32 usfTsMap;
} t_dynamicMode;

typedef union {
   t_singleBlkMode sBlk;
   t_fixedMode     fixed;
   t_dynamicMode   dynamic;
} u_tbfOpMode;

typedef struct {      
   t_chanState state;
   Uint8       tfi;
   Uint8       tbfMode;
   Uint8       timeSlotAllocation;   
   Uint8       channelCodec;
   Uint8       tai;
   Uint8       taiTimeSlot;
   Uint8       timingAdvance;
   Uint8       msPower;
   u_tbfOpMode opMode;
   t_ULSignalStrength  sigstren;
   t_ULSignalQuality   sigqual;
   t_ULAccum   accum;
} ulTbfStruct;   

typedef struct {         
   t_chanState  state;
   Uint8  tfi;
   Uint8  timeSlotAllocation;
   Uint8  channelCodec;
   Uint8  timingAdvance;   /* TBD - put back in later ??? */
   Uint8  msPower;
   Uint8  bsPower;
   Uint32 startTime;   
   t_PacketDownlinkAckNackInfo   pktDlAckNack;
}  dlTbfStruct;
 
typedef struct {
   Uint32       TLLI;    
   ulTbfStruct ulTbf;
   dlTbfStruct dlTbf;
} tbfStruct;
                
                 
/*5.2.1.5.9 SACCH Channel Information
The following data is associated with the sacch measurement reports.*/

/* ********************************************************************************** */
/* To get one SACCH burst takes 120ms (26 TDMA frames)                                */
/* For Group I, this occurs on all 8 timeslots.                                       */
/* 1 SACCH frame = 4 SACCH bursts = 480 ms.                                           */
/* ********************************************************************************** */
typedef struct {
   t_chanState     state;
   Uint8           sysInfo;   
   t_DLSigBufs     *dlSigBuf;       /* Define downlink signaling buffers; no longer use t_MSHeader */
   Int8            powerLevel;     /* header that gets transmitted to the */
   Int8            timingAdvance;  /* header that gets transmitted to the */
   Int8            powerLevelInit; /* header that gets transmitted to the */
   Uint8           lostCount;      /* SACCH consecutiveness to determine loss of RF link */

   UShort     bitErrorAccum;     /* accumulation of bit errors on all bursts */
   UShort     bitCountAccum;     /* accumulation of bits checked for bit errors */
   Uint8      frameErrorAccum;   /* accumulation of frame errors on non-DTX bursts */
   Uint8      frameCountAccum;   /* accumulation of frames checked for frame errors */
   Int16      MSPowerAccum;      /* accumulated uplink power (set to Sub or Full, see below) */
   Int16      MSPowerAccumSub;   /* uplink power on SACCH plus potential SID bursts */
   Int16      MSPowerAccumFull;  /* uplink power on all TCH bursts */
   Int8       toaAccum;          /* accumulation of time-of-arrival of all bursts */
   Int8       toaAccumSub;       /* accumulation of time-of-arrival of sub bursts */
   Int8       toaLastGood;       /* Last toa of a "good" quality burst */
   Int8       toaLastGoodActual; /* Last actual toa of  "good" quality */
   Uint8      reportNum;         /* Modulo 256 of the number of meas reports */
   Uint8      noNCellM;          /* NO-NCELL-M is the number of neighbouring cell measurments 
                                  * in the decoded UL sacch measurement report (gsm 04.08) */
   Uint8      dynPwrRprtCntr;    /* Counter for dyn. pwr control reports to host */ 

   Uint16     IntRxLevAccum;     /* RxLev accumulated value for measuring interference */
   Uint16     IntRxLev;          /* RxLev value averaged within the SACCH multiframe */

   t_ULSignalStrength       ulsigstren;
   t_ULSignalQuality        ulsigqual;
   t_DLSignalStrengthSCell  dlsigstrenSCell;   /* Serving Cell */
   t_DLSignalQualitySCell   dlsigqualSCell;
  
   Uint8      agcHoldFlag;        /* used during period between CH_ACT and actual traffic flow per TS */
   Uint8      dpcHoldFlag;        /* used during period between CH_ACT and RACH demod/decode */
   Uint8      averagingHoldCounter;  /* does not allow 5.08 averaging until all samples are collected */
} t_SACCHInfo;

/*
**  t_groupNum defines the possible groups available in GSM.  
*/
typedef enum {
   GROUP_NULL = 0,            /* No group is activated */
   GSM_GROUP_1,               /* GSM Group 1 */
   GSM_GROUP_2,               /* GSM Group 2 */
   GSM_GROUP_3,               /* GSM Group 3 */
   GSM_GROUP_4,               /* GSM Group 4 */
   GSM_GROUP_5,               /* GSM Group 5 */
   GSM_GROUP_6,               /* GSM Group 6 */
   GSM_GROUP_7,               /* GSM Group 7 */
   GSM_GROUP_8,               /* GSM Group 8 */
   GSM_GROUP_9,
   GSM_GROUP_10,
   GSM_GROUP_11,
   GSM_GROUP_12,
   GSM_GROUP_13
} t_groupNum; 

#define  MAX_BCCH_TC_VALUE    (8)
#define  MAX_NUM_ALLOCATIONS  (2)

typedef enum {
   BCCH_NORM   = 0,   
   BCCH_EXT    = 1
} t_BCCHAlloc;
    
/*
**  Encryption structures
*/
 
#define  ENCRYPT_KEY_LEN   2     /* Encryption key length in longwords */
#define  NUM_ENCRYPT_BITS  114   /* Per burst for one encryption stream */

typedef enum {
   ENCR_NORMAL = 0,        /* Normal state, "algorithm" is running */
   ENCR_CHANGE,            /* "newAlgorithm" is running on UL only */
   ENCR_PEND_CHANGE        /* Change to "newAlgorithm" at next frame */
} t_encryptMode;

typedef enum {
   ENCR_NONE = 0,       /* No encryption */
   ENCR_A51,            /* GSM A5-1 */
   ENCR_A52             /* GSM A5-2 */
} t_encryptAlgorithm; 

typedef struct {
   t_encryptAlgorithm   algorithm;                       /* algorithm in use */
   t_encryptAlgorithm   newAlgorithm;                    /* algorithm in use next */   
   t_encryptMode        encryptMode;                     /* Encryption state */ 
   Uint32               encryptKey[ENCRYPT_KEY_LEN];     /* Encryption key */
/*  new 1/22/01..remove from this offchip structure and put back on-chip in another gsmdata.h buffer*/
/*  UChar                ulEncryptStream[NUM_ENCRYPT_BITS]; */  /*To be applied to UL */
} t_encryptState;


/*5.2.1.6 Control Channel Information Structure
This structure contains all of the channels assosiated with control.
It is a pointer to enable the use of dynamic memory allocation "malloc" to 
configure the structure if needed.  For instance, for group V, 
SDCCHInfo,BCCHInfo and PAGCHInfo would be allocated.  For group I, 
they would not be allocated.
*/

typedef struct {
   t_chanState    state;         /* Channel activation state */
   t_DLSigBufs    *dlSigBuf;     /* Downlink signaling buffers */ 
   UChar          HOReference;   /* Key used to match access burst content */ 
   Bool           HOFlag;        /* Look for access burst (T) or normal (F) */               
   t_encryptState encryptState;  /* All the parameters for encryption process */
   UShort         DLPower;       /* Value for downlink dynamic power control */
   UShort         modPower;      /* Value for modulator output power level */
} t_SDCCHInfo;
typedef struct {
   Uint8    mode;          /* TRAU or VoIP mode */
   Uint8    *buf[3][40];   /* buffer declared external */
} t_netInterface;                    

#define MAX_NUM_NCELL_CANDIDATES 6

typedef struct {
  Uint8     size;                             /* if no candidates exist, size is 0 */
  Int16     rxLev[MAX_NUM_NCELL_CANDIDATES];  /* scratchpad */
  Uint8     hoReportCnt; /* enables HO msg reports AT LEAST 2 secs after last report to RRM */
} t_hoCanBuf;

typedef struct {
  Uint16    rfCh;       /* ARFCN value copied from g_HndOvrOAMInfo */
  Uint8     oamIdx;     /* mapped to ARFCN value in host SW */
  Uint8     bsic;       /* bsic value copied from g_HndOvrOAMInfo  */
  Int16     rxLev;
  Uint8     candidate;  /* Flag defaults to FALSE (=0) */
} t_NCELLInfo;  // list of allowable, adjacent Neighbor CELLs for handover

#define HO_REPORT_COUNTER  8
#define NXBUF_SIZ_MAX      32

typedef struct {
   Uint8    index;       /* current index into memory for px-nx history, where passes are flaged */ 
   Uint8    flag[NXBUF_SIZ_MAX];  /* actual size is parameterizable */
   Uint8    flagsum;     /* used for px-nx test */
} t_nxBuf;    /* circular buffers for px/nx tests for handover, power control */

typedef struct {
   t_nxBuf  sigstrenHi;
   t_nxBuf  sigstrenLo;
   t_nxBuf  sigqualHi;  
   t_nxBuf  sigqualLo;
} t_dpcULbuf;

typedef struct {
   t_nxBuf  sigstrenSCellHi;    // Serving Cell
   t_nxBuf  sigstrenSCellLo;  
   t_nxBuf  sigqualSCellHi;  
   t_nxBuf  sigqualSCellLo;
} t_dpcDLbuf;

typedef struct {  
   t_nxBuf  sigstren;    
   t_nxBuf  sigqual;
   t_nxBuf intsigstren;
} t_hoULbuf;

typedef struct {  
   t_nxBuf  sigstrenSCell;    // Serving Cell
   t_nxBuf  sigqualSCell;
   t_nxBuf intsigstrenSCell;
} t_hoDLbuf;

typedef struct {
   t_dpcULbuf dpcUL;
   t_dpcDLbuf dpcDL;
   t_hoULbuf  hoUL;
   t_hoDLbuf  hoDL;
} t_PNbufs;       

typedef struct {
   t_chanState       state;         /* channel activation state */
   Bool              ULDtxMode;     /* determine if uplink dtx is on or off */
   Bool              DLDtxMode;     /* determine if downlink dtx is on or off */
   Bool              ulDtxUsed;     /* indicates if uplink DTX is used currently*/
   Bool              ulDtxUsed104;  /* whether UL DTX was used ever in 104-frame block*/
   Bool              dlDtxUsed;     /* indicates if downlink DTX is used currently*/
   Uint8             stolenSid;  /* SID frames stolen by FACCH */
   t_speechDataInd   speechDataInd; /* indicates speech vs. data */
   t_vocAlgo         vocAlgo;       /* vocoder in use for speechDataInd=IND_SPEECH */
   t_transMode       transMode;     /* transparent vs non for speechDataInd=IND_DATA */
   t_dataRate        dataRate;      /* radio data rate, may depend on transMode */
   UChar             HOReference;   /* Key used to match access burst content */ 
   Bool              HOFlag;        /* Look for access burst (T) or normal (F) */               
   t_encryptState    encryptState;  /* All parameters for encryption process */
   Uint16            DLPower;       /* Value for downlink dynamic power control */
   Uint16            modPower;      /* Value for modulator output power level */
   /* the following are related to handover */
   t_NCELLInfo    NCell[MAX_NUM_NCELL_CANDIDATES]; /* Neighbor cell info for HO */
   t_hoCanBuf     hoCandidateBuf;    /* Buffer to be sent back to layer 3 RRM */
   t_PNbufs       *PNbufs;    
} t_TCHInfo;

#define NUM_TCH_JBUF 3
                
typedef struct
{                         
   Uint32   buffer[NUM_TCH_JBUF][14]; 
   Uint16   rtpSeqNum;  
   Int8     frameCount;    /* Number of TCH messages to send */
   Int8     readIndex;     /* Next buffer to read by the asynch code */
   Int8     writeIndex;    /* Next buffer to write by the synch code */ 
   Uint8    frameStarted;  /* Indicates first fill-up after activation */
   Uint8    tag[NUM_TCH_JBUF];
   Uint8    burstType[NUM_TCH_JBUF];
   Uint8    chCodec[NUM_TCH_JBUF];         
} dlTchFrameStruct, t_dlPdtchBufs;


/*5.2.1.5.8 FACCH Channel Information*/

typedef struct {
   t_DLSigBufs *dlSigBuf;     /* Define the downlink signaling buffers */
} t_FACCHInfo;

typedef struct {
  t_sysInfoQueue *dlSigBuf;      /* Define the downlink signaling buffers */
} t_BCCHInfo;



typedef struct {
  t_CBCHQueue *dlSigBuf;      /* Define the downlink signaling buffers */
} t_CBCHInfo;

typedef struct {
  t_DLSigBufs *dlSigBuf;      /* Define the downlink signaling buffers */
} t_PACCHInfo;

typedef struct {
  t_DLSigBufs   *dlSigBuf;  /* Define the downlink signaling buffers */
} t_PTCCHInfo;                           

typedef struct {
  t_dlPdtchBufs *dlSigBuf;    /* Define the downlink signaling buffers */  
} t_PDTCHInfo;

         
typedef struct {            
   /*
   * point to sacch idle function table
   * need to be initialized on frame synchronization time.
   */
   Uint32 dlIndex;
   Uint32 ulIndex;
   Uint32 *dlSachIdleFunc;
   Uint32 *ulSachIdleFunc;
   
   t_TCHInfo                        tch;
   t_SACCHInfo                      sacch;
   t_FACCHInfo                      facch;
} t_groupIInfo;

typedef struct {
   t_BCCHInfo     bcch;
   Uint8          bcchBufIndex;
                      
   t_SCHInfo      sch;
   
   t_CBCHInfo     cbch[2];
   Uint8          dlSmscbTB;    /*TB = (FN DIV 51)mod(8)*/
   Bool           cbchUsed;     /* if it is SDCCH or CBCH */

   t_SDCCHInfo    sdcch[NUM_SDCCH_SUBCHANS];
   Uint8          dlSdcchSubCh;

   t_SACCHInfo    sacch[NUM_SACCH_SUBCHANS];
   Uint8          dlSacchSubCh;

   Uint8          ppchOverflow;
   Uint8          dlPpchSubCh;  

   Uint8          ulSdcchSubCh;
   Uint8          ulSacchSubCh;
} t_groupVInfo;
  


typedef struct {
   t_SDCCHInfo    sdcch[NUM_SDCCH8_SUBCHANS];
   Uint8          dlSdcchSubCh;
   t_SACCHInfo    sacch[NUM_SACCH8_SUBCHANS];
   Uint8          dlSacchSubCh;
   Uint8          ulSdcchSubCh;
   Uint8          ulSacchSubCh;
} t_groupVIIInfo;


#define MAX_RADIO_BLOCKS 12

/*
*   In the uplink Dsp processing, for all three cases below, 
*   assume the block is set to RRB or DLA.
*
*    1. DL PDTCH, DL PACCH - unacknowledged mode - PACKET CONTROL ACK (MS->GP10 sent as access bursts)
*
*    2. DL PACCH block - acknowledged mode - PACKET CONTROL ACK (MS->GP10 sent as access bursts)
*        , otherwise sent as normal bursts.
*
*    3. DL PDTCH - acknowledged mode - PACKET DOWNLINK ACK/NACK (MS->GP10 sent as normal bursts)
*
*/


typedef enum {
   NONE = 0,
   SBA,            /* Uplink (Single Block) Assignment */
   DLA,            /* Downlink Assignment */
   RRB_NB,         /* Relative Reserved Block for NORMAL_BURST */
   RRB_AB,          /* Relative Reserved Block for ACCESS_BURST */
   GRP13_AB_FOUND,  /*  Transition state from DLA to NONE */
   LAST_CHANCE_4_AB_DET   /* Transition state set (and cleared) on ulPdchProc3() */
} t_tagFillCause; 

typedef struct { 
  tbfStruct      *tbf; 
  Uint8          tag;
  t_tagFillCause tagFillCause;
}t_blkNumWatch;


typedef struct{
        Uint8 bsPBcchBlks;   /*  # of blocks for PBCCH (1-4) */
        Uint8 bsPagBlksRes;  /*  # of blocks that are not PBCCH or Ppagch */
        Uint8 bsPrachBlks;   /*  # if blocks that are reserved forthe PRACH */
        Uint8 newAssignment; /* flag indicating a new assignment is pending */
        Uint8 psi1_repeat;     
        unsigned short psi1_cycle;   /*  counter used to indicate a new PSI1 cycle */
}  t_group11_params;



#define MAX_GPRS_PAGING 11


/* Define a type for each of the uplink bursts */

#define U_PRACH_T     0
#define U_PACCH_T     1
#define U_PDCH_T      2

/* Define a type for each of the downlink bursts */

#define D_PBCCH_T    3
#define D_PACCH_T    4
#define D_PDCH_T    5
#define D_PPCH_T     6
#define D_PAGCH_T    7

           
typedef struct 
{
        Uint8          currentUsf;
        Uint8          nextUsf;   
        Uint8          dlBlock;           /* B0 - B11 of 52 multi frame */
        Uint8          ulBlock;           /* B0 - B11 of 52 multi frame */
        Uint8          currentTAI;      /* TAI in use */
        t_blkNumWatch  blkNumWatch[MAX_RADIO_BLOCKS];
                        /* block number (B0-B11) is derived from FN1,
                         *  the calculated startTime with possible 
                         *  added delay */
        tbfStruct      *tbf;
        t_PACCHInfo    pacch;
        t_PDTCHInfo    pdtch;
        Uint8          dlPtcchIdleIndex;
        Uint8          ulPtcchIdleIndex;
        Uint8          pbcchBufIndex;
        t_BCCHInfo     pbcch;
        t_PTCCHInfo    ptcch;
        Uint8          ppchOverflow;
        Uint8          dlPpchSubCh;
} t_groupXIInfo;

typedef struct {
   Uint8          currentUsf;
   Uint8          nextUsf;   
   Uint8          dlBlock;           /* B0 - B11 of 52 multi frame */
   Uint8          ulBlock;           /* B0 - B11 of 52 multi frame */
   Uint8          currentTAI;      /* TAI in use */


   t_blkNumWatch  blkNumWatch[MAX_RADIO_BLOCKS];   /* block number (B0-B11) is derived from FN1,
                                                   *  the calculated startTime with possible 
                                                   *  added delay 
                                                   */
   tbfStruct      *tbf;  

   t_PACCHInfo    pacch;  
   t_PDTCHInfo    pdtch;  
   
   Uint8          dlPtcchIdleIndex;
   Uint8          ulPtcchIdleIndex;   
   t_PTCCHInfo    ptcch;

} t_groupXIIIInfo;      

typedef struct {
   Uint8          currentUsf;
   Uint8          dlBlock;           /* B0 - B11 of 52 multi frame */
   Uint8          ulBlock;           /* B0 - B11 of 52 multi frame */
   Uint8          currentTAI;      /* TAI in use */
   
   tbfStruct      *tbf;  

   t_PACCHInfo    pacch;
   Uint8          dlPtcchIdleIndex;
   Uint8          ulPtcchIdleIndex;        
   t_PTCCHInfo    ptcch;
   t_PDTCHInfo    pdtch;  
} t_groupXIIInfo;      

typedef struct {
   t_BCCHInfo     bcch;
   t_SDCCHInfo    sdcch;
   t_SACCHInfo    sacch;
}t_groupIVInfo;

typedef struct {
   Long I;
   Long Q;
} t_LongComplexSamp;
   
typedef struct {
   Short I;
   Short Q;
} t_DLComplexSamp;

typedef struct {
   Short I;   
   Short Q; 
} t_ULComplexSamp;
 
#define NUM_RTS_BUFS 2

typedef struct {
   Uint8    TN;             /* time slot */
   Uint8    chanSel;        /* logical channel type */
   Uint8    subCh;          /* subchannel number */
} rtsInfoStruc;

typedef struct {
   rtsInfoStruc   data[NUM_RTS_BUFS];
   Uint8          bufRdIndex;    /* Indicates buffer to transmit from */
   Uint8          bufWrIndex;   
   Uint8          numMsgsReady;  /* Number of pending messages to read */
   Bool           overflow;      /* Set to TRUE is buffer has been overloaded*/
   t_GsmFrameNum  frameNum;
} t_rtsStruc;
   
typedef struct {
   t_groupNum  groupNum;         /* Group that this timeslot is running */
   /*
   * Pointer to the UL BB processing selects,
   * has to be initialized on Time Slot configuration 
   */
   Uint32      *groupULTable;

   /*
   * Pointer to the DL BB processing selects,
   * has to be initialized on Time Slot configuration 
   */
   Uint32      *groupDLTable; 

   /*
   * length of the Group Table, has to be initialized on Time Slot configuration
   */
   Uint8       tblLength;
            
   /*
   * index to the UL BB processing function table,
   * has to be initialized on Time Slot configuration
   * and at GSM frame sync time.
   */
   Uint8       ulTblIndex;

   /*
   * index to the DL BB processing function table,
   * has to be initialized on Time Slot configuration
   * and at GSM frame sync time.
   */
   Uint8       dlTblIndex;

   Uint8       tsNumber;
   Uint8       tsConfig;
   union
   {
      t_groupIInfo      group1;
      t_groupIVInfo     group4;
      t_groupVInfo      group5;
      t_groupVIIInfo    group7;
      t_groupXIInfo     group11;    
      t_groupXIIInfo    group12;
      t_groupXIIIInfo   group13;
   } u;
                  
   UShort      dynamicPower;  /* BTS TX dynamic power level */
 
   /*  These are related to the timeslot, not subchannel, because
   **  other processes (AGC calc, SNR of access burst) do not have
   **  subchannels associated with them.
   */
   Int16       ULPower;     /* received power level of present burst */
   Int16       snr;         /* demod SNR of present burst */
   Int16       toa;         /* time-of-arrival of present burst */
} t_TNInfo;

typedef struct {
   Uint8    offset;         /* a random frame offset for RF Carrier */

   Uint8    g1TableLen;
   Uint8    dlG1Index;

   Uint8    ulG1Index;

   Uint8    g5TableLen;
   Uint8    dlG5Index;
   Uint8    dlG5SacchSubCh;
   Uint8    dlG5SdcchSubCh;
   Uint8    dlG5PagchSubCh;

   Uint8    ulG5Index;
   Uint8    ulG5SacchSubCh;
   Uint8    ulG5SdcchSubCh;
} t_frameProcIndex;

typedef struct
{            
   t_frameProcIndex frame;
   
   UShort   staticPower;   /* BTS TX static power level */
   UChar    tseqNum;       /* training sequence number */
   Uchar    ais;  
   UChar    bsic;          /* Base Station Identity Code */ 
                           /*   MSB                LSB   */
                           /*    0 0 b0 b1 b2 b3 b4 b5   */
                           /*           PLMN     BS      */
  
   UChar    RTSOffset;     /* Defines the number of frames that the RTS will
                           /* look ahead for the RTS indication (frameBoundary) */
   Bool     RTSFlag;       /* Indiactes that at least on TS has a RTS pending */

   Int8     ulBurst;
   Int8     dlBurst;
   
   t_DLComplexSamp  *dlBBSmpl;  /* pointer to a downlink burst buffer */
   t_ULComplexSamp  *ulBBSmpl;  /* pointer to a uplink burst buffer */    
               
   UChar       BSPaMfrms;       /* Paging scheduling parameter defined by msg (default 2) */
   t_TNInfo    TNInfo[NUM_TN_PER_RF];

   /* Data members below are used by the hopping sequence algorithm */ 
   Int16       MA[64+1];/* Mobile Allocation vector MA[N], 1<= N <= 64, N=0 is unused */  
                        /* N = total number of frequencies allocated to a mobile */
   Int8        MAIO;    /* MA Index Offset, scalar, from L3 (host) via TRX_CONFIG */
   Int8        HSN;     /* Hopping Seq Num, scalar, from L3 (host) via TRX_CONFIG */
   Uint8       MAI;     /* Mobile Allocation Index (into MA), scalar, calculated */ 
   Uint8       N;       /* Number of allocated frequencies, scalar, via TRX_CONFIG_MA */ 
   Uint8       NBIN;    /* Number of bits to represent N, NBIN =integer(log2(N)+1) */ 
   Uint8       hopFlag; /* 0=non-hopping, 1=cyclic, 2=random. Set by TUNE_SYNTH trx msg */
   Uint8       IQSwap;  /* indicates whether I-Q Samples from radio board need to be swapped or not */
} t_BBInfo;     

typedef struct {
  Uint8 rfCh;
  Uint8 dlTN;
  Uint8 ulTN;
} burstProcStruc;

typedef struct {                         
   Uint32 buffer[10];
   Int8 frameCount;
} dlSidFrameStruct;

#endif  /* end of include once only */
