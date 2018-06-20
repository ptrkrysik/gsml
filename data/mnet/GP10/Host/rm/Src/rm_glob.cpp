/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_GLOB_CPP__
#define __RM_GLOB_CPP__

#include "rm\rm_head.h"

char __DEBUGVAR__;
char __RDEBUG__ =1;
char __EDEBUG__ =1;
char __DDEBUG__ =1;
char __UDEBUG__ =0;
char __PDEBUG__ =1;
char __TDEBUG__ =0;
char __BDUMP__  =0;
char __EDUMP__  =0;
char __TDUMP__  =0;
char __UDUMP__  =0;

rm_UmSI2_t      rm_UmSI2; 
rm_UmSI3_t	    rm_UmSI3;
rm_UmSI4_t	    rm_UmSI4;
rm_UmSI5_t	    rm_UmSI5;
rm_UmSI6_t	    rm_UmSI6;


rm_UmMsg_t	rm_UmMsg;
rm_SmCtxt_t     *rm_pSmCtxt;

T_CNI_IRT_ID	rm_RachEntryId;
//T_CNI_L3_ID	rm_id, cc_id;

rm_SmAction_t * rm_SmAction []=
{
	rm_S0Null,				/* S0: TBD */
	rm_S1RmRunning,				/* S1: TBD */
	rm_S2ActivatingSigTrafficChan,  	/* S2 */
	rm_S3SigChanActivated,          	/* S3 */
	rm_S4TrafficChanActivated,		/* S4 */
	rm_S5SigModeEstablished,		/* S5 */
	rm_S6ActivatingTrafficMode,		/* S6 */
	rm_S7ReleasingSigTrafficChan,		/* S7 */
	rm_S8TrafficModeActivated,		/* S8 */
	rm_S9DeactivatingSigTrafficChan, 	/* S9 */
	rm_S10TrafficModeEstablished,		/* S10 */
	rm_S11TrafficChanSuspending		/* S11 */
};

u8		   rm_ItcRxOrigin;               
rm_IrtEntryId_t	  rm_ItcRxEntryId;
u8		   rm_BootstrapCnf;
u16		   rm_ItcRxPrim;

int		   rm_ItcRxPrimType;
int		   rm_ItcRxMsgType;

u8 		   rm_AgchGroup[]={0,1,2,3,4,5,6,7,8};
rm_ItcRxMsg_t	   rm_ItcRxMsg;
rm_SmscbcMsg_t	   rm_SmscbcMsg;

//MSG_Q_ID	   rm_MsgQId;		/* TBD: used in sys_init.cpp*/
rm_ItcRxMdMsg_t    *rm_pItcRxMdMsg;
rm_ItcRxL1Msg_t    *rm_pItcRxL1Msg;
rm_ItcRxMmMsg_t    *rm_pItcRxMmMsg;
rm_ItcRxBgMsg_t    *rm_pItcRxBgMsg;
rm_ItcRxCcMsg_t    *rm_pItcRxCcMsg;
rm_ItcRxRmMsg_t    *rm_pItcRxRmMsg;
rm_ItcRxOamMsg_t   *rm_pItcRxOamMsg;
rm_ItcRxVbMsg_t	 *rm_pItcRxVbMsg;
rm_PhyChanBCcch_t  rm_PhyChanBCcch;

//rm_PhyChanTchf_t   rm_PhyChanTchf;       //Move to rm_init
//rm_PhyChanSdcch4_t rm_PhyChanSdcch4;	 //Move to rm_init

//The following has been moved to src_init
//rm_SmCtxts_t	   rm_SmCtxt;
//rm_IrtEntryId_t	   rm_TchfEntryId[15];  //TBD:RM_L1MAX_TCHFS];
//rm_IrtEntryId_t	   rm_Sdcch4EntryId[4]; //TBDRM_L1MAX_SDCCH4S];


//PR 1323 SMSCBC Added
rm_PhyChanDesc_t  *rm_pCbchChan;
u8 			rm_ChanCbchActive;
msg_TableHdr_t rm_CbMsgTable;

//GP2
//
int rm_CbchOpState;
int rm_DbgDump=1;
rm_ItcRxGrrMsg_t  *rm_pItcRxGrrMsg;

#endif /* __RM_GLOB_CPP__ */
