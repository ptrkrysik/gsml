
/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_RSLRELEASE_CPP__
#define __RM_RSLRELEASE_CPP__

#include "rm\rm_head.h"

void rm_RslRelease(void)
{
  rm_LapdmOId_t       		lapdmOId;

  RDEBUG__(("ENTER-rm_RslRelease\n"));

  if (rm_pSmCtxt->smType==RM_MAIN_SM)
  {
      //SMS-SAPI3-Begin

      //lapdmOId = rm_pSmCtxt->mOId;
      rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
      //rm_SendRrRelInd(RM_MDLERR,rm_ItcRxEntryId);
      //rm_SendDlRelReq(lapdmOId, RM_SAPI0,RM_LOCAL_RELEASE);

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
                             //CNI_RIL3_CAUSE_TEMPORARY_FAILURE;   //41

      //Adjust SM state to S7
      rm_pSmCtxt->state           = RM_S7RELEASING_SIGTRAFFICCHAN;
      rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;	
   } 
   else 
   { 
      //SMS-SAPI3-Begin
 
      //This case should not happen
      IDEBUG__(("WARNING@rm_RslRelease: RSL for aEntryId %d\n",
                 rm_ItcRxEntryId));

      //lapdmOId = rm_pSmCtxt->aOId;
 
      //rm_SendRrRelInd(RM_MDLERR,rm_ItcRxEntryId);
      //rm_SendDlRelReq(lapdmOId, RM_SAPI0,RM_LOCAL_RELEASE);
      //SMS-SAPI3_End

      rm_pSmCtxt->state           = RM_S1RM_RUNNING;
   }  

} 


//PR1104
T_CNI_RIL3_RESULT rm_MsgAnalyze(void)
{
   T_CNI_RIL3_RESULT rrEdRet;

   PDEBUG__(("ENTER@rm_MsgAnalyze()\n"));

   rrEdRet = CNI_RIL3RRM_Decode(&(rm_pItcRxMdMsg->l3_data),&rm_UmMsg);

   switch(rrEdRet)
   {
   case	CNI_RIL3_NON_RRM_MESSAGE:
        //ignore this message. Maybe an MD-mux problem
        IDEBUG__(("WARNING@rm_MsgAnalyze: got a non-RM msg(%d,%d) on entryId(%d) at st(%d)\n",
                   rm_UmMsg.header.protocol_descriminator,
                   rm_UmMsg.header.message_type,
                   rm_ItcRxEntryId,
		   rm_pSmCtxt->state));
        break;
                   
   case CNI_RIL3_UNKNOWN_MESSAGE_TYPE:
        //Send a RR STATUS message to MS
        rm_SendRrStatus(CNI_RIL3_RR_MESSAGE_TYPE_NON_EXISTENT);
        break;        

   case CNI_RIL3_MANDATORY_IE_MISSING:
   case CNI_RIL3_NON_SEMANTICAL_MANDATORY_IE_ERROR:
        //Releasing the call if not in releasing yet
        rm_ProcRrRelease(CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER);
        break;

   case	CNI_RIL3_NON_IMPERATIVE_MESSAGE_PART_ERROR:
   case CNI_RIL3_RESULT_SUCCESS:
        if ( rm_UmMsg.header.message_type == CNI_RIL3RRM_MSGID_RR_STATUS) 
        {
            IDEBUG__(("WARNING@rm_MsgAnalyze: got RR STATUS cause(pres %d, val %d) on entryId(%d) at st(%d)\n",
                       rm_UmMsg.rrStatus.rrCause.ie_present,
                       rm_UmMsg.rrStatus.rrCause.causeValue,
                       rm_ItcRxEntryId,
		       rm_pSmCtxt->state));
            return (CNI_RIL3_NON_IMPERATIVE_MESSAGE_PART_ERROR); //reuse to skip processing in calling part
        }
            
        return (CNI_RIL3_RESULT_SUCCESS);
        break;

   default:
        break;
       
   }

   return (rrEdRet);

}

void rm_SendRrStatus(T_CNI_RIL3_RR_CAUSE_VALUE cause)
{
   rm_EdRet_t                  rrEdRet;	            
   rm_L3Data_t                 l3_data; 	           
   T_CNI_RIL3RRM_MSG_RR_STATUS *pUmRrStatus;

   //Monitoring entrance to a function
   RDEBUG__(("ENTER@rm_SendRrStatus\n"));


   //Select Rr Status message from the union
   pUmRrStatus = (T_CNI_RIL3RRM_MSG_RR_STATUS *) &(rm_UmMsg.rrStatus);

   // Populate message elements to encode
   RM_MEMSET( &rm_UmMsg, sizeof(rm_UmMsg_t) );

   //Header--PD,MT,SI
   pUmRrStatus->header.protocol_descriminator = RM_PD;
   pUmRrStatus->header.si_ti		      = RM_SI;
   pUmRrStatus->header.message_type	      = CNI_RIL3RRM_MSGID_RR_STATUS;

   //IE--Channel Description
   pUmRrStatus->rrCause.ie_present = RM_TRUE; 
   pUmRrStatus->rrCause.causeValue = cause;

   //Call RR message encoding functionality
   rrEdRet = CNI_RIL3RRM_Encode( &rm_UmMsg, &l3_data);

   //Check the encoding result before sending the message
   switch (rrEdRet)
   {
   case RM_RRED_SUCCESS:
	//Wrap the encoded Paging message in DL_DATA_REQ
        PostL3SendMsLog(rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,&l3_data );
        rm_SendDlDataReq(rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,&l3_data );

	break;

   default:
	//Um message encoding failed
        EDEBUG__(("ERROR@rm_SendRrStatus: Um encode error:%d\n", rrEdRet ));
   	break;

   } //End of switch(rrEdRet)

} //rm_SendRrStatus()

void rm_ProcRrRelease(T_CNI_RIL3_CAUSE_VALUE cause)
{
   int time;

   //Monitoring entrance to a function
   RDEBUG__(("ENTER@rm_StartRrRelease()\n"));
       
   //Release the RF channel as well as associated SMs
   PDEBUG__(("INFO@rm_ProcRrRelease: (module %d, entryId %d, mSt %d, prim %d, msg %d)\n",
                   rm_ItcRxOrigin,
                   rm_ItcRxEntryId,
                   rm_pSmCtxt->state,
                   rm_ItcRxPrimType,
                   rm_ItcRxMsgType));

   switch(rm_pSmCtxt->state)
   {
   case RM_S2ACTIVATING_SIGTRAFFIC_CHAN:
        IDEBUG__(("INFO@rm_ProcRrRelease: (module %d, entryId %d, mSt %d, prim %d, msg %d)\n",
                   rm_ItcRxOrigin,
                   rm_ItcRxEntryId,
                   rm_pSmCtxt->state,
                   rm_ItcRxPrimType,
                   rm_ItcRxMsgType));

        //Got a negative Ack to Channel Activation
        rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
        rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);

        //Check if Channel Activation is caused by Ho or Chan request
        if (rm_pSmCtxt->isForHo==RM_TRUE)
        {
            //If it's Handover Request caused, send HoReqAck back to MM
            rm_SendHoReqNAck(rm_pSmCtxt->pPhyChan->hoRef);
        } 

        //SMS-SAPI3-Begin
        //For benefit of LAPDm's internal cleanup 
        rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI0,RM_LOCAL_RELEASE);
        rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI3,RM_LOCAL_RELEASE);
        rm_SendDlRelReq(rm_pSmCtxt->aOId,RM_SAPI3,RM_LOCAL_RELEASE);

        //SMS-SAPI3-End
        //Reset relevant RM channel resources
        rm_pSmCtxt->state = RM_S1RM_RUNNING;
        rm_SmCtxt[rm_pSmCtxt->aEntryId].state = RM_S1RM_RUNNING;
        rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_FREE;
                  
        break;

   case RM_S7RELEASING_SIGTRAFFICCHAN:
        //Move to S9 and clean S7 watch timers
        rm_StopTimer(rm_ItcRxEntryId, RM_T3109);
        rm_StopTimer(rm_ItcRxEntryId, RM_T3L02);

        rm_SendRrRelInd(RM_MDLERR, rm_pSmCtxt->mEntryId);

   case RM_S3SIGCHAN_ACTIVATED:
        //Release the RF channel as well as associated SMs
        IDEBUG__(("INFO@rm_ProcRrRelease: (module %d, entryId %d, mSt %d, prim %d, msg %d)\n",
                   rm_ItcRxOrigin,
                   rm_ItcRxEntryId,
                   rm_pSmCtxt->state,
                   rm_ItcRxPrimType,
                   rm_ItcRxMsgType));

        rm_StopTimer( rm_ItcRxEntryId, RM_T3101 );
        rm_StartTimer( rm_ItcRxEntryId, RM_T3111 );
	  rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
        rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);

        rm_pSmCtxt->relCause = cause;

        rm_pSmCtxt->pPhyChan->state           = RM_PHYCHAN_RELEASING;
	  rm_pSmCtxt->pPhyChan->T3101.state     = RM_FALSE;
	  rm_pSmCtxt->state                     = RM_S9DEACTIVATING_SIGTRAFFICCHAN;
        rm_SmCtxt[rm_pSmCtxt->aEntryId].state = RM_S1RM_RUNNING;    
        break;

   case RM_S4TRAFFICCHAN_ACTIVATED:
        IDEBUG__(("INFO@rm_ProcRrRelease: (module %d, entryId %d, mSt %d, prim %d, msg %d)\n",
                   rm_ItcRxOrigin,
                   rm_ItcRxEntryId,
                   rm_pSmCtxt->state,
                   rm_ItcRxPrimType,
                   rm_ItcRxMsgType));

        if (rm_pSmCtxt->waitHoComplt == RM_TRUE)
            //ext-HO <xxu:06-25-01>
            rm_SendHoFailure(0x01,-1);  //Radio Interface Failure

        rm_StopTimer( rm_ItcRxEntryId, RM_T3105 );
        rm_StartTimer( rm_ItcRxEntryId, RM_T3111 );
   	  rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
        rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);

        rm_pSmCtxt->relCause = cause;

        rm_pSmCtxt->pPhyChan->state           = RM_PHYCHAN_RELEASING;
	rm_pSmCtxt->pPhyChan->T3105.state     = RM_FALSE;
	rm_pSmCtxt->state                     = RM_S9DEACTIVATING_SIGTRAFFICCHAN;
        rm_SmCtxt[rm_pSmCtxt->aEntryId].state = RM_S1RM_RUNNING;    
        break;

   case RM_S5SIGMODE_ESTABLISHED:
	//Release RF resource
	rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
	rm_SendChanRelease(rm_ItcRxEntryId);

	rm_StartTimer(rm_ItcRxEntryId, RM_T3109);
	    
        rm_pSmCtxt->relCause = cause;

	//Adjust SM state to S7
        rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;
	rm_pSmCtxt->state = RM_S7RELEASING_SIGTRAFFICCHAN;

        //PM Added: 
        rm_pSmCtxt->pPhyChan->chanBusy = RM_FALSE;

        if (rm_pSmCtxt->pPhyChan->chanType==RM_PHYCHAN_SDCCH4)
        { 
            if (--rm_NoOfBusySdcch4 >= 0)
                PM_CellMeasurement.meanNbrOfBusySDCCHs.addRecord(rm_NoOfBusySdcch4);
            else
                rm_NoOfBusySdcch4 = 0;

            if (--rm_maxNbrOfBusySDCCHs >= 0)
                PM_CellMeasurement.maxNbrOfBusySDCCHs.setValue(rm_maxNbrOfBusySDCCHs); // decrement();
                //PM_CellMeasurement.maxNbrOfBusySDCCHs.addRecord(rm_maxNbrOfBusySDCCHs); // decrement();
            else
                rm_maxNbrOfBusySDCCHs = 0;
        }
       
        break;

   case RM_S6ACTIVATING_TRAFFICMODE:
   case RM_S8TRAFFICMODE_ACTIVATED:
        IDEBUG__(("INFO@rm_ProcRrRelease: (module %d, entryId %d, mSt %d, prim %d, msg %d)\n",
                   rm_ItcRxOrigin,
                   rm_ItcRxEntryId,
                   rm_pSmCtxt->state,
                   rm_ItcRxPrimType,
                   rm_ItcRxMsgType));

	rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
	rm_SendChanRelease(rm_ItcRxEntryId);

	rm_StartTimer(rm_ItcRxEntryId, RM_T3109);
	    
        rm_pSmCtxt->relCause = cause;

	//Adjust SM state to S7
        rm_pSmCtxt->pPhyChan->state = RM_PHYCHAN_RELEASING;
	rm_pSmCtxt->state = RM_S7RELEASING_SIGTRAFFICCHAN;
        break;

   case RM_S10TRAFFICMODE_ESTABLISHED:
   case RM_S11TRAFFICCHAN_SUSPENDING:

        rm_SendChanRelease(rm_ItcRxEntryId);
        rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);

        rm_StartTimer(rm_ItcRxEntryId, RM_T3109);

        rm_pSmCtxt->relCause = cause;
	   
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
        else
            rm_maxNbrOfBusyTCHs = 0;

        rm_pSmCtxt->pPhyChan->chanBusy = RM_FALSE;

        break;
 
   case RM_S9DEACTIVATING_SIGTRAFFICCHAN:
        IDEBUG__(("INFO@rm_ProcRrRelease: (module %d, entryId %d, mSt %d, prim %d, msg %d)\n",
                   rm_ItcRxOrigin,
                   rm_ItcRxEntryId,
                   rm_pSmCtxt->state,
                   rm_ItcRxPrimType,
                   rm_ItcRxMsgType));

        rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
        rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);
        break;
  
   default:
        rm_SendDeactivateSacch(rm_pSmCtxt->pPhyChan);
        rm_SendRfChanRelease(rm_pSmCtxt->pPhyChan);
 
        IDEBUG__(("INFO@rm_ProcRrRelease: (module %d, entryId %d, mSt %d, prim %d, msg %d)\n",
                   rm_ItcRxOrigin,
                   rm_ItcRxEntryId,
                   rm_pSmCtxt->state,
                   rm_ItcRxPrimType,
                   rm_ItcRxMsgType));
        break;

   }

} //rm_ProcRrRelease()

#endif //__RM_RSLRELEASE_CPP__
