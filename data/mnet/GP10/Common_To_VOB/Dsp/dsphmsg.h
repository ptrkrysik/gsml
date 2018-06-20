/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: dsphmsg.h
*
* Description:
*   This file contains definitions, enumerations and structures for messages
*   between the DSP and Host. It does not declare any variables (i.e., does
*   not allocate any memory). 
*
******************************************************************************/
#ifndef _DSPHMSG_H_
#define _DSPHMSG_H_

#include "dsp/dsptypes.h"
      
#define TBF_NUMBER_MAX 40 
#define USF_RESERVED 0
           
/*
* HPI memory address and length in bytes
* at HPI_RX_MEM, a 32 bit of number of words can only
* be written by Host, when host finished writing messages
* to DSP HPI.  Host can write messages to HPI only if the 
* counter has been cleared by DSP.
*/                        
#define HPI_TX_NUM_BYTES_ADDR 0x80000000
#define HPI_TX_MSG_BUF_ADDR 0x00000204
#define HPI_TX_BUF_SIZE 0x204
#define HPI_TX_TOTAL_MSG_SIZE (HPI_TX_BUF_SIZE - 4)  
#define HPI_TX_MSG_SIZE 64
#define HPI_TX_MAX_MSGS (HPI_TX_TOTAL_MSG_SIZE / HPI_TX_MSG_SIZE)

/*
* at HPI_RX_MEM, a 32 bit of number of words can only
* be written by DSP, when DSP finished writing messages
* to DSP HPI memory. Host needs to clear the counter once
* all the messages have been read.
*/                        
#define HPI_RX_NUM_BYTES_ADDR 0x80000204
#define HPI_RX_MSG_BUF_ADDR 0x00000204  
#define HPI_RX_BUF_SIZE 0x204
#define HPI_RX_TOTAL_MSG_SIZE (HPI_RX_BUF_SIZE - 4)
#define HPI_RX_MSG_SIZE 64
#define HPI_RX_MAX_MSGS (HPI_RX_TOTAL_MSG_SIZE / HPI_RX_MSG_SIZE)
                        
#define SYNC_WORD 0xAAAA

#define HPI_MSG_SIZE 64

typedef enum
{                 
   MSG_RESERVED      = 0x00<<1,
   RR_MANAGE_MSG     = 0x01<<1,
   DCCH_MANAGE_MSG   = 0x04<<1,
   CCCH_MANAGE_MSG   = 0x06<<1,
   TRX_MANAGE_MSG    = 0x08<<1,
   OAM_MANAGE_MSG    = 0x0C<<1,
   DIAGNOSTIC_MSG    = 0x10<<1,
   PDCH_MANAGE_MSG   = 0x20<<1,
   TRAFFIC_MSG       = 0x02<<1
} msgDiscriminator;

typedef enum
{         
   BCCH_INFO_REQ = 0x0011,
   CCCH_LOAD_IND = 0x0012,
  
   DCH_CH_ACTIVATION_REQ = 0x0021,
   DCH_CH_ACTIVATION_REQ_ACK = 0x0022,
   DCH_CH_ACTIVATION_REQ_NACK = 0x0023,
    
   DCH_CH_RELEASE_REQ = 0x002E,
   DCH_CH_RELEASE_REQ_ACK = 0x0033,
    
   DCH_MODE_MOD_REQ = 0x0029,
   DCH_MODE_MOD_REQ_ACK = 0x002A,
   DCH_MODE_MOD_REQ_NACK = 0x002B,
    
   DCH_ENCRYPT_REQ = 0x0026,
  
   DCH_MEAS_RESULTS_REQ = 0x0028,
     
   DCH_PHY_CONTEXT_REQ = 0x002C,
   DCH_PHY_CONTEXT_REQ_CFM = 0x002D,

   DCH_MS_PWR_CONTROL = 0x002F,
   DCH_BS_PWR_CONTROL = 0x0030,
  
   DCH_SACCH_DEACTIVATE_REQ = 0x0025,  
   DCH_SACCH_INFO_MOD_REQ = 0x0034,	/*dedicated channel SACCH info modify */

   DCH_RF_LINK_LOST_IND = 0x0035,	/*Lost RF link on Failed receive SACCH frames */



   DCH_ASYNC_HO_REQUIRED_IND  = 0x0036,	/* Async HO request indication*/

   DCH_OM_ASYNC_HO_REQ  = 0x0037,	/* Force O&M Async HO request */  
  
   PH_DATA_REQ = 0x0001,
   PH_DATA_IND = 0x0002,
   PH_CONNECT_IND = 0x0100,
   PH_RACH_IND = 0x0200,
   PH_READY2SEND_IND = 0x0300,
   PH_EMPTY_FRAME_REQ = 0x0400,

   /*  TRX messages  */
   TRX_CONFIG  =     0x1500,
   TRX_CONFIG_ACK =  0x1600,
   SLOT_ACTIV     =  0x1700,
   SLOT_ACTIV_ACK =  0x1800,
   SLOT_ACTIV_NACK =  0x1900,
   TRX_CONFIG_MA =	 0x2000,
   TRX_CONFIG_MA_ACK = 0x2100,
   SLOT_REL  =    2,
   SACCH_FILL  =   0x001A,
   ERROR_REPORT =  0x001C,
   SET_UL_GAIN  =  0x1000,
   SET_DL_POWER =  0x1100,
   TUNE_SYNTH   =  0x1200,
   CONFIG_FPGA_DELAY = 0x1400,
   SYNCH     =     0x001D,
      
   TEMPERATURE_COMPENSATION = 0x1440,

   SET_RACH_SNR_THRESHOLD = 0x1A00,
      
   KEEP_ALIVE_PING = 0x1300, 
   KEEP_ALIVE_PONG = 0x1301,
    
   REQ_CONFIG_RADIO_TX_PWR      = 0x2200,
   RADIO_TX_PWR_CHAR_CONFIG     = 0x2400,
   RADIO_TX_PWR_CHAR_CONFIG_ACK = 0x2500, 
   TX_PWR_OFFSETS_CONFIG        = 0x2600,
   TX_PWR_OFFSETS_CONFIG_ACK    = 0x2700,
   SET_ARFCN                    = 0x2900,
   RADIO_GAIN_CHAR_CONFIG       = 0x2A00,
   RADIO_GAIN_CHAR_CONFIG_ACK   = 0x2B00,

   /* OA&M messages */
   OAM_DPC_CONFIG           = 0x5000,
   OAM_DPC_CONFIG_ACK       = 0x5001,
   OAM_HO_SCELL_CONFIG      = 0x5002,
   OAM_HO_SCELL_CONFIG_ACK  = 0x5003,
   OAM_HO_NCELL_CONFIG      = 0x5004,
   OAM_HO_NCELL_CONFIG_ACK  = 0x5005,
   OAM_TXPWR_MAX_RDCT_CONFIG     = 0x5006,
   OAM_TXPWR_MAX_RDCT_CONFIG_ACK = 0x5007,
   OAM_BTS_BASIC_CONFIG     = 0x5008,
   OAM_BTS_BASIC_CONFIG_ACK = 0x5009,
   OAM_G11_CONFIG           = 0x500A,
   OAM_G11_CONFIG_ACK      = 0x500B,
   OAM_PRACH_CONFIG           = 0x500C,
   OAM_PRACH_CONFIG_ACK      = 0x500D,


   /*
   * TCH messages
   */
   UP_LINK_TCH = 0x3000, 
   DONW_LINK_TCH = 0x3001, 
       
   /*
   * diagnostic messages                
   */                                   
  
   MPH_DSP_DIAG_REPORT_ERR_RSP = 0x001C,
    
   MPH_DSP_DIAG_PING_REQ = 0x2000,
   MPH_DSP_DIAG_PONG_RSP = 0x2001,    
   
   MPH_DSP_SET_ID_REQ = 0x2002,
     
   MPH_DSP_DIAG_READ_MEM_RSP = 0x2003,
   MPH_DSP_DIAG_READ_MEM_REQ = 0x2004,

   MPH_DSP_DIAG_GET_DSP_QUE_STATUS_REQ = 0x2005,
   MPH_DSP_DIAG_GET_DSP_QUE_STATUS_RSP = 0x2006,

   MPH_DSP_DIAG_ECHO_MSG_REQ = 0x2008,
   MPH_DSP_DIAG_ECHO_MSG_RSP = 0x2009, 
    
   MPH_DSP_DIAG_REPORT_REQ = 0x2010,
   MPH_DSP_DIAG_REPORT_RSP = 0x2011,                                  

   MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ = 0x2012,
   MPH_DSP_DIAG_IDLE_FRAME_LOG_RSP = 0x2013,

   MPH_DSP_DIAG_BURST_CAPTURE_REQ = 0x2014,
   MPH_DSP_DIAG_BURST_CAPTURE_RSP = 0x2015,
                                         
   MPH_DSP_DIAG_CHK_QUEUE_REQ = 0x2016,
   MPH_DSP_DIAG_CHK_QUEUE_RSP = 0x2017,
                                                                   
   MPH_DSP_SET_LOOPBACK_MODE_REQ = 0x2020,
   MPH_DSP_SET_LOOPBACK_MODE_RSP = 0x2121,
     
   MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST = 0x2030,
   
   MPH_DSP_DIAG_DL_DMA_ADJ = 0x2040,
                                          
   MPH_DSP_DIAG_SET_DUMMY_BURST_MODE = 0x2050,
                                          
   MPH_DSP_DIAG_SWITCH_POWER_CONTROL = 0x2200,

   MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN = 0x2280,
   
   MPH_DSP_DIAG_SET_MASKING_THRESHOLDS = 0x22ee,
   
   MPH_DSP_DIAG_SWITCH_AGC = 0x2300,
   
   MPH_DSP_DIAG_SET_AGC_DBR_TARGET = 0x2400,

   MPH_DSP_DIAG_TBF_ACT = 0x2401,
   MPH_DSP_DYNAMIC_PRACH = 0x2402,
   MPH_DSP_CODE_DOWNLOAD_DONE = 0x4000,

   

   /*
   *   Messages for Manufacturing Tests 
   */

   MPH_DSP_EXT_MEM_CHECK_REQ = 0x4000,
   MPH_DSP_EXT_MEM_CHECK_RSP = 0x4001,
   MPH_DSP_EXT_MEM_ERROR_RSP = 0x4007,

   MPH_DSP_GENERATE_TONE_REQ = 0x4002,
   MPH_DSP_GENERATE_TONE_RSP = 0x4003,

   MPH_DSP_MEASURE_TONE_REQ = 0x4004,
   MPH_DSP_MEASURE_TONE_RSP = 0x4005,

   MPH_DSP_PWR_UP_UNSOL_RSP = 0x4009,


   /*
   *   GPRS Related RR Messages 
   */
            
   PH_PRACH_IND = 0x6001,            
   PH_PTCCH_REQ = 0x6002,
   
   PH_PACCH_REQ = 0x6003,    
   PH_PACCH_IND = 0x6004,               /* normal bursts */
   PH_PDTCH_REQ = 0x6005,  
   PH_PDTCH_IND = 0x6006, 

   PH_PTCCH_IND = 0x6007,
   PH_PACCH_NACK = 0x6008,

   PH_PACCH_AB_IND = 0x6009,            /* access bursts */

   /*
   *   GPRS Related dedicated channel manage Messages 
   */     
   PDCH_TBF_PTC_IND = 0x6100,
   PDCH_TBF_PSI_IND = 0x6101,

   PDCH_UL_TBF_ACT_REQ = 0x6200,
   PDCH_UL_TBF_ACT_ACK = 0x6201,
   PDCH_DL_TBF_ACT_REQ = 0x6202,

   PDCH_DL_TBF_ACT_ACK = 0x6203,
   PDCH_UL_TBF_DEACT_REQ = 0x6204,
   PDCH_UL_TBF_DEACT_ACK = 0x6205,
   PDCH_DL_TBF_DEACT_REQ = 0x6206,
   PDCH_DL_TBF_DEACT_ACK = 0x6207,
   PDCH_UL_TBF_RECONFIG_REQ = 0x6208,
   PDCH_UL_TBF_RECONFIG_ACK = 0x6209,
   PDCH_DL_TBF_RECONFIG_REQ = 0x620A,
   PDCH_DL_TBF_RECONFIG_ACK = 0x620B,
   PDCH_CODE_RATE_REQ = 0x620C,
   
   PDCH_SINGLE_BLOCK_ASSIGN_ALERT = 0x620D,
   PDCH_SINGLE_BLOCK_ASSIGN_ALERT_ACK = 0x620E,
   PDCH_DL_ASSIGN_ALERT = 0x6300,
   PDCH_DL_ASSIGN_ALERT_ACK = 0x6301,

   PDCH_DL_ACK_NACK_INFO = 0x6400

} msgType;
  

typedef enum
{  
   RSP_MON_REPORT_ERR,
   MAX_COMMON_CMD,             
   MAX_COMMAND,                               	
   CC_DSP_SEND_PAYLOAD_PACKET_RSP,  
   MAX_RESPONSE
   
}DspHostMsgType;


typedef enum
{
   NO_ERROR = 0,
   MEM_ERROR,
   MALLOC, 
   WRONG_SYS_INFO_TYPE,
   RTS_MSG_OVERFLOW,
   UNKNOWN_MSG_DISCRIMINATOR,
   UNKNOWN_MSG_TYPE,
   UNKNOWN_CMD,
   UNAUTHORIZED_MSG_SIZE,
   COMMAND_FAILED,
   DCH_MSG_OVERFLOW,
   TIME_SLOT_NOT_CONFIGURED,
   NULL_MSG,
   NULL_POINT,
   BAD_NAWC
  
} ErrorType;
  

/*-------------------------------------------------------------
 * DSP diagnostic report types 
 *------------------------------------------------------------*/ 
typedef enum
{ 
   MS_REPORT                  =  1,
   DC_OFFSET_REPORT           =  2,
   INVALID_TIMING_ADV         =  0x55,
   HYPERFRAME_ROLLOVER        =  0x66,
   POWER_CONTROL_REPORT       =  0x7f,
   HO_REPORT                  =  0x86, /* Host-to-DSP value turns on/off both HO reports */
   HO_PN_REPORT               =  0x87, /* DSP-to-L1proxy value for PN report */
   HO_CAND_REPORT             =  0x88, /* DSP-to-L1proxy value for Candidate report */
   DL_MEAS_REPORT             =  0x8A,
   SPEECH_UNDERFLOW           =  0x89,
   SPEECH_OVERFLOW            =  0x91,
   SPEECH_OUT_OF_SEQ          =  0x90,
   IF_REPORT                  =  0x96,  /* Interference report: DSP-to-Host */
   GPRS_MS_REPORT             =  0x98,
   GPRS_DL_MEAS_REPORT        =  0x99,
   PPCH_BUF_OVERFLOW          =  0x9A,  /* from putPPCHBuff() */
} dspDiagReportType;
                       
                
/* fixed message header in bytes, make it a multiple of 4 byte */  
#define MSG_HEAD_SIZE 4

/*
* the total message length has to be lined up with 32-bit word
* boundary
*/
#define MAX_PARAMS (HPI_MSG_SIZE - MSG_HEAD_SIZE-4) /* max message payload in bytes */    
                                                       /* -4 to exclude msgSize */

typedef struct
{ 
	UINT8 IMM;		/* start now or on later time */
	UINT8 timeByte0;	/* t1'(5bit)t3 high(3bit) */
	UINT8 timeByte1;   /* t3 low(3bit) t2(5bit) */
} t_startTime;

typedef struct
{
   UINT16         chanNum;
   UINT8          sysInfoType;
   UINT8          msgSize;
   UINT8          sysInfo[23];
   t_startTime    startTime;
} ccchInfoMsgStruc;
             
typedef struct
{
   UINT16         chanNum;
   UINT8          sysInfoType;
   UINT8          msgSize;
   UINT8          sysInfo[18];
   t_startTime    startTime;
} dchSacchMsgStruc;

typedef struct
{
	UINT8			sysInfoType;
	UINT8			msgSize;
	UINT8			sysInfo[18];
	t_startTime	startTime;
} sacchMsgStruc;

/*
*  TBF tags are used by RLC/MAC for certain control messages.  Default value
*  is associated with messages not requiring TBF tagging.
*/
#define TBF_TAG_DEFAULT 0xff

/*
*  Channel Number IE, GPRS values for channel type
*/
#define CHAN_NUM_IE_PACCH  0x13
#define CHAN_NUM_IE_PDTCH  0x14
#define CHAN_NUM_IE_PTCCH  0x15
#define CHAN_NUM_IE_PPAGCH 0x16
#define CHAN_NUM_IE_PRACH  0x17



/*
*  Channel Activation Type
*/
typedef UINT8 t_chActType;

#define IMMEDIATE_ASSIGN_TYPE 0x00
#define NORMAL_ASSIGN_TYPE 0x01

#define ASYNC_HANDOVER_TYPE 0x02
#define SYNC_HANDOVER_TYPE 0x03

#define ADDITIONAL_ASSIGN_TYPE 0x04
#define MULTISLOT_ASSIGN_TYPE 0x05

#define CBCH_TYPE 0x06
/*
*  "dtxSelect" bit 0 and 1 defines Ul and DL DTX Mode
*/
#define UL_DTX 0x01      
#define DL_DTX 0x02
                                         
/*
*  "chSelect" defines channel type, speech, data, or signaling
*/                  
#define SPEECH_CHANNEL 0x01
#define DATA_CHANNEL 0x02
#define SIGNALING_CHANNEL 0x03

/*
*  "chType" defines logical channel type
*/
#define CH_SDCCH 0x01
#define CH_FR_TCH_BM 0x08
#define CH_HR_TCH_LM 0x09
#define CH_FR_TCH_BM_BI_MSLOT 0x0A
#define CH_HR_TCH_BM_UNI_MSLOT 0x0B
 
/*
*  "dataRate" defines vocoder algorithm used 
* or data rate selection if this is a data channel
*/
#define VOCODER_ALGORITHM_V1 0x01
#define VOCODER_ALGORITHM_V2 0x21
#define VOCODER_ALGORITHM_V3 0x41   
 
/*
*  "algorithmId" defines the type of encryption
*/
#define ENCRYPTION_ALGORITHM_NONE 0x01
#define ENCRYPTION_ALGORITHM_A51  0x02
#define ENCRYPTION_ALGORITHM_A52  0x03
     
typedef struct 
{
   UINT8 dtxSelect;      /* uplink and DL DTX selection */
   UINT8 chSelect;       /* Speech, Data or Signaling */
   UINT8 chType;         /* SDCCH, FR, HR, EFR */
   UINT8 dataRate;       /* data rate and transparent */
} t_chModes;

typedef struct
{
	UINT8 algorithmId;
	UINT8 key[8];
} t_encryptInfo;
							
typedef struct
{
	UINT16         chanNum;
	t_chActType    chActType;
	t_chModes      chModes;
	t_encryptInfo  encryptInfo;
	UINT8          hoRef;
	UINT8          bsPower;
	INT8           msPower;
	INT8           timeAdv;
	INT8           msPowerInit;
	UINT8          UIC;
} chActMsgStruc;

typedef struct
{
	UINT16         chanNum;
   UINT8          linkId;
	t_encryptInfo  encryptInfo;
} encryptMsgStruc;
 
/*
* Note: &tchFrame[0] is cast to UINT32* in the source code. Therefore, we must
* define a multiple of 4 bytes of storage before it to maintain alignment
*/ 
typedef struct
{
  UINT16 chanNum;
  UINT16 rtpSeqNum;    /*replace filler bytes with RTP frame number */
  UINT8  tchFrame[40];     
} tchMsgStruc;
                          
/*
* Note: &params[0] is cast to UINT32* in the source code. Therefore, we must
* define a multiple of 4 bytes of storage before it to maintain alignment
*/ 
typedef struct
{
  INT32 msgSize; 
  UINT8 function;  		/* Message descriminator */
  UINT8 typeHiByte;		/* Message Type */
  UINT8 typeLoByte;		/* Message Type */
  UINT8 trx; 
  UINT8 params[MAX_PARAMS];  
} msgStruc, rspStruc, cmdStruc;
                                   
/*
* Note: &params[0] is cast to UINT32* in the source code. Therefore, we must
* define a multiple of 4 bytes of storage before it to maintain alignment
*/ 
typedef struct
{
  INT32 msgSize;   
  UINT8 function;  		/* Message descriminator */
  UINT8 typeHiByte;		/* Message Type */
  UINT8 typeLoByte;		/* Message Type */
  UINT8 trx; 
  union
  {
   UINT8             params[MAX_PARAMS];
   ccchInfoMsgStruc  ccchInfo;
   chActMsgStruc     chActInfo;
   encryptMsgStruc   encryptCmdInfo;
   dchSacchMsgStruc  dchSacchInfo;
   sacchMsgStruc     sacchInfo;
   tchMsgStruc       tchInfo;
  }u;
}msgUnionStruc;



/*
*  Definitions to be shared by MAC/RLC and DSP
*/

typedef enum {
   TBF_STATIC = 0,
   TBF_DYNAMIC = 1,
   TBF_DYNAMIC_EXTENDED = 2,
   TBF_SBLOCK_MODE = 3
} t_tbfOpMode;
        

#endif  /* end of include once only */



         
