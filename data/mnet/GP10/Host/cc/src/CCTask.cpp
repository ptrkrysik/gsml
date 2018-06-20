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
// File        : CCTask.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : The Main Loop for the controlling root CC task.
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "stdio.h"
#include "JCC/JCCLog.h"
#include "logging/VCLOGGING.h"

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"
#include "jcc/LUDBInstance.h"
#include "JCC/LUDBapi.h"

// CC Types
#include "CC/CCHalfCall.h"
// ext-HO <chenj:05-16-01>
#include "CC/HOHalfCall.h"

#include "CC/CCHandover.h"

#include "CC/CCSessionHandler.h"

#include "CC/CCInt.h"
#include "CC/CCUtil.h"

#include "CC/CCconfig.h"
#include "CC/CallConfig.h"

#include "CC/CCH323Util.h"

#include "oam_api.h"

#include "CC/hortp.h"
#include "jcc/viperchannel.h"
#include "Os/JCTask.h"

#include "ril3\ril3_sms_rp.h"

bool smsSendVbLinkRelease (LUDB_ID, UINT32, T_CNI_RIL3_RP_CAUSE_VALUE cause = 
                           CNI_RIL3_RP_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE);

JCTask ccTaskObj("CCTask");

//ext-HO <xxu: 05-16-01>
DbgOutput ccExtHoEiDbg("GP10ei:", CC_EI_LAYER);
DbgOutput ccExtHoEoDbg("GP10eo:", CC_EO_LAYER);
DbgOutput ccExtHoHoDbg("GP10ho:", CC_HO_LAYER);
bool ccCallTrace=false;

// Call initialization function
extern int callInit(const short callIndex);

// Call Oam functions
extern void callOamProcessTrapMsg(TrapMsg* oamInMsg);
extern void ccOamDataInit(void);
extern void callOamTrapRegister(void);

FUNCPTR                      callTimerExpiry; // call timer action - with the Qid

INT_32_T ccOldBTSState = VAL_bts_operationalState_disabled;

bool callTrace = false;

DBG_FUNC("CC Main", CC_LAYER);

void 
callTraceOn (void)
{
  callTrace = true;
}

void 
callTraceOff (void)
{
  callTrace = false;
}

int callTimerExpiryAction(int timerData)
{   
  // The timerData parameter is of 4 bytes long 
  //    
  // When timer is fired and this function is called by the System Clock ISR, 
  // the timerData is passed back.

  // format a timer expiration message and send to the CC/MM task that
  // is using the timer.
 
  UINT16 uint16TimerData;
  short callId;
  MSG_Q_ID qid;  

  IntraL3Msg_t  timeoutMsg;

  uint16TimerData = ((UINT16)timerData) & 0x0FFF;

  callId = (short)(uint16TimerData & 0x00FF);

  if ((0 <= callId) &&
      ( callId < CC_MAX_CC_CALLS))
    {
      qid = ccSession[callId].msgQId;
    }
  else
    {
      // send it to the cc main task so it can print out a log.
      // Don't log from here.
      qid = ccMsgQId;
    }

  timeoutMsg.module_id = MODULE_CC;

  // send the call leg number in this field
  timeoutMsg.entry_id = (short) ((uint16TimerData & 0x0F00) >> 8);
  
  timeoutMsg.message_type = INTRA_L3_TIMEOUT_MSG;

  // CDR <xxu:08-22-00>
  timeoutMsg.signature = timerData;
  
  // send the message. Since this is invoked by the System Clock ISR,
  // we do NOT want to wait for the message delivery. 
  // Return it right away if cannot send on the message queue.
  msgQSend( qid,
            (char *) &timeoutMsg, 
            sizeof(struct  IntraL3Msg_t), 
            NO_WAIT,      // this is called by ISR, don't wait
            MSG_PRI_NORMAL
            );
  return 0;
  
}

int CCTimerExpiryAction(int timerData)
{   
  // The timerData parameter is of 4 bytes long 
  //    
  // When timer is fired and this function is called by the System Clock ISR, 
  // the timerData is passed back.

  // format a timer expiration message and send to the CC/MM task that
  // is using the timer.
 
  IntraL3Msg_t  timeoutMsg;

  timeoutMsg.module_id = MODULE_CC;
  timeoutMsg.entry_id = timerData;

  timeoutMsg.message_type = INTRA_L3_TIMEOUT_MSG;
        
  // send the message. Since this is invoked by the System Clock ISR,
  // we do NOT want to wait for the message delivery. 
  // Return it right away if cannot send on the message queue.
  msgQSend( ccMsgQId, 
            (char *) &timeoutMsg, 
            sizeof(struct  IntraL3Msg_t), 
            NO_WAIT,      // this is called by ISR, don't wait
            MSG_PRI_NORMAL
            );
  return 0;
}


// Initialize the Message Handler Member function table.
void msgHandlerInit(void)
{
  DBG_FUNC("msgHandlerInit", CC_LAYER);
  DBG_ENTER();

  // First initialize everything to a handler for invalid event.
  short i, j;
  
  for (i=0; i< MAX_GENERATED_EVENTS; i++)
    {
      generatedEventHandler[i] = &CCHalfCall::handleInvalidEvent;
    }

  for (i=0; i< MAX_CALL_EXTERNAL_EVENTS; i++)
    {
      for (j=0; j< MAX_CALL_STATES ; j++)
        {
          callStateHandler[j][i] = &CCHalfCall::handleInvalidEvent;
        }
    }

  // ext-HO <chenj:06-13-01>
  for (i=0; i< MAX_EHO_CALL_EXTERNAL_EVENTS; i++)
    {
      for (j=0; j< MAX_EHO_CALL_STATES ; j++)
        {
          ehoCallStateHandler[j][i] = &HOHalfCall::handleInvalidEvent;
        }
    }

  for (i=0; i< MAX_HO_SRC_EVENTS; i++)
    {
      for (j=0; j< MAX_HO_SRC_STATES ; j++)
    {
      hoSrcHandler[j][i] = &CCHandover::handleInvalidEvent;
    }
    }

  for (i=0; i< MAX_HO_TRG_EVENTS; i++)
    {
      for (j=0; j< MAX_HO_TRG_STATES ; j++)
    {
      hoTrgHandler[j][i] = &CCHandover::handleInvalidEvent;
    }
    }

  for (i=0; i< MAX_HO_TRG_EVENTS+MAX_HO_SRC_EVENTS; i++)
    {
      for (j=0; j< MAX_HO_TRG_STATES+MAX_HO_SRC_STATES ; j++)
    {
      extHoTrgHandler[j][i] = &CCTargetHandoverExtHo::handleInvalidEvent;
    }
    }

  // Handler for entire row of handover: 
  for (j=0; j<  MAX_HO_SRC_STATES; j++)
    {
      hoSrcHandler[j][HAND_TRG_HO_REQ_ACK] = &CCHandover::handleMMLateEstCnf;
    }

  // Handler for entire row of handover: 
  for (j=0; j<  MAX_HO_TRG_STATES; j++)
    {
      hoSrcHandler[j][HAND_TRG_HO_REQ_ACK] = &CCHandover::handleMMLateEstCnf;
    }

  for (i=0; i< MAX_CHANN_EXTERNAL_EVENTS; i++)
    {
      for (j=0; j< MAX_CHANN_STATES; j++)
        {
          channStateHandler[j][i] = &CCHalfCall::handleInvalidChannEvent;
        }
    }

  // Assign Chann Handlers.
  channStateHandler[RES_ST_ASS_REQ][CC_RESOURCE_ASSIGN_COMPLT - CC_INTERNAL_CHANN_EVENT_BASE] 
    = &CCHalfCall::handleChannAssignComplt ;

  channStateHandler[RES_ST_ASS_REQ][CC_RESOURCE_ASSIGN_FAIL - CC_INTERNAL_CHANN_EVENT_BASE] 
    = &CCHalfCall::handleChannAssignFail;

  // Generated Event Handlers.
  generatedEventHandler[CC_TWO_STAGE_DISCONNECT - CC_INTERNAL_GEN_EVENT_BASE]
    = &CCHalfCall::handleDisconnectReq;
  generatedEventHandler[CC_RESOURCE_ASSIGN_REQ - CC_INTERNAL_GEN_EVENT_BASE]
    = &CCHalfCall::handleChannAssignReq;
  generatedEventHandler[CC_RES_RELEASE  - CC_INTERNAL_GEN_EVENT_BASE]
    = &CCHalfCall::handleMobResRelease;
  generatedEventHandler[CC_MAKE_CONNECTION_REQ  - CC_INTERNAL_GEN_EVENT_BASE]
    = &CCHalfCall::handleConnReq;
  
  //CH<xxu:Mod 12-13-99>
  generatedEventHandler[CC_CONNECT_COMPLT - CC_INTERNAL_GEN_EVENT_BASE]
    = &CCHalfCall::handleConnComplt;
  generatedEventHandler[CC_CONNECT_FAIL - CC_INTERNAL_GEN_EVENT_BASE]
    = &CCHalfCall::handleConnFail;
                                 
  // Entire column initialization
  for (i=0; i< MAX_CALL_EXTERNAL_EVENTS; i++)
    {
      callStateHandler[HC_ST_FREE][i]
        = &CCHalfCall::handleNullEvent;

      callStateHandler[HC_ST_NULL][i]
        = &CCHalfCall::handleNullEvent;

    }

  // ext-HO <chenj:06-13-01>
  // Entire column initialization
  for (i=0; i< MAX_EHO_CALL_EXTERNAL_EVENTS; i++)
    {
      ehoCallStateHandler[EHO_ST_FREE][i]
        = &HOHalfCall::handleNullEvent;

      ehoCallStateHandler[EHO_ST_NULL][i]
        = &HOHalfCall::handleNullEvent;
    }

  // Handler for entire row: for error, disconnect, release

  for (j=HC_ST_ACTIVE; j< MAX_CALL_STATES ; j++)
    {
      // None of these events are applicable to Free or Null states.

      callStateHandler[j][HC_LOCAL_PROTOCOL_PROBLEM - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleMobProtocolProblem;

      callStateHandler[j][CC_MM_CONNECTION_REL - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleMMRelInd;

      callStateHandler[j][HC_REMOTE_RELEASE_COMPLETE - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleMobRemoteAbnormalRelCom;
      
      callStateHandler[j][HC_LOCAL_DISCONNECT_REQ - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleDisconnectReq;
      
      callStateHandler[j][HC_REMOTE_DISCONNECT_REQ - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleMobRemoteDisconnect;
      
      callStateHandler[j][HC_LOCAL_RELEASE_REQ - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleReleaseReq;
      
      callStateHandler[j][HC_LOCAL_RELEASE_COMPLETE - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleMobReleaseComplete;

      // H245 Channel established
      callStateHandler[j][HC_REMOTE_CONNECTION_CONTROL - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleMobRemoteH323ChannReady;

      // Status Handling
      callStateHandler[j][HC_LOCAL_STATUS_INFO - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleMobStatusInfo;
      callStateHandler[j][HC_LOCAL_STATUS_ENQUIRY - CC_INTERNAL_CALL_EVENT_BASE]
        = &CCHalfCall::handleMobStatusEnquiry;
    }

  // External Event Handlers.

  callStateHandler[HC_ST_FREE][THC_REMOTE_TERM_ADDRESS - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteTermAddress;
  
  // To take care of the radio link loss even before setup arrives.
  callStateHandler[HC_ST_NULL][CC_MM_CONNECTION_REL - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMMRelInd;

  //For getting rid of CC stuck at this state. This timeout linked to T999 timer
    callStateHandler[HC_ST_NULL][HC_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobResRelease;

  // Handle a Release coming in even before setup
  callStateHandler[HC_ST_NULL][HC_LOCAL_RELEASE_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleReleaseReq;
  callStateHandler[HC_ST_NULL][OHC_LOCAL_TERM_ADDRESS - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobTermAddress;
  callStateHandler[HC_ST_NULL][OHC_LOCAL_EMERGENCY_ORIGINATION - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobEmergencyOrigination;

  callStateHandler[HC_ST_ACTIVE][HC_REMOTE_DISCONNECT_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteDisconnect;
  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_DISCONNECT_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleDisconnectReq;

//BCT <xxu:09-22-00>
//  callStateHandler[HC_ST_ACTIVE][OHC_REMOTE_ALERTING - CC_INTERNAL_CALL_EVENT_BASE]
//    = &CCHalfCall::handleMobRemoteAlerting;
//  callStateHandler[HC_ST_ACTIVE][HC_REMOTE_CHANN_SELECT - CC_INTERNAL_CALL_EVENT_BASE]
//    = &CCHalfCall::handleMobRemoteChannSelected;

  callStateHandler[HC_ST_ACTIVE][HC_REMOTE_CONNECTION_CONTROL - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteH323ChannReady;

//BCT <xxu:09-22-00>
//callStateHandler[HC_ST_ACTIVE][OHC_REMOTE_ANSWER - CC_INTERNAL_CALL_EVENT_BASE]
//    = &CCHalfCall::handleNullEvent;

  callStateHandler[HC_ST_ACTIVE][HC_REMOTE_CALL_PROGRESS - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleNullEvent;

  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_START_DTMF - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleStartDtmf;
  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_STOP_DTMF - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleStopDtmf;

  //CH<xxu:11-10-99>
  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_HOLD_ACT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleHoldAct;
  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_HOLD_REJ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleHoldRej;
  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_RETV_ACT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleRetvAct;
  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_RETV_REJ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleRetvRej;
  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_HOLD_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleHoldReq;

  //CH<xxu:Mod 12-13-99>
  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_RETV_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleRetvReq;

  //BCT <xxu:09-21-00> BEGIN
  callStateHandler[HC_ST_ACTIVE][HC_REMOTE_BCT_UPDATE_HANDLES - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleBctUpdateHandles;
  callStateHandler[HC_ST_ACTIVE][OHC_REMOTE_ALERTING - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteAlerting2nd;
  callStateHandler[HC_ST_ACTIVE][HC_REMOTE_CHANN_SELECT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteChannSelected2nd;
  callStateHandler[HC_ST_ACTIVE][OHC_REMOTE_ANSWER - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteAnswer2nd;
  //BCT <xxu:09-21-00> END

  callStateHandler[HC_ST_ACTIVE][HC_LOCAL_CHTR_EXP - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleSwapTimerExpiry;

  callStateHandler[HC_ST_RELEASE_REQ][HC_LOCAL_RELEASE_COMPLETE - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobReleaseComplete;
  callStateHandler[HC_ST_RELEASE_REQ][HC_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleReleaseTimeout;
  callStateHandler[HC_ST_RELEASE_REQ][HC_REMOTE_RELEASE_COMPLETE - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleNullEvent;
  callStateHandler[HC_ST_RELEASE_REQ][HC_LOCAL_DISCONNECT_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleNullEvent;
  callStateHandler[HC_ST_RELEASE_REQ][HC_REMOTE_DISCONNECT_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleNullEvent;

  // Take care of the race condition where a release request is received 
  // when a release request is sent to the MS.
  callStateHandler[HC_ST_RELEASE_REQ][HC_LOCAL_RELEASE_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobReleaseComplete;

  callStateHandler[HC_ST_DISCONNECT_IND][HC_LOCAL_RELEASE_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleReleaseReq;
  callStateHandler[HC_ST_DISCONNECT_IND][HC_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleDisconnectTimeout;
  callStateHandler[HC_ST_DISCONNECT_IND][HC_LOCAL_DISCONNECT_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleDisconnectReq; 
  // The above rare situation will send two Disconnects to the remote side - 
  // No problem - ignored through NULL event.
  callStateHandler[HC_ST_DISCONNECT_IND][HC_REMOTE_DISCONNECT_REQ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleNullEvent;
  callStateHandler[HC_ST_DISCONNECT_IND][HC_REMOTE_RELEASE_COMPLETE - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleNullEvent;

  callStateHandler[OHC_ST_OFFERING_CALL][HC_REMOTE_CALL_PROGRESS_1 - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteCallProgress1;
  callStateHandler[OHC_ST_OFFERING_CALL][HC_REMOTE_CALL_PROGRESS - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteCallProgress;
  callStateHandler[OHC_ST_OFFERING_CALL][OHC_REMOTE_ALERTING - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteAlerting;
  callStateHandler[OHC_ST_OFFERING_CALL][OHC_REMOTE_ANSWER - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteAnswer;
  callStateHandler[OHC_ST_OFFERING_CALL][HC_REMOTE_CHANN_SELECT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteChannSelected;

  //BCT <xxu:08-11-00> BEGIN
  callStateHandler[OHC_ST_OFFERING_CALL][HC_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleBctNumberReqTimeout;
  callStateHandler[OHC_ST_OFFERING_CALL][HC_VB_GET_BCT_NUMBER_RSP - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleGetBctNumberNack;
  //BCT <xxu:08-11-00> END

  callStateHandler[OHC_ST_CALL_PROCEEDING][OHC_REMOTE_ALERTING - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteAlerting;
  callStateHandler[OHC_ST_CALL_PROCEEDING][OHC_REMOTE_ANSWER - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteAnswer;
  callStateHandler[OHC_ST_CALL_PROCEEDING][HC_REMOTE_CHANN_SELECT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteChannSelected;
  callStateHandler[OHC_ST_CALL_PROCEEDING][HC_REMOTE_CALL_PROGRESS_1 - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteCallProgress1;


  callStateHandler[OHC_ST_ANSWER_PENDING][OHC_REMOTE_ANSWER - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteAnswer;
  callStateHandler[OHC_ST_ANSWER_PENDING][HC_REMOTE_CHANN_SELECT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteChannSelected;
  callStateHandler[OHC_ST_ANSWER_PENDING][HC_REMOTE_CALL_PROGRESS - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteCallProgress;
  callStateHandler[OHC_ST_ANSWER_PENDING][HC_REMOTE_CALL_PROGRESS_1 - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteCallProgress1;

  callStateHandler[OHC_ST_CONNECT_INDICATION][OHC_LOCAL_CONNECT_ACK - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleConnectAck;
  callStateHandler[OHC_ST_CONNECT_INDICATION][HC_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleDisconnectReq;
  callStateHandler[OHC_ST_CONNECT_INDICATION][HC_REMOTE_CALL_PROGRESS_1 - CC_INTERNAL_CALL_EVENT_BASE]
    = CCHalfCall::handleNullEvent;

  callStateHandler[THC_ST_PRESENTING_CALL][HC_LOCAL_CALL_PROGRESS - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobCallProgress;
  callStateHandler[THC_ST_PRESENTING_CALL][HC_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleReleaseReq;  // to send Release Complete

  callStateHandler[THC_ST_ALERTING][THC_LOCAL_ANSWER - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobAnswer;
  callStateHandler[THC_ST_ALERTING][HC_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobNoReply;
  callStateHandler[THC_ST_ALERTING][HC_REMOTE_CHANN_SELECT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteChannSelected;

  callStateHandler[THC_ST_CALL_CONFIRMED][THC_LOCAL_ALERTING - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobAlerting;
  callStateHandler[THC_ST_CALL_CONFIRMED][THC_LOCAL_ANSWER - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobAnswer;
  callStateHandler[THC_ST_CALL_CONFIRMED][HC_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleDisconnectReq;
  callStateHandler[THC_ST_CALL_CONFIRMED][HC_REMOTE_CHANN_SELECT - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMobRemoteChannSelected;

  callStateHandler[THC_ST_MM_CONN_REQ][CC_MM_CONNECTION_CNF - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMMEstCnf;
  callStateHandler[THC_ST_MM_CONN_REQ][CC_MM_CONNECTION_REJ - CC_INTERNAL_CALL_EVENT_BASE]
    = &CCHalfCall::handleMMEstRej;

  // ---------------------------- EXTERNAL HANDOVER -----------------------------
  // ext-HO <chenj:05-29-01>
  ehoCallStateHandler[EHO_ST_NULL][OHC_EHO_LOCAL_TERM_ADDRESS - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobTermAddress;

  ehoCallStateHandler[EHO_ST_OFFERING_CALL][OHC_EHO_REMOTE_CALL_PROGRESS - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobCallProgress;
  ehoCallStateHandler[EHO_ST_OFFERING_CALL][HC_EHO_REMOTE_CHANN_SELECT - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteChannSelected;
  ehoCallStateHandler[EHO_ST_OFFERING_CALL][OHC_EHO_REMOTE_ANSWER - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteAnswer;
  ehoCallStateHandler[EHO_ST_OFFERING_CALL][HC_EHO_REMOTE_CONNECTION_CONTROL - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteChannReady;
  ehoCallStateHandler[EHO_ST_OFFERING_CALL][HC_EHO_REMOTE_DISCONNECT_REQ - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteDisconnect;
  ehoCallStateHandler[EHO_ST_OFFERING_CALL][HC_EHO_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleHoCallTimeout;

  ehoCallStateHandler[EHO_ST_ACTIVE][HC_EHO_LOCAL_DISCONNECT_REQ - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobLocalDisconnect;
  ehoCallStateHandler[EHO_ST_ACTIVE][HC_EHO_REMOTE_DISCONNECT_REQ - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteDisconnect;
  ehoCallStateHandler[EHO_ST_ACTIVE][HC_EHO_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleHoCallTimeout;

  // ext-HO <xxu:06-07-01>
  ehoCallStateHandler[HC_ST_FREE][THC_EHO_REMOTE_TERM_ADDRESS - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteTermAddress;
  ehoCallStateHandler[EHO_ST_CONFIRMED][HC_EHO_LOCAL_PROTOCOL_TIMEOUT - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleHoCallTimeout;
  ehoCallStateHandler[EHO_ST_CONFIRMED][HC_EHO_REMOTE_DISCONNECT_REQ - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteDisconnect;
  ehoCallStateHandler[EHO_ST_CONFIRMED][HC_EHO_REMOTE_CHANN_SELECT - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteChannSelectedEi;
  ehoCallStateHandler[EHO_ST_ACTIVE][HC_EHO_REMOTE_CONNECTION_CONTROL - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteChannReady;
  ehoCallStateHandler[EHO_ST_ACTIVE][HC_EHO_REMOTE_DISCONNECT_REQ - CC_INTERNAL_EHO_CALL_EVENT_BASE]
    = &HOHalfCall::handleEHOMobRemoteDisconnect;

  // ext-HO <chenj:05-29-01>
  // external handout scenerio
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleExtPerformHOAck;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleExtPerformHONack;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchEndHandover;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoTimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;

  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_REQ_ACK][HAND_HO_ACCESS - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;  // Nothing is done for HO ACCESS
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoComplt;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_REQ_ACK][HAND_SRC_HO_FAIL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoFail;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_REQ_ACK][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchEndHandover;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_REQ_ACK][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoTimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_REQ_ACK][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;

  hoSrcHandler[HAND_ST_ANCH_EHO_COMPLT][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchEndHandover;
  hoSrcHandler[HAND_ST_ANCH_EHO_COMPLT][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostExtHoMobEvent;
  hoSrcHandler[HAND_ST_ANCH_EHO_COMPLT][HAND_PERFORM_HO_REQ - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostExtHoHandbackEvent;  // Handback event
  hoSrcHandler[HAND_ST_ANCH_EHO_COMPLT][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;


  // external handback (to anchor GP10) scenerio
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_HB_REQ][HAND_PERFORM_HO_REQ - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_HB_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchEndHandover;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_HB_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoTimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_HB_REQ][HAND_TRG_HO_REQ_ACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoReqAck;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_HB_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;

  hoSrcHandler[HAND_ST_ANCH_SND_EHB_REQ_ACK][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleNullEvent;
  hoSrcHandler[HAND_ST_ANCH_SND_EHB_REQ_ACK][HAND_SRC_HO_FAIL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHandbackFail;
  hoSrcHandler[HAND_ST_ANCH_SND_EHB_REQ_ACK][HAND_TRG_HO_COMPLT - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoComplt;
  hoSrcHandler[HAND_ST_ANCH_SND_EHB_REQ_ACK][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchEndHandover;
  hoSrcHandler[HAND_ST_ANCH_SND_EHB_REQ_ACK][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHOT104TimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_SND_EHB_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;


  // external handback (to another PLMN) scenerio   AND
  // internal handback from non-achor MNET to external MSC3
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_H3_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleExtPerformHOAck;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_H3_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleExtPerformHONack;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_H3_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchEndHandover;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_H3_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoTimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_H3_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;
  hoSrcHandler[HAND_ST_ANCH_PERF_EHO_H3_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;

  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK][HAND_HO_ACCESS - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;  // JOE: nothing is done with HANDOVER ACCESS
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoComplt;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK][HAND_SRC_HO_FAIL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHandbackFail;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchEndHandover;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoTimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;
  hoSrcHandler[HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;


  // external handback (to non-anchor GP10) scenerio
  hoSrcHandler[HAND_ST_ANCH_EHO_H3_VC_IP_ADDRESS_REQ][HC_VB_GET_VC_ADDRESS_RSP - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleVcAddressRsp;
  hoSrcHandler[HAND_ST_ANCH_EHO_H3_VC_IP_ADDRESS_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchTvbTimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_EHO_H3_VC_IP_ADDRESS_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;
  hoSrcHandler[HAND_ST_ANCH_EHO_H3_VC_IP_ADDRESS_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;


  hoSrcHandler[HAND_ST_ANCH_EHO_SND_H3_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHoAck;
  hoSrcHandler[HAND_ST_ANCH_EHO_SND_H3_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHoNack;
  hoSrcHandler[HAND_ST_ANCH_EHO_SND_H3_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchThoTimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_EHO_SND_H3_REQ][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoMobEvent;
  hoSrcHandler[HAND_ST_ANCH_EHO_SND_H3_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleNullEvent;
  hoSrcHandler[HAND_ST_ANCH_EHO_SND_H3_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;


  hoSrcHandler[HAND_ST_ANCH_EHO_RCV_H3_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchExtHoComplt;
  hoSrcHandler[HAND_ST_ANCH_EHO_RCV_H3_REQ_ACK][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchEndHandover;
  hoSrcHandler[HAND_ST_ANCH_EHO_RCV_H3_REQ_ACK][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchT103TimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_EHO_RCV_H3_REQ_ACK][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleNullEvent;
  hoSrcHandler[HAND_ST_ANCH_EHO_RCV_H3_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostExtHoRls;


  // ----------------------------------------------------------------------------


  // Source Handover Event Handlers
  hoSrcHandler[HAND_ST_IDLE][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchHoReqd;

  hoSrcHandler[HAND_ST_ANCH_VC_IP_ADDRESS_REQ][HC_VB_GET_VC_ADDRESS_RSP - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleVcAddressRsp;
  hoSrcHandler[HAND_ST_ANCH_VC_IP_ADDRESS_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;

  //RETRY<xxu:04-21-00> BEGIN
  hoSrcHandler[HAND_ST_ANCH_VC_IP_ADDRESS_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchTvbTimerExpiry;
  //RETRY<xxu:04-21-00> END

  //HO<xxu:01-28-00>
  hoSrcHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HC_VB_GET_VC_ADDRESS_RSP - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleVcAddressRsp;
  hoSrcHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
        = &CCAnchorHandover::handleAnchPostHoMobEvent;
  hoSrcHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoRls;
  hoSrcHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
        = &CCAnchorHandover::handleNullEvent;

  //RETRY<xxu:04-21-00> BEGIN
  hoSrcHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
        = &CCAnchorHandover::handleTrgToAnchAbortHb;
  hoSrcHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
        = &CCAnchorHandover::handleAnchTvbTimerExpiry;
  //RETRY<xxu:04-21-00> END

  hoSrcHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHoNack;
  hoSrcHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHoAck;
  hoSrcHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHoNack;
  hoSrcHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchThoTimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;

  //HO<xxu:01-28-00> H3
  //hoSrcHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
  //    = &CCAnchorHandover::handleTrgToAnchPerfHoNack;
  hoSrcHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoRls;
  hoSrcHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHoAck;
  hoSrcHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHoNack;
  hoSrcHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchThoTimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoMobEvent;
  hoSrcHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleNullEvent;

  //RETRY<xxu:04-27-00> BEGIN
  hoSrcHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
        = &CCAnchorHandover::handleTrgToAnchAbortHb;
  //RETRY<xxu:04-27-00> END

  hoSrcHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHoNack;
  hoSrcHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_SRC_HO_FAIL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchHoFail;
  hoSrcHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHoNack;
  hoSrcHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchCompltHo;
  hoSrcHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchT103TimerExpiry;
  // Race condition
  hoSrcHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleNullEvent;

  hoSrcHandler[HAND_ST_ANCH_HO_COMPLT][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoRls;
  hoSrcHandler[HAND_ST_ANCH_HO_COMPLT][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoMobEvent;
  hoSrcHandler[HAND_ST_ANCH_HO_COMPLT][HAND_PERFORM_HO_REQ - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchPerfHbReq;

  hoSrcHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoRls;
  hoSrcHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoMobEvent;

  //HO<xxu:02-01-00> 
  hoSrcHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleNullEvent; 

  hoSrcHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_TRG_HO_REQ_ACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchHoReqAck;
  hoSrcHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_TRG_HO_REQ_NACK - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchHoReqNack;
  hoSrcHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchAbortHb;

  hoSrcHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoRls;
  hoSrcHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoMobEvent;

  //HO<xxu:02-01-00> 
  hoSrcHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleNullEvent; 

  hoSrcHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_TRG_HO_COMPLT - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchHoComplt;
  hoSrcHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchAbortHb;
  hoSrcHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchT104TimerExpiry;

  //RETRY<xxu:04-27-00> BEGIN
  hoSrcHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleAnchPostHoRls;
  //RETRY<xxu:04-27-00> END

  //HO<xxu:01-28-00> H3 handle RR->A or B'->A HoComplt or HAND_RLS_CALL due to timeout
  //hoSrcHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
  //    = &CCAnchorHandover::handleTrgToAnchPerfHoNack;
  hoSrcHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoRls;
  hoSrcHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchCompltHo;
  hoSrcHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleTrgToAnchAbortHb; //H3 or HB failed at MSC-B
  hoSrcHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchT103TimerExpiry;
  hoSrcHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCAnchorHandover::handleAnchPostHoMobEvent;
  hoSrcHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
      = &CCAnchorHandover::handleNullEvent;

  
  // Target Handover Event Handlers
  hoTrgHandler[HAND_ST_IDLE][HAND_PERFORM_HO_REQ - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgPerfHoReq;

  hoTrgHandler[HAND_ST_TRG_RCV_HO_REQ][HAND_TRG_HO_REQ_ACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleTrgHoReqAck;
  hoTrgHandler[HAND_ST_TRG_RCV_HO_REQ][HAND_TRG_HO_REQ_NACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleTrgHoReqNack;
  hoTrgHandler[HAND_ST_TRG_RCV_HO_REQ][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgAbortHo;

  hoTrgHandler[HAND_ST_TRG_SND_HO_REQ_ACK][HAND_TRG_HO_COMPLT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleTrgHoComplt;
  hoTrgHandler[HAND_ST_TRG_SND_HO_REQ_ACK][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgAbortHo;
  hoTrgHandler[HAND_ST_TRG_SND_HO_REQ_ACK][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleTrgT204TimerExpiry;
  
  hoTrgHandler[HAND_ST_TRG_HO_COMPLT][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgAbortHo;
  hoTrgHandler[HAND_ST_TRG_HO_COMPLT][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleTrgHoReqd;

  hoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgAbortHo;
  hoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgPerfHbAck;
  hoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgPerfHbNack;
  hoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleTrgTshoTimerExpiry;
  hoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleNullEvent;

  hoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgAbortHo;
  hoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_SRC_HO_FAIL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleTrgHoFail;
  hoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgPerfHbNack;
  hoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleAnchToTrgCompltHb;
  hoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleTrgT203TimerExpiry;
  // Race condition
  hoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCTargetHandover::handleNullEvent;

    //ext-HO <xxu:06-07-01> : External Handover--Handin case handlers
  extHoTrgHandler[HAND_ST_IDLE][HAND_PERFORM_HO_REQ - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPerfHoReq;

  extHoTrgHandler[HAND_ST_TRG_RCV_HO_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_TRG_RCV_HO_REQ][HAND_TRG_HO_REQ_ACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgHoReqAck;
  extHoTrgHandler[HAND_ST_TRG_RCV_HO_REQ][HAND_TRG_HO_REQ_NACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgHoReqNack;
  
  extHoTrgHandler[HAND_ST_TRG_SND_HO_REQ_ACK][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_TRG_SND_HO_REQ_ACK][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT204TimerExpiry;
  extHoTrgHandler[HAND_ST_TRG_SND_HO_REQ_ACK][HAND_HO_ACCESS - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgHoAccess;
  extHoTrgHandler[HAND_ST_TRG_SND_HO_REQ_ACK][HAND_TRG_HO_COMPLT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgHoComplt;
  
  extHoTrgHandler[HAND_ST_TRG_HO_COMPLT][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_TRG_HO_COMPLT][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT204TimerExpiry;
  extHoTrgHandler[HAND_ST_TRG_HO_COMPLT][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgMmRelInd;
  extHoTrgHandler[HAND_ST_TRG_HO_COMPLT][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;

  //ext-HO <xxu:06-07-01> : External Handover postHandin--Handback anchor GP->PLMN
  extHoTrgHandler[HAND_ST_TRG_HO_COMPLT][HAND_SRC_HO_REQD - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgHoReqd;

  extHoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPerfHbAck;
  extHoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPerfHbNack;
  extHoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT211TimerExpiry; //formerly Tsho timer
  extHoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgMmRelInd;
  extHoTrgHandler[HAND_ST_TRG_SND_HB_REQ][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;

  extHoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_SRC_HO_FAIL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgHoFail;
  extHoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgCompltHb;
  extHoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT204TimerExpiry; //formerly T203 timer
  extHoTrgHandler[HAND_ST_TRG_RCV_HB_REQ_ACK][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleNullEvent;

  //ext-HO <xxu:06-07-01> : External Handover postHandin--Handover to another GP
  extHoTrgHandler[HAND_ST_ANCH_VC_IP_ADDRESS_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_VC_IP_ADDRESS_REQ][HC_VB_GET_VC_ADDRESS_RSP - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleVcAddressRsp;
  extHoTrgHandler[HAND_ST_ANCH_VC_IP_ADDRESS_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchTvbTimerExpiry;
  extHoTrgHandler[HAND_ST_ANCH_VC_IP_ADDRESS_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgMmRelInd;
  extHoTrgHandler[HAND_ST_ANCH_VC_IP_ADDRESS_REQ][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;
  
  extHoTrgHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgMmRelInd;
  extHoTrgHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT202TimerExpiry;  //interBSS handover under MSC-B
  extHoTrgHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPerfHoAck;
  extHoTrgHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPerfHoNack;
  extHoTrgHandler[HAND_ST_ANCH_SND_HO_REQ][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;
    
  extHoTrgHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT202TimerExpiry;  //interBSS handover under MSC-B
  extHoTrgHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchCompltHo;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_SRC_HO_FAIL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchHoFail;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HO_REQ_ACK][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;

  extHoTrgHandler[HAND_ST_ANCH_HO_COMPLT][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_HO_COMPLT][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchPostHoRls;
  extHoTrgHandler[HAND_ST_ANCH_HO_COMPLT][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;
  extHoTrgHandler[HAND_ST_ANCH_HO_COMPLT][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent;
      
  //ext-HO <xxu:06-07-01> : External Handover postHandin-->Handback nonanchor GP->PLMN
  extHoTrgHandler[HAND_ST_ANCH_HO_COMPLT][HAND_PERFORM_HO_REQ - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPerfHbReq;

  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchPostHoRls;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchAbortHb;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPerfHbAck;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPerfHbNack;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT211TimerExpiry; //formerly Tsho timer
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;
  
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ_ACK][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ_ACK][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT204TimerExpiry; //formerly T203 timer
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ_ACK][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchHbFail;   //revert to old cell case
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ_ACK][HAND_HO_ACCESS - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchHoAccess;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgCompltHb;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ_ACK][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleNullEvent;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ_ACK][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent;
  
  //ext-HO <xxu:06-07-01> : External Handover postHandin-->Handback nonanchor GP->anchor GP
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchPostHoRls;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchAbortHb;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_TRG_HO_REQ_ACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchHoReqAck;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_TRG_HO_REQ_NACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchHoReqNack;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_TRG_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT202TimerExpiry; //formerly T203 timer
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;
  extHoTrgHandler[HAND_ST_ANCH_RCV_HB_REQ][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent;
  
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT202TimerExpiry; //interBSS handover under MSC-B
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_TRG_HO_COMPLT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchHoComplt;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchHbFail;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;
  extHoTrgHandler[HAND_ST_ANCH_SND_HB_REQ_ACK][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent;
    
  //ext-HO <xxu:06-07-01> : External Handover postHandin-->Handover to 3rd within MNET
  extHoTrgHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchPostHoRls;
  extHoTrgHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchTvbTimerExpiry;
  extHoTrgHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchAbortHb;
  extHoTrgHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;
  extHoTrgHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent;
  extHoTrgHandler[HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ][HC_VB_GET_VC_ADDRESS_RSP - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleVcAddressRsp;
  
  extHoTrgHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchPostHoRls;
  extHoTrgHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT202TimerExpiry;  //formerly Tho timer
  extHoTrgHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchAbortHb;
  extHoTrgHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_PERFORM_HO_ACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPerfHoAck;
  extHoTrgHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_PERFORM_HO_NACK - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPerfHoNack;
  extHoTrgHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;
  extHoTrgHandler[HAND_ST_ANCH_SND_H3_REQ][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent;
  
  extHoTrgHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_END_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgEndHandover;
  extHoTrgHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_RLS_CALL - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchPostHoRls;
  extHoTrgHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_SRC_TIMER_EXPIRY - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgT202TimerExpiry;  //formerly Tho timer
  extHoTrgHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchAbortHb;
  extHoTrgHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_COMPLT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchCompltHo;
  extHoTrgHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_ABORT_HO - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchHbFail;  
  extHoTrgHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_POSTHO_HOA_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent;
  extHoTrgHandler[HAND_ST_ANCH_RCV_H3_REQ_ACK][HAND_POSTHO_MOB_EVENT - HAND_SRC_EVENT_BASE]
    = &CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent;
  DBG_LEAVE();

}

// Data Structure Initialization for an individual call task
void callDataInit(void)
{
  DBG_FUNC("callDataInit", CC_LAYER);
  DBG_ENTER();

  // Initialize the Message Handler Member function table.
  msgHandlerInit();


  // create the function pointers for use in each session.
  callTimerExpiry = (FUNCPTR) callTimerExpiryAction;

  DBG_LEAVE();

}

// Data Structure Initialization
void ccInitialize(void)
{
  DBG_FUNC("ccInitialize", CC_LAYER);
  DBG_ENTER();

  ccOamDataInit();
  callDataInit();
  
  short i;
  for(i=0; i< CC_MAX_CC_CALLS;i++)
    {
      // First create the semaphore and hold for initialize
      ccSession[i].semId = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
      
      // Take the semaphore
      semTake(ccSession[i].semId, WAIT_FOREVER);
      
      // Initialize the parent session first before the Calls.
      // Particularly the callIndex below is important, as it is used in the HC
      ccEntryInit(&ccSession[i]);
      ccSession[i].callIndex = i;
      ccSession[i].taskId = NULL;
      
      // create two way links (pointers) between the session and half call
      ccSession[i].sessionHandler = new CCSessionHandler(&ccSession[i]);
      
    }
  DBG_LEAVE();

}

void sessionDataInit(int i)
{
  DBG_FUNC("sessionDataInit", CC_LAYER);
  DBG_ENTER();

  semTake(ccSession[i].semId, WAIT_FOREVER);
  ccSession[i].safetyTimer->cancelTimer();

  //CAUSE<xxu:03-27-00> Begin
  //ccSession[i].sessionHandler->cleanup(CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER, TRUE);
  ccSession[i].sessionHandler->cleanup(CNI_RIL3_CAUSE_TEMPORARY_FAILURE, TRUE);

  //CAUSE<xxu:03-27-00> Begin

  // semFlush(ccSession[i].semId);
  handleReleaseSession(&ccSession[i]);
  semGive(ccSession[i].semId);
  DBG_LEAVE();
}

void ccCallDataInit(void)
{
  DBG_FUNC("ccCallDataInit", CC_LAYER);
  DBG_ENTER();
  short i;
  for(i=0; i< CC_MAX_CC_CALLS;i++)
    {
      sessionDataInit(i);
    }
  DBG_LEAVE();
}

int findCallUsingCallHandle( VOIP_CALL_HANDLE callHandle)
{
  DBG_FUNC("findCallUsingCallHandle", CC_LAYER);
  DBG_ENTER();

  short i;
  for(i=0; i< CC_MAX_CC_CALLS;i++)
    {
      if (!(ccSession[i].free))
        {
          if ((ccSession[i].sessionHandler)->findHandle(callHandle ))
            {
              DBG_LEAVE();
              return (i);
            }
        }
    }
  DBG_LEAVE();

  return (CC_MAX_CC_CALLS);
  
}

// entry function for the CC Module (VxWorks Task)
int ccMain(void)
{
  //ext-HO <xxu: 05-16-01>
  //Make sure external handover trace in place
  DBG_ENTERei();
  DBG_ENTEReo();
  DBG_ENTERho();

  // Semaphores are defined and also taken here.
  ccInitialize();

  // Register for dynamic changes in timer values for call configuration.
  // Moved here so that the message queue is initialized correctly.
  callOamTrapRegister();

  // Initialize the Handover RTP Pool
  if (!InitializeHandoverRtpPool() )
    {
      // failed to initialize Handover RTP Pool. Abort the task.
      printf("CC MAIN FATAL ERROR: failed to create Handover RTP Pool\n" );
      return ERROR;
    }

  // Initialize Viper to Viper channel
  // InitializeViperChannel(TRUE); // moved to SysCommand_VChan

  // create all the call tasks.
  
  short callId;
  for(callId=0; callId< CC_MAX_CC_CALLS;callId++)
    {
      if ( (ccSession[callId].taskId = callInit(callId)) == ERROR)
        {
          // Call task creation failure. No point to continue. 
          // Abort the CC task.
          return ERROR;
        }

      ccSession[callId].safetyTimer
        = new JCCTimer((FUNCPTR) CCTimerExpiryAction, 
                       ccSession[callId].taskId); 
          
      semGive(ccSession[callId].semId);

    }
  
  // local message queue buffer
  CCMMInputMsg_t              inMsg;

  // enter infinite loop to wait for message and process message from MS 
  // or other layer-3 modules
  ccTaskObj.JCTaskEnterLoop();
  while(true)
    {
      // 

      if (msgQReceive(ccMsgQId,
                      (char *) &inMsg, 
                      CC_MAX_MSG_LENGTH, 
                      WAIT_FOREVER)
          == ERROR)
        {
          // failed to receive message. VxWorks error. Abort the task.
          printf("CC MAIN FATAL ERROR: failed to receive on CC Message Queue\n" );
          return ERROR;
        }
         
      DBG_TRACE ("M%d->CC MAIN LOG: Task \"%s\" Received a message\n",
                  inMsg.intraL3Msg.module_id, taskName(taskIdSelf()));

      // check for BTS Operational first 
          
      if (ccOldBTSState == VAL_bts_operationalState_disabled)
        {
          switch (inMsg.intraL3Msg.module_id) { 
          case MODULE_OAM:
            callOamProcessTrapMsg((TrapMsg *) &inMsg);
            break;
          default:
            DBG_WARNING("CC MAIN WARNING: BTS Operation Disabled. Ignoring new call request from (Module = %d)\n.",
                        inMsg.intraL3Msg.module_id);
          }
        }
      else
        {
          // action based on the sending module
          // 
          switch (inMsg.intraL3Msg.module_id) { 
          case MODULE_MM:
            {
              IntraL3Msg_t      *mmInMsg = &inMsg.intraL3Msg;
          
              DBG_TRACE("MM->CC MAIN LOG: Received Msg(%d) Prim(%d) entryId(%d)\n",  
                        mmInMsg->message_type,
                        mmInMsg->primitive_type, 
                        mmInMsg->entry_id);

              switch (mmInMsg->primitive_type) {
              case INTRA_L3_MM_EST_IND:
                {
                  T_CNI_RIL3_CM_SERVICE_TYPE cmServType =
                    mmInMsg->l3_data.servReq.cmServiceType.serviceType;

                  if ((cmServType != CNI_RIL3_CM_SERVICE_TYPE_MO_CALL_OR_PACKET_MODE) &&
                      (cmServType != CNI_RIL3_CM_SERVICE_TYPE_EMERGENCY_CALL        ) &&
                      (cmServType != CNI_RIL3_CM_SERVICE_TYPE_SUPPLEMENTARY_SERVICE ) &&
                      (cmServType != CNI_RIL3_CM_SERVICE_TYPE_SHORT_MESSAGE_SERVICE )   )
                    {
                      // This CM Service Type not allowed
                      // Clear the call - Send Rel Req to MM 
                      IntraL3Msg_t      mmOutMsg;
                      sendMM   (INTRA_L3_MM_REL_REQ,
                                INTRA_L3_NULL_MSG,
                                mmInMsg->entry_id,
                                &mmOutMsg);
                      break;
                    }

                  if ((callId = mobInCall2 (mmInMsg->entry_id)) != JCC_ERROR)
                  {
                      DBG_WARNING ("MM->CC MAIN WARNING: recv MM-EST-IND but CallTask already allocated sessionId(%d)\n",
						callId);
                  }
                  else if ( initNewMSConn(mmInMsg->entry_id, callId) == JCC_ERROR )
                  {
                      DBG_WARNING("MM->CC MAIN WARNING: New Connection could not be initialized\n");

                      // Clear the call - Send Rel Req to MM 
                      IntraL3Msg_t      mmOutMsg;
                      sendMM   (INTRA_L3_MM_REL_REQ,
                                INTRA_L3_NULL_MSG,
                                mmInMsg->entry_id,
                                &mmOutMsg);
                      break;
                  }
                          
                  //
                  semTake(ccSession[callId].semId, WAIT_FOREVER);
                          
                  // Find the ludbIndex for the Originating IMSI
                  if (JCC_ERROR ==
                      (ccSession[callId].sessionHandler->ludbIndex 
                       = ludbGetEntryByMobileId(mmInMsg->l3_data.servReq.mobileId)))
                    {
                      // We need to take care of the case where we 
                      // retrieve the profile when not present.
                      DBG_WARNING("MM->CC MAIN WARNING: Originating Mobile not registered in LUDB sessionId(%d)\n",
					     callId);
                                  
                      // Reset the IRT Table entry to point to the mother task's Queue
                      ccSession[callId].sessionHandler->resetIrt();

                      handleReleaseSession(&ccSession[callId]);
                      IntraL3Msg_t      mmOutMsg;
                      sendMM   (INTRA_L3_MM_REL_REQ,
                                INTRA_L3_NULL_MSG,
                                mmInMsg->entry_id,
                                &mmOutMsg);
                      semGive(ccSession[callId].semId);

                      break;
                    }

                  //CDR <xxu:08-23-00>
                  memcpy(&ccSession[callId].sessionHandler->mobileId,
                         &mmInMsg->l3_data.servReq.mobileId,
                         sizeof(T_CNI_RIL3_IE_MOBILE_ID));

                  // Store Mobile Classmark Info <chenj:07-24-01>
                  ccSession[callId].sessionHandler->classmark2Info = 
                         mmInMsg->l3_data.servReq.classmark2;

                  // Setup the Session for Mobile Origination
                  if (!(ccSession[callId].sessionHandler->setupOrig(mmInMsg->entry_id, cmServType)))
                    {
                      handleReleaseSession(&ccSession[callId]);
                      IntraL3Msg_t      mmOutMsg;
                      sendMM   (INTRA_L3_MM_REL_REQ,
                                INTRA_L3_NULL_MSG,
                                mmInMsg->entry_id,
                                &mmOutMsg);
                      semGive(ccSession[callId].semId);
                      break;
                    }
                          
                  // Give up the originating session semaphore.
                  semGive(ccSession[callId].semId);
                  break;
                }
                
              default:
                DBG_WARNING("MM->CC MAIN WARNING: Unexpected Msg(%d) Prim(%d)MM.\n",  
                             mmInMsg->message_type, mmInMsg->primitive_type);
              }
                  
              break;
            }
          case MODULE_CC:
            {
              IntraL3Msg_t      *ccInMsg = &inMsg.intraL3Msg;

              DBG_TRACE("CC->CC MAIN LOG: Received msg(%d) callLeg(%d) signature(%d))\n",
                         ccInMsg->message_type, ccInMsg->entry_id, ccInMsg->signature);

              if (ccInMsg->message_type != INTRA_L3_TIMEOUT_MSG)
                {
                  DBG_ERROR("CC->CC MAIN LOG: Unexpected (Msg Type = %d) from CC.\n",  
                            ccInMsg->message_type);
                  break;
                }
                  
              // Inform for debugging
              // Action - Kill and restart the call process?
              DBG_ERROR("CC->CC MAIN ERROR: Call running too long(infinite loop?) CallTaskId(%d)\n",
                         ccInMsg->entry_id);
              break;
            }

          case MODULE_H323:
			{
              IntraL3Msg_t *h323InMsg    = &inMsg.intraL3Msg  ;
              short         ludbIndex    = h323InMsg->entry_id;
              bool          isFirstSetup = true               ;
    
			  DBG_TRACE("H323->CC MAIN LOG: Received a VOIP message {msgType=%d, h323Hndl=%p, entryId=%d}\n",
                         h323InMsg->message_type,
                         (int)(h323InMsg->call_handle),
				         h323InMsg->entry_id);
    
              switch (h323InMsg->message_type) {
              case INTRA_L3_CC_Q931_SETUP:
                   if ((callId = ccIsHandoverCallSetup(h323InMsg)) != JCC_ERROR)
				   {
				       DBG_TRACEei("{\nMNETeiTRACE(ccMain): received ext-handin SETUP {msgType=%d, callId=%d, h323Hndl=%p, entryId=%d\n}\n",
                                    h323InMsg->message_type,
					     			callId,
                                    (int)(h323InMsg->call_handle),
						            h323InMsg->entry_id);

					   //Proceed with setting up the handover call leg
					   if ( !ccSession[callId].sessionHandler->setupTermExtHo(h323InMsg) )
					   {
                           //Failed to setup the handover call leg, report the failure and abort ext-handin
						   DBG_ERRORei("{\nMNETeiERROR(ccMain): setupTermExtHo failed {msgType=%d, callId=%d, h323Hndl=%p, entryId=%d\n}\n",
                                        h323InMsg->message_type,
										callId,
                                        (int)(h323InMsg->call_handle),
								        h323InMsg->entry_id);

						   DBG_CCDATA(callId);

                           //Give up this ext-handin call leg setup and notify VOIP task
                           if (!sendH323DisconnectMsgExtHo(h323InMsg->call_handle,
                                                           CNI_RIL3_CAUSE_CALL_REJECTED) )
						   {
                               DBG_ERRORei("{\nMNETeiERROR(ccMain): sendH323 DISCONNECT failed {msgType=%d, callId=%d, h323Hndl=%p, entryId=%d, ccCause=%d\n}\n",
								            h323InMsg->message_type,
										    callId,
                                            (int)(h323InMsg->call_handle),
										    h323InMsg->entry_id,
						                    (int)CNI_RIL3_CAUSE_CALL_REJECTED);
						   }

						   //Cleanup
						   sessionDataInit(callId);

						   break;
						}

					    //Proceed the handin callleg setup to its responsible call task
					    
					    ccSession[callId].sessionHandler->hoCallHandle = h323InMsg->call_handle;

					    if (ERROR == msgQSend( ccSession[callId].msgQId,
                                               (char *) h323InMsg, 
                                               sizeof(struct IntraL3Msg_t), 
                                               NO_WAIT,
                                               MSG_PRI_NORMAL
                                             ) )
						{
                            DBG_ERRORei("{\nMNETeiERROR(ccMain): msgQSend error {QID=%p, msgType=%d, callId=%d, h323Hndl=%p, entryId=%d\n}\n", 
                                         (int)ccSession[callId].msgQId,
										 h323InMsg->message_type,
										 callId,
										 (int)(h323InMsg->call_handle),
									     h323InMsg->entry_id
										);

							DBG_CCDATA(callId);
					
							//vxWorks API failed, abort the the handin effort.
                            if (!sendH323DisconnectMsgExtHo( h323InMsg->call_handle,
                                                             CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER ))                        
							{
                                DBG_ERRORei("{\nMNETeiERROR(ccMain): sendH323 DISCONNECT failed {msgType=%d, callId=%d, h323Hndl=%p, entryId=%d, ccCause=%d\n}\n",
								             h323InMsg->message_type,
										     callId,
                                             (int)(h323InMsg->call_handle),
										     h323InMsg->entry_id,
		                                     (int)CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER);
							}

							//Cleanup
						    sessionDataInit(callId);

					        break;
						}

						break;

				  }  //ext-HO <xxu:05-07-10> END

				  else
				  {
                    if (!ludbGetSubscInfoExists(ludbIndex))
                    {

                      DBG_ERROR("H323->CC MAIN ERROR: VOIP passed an invalid ludb index(%d) Msg(%d)\n",
                                ludbIndex, h323InMsg->message_type);

                      //CDR <xxu:08-23-00>                              
                      if (!sendH323DisconnectMsg(h323InMsg,
                                        CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT,
                                        CNI_RIL3_CAUSE_CALL_REJECTED))
                        {
                          DBG_ERROR("H323->CC MAIN ERROR: Failed to send Disconnect cdrCause(%d) ccCause(%d)\n",
						 (int)CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT,
						 (int)CNI_RIL3_CAUSE_CALL_REJECTED);
                        }
                                  
                      break;
                    }

                  // Check if the user is already in a call. Entry id has the ludb index
                  if ((callId = mobInCall(ludbIndex))
                      != JCC_ERROR)
                    {
                      // Need to send this event to the existing call.
                      // Use the current call Id.
                      isFirstSetup = false;
                    }

                  // Connection Initialization - 
                  // New MS MM Connection that needs a CC level service
                  else if ((callId = ccCallAlloc ())
                           == JCC_ERROR)
                    {
                        DBG_WARNING("H323->CC MAIN WARNING: No CC Connections left for allocation Msg(%d)\n",
						 h323InMsg->message_type);
    
                        //CDR <xxu:08-23-00> 
                        if (!sendH323DisconnectMsg( h323InMsg,
                                                  CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT,
                                                  CNI_RIL3_CAUSE_SWITCHING_EQUIPMENT_CONGESTION ))
                        {

                          DBG_ERROR("H323->CC MAIN ERROR: Failed to send Disconnect cdrCause(%d) ccCause(%d)\n",
						 (int)CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT,
						 (int)CNI_RIL3_CAUSE_SWITCHING_EQUIPMENT_CONGESTION);
                        }

                      
                      break;
                    }

                  // first and the concecutive MT call setups
                  semTake(ccSession[callId].semId, WAIT_FOREVER);

                  //3TY<xxu: 04-09-99> race condition
                  ccSession[callId].free                      = false    ;
                  ccSession[callId].sessionHandler->ludbIndex = ludbIndex;
                          
                  // Setup the Session for Mobile Termination 
                  if (!ccSession[callId].sessionHandler->setupTerm( h323InMsg->call_handle ,
                                                                    (int)h323InMsg->H323CRV,
                                                                    0                      ,
                                                                    isFirstSetup          ))
                    {
                      // Do not proceed with the termination attempt.
                      // MM Setup in progress initiated by the SMS-PP or another call setup
                      // Busy and Call Waiting cases are handled inside setupTerm.

                      DBG_TRACE("H323->CC MAIN TRACE: setupTerm failed Msg(%d) h323CallHandle(%p) sessionId(%d) ludbId(%d)\n",
                         h323InMsg->message_type,
                        (int)(h323InMsg->call_handle), callId,
				 h323InMsg->entry_id);

                      //CDR <xxu:08-23-00>
                      if (!sendH323DisconnectMsg( h323InMsg,
                                                  CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT,
                                                  CNI_RIL3_CAUSE_USER_BUSY))
                        {
                          DBG_ERROR("H323->CC MAIN ERROR: Failed to send Disconnect cdrCause(%d) ccCause(%d)\n",
						 (int)CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT,
						 (int)CNI_RIL3_CAUSE_USER_BUSY);
                        }

                      // Give up the terminating session semaphore.
                      semGive(ccSession[callId].semId);
                          
                      break;
                    }

                  // Give up the terminating session semaphore.
                  semGive(ccSession[callId].semId);

                  if (ERROR == msgQSend( ccSession[callId].msgQId,
                                         (char *) h323InMsg, 
                                         sizeof(struct  IntraL3Msg_t), 
                                         NO_WAIT, 		//PR1553 <xxu> WAIT_FOREVER,
                                         MSG_PRI_NORMAL
                                         ) )
                    {

                      DBG_ERROR("H323->CC MAIN ERROR: sendCC msgQSend (QID = %p) error\n ", 
                                (int)ccSession[callId].msgQId);

                      //CAUSE<xxu:03-21-00> BEGIN
                      //Major problem with the queue. Need to release the call!
                      //sessionDataInit(callId);

                      //CDR <xxu:08-23-00>
                      if (!sendH323DisconnectMsg( h323InMsg,
                                                  CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT,
                                                  CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER ))
                        {
                          DBG_ERROR("H323->CC MAIN ERROR: Failed to send Disconnect cdrCause(%d) ccCause(%d)\n",
						 (int)CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT,
						 (int)CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER);
                        }

                      sessionDataInit(callId);

                      break;
                    }

                  break;
                }

              case INTRA_L3_SMS_MESSAGE:
                { 
                  /* Expecting MNSMS-EST Req only. MNSMS-ABORT Req should be discarded. 
                     All the other messages will be rejected. */

                  SMS_VBLINK_MSGTYPE MsgType = (SMS_VBLINK_MSGTYPE)h323InMsg->l3_data.smsVbMsg.hdr.msgType;

                  if (MsgType == MNSMS_ABORT_Req || MsgType == MNSMS_REL_Req)
                  {
                      // Late Abort Mobile is not involved in any activities. Do nothing
                      DBG_TRACE ("Late MNSMS-ABORT Req ref. number %d received from ViperBase.\n",
                                 h323InMsg->l3_data.smsVbMsg.hdr.refNum);
                      break;
                  }

                  if (MsgType != MNSMS_EST_Req)
                  {
                      // Late SMS message other than Abort or Release. Need to be rejected.
                      DBG_TRACE ("Late MNSMS message type %d, ref. number %d received from ViperBase.\n",
                                 MsgType, h323InMsg->l3_data.smsVbMsg.hdr.refNum);

                      if (!smsSendVbLinkRelease(ludbIndex, h323InMsg->l3_data.smsVbMsg.hdr.refNum)) 
                      {
                          DBG_ERROR("SMS->VOIP Error : Failed to reject SMS transaction.\n");
                      }
                      break;
                  }

                  if (!ludbGetSubscInfoExists (ludbIndex))
                  {  // received ludb index is not valid
                     DBG_ERROR ("SMS->VOIP Error : Message contains invalid LUDB index =%d\n",
                                ludbIndex);

                     /* This error could only happend due to an implementation error in VB<->VC.
                        Send MNSMS-ERROR Ind to the originator.
                        This will require allocate Call task, initialize it, send reject and
                        reliase. Very low chance for a potential problem: could be no availiable 
                        CC. */

                      if (!smsSendVbLinkRelease(ludbIndex, h323InMsg->l3_data.smsVbMsg.hdr.refNum,
                                                       CNI_RIL3_RP_CAUSE_UNIDENTIFIED_SUBSCRIBER)) 
                      {
                          DBG_ERROR("SMS->VOIP Error: Failed to reject SMS transaction.\n");
                      }
                      break;
                  }

                  if ((callId = mobInCall(ludbIndex)) != JCC_ERROR)
                  {
                      // Need to send this event to the existing call.
                      // Use an existing call Id.
                      isFirstSetup = false;
                  }
                  // Connection Initialization - 
                  // New MS MM Connection that needs CM level service
                  else if ((callId = ccCallAlloc ()) == JCC_ERROR)
                  {
                      // Error - No Connections left. 
                      if (!smsSendVbLinkRelease(ludbIndex, h323InMsg->l3_data.smsVbMsg.hdr.refNum,
                                                           CNI_RIL3_RP_CAUSE_CONGESTION          )) 
                      {
                          DBG_ERROR("SMS->VOIP Error: Failed to reject SMS transaction.\n");
                      }
                      
                      DBG_WARNING ("CC Warning: No Connections left.\n");
                      break;
                  }

                  // first and the concecutive MT SMS setups
                  semTake (ccSession[callId].semId, WAIT_FOREVER);

                  ccSession[callId].free                      = false    ;
                  ccSession[callId].sessionHandler->ludbIndex = ludbIndex;

                  // Setup the Session for SMS Mobile Termination
                  // New function (setupSmsTerm) or modify the existing one???
                  if (!ccSession[callId].sessionHandler->setupTerm(VOIP_NULL_CALL_HANDLE,
                                                                   0                    , //CDR <xxu:08-24-00>
                                                                   0                    , //CDR <xxu:08-24-00>
                                                                   isFirstSetup         ))
                  {
                      // Do not proceed with the termination attempt.
                      // 1.MM MT Setup in progress initiated by SMS-PP or MT call setup
                      // 2.MS is involved in ongoing MT SMS transfer

                      if (!smsSendVbLinkRelease (ludbIndex, h323InMsg->l3_data.smsVbMsg.hdr.refNum,
                                                            CNI_RIL3_RP_CAUSE_CONGESTION          )) 
                      {
                          DBG_ERROR ("SMS->VOIP Error: Failed to reject SMS transaction.\n");
                      }

                      // Give up the terminating session semaphore.
                      semGive (ccSession[callId].semId);

                      DBG_TRACE ("MT SMS attempt is rejected due to busy conditions.\n");                      
                      break;
                  }

                  // Give up the terminating session semaphore.
                  semGive(ccSession[callId].semId);

                  if (ERROR == msgQSend (ccSession[callId].msgQId    ,
                                         (char *) h323InMsg          , 
                                         sizeof(struct  IntraL3Msg_t), 
                                         NO_WAIT			   ,  	//PR1553 <xxu> WAIT_FOREVER
                                         MSG_PRI_NORMAL              ))
                  {
                      // Major problem with the queue. Need to release the call
                      if (!smsSendVbLinkRelease(ludbIndex, h323InMsg->l3_data.smsVbMsg.hdr.refNum)) 
                      {
                          DBG_ERROR ("SMS->VOIP Error: Failed to reject SMS transaction.\n");
                      }

                      DBG_ERROR ("CC->CC Call(SMS) Error: sendCC msgQSend (QID = %p) error\n ", 
                                 (int)ccSession[callId].msgQId);

                      sessionDataInit (callId);

                      break;
                  }
                }
                break;

              case INTRA_L3_CC_VC_UPDATE_INFO:
                {
                   DBG_TRACE ("H323->CC MAIN LOG: Received VC Update Msg(%d) for this GP\n",  
                              h323InMsg->message_type);

                  if (h323InMsg->l3_data.statusVCMsg.status != VC_STATUS_DOWN)
                  {
                      // Unknown ViperCell Status
                       DBG_WARNING ("H323->CC MAIN WARNING: Received unknown ViperCell update status %d\n",  
                                   h323InMsg->l3_data.statusVCMsg.status);
                      break;
                  }

                  for (callId = 0; callId < CC_MAX_CC_CALLS; ++callId)
                  {
                    if (ccSession[callId].free           != True && 
                        ccSession[callId].sessionHandler != NULL   )
                    {
                        //GCELL<tyu:06-01-01> 
                        if (ccSession[callId].sessionHandler->handoverSession->hoGlobalCellId_.ci == 
                            h323InMsg->l3_data.statusVCMsg.btsID.cellId &&
                            ccSession[callId].sessionHandler->handoverSession->hoGlobalCellId_.lac == 
                            h323InMsg->l3_data.statusVCMsg.btsID.networkId)
                            // networkId is not in use
                        {
                            // This active CC session is involved in Handover with 
                            // the failed ViperCell
                            if (msgQSend (ccSession[callId].msgQId,
                                      (char *)h323InMsg       ,
                                      sizeof (IntraL3Msg_t)   ,
                                      NO_WAIT 		      , 		//PR1553 <xxu> WAIT_FOREVER
                                      MSG_PRI_NORMAL          ) == ERROR)
                            {
                                DBG_ERROR ("H323->CC MAIN ERROR: sendCC msgQSend (QID = %p) error Msg(%d)\n ", 
                                           (int)ccSession[h323InMsg->entry_id].msgQId,
							  h323InMsg->message_type);
                            } // Sending Msg
                        } // Compare cell ID
                    }  // Check Call session
                  }  // for loop
                  break;
                }

              default:

                // route messages that come with a null txn id through CC Main Task.
                if (h323InMsg->entry_id == VOIP_API_NULL_TXN_ID)
                  {
                    // Null value is received for transaction id.
                    // Check if there is a valid call with this call handle
                    if ((h323InMsg->entry_id = findCallUsingCallHandle( h323InMsg->call_handle))
                        == (CC_MAX_CC_CALLS))
                      {
                        DBG_WARNING("H323->CC MAIN WARNING: Late Msg. for (Handle = %p) with Null Transaction Id.\n",
                                    (int)h323InMsg->call_handle);

                      }
                    else
                      {
                        if (ERROR == msgQSend( ccSession[h323InMsg->entry_id].msgQId,
                                               (char *) h323InMsg, 
                                               sizeof(struct  IntraL3Msg_t), 
                                               NO_WAIT,				//PR1553 <xxu> WAIT_FOREVER,
                                               MSG_PRI_NORMAL
                                               ) )
                          {
                            DBG_ERROR("H323->CC MAIN ERROR: sendCC msgQSend (QID = %p) error Msg(%d)\n ", 
                                      (int)ccSession[h323InMsg->entry_id].msgQId,
						   h323InMsg->message_type);

                          }
                        else
                          {
                            DBG_WARNING("H323->CC MAIN WARNING: Msg(%d) for (Handle=%p) w/Null Tran. for (call=%d),(ludbId=%d)\n",
                                         h323InMsg->message_type,
                                        (int)h323InMsg->call_handle,
                                        h323InMsg->entry_id,
                                        ccSession[h323InMsg->entry_id].sessionHandler->ludbIndex);
                          }
                      }
                  }
              }
            }
            break;

          //ext-HO <xxu:05-16-01>
		  case MODULE_EXTHOA:
			{
			  InterHoaVcMsg_t *hoaInMsg = &inMsg.hoaCcMsg;
              DBG_TRACEei("{\nMNETeiTRACE(ccMain): Received Msg from HOA (msgType=%d)\n}\n",  
                           hoaInMsg->msgType);

              switch (hoaInMsg->msgType)
			  {
              case INTER_EHO_PERFORM_HANDOVER_REQUEST:
				{
                   // Handle external-handin scenario
                   if (JCC_ERROR == (callId=ccCallAlloc()) )
                   {
                      // Error - No Connections left. N need to send Handover Nack for now
                      // other side will timeout
                      DBG_WARNINGei("{\nMNETeiWARNING(ccMain): No Connections left in GP10\n}\n");
                      break;
                   }
              
                   //
                   semTake(ccSession[callId].semId, WAIT_FOREVER);
              
                   if (ERROR == msgQSend( ccSession[callId].msgQId,
                                          (char *) &inMsg, 
                                          sizeof(struct  VcCcMsg_t), 
                                          NO_WAIT,
                                          MSG_PRI_NORMAL
                                        ) )
                    {
                      // Need to release the call!
                      handleReleaseSession(&ccSession[callId]);

                      // no need to send Handover Nack for now. other side will timeout
                      DBG_ERRORei("{\nMNETeiERROR(ccMain): msgQSend failed! (QID=%p, callId=%d)\n}\n", 
                                   (int)ccSession[callId].msgQId, callId);
                    }

                    // Setup the Session for Mobile Handover 
                    ccSession[callId].sessionHandler->setupTargetHandoverExtHo();

                    // Give up the handover session semaphore.
                    semGive(ccSession[callId].semId);
                    break;
                }
			  
              default:
				{
      
                    DBG_ERRORei("{\nMNETeiWARNING(ccMain): received unexpected msg from HOA (msgType=%d)\n}\n",
                                hoaInMsg->msgType);
                    break;
				}

			}

            break;

            }

          case MODULE_EXT_VC:
            {
              InterVcMsg_t *vcInMsg = &inMsg.vcCcMsg.vcToVcMsg;
                  
              DBG_TRACE ("VC->CC MAIN LOG: Received Msg(%d) from another GP\n",  
                          vcInMsg->msgType);

              switch (vcInMsg->msgType) {
              case INTER_VC_CC_PERFORM_HANDOVER:
                {
                          
                  // Connection Initialization - 
                  // New Handover Connection that needs a CC level service
              
                  if ((callId = ccCallAlloc())
                      == JCC_ERROR)
                    {
                      // Error - No Connections left. 
                      // no need to send Handover Nack for now
                      // other side will timeout
                      DBG_WARNING("VC->CC MAIN WARNING: No Connections left for handover\n ");
                      break;
                    }
              
                  //
                  semTake(ccSession[callId].semId, WAIT_FOREVER);
              
                  if (ERROR == msgQSend( ccSession[callId].msgQId,
                                         (char *) &inMsg, 
                                         sizeof(struct  VcCcMsg_t), 
                                         NO_WAIT,		//PR1553 <xxu> WAIT_FOREVER,
                                         MSG_PRI_NORMAL
                                         ) )
                    {
                      // Need to release the call!
                      handleReleaseSession(&ccSession[callId]);

                      // no need to send Handover Nack for now
                      // other side will timeout
                      DBG_ERROR("VC->CC MAIN ERROR: sendCC msgQSend (QID = %p) sessionId(%d) error\n ", 
                              (int)ccSession[callId].msgQId, callId);
                    }

                  // Setup the Session for Mobile Handover 
                  ccSession[callId].sessionHandler->setupTargetHandover();

                  // Give up the handover session semaphore.
                  semGive(ccSession[callId].semId);
                  break;
                }

              default:
                DBG_ERROR("VC->CC MAIN ERROR: Unexpected (Msg Type = %d) from other ViperCell.\n",  
                           vcInMsg->msgType);
              }

              break;
            }

          case MODULE_OAM:
            TrapMsg *oamInMsg;
            oamInMsg = (TrapMsg*)&inMsg;
            DBG_TRACE("OAM->CC MAIN LOG: received an OAM msg(%d) tag(%x) valTyp(%d) val(%d)\n",
                       oamInMsg->msgType,
			     oamInMsg->mibTag,
			     oamInMsg->valType,
			     oamInMsg->val.varVal);

            callOamProcessTrapMsg((TrapMsg *) &inMsg);
            break;

          case MODULE_RM:
          case T_CNI_MD_ID:
          default:
            // log as a warning. let the user of the system take action based on other symptoms.
            DBG_WARNING("CC MAIN WARNING: (Late?) Msg. from unexpected Module, (Mod. Id = %d)(primTyp=%d) (sapi %d, entryId %d)\n",  
                        inMsg.intraL3Msg.module_id, inMsg.mdMsg.primitive_type,
                        inMsg.mdMsg.sapi,
                        inMsg.mdMsg.entry_id
                        );
          }
        }
    }
}

