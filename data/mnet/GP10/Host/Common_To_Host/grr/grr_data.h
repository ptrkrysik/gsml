/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_DATA_H__
#define __GRR_DATA_H__

#include "grr_head.h"

extern JCTask grrTaskObj;
extern JCTask grrMonTaskObj;

//G2TEST
extern int								grr_DbgDump;
extern int							    grr_GprsCtrl;

extern unsigned char					grr_DspCiphCap;
extern grr_OamData_t					grr_OamData;
extern grr_TrxMgmt_t					grr_TrxMgmt[OAMgrr_MAX_TRXS];
extern MSG_Q_ID       				    grr_MsgQId;

extern unsigned char	 				grr_ItcRxOrigin;
extern grr_ItcRxMsg_t					grr_ItcRxMsg;

extern grr_ItcRxDspMsg_t				   *grr_pItcRxDspMsg;
extern grr_ItcRxRlcMsg_t		       *grr_pItcRxRlcMsg;
extern grr_ItcRxGrrMsg_t       	       *grr_pItcRxGrrMsg;
extern grr_ItcRxOamMsg_t               *grr_pItcRxOamMsg;
extern grr_ItcRxRrmMsg_t               *grr_pItcRxRrmMsg;

extern T_CNI_RIL3RRM_MSG 				grr_Msg;
extern grr_Si2_t					    grr_Si2;
extern grr_Si3_t					    grr_Si3;
extern grr_Si4_t					    grr_Si4;
extern grr_Si5_t					    grr_Si5;
extern grr_Si6_t						grr_Si6;
extern grr_Si13_t						grr_Si13;


//OAM tables definitions
//
extern const unsigned short grr_phTrxSelectBit[2];          //b14
extern const unsigned short grr_phTrxSynthSelectBit[2];     //b15
extern const unsigned short grr_phTrxHoppingSelectBit[2];   //b13
extern const unsigned short grr_phTrxPwrSelectBit[2];       //b12

//Tables in Basic Package
extern CellAllocationEntry 				cellAllocationTable[CellAllocationTableLimit];
extern PlmnPermittedEntry 				plmnPermittedTable[PlmnPermittedTableLimit];

//Table in Option Package
extern NotAllowedAccessClassEntry 		notAllowedAccessClassTable[NotAllowedAccessClassTableLimit];

//Table in TRX packages
extern ChannelEntry						firstTrxPackageChannelTable[ChannelTableLimit];
extern ChannelEntry						secondTrxPackageChannelTable[ChannelTableLimit];

extern CarrierFrequencyEntry 			firstRadioCarrierPackageFrequencyListTable[CarrierFrequencyTableLimit];
extern CarrierFrequencyEntry 			secondRadioCarrierPackageFrequencyListTable[CarrierFrequencyTableLimit];

//Tables in AdjCell Packages
extern AdjCell_HandoverEntry 			adjCell_HandoverTable[HandoverTableLimit];
extern AdjCell_HandoverEntry 			adjCell_HandoverTable_t[HandoverTableLimit];
extern AdjCell_ReselectionEntry 		adjCell_ReselectionTable[ReselectionTableLimit];

//A complete list of GRR-raised alarms from reference

//Section I: critical alarms

//EC_BOOTING_UP

//EC_GRR_OS_CALL_WDCREATE_FAILED
//EC_GRR_OS_CALL_WDSTART_FAILED
//EC_GRR_OS_CALL_MSGQCREATE_FAILED
//EC_GRR_OS_CALL_MSGQRECEIVE_FAILED
//EC_GRR_OS_CALL_MSGQSEND_FAILED

//EC_GRR_OAM_CALL_SETMIBINTVAR_FAILED
//EC_GRR_OAM_CALL_SETTBLENTRYFIELD_FAILED
//EC_GRR_OAM_CALL_GETMIBSTRUCT_FAILED
//EC_GRR_OAM_CALL_GETMIBTBL_FAILED
//EC_GRR_OAM_CALL_UNSETTRAPBYMODULE_FAILED
//EC_GRR_OAM_CALL_SETTRAP_FAILED

//EC_GRR_RIL3_CALL_RRM_ENCODER_FAILED

//EC_GRR_BTS_LOCKED
//EC_GRR_BTS_UNLOCKED
//EC_GRR_BTS_SHUTTINGDOWN

//Critical alarms
extern const Alarm_errorCode_t grr_DspNotResponding[OAMgrr_MAX_TRXS];
extern const Alarm_errorCode_t grr_DspNotTickingAnyMore[OAMgrr_MAX_TRXS];
extern const Alarm_errorCode_t grr_DspInitWatchTimerExpiredAlarmCode[OAMgrr_MAX_TRXS];
extern const Alarm_errorCode_t grr_TrxSlotActNackAlarmCode[OAMgrr_MAX_TRXS][8];
      
//Informational Alarms
extern const Alarm_errorCode_t grr_TrxSlotChanCombChangeAlarmCode[OAMgrr_MAX_TRXS][8];
extern const Alarm_errorCode_t grr_TrxAmStateLockedAlarmCode[OAMgrr_MAX_TRXS];
extern const Alarm_errorCode_t grr_TrxAmStateUnlockedAlarmCode[OAMgrr_MAX_TRXS];
extern const Alarm_errorCode_t grr_TrxAmStateShuttingdownAlarmCode[OAMgrr_MAX_TRXS];
extern const Alarm_errorCode_t grr_TrxAmStateInvalidAlarmCode[OAMgrr_MAX_TRXS];
      
extern const Alarm_errorCode_t grr_TrxSlotAmStateLockedAlarmCode[OAMgrr_MAX_TRXS][8];
extern const Alarm_errorCode_t grr_TrxSlotAmStateUnlockedAlarmCode[OAMgrr_MAX_TRXS][8];
extern const Alarm_errorCode_t grr_TrxSlotAmStateShuttingdownAlarmCode[OAMgrr_MAX_TRXS][8];

//Variables defined outside
//
extern MSG_Q_ID		rlcMsgQId;
extern MSG_Q_ID		rm_MsgQId;      

#endif //__GRR_DATA_H__
