/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_S10TRAFFICMODEESTABLISHED_CPP__
#define __RM_S10TRAFFICMODEESTABLISHED_CPP__

#include "rm\rm_head.h"

void rm_S10TrafficModeEstablished(void)
{
   u8	ret;
   int  time;

   //Monitoring entrance to a function
   RDEBUG__(("ENTER@rm_S10TrafficModeEstablished\n"));

   //In this design, RM does not watch any MEAS RPTS. Thus not deciding RSL here

   switch(rm_ItcRxOrigin)
   {
   case MODULE_L1:
        //HOADDED
	  if ( rm_pItcRxL1Msg->l3_data.buffer[0]==RM_L1MG_DEDMGMT              &&  
             rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_ASYNCHO_REQUIRED_MSB &&
             rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_ASYNCHO_REQUIRED_LSB  )
        {
             //Received Async Handover Required Indication from DSP & pass to CC
             rm_SendAsyncHoRequired();
        } else 
        {
             rm_ProcL1ModeAckNAck();
        }
	  break;

   case MODULE_MD:
        //MDLRrrRpt processing centralized somewhere else 
        //Handle DL_DATA_IND w CHN MODE MODIFY ACK inside

	if ( rm_pItcRxMdMsg->primitive_type == DL_REL_IND )
        { 

             if (rm_pSmCtxt->smType==RM_MAIN_SM)
             {
                 rm_ProcRrRelease(CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED);
                 return;
                 //SMS-SAPI3-Begin
   	           //lapdmOId = rm_pSmCtxt->mOId;
	           // rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
                 //rm_SendRrRelInd(RM_MDLERR,rm_ItcRxEntryId);

                 //CT_CHN rm_SendDlRelReq(lapdmOId, RM_SAPI0,
                 //                       rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
                 //rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI0, RM_LOCAL_RELEASE);
        
  	           //if (rm_pSmCtxt->sapi != RM_SAPI0)
                 //    //CT_CHN rm_SendDlRelReq(lapdmOId, rm_pSmCtxt->sapi,
                 //    //			          rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
                 //    rm_SendDlRelReq(lapdmOId, rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);
        
                 //The following is for safety-guard purpose, thus may be redundancy
                 //rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI3, RM_LOCAL_RELEASE);
                 //rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI3, RM_LOCAL_RELEASE);

                 //rm_SmCtxt_t *pASmCtxt;
                 //pASmCtxt = rm_pSmCtxt->pPhyChan->pASmCtxt;
                 //pASmCtxt->state = RM_S1RM_RUNNING;

                 //SMS-SAPI3-End

	           //rm_StartTimer(rm_ItcRxEntryId, RM_T3109);

                 //rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED;
                 //                       //CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER;
                 //                       //CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
        	
	           // Adjust SM state to S7
	           //rm_pSmCtxt->state           = RM_S7RELEASING_SIGTRAFFICCHAN;
	           //rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	

                 //SMS-SAPI3-Begin
             }
             //} else 
             //{ 
	  	     //lapdmOId = rm_pSmCtxt->aOId;
               
                 //SMS-SAPI3-Begin

                 //rm_SendRrRelInd(RM_MDLERR,rm_ItcRxEntryId);
                 //CT_CHN rm_SendDlRelReq(lapdmOId, RM_SAPI0,
                 //				rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
                 //rm_SendDlRelReq(lapdmOId, RM_SAPI3, RM_LOCAL_RELEASE);

                 //SMS-SAPI3-End

	           //rm_pSmCtxt->state           = RM_S1RM_RUNNING;
             //}  
        
        //SMS-SAPI3-End

        } else if (rm_pItcRxMdMsg->primitive_type == DL_DATA_IND)
        {
            rm_ProcMsModeModifyAckNack();

        } else
        {
            EDEBUG__(("ERROR@rm_S10TrafficModeEstablished: unexp L2/prim:%d\n",
		           rm_pItcRxMdMsg->primitive_type));
 	  }
        break;
 
   case MODULE_MM:
	  //Handle RR_REL_REQ on both main & acch signaling 
        if ( rm_pItcRxMmMsg->primitive_type == RR_REL_REQ )
   	  {
            //SMS-SAPI3-Begin
	      //Check if entryId is linked to main or acch SM */
	      if (rm_SmCtxt[rm_ItcRxEntryId].smType != RM_MAIN_SM)
            {
	         //RR_REL_REQ linked to acch SM. Main SM not affected
	         IDEBUG__(("WARNING@rm_S10TrafficModeEstablished: MM msgType %d for aEntryId %d\n",
		              rm_pItcRxMmMsg->message_type,
                          rm_ItcRxEntryId));

            //    rm_SendRrRelInd(RM_MDLERR, rm_pSmCtxt->aEntryId);
            //    //CT_CHN rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI0,RM_CTSACCH,RM_NORMAL);
            //    rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI0,RM_NORMAL);

            //    if (rm_pSmCtxt->sapi != RM_SAPI0)
            //        //CT_CHN rm_SendDlRelReq(rm_pSmCtxt->aOId, rm_pSmCtxt->sapi,
            //        //			         RM_CTSACCH,RM_LOCAL_RELEASE);
            //        rm_SendDlRelReq(rm_pSmCtxt->aOId, rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);
	      } else
	      { 
	          //EntryId linked to main SM. Release everything
                //May need to handle a little bit more here later
	          //rm_SendRrRelInd(RM_MDLERR, rm_pSmCtxt->aEntryId); 

	          rm_SendChanRelease(rm_ItcRxEntryId);
                rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
	          rm_StartTimer(rm_ItcRxEntryId, RM_T3109);

                rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
	   
	          //Adjust SM state to S7
	          rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	
	          rm_pSmCtxt->state = RM_S7RELEASING_SIGTRAFFICCHAN;
                
                //PM: maxNbrOfBusyTCHs, meanNbrOfBusyTCHs

                if (--rm_NoOfBusyTchf >= 0)
                    PM_CellMeasurement.meanNbrOfBusyTCHs.addRecord(rm_NoOfBusyTchf);
                else
                    rm_NoOfBusyTchf = 0;

                time = tickGet() - rm_pSmCtxt->pPhyChan->startTime;
                PM_CellMeasurement.meanTCHBusyTime.addRecord(time);

                rm_pSmCtxt->pPhyChan->startTime=tickGet();

                if (--rm_maxNbrOfBusyTCHs >= 0)
                    PM_CellMeasurement.maxNbrOfBusyTCHs.setValue(rm_maxNbrOfBusyTCHs); //decrement();
                    //PM_CellMeasurement.maxNbrOfBusyTCHs.addRecord(rm_maxNbrOfBusyTCHs); //decrement();
                else
                    rm_maxNbrOfBusyTCHs = 0;

                rm_pSmCtxt->pPhyChan->chanBusy = RM_FALSE;
            } 

	  } else
 	      //Unexpected message from MM
	      IDEBUG__(("WARNING@rm_S10TrafficModeEstablished:MM msgType:%d\n",
		           rm_pItcRxMmMsg->message_type));
  	  break;
   
   case MODULE_CC:
	  //Process a message received from CC
        switch (rm_pItcRxCcMsg->message_type)
        {
        case INTRA_L3_RR_CHANN_ASSIGN_CMD:
             rm_ProcChanAssignCmd();
             break;
        case INTRA_L3_RR_HANDOVER_COMMAND:
             //Send HO Command to MS and advance SM state
       	 rm_SendHandoverCommand();     
             rm_pSmCtxt->state = RM_S11TRAFFICCHAN_SUSPENDING;
             break;
        //case INTRA_L3_CM_CONN_CMD:
        default:
             IDEBUG__(("WARNING@rm_S10TrafficModeEstablished: invalid CC msgType=%d\n",
                        rm_pItcRxCcMsg->message_type));
             break;
        }
        break;

   case MODULE_RM:
        //Handle T3L01 exp here!
        rm_ProcT3L01Expiry();
        break;

   default:
	  //Unexpected message orgination
        IDEBUG__(("WARNING@rm_S10TrafficModeEstablished:wrong msg origination:%d\n",
		       rm_ItcRxOrigin));
	  break;

   } //End of switch()

} //End of rm_S10TrafficModeEstablished()

#endif //__RM_S10TRAFFICMODEESTABLISHED_CPP__

