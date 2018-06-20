/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_DESPATCH_CPP__
#define __RM_DESPATCH_CPP__

#include "rm\rm_head.h"

void rm_Despatch( void )
{
   //Monitor entrance to this func
   RDEBUG__(("ENTER-rm_Despatch\n"));

   //Check if watchdog timer expired
   //if (rm_ItcRxOrigin==MODULE_OAM && rm_pItcRxRmMsg->timerId==RM_TWDOG)  
   //{   //Watchdog expired, entryId=trx
   //    rm_ProcTrxWDogExp(rm_ItcRxEntryId);
   //    return;
   //}
          
   //BTS not in service now
   //if ( !OAMrm_BTS_OPSTATE )
   // {
   //     IDEBUG__(("INFO@rm_Despatch:Bts is not in service right now\n"));
   //     return;
   //}

   //Go by msg primitive
   switch ( rm_ItcRxPrim )
   {
   case RRm_EST_REQ:case RRg_EST_REQ:
        if (!OAMrm_TRX_OPSTATE(0)) //Assume trx0 always the bcchTrx here
        {   //bcch trx not in service now, this should already filter out
            IDEBUG__(("WARNING@rm_Despatch: TRX 1 not operating now!\n"));
            return;
        }
	  rm_RrEstReq();
	  break;
   
   case DLm_RA_IND:
        if (!OAMrm_TRX_OPSTATE(0)) //Assume trx0 always the bcchTrx here
        {   
            //bcch trx not in service now, this should already filter out
            IDEBUG__(("WARNING@rm_Despatch: TRX 1 not operating now!\n"));
            return;
        }
	  rm_DlRaInd();
	  break;
  
   default:
        //Despatch message to a proper SM action
        if ( rm_ItcRxEntryId != CNI_IRT_NULL_ID )  
        {
             rm_pSmCtxt=&rm_SmCtxt[rm_ItcRxEntryId];

             //Check if trx is operating 
             if (!OAMrm_TRX_OPSTATE(rm_pSmCtxt->pPhyChan->trxNumber))
             {   
		     //trx not in service now
                 IDEBUG__(("WARNING@rm_Despatch: trx%d not in service\n", 
				    rm_pSmCtxt->pPhyChan->trxNumber));
                 return;
             }

             if ( (rm_pSmCtxt == RM_SMCTXT_NULL)    ||
                  (rm_pSmCtxt->smType != RM_MAIN_SM) )
             {
                 //RM doesnt handle any message assoc with SACCH any more
                 IDEBUG__(("WARNING@rm_Despatch: recv msg not for mSM (smType %d, pSmCtxt %p)\n",
                            rm_pSmCtxt->smType, rm_pSmCtxt));
                 return;
             }
                  
 	       //Act accordingly in terms of SM state
             if ( (MODULE_MD==rm_ItcRxOrigin) &&
                  (rm_pItcRxMdMsg->primitive_type==MDL_ERR_RPT))
             { 
		      //rm_MdlErrRptCauseRel();
                  rm_ProcRrRelease(CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER);
             }
             else if ( (MODULE_L1==rm_ItcRxOrigin) &&
                       (rm_pItcRxL1Msg->l3_data.buffer[0]==RM_L1MG_DEDMGMT) &&
                       (rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_RSL_MSB) &&
                       (rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_RSL_LSB) )
             {
			 //rm_RslRelease();
                   rm_ProcRrRelease(CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER);

                   //PMADDED:nbrOfLostRadioLinksTCH,nbrOfLostRadioLinksSDCCH
                   if (rm_pSmCtxt->pPhyChan->chanBusy)
                   {
                       if (rm_pSmCtxt->pPhyChan->chanType==RM_PHYCHAN_TCHF_S)
                           PM_CellMeasurement.nbrOfLostRadioLinksTCH.increment();
                       else if (rm_pSmCtxt->pPhyChan->chanType==RM_PHYCHAN_SDCCH4)
                           PM_CellMeasurement.nbrOfLostRadioLinksSDCCH.increment();
                       else
                           EDEBUG__(("ERROR@rm_despatch:chanType=%d\n",
                                      rm_pSmCtxt->pPhyChan->chanType));
                   }
             } else if ( (MODULE_MM==rm_ItcRxOrigin) &&
                         (rm_pItcRxMmMsg->primitive_type== RR_REL_REQ) )
             {
                       rm_ProcRrRelease(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);
             }
             else
      	       (*rm_SmAction[rm_pSmCtxt->state])();
        }
 	  else
	       IDEBUG__(("ERROR@rm_Despatch: null entry Id encountered (module %d, prim %d)\n",
                        (u8)(rm_ItcRxPrim>>8), (u8)(rm_ItcRxPrim)));

        break;

   } //switch()

} //rm_Despatch()

#endif //__RM_DESPATCH_CPP__

