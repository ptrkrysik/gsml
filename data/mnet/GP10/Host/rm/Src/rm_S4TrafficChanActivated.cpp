/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __RM_S4TRAFFICCHANACTIVATED_CPP__
#define __RM_S4TRAFFICCHANACTIVATED_CPP__

//HOADDED
#include "rm\rm_head.h"

void rm_S4TrafficChanActivated(void)
{
   rm_EdRet_t       rrEdRet;	//Return code from RR decode
   rm_L3Data_t      l3_data; 	//Store encoded to Physical Information message

   // Monitoring entrance to a function
   RDEBUG__(("ENTER-rm_S4TrafficChanActivated\n"));
 
   if (rm_pSmCtxt->smType != RM_MAIN_SM ||
       rm_pSmCtxt->isForHo == RM_FALSE   )
   {
       EDEBUG__(("ERROR-rm_S4TrafficChanActivated: SMtype=%d,isForHo=%d\n",
                  rm_pSmCtxt->smType, rm_pSmCtxt->isForHo));
       return;
   }

   switch(rm_ItcRxOrigin)
   {
   case MODULE_MD:
        //Check if got valid Handover Access from Mobile
        if ( rm_ItcRxPrim == DLm_OTHRA_IND  )
        {
		//Check if this RA is as expected by RM
            if (rm_pItcRxMdMsg->sapi != RM_SAPI0 ||
                rm_pItcRxMdMsg->l3_data.buffer[0] != rm_pSmCtxt->pPhyChan->hoRef)
            {
                EDEBUG__(("ERROR-rm_S4TrafficChanActivated: RmRef=%d,MdRef=%d\n",
                           rm_pSmCtxt->pPhyChan->hoRef,
                           rm_pItcRxMdMsg->l3_data.buffer[0]));
                return;
            }

 	      rm_StopTimer(rm_ItcRxEntryId, RM_T3101);

            //Got an expected RA from Mobile on allocated channel. Save relevant
            //data for future use
            rm_pSmCtxt->sapi = rm_pItcRxMdMsg->sapi;
            rm_pSmCtxt->pPhyChan->hoTa = (u8)rm_pItcRxMdMsg->l3_data.buffer[3];	
 

            //Got Handover Access from Mobile, send out Physical Information
            //Encode Physical Information message
            l3_data.buffer[0] = (u8)RM_PD;
            l3_data.buffer[1] = (u8)CNI_RIL3RRM_MSGID_PHYSICAL_INFORMATION;
            l3_data.buffer[2] = rm_pSmCtxt->pPhyChan->hoTa;
            l3_data.msgLength = 3;

            //Send out l3_data via DL_UNIT_DATA_REQ
            //Send four Physical Information together for  TEST purpose
            //CNI_LAPDM_Dl_Unit_Data_Request( rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,
            //                                L2T_NORMAL_HEADER, &l3_data);
            //CNI_LAPDM_Dl_Unit_Data_Request( rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,
            //                                L2T_NORMAL_HEADER, &l3_data);
            //CNI_LAPDM_Dl_Unit_Data_Request( rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,
            //                                L2T_NORMAL_HEADER, &l3_data);
            CNI_LAPDM_Dl_Unit_Data_Request( rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,
                                            L2T_NORMAL_HEADER, &l3_data);
            //Reset ny1 and start timer to monitor arrival of right decoded msg
            //on the handed-over channel
            rm_pSmCtxt->pPhyChan->ny1 = 0;
 	      rm_StartTimer(rm_ItcRxEntryId, RM_T3105);
     	  } else 
        {
            //Check if received Handover Complete message from Mobile
            switch(rm_pItcRxMdMsg->primitive_type)
            {
		case DL_EST_IND:
                 rm_pSmCtxt->pPhyChan->ny1 = OAMrm_NY1;
  	           rm_StopTimer(rm_ItcRxEntryId, RM_T3105);
  	           rm_StartTimer(rm_ItcRxEntryId, RM_T3105);
                 return;
            case L23PT_DL_DATA_IND:
                 //Decode received message carried up in DL_EST_IND
                 //PR1104
                 //rrEdRet = CNI_RIL3RRM_Decode( &(rm_pItcRxMdMsg->l3_data),&rm_UmMsg );
                 rrEdRet = rm_MsgAnalyze(); 

                 if ( rrEdRet != RM_RRED_SUCCESS )
                 {
	                EDEBUG__(("ERROR-rm_S4TrafficChanActivated: l3_data decoded KO\n"));
                      return;
                 }

                 //Successfully decode L3 data carried in DL_EST_IND 
                 if ( rm_UmMsg.header.protocol_descriminator!=RM_PD  ||
                      rm_UmMsg.header.message_type!=CNI_RIL3RRM_MSGID_HANDOVER_COMPLETE )   
                 {
                      EDEBUG__(("ERROR-rm_S4TrafficChanActivated:unexped msg PD:%d;MT:%d\n",
		                     rm_UmMsg.header.protocol_descriminator, 
		                     rm_UmMsg.header.message_type ));
                      //PR1104
                      rm_SendRrStatus(CNI_RIL3_RR_MESSAGE_TYPE_NOT_COMPATIBLE);
                      return;
                 }

	           //Got Handover Complete message from Mobile, i.e. Mobile has successfully
                 //moved to new frequency/cell
                 RDEBUG__(("HANDOVER SUCCESS!!!-rm_S4TrafficChanActivated\n"));
  	           rm_StopTimer(rm_ItcRxEntryId, RM_T3105);
 	           rm_SendHoComplete();
	           rm_pSmCtxt->state = RM_S10TRAFFICMODE_ESTABLISHED;

                 //PM: succTCHSeizures, maxNbrOfBusyTCHs, meanNbrOfBusyTCHs

                 PM_CellMeasurement.succTCHSeizures.increment();
                 rm_maxNbrOfBusyTCHs++;
                 PM_CellMeasurement.maxNbrOfBusyTCHs.setValue(rm_maxNbrOfBusyTCHs);

                 //PM_CellMeasurement.maxNbrOfBusyTCHs.addRecord(rm_maxNbrOfBusyTCHs);
                 rm_NoOfBusyTchf++;
                 PM_CellMeasurement.meanNbrOfBusyTCHs.addRecord(rm_NoOfBusyTchf);

                 rm_pSmCtxt->pPhyChan->startTime=tickGet();

		     rm_pSmCtxt->pPhyChan->chanBusy = RM_TRUE;

                 break;

            default:
                 PDEBUG__(("TRACE@rm_S4TrafficChanActivated:unexpected L2/primitive:%d\n",
		                rm_pItcRxMdMsg->primitive_type));
                 return;
            } 
   	  } 
	  break;

   case MODULE_RM: 
        PDEBUG__(("rm_S4TrafficChanActivated: Timer Expiry for entryID=%d\n", 
                   rm_ItcRxEntryId));

        if ( rm_pItcRxRmMsg->primitive_type != RM_TIMER_IND  ||
             rm_pSmCtxt->pPhyChan->T3105.state == RM_FALSE   ||
             rm_pItcRxRmMsg->timerId != RM_T3105             ||
             rm_pItcRxRmMsg->timerId != RM_T3101              )
        {
   	       EDEBUG__(("ERROR-rm_S4TrafficChanActivated: prim:%d,timer:%d, sta:%d\n",
		            rm_pItcRxRmMsg->primitive_type,
                        rm_pItcRxRmMsg->timerId,
                        rm_pSmCtxt->pPhyChan->T3105.state ));
             return;
        }


        // Check if need to send more Physical Information or fail the HO effort
        if ( (rm_pSmCtxt->pPhyChan->ny1 < OAMrm_NY1) &&
             (rm_pItcRxRmMsg->timerId != RM_T3101  )  )
         {
            //T3105 expired but not got Handover Complete message, resend Physical
            //Information message
            l3_data.buffer[0] = (u8)RM_PD;
            l3_data.buffer[1] = (u8)CNI_RIL3RRM_MSGID_PHYSICAL_INFORMATION;
            l3_data.buffer[2] = rm_pSmCtxt->pPhyChan->hoTa;
            l3_data.msgLength = 3;

            //Send out l3_data via DL_UNIT_DATA_REQ
            CNI_LAPDM_Dl_Unit_Data_Request( rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,
                                            L2T_NORMAL_HEADER, &l3_data);
            //Increment ny1 and restart timer T3105
            rm_pSmCtxt->pPhyChan->ny1++;
 	      rm_StartTimer(rm_ItcRxEntryId, RM_T3105);
        } else
        {
            //Already finished retry of all ny1 times in sending Physical Info.
            //Send Handover Failure message to CC and release RM resources
            rm_pSmCtxt->waitHoComplt = RM_FALSE;

            //ext-HO <xxu:06-25-01>
            rm_SendHoFailure(0x01,-1);  //Radio Interface Failure

            //rm_RslRelease();
 	      rm_StopTimer(rm_ItcRxEntryId, RM_T3105);
            rm_ProcRrRelease(CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER);
        }

        break; 
	
   default:
	  //Unexpected message origin at this SM state
	  EDEBUG__(("ERROR-rm_S4TrafficChanActivated:unexpected message origin:%d\n",
		       rm_ItcRxOrigin));
	  break;

   } //End of switch()

}

#endif //__RM_S4TRAFFICCHANACTIVATED_CPP__

