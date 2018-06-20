// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CCMobRemHnd.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : CC Handlers for events triggered by the other half
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "JCC/JCCLog.h"

#include "logging/VCLOGGING.h"
#include "JCC/LUDBInstance.h"
#include "JCC/LUDBapi.h"

#include "CC/CCSessionHandler.h"
#include "CC/CCUtil.h"
#include "CC/CallConfig.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"

//CDR <xxu:08-21-00>
#include "CDR/CdrVoiceCall.h"

#include "rm/rm_head.h"
extern u8 rm_EntryIdToTrxSlot(T_CNI_IRT_ID entryId, u16 *gsmHandler);
extern bool callTrace;

// handle remote events
// !< For all msgs,local and remote, User-to-User support can be added later,
// connected subaddress is related to this.

// Remote Disconnect
JCCEvent_t 
CCHalfCall::handleMobRemoteDisconnect(void)
{

  DBG_FUNC("handleMobRemoteDisconnect", CC_HALFCALL_LAYER);
  DBG_ENTER();

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  // Cause is Mandatory. Use rlsCause to set it before building
  // !< For now there is a generic rlsCause in the Half Call Object.

  // To avoid sending the release to H323
  parent->disconnectSentToVoip = true;

  //CAUSE<xxu:03-21-00> BEGIN

  rlsCause.causeValue = 
           (T_CNI_RIL3_CAUSE_VALUE) remMsgData.basicCallInMsg.ReasonCode;

  // Set the release cause for the Mobile based on the Voip value.

  //switch (remMsgData.basicCallInMsg.ReasonCode) {
  //
  //case VOIP_MOBILE_REASON_DESTINATION_BUSY: 
  //  rlsCause.causeValue = CNI_RIL3_CAUSE_USER_BUSY;
  //  break;
  //
  //case VOIP_MOBILE_REASON_DESTINATION_REJECTED:
  //  rlsCause.causeValue = CNI_RIL3_CAUSE_CALL_REJECTED;
  //  break;
  //      
  //case VOIP_MOBILE_REASON_DESTINATION_UNREACHABLE:
  //  rlsCause.causeValue = CNI_RIL3_CAUSE_NO_USER_RESPONDING; // Or is it CNI_RIL3_CAUSE_NOT_ROUTE_TO_DESTINATION
  //  // Or CNI_RIL3_CAUSE_USER_ALERTING_NO_ANSWER
  //  break;
  //
  //case VOIP_MOBILE_REASON_REMOTE_HANGUP:
  //  rlsCause.causeValue = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
  //  break;
  //
  //case VOIP_MOBILE_REASON_VOIP_TASK_ERROR:
  //  rlsCause.causeValue = CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
  //  break;
  //
  //default:
  //  // Do Nothing
  //  // Use the pre-stored value (normal, unspecified)
  //  break;
  //
  //}

  // print out the trx, timeslot for better debugging
  UINT16 gsmHandler;
  UINT8 trx, slot;
  T_SUBSC_IE_ISDN* isdnPtr;
  
  if (parent->entryId() != (T_CNI_IRT_ID) JCC_UNDEFINED )
    if (rm_EntryIdToTrxSlot(parent->entryId(), 
                            &gsmHandler) ==
        RM_TRUE)
    {
      
      trx  = (UINT8)(gsmHandler>>8);
      slot = (UINT8)(gsmHandler);
      
      if (callTrace)
        {
          DBG_WARNING("CC Log: Remote(VOIP) Side Disconnected on (TRX=%d),(Slot=%d),(entryId=%d),(cause=%d)\n",
                      trx, slot, parent->entryId(), rlsCause.causeValue);
          if ((isdnPtr = ludbGetMSISDNPtr(parent->ludbIndex())) != NULL)
            {
              DBG_WARNING("CC Log: MS (DN=%s)\n",
                          isdnPtr->digits);
            }
        }
      else
        {
          DBG_TRACE("CC Log: Remote(VOIP) Side Disconnected on (TRX=%d),(Slot=%d),(entryId=%d),(cause=%d)\n",
                    trx, slot, parent->entryId(), rlsCause.causeValue);
          if ((isdnPtr = ludbGetMSISDNPtr(parent->ludbIndex())) != NULL)
            {
              DBG_TRACE("CC Log: MS (DN=%s)\n",
                        isdnPtr->digits);
            }
        }
    }

  
  // If the state is currently waiting for a connection response from a lower layer
  // then just do a mobile resource release
  if (callState_ == THC_ST_MM_CONN_REQ)
    {
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }

  // Release Connection resources, if there are any, rightaway
  // so that the RTP Port could be re-assigned from VOIP.
  if (parent->voiceConnected)
    {
      T_CSU_PORT_ID rtpSourcePort;
      T_CSU_RESULT_CON csuResult;
          
      rtpSourcePort.portType = CSU_RTP_PORT;
      rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(parent->h323CallHandle);
          
      if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort))
          != CSU_RESULT_CON_SUCCESS)
        {
                  
          JCCLog2("CSU Error : Disconn. Failure, (Result = %d) for (RTPHandle = %p)\n",  
                  csuResult, (int)rtpSourcePort.portId.rtpHandler);
          DBG_ERROR("CSU Error : Disconn. Failure, (Result = %d) for (RTPHandle = %p)\n",  
                    csuResult, rtpSourcePort.portId.rtpHandler);

          // Can only generate OA&M log. 
        }

      parent->voiceConnected = false;   

    }

  // Build the Disconnect Message towards the MS. 
  if ( (msOutResult = buildDisconnect (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Release Channel resources
      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }
  
  // start the disconnect timer 
  hcTimer->setTimer(CALL_DISCONNECT_T305);
  
  callState_ = HC_ST_DISCONNECT_IND;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);

}

// Remote Release Complete - Abnormal
// Can ignore the normal ones for now - Resources are
// released even before that msg. is received.

JCCEvent_t 
CCHalfCall::handleMobRemoteAbnormalRelCom(void)
{
  DBG_FUNC("handleMobRemoteAbnormalRelCom", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // cancel any timers
  hcTimer->cancelTimer();

  // To avoid sending the release to H323
  parent->disconnectSentToVoip = true;

  // If the state is currently waiting for a connection response from a lower layer
  // then just do a mobile resource release
  if (callState_ == THC_ST_MM_CONN_REQ)
    {
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }

  // One stage release
  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  // Build the Release Message towards the MS. 
  if ( (msOutResult = buildReleaseComplete (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Release RR resources
      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }
  
  
  // !< Transaction Id. has to be released, maybe in res_release? Yes

  callState_ = HC_ST_NULL;
  DBG_LEAVE();
  return (CC_RES_RELEASE);

  // return (CC_MESSAGE_PROCESSING_COMPLT);
}

// Remote Alerting
JCCEvent_t 
CCHalfCall::handleMobRemoteAlerting(void)
{

  DBG_FUNC("handleMobRemoteAlerting", CC_HALFCALL_LAYER);
  DBG_ENTER();

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  ULONG setupCompltTime;

  // Make sure that the channel is assigned. Always assume that 
  // you want to do in-band alerting. 
  // if ((channState_ == RES_ST_ASSIGNED) &&
  //    !alertingSent_)

  //if (!progressSent_)
  //{
      //Test code for progress IE
      if (h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.ie_present)
	  { 
		  DBG_TRACE("CC CALL LOG: Alerting progress IE info exist(%d) codingStd(%d) loc(%d) desc(%d)\n",
                     h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.ie_present,
                     h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.codingStandard,
                     h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.location,
                     h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.progressDesp);
  
          if ( (msOutResult = buildProgress(&msEncodedMsg))
               != CNI_RIL3_RESULT_SUCCESS )
          {
               //Log the error. Release RR resources
               callState_ = HC_ST_NULL;
               DBG_LEAVE();
               return (CC_RES_RELEASE);
          }
      }

      //} else
	  //{	  
      //   // Build the Alerting Message towards the MS. 
      //   if ( (msOutResult = buildAlerting (&msEncodedMsg))
      //         != CNI_RIL3_RESULT_SUCCESS )
      //     {
      //        // Log the error. Release RR resources
      //        callState_ = HC_ST_NULL;
      //        DBG_LEAVE();
      //        return (CC_RES_RELEASE);
	  //	 }
	  //}

      // Build the Alerting Message towards the MS. 
      if ( (msOutResult = buildAlerting (&msEncodedMsg))
               != CNI_RIL3_RESULT_SUCCESS )
      {
              // Log the error. Release RR resources
              callState_ = HC_ST_NULL;
              DBG_LEAVE();
              return (CC_RES_RELEASE);
      }

      // calculate the setup time
      setupCompltTime = tickGet();

      DBG_TRACE("Call Setup time in number of system ticks(%d)\n", 
                (setupCompltTime - setupTime_));

      PM_CCMeasurement.meanTimeToCallSetupService.addRecord(setupCompltTime - setupTime_);

      if (!isEmergencyCall)
        {
          PM_CCMeasurement.succMobileOriginatingCalls.increment();
        }
      else
        {
          PM_CCMeasurement.succMobileEmergencyCalls.increment();
        }

      alertingSent_ = true;

  //}

  callState_ = OHC_ST_ANSWER_PENDING;

  // With current implementation, VOIP could send another Codec Selected
  // just before connect. Always do Mode Modify(to traffic) just before connect. 
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

// Remote Answer
JCCEvent_t 
CCHalfCall::handleMobRemoteAnswer(void)
{

  DBG_FUNC("handleMobRemoteAnswer", CC_HALFCALL_LAYER);
  DBG_ENTER();

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;


  // If external handover has occurred
  if (parent->parent->handoverSession->hoState_ == HAND_ST_ANCH_EHO_COMPLT)
    {
       T_CSU_PORT_ID rfSourcePort, rtpSinkPort;
       T_CSU_RESULT_CON csuResult;

       rtpSinkPort.portType = CSU_RTP_PORT;
       rtpSinkPort.portId.rtpHandler = VoipCallGetRTPHandle(parent->h323CallHandle);

       rfSourcePort = parent->parent->mobileCsuPort();

       if ((csuResult = csu_DuplexConnect(&rfSourcePort, &rtpSinkPort))
           != CSU_RESULT_CON_SUCCESS)
         {
           JCCLog2("CSU Error : Duplex Conn. Failure, (Result = %d) for (rtp port = %x)\n",  
                    csuResult, 
                    (int)rtpSinkPort.portId.rtpHandler);
           DBG_ERROR("CSU Error : Duplex Conn. Failure, (Result = %d) for (rtp port = %x)\n",  
                     csuResult, 
                     (int)rtpSinkPort.portId.rtpHandler);

           rlsCause.causeValue = CNI_RIL3_CAUSE_TEMPORARY_FAILURE; //CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;

           //CAUSE<xxu:03-21-00> BEGIN

           voipReleaseCause_ = CNI_RIL3_CAUSE_TEMPORARY_FAILURE;  //CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;

           //CAUSE<xxu:03-21-00> END
            
           DBG_LEAVE();
           return (CC_TWO_STAGE_DISCONNECT);
         }

       parent->voiceConnected = true;

       chState_ = CH_ST_IDLE;
       channState_ = RES_ST_ASSIGNED;
    }

  // Make sure that the channel is assigned.
  if ( ( (channState_ == RES_ST_ASSIGNED) ) &&
         !connectSent_)
    {
      // Build the Connect Message towards the MS. 
      if ( (msOutResult = buildConnect (&msEncodedMsg))
           != CNI_RIL3_RESULT_SUCCESS )
        {
          //CAUSE<xxu:03-22-00> BEGIN

          voipReleaseCause_ = CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
          rlsCause.causeValue = CNI_RIL3_CAUSE_TEMPORARY_FAILURE;

          //CAUSE<xxu:03-22-00> END

          // Log the error. Release RR resources
          callState_ = HC_ST_NULL;
          DBG_LEAVE();
          return (CC_RES_RELEASE);
        }
      
      // start the connect timer 
      hcTimer->setTimer(CALL_CONNECT_T313);

      connectSent_ = true;
    }

  //    // Very early assignment - begin
  //    // Synchronize with the Channel state machine
  //    if (channState_ != RES_ST_ASSIGNED)
  //      {
  //        // Channel is not assigned. Cleanup.
  //        channState_ = RES_ST_IDLE;
  
  //        rlsCause.causeValue = CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
          
  //        DBG_LEAVE();
  //        return (CC_TWO_STAGE_DISCONNECT);
  //      }
  //    // Very early assignment - end
  
  callState_ = OHC_ST_CONNECT_INDICATION;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleMobRemoteCallProgress1(void)
{
  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  DBG_FUNC("handleMobRemoteCallProgress1", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // !< Do we need to wait for Call Confirmed before sending Call Proceeding 
  // to the originating side to get the negotiated BCs 
  // - No need, clarified with Ian

  // !! For Call Proceeding, we need the Bearer Cap1, Cap2, and Repeat Ind.
  // if needed by Setup, as specified by 07.01.

  DBG_TRACE("CC CALL LOG: Progress progress IE info exist(%d) codingStd(%d) loc(%d) desc(%d)\n",
                     h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.ie_present,
                     h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.codingStandard,
                     h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.location,
                     h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.progressDesp);
  
  if ( (msOutResult = buildProgress(&msEncodedMsg))
        != CNI_RIL3_RESULT_SUCCESS )
  {
        //Log the error. Release RR resources
        callState_ = HC_ST_NULL;
        DBG_LEAVE();
        return (CC_RES_RELEASE);
  }


  progressSent_ = true;

  if (callState_ != OHC_ST_ANSWER_PENDING)
      callState_ = OHC_ST_CALL_PROCEEDING;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleMobRemoteCallProgress(void)
{
  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  DBG_FUNC("handleMobRemoteCallProgress", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // !< Do we need to wait for Call Confirmed before sending Call Proceeding 
  // to the originating side to get the negotiated BCs 
  // - No need, clarified with Ian

  // !! For Call Proceeding, we need the Bearer Cap1, Cap2, and Repeat Ind.
  // if needed by Setup, as specified by 07.01.

  
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleMobRemoteTermAddress(void)
{
  JCCEvent_t retEvent;

  DBG_FUNC("handleMobRemoteTermAddress", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // copy from the remote message to the local var
  cpn = remMsgData.callOffering.CalledPartyNumber;
  callingPartyNum = remMsgData.callOffering.CallingPartyNumber;

  //CDR <xxu:08-24-00>
  if (callingPartyNum.ie_present)
      parent->ccCdr.setCallingPartyNumber(&callingPartyNum);
  //parent->ccCdr.setCalledPartyNumber(&cpn);

  DBG_TRACE("Call: calling party number received from Voip:\n");
  DBG_TRACE("(ie_present=%d),(numberType=%d),(plan=%d),(numDigits=%d) no(%x,%x,%x,%x)\n",
              callingPartyNum.ie_present,
              callingPartyNum.numberType,
              callingPartyNum.numberingPlan,
              callingPartyNum.numDigits,
              callingPartyNum.digits[0],
              callingPartyNum.digits[1],
              callingPartyNum.digits[2],
              callingPartyNum.digits[3]);
  //    callingPartyNum.ie_present = true;

  // !< Map the incoming Called Party Number to a service (like fax, data, voice)?

  // Authorize the termination attempt
  if ( ! (gLUDB.authTerm(parent->ludbIndex(), cpn)) )
    {
      DBG_TRACE("Termination Authorization failed, (LUDB Index = %d)\n", 
                (int)parent->ludbIndex());

      voipReleaseCause_ = CNI_RIL3_CAUSE_NOT_ROUTE_TO_DESTINATION;
      rlsCause.causeValue = CNI_RIL3_CAUSE_NOT_ROUTE_TO_DESTINATION;

      DBG_LEAVE();
      return(handleMobReleaseCompleteUtil());
    }

  // <Igal: 05-15-00> Allows MT Call setup during active SMS transfer.
  if (parent->parent->isMmConnected() == false)
    {
      IntraL3Msg_t mmOutMsg;
      
      // Fill the data for MM message
      mmOutMsg.l3_data.pageReq.imsi = ludbGetMobileId(parent->ludbIndex());
      mmOutMsg.l3_data.pageReq.qid = parent->msgQId;

      DBG_TRACE("page request msgQId: %p/n", parent->msgQId);
      
      // Send internal MM msg - imsi needs to be included for paging
      sendMM(INTRA_L3_MM_EST_REQ, INTRA_L3_MM_LOCATE_MS, parent->entryId(), &mmOutMsg);
      
      callState_ = THC_ST_MM_CONN_REQ;

      DBG_LEAVE();
      return   (CC_MESSAGE_PROCESSING_COMPLT);
    }
  else
    {
      // waiting call or MM connection is already established by the SMS. No need to locate him.
      retEvent = handleMMEstCnf();
      DBG_LEAVE();
      return   (retEvent);
    }
      
}

JCCEvent_t 
CCHalfCall::handleMobRemoteH323ChannReady(void)
{

  DBG_FUNC("handleMobRemoteH323ChannReady", CC_HALFCALL_LAYER);
  DBG_ENTER();

  //BCT <xxu:09-22-00> BEGIN
  if (parent->h245ChannState==RES_ST_ASSIGNED)
  {
      DBG_TRACE("CC BCT VConn: h245ChState(%d) hoHndl(%p) callSt(%d) chanSt(%d) bct(%d,%d)\n",
                 parent->h245ChannState, callState_, channState_,
                 parent->getBctInvoke(), parent->bctArrival_);

      //This is a second voice connected message
      if (callState_ != HC_ST_ACTIVE)
      {  
          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);
      }
  }
  //BCT <xxu:09-22-00> END

  // Synchronize with the main state machine
  parent->h245ChannState = RES_ST_ASSIGNED;

  // Connection Manager functionality
  if (channState_ == RES_ST_ASSIGNED)
    {
      // Both channels are setup. 
      DBG_LEAVE();
      return (CC_MAKE_CONNECTION_REQ);
    }
  else
    {
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
}

JCCEvent_t 
CCHalfCall::handleMobRemoteChannSelected(void)
{

  DBG_FUNC("handleMobRemoteChannSelected", CC_HALFCALL_LAYER);
  DBG_ENTER();

  currSpeechCh_.version = remMsgData.codecSelected.Codec;

  if (channState_ != RES_ST_IDLE)
    {
      // Ignore the second request.
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }

  if (!parent->waitingCallLeg)
    {
      DBG_LEAVE();
      return (CC_RESOURCE_ASSIGN_REQ);
    }
  else
    {
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
}


//BCT <xxu:08-11-00> BEGIN
JCCEvent_t 
CCHalfCall::handleBctNumberReqTimeout(void)
{
  DBG_FUNC("handleBctNumberReqTimeout", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE("CC->BCT(BctNumReqTimeout): h245ChState(%d) hoHndl(%p) callSt(%d) chanSt(%d) bct(%d,%d)\n",
             parent->h245ChannState, callState_, channState_,
             parent->getBctInvoke(), parent->bctArrival_);

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  parent->disconnectSentToVoip = true;

  rlsCause.causeValue = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
  // Build the Disconnect Message towards the MS. 
  if ( (msOutResult = buildDisconnect (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Release Channel resources
      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RELEASE_CALL);
    }
  
  // start the disconnect timer 
  hcTimer->setTimer(CALL_DISCONNECT_T305);
  
  callState_ = HC_ST_DISCONNECT_IND;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleGetBctNumberNack(void)
{
  DBG_FUNC("handleGetBctNumberNack", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE("CC->BCT(BctNumNack): h245ChState(%d) hoHndl(%p) callSt(%d) chanSt(%d) bct(%d,%d)\n",
             parent->h245ChannState, callState_, channState_,
             parent->getBctInvoke(), parent->bctArrival_);

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  parent->disconnectSentToVoip = true;

  rlsCause.causeValue = (T_CNI_RIL3_CAUSE_VALUE) 
                        remMsgData.basicCallInMsg.ReasonCode;

  // Build the Disconnect Message towards the MS. 
  if ( (msOutResult = buildDisconnect (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Release Channel resources
      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RELEASE_CALL);
    }
  
  // start the disconnect timer 
  hcTimer->setTimer(CALL_DISCONNECT_T305);
  
  callState_ = HC_ST_DISCONNECT_IND;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);

}
//BCT <xxu:08-11-00> END


//BCT <xxu:09-22-00> BEGIN
JCCEvent_t 
CCHalfCall::handleBctUpdateHandles(void)
{
  DBG_FUNC("handleBctUpdateHandles", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE("CC->BCT(UpdateHandles): chanSt(%d,%d) callSt(%d) h323CallHdnl(%x,%x) bct(%d,%d)\n",
             parent->h245ChannState, channState_, callState_,
             parent->oH323CallHandle, parent->h323CallHandle,
             parent->getBctInvoke(), parent->bctArrival_);

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleMobRemoteAlerting2nd(void)
{
   DBG_FUNC("handleMobRemoteAlerting2nd", CC_HALFCALL_LAYER);
   DBG_ENTER();

   DBG_TRACE("CC->BCT(Alerting): h245ChState(%d) hoHndl(%p) callSt(%d) chanSt(%d) bct(%d,%d)\n",
             parent->h245ChannState, callState_, channState_,
             parent->getBctInvoke(), parent->bctArrival_);

   //second alerting received from H323. This can only happen to BCT now
   if (parent->h245ChannState != RES_ST_ASSIGNED)
   {
        //This is an unexpected alterting, ignore
        DBG_LEAVE();
        return (CC_MESSAGE_PROCESSING_COMPLT);
   } 

   //Play annoucement in terms of handover condition
   CCHandover        *handoverSession;
   handoverSession = parent->getHandoverSession();

   if ( handoverSession != NULL )
   {
        if ( (handoverSession->hoState_ == HAND_ST_ANCH_HO_COMPLT) ||
             (handoverSession->hoState_ == HAND_ST_ANCH_EHO_COMPLT) )
            annState_ = ANN_PLAY_ON;
        else
            annState_ = ANN_PLAY_WAITING;
   } else
        annState_ = ANN_PLAY_ON;
   
   if (annState_ == ANN_PLAY_ON)
   {
       //Play inband annoucement       
       T_CSU_RESULT_CON csuResult;

       annSrcPort_.portType = CSU_ANN_PORT;
       annSrcPort_.portId.annHandler = 0;  //CSU_ANN_RINGBACK ID
       annSnkPort_ = parent->parent->mobileCsuPort();
       annSnkPort_.speechVersion = currSpeechCh_.version;

       DBG_TRACE("CC->CSU(bct): start inband alert ann(%d,%x) --> snk(%d,%x,%d)\n",
                  annSrcPort_.portType,
                 (int)annSrcPort_.portId.rtpHandler,
                  annSnkPort_.portType,
                 (int)annSnkPort_.portId.rtpHandler,
                 (int)annSnkPort_.speechVersion);

       if ( (csuResult = csu_SimplexConnect(&annSrcPort_, &annSnkPort_))
           != CSU_RESULT_CON_SUCCESS)
       {
            DBG_WARNING("CC->CSU(bct): inband alert s-conn failed xlst(%d) ann(%d,%x) snk(%d,%x,%d)\n",
                         csuResult,
                         annSrcPort_.portType,
                         (int)annSrcPort_.portId.rtpHandler,
                          annSnkPort_.portType,
                         (int)annSnkPort_.portId.rtpHandler,
                         (int)annSnkPort_.speechVersion);

            //ignore this remote alerting

       }

   }

   DBG_LEAVE();
   return (CC_MESSAGE_PROCESSING_COMPLT);
} 

JCCEvent_t 
CCHalfCall::handleMobRemoteChannSelected2nd(void)
{
  JCCEvent_t ret_val;

  DBG_FUNC("handleMobRemoteChannSelected2nd", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE("CC->BCT(ChanSelect): h245ChState(%d) hoHndl(%p) callSt(%d) chanSt(%d) bct(%d,%d)\n",
             parent->h245ChannState, callState_, channState_,
             parent->getBctInvoke(), parent->bctArrival_);

  currSpeechCh_.version = remMsgData.codecSelected.Codec;

  if ( (channState_ != RES_ST_ASSIGNED) || (callState_ != HC_ST_ACTIVE) )
  {
      // Ignore the unexpected 2nd request
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
  }

  //Cancel current annoucement if there is one
  if (annState_ == ANN_PLAY_ON)
  {
      //Stop inband annoucement       
      T_CSU_RESULT_CON csuResult;

      DBG_TRACE("CC->CSU(bct): stop inband alert  ann(%d,%x) --> snk(%d,%x,%d)\n",
                 annSrcPort_.portType,
                 (int)annSrcPort_.portId.rtpHandler,
                 annSnkPort_.portType,
                 (int)annSnkPort_.portId.rtpHandler,
                 (int)annSnkPort_.speechVersion);

      if ( (csuResult = csu_SimplexBreak(&annSrcPort_, &annSnkPort_))
           != CSU_RESULT_CON_SUCCESS)
      {
           DBG_WARNING("CC->CSU(bct): inband alert s-break failed xlst(%d) ann(%d,%x) snk(%d,%x,%d)\n",
                        csuResult,
                        annSrcPort_.portType,
                        (int)annSrcPort_.portId.rtpHandler,
                         annSnkPort_.portType,
                        (int)annSnkPort_.portId.rtpHandler,
                        (int)annSnkPort_.speechVersion);

      }

  }

  annState_ = ANN_PLAY_OFF;

  // if external handover is in effect, don't bother re-assigning
  // channels (RR establishment), since it is still UP
  if ( ( parent->parent->handoverSession != NULL ) && 
       ( parent->parent->handoverSession->hoState_ == HAND_ST_ANCH_EHO_COMPLT ) )
   {
      channState_ = RES_ST_ASSIGNED;
      ret_val = CC_MESSAGE_PROCESSING_COMPLT;
   }
  else
   {
      channState_ = RES_ST_ASS_REQ;
      ret_val = CC_RESOURCE_ASSIGN_REQ;
   }

  DBG_LEAVE();
  return (ret_val);
}

JCCEvent_t 
CCHalfCall::handleMobRemoteAnswer2nd(void)
{

  DBG_FUNC("handleMobRemoteAnswer2nd", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE("CC->BCT(Answer): h245ChState(%d) hoHndl(%p) callSt(%d) chanSt(%d) bct(%d,%d)\n",
             parent->h245ChannState, callState_, channState_,
             parent->getBctInvoke(), parent->bctArrival_);

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}
//BCT <xxu:08-22-00> END
