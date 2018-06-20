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

#include <vxWorks.h>
#include <time.h>
#include <timers.h>
#include "stdlib.h"
#include "memLib.h"
#include "string.h"
#include "intLib.h"
#include "logLib.h"
#include "taskLib.h"
#include "msgQLib.h"
#include "vxLib.h"
#include "errnoLib.h"
#include "stdio.h"
#include <semLib.h>

#include "tickLib.h"
#include "dsp/dsptypes.h"
#include "dsp/dsphmsg.h"
#include "l1proxy/l1proxy.h"
#include "MCH/MCHIntf.h"
#include "logging/vcmodules.h"
#include "logging/vclogging.h"

#ifdef __cplusplus
extern "C" {
#endif
void hpi2dsp(void *buffer, unsigned int addr, int nBytes);

extern void   SetUpForDSPTestBootupMsg(void);
extern int    WaitForDSPTestBootupMsg(void);

extern int  snmp_getValue (int);
extern int  snmp_nameToTag (char *);

#ifdef __cplusplus
}
#endif

void dspExtMemTestRsp (unsigned char*);
void dspToneMeasureRsp (unsigned char*);
void dspDump (char *p, int len, l1ProxyLogType);
void sendDsp (unsigned char *buffer, int len);
void sendIdentityReq (unsigned char trx, unsigned char timeSlot);

void  clearMobileMeas (UINT8 trx);
void  showMobileMeas (UINT8 trx);
INT32 procUlTchFrame (UINT8 *tchMsg);

SEM_ID      dspLoadSem;
int         numBootUpMsgs = 0;

MSG_Q_ID    dspPwrUpMsgQID;
int         dspPwrUpCallerType;
int         dspPwrUpCallerMsgType;

/*
*  enable error and diagnostic message print to console
*/
int g_l1proxyErrorPrint = 0;
int g_l1proxyDiagPrint = 0;
int g_l1proxyShellDisplay = 0;
int g_l1proxyPowerControlMsgPrint = 0;
int g_l1proxySacchMeasReportFromDL = 0;

int g_l1proxyTchFrameMissDisplay = 0;

int g_l1proxyFacchTest = 0;

extern int l1pTrxSwapState;              /* 1/0 - swap/don't swap trx-dsp mapping */

DbgOutput l1proxyDbg;

typedef struct
{
  int sdcchRcvd[8];
  int sacchRcvd[8];
  int rachRcvd;
  int facchRcvd;
  int tchRcvd;
} ulMsMsgStruc;

ulMsMsgStruc ulMsMsgLog[2][8] = {0};

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

ulMsMearsStruc ulMsMearsLog[2][8] = {0}; 

typedef enum 
{
   ILLEGAL_MSG = 0,
   SACCH = 1,
   RACH = 2,
   FACCH = 3,
   SDCCH4 = 4,
   SACCH4 = 5,
   SDCCH8 = 6,
   SACCH8 = 7,
   TCH = 8,
   RTS_MSG = 9
} ulMsMsgType;

int ulMobileMessagePreProc(unsigned char *msgBuf)
{
   l1proxyDbg.Enter();
   unsigned char TRX;
   unsigned char timeSlot;
   unsigned char subCh;
   unsigned char reportMode;
   unsigned char *payload;
   unsigned char cbits;
   unsigned char function;
   unsigned short msgType;
   unsigned int t1, t2, t3, frameNum, hpiTxFrameNumber;
   msgDiscriminator msgDes;
   unsigned short count;

   signed int iSample, qSample;
   /*
   * Message to Layer2 is set enable as default
   */
   unsigned char sendMsg2L2;
   short i;
   ulMsMsgType msMsg;

   TRX = msgBuf[7];
   function = msgBuf[4];
   msgType  = msgBuf[5]<<8 | msgBuf[6];
   timeSlot = msgBuf[8] & 7;
   subCh    = (msgBuf[8] >> 3) & 3;
   reportMode = msgBuf[9];
   payload = & msgBuf[10];

   sendMsg2L2 = 1;  
   msMsg = ILLEGAL_MSG; 
   
   switch(function)
   {
   case TRX_MANAGE_MSG:

      switch(msgType)
      {
      case REQ_CONFIG_RADIO_TX_PWR:
         printf("REQ_CONFIG_RADIO_TX_PWR Received From TRX: %d \n", TRX);
         sendMsg2L2 = 0;
         break;

      case RADIO_TX_PWR_CHAR_CONFIG_ACK:
         printf("RADIO_TX_PWR_CHAR_CONFIG_ACK Received \n");
         sendMsg2L2 = 0;
         break;

      case TX_PWR_OFFSETS_CONFIG_ACK:
         printf("TX_PWR_OFFSETS_CONFIG_ACK Received \n");
         sendMsg2L2 = 0;
         break;

      case RADIO_GAIN_CHAR_CONFIG_ACK:
         printf("RADIO_GAIN_CHAR_CONFIG_ACK Received \n");
         sendMsg2L2 = 0;
         break;

      default:
         break;
      } /* switch(msgType) */
      break; 

   case DIAGNOSTIC_MSG:
      
      switch(msgType) 
      {
      case ERROR_REPORT:   /* Error Messages */ 
         
         if(reportMode == UNAUTHORIZED_MSG_SIZE)
         {
            l1proxyDbg.Error("Trx%1d Ts%1d  Host Message Size Out of Bounds \n",
                  TRX, timeSlot);
         }
         else if(reportMode == UNKNOWN_MSG_DISCRIMINATOR)
         {
            l1proxyDbg.Error("Trx%1d Ts%1d  Unknown Message Discriminator(0x%x) \n",
                  TRX, timeSlot, payload[4]);
         }
         else if(reportMode == 0x20)
         {
            l1proxyDbg.Error("Trx%1d Ts%1d  Illegal TRX Number(%d) \n",
                  TRX, timeSlot, payload[0]);
         }
         else if(reportMode == UNKNOWN_CMD)
         {
            l1proxyDbg.Error("Trx%1d Ts%1d  Unknown Message Type(0x%04X) \n",
                  TRX, timeSlot, payload[0] <<8 | msgBuf[11]);
         }
         else 
         {
            l1proxyDbg.Error("Trx%1d Ts%1d  Function=DspDiag, msgType=ErrorReport, ReportMode=%d \n",
               TRX, timeSlot, reportMode);
         }      

         /*
         * display GSM frame number for all error messages
         */
         if(g_l1proxyDiagPrint)
         {
            hpiTxFrameNumber = (int) (msgBuf[60] |  msgBuf[61]<<8 |  msgBuf[62]<<16 |  msgBuf[63]<<24);
            l1proxyDbg.Trace("GSM Frame Number: 0x%08X \n", hpiTxFrameNumber);
         }
         break; // End of ErrorReport catagory fo DSP diagnostic messsages

      case MPH_DSP_DIAG_REPORT_RSP:   /* Diagnostic Message */
         
         switch(reportMode)
         {
         case MS_REPORT: 
            DiagSendMsg((char *)msgBuf, HPI_MSG_SIZE, DIAG_MS_REPORT);            
            unsigned long  cumErrs, cumBits, cumFrameErrs, cumFrames;

            cumErrs = (unsigned long)(msgBuf[10])<<24 |
                      (unsigned long)(msgBuf[11])<<16 |
                      (unsigned long)(msgBuf[12])<< 8 |
                      (unsigned long)(msgBuf[13]);
            cumBits = (unsigned long)(msgBuf[14])<<24 |
                      (unsigned long)(msgBuf[15])<<16 |
                      (unsigned long)(msgBuf[16])<< 8 |
                      (unsigned long)(msgBuf[17]);
            cumFrameErrs= (unsigned long)(msgBuf[26])<< 8 |
                          (unsigned long)(msgBuf[27]);
            cumFrames   = (unsigned long)(msgBuf[28])<< 8 |
                          (unsigned long)(msgBuf[29]);

            if(TRX == 0 && timeSlot == 0)
            {
               ulMsMearsLog[TRX][timeSlot].badSdcchBits[subCh] += cumErrs;
               ulMsMearsLog[TRX][timeSlot].totalSdcchBits[subCh] += cumBits;
               ulMsMearsLog[TRX][timeSlot].badSdcchFrames[subCh] += cumFrameErrs;
               ulMsMearsLog[TRX][timeSlot].totalSdcchFrames[subCh] += cumFrames;
            }
            else
            {
               ulMsMearsLog[TRX][timeSlot].badBits += cumErrs;
               ulMsMearsLog[TRX][timeSlot].totalBits += cumBits;
               ulMsMearsLog[TRX][timeSlot].badFrames += cumFrameErrs;
               ulMsMearsLog[TRX][timeSlot].totalFrames += cumFrames;

               ulMsMearsLog[TRX][timeSlot].pdchChannel = 0;

               if ( ((unsigned long)(msgBuf[24]) == 0xff) &&
                    ((unsigned long)(msgBuf[25]) == 0xff) )
               {
                  ulMsMearsLog[TRX][timeSlot].pdchChannel = 1;
               }

               if(ulMsMearsLog[TRX][timeSlot].totalBits > 0xFFF00000)
               {
                 if(g_l1proxyDiagPrint)
                 {
                    showMobileMeas(TRX);
                 }
                 clearMobileMeas(TRX);
               }
            }
            break;

         case GPRS_MS_REPORT: 
            DiagSendMsg((char *)msgBuf, HPI_MSG_SIZE, DIAG_GPRS_MS_REPORT);            
            break;
 
         case DC_OFFSET_REPORT:
            {
               short  offsetI[13], offsetQ[13], i;
               for ( i=0; i<13; i++ )
               {
                  offsetI[i] = (short) ( (unsigned short)(payload[4*i])<<8 |
                                    (unsigned short)(payload[4*i+1]) );
                  offsetQ[i] = (short) ( (unsigned short)(payload[4*i+2])<<8 |
                                    (unsigned short)(payload[4*i+3]) );                                               
               }

               l1proxyDbg.Trace("Trx%1d DC Offsets (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d)"
                  " (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d) \n", TRX,
                  offsetI[0], offsetQ[0], offsetI[1], offsetQ[1], offsetI[2], offsetQ[2],
                  offsetI[3], offsetQ[3], offsetI[4], offsetQ[4], offsetI[5], offsetQ[5],
                  offsetI[6], offsetQ[6], offsetI[7], offsetQ[7], offsetI[8], offsetQ[8],
                  offsetI[9], offsetQ[9],offsetI[10],offsetQ[10],offsetI[11],offsetQ[11],
                  offsetI[12],offsetQ[12]);
            }      
            break;

         case POWER_CONTROL_REPORT:
            if(g_l1proxyPowerControlMsgPrint)
            {
               short cmdPwr, rxlev, smPwr, gain, ta;
                           
               cmdPwr  = (short) (msgBuf[10]);
               rxlev   = (short) ( (unsigned short)(msgBuf[11])<<8 |
                                 (unsigned short)(msgBuf[12]) );
               ta      = (short) (msgBuf[13]);
               smPwr   = (short) ( (unsigned short)(msgBuf[14])<<8 |
                                 (unsigned short)(msgBuf[15]) );
               gain   = (short) ( (unsigned short)(msgBuf[16])<<8 |
                                 (unsigned short)(msgBuf[17]) );

               l1proxyDbg.Trace( "Trx%1d Ts%1d Sub%1d RXLEV%3d CmdPwr %02d SmPwr %02d"
                  " Gain %03d TA%2d  \n",
                  TRX, timeSlot, subCh, rxlev, cmdPwr, smPwr, gain, ta);
            }
            break;

         case INVALID_TIMING_ADV:
            if(g_l1proxyDiagPrint)
            {
               l1proxyDbg.Trace("Trx%1d Ts%1d  TCH Invalid TA (%d) \n",
                     TRX, timeSlot, msgBuf[10]);
            }
            break;

         case HO_PN_REPORT:
            DiagSendMsg((char *)msgBuf, HPI_MSG_SIZE, DIAG_HO_PN_REPORT);            
            break;

         case HO_CAND_REPORT:
            DiagSendMsg((char *)msgBuf, HPI_MSG_SIZE, DIAG_HO_CAND_REPORT);            
            break;

         case IF_REPORT:
            DiagSendMsg((char *)msgBuf, HPI_MSG_SIZE, DIAG_IF_REPORT);            
            break;

         case DL_MEAS_REPORT:
            DiagSendMsg((char *)msgBuf, HPI_MSG_SIZE, DIAG_DL_REPORT);            
            break;


         case HYPERFRAME_ROLLOVER: 
            if ( g_l1proxyShellDisplay )
               l1proxyDbg.Trace("\nHyperframe rolled over at system time: %d \n", time);
            break;

         case SPEECH_UNDERFLOW:
            if ( g_l1proxyTchFrameMissDisplay )
            {
               t1 = (short) ( (unsigned short)(payload[0])<<8 |
                              (unsigned short)(payload[1]) );
               t2 = payload[2];    
               t3 = payload[3];    
               count = (short) ( (unsigned short)(payload[4])<<8 |
                                 (unsigned short)(payload[5]) );   

               l1proxyDbg.Trace("Trx%1d Ts%1d  DL TCH speech frame underflow %d times in SF "
                        "ending T1=%d T2=%d T3=%d \n", TRX, timeSlot, count, t1, t2, t3);
            }
            break;
            
         case SPEECH_OVERFLOW:
            if ( g_l1proxyTchFrameMissDisplay )
            {   
               t1 = (short) ( (unsigned short)(payload[0])<<8 |
                              (unsigned short)(payload[1]) );
               t2 = payload[2];    
               t3 = payload[3];    
               count = (short) ( (unsigned short)(payload[4])<<8 |
                                 (unsigned short)(payload[5]) );   

               l1proxyDbg.Trace("Trx%1d Ts%1d  DL TCH speech frame overflow %d times in SF "
                        "ending T1=%d T2=%d T3=%d \n", TRX, timeSlot, count, t1, t2, t3);
            }
            break;
            
         case SPEECH_OUT_OF_SEQ:
            if ( g_l1proxyTchFrameMissDisplay )
            {   
               t1 = (short) ( (unsigned short)(payload[0])<<8 |
                              (unsigned short)(payload[1]) );
               t2 = payload[2];    
               t3 = payload[3];    
               count = (short) ( (unsigned short)(payload[4])<<8 |
                                 (unsigned short)(payload[5]) );   

               l1proxyDbg.Trace("Trx%1d Ts%1d  DL TCH Out Of Sequence %d times in SF "
                        "ending T1=%d T2=%d T3=%d \n", TRX, timeSlot, count, t1, t2, t3);
            }
            break;
          
         default:
            if ( g_l1proxyDiagPrint )
            {
               l1proxyDbg.Trace("Trx%1d Ts%1d  Function=DspDiag, msgType=DiagReport, ReportMode=%d \n",
                     TRX, timeSlot, reportMode);
               dspDump((char *) & msgBuf[0], msgBuf[0]+4, L1PROXY_LOG_TRACE); 
               /*
               * display GSM frame number for unspecified diag report messages
               */
               hpiTxFrameNumber = (int) (msgBuf[60] |  msgBuf[61]<<8 |  msgBuf[62]<<16 |  msgBuf[63]<<24);
               l1proxyDbg.Trace("GSM Frame Number: 0x%08X\n", hpiTxFrameNumber);
            }
            break;                    
         }
         break;  // End of DiagReport catagory of DSP diagnostic messsages

      /*----------------------------------------------------------------------
      * Loopback Modes 
      *---------------------------------------------------------------------*/
       case MPH_DSP_SET_LOOPBACK_MODE_RSP:
         l1proxyDbg.Trace("Set LoopBack Response -- TRX: %d: Loopback Type: %x\n", TRX, msgBuf[8]); 
         break;

      /*----------------------------------------------------------------------
      * Burst Capture and Idle Frame Capture (combine later) 
      *---------------------------------------------------------------------*/
      case MPH_DSP_DIAG_IDLE_FRAME_LOG_RSP:
      case MPH_DSP_DIAG_BURST_CAPTURE_RSP:
         /*--- Print Burst Capture always, but Idle Frame only if Diag Print on ---*/
         if( (msgType == MPH_DSP_DIAG_BURST_CAPTURE_RSP) || g_l1proxyDiagPrint )
         {         
            if ( msgBuf[11] ==0 )
            {
               if ( msgType == MPH_DSP_DIAG_IDLE_FRAME_LOG_RSP )
               { 
                  l1proxyDbg.Trace("DSP Idle Frame Log -- Trx%1d Ts%1d \n", TRX,timeSlot);
               }
               else 
               {
                  l1proxyDbg.Trace("DSP Burst Capture -- Trx%1d Ts%1d \n", TRX,timeSlot);
               }
            } 
            
            char buf[DSPBUFFERMAX];
            int curPtr = 0;
            
            curPtr += sprintf(buf+curPtr, "Block: %d\n", msgBuf[11]);

            curPtr += sprintf(buf+curPtr, "I: ");
            for(i=0; i<12; i++)  /* note: 12 = SAMPLES_PER_CAPTURE_MSG */
            {
               iSample = msgBuf[12+i*4] | msgBuf[13+i*4]<<8;
               curPtr += sprintf(buf+curPtr, "0x%04X ",iSample);
            }
            curPtr += sprintf(buf+curPtr, "\n");

            curPtr += sprintf(buf+curPtr, "Q: ");
            for(i=0; i<12; i++)
            {
               qSample = msgBuf[14+i*4] | msgBuf[15+i*4]<<8;
               curPtr += sprintf(buf+curPtr, "0x%04X ", qSample);
            }
            curPtr += sprintf(buf+curPtr, " \n");
            
            l1proxyDbg.Trace(buf);
         }
         break;
         
      case MPH_DSP_DIAG_ECHO_MSG_RSP:
         DiagSendMsg((char *)msgBuf, HPI_MSG_SIZE, DIAG_DSP_ECHO_MSG);            
         break;

      case MPH_DSP_EXT_MEM_CHECK_RSP:
         DiagSendMsg((char*)msgBuf, HPI_MSG_SIZE, DSP_EXTMEM_TEST_RSP); 
         break;

      case MPH_DSP_EXT_MEM_ERROR_RSP:
         DiagSendMsg((char*)msgBuf, HPI_MSG_SIZE, DSP_EXTMEM_ERROR_RSP); 
         break;

      case MPH_DSP_GENERATE_TONE_RSP:
         DiagSendMsg((char*)msgBuf, HPI_MSG_SIZE, DSP_TONE_GENERATE_RSP);
         break;

      case MPH_DSP_MEASURE_TONE_RSP:
         DiagSendMsg((char*)msgBuf, HPI_MSG_SIZE, DSP_TONE_MEASURE_RSP);   
         break;

      case MPH_DSP_PWR_UP_UNSOL_RSP: 
         numBootUpMsgs++;
         if ( numBootUpMsgs == 2 )
         {
            semGive (dspLoadSem); 
            numBootUpMsgs = 0;
         }
         break;

      default:
         l1proxyDbg.Trace("Trx%1d  Function=DspDiag, msgType=%d \n", TRX, msgType);
         dspDump((char *) & msgBuf[0], msgBuf[0]+4, L1PROXY_LOG_TRACE); 
         /*
         * display GSM frame number for unspecified diag messages
         */
         if(g_l1proxyDiagPrint)
         {
            hpiTxFrameNumber = (int) (msgBuf[60] |  msgBuf[61]<<8 |  msgBuf[62]<<16 |  msgBuf[63]<<24);
            l1proxyDbg.Trace("GSM Frame Number: 0x%08X \n", hpiTxFrameNumber);
         }
         break;
      } /* switch(msgType) */
      sendMsg2L2 = 0;   // don't send Diagnostic Message to up layer

      break;            // End of Diagnostic message

   case TRAFFIC_MSG:
      switch(msgType)
      {
      case UP_LINK_TCH:
         //case UP_LINK_TCH:  //0x3000
         msMsg = TCH;
         /*
         *  loopback ac l1 proxy
         msgBuf[5] = DONW_LINK_TCH>>8;
         msgBuf[6] = DONW_LINK_TCH;
         sendDsp(msgBuf,msgBuf[0]+4);
         */       
         procUlTchFrame(msgBuf);
         break;
      } /* switch(msgType) */
      sendMsg2L2 = 0;   // don't send Diagnostic Message to up layer
      break;
    
   default:
      switch(msgType)
      {     
      case PH_DATA_IND: // PH-DATA-IND      
         cbits =  (msgBuf[8] & 0xf8) >> 3;
     
         if(cbits == 1)
         {
           /*
           * TCH Full Rate
           */
           if((msgBuf[10] & 0xC0) != 0x40)
           {
               msMsg = FACCH;

            }
            else 
            {
               msMsg = SACCH;

            /* If a measurement report, do not send it upper layer */                
            if ( ( msgBuf[17] & 0x7f ) == 0x15 )  
               sendMsg2L2 = 0;
            }
           subCh = 0;

         }
         else if(cbits <4)
         {
           /*
           * TCH Half Rate
           */
           if((msgBuf[10] & 0xC0) != 0x40)
               msMsg = FACCH;
            else 
            {
               msMsg = SACCH;
            /* If a measurement report, do not send it upper layer */                
            if ( ( msgBuf[17] & 0x7f ) == 0x15 )  
               sendMsg2L2 = 0;
            }
            subCh = cbits & 1;
         } 
         else if(cbits <8)
         {
            if((msgBuf[10] & 0xC0) == 0x40)
            {
               msMsg = SACCH4;
            /* If a measurement report, do not send it upper layer */                
            if ( ( msgBuf[17] & 0x7f ) == 0x15 )  
               sendMsg2L2 = 0;
            }
            else 
               msMsg = SDCCH4;

            subCh = cbits & 3;
         } 
         else if(cbits < 16)
         {
            if((msgBuf[10] & 0xC0) != 0x40)
               msMsg = SDCCH8;
            else 
         {
             /* If a measurement report, do not send it upper layer */               
             if ( ( msgBuf[17] & 0x7f ) == 0x15 )  
               sendMsg2L2 = 0;
      
                msMsg = SACCH8;
         }
            subCh = cbits & 7;
         }

         if(g_l1proxyFacchTest && (msgBuf[13] == 0x03) && ((msgBuf[14] & 0x1F) == 0x19) && (msgBuf[15] == 0x01))
         {
             sendMsg2L2 = 0;
             //sendIdentityReq(TRX, timeSlot);
         }

         if(msgBuf[13] == 0 && msgBuf[14] == 0) sendMsg2L2 = 0;
         if(msgBuf[13] == 1 && msgBuf[14] == 3 && msgBuf[15] == 1) sendMsg2L2 = 0; 
         if(sendMsg2L2)
         {
           l1proxyDbg.Trace("DSP To LAPDm -> PH-DATA-IND:\n %02x %02x %02x %02x %02x %02x %02x %02x "
               "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
               msgBuf[13],msgBuf[14],msgBuf[15],msgBuf[16],msgBuf[17],msgBuf[18],msgBuf[19],msgBuf[20],
               msgBuf[21],msgBuf[22],msgBuf[23],msgBuf[24],msgBuf[25],msgBuf[26],msgBuf[27],msgBuf[28],
               msgBuf[29],msgBuf[30],msgBuf[31],msgBuf[32],msgBuf[33]);
         }               
         break;
    
      case PH_RACH_IND:
         l1proxyDbg.Trace("DSP To LAPDm -> RACH: %02x %02x %02x %02x %02x %02x \n",
            msgBuf[11],msgBuf[12],msgBuf[13],msgBuf[14],msgBuf[15],msgBuf[16]);
           
         msMsg = RACH;
         break;

      case PH_READY2SEND_IND:
         if(function == RR_MANAGE_MSG)
            l1proxyDbg.Trace("DSP To LAPDm -> RTS on Trx%d Ts%d \n", TRX, timeSlot);  
         else if(function == PDCH_MANAGE_MSG)
         {
            l1proxyDbg.Trace("DSP To RLC/MAC -> RTS on Trx%d Ts%d SubCh%d RdyBuf%d \n", TRX, timeSlot, subCh, msgBuf[11]); 
         }         
         else
         {
            l1proxyDbg.Trace("DSP To RRM -> RTS on Trx%d Ts%d SubCh%d RdyBuf%d \n", TRX, timeSlot, subCh, msgBuf[11]); 
         }
         msMsg = RTS_MSG;   
         break;

      case 0x2000: //BootStrap  
         l1proxyDbg.Trace("DSP To RRM -> Bootstrap Completed \n");
         break;

      case DCH_RF_LINK_LOST_IND: //Radio Link Failed  
         l1proxyDbg.Warning("DSP To RRM -> Radio Link Failed on Trx%d Ts%d \n", TRX, timeSlot);
         break;

      default:
         break; 
      } /* switch(msgType) */
      if(g_l1proxyDiagPrint && sendMsg2L2)     
      {  

         hpiTxFrameNumber = (int) (msgBuf[60] |  msgBuf[61]<<8 |  msgBuf[62]<<16 |  msgBuf[63]<<24);
         t1 = hpiTxFrameNumber / 1326;
         t2 = hpiTxFrameNumber % 26;
         t3 = hpiTxFrameNumber % 51;

         l1proxyDbg.Trace("SysTick: %08x; FN = 0x%08X; T1 = %d; T2 = %d ; T3 = %d \n",
                        tickGet(),
                        hpiTxFrameNumber, 
                        t1, t2, t3);       
      }
      break;
   } /* end of switch(function) */
   
    
   switch( msMsg )
   {
      case TCH:
         ulMsMsgLog[TRX][timeSlot].tchRcvd++;
         break;

      case SDCCH4:
      case SDCCH8:
         ulMsMsgLog[TRX][timeSlot].sdcchRcvd[subCh]++;
         break;

      case SACCH:
      case SACCH4:
      case SACCH8:
         ulMsMsgLog[TRX][timeSlot].sacchRcvd[subCh]++;
         break;

      case RACH:
         ulMsMsgLog[TRX][timeSlot].rachRcvd++;
         break;

      case FACCH:
      
         ulMsMsgLog[TRX][timeSlot].facchRcvd++;
         break;

      default:
         break;
   }   
   l1proxyDbg.Leave();
   return(sendMsg2L2);
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: SetUpForDSPTestBootupMsg
**
**    PURPOSE: Executes DSP Configuration modules. 
**
**    INPUT PARAMETERS: N/A
**
**    RETURN VALUE(S):  N/A
**
**----------------------------------------------------------------------------*/

void  SetUpForDSPTestBootupMsg(void)
{
    initDsp(0);
    initDsp(1);
    initInt();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: WaitForDSPTestBootupMsg
**
**    PURPOSE: Wait DSP Prod Test Bootup Message. 
**
**    INPUT PARAMETERS: N/A
**
**    RETURN VALUE(S):  N/A
**
**----------------------------------------------------------------------------*/

int  WaitForDSPTestBootupMsg(void)
{
    STATUS  result;

    taskDelay(300);

    go(0);
    go(1);

    result = semTake (dspLoadSem, WAIT_FOREVER);

    return(result);
}
