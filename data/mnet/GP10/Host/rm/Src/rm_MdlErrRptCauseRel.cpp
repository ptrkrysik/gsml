/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_MDLERRRPTCAUSEREL_CPP__
#define __RM_MDLERRRPTCAUSEREL_CPP__

#include "rm\rm_head.h"

void rm_MdlErrRptCauseRel(void)
{
  rm_LapdmOId_t       		lapdmOId;

  RDEBUG__(("ENTER-rm_MdlErrRptCauseRel: cause=%d\n", rm_pItcRxMdMsg->error_cause));

  if (rm_pSmCtxt->smType==RM_MAIN_SM)
  {
      //SMS-SAPI3-Begin

      //lapdmOId = rm_pSmCtxt->mOId;
      rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
      //rm_SendRrRelInd(RM_MDLERR,rm_ItcRxEntryId);
      //rm_SendDlRelReq(lapdmOId, RM_SAPI0, RM_LOCAL_RELEASE);

      //Cut off trx/slot <--> hrtp connection 
      //if (rm_pSmCtxt->state == RM_S10TRAFFICMODE_ESTABLISHED)
      //{
      //    u8 trx, slot;
      //    trx  = rm_pSmCtxt->pPhyChan->trxNumber;
      //    slot = (rm_pSmCtxt->pPhyChan->chanNumberMSB) & 0x07;
           
      //    int ret1;
      //    ret1 = clrConnectionByTrxSlot(trx, slot);
      //    if (ret1<0)
      //    { 
      //        EDEBUG__(("ERROR-rm_S10TrafficModeEstablished:
      //                   Clear TN-hrtp conn failed:trx=%d/slot=%d\n",
      //                   trx, slot));
	//        return;
      //    }
      //}

      //if (rm_pSmCtxt->sapi != RM_SAPI0)
      //    rm_SendDlRelReq(lapdmOId, rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);

      //rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI3, RM_LOCAL_RELEASE);
      //rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI3, RM_LOCAL_RELEASE);

      //rm_SmCtxt_t *pASmCtxt;
      //pASmCtxt = rm_pSmCtxt->pChan->pASmCtxt;
      //pASmCtxt->state = RM_S1RM_RUNNING;

      //SMS-SAPI3-End

      rm_StartTimer(rm_ItcRxEntryId, RM_T3109);
	
      rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER;
                             //CNI_RIL3_CAUSE_TEMPORARY_FAILURE; //41
      //Adjust SM state to S7
      rm_pSmCtxt->state           = RM_S7RELEASING_SIGTRAFFICCHAN;
      rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	
   } 
   else 
   { 
      //SMS-SAPI3-Begin

      IDEBUG__(("WARNING@rm_MdlErrRptCauseRel:MDL for aEntryId %d\n",
                 rm_ItcRxEntryId));

      //lapdmOId = rm_pSmCtxt->aOId;

      //rm_SendRrRelInd(RM_MDLERR, rm_ItcRxEntryId);
      //rm_SendDlRelReq(lapdmOId, RM_SAPI3, RM_LOCAL_RELEASE);

      //SMS-SAPI3-End

      rm_pSmCtxt->state           = RM_S1RM_RUNNING;
   }  
}   
 
#endif //__RM_MDLERRRPTCAUSEREL_CPP__
