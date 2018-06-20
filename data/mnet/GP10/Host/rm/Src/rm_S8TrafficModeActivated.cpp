/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/
#ifndef __RM_S8TRAFFICMODEACTIVATED_CPP__
#define __RM_S8TRAFFICMODEACTIVATED_CPP__

#include "rm\rm_head.h"

void rm_S8TrafficModeActivated(void)
{

   //Monitoring entrance to a function 
   RDEBUG__(("ENTER@rm_S8TrafficModeActivated\n"));
 
   if (rm_pSmCtxt->smType != RM_MAIN_SM)
   {
       //Mode Modify only occurs on main DCCH assumed
       EDEBUG__(("ERROR@rm_S8TrafficModeActivated: not main SM\n"));
       return;
   }
 
   switch(rm_ItcRxOrigin)
   {
   case MODULE_MD:
	  //Handle DL_DATA_IND w CHN MODE MODIFY ACK inside
        if (rm_pItcRxMdMsg->primitive_type != DL_DATA_IND)
            EDEBUG__(("ERROR@rm_S8TrafficModeActivated: unexp. L2/prim:%d\n",
		           rm_pItcRxMdMsg->primitive_type));
	  else
        {
            rm_ProcMsModeModifyAckNack();
        }
        break;

   case MODULE_RM:
        //Handle T3L01 exp here!
        rm_pSmCtxt->relCause = 
                    CNI_RIL3_CAUSE_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE;
        rm_ProcT3L01Expiry();
        break;

   case MODULE_MM:
  	  if ( rm_pItcRxMmMsg->primitive_type == RR_REL_REQ )
	  {
	       //Release RF resource in terms of MM command
	       rm_SendChanRelease(rm_ItcRxEntryId);
	       rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
	       rm_StartTimer(rm_ItcRxEntryId, RM_T3109);
	    
	       //Adjust SM state to S7
             rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	
	       rm_pSmCtxt->state = RM_S7RELEASING_SIGTRAFFICCHAN;
   	   } else
	       //Unexpected message from MM
	       IDEBUG__(("WARNING@rm_S8TrafficModeActivated:MM msgType:%d\n",
		            rm_pItcRxMmMsg->message_type));
	   break;
   default:
	   //Unexpected message origin at this SM state
	   IDEBUG__(("WARNING@rm_S8TrafficModeActivated:unexpected message origin:%d\n",
		        rm_ItcRxOrigin));
	   break;

   } //End switch()

} //End of rm_S8TrafficModeActivated()


void rm_ProcMsModeModifyAckNack(void)
{
    int time;

    rm_EdRet_t       rrEdRet;

    PDEBUG__(("INFO@rm_ProcMsModeModifyAckNack:callInfo(%d,%d)\n",
              rm_ItcRxEntryId, rm_pSmCtxt->callIdx));

    //Decode this L3 message received to see if it is CHN MODE MODIFY ACK
    //PR1104 RR STATUS MESSAGE
    //rrEdRet = CNI_RIL3RRM_Decode( &(rm_pItcRxMdMsg->l3_data),&rm_UmMsg );
    rrEdRet = rm_MsgAnalyze();     

    if ( rrEdRet != RM_RRED_SUCCESS )
    {
	   EDEBUG__(("ERROR@rm_S8TrafficModeActivated: L2/l3_data decoded KO\n"));
         return;
    }

    //PDEBUG__(("INFO@rm_ProcMsModeModifyAckNack: ret %d ........................\n", rrEdRet ));

    //PDEBUG__(("....Header: md %x, msgType %x\n",
    //           rm_UmMsg.header.protocol_descriminator,
    //           rm_UmMsg.header.message_type));

    //PDEBUG__(("....ChanDescription: (ie_pre %d, hop %d, chnTyp %d, tn %d, sub %d, tsc %x, freq %d\n",
    //          rm_UmMsg.channelModeModifyAck.channelDescription.ie_present,
    //          rm_UmMsg.channelModeModifyAck.channelDescription.hopping,
    //          rm_UmMsg.channelModeModifyAck.channelDescription.channelType,
    //          rm_UmMsg.channelModeModifyAck.channelDescription.subChannelNumber,
    //          rm_UmMsg.channelModeModifyAck.channelDescription.TN,
    //          rm_UmMsg.channelModeModifyAck.channelDescription.TSC,
    //          rm_UmMsg.channelModeModifyAck.channelDescription.arfcn));

    //PDEBUG__(("....ChanMode: (pre %d, value %d\n",
    //          rm_UmMsg.channelModeModifyAck.channelMode.ie_present,
    //          rm_UmMsg.channelModeModifyAck.channelMode.value));

    //Decoding successful. Further check if 
    if ( rm_UmMsg.header.protocol_descriminator != RM_PD              ||
         rm_UmMsg.header.message_type != RM_CHANNEL_MODE_MODIFY_ACK   )   
    {
         EDEBUG__(("ERROR-rm_S8TrafficModeActivated:unexpected msg PD:%d;MT:%d",
	              rm_UmMsg.header.protocol_descriminator, 
	              rm_UmMsg.header.message_type ));
         rm_SendRrStatus(CNI_RIL3_RR_MESSAGE_TYPE_NOT_COMPATIBLE);
         return;
     }

     //Got expected msg i.e. Channel Mode Modify Acknowlegement, move to S10

     rm_StopTimer(rm_ItcRxEntryId, RM_T3L01);

    if (rm_pSmCtxt->codAlgo == rm_UmMsg.channelModeModifyAck.channelMode.value)
    {
         rm_SendChanAssignComplete();
         if (rm_pSmCtxt->state != RM_S10TRAFFICMODE_ESTABLISHED)
         {
             //PM: succTCHSeizures, maxNbrOfBusyTCHs, meanNbrOfBusyTCHs

             PM_CellMeasurement.succTCHSeizures.increment();
             rm_maxNbrOfBusyTCHs++;
             PM_CellMeasurement.maxNbrOfBusyTCHs.setValue(rm_maxNbrOfBusyTCHs); //increment();
             //PM_CellMeasurement.maxNbrOfBusyTCHs.addRecord(rm_maxNbrOfBusyTCHs); //increment();

             rm_NoOfBusyTchf++;
             PM_CellMeasurement.meanNbrOfBusyTCHs.addRecord(rm_NoOfBusyTchf);

             rm_pSmCtxt->pPhyChan->startTime=tickGet();

             rm_pSmCtxt->state = RM_S10TRAFFICMODE_ESTABLISHED;
         }
     } 
     else
     {
         PDEBUG__(("INFO@rm_S8TrafficModeActivated: unsupport mode(cur %d, req %d), entryId '%d'\n",
                    rm_UmMsg.channelModeModifyAck.channelMode.value,
                    rm_pSmCtxt->codAlgo,
                    rm_ItcRxEntryId));

         //SMS-SAPI3-Begin

	   //Get a negative ack from MS, release the call !
	 
	   rm_SendChanRelease(rm_ItcRxEntryId);
	   rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
	   rm_StartTimer(rm_ItcRxEntryId, RM_T3109);
	    
         rm_pSmCtxt->relCause = 
                     CNI_RIL3_CAUSE_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE;

	   //Adjust SM state to S7
         rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;
	   rm_pSmCtxt->state = RM_S7RELEASING_SIGTRAFFICCHAN;

         if (rm_pSmCtxt->state == RM_S10TRAFFICMODE_ESTABLISHED)
         {
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
         //rm_RslRelease();

         //SMS-SAPI3-End

     }
}


void rm_ProcT3L01Expiry(void)
{
    int time;

    PDEBUG__(("INFO@rm_ProcT3L01Expiry: smInfo(%d,%d),callIdx(%d,%d)\n",
               rm_pSmCtxt->smType, rm_pSmCtxt->state, rm_ItcRxEntryId,
               rm_pSmCtxt->callIdx));

    if ( rm_pItcRxRmMsg->primitive_type    == RM_TIMER_IND ||
         rm_pItcRxRmMsg->timerId           != RM_T3L01     ||
         rm_pSmCtxt->pPhyChan->T3L01.state == RM_FALSE     )
    {
         EDEBUG__(("ERROR@rm_S8ReleasingSigTrafficChan: prim:%d,timer:%d, sta:%d\n",
	              rm_pItcRxRmMsg->primitive_type,
                    rm_pItcRxRmMsg->timerId,
                    rm_pSmCtxt->pPhyChan->T3L01.state ));
         return;
    }

    //SMS-SAPI3-Begin

    // No rsp to Channel Mode Modify from Mobile
    rm_SendChanRelease(rm_ItcRxEntryId);
    rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
    //CT_CHN rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI0, 
    //                       rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
    //rm_SendDlRelReq(rm_pSmCtxt->mOId, RM_SAPI0, RM_LOCAL_RELEASE);

    //if (rm_pSmCtxt->sapi != RM_SAPI0)
       //CT_CHN rm_SendDlRelReq(rm_pSmCtxt->mOId, rm_pSmCtxt->sapi, 
       //			         rm_pSmCtxt->chanType,RM_LOCAL_RELEASE);
    //    rm_SendDlRelReq(rm_pSmCtxt->mOId, rm_pSmCtxt->sapi, RM_LOCAL_RELEASE);

    // Send RR_REL_IND to MM
    //rm_SendRrRelInd( RM_RSLIND, rm_ItcRxEntryId );

    //SMS-SAPI3-End
    
    rm_StartTimer(rm_ItcRxEntryId, RM_T3109);

    rm_pSmCtxt->relCause = CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER; //no rsp from MS

    //Adjust SM state to S7
    rm_pSmCtxt->state           = RM_S7RELEASING_SIGTRAFFICCHAN;
    rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	

    if (rm_pSmCtxt->state == RM_S10TRAFFICMODE_ESTABLISHED)
    {
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
}  
	
#endif // __RM_S8TRAFFICMODEACTIVATED_CPP__

