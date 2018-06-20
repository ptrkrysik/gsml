/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_SENDASYNCHOREQUIRED_CPP__
#define __RM_SENDASYNCHOREQUIRED_CPP__

#include "rm\rm_head.h"

void rm_SendAsyncHoRequired(void)
{
   rm_ItcTxL3Msg_t  msgToCc;	     /* Send inter CC-RM siganling */
   T_CNI_L3_ID      ccId;
   IntraL3HandoverRequired_t* pCcHoRequired;     
 
   /* Monitoring entrance to a function */
   DBG_FUNC("rm_SendAsyncHoRequired", RM_LAYER);
   DBG_ENTER();
 
   /* TBD: may contain SAPI later if requested */
   msgToCc.module_id      = MODULE_RM;
   msgToCc.entry_id	  = rm_ItcRxEntryId;
   msgToCc.primitive_type = INTRA_L3_DATA;
   msgToCc.message_type   = INTRA_L3_RR_HANDOVER_REQUIRED;

   //Send Handover Required message to CC if CC QId found
   ccId = CNI_RIL3_IRT_Get_CC_Id(rm_ItcRxEntryId);
   if ( ccId.msgq_id == CNI_NULL_L3_ID.msgq_id &&
        ccId.sub_id  == CNI_NULL_L3_ID.sub_id  )
       EDEBUG__(("ERROR-rm_SendAsyncHoRequired: null CC QId\n"));
   else
   {
       //Found CC QId, send Handed-over candidates to CC
       pCcHoRequired = (IntraL3HandoverRequired_t*)&(msgToCc.l3_data.handReqd);
       pCcHoRequired->hoCause = rm_pItcRxL1Msg->l3_data.buffer[7];
       pCcHoRequired->numCandidates = rm_pItcRxL1Msg->l3_data.buffer[8];

       DBG_TRACE("TRACE-rm_SendAsyncHoRequired: hoCause %x numCand %d\n",
                  pCcHoRequired->hoCause, pCcHoRequired->numCandidates);
//HOADDED
//TEST DATA here
//<taiyu:05-18-01> Changed to Global Cell ID
       int j;
       int mccInteger;
       int mncInteger;
       for (int i=0;i<pCcHoRequired->numCandidates;i++)
       {    
            j = rm_pItcRxL1Msg->l3_data.buffer[9+(i<<1)];
            pCcHoRequired->candGlobalCellId[i].candGlobalCellId.ci_disc = 0; // Whole cell global Id;

            mccInteger = OAMrm_NCELL_MCC(j);
            pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mcc[0] = (mccInteger / 100) % 10;
            pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mcc[1] = (mccInteger / 10) % 10;
            pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mcc[2] = mccInteger % 10;

            mncInteger = OAMrm_NCELL_MNC(j);
            pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mnc[0] = (mncInteger / 10) % 10;
            pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mnc[1] = mncInteger % 10;
            pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mnc[2] = 0xf;

            pCcHoRequired->candGlobalCellId[i].candGlobalCellId.lac = OAMrm_NCELL_LAC(j);
            pCcHoRequired->candGlobalCellId[i].candGlobalCellId.ci = OAMrm_NCELL_CI(j);

            pCcHoRequired->candGlobalCellId[i].candIsExternal = OAMrm_NCELL_ISEXTERNAL(j);
            pCcHoRequired->candGlobalCellId[i].candIsTried = 0;
        
            DBG_TRACE("TRACE-rm_SendAsyncHoRequired: Cand MCC %x%x%x MNC %x%x%x Lac %d Ci %d IsExtern %d IsTried %d\n",
                  pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mcc[0], pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mcc[1], pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mcc[2], 
                  pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mnc[0], pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mnc[1], pCcHoRequired->candGlobalCellId[i].candGlobalCellId.mnc[2], 
                  pCcHoRequired->candGlobalCellId[i].candGlobalCellId.lac, pCcHoRequired->candGlobalCellId[i].candGlobalCellId.ci, pCcHoRequired->candGlobalCellId[i].candIsExternal,
                  pCcHoRequired->candGlobalCellId[i].candIsTried);
       }
             
       api_SendMsgFromRmToCc(ccId.msgq_id, sizeof(rm_ItcTxL3Msg_t), (u8*)&msgToCc );
   }
 
} /* End of rm_SendAsyncHoRequired() */

#endif /* __RM_SENDASYNCHOREQUIRED_CPP__ */

