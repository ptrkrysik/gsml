/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_S11TRAFFICCHANSUSPENDING_CPP__
#define __RM_S11TRAFFICCHANSUSPENDING_CPP__

//HOADDED
#include "rm\rm_head.h"

void rm_S11TrafficChanSuspending(void)
{
   rm_EdRet_t  rrEdRet;	     //Return code from RR decode

   RDEBUG__(("ENTER-rm_S11TrafficChanSuspending\n"));

   switch(rm_ItcRxOrigin)
   {
   case MODULE_MM:
        //SMS-SAPI3-Begin
          
  	  //Got a message from MM:
        //If it is RR_REL_REQ. In terms of entryId, it could be linked to either
        //main signaling or associated signaling.

        if (rm_SmCtxt[rm_ItcRxEntryId].smType != RM_MAIN_SM)
        {
	      //RR_REL_REQ linked to acch SM. Main SM not affected
	      IDEBUG__(("WARNING@rm_S11TrafficChanSuspending: MM msgType %d for aEntryId %d\n",
	                 rm_pItcRxMmMsg->message_type,
                       rm_ItcRxEntryId));
            return;
        }

        if ( rm_pItcRxMmMsg->primitive_type == RR_REL_REQ )
	  {
             // Release caused by HO SUCCESS
             if (rm_pItcRxMmMsg->message_type == INTRA_L3_RR_HANDOVER_COMPLETE)
             {
                 //HOADDED
                 //HO SUCCESS. Do local release of old connection and change both SM's
                 //and phyChan's state
                 PDEBUG__(("rm@S11TrafficChanSuspending: HO SUCCESS\n"));
                 rm_ProcRrRelease(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);
   	           //if (rm_SmCtxt[rm_ItcRxEntryId].smType != RM_MAIN_SM)
                 //    EDEBUG__(("ERROR@rm_S11TrafficChanSuspending:HO going on SACCH\n"));
                 //rm_RslRelease();
                 return;
             }
     
	       // Release not caused by HO SUCCESS. Check if entryId is linked to
             // main or acch SM
	       if (rm_SmCtxt[rm_ItcRxEntryId].smType != RM_MAIN_SM)
             {
	          //RR_REL_REQ linked to acch SM. Main SM not affected
 	          IDEBUG__(("WARNING@rm_S11TrafficChanSuspending:RrRelReq on acch SM '%d'\n",
                           rm_ItcRxEntryId));

//  	          if (rm_pSmCtxt->sapi != RM_SAPI0)
//CT_CHN            rm_SendDlRelReq(rm_pSmCtxt->aOId, rm_pSmCtxt->sapi,
//			                  RM_CTSACCH,RM_LOCAL_RELEASE);
//                  rm_SendDlRelReq(rm_pSmCtxt->aOId, rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);
//
//CT_CHN        rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI0,RM_CTSACCH,RM_NORMAL);
//              rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI3, RM_LOCAL_RELEASE);
//              rm_SendRrRelInd(RM_MDLERR, rm_pSmCtxt->aEntryId);
	       }
	       else
	       { 
	   	    //EntryId linked to main SM. Release everything
                //SMSADDED
  		    //rm_SendRrRelInd(RM_MDLERR, rm_pSmCtxt->aEntryId); 
	          rm_SendChanRelease(rm_ItcRxEntryId);
        
                //Deactivate sacch to start RF channel release
	          rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
	          rm_StartTimer(rm_ItcRxEntryId, RM_T3109);
	    
                rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER;
                                       //CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED;

	          //Adjust SM state to S7
	          rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	
	          rm_pSmCtxt->state = RM_S7RELEASING_SIGTRAFFICCHAN;
		    /*PR1247 begin*/
		    //PM: maxNbrOfBusyTCHs, meanNbrOfBusyTCHs
		
                if (--rm_NoOfBusyTchf >= 0)
                    PM_CellMeasurement.meanNbrOfBusyTCHs.addRecord(rm_NoOfBusyTchf);
                else
                    rm_NoOfBusyTchf = 0;

                int time = tickGet() - rm_pSmCtxt->pPhyChan->startTime;
                PM_CellMeasurement.meanTCHBusyTime.addRecord(time);

                rm_pSmCtxt->pPhyChan->startTime=tickGet();

                if (--rm_maxNbrOfBusyTCHs >= 0)
                    PM_CellMeasurement.maxNbrOfBusyTCHs.setValue(rm_maxNbrOfBusyTCHs); //decrement();
                    //PM_CellMeasurement.maxNbrOfBusyTCHs.addRecord(rm_maxNbrOfBusyTCHs); //decrement();
                else
                    rm_maxNbrOfBusyTCHs = 0;

                rm_pSmCtxt->pPhyChan->chanBusy = RM_FALSE;
		    /*PR1247 end*/
   	       } 
	  } else if (rm_pItcRxMmMsg->message_type == INTRA_L3_RR_HANDOVER_FAILURE)
        {   
            //HO FAILURE. Stay with current connection
            //Currently neither MM nor CC will pass this FAILURE to RM module.    
            EDEBUG__(("ERROR-rm_S11TrafficChanSuspending: got HO FAILURE from MM\n"));
            rm_pSmCtxt->state = RM_S10TRAFFICMODE_ESTABLISHED;
           
	  } else
	       //Unexpected message from MM
	       UDEBUG__(("UNKNOWN-rm_S11TrafficChanSuspending:MM msgType:%d\n",
	                  rm_pItcRxMmMsg->message_type));
	  break;

   case MODULE_MD:
        //Check if received HO FAILURE from MS. And pass this possible HO FAILURE to
        //CC directly.
	  if ( rm_pItcRxMdMsg->primitive_type == DL_REL_IND )
        { 
             if (rm_pSmCtxt->smType==RM_MAIN_SM)
             {
                 rm_ProcRrRelease(CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED);
                 return;

                 //SMS-SAPI3-Begin
	           //rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
                 //rm_SendDlRelReq(lapdmOId, RM_SAPI0, RM_LOCAL_RELEASE);

                 //The following is for safety-guard purpose, thus may be redundancy
                 //rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI3, RM_LOCAL_RELEASE);
                 //rm_SendDlRelReq(rm_pSmCtxt->aOId, RM_SAPI3, RM_LOCAL_RELEASE);
        
	           //rm_StartTimer(rm_ItcRxEntryId, RM_T3109);

                 //rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER;
                                        //CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
        	
	           // Adjust SM state to S7
	           //rm_pSmCtxt->state           = RM_S7RELEASING_SIGTRAFFICCHAN;
	           //rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	

             }

        } else if (rm_pItcRxMdMsg->primitive_type == DL_DATA_IND)
        {
	      //Decode the received l3_data to see if it's expected HO FAILURE message
            //PR1104
            //rrEdRet = CNI_RIL3RRM_Decode( &(rm_pItcRxMdMsg->l3_data),&rm_UmMsg );
            rrEdRet = rm_MsgAnalyze(); 
 
            if ( rrEdRet != RM_RRED_SUCCESS )
            {
	           EDEBUG__(("ERROR-rm_S11TrafficChanSuspending:Failed decode L2/l3 msg\n"));
                 return;
            }

            //Decoding successful. Further check if it's expected HO FAILURE message
            if ( rm_UmMsg.header.protocol_descriminator != RM_PD ||
                 rm_UmMsg.header.message_type != CNI_RIL3RRM_MSGID_HANDOVER_FAILURE )   
            {
                 EDEBUG__(("ERROR-rm_S11TrafficChanSuspending:unexpected msg:PD=%d;MT=%x",
		           rm_UmMsg.header.protocol_descriminator, 
		           rm_UmMsg.header.message_type ));
                 rm_SendRrStatus(CNI_RIL3_RR_MESSAGE_TYPE_NOT_COMPATIBLE);
                 return;
            }

            //PR: gathering handover failure causes rather than normal event
            if ( (!rm_UmMsg.handoverFailure.rrCause.ie_present) || 
                 (rm_UmMsg.handoverFailure.rrCause.causeValue != CNI_RIL3_RR_NORMAL_EVENT) )
            {
                 IDEBUG__(("WARNING@rm_S11TrafficChanSuspending: ho failed on (ie_present %d, cause %d)\n",
                            rm_UmMsg.handoverFailure.rrCause.ie_present,
                            rm_UmMsg.handoverFailure.rrCause.causeValue));               
            }

	      //Got HO FAILURE message from mobile
	      //No timer monitoring if RM needs go back to S10 while it is staying @S11
            //thus no action seen here to stop any running timer.
            //Send HO FAILURE message to CC and continue with the current RF Channel.
	      rm_SendHoFailure(0x0A,-1);  //Radio Interface failure, revert to old channel
	      rm_pSmCtxt->state = RM_S10TRAFFICMODE_ESTABLISHED;

        } else
        {
            EDEBUG__(("ERROR-rm_S11TrafficChanSuspending: unexpected L2/primitive:%d\n",
		           rm_pItcRxMdMsg->primitive_type));

        }
	
	  break;

   default:
        //Processing of RSL(L1) and MDL_ERR_RPT(L2) have been extracted out of
        //here and centralized in one place. Thus, they should not appear here.
        UDEBUG__(("rm_S11TrafficChanSuspending: unexpected msg origination:%d\n",
		       rm_ItcRxOrigin));
	  break;

   } //End of switch()
}

#endif //_RM_S11TRAFFICCHANSUSPENDING_CPP__
