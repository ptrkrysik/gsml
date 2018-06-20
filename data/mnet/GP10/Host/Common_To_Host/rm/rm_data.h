/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_DATA_H__
#define __RM_DATA_H__

#include "rm_head.h"
extern char __DEBUGVAR__;
extern char __RDEBUG__;
extern char __EDEBUG__;
extern char __DDEBUG__;
extern char __UDEBUG__;
extern char __PDEBUG__;
extern char __TDEBUG__;
extern char __BDUMP__ ;
extern char __EDUMP__ ;
extern char __TDUMP__ ;
extern char __UDUMP__ ;

extern DbgOutput rm_Dbg;

extern rm_UmSI2_t       rm_UmSI2;
extern rm_UmSI3_t		rm_UmSI3;
extern rm_UmSI4_t		rm_UmSI4;
extern rm_UmSI5_t		rm_UmSI5;
extern rm_UmSI6_t		rm_UmSI6;

extern T_CNI_IRT_ID	rm_RachEntryId;

extern MSG_Q_ID         mmMsgQId;
extern MSG_Q_ID         rm_MsgQId;
extern T_CNI_L3_ID	rm_id;
extern T_CNI_L3_ID	mm_id;
extern T_CNI_L3_ID	cc_id;
extern rm_UmMsg_t	  rm_UmMsg;
extern rm_SmCtxt_t        *rm_pSmCtxt;
extern rm_SmAction_t      *rm_SmAction[];
extern u8		  rm_ItcRxOrigin;
extern rm_IrtEntryId_t	  rm_ItcRxEntryId;
extern u8		  rm_BootstrapCnf;
extern u16		  rm_ItcRxPrim;
extern u8 		  rm_AgchGroup[];
extern rm_SmscbcMsg_t	  rm_SmscbcMsg;
extern rm_ItcRxMsg_t	  rm_ItcRxMsg;

extern rm_ItcRxMdMsg_t    *rm_pItcRxMdMsg;
extern rm_ItcRxL1Msg_t    *rm_pItcRxL1Msg;
extern rm_ItcRxMmMsg_t    *rm_pItcRxMmMsg;
extern rm_ItcRxBgMsg_t	  *rm_pItcRxBgMsg;
extern rm_ItcRxCcMsg_t    *rm_pItcRxCcMsg;
extern rm_ItcRxRmMsg_t    *rm_pItcRxRmMsg;
extern rm_ItcRxOamMsg_t   *rm_pItcRxOamMsg;
extern rm_ItcRxVbMsg_t	 *rm_pItcRxVbMsg;

extern rm_PhyChanBCcch_t  rm_PhyChanBCcch;
extern rm_PhyChanTchf_t	  rm_PhyChanTchf;
extern rm_PhyChanSdcch4_t rm_PhyChanSdcch4;

extern rm_SmCtxts_t	  rm_SmCtxt;

extern rm_IrtEntryId_t    rm_TchfEntryId[];
extern rm_IrtEntryId_t    rm_Sdcch4EntryId[];

extern T_CNI_L3_ID	  rm_id;
extern T_CNI_L3_ID	  mm_id;
extern T_CNI_L3_ID	  cc_id;

//SMSCBAdded
extern rm_PhyChanDesc_t  *rm_pCbchChan;
extern u8 rm_ChanCbchActive;

//AMADDED
extern const Alarm_errorCode_t rm_DspNotTickingAnyMore[OAMrm_MAX_TRXS];
extern const Alarm_errorCode_t rm_DspNotResponding[OAMrm_MAX_TRXS];
extern const Alarm_errorCode_t rm_TrxRadioOff[OAMrm_MAX_TRXS];
extern const Alarm_errorCode_t rm_TrxAdminLockedAlarmCode[OAMrm_MAX_TRXS];
extern const Alarm_errorCode_t rm_TrxAdminUnlockedAlarmCode[OAMrm_MAX_TRXS];
extern const Alarm_errorCode_t rm_TrxAdminShuttingDownAlarmCode[OAMrm_MAX_TRXS];
extern const Alarm_errorCode_t rm_TrxSlotAdminLockedAlarmCode[OAMrm_MAX_TRXS][OAMrm_SLOTS_PER_TRX];
extern const Alarm_errorCode_t rm_TrxSlotAdminUnlockedAlarmCode[OAMrm_MAX_TRXS][OAMrm_SLOTS_PER_TRX];
extern const Alarm_errorCode_t rm_TrxSlotAdminShuttingDownAlarmCode[OAMrm_MAX_TRXS][OAMrm_SLOTS_PER_TRX];

extern int   rm_ItcRxPrimType;
extern int   rm_ItcRxMsgType;

extern msg_TableHdr_t rm_CbMsgTable;

//GP2
extern int							rm_DbgDump;
extern rm_ItcRxGrrMsg_t*			rm_pItcRxGrrMsg;
extern int							rm_CbchOpState;

extern unsigned char rm_DspCiphCap;

#endif //__RM_DATA_H__
