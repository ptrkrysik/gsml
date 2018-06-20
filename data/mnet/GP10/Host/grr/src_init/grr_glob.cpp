/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __GRR_GLOB_CPP__
#define __GRR_GLOB_CPP__

#include "grr\grr_head.h"

//G2TEST
int							grr_DbgDump=0;
int							grr_GprsCtrl=0;

JCTask grrTaskObj("grrTask");
JCTask grrMonTaskObj("grrMonTask");

unsigned char				grr_DspCiphCap=0;

grr_OamData_t					grr_OamData;
grr_TrxMgmt_t					grr_TrxMgmt[OAMgrr_MAX_TRXS];
MSG_Q_ID        				      grr_MsgQId;
Alarm_errorCode_t  				grr_CurAlarm;

unsigned char	 				grr_ItcRxOrigin;
grr_ItcRxMsg_t					grr_ItcRxMsg;

grr_ItcRxDspMsg_t				     *grr_pItcRxDspMsg;
grr_ItcRxRlcMsg_t		                 *grr_pItcRxRlcMsg;
grr_ItcRxGrrMsg_t       	           *grr_pItcRxGrrMsg;
grr_ItcRxOamMsg_t             	     *grr_pItcRxOamMsg;
grr_ItcRxRrmMsg_t                  	     *grr_pItcRxRrmMsg;

T_CNI_RIL3RRM_MSG 				grr_Msg;

grr_Si2_t		grr_Si2;
grr_Si3_t		grr_Si3;
grr_Si4_t		grr_Si4;
grr_Si5_t		grr_Si5;
grr_Si6_t		grr_Si6;
grr_Si13_t		grr_Si13;


//OAM tables definitions
//
const unsigned short grr_phTrxSelectBit[2]= {0x0000,0x4000};  	     //b14
const unsigned short grr_phTrxSynthSelectBit[2] = {0x0000,0x8000};     //b15
const unsigned short grr_phTrxHoppingSelectBit[2] = {0x0000,0x2000};   //b13
const unsigned short grr_phTrxPwrSelectBit[2] = {0x0000,0x1000};       //b12

//Tables in Basic Package
CellAllocationEntry 		cellAllocationTable[CellAllocationTableLimit];
PlmnPermittedEntry 		plmnPermittedTable[PlmnPermittedTableLimit];

//Table in Option Package
NotAllowedAccessClassEntry 	notAllowedAccessClassTable[NotAllowedAccessClassTableLimit];

//Table in TRX packages
ChannelEntry			firstTrxPackageChannelTable[ChannelTableLimit];
ChannelEntry			secondTrxPackageChannelTable[ChannelTableLimit];

CarrierFrequencyEntry 		firstRadioCarrierPackageFrequencyListTable[CarrierFrequencyTableLimit];
CarrierFrequencyEntry 		secondRadioCarrierPackageFrequencyListTable[CarrierFrequencyTableLimit];

//Tables in AdjCell Packages
AdjCell_HandoverEntry 		adjCell_HandoverTable[HandoverTableLimit];
AdjCell_HandoverEntry 		adjCell_HandoverTable_t[HandoverTableLimit];
AdjCell_ReselectionEntry 	adjCell_ReselectionTable[ReselectionTableLimit];

//A complete list of GRR-raised alarms from reference

//Section I: critical alarms

//EC_GRR_BOOTING_UP

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
//EC_GRR_MIB_INVALID_CB_CONFIG

const Alarm_errorCode_t
      grr_DspNotResponding[OAMgrr_MAX_TRXS]
      = {EC_GRR_DSP_1_NOT_RESPONDING, EC_GRR_DSP_2_NOT_RESPONDING};

const Alarm_errorCode_t
      grr_DspNotTickingAnyMore[OAMgrr_MAX_TRXS]
      = {EC_GRR_DSP_1_NOT_TICKING, EC_GRR_DSP_2_NOT_TICKING};

const Alarm_errorCode_t
      grr_DspInitWatchTimerExpiredAlarmCode[OAMgrr_MAX_TRXS]
      = {EC_GRR_DSP_1_INIT_WATCH_TIMER_EXPIRED, EC_GRR_DSP_2_INIT_WATCH_TIMER_EXPIRED};

const Alarm_errorCode_t
      grr_TrxSlotActNackAlarmCode[OAMgrr_MAX_TRXS][8]
      = { {EC_GRR_TRX_1_SLOT_1_ACT_NACK, EC_GRR_TRX_1_SLOT_2_ACT_NACK,
	     EC_GRR_TRX_1_SLOT_3_ACT_NACK, EC_GRR_TRX_1_SLOT_4_ACT_NACK,
           EC_GRR_TRX_1_SLOT_5_ACT_NACK, EC_GRR_TRX_1_SLOT_6_ACT_NACK,
	     EC_GRR_TRX_1_SLOT_7_ACT_NACK, EC_GRR_TRX_1_SLOT_8_ACT_NACK
          },
	
	    {EC_GRR_TRX_2_SLOT_1_ACT_NACK, EC_GRR_TRX_2_SLOT_2_ACT_NACK,
	     EC_GRR_TRX_2_SLOT_3_ACT_NACK, EC_GRR_TRX_2_SLOT_4_ACT_NACK,
           EC_GRR_TRX_2_SLOT_5_ACT_NACK, EC_GRR_TRX_2_SLOT_6_ACT_NACK,
	     EC_GRR_TRX_2_SLOT_7_ACT_NACK, EC_GRR_TRX_2_SLOT_8_ACT_NACK
          }
       };


//Section II: Informational Alarms

//Informational alarms related to trx slot channel combination change
const Alarm_errorCode_t
      grr_TrxSlotChanCombChangeAlarmCode[OAMgrr_MAX_TRXS][8]
      = { {EC_GRR_TRX_1_SLOT_1_CHANCOMB_CHANGED, EC_GRR_TRX_1_SLOT_2_CHANCOMB_CHANGED,
	     EC_GRR_TRX_1_SLOT_3_CHANCOMB_CHANGED, EC_GRR_TRX_1_SLOT_4_CHANCOMB_CHANGED,
           EC_GRR_TRX_1_SLOT_5_CHANCOMB_CHANGED, EC_GRR_TRX_1_SLOT_6_CHANCOMB_CHANGED,
	     EC_GRR_TRX_1_SLOT_7_CHANCOMB_CHANGED, EC_GRR_TRX_1_SLOT_8_CHANCOMB_CHANGED
          },
	
	    {EC_GRR_TRX_2_SLOT_1_CHANCOMB_CHANGED, EC_GRR_TRX_2_SLOT_2_CHANCOMB_CHANGED,
	     EC_GRR_TRX_2_SLOT_3_CHANCOMB_CHANGED, EC_GRR_TRX_2_SLOT_4_CHANCOMB_CHANGED,
           EC_GRR_TRX_2_SLOT_5_CHANCOMB_CHANGED, EC_GRR_TRX_2_SLOT_6_CHANCOMB_CHANGED,
	     EC_GRR_TRX_2_SLOT_7_CHANCOMB_CHANGED, EC_GRR_TRX_2_SLOT_8_CHANCOMB_CHANGED
          }
       };


//Informational alarm related to administrative state operations of radio resoureces
const Alarm_errorCode_t
      grr_TrxAmStateLockedAlarmCode[OAMgrr_MAX_TRXS] 
      = {EC_GRR_TRX_1_LOCKED,EC_GRR_TRX_2_LOCKED};

const Alarm_errorCode_t
      grr_TrxAmStateUnlockedAlarmCode[OAMgrr_MAX_TRXS]
      = {EC_GRR_TRX_1_UNLOCKED,EC_GRR_TRX_2_UNLOCKED};

const Alarm_errorCode_t
      grr_TrxAmStateShuttingdownAlarmCode[OAMgrr_MAX_TRXS]
      = {EC_GRR_TRX_1_SHUTTINGDOWN,EC_GRR_TRX_2_SHUTTINGDOWN};

const Alarm_errorCode_t
      grr_TrxAmStateInvalidAlarmCode[OAMgrr_MAX_TRXS] 
      = {EC_GRR_TRX_1_AMSTATE_INVALID,EC_GRR_TRX_2_AMSTATE_INVALID};

const Alarm_errorCode_t
      grr_TrxSlotAmStateLockedAlarmCode[OAMgrr_MAX_TRXS][8]
      = {
          {EC_GRR_TRX_1_SLOT_1_LOCKED, EC_GRR_TRX_1_SLOT_2_LOCKED,
	     EC_GRR_TRX_1_SLOT_3_LOCKED, EC_GRR_TRX_1_SLOT_4_LOCKED,
           EC_GRR_TRX_1_SLOT_5_LOCKED, EC_GRR_TRX_1_SLOT_6_LOCKED,
	     EC_GRR_TRX_1_SLOT_7_LOCKED, EC_GRR_TRX_1_SLOT_8_LOCKED},
	
	    {EC_GRR_TRX_2_SLOT_1_LOCKED, EC_GRR_TRX_2_SLOT_2_LOCKED,
	     EC_GRR_TRX_2_SLOT_3_LOCKED, EC_GRR_TRX_2_SLOT_4_LOCKED,
           EC_GRR_TRX_2_SLOT_5_LOCKED, EC_GRR_TRX_2_SLOT_6_LOCKED,
	     EC_GRR_TRX_2_SLOT_7_LOCKED, EC_GRR_TRX_2_SLOT_8_LOCKED}
	  };

const Alarm_errorCode_t
      grr_TrxSlotAmStateUnlockedAlarmCode[OAMgrr_MAX_TRXS][8]
      = {
	    {EC_GRR_TRX_1_SLOT_1_UNLOCKED, EC_GRR_TRX_1_SLOT_2_UNLOCKED,
	     EC_GRR_TRX_1_SLOT_3_UNLOCKED, EC_GRR_TRX_1_SLOT_4_UNLOCKED,
           EC_GRR_TRX_1_SLOT_5_UNLOCKED, EC_GRR_TRX_1_SLOT_6_UNLOCKED,
	     EC_GRR_TRX_1_SLOT_7_UNLOCKED, EC_GRR_TRX_1_SLOT_8_UNLOCKED},
	
	    {EC_GRR_TRX_2_SLOT_1_UNLOCKED, EC_GRR_TRX_2_SLOT_2_UNLOCKED,
	     EC_GRR_TRX_2_SLOT_3_UNLOCKED, EC_GRR_TRX_2_SLOT_4_UNLOCKED,
           EC_GRR_TRX_2_SLOT_5_UNLOCKED, EC_GRR_TRX_2_SLOT_6_UNLOCKED,
	     EC_GRR_TRX_2_SLOT_7_UNLOCKED, EC_GRR_TRX_2_SLOT_8_UNLOCKED}
	  };

const Alarm_errorCode_t
      grr_TrxSlotAmStateShuttingdownAlarmCode[OAMgrr_MAX_TRXS][8]
      = {
	    {EC_GRR_TRX_1_SLOT_1_SHUTTINGDOWN, EC_GRR_TRX_1_SLOT_2_SHUTTINGDOWN,
	     EC_GRR_TRX_1_SLOT_3_SHUTTINGDOWN, EC_GRR_TRX_1_SLOT_4_SHUTTINGDOWN,
           EC_GRR_TRX_1_SLOT_5_SHUTTINGDOWN, EC_GRR_TRX_1_SLOT_6_SHUTTINGDOWN,
	     EC_GRR_TRX_1_SLOT_7_SHUTTINGDOWN, EC_GRR_TRX_1_SLOT_8_SHUTTINGDOWN},
	
	    {EC_GRR_TRX_2_SLOT_1_SHUTTINGDOWN, EC_GRR_TRX_2_SLOT_2_SHUTTINGDOWN,
	     EC_GRR_TRX_2_SLOT_3_SHUTTINGDOWN, EC_GRR_TRX_2_SLOT_4_SHUTTINGDOWN,
           EC_GRR_TRX_2_SLOT_5_SHUTTINGDOWN, EC_GRR_TRX_2_SLOT_6_SHUTTINGDOWN,
	     EC_GRR_TRX_2_SLOT_7_SHUTTINGDOWN, EC_GRR_TRX_2_SLOT_8_SHUTTINGDOWN}
	  };


#endif //__GRR_GLOB_CPP__
