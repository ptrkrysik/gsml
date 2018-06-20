/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_PROTO_H__
#define __RM_PROTO_H__


#include "vxWorks.h"
#include "msgQlib.h"
#include "rm_extdef.h"
#include "rm_import.h"
#include "rm_types.h"
#include "rm_itcmsg.h"
#include "ril3\ril3_mm_msg.h"
#include "ril3\ril3irt.h"
#include "ril3\ie_rach_control_param.h"
#include "ril3\ie_cause.h"
#include "ril3\ie_rr_cause.h"
#include "ril3\ril3_result.h"
#include "ril3\msg_channel_request.h"

// Initialization and DSP interaction

void rm_RadioMonitor			(      );
void rm_InitAll				( void );
void rm_Despatch			( void );
void rm_WaitForL1Bootstrap		( void );
void rm_RetrvConfigFromOam		( void );
void rm_InitRm				( void );
void rm_InitLAPDm			( void );
void rm_InitIRT				( void );
void rm_InitL1				( void );
u8   rm_ConfigTrxSlots  		( u8 );	    	    
void rm_SendSynchInfoToBcchTrx  	( u8 );
void rm_WaitForL1Rsp            	( u8, u8, u8 ,u8 );
void rm_SendTrxConfigToTrx		( u8 );
void rm_SendSlotActivateToTrx	        ( u8, u8 );
void rm_WaitForSlotActivateRsp	        ( u8, u8 );
void rm_SendSetUGainToTrx		( u8 );
void rm_SendSetDGainToTrx		( u8 );
void rm_SendConfigFpgaDelayToTrx	( u8 );

// System Informations Update

void rm_SendAllSystemInfosToBcchTrx     ( u8 );
void rm_SendSystemInfo2ToBcchTrx	( u8,u8,T_CNI_RIL3_CELL_BARRED_ACCESS );
void rm_SendSystemInfo3ToBcchTrx	( u8,u8,T_CNI_RIL3_CELL_BARRED_ACCESS );
void rm_SendSystemInfo4ToBcchTrx	( u8,u8,T_CNI_RIL3_CELL_BARRED_ACCESS );
void rm_SendSacchFillingSI5		( u8 trx, u8 SI );
void rm_SendSacchFillingSI6		( u8 trx, u8 SI );
void rm_PopulateSI2			( T_CNI_RIL3_CELL_BARRED_ACCESS );
void rm_PopulateSI3			( T_CNI_RIL3_CELL_BARRED_ACCESS );
void rm_PopulateSI4			( T_CNI_RIL3_CELL_BARRED_ACCESS );
void rm_PopulateSI5			( void );
void rm_PopulateSI6			( void );

// Dedicated channel handling and utilies

void rm_Despatch			( void );
void rm_RrEstReq			( void );
void rm_DlRaInd				( void );
void rm_SendPagingRequestType1       	( void );
void rm_ChanReq				( rm_ChanReq_t *    );
u8   rm_SolvePagingGroup             	( rm_IeMobileId_t * );
u16  rm_GetImsiMod1000			( rm_IeImsi_t *     );
void rm_SendChanActivation		( rm_PhyChanDesc_t *, rm_PhyChanActDesc_t * );
void rm_SendImmAssign			( rm_PhyChanDesc_t * );
void rm_SendImmAssignRej     		(int cause);
int  rm_TimerExpired		    	( int param );
void rm_StartTimer			( rm_IrtEntryId_t, u8 );
void rm_StopTimer			( rm_IrtEntryId_t, u8 );
void rm_SendPagingRsp			( void );
void rm_SendRfChanRelease               ( rm_PhyChanDesc_t * );
void rm_SendDeactivateSacch		( rm_PhyChanDesc_t * );
void rm_SendChanRelease			(rm_IrtEntryId_t entryId);
void rm_SendRrRelCnf			( void );
void rm_SendChanModeModify		( rm_SmCtxt_t * );
void rm_SendCiphModeCmd         	( rm_SmCtxt_t *pSmCtxt );
void rm_SendEncryptCmdToTrx      	( rm_SmCtxt_t *pSmCtxt );
void rm_ProcMsCiphModeCmplt      	( void );
void rm_SendChanAssignComplete   	( void );
u8   rm_ProcMeasRes			( void );
void rm_SendDlRelReq			( u8 );
void rm_SendRrRelInd			( u8, rm_IrtEntryId_t );
void rm_SendModeModify		        ( rm_PhyChanDesc_t *, rm_PhyChanActDesc_t *);
rm_IrtEntryId_t rm_GetIrtEntryId        ( rm_ItcRxL1Msg_t * );
rm_SmCtxt_t *rm_AllocSmCtxt		( u8 );
rm_SmCtxt_t *rm_PreAllocSmCtxt		( u8 );
void rm_ProcClassMarkIE			(void);
void rm_MdlErrRptCauseRel		(void);
void rm_RslRelease			(void);
T_CNI_RIL3_RESULT rm_MsgAnalyze		(void);
void rm_SendRrStatus			(T_CNI_RIL3_RR_CAUSE_VALUE cause);
void rm_ProcRrRelease			(T_CNI_RIL3_CAUSE_VALUE cause);
int  rm_msgQReceive                     ( MSG_Q_ID, s8 *, UINT, int );

// Interface with CC and MM

void api_SendMsgFromRmToCc 		(MSG_Q_ID,u16,u8*);
void api_SendMsgFromRmToMm 		(u16,u8*);

// RM module entrance

void rm_Task				( void );
void rm_Main				( void );

// RM State Machine Actions

void rm_S0Null				( void );
void rm_S1RmRunning			( void );
void rm_S2ActivatingSigTrafficChan   	( void );
void rm_S3SigChanActivated		( void );
void rm_S4TrafficChanActivated       	( void );
void rm_S5SigModeEstablished		( void );
void rm_S6ActivatingTrafficMode      	( void );
void rm_S7ReleasingSigTrafficChan    	( void );
void rm_S8TrafficModeActivated       	( void );
void rm_S9DeactivatingSigTrafficChan 	( void );
void rm_S10TrafficModeEstablished    	( void );
void rm_S11TrafficChanSuspending	( void );

// Test Functions

void rm_InitRmTestPrint			(u8 tchfIdx,u8 sdcch4Idx);
void rm_InitLAPDmTestPrint		(u8 tchfIdx, u8 sdcch4Idx);
void rm_InitIrtTestPrint		(u8 tchfIdx, u8 sdcch4Idx);
void rm_PrintTchEntryId			(u8 noOfTchfs,u8 noOfSdcch4s);
void rm_UmMmMsgTestPrint		( T_CNI_RIL3MM_MSG *pMsg );
void rm_EntryIdTestPrint		(u8 tchfIdx, u8 sdcch4Idx);
void intg_SendChanActivation		(u8 trx, u8 chan);
void intg_SendChanDeActivation		(u8 trx, u8 chan);
void intg_ShowRmChans			(void);
void intg_ClearRmChans			(void);
void intg_SetDlSG			(u8 trx, u16 gain);
void intg_SetUlSG			(u8 trx, u16 gain);
void intg_SetArfcn			(s16,s16);
void intg_SetGsmCodec			(u8);
void intg_CellCfg			(s16, u8, u8, u8*);
void intg_SetNextAssignment		(u8 trx, u8 slot);
void intg_SendPhConnIndToLapdm		(void);
void intg_SendAsyncHoRequestToDsp	(u8 trx, u8 chan);

// Addition for Handover

void rm_SendAsyncHoRequired		(void);
void rm_S11TrafficChanSuspending	(void);
void rm_HoReq				(void);
void rm_SendHoFailure			(int cause, int rrCause);
void rm_SendHoReqNAck			(u8 ref);
u8   rm_SendHoReqAck			(void);
void rm_SendHoComplete			(void);
void rm_SendHandoverCommand		(void);
u8 rm_BeaerCapVerToChanMode		(T_CNI_RIL3_SPEECH_VERSION ver);

// Addition for SMS-CB

void rm_ProcSmscbcMsg			(void);
void rm_BroadcastNewCbMsg		(u8 page);
void rm_BroadcastNilCbMsg		(void);
void rm_PassDownBlock			(u8 blockSeq, u8 blockType, u8 page, u8 nullFlag);
void rm_ActivateCbchChan		(void);
void rm_SendTuneSynthToTrx		(u8 trx,u8 pwr);
void rm_TuneSynth			(u8 phTrx, u8 synth, u8 hopping, u8 state, u16 arfcn);
void intg_SendCbCmd			(u8 cmdCode, u16 msgCode, u16 updateNo);

// AM
void rm_SendRrRelInd			(rm_IrtEntryId_t entryId);

// ChnAssign

void rm_ProcChanAssignCmd		(void);
void rm_ProcL1ModeAckNAck		(void);
void rm_ProcMsModeModifyAckNack		(void);
void rm_ProcT3L01Expiry			(void);

// Addition for GPRS

void rm_WaitForGrrReadyToGo		(void);
void rm_WaitForCbchChanActAck		(void);
void rm_InitCb				(void);
void rm_ProcGrrMsgs			(void);
bool rm_ValidateGrrMsgs			(void);
void rm_ProcCbSettingChange		(void);
void rm_ProcTrxSlotCombChange		(void);
void rm_ProcTrxSlotLockReq		(void);
void rm_ProcTrxSlotShutdownReq		(void);
void rm_PostTchfRelHandler		(rm_PhyChanDesc_t *pPhyChan);
void rm_PostSdcch4RelHandler		(rm_PhyChanDesc_t *pPhyChan);
bool rm_IsGprsChanReq			(T_CNI_RIL3_ESTABLISH_CAUSE cause);

#endif //__RM_PROTO_H__
