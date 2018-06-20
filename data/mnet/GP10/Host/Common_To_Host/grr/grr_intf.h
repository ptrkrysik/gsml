/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_INTF_H__
#define __GRR_INTF_H__

//#include "grr_head.h"				//message structure def. btw grr and rlc&rrm
#include "vxWorks.h"
#include "stdio.h"
#include "string.h"
#include "msgQlib.h"
#include "vipermib.h"
#include "ril3/ril3_common.h"
#include "ril3\ril3_gmm_msg.h"

extern MSG_Q_ID		grr_MsgQId;

//Administrative & operational state & chan config related inquiries
//
extern AdministrativeState 				grr_GetBtsAmState	      (void);
extern AdministrativeState 				grr_GetTrxAmState		  (int trx);
extern AdministrativeState 				grr_GetTrxSlotAmState	  (int trx, int slot);

extern EnableDisable 					grr_GetBtsOpState		  (void);
extern EnableDisable 					grr_GetTrxOpState		  (int trx);
extern EnableDisable 					grr_GetTrxSlotOpState	  (int trx, int slot);

extern bool					grr_IsPccchExisting		  (int *trx, int* slot, int* comb);
extern int					grr_GetTrxSlotChanComb	  (int trx, int slot);
extern int					grr_GetTrxSlotTsc		  (int trx, int slot);
extern int					grr_GetTrxSlotArfcn		  (int trx, int slot);
extern int					grr_getBtsGprsNetworkMode (void);



extern bool								grr_IsBtsBlocked		  (void);
extern bool								grr_IsBtsAmTypeBlocked	  (void);
extern bool								grr_IsBtsOpTypeBlocked	  (void);
extern bool								grr_IsBtsGprsFeatureActive(void);
extern bool								grr_IsBtsCbFeatureActive  (void);

//Update application modules i.e. RLC, RRM with current setting
//
extern void grr_SendTrxSlotChanComb		(int src_module, MSG_Q_ID dst_q, int trx, int slot, int chanComb);
extern void grr_SendGprsFeatureSetting	(int src_module, MSG_Q_ID dst_q);
extern void	grr_SendGprsFeatureSetting	(int src_module, MSG_Q_ID dst_q);
extern void	grr_SendCbFeatureSetting	(int src_module, MSG_Q_ID dst_q);
extern void	grr_SendReadyToGoReq		(int src_module, MSG_Q_ID dst_q);
extern void	grr_SendReadyToGoAck		(int src_module, MSG_Q_ID dst_q);


//Administrative State related operations: requests and acknowledgements
//
//extern void	grr_SendBtsLockReq	    (int src_module, MSG_Q_ID dst_q);
//extern void	grr_SendBtsUnlockReq	(int src_module, MSG_Q_ID dst_q);
//extern void	grr_SendBtsShutdownReq	(int src_module, MSG_Q_ID dst_q);
//extern void	grr_SendBtsLockAck	    (int src_module, MSG_Q_ID dst_q);
//extern void	grr_SendBtsUnlockAck	(int src_module, MSG_Q_ID dst_q);
//extern void	grr_SendBtsShutdownAck	(int src_module, MSG_Q_ID dst_q);

//extern void	grr_SendTrxLockReq	    (int src_module, MSG_Q_ID dst_q, int trx);
//extern void	grr_SendTrxUnlockReq	(int src_module, MSG_Q_ID dst_q, int trx);
//extern void	grr_SendTrxShutdownReq	(int src_module, MSG_Q_ID dst_q, int trx);
//extern void	grr_SendTrxLockAck	    (int src_module, MSG_Q_ID dst_q, int trx);
//extern void	grr_SendTrxUnlockAck	(int src_module, MSG_Q_ID dst_q, int trx);
//extern void	grr_SendTrxShutdownAck	(int src_module, MSG_Q_ID dst_q, int trx);

extern void	grr_SendTrxSlotLockReq		(int src_module, MSG_Q_ID dst_q, int trx, int slot);
extern void	grr_SendTrxSlotShutdownReq	(int src_module, MSG_Q_ID dst_q, int trx, int slot);
extern void	grr_SendTrxSlotLockAck		(int src_module, MSG_Q_ID dst_q, int trx, int slot);
extern void	grr_SendTrxSlotShutdownAck	(int src_module, MSG_Q_ID dst_q, int trx, int slot);
//extern void	grr_SendTrxSlotUnlockAck(int src_module, MSG_Q_ID dst_q, int trx, int slot);
//extern void	grr_SendTrxSlotUnlockReq(int src_module, MSG_Q_ID dst_q, int trx, int slot);

//Synchronize with partners
//
extern void	grr_SendCbCfgSetting		(int src_module, MSG_Q_ID dst_q);
extern void	grr_SendReadyToGoReq		(int src_module, MSG_Q_ID dst_q);
extern void	grr_SendReadyToGoAck		(int src_module, MSG_Q_ID dst_q);
extern void grr_OverloadedCellBarOff	(void);

//Messaging related to radio resource scheduling
//
extern void	grr_ReportRadioResUsage		(int src_module, MSG_Q_ID dst_q);
extern void	grr_GetRadioResRelCandiate	(int *trx, int *slot);

//Pass messages from L1proxy to GRR
//
void grr_SendMsgFromL1ToGrr(unsigned short length, unsigned char* buf);

T_CNI_RIL3_ACCESS_TECHNOLOGY_TYPE grr_GetRadioAccessTechnology(void);

//Test functions
//
void grr_SetTrxSlotCombination(int trx, int slot, int comb);

short grr_DecideCipherAlgo(T_CNI_RIL3_IE_CIPHER_MODE_SETTING *setting,
                           T_CNI_RIL3_IE_MS_CLASSMARK_2 *classmark2);

#endif  //__GRR_INTF_H__
