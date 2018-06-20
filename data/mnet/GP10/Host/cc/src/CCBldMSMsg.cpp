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
// File        : CCBldMSMsg.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included VxWorks headers
#include "vxWorks.h"

#include "JCC/JCCLog.h"
#include "Logging/VCLOGGING.h"

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"

#include "CC/CCHalfCall.h"

#include "CC/CCSessionHandler.h"

#include "ril3/ril3_common.h"

#include "ril3/ril3md.h"

#include "pm/pm_class.h"

#include "CC/CallLeg.h"

void 
CCHalfCall::buildMSMsgCommon()
{
  // But, before we start populating the message content, 
  // zap it clean first.
  DBG_FUNC("CCHalfCall::buildMSMsgCommon", CC_HALFCALL_LAYER);
  DBG_ENTER();

  CNI_RIL3_ZAPMSG(&msOutMsg, sizeof(T_CNI_RIL3CC_MSG));
          
  // header portion --------------------------------------------------
  msOutMsg.header.protocol_descriminator = CNI_RIL3_PD_CC;
  msOutMsg.header.si_ti = ti;
  DBG_LEAVE();
}  

T_CNI_RIL3_RESULT 
CCHalfCall::buildMSMsg(T_CNI_LAPDM_L3MessageUnit        *msEncodedMsg)
{

  DBG_FUNC("CCHalfCall::buildMSMsg", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // call the Encoder function to encode the message
  msOutResult = CNI_RIL3CC_Encode(&msOutMsg,    // input message definition
                                  msEncodedMsg  // encoded layer-3 message
                                  );
  
  // check the encoding result before sending the message
  if (msOutResult != CNI_RIL3_RESULT_SUCCESS)
    {
      JCCLog1("CC Error : Encoding MS Message (Problem = %d) \n ", 
              msOutResult);
      DBG_ERROR("CC Error : Encoding MS Message (Problem = %d) \n ", 
                msOutResult);
    }
  else
    {
      // encoding successful, send the message to LAPDm for delivery
      parent->parent->sendL2Msg(msEncodedMsg);

      JCCLog1("CC Call Log: Sent Message to LAPDm, (Message Type = %d)\n",  
              msOutMsg.header.message_type);
      DBG_TRACE("CC Call Log: Sent Message to LAPDm, (Message Type = %d)\n",  
                msOutMsg.header.message_type);
      
    }

  DBG_LEAVE();
  return (msOutResult);
} 

T_CNI_RIL3_RESULT 
CCHalfCall::buildStartDtmfAck(T_CNI_LAPDM_L3MessageUnit *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_START_DTMF_ACK                               
    *startDTMFAck
    = &msOutMsg.startDTMFAck ;

  DBG_FUNC("CCHalfCall::buildStartDtmfAck", CC_HALFCALL_LAYER);
  DBG_ENTER();
   
  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_START_DTMF_ACKNOWLEDGE;
  startDTMFAck->keypadFacility = currKeypadFacility_;
    
  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildStartDtmfRej(T_CNI_LAPDM_L3MessageUnit *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_START_DTMF_REJECT
    *startDTMFReject
    = &msOutMsg.startDTMFReject ;
   
  DBG_FUNC("CCHalfCall::buildStartDtmfRej", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_START_DTMF_REJECT;

  // Worry about exact reason later!
  startDTMFReject->cause = rlsCause;
    
  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildStopDtmfAck(T_CNI_LAPDM_L3MessageUnit  *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_STOP_DTMF_ACK
    *stopDTMFAck
    = &msOutMsg.stopDTMFAck ;
   
  DBG_FUNC("CCHalfCall::buildStopDtmfAck", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_STOP_DTMF_ACKNOWLEDGE;
    
  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildAlerting(T_CNI_LAPDM_L3MessageUnit     *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_ALERTING
    *alerting
    = &msOutMsg.alerting ;
   
  DBG_FUNC("CCHalfCall::buildAlerting", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_ALERTING;

  //Test Code
  //msOutMsg.alerting.progressInd = h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE;
  DBG_TRACE("CC CALL LOG: progressIE info exist(%d) codingStd(%d) loc(%d) desc(%d)\n",
             h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.ie_present,
             h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.codingStandard,
             h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.location,
             h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.progressDesp);
    
  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}


T_CNI_RIL3_RESULT 
CCHalfCall::buildProgress(T_CNI_LAPDM_L3MessageUnit     *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_PROGRESS
    *progress
    = &msOutMsg.progress ;
   
  DBG_FUNC("CCHalfCall::buildProgress", CC_HALFCALL_LAYER);
  DBG_ENTER();

  //Test Code
  DBG_TRACE("CC CALL LOG: buildProgress progress IE info exist(%d) codingStd(%d) loc(%d) desc(%d)\n",
             h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.ie_present,
             h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.codingStandard,
             h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.location,
             h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE.progressDesp);

  buildMSMsgCommon();
  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_PROGRESS;
  msOutMsg.progress.progressInd = h323InMsg->l3_data.voipMsg.basicCallInMsg.EventInfo.AlertingProgressIE;
      
  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildCallProceeding(T_CNI_LAPDM_L3MessageUnit       *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_CALL_PROCEEDING
    *callProceeding
    = &msOutMsg.callProceeding ;

  DBG_FUNC("CCHalfCall::buildCallProceeding", CC_HALFCALL_LAYER);
  DBG_ENTER();
   
  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_CALL_PROCEEDING;

  // Add Bearer Capability  standard one from the local variable.
  callProceeding->bearCap1 = bearerCap;
  
  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildConnect(T_CNI_LAPDM_L3MessageUnit      *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_CONNECT
    *connect
    = &msOutMsg.connect;

  DBG_FUNC("CCHalfCall::buildConnect", CC_HALFCALL_LAYER);
  DBG_ENTER();
   
  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_CONNECT;
    
  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildConnectAck(T_CNI_LAPDM_L3MessageUnit   *msEncodedMsg)
{
  DBG_FUNC("CCHalfCall::buildConnectAck", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  // No IEs in this message other than header.
  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_CONNECT_ACKNOWLEDGE;
    
  // peg
  PM_CCMeasurement.ansMobileTerminatingCalls.increment();

  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildDisconnect(T_CNI_LAPDM_L3MessageUnit   *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_DISCONNECT
    *disconnect
    = &msOutMsg.disconnect ;

  DBG_FUNC("CCHalfCall::buildDisconnect", CC_HALFCALL_LAYER);
  DBG_ENTER();
   
  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_DISCONNECT;
    
  disconnect->cause = rlsCause;
  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildReleaseComplete(T_CNI_LAPDM_L3MessageUnit      *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_RELEASE_COMPLETE
    *releaseComplete
    = &msOutMsg.releaseComplete;
   
  DBG_FUNC("CCHalfCall::buildReleaseComplete", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type =CNI_RIL3CC_MSGID_RELEASE_COMPLETE;
   
  releaseComplete->cause = rlsCause;
  
  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildReleaseReq(T_CNI_LAPDM_L3MessageUnit   *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_RELEASE
    *release 
    = &msOutMsg.release ;
   
  DBG_FUNC("CCHalfCall::buildReleaseReq", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_RELEASE;
    
  release->cause = rlsCause;

  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

T_CNI_RIL3_RESULT 
CCHalfCall::buildSetup(T_CNI_LAPDM_L3MessageUnit        *msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_SETUP
    *setup 
    = &msOutMsg.setup ;

  DBG_FUNC("CCHalfCall::buildSetup", CC_HALFCALL_LAYER);
  DBG_ENTER();
   
  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_SETUP;

  // Add Bearer Capability  standard one from the local variable.
  setup->bearCap1 = bearerCap;

  setup->callingBCD = callingPartyNum;
      
  if(parent->waitingCallLeg)
    {
      // Add signal Information Element
      setup->signal.ie_present = true;
      setup->signal.signalValue = CNI_RIL3_SIGNAL_CALL_WAITING_TONE_ON;
    }

  // peg
  PM_CCMeasurement.attMobileTerminatingCalls.increment();

  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}


//CH<xxu:11-10-99>
T_CNI_RIL3_RESULT 
CCHalfCall::buildHoldAck(T_CNI_LAPDM_L3MessageUnit	*msEncodedMsg)
{
  DBG_FUNC("CCHalfCall::buildHoldAck", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_HOLD_ACKNOWLEDGE;

  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));

}

//CH<xxu:11-10-99>
T_CNI_RIL3_RESULT 
CCHalfCall::buildRetvAck(T_CNI_LAPDM_L3MessageUnit	*msEncodedMsg)
{
  DBG_FUNC("CCHalfCall::buildRetvAck", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_RETRIEVE_ACKNOWLEDGE;

  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));

}

//CH<xxu:11-10-99>
T_CNI_RIL3_RESULT 
CCHalfCall::buildHoldRej(T_CNI_LAPDM_L3MessageUnit	*msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_HOLD_REJECT
    *holdReject 
    = &msOutMsg.holdReject ;

  DBG_FUNC("CCHalfCall::buildHoldRej", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_HOLD_REJECT;
  
  //Mandatory IE: cause
  holdReject->cause.ie_present = true; 
  holdReject->cause.codingStandard = CNI_RIL3_GSMPLMN_CODING;
  holdReject->cause.location =CNI_RIL3_LOCATION_PRIVATE_NETWORK_LOCAL_USER;
  holdReject->cause.causeValue = chCause_; 
  holdReject->cause.diagnosticsLength = 0;

  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));
}

//CH<xxu:11-10-99>
T_CNI_RIL3_RESULT 
CCHalfCall::buildRetvRej(T_CNI_LAPDM_L3MessageUnit	*msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_RETRIEVE_REJECT
    *retvReject
    = &msOutMsg.retrieveReject;

  DBG_FUNC("CCHalfCall::buildRetvRej", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_RETRIEVE_REJECT;

  //Mandatory IE: cause
  retvReject->cause.ie_present = true; 
  retvReject->cause.codingStandard = CNI_RIL3_GSMPLMN_CODING;
  retvReject->cause.location =CNI_RIL3_LOCATION_PRIVATE_NETWORK_LOCAL_USER;
  retvReject->cause.causeValue = chCause_; 
  retvReject->cause.diagnosticsLength = 0;

  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));

}

T_CNI_RIL3_RESULT 
CCHalfCall::buildStatusInfo(T_CNI_LAPDM_L3MessageUnit	*msEncodedMsg)
{
  T_CNI_RIL3CC_MSG_STATUS
    *statusMsg
    = &msOutMsg.status;

  DBG_FUNC("CCHalfCall::buildStatusInfo", CC_HALFCALL_LAYER);
  DBG_ENTER();

  buildMSMsgCommon();

  msOutMsg.header.message_type = CNI_RIL3CC_MSGID_STATUS;

  // Mandatory IE: cause
  statusMsg->cause = rlsCause; 
  if (msDecodeResult == CNI_RIL3_RESULT_SUCCESS)   // not caused by protocol errors
    {
      DBG_TRACE("CC Call Log: Response to status enquiry\n");
      statusMsg->cause.causeValue = CNI_RIL3_CAUSE_RESPONSE_TO_STATUS_ENQUIRY; 
    }
  else
    {
      DBG_TRACE("CC Call Log: status message because of decoding error, (dec.res.=%d)\n",
                msDecodeResult);
    }

  // Mandatory IE: call state
  statusMsg->callState.ie_present = true;
  statusMsg->callState.codingStandard = CNI_RIL3_GSMPLMN_CODING;

  switch(callState_) {
  case HC_ST_DISCONNECT_IND:  			
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_DISCONNECT_INDICATION;
    break;
  case HC_ST_RELEASE_REQ:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_RELEASE_REQUEST;
    break;
  case HC_ST_ACTIVE:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_ACTIVE;
    break;
  case THC_ST_MM_CONN_REQ:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_MM_CONNECTION_PENDING;
    break;
  case THC_ST_CALL_CONFIRMED:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_MT_CALL_CONFIRMED;
    break;
  case THC_ST_ALERTING:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_CALL_RECEIVED;
    break;
  case THC_ST_PRESENTING_CALL:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_CALL_PRESENT;
    break;
  case OHC_ST_CONNECT_INDICATION:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_CONNECT_INDICATION;
    break;
  case OHC_ST_ANSWER_PENDING:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_CALL_DELIVERED;
    break;
  case OHC_ST_CALL_PROCEEDING:
  case OHC_ST_OFFERING_CALL:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_MO_CALL_PROCEEDING;
    break;
  case HC_ST_NULL:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_NULL;
    break;
  default:
    statusMsg->callState.state = CNI_RIL3_CALL_STATE_NULL;

  }
		
  if (chState_ != CH_ST_IDLE)
    {
      statusMsg->auxStates.ie_present = true;
      statusMsg->auxStates.holdState = (T_CNI_RIL3_HOLD_AUXILIARY_STATES)(chState_ - 1);
      // Change when multiparty is implemented.
      statusMsg->auxStates.mptyState = CNI_RIL3_MPTY_STATE_IDLE;
    }

  DBG_LEAVE();
  return (buildMSMsg(msEncodedMsg));

}
