/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*
 * The code in this file implement a vxWorks layer 1 interface
 * to a TI CX6 DSP over the DSP's HPI interface. The message
 * structure over the interface is defined by JetCell. The basic
 * buffer consists of four bytes giving the number of blocks "n"
 * followed by "n" 64 byte blocks or messages. The first four
 * bytes of each message or block represent "len", or the
 * the length of the message block, minus an 8 byte header.
 *
 * Downstream messaging is initiated by calling the vxWorks API
 * sendDsp(void *msg, int len). sendDsp, executing in the thread
 * of the caller, will do a buffer copy to a vxWorks queue and exit
 * A vxWorks task, taskDspDn, blocks on this queue, awakening to
 * perform the actual blocking and transmission to to the DSP over
 * HPI.
 *
 * Upstream messaging is initiated by the DSP issuing an interrupt
 * to vxWorks. The interrupt is handled by the routine fooInt, which
 * will "give" the semaphore semDspUp. A vxWorks task, taskDspUp,
 * blocks on this semaphore. The HPI buffer is transferred and unblocked
 * by this task. Notice that all buffer processing is done at task level.
 * All vxWorks artifacts, queues, tasks, interrupt routines, semaphores,
 * are intialialized by initL1. The DSP is initialized by initDSP. The
 * go routine is used to enable interrupts and start the DSP in
 * HPI boot mode.
 */

#include "vxWorks.h"
#include "memLib.h"
#include "string.h"
#include "intLib.h"
#include "logLib.h"
#include "taskLib.h"
#include "msgQLib.h"
#include "vxLib.h"
#include "errnoLib.h"
#include "stdio.h"
#include "logging/vclogging.h"

#include "dsp/dsptypes.h"
#include "dsp/dsphmsg.h"

//#include "foo.h"

#ifdef __cplusplus
extern "C" {
#endif
void hpi2dsp
    (
    void *buffer,
    unsigned int addr,
    int nBytes
    );
#ifdef __cplusplus
}
#endif




typedef struct
{
  int sdcchRcvd[8];
  int sacchRcvd[8];
  int rachRcvd;
  int facchRcvd;
  int tchRcvd;
} ulMsMsgStruc;

extern ulMsMsgStruc ulMsMsgLog[2][8];
      



typedef struct
{
  unsigned long badSdcchBits[8];
  unsigned long totalSdcchBits[8];
  unsigned long badSdcchFrames[8];
  unsigned long totalSdcchFrames[8];

  unsigned long badBits;
  unsigned long totalBits;
  unsigned long badFrames;
  unsigned long totalFrames;

  unsigned short pdchChannel;

} ulMsMearsStruc;
      
extern ulMsMearsStruc ulMsMearsLog[2][8]; 
 
extern DbgOutput l1proxyDbg;

void api_SendMsgFromRmToL1 (
		unsigned short 	length,
		unsigned char	*msg
		   );


void SendFacchMsg(unsigned char timeSlot)
{
   unsigned short    length;	   
	unsigned char     i;
   unsigned char     UA_buffer[]= {
				0x02,0x00,0x01,0x00,0x08,0x00,0x00,
				0x01,0x73,0x49,0x05,0x08,0x70,0x00,
				0xf1,0x10,0xff,0xfe,0x20,0x08,0x29,
				0x43,0x03,0x75,0x47,0x00,0x24,0xff,
				0x2b,0x2b
				};
   UA_buffer[4] |= timeSlot;
   l1proxyDbg.Trace("Send A Facch -> Dsp ....\n");
	for(i=0;i<1;i++)
	{
      api_SendMsgFromRmToL1(30, UA_buffer);
	}
      
}

void intg_SendChanActivation( unsigned char chan);
void intg_SendChanDeActivation( unsigned char chan);


void t_sendDlFacchMsg(void)
{
  unsigned char TN;

  while(1)
  {		
    for(TN=1; TN < 7; TN++)
    {
      taskDelay(100);
      l1proxyDbg.Trace("Activate Channel(TN): %d\n", TN);
      //intg_SendChanActivation(2);

      l1proxyDbg.Trace("Send a Facch on TimeSlot: %d \n", TN);
      SendFacchMsg(2);

      l1proxyDbg.Trace("DeActivate Channel(TN): %d\n", TN);
      //intg_SendChanActivation(2);

    }
  }
}

/* taskSpawn("FacchTest, 100, 0, 1024,t_sendDlFacchMsg,0, 0, 0, 0, 0, 0, 0, 0, 0, 0); */
void testFacch(void)
{
   taskSpawn(
            "facchTest",               // Task name
            50,                        // priority
            0,                         // option word
            1024*32,                   // stack size
            (FUNCPTR)t_sendDlFacchMsg, // entry function pointer
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            );
}

void l1test_SendDnFacchMsg(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
/*   buffer[length++] = 40;
   buffer[length++] = 0;
   buffer[length++] = 0;
   buffer[length++] = 0;
*/
   buffer[length++] = 0x02;
   buffer[length++] = 0x00;
   buffer[length++] = 0x01;
   buffer[length++] = 0;
   buffer[length++] = 0x09;  //FACCH CHN NUMBER hi byte
   buffer[length++] = 0x00;  //FACCH CHN NUMBER lo byte
   buffer[length++] = 0x00;  //Main signaling
        
   memset(&buffer[length], 0x2b, 23);

   while(1)
  {
    taskDelay(1);

    /* Send PH_DATA_REQ to L1 */
    api_SendMsgFromRmToL1(30, buffer);

  }
}

void loopbackDsp(unsigned char trx, unsigned char loopbackType)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_SET_LOOPBACK_MODE_REQ>>8;
   buffer[length++] = MPH_DSP_SET_LOOPBACK_MODE_REQ;
   buffer[length++] = trx;
   buffer[length++] = loopbackType;
 
   api_SendMsgFromRmToL1(length, buffer);
}        

void sendSyncCmdToDsp(unsigned char trx)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = TRX_MANAGE_MSG;     /* function type */
   buffer[length++] = SYNCH>>8;
   buffer[length++] = SYNCH;
   buffer[length++] = trx;
 
   api_SendMsgFromRmToL1(length, buffer);
}        


void sendPAGMsgToDsp(unsigned char trx, unsigned char pagingGroup, unsigned char repeats)
{
   unsigned short    length = 0x1e;  
   unsigned char     buffer[0x1e] = { 0x02, 0x00, 0x01, 0x00, 0x90, 0x1c, 0x20, 0x2d, 0x06, 0x3f, 0x03, 0x09,
                                      0xa3, 0x14, 0xe0, 0x24, 0x0f, 0x00, 0x00, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
									  0x2b, 0x2b, 0x2b, 0x2b, 0x2b};
 
   buffer[5] = pagingGroup;

   while(repeats--)
      api_SendMsgFromRmToL1(length, buffer);
}        


void rachThresh(UINT8 rachThresh)
{

   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = TRX_MANAGE_MSG;     /* function type */
   buffer[length++] = SET_RACH_SNR_THRESHOLD>>8;
   buffer[length++] = SET_RACH_SNR_THRESHOLD;
   buffer[length++] = 0;
   buffer[length++] = rachThresh;
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = TRX_MANAGE_MSG;     /* function type */
   buffer[length++] = SET_RACH_SNR_THRESHOLD>>8;
   buffer[length++] = SET_RACH_SNR_THRESHOLD;
   buffer[length++] = 1;
   buffer[length++] = rachThresh;
    
   api_SendMsgFromRmToL1(length, buffer);
}        

void msReport(unsigned char trx, unsigned char tsMask)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = trx;
   buffer[length++] = MS_REPORT;
   buffer[length++] = tsMask;
 
   api_SendMsgFromRmToL1(length, buffer);
}        

void gprsMsReport(unsigned char trx, unsigned char tsMask)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = trx;
   buffer[length++] = GPRS_MS_REPORT;
   buffer[length++] = tsMask;
 
   api_SendMsgFromRmToL1(length, buffer);
}       

void dcchReport(unsigned char trx, unsigned char tsMask)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = trx;
   buffer[length++] = MS_REPORT;
   buffer[length++] = tsMask;
   buffer[length++] = 1;        /* report on FACCH and SDCCH only */
 
   api_SendMsgFromRmToL1(length, buffer);
}        

void hoReport(unsigned char trx, unsigned char tsMask)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = trx;
   buffer[length++] = HO_REPORT;
   buffer[length++] = tsMask;
 
   api_SendMsgFromRmToL1(length, buffer);
}        

void dlMeasReport(unsigned char trx, unsigned char tsMask)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = trx;
   buffer[length++] = DL_MEAS_REPORT;
   buffer[length++] = tsMask;
 
   api_SendMsgFromRmToL1(length, buffer);
}        

void dcOffsetReport(unsigned char trx, unsigned char state, unsigned char tsMask)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = trx;
   buffer[length++] = DC_OFFSET_REPORT;
   buffer[length++] = state;     /* 0=disable; 1=enable */
   buffer[length++] = tsMask;    /* TS to observe in DC offset calc */
 
   api_SendMsgFromRmToL1(length, buffer);
}      
  
void agcDbrTarget(unsigned int val)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SET_AGC_DBR_TARGET>>8;
   buffer[length++] = MPH_DSP_DIAG_SET_AGC_DBR_TARGET;
   buffer[length++] = 0;                  /* trx 0 */
   buffer[length++] = (val >> 8) & 0xff;   
   buffer[length++] =  val; 

   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SET_AGC_DBR_TARGET>>8;
   buffer[length++] = MPH_DSP_DIAG_SET_AGC_DBR_TARGET;
   buffer[length++] = 1;                  /* trx 1 */
   buffer[length++] = (val >> 8) & 0xff;   
   buffer[length++] =  val; 

   api_SendMsgFromRmToL1(length, buffer);
}      

void tchMaskingThresh(unsigned char berThresh, unsigned int metricThresh)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = 0x22;               /* temporary, and it's   */
   buffer[length++] = 0xee;               /* hard coded into tch.c */
   buffer[length++] = 0;                  /* trx 0 */
   buffer[length++] = berThresh;
   buffer[length++] = (metricThresh >> 8) & 0xff;   
   buffer[length++] =  metricThresh; 
 
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = 0x22;               /* temporary, and it's   */
   buffer[length++] = 0xee;               /* hard coded into tch.c */
   buffer[length++] = 1;                  /* trx 1 */
   buffer[length++] = berThresh;
   buffer[length++] = (metricThresh >> 8) & 0xff;   
   buffer[length++] =  metricThresh; 
 
   api_SendMsgFromRmToL1(length, buffer);
} 

void capture(unsigned char trx, unsigned char timeSlot)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_BURST_CAPTURE_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_BURST_CAPTURE_REQ;
   buffer[length++] = trx;
   buffer[length++] = timeSlot;
 
   api_SendMsgFromRmToL1(length, buffer);
} 

void disableRadioLinkLost(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST;
   buffer[length++] = 0;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST;
   buffer[length++] = 1;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);
}        

void enableRadioLinkLost(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST;
   buffer[length++] = 0;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_RADIO_LINK_LOST;
   buffer[length++] = 1;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);
}

void disableGprsRateTransition(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN;
   buffer[length++] = 0;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN;
   buffer[length++] = 1;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);
}        

void enableGprsRateTransition(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN;
   buffer[length++] = 0;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_GPRS_RATE_TRAN;
   buffer[length++] = 1;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);
} 

void disableAgc(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_AGC>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_AGC;
   buffer[length++] = 0;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_AGC>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_AGC;
   buffer[length++] = 1;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);
}        

void enableAgc(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_AGC>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_AGC;
   buffer[length++] = 0;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_AGC>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_AGC;
   buffer[length++] = 1;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);
} 

void disablePowerControl(void)
{

   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_POWER_CONTROL>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_POWER_CONTROL;
   buffer[length++] = 0;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_POWER_CONTROL>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_POWER_CONTROL;
   buffer[length++] = 1;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);
}        

void enablePowerControl(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_POWER_CONTROL>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_POWER_CONTROL;
   buffer[length++] = 0;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SWITCH_POWER_CONTROL>>8;
   buffer[length++] = MPH_DSP_DIAG_SWITCH_POWER_CONTROL;
   buffer[length++] = 1;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);
} 
 
void disableIdleFrameCapture(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ;
   buffer[length++] = 0;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ;
   buffer[length++] = 1;
   buffer[length++] = 0;   /* 0 = disable */
    
   api_SendMsgFromRmToL1(length, buffer);
}        

void enableIdleFrameCapture(void)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ;
   buffer[length++] = 0;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_IDLE_FRAME_LOG_REQ;
   buffer[length++] = 1;
   buffer[length++] = 1;   /* 1 = enable */
    
   api_SendMsgFromRmToL1(length, buffer);
}        
      
typedef unsigned char uchar;
       
void dspEcho(uchar size, uchar c0, uchar c1, uchar c2, uchar c3, uchar c4,
             uchar c5, uchar c6, uchar c7, uchar c8 )
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];
   unsigned char     i;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_ECHO_MSG_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_ECHO_MSG_REQ;
   buffer[length++] = 0;
   buffer[length++] = size;   /* 1st byte in response = LSB of msgSize */  
   buffer[length++] = 0;      /* 2nd byte in response = 2nd byte of msgSize */  
   buffer[length++] = 0;      /* 3rd byte in response = 3rd byte of msgSize */  
   buffer[length++] = 0;      /* 4th byte in response = 4th byte of msgSize */   
   buffer[length++] = c0;     /* 5th byte in response = function */  
   buffer[length++] = c1;     /* 6th byte in response = msgType high byte */  
   buffer[length++] = c2;     /* 7th byte in response = msgType low byte */  
   buffer[length++] = c3;     /* 8th byte in response = trx */  
   buffer[length++] = c4;     /* 9th byte in response = start of payload */  
   buffer[length++] = c5;  
   buffer[length++] = c6;  
   buffer[length++] = c7;  
   buffer[length++] = c8;  
    
   api_SendMsgFromRmToL1(length, buffer);
}        
     
void sendDspDmaAdj(unsigned char trx, char offset)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_DL_DMA_ADJ>>8;
   buffer[length++] = MPH_DSP_DIAG_DL_DMA_ADJ;
   buffer[length++] = trx;
   buffer[length++] = offset;   /* */
    
   api_SendMsgFromRmToL1(length, buffer);
} 

void chkDspQueue(unsigned char trx)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_CHK_QUEUE_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_CHK_QUEUE_REQ;
   buffer[length++] = trx;
    
   api_SendMsgFromRmToL1(length, buffer);
} 

void dummyBurstMode(unsigned char trx, unsigned char dummyBurstMode)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_SET_DUMMY_BURST_MODE>>8;
   buffer[length++] = MPH_DSP_DIAG_SET_DUMMY_BURST_MODE;
   buffer[length++] = trx;
   buffer[length++] = dummyBurstMode;
    
   api_SendMsgFromRmToL1(length, buffer);
} 


void sendTBFAct(int trx, int timeSlot, int CS_SEL, 
                int usfCodeRateTestFlag, unsigned long usfTsMap)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;
   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_TBF_ACT>>8;
   buffer[length++] = MPH_DSP_DIAG_TBF_ACT;
   buffer[length++] = trx;
   buffer[length++] = timeSlot & 0x7;
   buffer[length++] = CS_SEL & 3;
	buffer[length++] = usfCodeRateTestFlag;
   buffer[length++] = (unsigned char) (usfTsMap >> 24);
   buffer[length++] = (unsigned char) (usfTsMap >> 16);
   buffer[length++] = (unsigned char) (usfTsMap >> 8);
   buffer[length++] = (unsigned char)  usfTsMap;

   api_SendMsgFromRmToL1(length, buffer);
}


void dspTestDlPdtch(int trx, int timeSlot, int CS_SEL)
{
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

  buffer[length++] = PDCH_MANAGE_MSG;
  buffer[length++] = PH_PDTCH_REQ >> 8;
  buffer[length++] = PH_PDTCH_REQ;
  buffer[length++] = trx;
  buffer[length++] = timeSlot & 7;  /* channel */
  buffer[length++] = 0;
  buffer[length++] = 0;   /* stub tag */
  buffer[length++] = 0;   /* stub for NB=0, AB=1 */
  buffer[length++] = CS_SEL & 3;   /* CS-1 code rate = 0 */ 
  buffer[length++] = 0x0d;  /* MAC header, if 0x00 then it's data, else control */
  buffer[length++] = 0x01;  /* tfi=0 */
  buffer[length++] = 0x56;
  buffer[length++] = 0x32;
  buffer[length++] = 0x78;
  buffer[length++] = 0x99;
  api_SendMsgFromRmToL1(30, buffer);

}


unsigned char tx_buf[]={
0x02, 0x00, 0x01, 0x00, 0x0c, 0x00, 0x00, 0x03, 0x10, 0x0d, 0x05, 0x18,
0x01, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b, 0x2b,
0x2b, 0x2b};

extern int g_l1proxyFacchTest;

void sendIdentityReq(unsigned char trx, unsigned char timeSlot, int numMsg, int delay)
{
   unsigned short    i, length=0;	   
   unsigned char     buffer[40];

   for(i=0 ; i<24; i++)
   {
       buffer[length++] = tx_buf[i];
   }
   buffer[3] = trx;
   buffer[4] = timeSlot | 0x08;
             
   g_l1proxyFacchTest = 1;

   while(numMsg--)
  {
             
    api_SendMsgFromRmToL1(length, buffer);
    taskDelay(delay);
  }
  taskDelay(delay);
  g_l1proxyFacchTest = 0;
} 


// extern unsigned int msg2DspDropped;

void showMobileMessage(UINT8 trx)
{
   int i,j,k, timeSlot, subCh;
  
//   printf("Message sent to DSP dropped: %d \n\n", msg2DspDropped);
   printf ("Uplink MS messages on trx(%d):\n", trx);

   for(timeSlot=0; timeSlot<8; timeSlot++)
   {
      printf ("TimeSlot[%d]: SDCCH/SACCH:", timeSlot);
      for(subCh=0; subCh<4; subCh++)
      {
         printf ("%d/", ulMsMsgLog[trx][timeSlot].sdcchRcvd[subCh]);
         printf ("%d   ", ulMsMsgLog[trx][timeSlot].sacchRcvd[subCh]);      
      }
      printf ("RACH: %d     ", ulMsMsgLog[trx][timeSlot].rachRcvd);
      printf ("TCH: %d     ", ulMsMsgLog[trx][timeSlot].tchRcvd);
      printf ("FACCH: %d\n" , ulMsMsgLog[trx][timeSlot].facchRcvd);
   }
}

void clearMobileMessage(UINT8 trx)
{ 
//   msg2DspDropped = 0; 
//   printf("Message sent to DSP dropped: %d \n\n", msg2DspDropped);

   memset(& ulMsMsgLog[trx], 0, 8*sizeof(ulMsMsgStruc));
   showMobileMessage(trx);
}





void showMobileMeas(UINT8 trx)
{

   int i,j,k, timeSlot, subCh;
  
//   printf("Message sent to DSP dropped: %d \n\n", msg2DspDropped);
   printf ("Uplink MS Measurement on trx(%d):\n", trx);

   for(timeSlot=0; timeSlot<8; timeSlot++)
   {    
      if(trx == 0 && timeSlot == 0)
      {
         for(subCh=0; subCh<4; subCh++)
         {
            printf ("SDCCH SubCh[%d]\n", subCh);
            if(ulMsMearsLog[trx][timeSlot].totalSdcchBits[subCh])
            {
               printf ("Error Bits:  %10d ", ulMsMearsLog[trx][timeSlot].badSdcchBits[subCh]);
               printf ("Total Bits:  %10d ",ulMsMearsLog[trx][timeSlot].totalSdcchBits[subCh]);      
               if(ulMsMearsLog[trx][timeSlot].totalSdcchFrames[subCh])
               printf ("BER: %6f\n ", 100*(float)ulMsMearsLog[trx][timeSlot].badSdcchBits[subCh]/
                               (float)ulMsMearsLog[trx][timeSlot].totalSdcchBits[subCh]);
            }

            if(ulMsMearsLog[trx][timeSlot].totalSdcchFrames[subCh])
            {
               printf ("Error Frames:%10d ",ulMsMearsLog[trx][timeSlot].badSdcchFrames[subCh]);
               printf ("Total Frames:%10d ",ulMsMearsLog[trx][timeSlot].totalSdcchFrames[subCh]);

               printf ("FER: %6f\n",100*(float)ulMsMearsLog[trx][timeSlot].badSdcchFrames[subCh]/
                                       (float)ulMsMearsLog[trx][timeSlot].totalSdcchFrames[subCh]);         
            }
         }
      }
      else
      {
            if( ulMsMearsLog[trx][timeSlot].pdchChannel)
            {
              printf ("PDCH TimeSlot[%d]\n", timeSlot);

              if(ulMsMearsLog[trx][timeSlot].totalFrames)
              {
                printf ("Error RLC Blocks:%10d ",ulMsMearsLog[trx][timeSlot].badFrames);
                printf ("Total RLC Blocks:%10d ",ulMsMearsLog[trx][timeSlot].totalFrames);

                printf ("BLER: %6f\n", 100*(float)ulMsMearsLog[trx][timeSlot].badFrames/
                                           (float)ulMsMearsLog[trx][timeSlot].totalFrames);         
              }
            }
            else
            {
              printf ("TCH TimeSlot[%d]\n", timeSlot);

              if(ulMsMearsLog[trx][timeSlot].totalBits)
              {
                printf ("Error Bits: %10d ",ulMsMearsLog[trx][timeSlot].badBits);
                printf ("Total Bits: %10d ",ulMsMearsLog[trx][timeSlot].totalBits);      
                if(ulMsMearsLog[trx][timeSlot].totalFrames)
                printf ("BER: %6f\n ", 100*(float)ulMsMearsLog[trx][timeSlot].badBits/
                                (float)ulMsMearsLog[trx][timeSlot].totalBits);
              }

              if(ulMsMearsLog[trx][timeSlot].totalFrames)
              {
                printf ("Error Frames:%10d ",ulMsMearsLog[trx][timeSlot].badFrames);
                printf ("Total Frames:%10d ",ulMsMearsLog[trx][timeSlot].totalFrames);

                printf ("FER: %6f\n", 100*(float)ulMsMearsLog[trx][timeSlot].badFrames/
                                       (float)ulMsMearsLog[trx][timeSlot].totalFrames);         
              }
            }
      }
   }
}

void clearMobileMeas(UINT8 trx)
{
   memset(& ulMsMearsLog[trx], 0, 8*sizeof(ulMsMearsStruc));
}


/*****************************************************************************
 *
 *  Module Name: TriggerDspMsg
 *  
 *  Purpose:     Sends a message to the DSP so that the Request to Configure Msg 
 *               is sent from the DSP.
 *
 *****************************************************************************/

void sendTrxCalReq(UINT8 trx)
{
   unsigned short    length=0;	   
   unsigned char     buffer[40];

   buffer[length++] = TRX_MANAGE_MSG;     /* function type */
   buffer[length++] = REQ_CONFIG_RADIO_TX_PWR>>8;
   buffer[length++] = REQ_CONFIG_RADIO_TX_PWR;
   buffer[length++] = trx;
    
   api_SendMsgFromRmToL1(length, buffer);
}



