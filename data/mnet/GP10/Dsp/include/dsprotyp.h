/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: dsprotyp.h
*
* Description:
*   This file contains function prototypes for all DSP source files 
*
******************************************************************************/
#ifndef _DSPROTYP_H_
#define _DSPROTYP_H_
                       
#include "dsp/dsphmsg.h"
#include "comtypes.h"
#include "gsmtypes.h"
#include "bbtypes.h"

/* tch.c */
void ulTchProcBurst_0 (Uint8 rfCh, Uint8 timeSlot);
void ulTchProcBurst_3 (Uint8 rfCh, Uint8 timeSlot);
void ulSachIdleProc   (Uint8 rfCh, Uint8 timeSlot);
void zeroTxBurst(Uint32 *burstBuf);
void dlTchProcBurst_0 (Uint8 rfCh, Uint8 timeSlot);
void dlTchProcBurst_3 (Uint8 rfCh, Uint8 timeSlot);
void dlSachIdleProc (Uint8 rfCh, Uint8 timeSlot);
void ulIdleProc (Uint8 rfCh, Uint8 timeSlot);
void dlIdleProc (Uint8 rfCh, Uint8 timeSlot);

/* sdcch.c */
void ulSdcchProc0 (Uint8 rfCh, Uint8 timeSlot);
void ulSdcchProc3 (Uint8 rfCh, Uint8 timeSlot);
void dlSdcchProc0 (Uint8 rfCh, Uint8 timeSlot);
void dlSdcchProc3 (Uint8 rfCh, Uint8 timeSlot);

/* accum.c */
ProcResult accumPower (Uint8 rfCh, Uint8 timeSlot, t_groupNum groupNum,
                       t_chanSel chanSel, t_SACCHInfo *sacchInfo, tbfStruct *TBF);
Int16 computeUplinkRxLev(t_ULSignalStrength *sigstren);
ProcResult accumToa (Uint8 rfCh, Uint8 timeSlot, t_groupNum groupNum,
                     Uint8 subChan, t_chanSel chanSel, tbfStruct *TBF);
ProcResult accumBer (Uint8 rfCh, Uint8 timeSlot, t_groupNum groupNum,
                     t_chanSel chanSel, Uint8 subChan, t_SACCHInfo *sacchInfo,   
                     Bool dtxMode, Bool dtxSid);
ProcResult accumBitBlockErrs(Uint8 rfCh, Uint8 timeSlot, t_chanSel chanSel, tbfStruct *TBF);
Uint16     expAverage(Uint16 input_x, Uint16 output_y_current, Uint8 smoothMult, Uint8 smoothDiv);
void  rateTransition(Uint8 rfCh, tbfStruct *TBF);

/* sacch.c */
void ulSacchProc0 (Uint8 rfCh, Uint8 timeSlot);
void ulSacchProc3 (Uint8 rfCh, Uint8 timeSlot);
void dlSacchProc0 (Uint8 rfCh, Uint8 timeSlot);
void dlSacchProc1 (Uint8 rfCh, Uint8 timeSlot);
void dlSacchProc3 (Uint8 rfCh, Uint8 timeSlot);
void idleChanDiagReport(Uint8 rfCh, Uint8 timeSlot);

/* pagch.c */
void dlPagchProc0 (Uint8 rfCh, Uint8 timeSlot);
void dlPagchProc1 (Uint8 rfCh, Uint8 timeSlot);
void dlPagchProc2 (Uint8 rfCh, Uint8 timeSlot);
void dlPagchProc3 (Uint8 rfCh, Uint8 timeSlot);
void ulRachProc (Uint8 rfCh, Uint8 timeSlot);
void ulPrachProc0(Uint8 rfCh, Uint8 timeSlot);
void ulPrachProc3 (Uint8 rfCh, Uint8 timeSlot);
void putPPCHBuff(Uint8 rfCh, Uint8 timeslot, Uint8 ccch, Uint8 msgDiscrim, msgStruc *p_msg);
Uint8 getPPCHBuff(Uint8 rfCh, Uint8 timeSlot);
Uint8 availablePPCHBuff(Uint8 rfCh, Uint8 timeSlot);

/* bcch.c */
void dlBcchProc0 (Uint8 rfCh, Uint8 timeSlot);
void dlBcchProc1 (Uint8 rfCh, Uint8 timeSlot);
void dlBcchProc2 (Uint8 rfCh, Uint8 timeSlot);
void dlBcchProc3 (Uint8 rfCh, Uint8 timeSlot);
void dlFcchProc (Uint8 rfCh, Uint8 timeSlot);
void dlSchProc  (Uint8 rfCh, Uint8 timeSlot);
void dlPbcchProc0(Uint8 rfCh, Uint8 timeSlot);
void dlPpchProc0(Uint8 rfCh, Uint8 timeSlot);
void dlPpchProc1(Uint8 rfCh, Uint8 timeSlot);
void dlPpchProc2(Uint8 rfCh, Uint8 timeSlot);
void dlPpchProc3(Uint8 rfCh, Uint8 timeSlot);


/* dspmain.c */
void ulTchNetProc (void);
void dlTchNetProc (void);                  

/* grouptbl.c */
void group0TableInit (void);
void group4TableInit (void);
void dlGroupVTableInit (void);
void ulGroupVTableInit (void);
void sacchIdleTableInit (void);
void dlGroupVIITableInit (void);
void ulGroupVIITableInit (void);            
void dlGroupITableInit (void);
void ulGroupITableInit (void);
void ulGroupXITableInit(void);
void dlGroupXITableInit(void);
void dlGroupXIIITableInit(void);
void ulGroupXIIITableInit(void);
void dlPtcchTableInit(void);
void ulPtcchTableInit(void);
void dlPbcchTableInit(void);
void dlPpchTableInit(void);
void ulPtcchIdleProc (Uint8 rfCh, Uint8 timeSlot);
void dlPtcchProc0 (Uint8 rfCh, Uint8 timeSlot);
void dlPtcchProc3 (Uint8 rfCh, Uint8 timeSlot);
void dlPtcchIdleProc0 (Uint8 rfCh, Uint8 timeSlot);
void dlPtcchIdleProc3 (Uint8 rfCh, Uint8 timeSlot);
void ulPtcchProc (Uint8 rfCh, Uint8 timeSlot);
void ulPtcchIdleProc (Uint8 rfCh, Uint8 timeSlot);
void dlPtcchIdleProc (Uint8 rfCh, Uint8 timeSlot);
void ulPtcchIdleProc0 (Uint8 rfCh, Uint8 timeSlot);

/* pdch.c */
tbfStruct * ulChan2TbfMap(Uint8 rfCh, Uint8 timeSlot);
void dlPdchProc0 (Uint8 rfCh, Uint8 timeSlot);
void dlPdchProc3 (Uint8 rfCh, Uint8 timeSlot);
void ulPdchProc0 (Uint8 rfCh, Uint8 timeSlot);
void ulPdchProc3 (Uint8 rfCh, Uint8 timeSlot);

/* groupini.c */
ProcResult tsGroupInit (void);
ProcResult tsConfigure (Int16 rfCh, Int16 ts, t_groupNum groupNum);

/* rrmsg.c */
ProcResult rrMsgProc (msgStruc  *p_msg);
ProcResult rcvPhDataReq (msgStruc *p_msg);
void chXlate (t_groupNum groupNum, t_chanSel chanType, Uint8 subchan,
              Uint8 *p_chanNum, Uint8 *p_linkId);

/* trxmsg.c */
ProcResult trxMsgProc (msgStruc *p_msg);
Uint8 frequencyHoppingAlg (Uint8 *ndx, Uint8 *seqnum);

/* oammsg.c */
ProcResult oamMsgProc(msgStruc *p_msg);

/* diagmsg.c */
ProcResult diagMsgProc(cmdStruc *diagCmd);

/* dynpwrctrl.c */
void uplink_dynamic_power_control (t_TNInfo *ts, Uint8 subChan);
void averageS (t_ULSignalStrength *s, t_DLSignalStrengthSCell *sD, Uint8 bufSiz, Int16 *newSmpl, Bool convertNewSmpl);
void averageQ (t_ULSignalQuality *q,  t_DLSignalQualitySCell *qD, Uint8 bufSiz, Uint16 *newSmpl, Bool convertNewSmpl);
Uint8 ber2rxQualConv (Uint16 berval);

/* hoProc.c */
void handOverProc (Uint8 rfCh, Uint8 timeSlot);
void handOverProcSort (Uint8 rfCh, Uint8 timeSlot);
void oamInit (void);
char hoPNtest (t_nxBuf *nxBuffer, Uint8 nx, Uint8 pass);
void hoCompare2NCells (Uint8 rfCh, Uint8 timeSlot, Uint8 hoCause);
void sendAsyncHoMsg (Uint8 rfCh, Uint8 timeSlot, Uint8 hoCause);
void UCopy2SendBuf (Uint8 rfCh, Uint8 timeSlot);

/* agc.c */
Int16 ul_agc (t_ULSignalStrength *sigstren);
Int16 rcvPwrSmoothingFilter(t_ULSignalStrength *sigstren, Short rcvPower);

/* demodc.c */
void DCOffset (t_ULComplexSamp samples[], t_chanSel groupNum, Uint8 rfCh, Uint8 index);

/* report.c */
ProcResult initMsReport (Uint8 rfCh, Uint8 reportIndex);
ProcResult initGprsMsReport(Uint8 rfCh, Uint8 tfi);
ProcResult initSdcch8Report (Uint8 rfCh, Uint8 reportIndex);
void ReportDiagToHost (void);
void ReportErrorToHost (void);

/* dspmaint.c */
Void getRspQueStatus (cmdStruc *diagCmd);
Void ReportError (ErrorType Error, Uint8 trx, Uint8 timeSlot,
                  Uint8 numParams, Uint8 *params);
Void sendDiagMsg (Uint8 diagMsgType, Uint8 trx, Uint8 timeSlot,
                  Uint8 numParams, Uint8 *params);
                  
/* queue.c */
qStruc *qCreate (Int16 NumberQ, Int16 Priority, qStruc *qName);
Bool qPut (msgStruc *msg, qStruc *qName);
Bool qGet (msgStruc *msg, qStruc *qName);
Int16 qGetAll (Int32 *wrPtr, qStruc *qName, Uint8 maxMsgs);

/* hpi.c */
Void SendMsgHost_Que (msgStruc *DspMsg); 
void sendMsgHost (msgStruc *DspFastMsg);
Void hpiRcvMsg (Void);
Void hpiXmtMsg (Void);

/* dspinit.c */
void dsp_init(void);
                  
/* rtscheck.c */
void rtsCheck (Uint8 rfCh, Uint8 timeSlot, Uint8 logCh,  Uint8 subCh, Int8 availableBuf);

/* syncmsg.c */
void dlSyncMsgProcess (t_chanSel chanSel, void *buffPtr);
void processCCHBuff (t_DLSigBufs* sigBufPtr);
void processPCCCHBuff(t_DLPPCHBufs* sigBufPtr);
void ulSyncMsgProcess (t_chanSel chanSel, Uint8 subchan, Uint8 TN, Uint8 rfCh);

/* dcchmsg.c */
ProcResult initPower (chActMsgStruc *hostChActInfo, t_SACCHInfo *dspSacchInfo);
ProcResult dcchMsgProc (msgUnionStruc *dcchMsg);
void sacchMsgProc (sacchMsgStruc *sacchMsg, t_sysInfoQueue *sacchBuf, Uint8 sysInfoType);

/* ccchmsg.c */
ProcResult ccchMsgProc(msgUnionStruc *ccchMsg);

/* init_hw2.c */
void init_hw(void);

/* burstisr.c */
interrupt void dma0Isr (void);
interrupt void dma1Isr (void);    
void interrupt gsmFrameIsr (void);
void dlBurstProcIsr (void);
void ulBurstProcIsr (void);

/* frameisr.c */
void  frameProc (void);

/* codecs.c */
void codecInit (void);
void ulCCHFireDecoder (void);

/* miscfunc.c */
void unpackFromByteBuffer (UChar *input, UChar *output, Int numBits);
void packToByteBuffer (UChar *input, UChar *output, Int numBits);
void unpackByteMSB (UChar *input, UChar *output, Int numBits);
void packToByteMSB (UChar *input, UChar *output, Int numBits);
 
/* topcodec.c */
void chanEncoder (t_chanSel chanSel, Bool frameBoundary, Uint8 TN, Bool tx);
Bool chanDecoder (t_chanSel chanSel, Bool frameBoundary, UInt TN);                 
void chanCodecInit (void);

/* burst.c */
void buildGSMBurst (t_chanSel chanSel, UChar TSEQ); 
                   
/* mod2spb.c */
void GMSKModulate (t_DLComplexSamp *outBurstPtr, t_chanSel chanSel);   

/* demtop.c */
void demodulate(t_ULComplexSamp  *samples, 
                t_chanSel        chanSel, 
                UChar            tseq, 
                t_ULBurstBuf     *ULBurstData,
                t_groupNum       groupNum,
                UChar            rfCh,
                UChar            timeSlot,
                UChar            subChan); 
                
/* framesync.c */
void tsFrameSync (Uint8 rfCh, Uint8 timeSlot);
void sysSyncProc (void);

/* encrypt.c */
void decrypt (Uint8 rfCh, Uint8 timeSlot);
void encrypt (Uint8 rfCh, Uint8 timeSlot, Uint8 burstPhase);

/* siddetect.c */
Uint8 sidDetection (Uint8 *speechBits, t_vocAlgo vocoAlgo);

/* loopback.c */
void mod2DmodLpbk(void);
void compLpbkBuf (Uint32 *inBuf, Uint32 *outBuf);
void dlGroupVTableLpbkInit(void);
void ulGroupVTableLpbkInit(void);
void ulGroupVIITableLpbkInit(void);

/* tchmsg.c */
void tchFrameInit (void);
ProcResult tchMsgProc (msgUnionStruc *tchMsg);

/* drv6x.c */
int board_init(void);

/* dspmh.c */
void dspMsgHandler(Void);
ProcResult ProcessHostMsg(cmdStruc * hmsg);


#endif  /* end of include once only */


 
