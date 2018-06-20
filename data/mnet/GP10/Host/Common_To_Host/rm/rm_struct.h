/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef	__RM_STRUCT_H__
#define __RM_STRUCT_H__

#include "rm_head.h"

typedef void rm_SmAction_t ( void );

typedef struct {
        u8	state;	//TRUE: running, FALSE: stopped
	int	time;
	WDOG_ID id;	//MSB-byte: entry id, LSB-byte: timer id
} rm_Timer_t;

typedef struct {
	u8      actType;
        u8      dtxCtrl;
        u8      datAttr;
        u8      ratType;
        u8	codAlgo;
        u8      ciphSet;
        u8      ciphKey[RM_L1CIPHKEY_LEN];
	u8	hoRef;
        u8      bsPower;
        u8      msPower;
        u8      taValue;
        u8      UIC;
} rm_PhyChanActDesc_t;

typedef struct rm_PhyChanDescT
{
//AMADDED
      int 			amState; //Reflection of corresponding TS amState
	int			opState; //Reflection of corresponding TS opState
	int			trxNo;   //Trx where this channel is residing 
	int			slotNo;  //Slot where this channel is residing 
	int			chanIdx; //Index in the chan table of this chann

//PMADDED:
      int 			startTime;
      u8		 	estCause;
      u8			chanBusy;

//HOADDED
	rm_Timer_t		T3105;
      u8			ny1;	 //Init to 0 each time allocated for HO
      u8 			hoTa;    //Got from RA on DCCH, measured at DSP
      u16			hoRef;   //Remember handover ref # MM sent over
      u8			codAlgo; //Remember speech algorithm version
	u8			usable;
	u8			state;
      rm_UmEdChanType_t	chanType;
	u8			subChanNumber;
	u8			trxNumber;
	u16			chanNumber;
	u8			chanNumberMSB;
	u8			chanNumberLSB;
	u8			TSC;
	u8			quality;
	u8			rslCount;
	rm_Timer_t		T3101;
	rm_Timer_t		T3111;
	rm_Timer_t		T3109;
	rm_Timer_t		T3L01;
	rm_Timer_t		T3L02;
	rm_IrtEntryId_t	mEntryId;
	rm_IrtEntryId_t	aEntryId;
	rm_LapdmOId_t	mOId;
	rm_LapdmOId_t   	aOId;
      u8			savChanReq[RM_SAVCHANREQ_LEN];
	struct rm_SmCtxtT	*pMSmCtxt;
	struct rm_SmCtxtT	*pASmCtxt;
} rm_PhyChanDesc_t,
  rm_PhyChanBCcch_t[RM_L1MAX_BCCCHS],
  rm_PhyChanTchf_t[RM_L1MAX_TCHFS],
  rm_PhyChanSdcch4_t[RM_L1MAX_SDCCH4S];

typedef struct rm_SmCtxtT
{
      T_CNI_RIL3_CAUSE_VALUE	relCause;
      int			waitHoComplt;
      //ChnAssign
      u8			spchVer;
      u8			state;
      u8			isForHo;
      u8 			codAlgo;
      //ChnAssign
      u8			callIdx;
      rm_IrtEntryId_t		entryId;
      struct rm_PhyChanDescT	*pPhyChan;
      u8			smType;		//Main SM or Acch SM
      rm_LapdmOId_t 		mOId;
      rm_LapdmOId_t 		aOId;
      rm_IrtEntryId_t		mEntryId;
      rm_IrtEntryId_t		aEntryId;
      u8			callType;
      rm_LapdmSapi_t	        sapi; 
      rm_LapdmGsmChanType_t	chanType;
      rm_LapdmRelMode_t	relMode;
} rm_SmCtxt_t,
  rm_SmCtxts_t[RM_MAX_SMCTXTS];

#define RM_SMCTXT_NULL		(rm_SmCtxt_t *) 0

//SMSCBC
typedef struct msg_Table{
     int state;	                //1: activing, -1: inactiving, 0:empty row
     u8  msg[INTER_CBC_CB_MAX_DAT_LEN];
     u8  msgId;	                //message id
     u8	 curPageInCurMsg;
     u8  totPage; 
     u16 size;
}rm_SmsCbc_msg_t; 

typedef struct msg_TableHdr
{
     rm_SmsCbc_msg_t	table[SmsCbc_TableRow_MAX];
     u8			curMsg;

}  msg_TableHdr_t;

#endif //__RM_STRUCT_H__
