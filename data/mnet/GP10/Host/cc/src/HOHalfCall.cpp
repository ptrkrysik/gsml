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
// File        : HOHalfCall.cpp
// Author(s)   : Xiaode and Joe
// Create Date : 05-15-2001
// Description : HOHalfCall functionality for External Handovers
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************


#include "CC/CCSessionHandler.h"
#include "CC/CallConfig.h"
#include "CC/HOHalfCall.h"

#include "voip/voipapi.h"
#include "CC/CCH323Util.h"

#include "CC/CCUtil.h"


HOHalfCallMsgHandler_t ehoCallStateHandler[MAX_EHO_CALL_STATES][MAX_EHO_CALL_EXTERNAL_EVENTS];

/*******************************************************************/

HOHalfCall::HOHalfCall ()
{
   
}
   

void
HOHalfCall::cleanup(void)
{

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::cleanup): entering......\n}\n");

  // Release Connection resources, if there are any
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
          DBG_ERRORho("CSU Error : Disconn. Failure, (Result = %d) for (RTPHandle = %p)\n",  
                      csuResult, rtpSourcePort.portId.rtpHandler);

          // Can only generate OA&M log. 
        }

      parent->voiceConnected = false;   

    }

  // Always do sendRemote as the LAST step of the handler.
  sendRemote(HC_REMOTE_RELEASE_COMPLETE);

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::cleanup): leaving.\n}\n");
}


JCCEvent_t 
HOHalfCall::handleNullEvent(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleNullEvent): entering......\n}\n");

  // Do Nothing.
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleNullEvent): leaving.\n}\n");
  return (CC_MESSAGE_PROCESSING_COMPLT);
}


void
HOHalfCall::initData(void)
{

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::initData): entering......\n}\n");

  // cancel any timers
  hcTimer->cancelTimer();

  releaseCounter = 0;
  callState_ = HC_ST_FREE;

  //CH<xxu:11-10-99>
  chState_ = CH_ST_IDLE;

  channState_ = RES_ST_IDLE;

  currSpeechCh_.version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;

  currEvent = JCC_NULL_EVENT;

  // Have a standard release cause to use
  memset(&rlsCause, 0, sizeof(T_CNI_RIL3_IE_CAUSE)); // first zap this ie
  rlsCause.ie_present = true; 
  rlsCause.codingStandard = CNI_RIL3_GSMPLMN_CODING;
  rlsCause.location = CNI_RIL3_LOCATION_PRIVATE_NETWORK_LOCAL_USER;
  rlsCause.causeValue = CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED;
  rlsCause.diagnosticsLength = 0;

  voipReleaseCause_ = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;

  // initialize party numbers
  cpn.ie_present = false;
  callingPartyNum.ie_present = false;

  // Have a simple Bearer Capability to use - for speech only
  // Allocate a full rate channel based on this.
  memset(&bearerCap, 0, sizeof(T_CNI_RIL3_IE_BEARER_CAPABILITY)); // first zap this ie
  bearerCap.ie_present = true;

  // octet 3 after TL - only one byte needed
  bearerCap.codingStandard = CNI_RIL3_GSM_STANDARD_CODING;
  bearerCap.transferMode = CNI_RIL3_TRANSFER_CIRCUIT_MODE;
  bearerCap.infoTransferCap = CNI_RIL3_INFO_TRANSFER_SPEECH;
  // Following is used only in Mob-to-Nwk direction. It should be set to 
  // default value in the Encoding routine for the Nwk-to-Mob direction - Syang
  bearerCap.radioChannelReq = CNI_RIL3_FULL_RATE_SUPPORT_ONLY_MS; // 01

  callingPartyNum.ie_present = false;

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::initData): leaving.\n}\n");

}

void
HOHalfCall::printData(JCCPrintStFn fnPtr)
{
  char * hcStr1Ptr = "Half Call Data: \n";
  char hcStr2[120];

  char callTaskName[22];

  sprintf(&callTaskName[0],
          "Call-%d EntryId-%d: \0",
          parent->callIndex,
          parent->entryId()); 

  //JOE: Add internal variables
  sprintf(&hcStr2[0],
          "curEv(%d),callSt(%d),channSt(%d),chState(%d),rlsCause(%d)\n",
          currEvent,
          callState_,
          channState_,
          chState_,
          rlsCause.causeValue
          );

  (*fnPtr)(callTaskName);
  (*fnPtr)(hcStr1Ptr);
  (*fnPtr)(&hcStr2[0]);

}


int
HOHalfCall::handleTimeoutMsg(IntraL3Msg_t    *ccInMsg)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleTimeoutMsg): entering......\n}\n");

  currEvent = HC_EHO_LOCAL_PROTOCOL_TIMEOUT;

  rlsCause.causeValue = CNI_RIL3_CAUSE_RECOVERY_ON_TIMER_EXPIRY;

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleTimeoutMsg): leaving.\n}\n");

  return (smHandleFirstCallEvent());
}

int
HOHalfCall::handleRemoteEvent(IntraL3Msg_t* h323Msg, JCCEvent_t remoteEvent,
                              CCRemMsgData_t  inRemMsg)
{ 
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleRemoteEvent): entering......\n}\n");

  // copy message data
  h323InMsg = h323Msg;
  remMsgData = inRemMsg;

  currEvent = remoteEvent;
  
  // Just call the SM Handle Event
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleRemoteEvent): leaving.\n}\n");
  return (smHandleFirstCallEvent());
}

int
HOHalfCall::smHandleEvent(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::smHandleEvent): entering......\n}\n");

  if (currEvent == CC_RELEASE_CALL)
    {
      int storedCause;
    
      parent->handleReleaseCall();

      // causeValue below Works because 0 is unused.  
      storedCause = (0 - rlsCause.causeValue);

      initData();
      DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::smHandleEvent): leaving.\n}\n");
      return(storedCause);
    }
      
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::smHandleEvent): leaving.\n}\n");
  return(0);
}

int
HOHalfCall::smHandleFirstCallEvent(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::smHandleFirstCallEvent): entering......\n}\n");

  // For Call States:
  if (currEvent != JCC_NULL_EVENT)
    {
      currEvent = 
        (this->*ehoCallStateHandler[callState_][currEvent - CC_INTERNAL_EHO_CALL_EVENT_BASE])();
    }

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::smHandleFirstCallEvent): leaving.\n}\n");
  return(smHandleEvent());
}

JCCEvent_t
HOHalfCall::handleInvalidEvent(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleInvalidEvent): entering......\n}\n");

  DBG_WARNINGho("{\nMNEThoTRACE(HOHalfCall::handleInvalidEvent):CC Call Warning: Invalid (event = %d)  in (State = %d) \n}\n",  
                currEvent, 
                callState_);
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleInvalidEvent): leaving.\n}\n");
  return (JCC_NULL_EVENT);
  
}

bool
HOHalfCall::sendRemote(JCCEvent_t remoteEvent)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::sendRemote): entering......\n}\n");

  // Copy the disconnect reason, if it is a release message.
  if (remoteEvent == HC_REMOTE_RELEASE_COMPLETE)
    {
      remMsgData.callRelease.cause = voipReleaseCause_;
    }

  // send to H323 task
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::sendRemote): leaving.\n}\n");
  return (sendH323MsgExtHo(parent, remoteEvent, remMsgData, parent->callLegNum));
}


JCCEvent_t 
HOHalfCall::handleEHOMobTermAddress(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobTermAddress): entering......\n}\n");

  T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  hoNumber;
  SpeechChann_t  speechchann;
  int i;

  hoNumber = parent->parent->handoverSession->hoNumber_;

  DBG_TRACEho("   EHO Call Log: hoNumber (ie_present=%d),(numberType=%d),(plan=%d),(numDigits=%d)\n",
                  hoNumber.ie_present,
                  hoNumber.numberType,
                  hoNumber.numberingPlan,
                  hoNumber.numDigits
                       );
  DBG_TRACEho("                 handover number digits (%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x)\n}\n",
               hoNumber.digits[0],hoNumber.digits[1],hoNumber.digits[2],
               hoNumber.digits[3],hoNumber.digits[4],
               hoNumber.digits[5],hoNumber.digits[6],
               hoNumber.digits[7],hoNumber.digits[8],hoNumber.digits[9],hoNumber.digits[10]);

  // setup remote data struct
  remMsgData.setup.CalledPartyNumber = hoNumber;
  // no calling party number needed
  remMsgData.setup.CallingPartyNumber.ie_present = false;

  speechchann = parent->parent->currSpeechCh();

  remMsgData.setup.bearerCap.speechVersionInd[0].fieldPresent = true;
  remMsgData.setup.bearerCap.speechVersionInd[0].version = speechchann.version; 

  for (i=1; i < 3; i++)
    {
        remMsgData.setup.bearerCap.speechVersionInd[i].fieldPresent = false;
    }


  DBG_TRACEho("   SpeechVersionInd: 1st(%d,%d) 2nd(%d,%d) 3rd(%d,%d)\n",
                  remMsgData.setup.bearerCap.speechVersionInd[0].fieldPresent,
                  remMsgData.setup.bearerCap.speechVersionInd[0].version,
                  remMsgData.setup.bearerCap.speechVersionInd[1].fieldPresent,
                  remMsgData.setup.bearerCap.speechVersionInd[1].version,
                  remMsgData.setup.bearerCap.speechVersionInd[2].fieldPresent,
                  remMsgData.setup.bearerCap.speechVersionInd[2].version);

  remMsgData.setup.IsEmergencyCall = false;

  callState_ = EHO_ST_OFFERING_CALL;


  DBG_TRACEho("   Sending SETUP to remote side.\n");

  // Now send setup to the other half call.
  if (sendRemote(THC_REMOTE_TERM_ADDRESS) == false)
    // failed commn. with the remote side - treat as a release complete
    // from the remote side and cleanup - skip event loop.
    {
      DBG_ERRORho("   ERROR in Sending SETUP.\n");
      DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobTermAddress): leaving.\n}\n");
      return (CC_RELEASE_CALL);
    }

  //reuse this timer for watch dog purpose in case no response from VOIP 
  hcTimer->setTimer(CALL_SETUP_T303);


  // set this boolean to true until we actually send a setup request to VOIP.
  parent->disconnectSentToVoip = false;

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobTermAddress): leaving.\n}\n");
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
HOHalfCall::handleEHOMobCallProgress(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobCallProgress): entering......\n}\n");

  // as long as handover is allowed, active leg is mandatory here.
  if ((parent->parent->activeLeg()) == NULL)
  {
    DBG_TRACEho("   Call no longer active.\n");
    DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobCallProgress): leaving.\n}\n");
    return (CC_RELEASE_CALL);
  }

  DBG_TRACEho("   Received CALL PROGRESS.  (h323InMsg->call_handle=%d)\n",
                  h323InMsg->call_handle);

  // Save h323 call handle
  parent->h323CallHandle = h323InMsg->call_handle;

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobCallProgress): leaving.\n}\n");
  return (CC_MESSAGE_PROCESSING_COMPLT);

}

JCCEvent_t 
HOHalfCall::handleEHOMobRemoteChannSelected(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobRemoteChannSelected): entering......\n}\n");

  // as long as handover is allowed, active leg is mandatory here.
  if ((parent->parent->activeLeg()) == NULL)
  {
    DBG_TRACEho("   Call no longer active.\n");
    DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobRemoteChannSelected): leaving.\n}\n");
    return (CC_RELEASE_CALL);
  }

  currSpeechCh_.version = remMsgData.codecSelected.Codec;

  DBG_TRACEho("   Received Channel Selected: speech version:(%d)\n",
                  currSpeechCh_.version);

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobRemoteChannSelected): leaving.\n}\n");
  return (CC_MESSAGE_PROCESSING_COMPLT);

}

JCCEvent_t 
HOHalfCall::handleEHOMobRemoteAnswer(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobRemoteAnswer): entering......\n}\n");

  //Stop guarding timer
  hcTimer->cancelTimer();

  // as long as handover is allowed, active leg is mandatory here.
  if ((parent->parent->activeLeg()) == NULL)
  {
    DBG_TRACEho("   Call no longer active.\n}\n");
    DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobRemoteAnswer): leaving.\n}\n");
    return (CC_RELEASE_CALL);
  }

  DBG_TRACEho("   Connect received.\n");

  callState_ = EHO_ST_ACTIVE;

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobRemoteAnswer): leaving.\n}\n");
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

// Remote Disconnect
JCCEvent_t 
HOHalfCall::handleEHOMobRemoteDisconnect(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobRemoteDisconnect): entering......\n}\n");

  // Cause is Mandatory. Use rlsCause to set it before building
  // !< For now there is a generic rlsCause in the Half Call Object.

  // To avoid sending the release to H323
  parent->disconnectSentToVoip = true;

  //CAUSE<xxu:03-21-00> BEGIN

  rlsCause.causeValue = 
           (T_CNI_RIL3_CAUSE_VALUE) remMsgData.basicCallInMsg.ReasonCode;

  callState_ = EHO_ST_FREE;

  // Trigger the sending of END HANDOVERS and Releasing HO Call Legs
  // (make sure this is an anchor session... should not be called in target!)
  if (parent->parent->handoverSession == parent->parent->anchorHandoverSession)
    {
       parent->parent->anchorHandoverSession->handleAnchPostExtHoRls();
    }

  DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobRemoteDisconnect): leaving.\n}\n");
  return (CC_RELEASE_CALL);

}



/*******************************************************************/
HOMoHalfCall::HOMoHalfCall (HOCallLeg *callLeg, JCCTimer *tclTimer)
{
    parent = callLeg;

    hcTimer = tclTimer;
    initData();
}

void
HOMoHalfCall::setupNewCall (void)
{

  DBG_TRACEho("{\nMNEThoTRACE(HOMoHalfCall::setupNewCall): entering......\n}\n");

  // set to null state so that we can take care of abnormal cleanups.
  callState_ = EHO_ST_NULL;

  // set first event of SETUP message to VoIP
  currEvent = OHC_EHO_LOCAL_TERM_ADDRESS;

  smHandleFirstCallEvent();

  DBG_TRACEho("{\nMNEThoTRACE(HOMoHalfCall::setupNewCall): leaving.\n}\n");
}

void
HOMoHalfCall::disconnectNewCall (void)
{

  DBG_TRACEho("{\nMNEThoTRACE(HOMoHalfCall::disconnectNewCall): entering......\n}\n");

  currEvent = HC_EHO_LOCAL_DISCONNECT_REQ;

  smHandleFirstCallEvent();

  DBG_TRACEho("{\nMNEThoTRACE(HOMoHalfCall::disconnectNewCall): leaving.\n}\n");
}



JCCEvent_t 
HOHalfCall::handleEHOMobLocalDisconnect(void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOMoHalfCall::handleEHOMobLocalDisconnect): entering......\n}\n");

  voipReleaseCause_ = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;

  // Always do sendRemote at the end of the handler.
  // Now send the internal Rel Complt message to the other H.323 half call - 
  if (sendRemote(HC_REMOTE_RELEASE_COMPLETE) == false)
  // failed commn. with the remote side - treat as a release complete
  // from the remote side and cleanup - skip event loop.
  {
      DBG_ERRORho("   ERROR in sending RELEASE COMPLETE to remote side.\n");

      // Since this is the release sequence.  Continue anyway.
      DBG_TRACEho("   Continue release anyway.\n");
  }

  DBG_TRACEho("{\nMNEThoTRACE(HOMoHalfCall::handleEHOMobLocalDisconnect): leaving.\n}\n");
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

void
HOMoHalfCall::initData(void)
{
  HOHalfCall::initData();

}



/*******************************************************************/
HOMtHalfCall::HOMtHalfCall (HOCallLeg *callLeg, JCCTimer *tclTimer)
{
    HOHalfCall::parent = callLeg;

    hcTimer = tclTimer;
    initData();
}

void
HOMtHalfCall::initData(void)
{
  HOHalfCall::initData();
}

//ext-HO <xxu:06=07-01>
JCCEvent_t 
HOHalfCall::handleEHOMobRemoteTermAddress(void)
{
  JCCEvent_t retEvent;

  DBG_FUNC("HOHalfCall::handleEHOMobRemoteTermAddress", CC_ETRG_LAYER);
  DBG_ENTER();
	         

  DBG_TRACEho("{\nMNETeiTRACE(hoCall::handleEHOMobRemoteTermAddress): [<==VOIP SETUP] (hoAgId=%d,callId=%d]\n}\n",
	            parent->hoAgId(), parent->callIndexEHO() );
  
  // copy from the remote message to the local var
  cpn = remMsgData.callOffering.CalledPartyNumber;

  DBG_TRACE("{\nMNETeiTRACE(hoCall::handleEHOMobRemoteTermAddress): hoNumber (cpn) received in SETUP\n");
  DBG_TRACE("                : (ie_present=%d),(numberType=%d),(plan=%d),(numDigits=%d)\n",
              cpn.ie_present,
              cpn.numberType,
              cpn.numberingPlan,
              cpn.numDigits
			);
  
  DBG_TRACE("                : handover number digits (%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x)\n}\n",
              cpn.digits[0],cpn.digits[1],cpn.digits[2],cpn.digits[3],cpn.digits[4],
			  cpn.digits[5],cpn.digits[6],cpn.digits[7],cpn.digits[8],cpn.digits[9],cpn.digits[10]
			);
  
  //Eject CALL PROGRESS to VOIP right away 
  retEvent = fakeEHOMobCallProgress();
  
  DBG_LEAVE();
  return (retEvent);
}

//ext-HO <xxu:06=07-01>
JCCEvent_t 
HOHalfCall::fakeEHOMobCallProgress(void)
{
  DBG_FUNC("HOHalfCall::ejectEHOMobCallProgress", CC_ETRG_LAYER);
  DBG_ENTER();
  
  if (parent->speechVersion() == -1)
  {
      DBG_ERRORho("{\nMNETeiERROR(hoCall::fakeEHOMobCallProgress): no speech version recorded\n}\n");
	  rlsCause.causeValue = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
	  voipReleaseCause_   = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
     
	  DBG_LEAVE();
      return (CC_RELEASE_CALL);
  }
  
  remMsgData.callProceeding.bearerCap.speechVersionInd[0].fieldPresent = 1;
  remMsgData.callProceeding.bearerCap.speechVersionInd[0].version = parent->speechVersion();

  remMsgData.callProceeding.bearerCap.speechVersionInd[1].fieldPresent = 0;
  remMsgData.callProceeding.bearerCap.speechVersionInd[2].fieldPresent = 0;
      
  // Always do sendRemote at the end of the handler
  if (sendRemote(HC_REMOTE_CALL_PROGRESS) == false)
  {
      DBG_ERRORho("{\nMNETeiERROR(hoCall::fakeEHOMobCallProgress): [==>VOIP PROGRESS failed] (hoAgId=%d, callId=%d,speechVersion=%d)\n}\n",
                   parent->hoAgId(), parent->callIndexEHO(),parent->speechVersion());
	  rlsCause.causeValue = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
	  voipReleaseCause_   = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
     
	  DBG_LEAVE();
      return (CC_RELEASE_CALL);
  }

  DBG_TRACEho("{\nMNETeiTRACE(hoCall::fakeEHOMobCallProgress): [==>VOIP CALL PROGRESS] (hoAgId=%d,callId=%d,speechVersion=%d)\n}\n",
                  parent->hoAgId(), parent->callIndexEHO(),parent->speechVersion() );

  //reuse this timer for watch dog purpose in case no response from VOIP 
  hcTimer->setTimer(CALL_CALL_CONFIRMED_T310);

  callState_ = EHO_ST_CONFIRMED;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

//ext-HO <xxu:06=07-01>
JCCEvent_t 
HOHalfCall::handleHoCallTimeout(void)
{

  DBG_FUNC("HOHalfCall::handleHoCallTimeout", CC_HO_LAYER);
  DBG_ENTER();

  DBG_TRACEho("{\nMNETeiTRACE(hoCall::handleHoCallTimeout): [<==CC HO-CALL TIMEOUT] (hoAgId=%d, callId=%d,callState=%d)\n}\n",
	             parent->hoAgId(), parent->callIndexEHO(), callState_);

  if (sendRemote(HC_REMOTE_RELEASE_COMPLETE) == false)
  {
      DBG_ERRORho("{\nMNETeiERROR(hoCall::handleHoCallTimeout): [==>VOIP RELEASE_COMPLETE failed] (hoAgId=%d, callId=%d)\n}\n",
                    parent->hoAgId(), parent->callIndexEHO());
  }

  DBG_TRACEho("{\nMNETeiTRACE(hoCall::handleHoCallTimeout): [==>VOIP RELEASE_COMPLETE cos timeout] (hoAgId=%d, callId=%d)\n}\n",
                    parent->hoAgId(), parent->callIndexEHO());

  rlsCause.causeValue = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
  voipReleaseCause_   = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
     
  DBG_LEAVE();

  // JOE: probably a better way than looking at procType (esp considering post handover cases)
  if (parent->procType == CC_PROC_MOB_ORIG)
    { // hand-out scenario
      return (CC_MESSAGE_PROCESSING_COMPLT);
    } 
  else
    { // Hand-in scenario
      return (CC_RELEASE_CALL);
    }

}

JCCEvent_t  
HOHalfCall::handleEHOMobRemoteChannSelectedEi(void)
{
  JCCEvent_t retEvent;

  DBG_FUNC("HOHallCall::handleEHOMobRemoteChannSelectedEi", CC_ETRG_LAYER);
  DBG_ENTER();

  //Stop guarding timer
  hcTimer->cancelTimer();
  
  //Sanitory check applied
  if (remMsgData.codecSelected.Codec != parent->speechVersion() )
  {
      DBG_ERRORho("{\nMNETeiERROR(hoCall::handleEHOMobRemoteChannSelectedEi): [<==VOIP CODEC SELECT failed]  failed] (hoAgId=%d, callId=%d,loCodec=%d,reCodec=%d)\n}\n",
		        parent->hoAgId(), parent->callIndexEHO(), remMsgData.codecSelected.Codec,
				parent->speechVersion());

	  rlsCause.causeValue = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
	  voipReleaseCause_   = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
     
	  DBG_LEAVE();
      return (CC_RELEASE_CALL);
  }

  DBG_TRACEho("{\nMNETeiTRACE(hoCall::handleEHOMobRemoteChannSelectedEi): [<==VOIP CODEC SELECT] (hoAgId=%d, callId=%d, codec=%d)\n}\n",
		         parent->hoAgId(), parent->callIndexEHO(), remMsgData.codecSelected.Codec);
				 
  currSpeechCh_.version = remMsgData.codecSelected.Codec;
 
  //inject CONNECT to VOIP here
  retEvent = fakeEHOMobConnect();

  DBG_LEAVE();
  return (retEvent);

}

//ext-HO <xxu:06-07-01>
JCCEvent_t 
HOHalfCall::handleEHOMobRemoteChannReady(void)
{
  DBG_FUNC("HOHallCall::handleEHOMobRemoteChannReady", CC_HO_LAYER);
  DBG_ENTER();

  DBG_TRACEho("{\nMNEThoTRACE(hoCall::handleEHOMobRemoteChannReady): [<==VOIP VOICE CONNECTED] (hoAgId=%d, callId=%d)\n}\n",
               parent->hoAgId(), parent->callIndexEHO());
  
  //JOE: to cover post-handout case where src could be either RTP or GSM
  //Make two-way connnection
  T_CSU_PORT_ID     src, snk;
  T_CSU_RESULT_CON  csuResult;

  snk.portType = CSU_RTP_PORT;
  snk.portId.rtpHandler = VoipCallGetRTPHandle(parent->h323CallHandle);

  // If hand-out then SRC is RTP, else hand-in SRC is GSM
  // JOE: probably a better way than looking at procType (esp considering post handover cases)
  if (parent->procType == CC_PROC_MOB_ORIG)
    { // hand-out scenario
       TwoPartyCallLeg *whichLeg;

       // as long as handover is allowed, active leg is mandatory here.
       if ((whichLeg = parent->parent->activeLeg()) == NULL)
       {
         DBG_TRACEho("   Call no longer active.\n");
         DBG_TRACEho("{\nMNEThoTRACE(HOHalfCall::handleEHOMobRemoteChannReady): leaving.\n}\n");
         return (CC_RELEASE_CALL);
       }

       src.portType = CSU_RTP_PORT;
       src.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->h323CallHandle);
    } 
  else
    { // Hand-in scenario
       src.portType = CSU_GSM_PORT;
       src.portId.gsmHandler = parent->entryId();
    }

  if ( (csuResult = csu_DuplexConnect(&src, &snk))
       != CSU_RESULT_CON_SUCCESS)
  {
       DBG_ERROR("{\nMNEThoERROR(hoCall::handleEHOMobRemoteChannReady): [==>CSU DUPLEX CONNECT failed] (hoAgId=%d,callId=%d,result=%d,src(%d,%x),snk(%d,%x))\n}\n",
                  parent->hoAgId(), parent->callIndexEHO(), csuResult, src.portType, src.portId.rtpHandler, snk.portType, snk.portId.rtpHandler);  csuResult, 
                
       rlsCause.causeValue = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
       voipReleaseCause_   = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;

       DBG_LEAVE();
       return (CC_RELEASE_CALL);
  }
 
  DBG_TRACEho("{\nMNEThoTRACE(hoCall::handleEHOMobRemoteChannReady): [==>CSU DUPLEX CONNECTION] (hoAgId=%d,callId=%d, src(%d,%x),snk(%d,%x))\n}\n",
               parent->hoAgId(), parent->callIndexEHO(), src.portType, src.portId.rtpHandler, snk.portType, snk.portId.rtpHandler); 
 
  parent->voiceConnected = true;

  //Cancel handover timer if needed
  parent->cancelHoTimerIfNeeded();
  //if (!parent->cancelHoTimerIfNeeded())
  //{
  //    DBG_WARNING("{\nMNEThoWARNING(\nMENTeiWARNING(hoCall::handleEHOMobRemoteChannReady): [HO CALL UP BUT NO HANDOVER] (hoAgId=%d,callId=%d)\n}\n",
  //                  parent->hoAgId(), parent->callIndexEHO() );
  //              
  //    rlsCause.causeValue = CNI_RIL3_CAUSE_CALL_REJECTED;
  //    voipReleaseCause_   = CNI_RIL3_CAUSE_CALL_REJECTED;
  //
  //    DBG_LEAVE();
  //    return (CC_RELEASE_CALL);
  //}

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

//ext-HO <xxu:06-07-01>
JCCEvent_t 
HOHalfCall::fakeEHOMobConnect(void)
{
  DBG_FUNC("HOHallCall::fakeEHOMobConnect", CC_HO_LAYER);
  DBG_ENTER();

  //inject a fake CONNECT message sent to VOIP right now
  if (sendRemote(OHC_REMOTE_ANSWER) == false)
  {
      rlsCause.causeValue = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
      voipReleaseCause_   = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;

	  
      DBG_ERRORho("{\nMNEThoTRACE(hoCall::fakeEHOMobConnect): [==>VOIP CONNECT sendRemote failed] (hoAgId=%d, callId=%d)\n}\n",
		        parent->hoAgId(), parent->callIndexEHO());


      DBG_LEAVE();
      return (CC_RELEASE_CALL);
  }

  DBG_TRACEho("{\nMNEThoTRACE(hoCall::fakeEHOMobConnect): [==>VOIP CONNECT] (hoAgId=%d, callId=%d)\n}\n",
               parent->hoAgId(), parent->callIndexEHO());

  callState_ = EHO_ST_ACTIVE;
  
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}
