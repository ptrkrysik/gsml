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
// File        : CCMobExtHnd.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : CC Mobile side External Event Handlers.
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "JCC/JCCLog.h"
#include "logging/VCLOGGING.h"

#include "stdio.h"

#include "sysLib.h"

//CH<xxu:11-11-99>
#include "csunew/csu_head.h"
 
// 
#include "CC/CCHalfCall.h"
#include "CC/CallConfig.h"

#include "CC/CallLeg.h"
#include "CC/CCSessionHandler.h"

#include "jcc/LUDBInstance.h"
#include "JCC/LUDBapi.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"

#include "rm/rm_head.h"
extern u8 rm_EntryIdToTrxSlot(T_CNI_IRT_ID entryId, u16 *gsmHandler);
extern bool callTrace;

const int CALL_CFNRY_TIMER = 300; // 5 seconds
const int CALL_WAITING_T3_TIMER = 1800; // 30 seconds

//BCT <xxu:08-11-00> BEGIN
extern int CALL_BCT_NUMBER_TVB;
//BCT <xxu:08-11-00> END


// *******************************************************************
// forward declarations.
// *******************************************************************

// Current State is THC_ST_CALL_CONFIRMED

JCCEvent_t
CCHalfCall::handleMobNoReply(void)
{
  DBG_FUNC("CCHalfCall::handleMobNoReply", CC_HALFCALL_LAYER);
  DBG_ENTER();

  //CAUSE<xxu:03-21-00> BEGIN

  //skip event loop
  //voipReleaseCause_ = MOBILE_CAUSE_USER_ALERTING_NO_ANSWER;
  voipReleaseCause_ = CNI_RIL3_CAUSE_USER_ALERTING_NO_ANSWER;
  
  //CAUSE<xxu:03-21-00> END

  rlsCause.causeValue = CNI_RIL3_CAUSE_USER_ALERTING_NO_ANSWER;

  // print out the trx, timeslot for better debugging
  UINT16 gsmHandler;
  UINT8 trx, slot;
  T_SUBSC_IE_ISDN* isdnPtr;
  
  if (rm_EntryIdToTrxSlot(parent->entryId(), 
                          &gsmHandler) ==
        RM_TRUE)
    {
      
      trx  = (UINT8)(gsmHandler>>8);
      slot = (UINT8)(gsmHandler);
      
      if (callTrace)
        {
          DBG_WARNING("CC Log: User No Reply on (TRX=%d),(Slot=%d),(entryId=%d)\n",
                      trx, slot, parent->entryId());
          if ((isdnPtr = ludbGetMSISDNPtr(parent->ludbIndex())) != NULL)
            {
              DBG_WARNING("CC Log: MS (DN=%s)\n",
                          isdnPtr->digits);
            }
        }
      else
        {
          DBG_TRACE("CC Log: User No Reply on (TRX=%d),(Slot=%d),(entryId=%d)\n",
                    trx, slot, parent->entryId());
          if ((isdnPtr = ludbGetMSISDNPtr(parent->ludbIndex())) != NULL)
            {
              DBG_TRACE("CC Log: MS (DN=%s)\n",
                        isdnPtr->digits);
            }
        }
    }

  //CAUSE<xxu:04-06-00> BEGING

  // Always do sendRemote at the end of the handler.
  // Now send the internal Rel Complt message to the other H.323 half call - 
  if (sendRemote(HC_REMOTE_RELEASE_COMPLETE) == false)
  // failed commn. with the remote side - treat as a release complete
  // from the remote side and cleanup - skip event loop.
  {
      DBG_LEAVE();
      return (handleMobRemoteAbnormalRelCom());
  }

  voipReleaseCause_ = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
  rlsCause.causeValue = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
 
  //CAUSE<xxu:04-06-00> END
 
  DBG_LEAVE();
  return(handleDisconnectReq());
}

JCCEvent_t 
CCHalfCall::handleMobAlerting(void)
{
  int timerForCFNRy;

  DBG_FUNC("CCHalfCall::handleMobAlerting", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // cancel call confirmed timer
  hcTimer->cancelTimer();

  callState_ = THC_ST_ALERTING;

  if (ludbIsProvCFNRY(parent->ludbIndex()) &&
      ludbIsActiveCFNRY(parent->ludbIndex()))
    {
      if ((timerForCFNRy = ludbGetTimerCFNRY(parent->ludbIndex()))
          >= 5)
        {
          timerForCFNRy = timerForCFNRy * (sysClkRateGet());
        }
      else
        {
          timerForCFNRy = CALL_CFNRY_TIMER;
        }
      
      hcTimer->setTimer(timerForCFNRy);
    }
  else if (parent->waitingCallLeg)
    {
      hcTimer->setTimer(CALL_WAITING_T3_TIMER);
    }
  else
    {
      hcTimer->setTimer(CALL_ALERTING_T301);
    }

  // Always do sendRemote at the end of the handler.
  // Now send the internal Alerting message to the other H.323 half call - 
  if (sendRemote(OHC_REMOTE_ALERTING) == false)
    // failed commn. with the remote side - treat as a release complete
    // from the remote side and cleanup - 
    // one stage release - skip event loop.
    {
      DBG_LEAVE();
      return (handleMobRemoteAbnormalRelCom());
    }

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleMobCallProgress(void)
{

  DBG_FUNC("CCHalfCall::handleMobCallProgress", CC_HALFCALL_LAYER);
  DBG_ENTER();

  int i;

  // cancel setup timer
  hcTimer->cancelTimer();

  // declare a pointer to the call confirmed portion of the message
  T_CNI_RIL3CC_MSG_CALL_CONFIRMED *callConf  = &(msDecodedMsg->callConfirmed);

  // !< handle user busy cause value. - Network should know this already,
  // - The MS maybe talking to a different network??

  // !! handle BC info elements as per 7.01

  // !< Call Control capabilities can be ignored for now

  callState_ = THC_ST_CALL_CONFIRMED;
  
  hcTimer->setTimer(CALL_CALL_CONFIRMED_T310);

  //PR1508 FR only case
  DBG_TRACE("RIL3->CC code(Ms)(MTC): 1st(%d,%d) 2nd(%d,%d) 3rd(%d,%d)\n",
             callConf->bearCap1.speechVersionInd[0].fieldPresent,
             callConf->bearCap1.speechVersionInd[0].version,
             callConf->bearCap1.speechVersionInd[1].fieldPresent,
             callConf->bearCap1.speechVersionInd[1].version,
             callConf->bearCap1.speechVersionInd[2].fieldPresent,
             callConf->bearCap1.speechVersionInd[2].version);

  if (!(callConf->bearCap1.ie_present))
   {
      callConf->bearCap1.ie_present = true;
      callConf->bearCap1.speechVersionInd[0].fieldPresent = true;
      callConf->bearCap1.speechVersionInd[0].version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
      callConf->bearCap1.speechVersionInd[1].fieldPresent = false;
      callConf->bearCap1.speechVersionInd[2].fieldPresent = false;
   }

  // setup remote data struct
  for (i=0; i < 3; i++)
    {
      remMsgData.callProceeding.bearerCap.speechVersionInd[i] = 
        callConf->bearCap1.speechVersionInd[i];
    }

  
  if ( !( callConf->bearCap1.speechVersionInd[0].fieldPresent ||
          callConf->bearCap1.speechVersionInd[1].fieldPresent ||
          callConf->bearCap1.speechVersionInd[2].fieldPresent ) )
		  
  {
          remMsgData.callProceeding.bearerCap.speechVersionInd[0].fieldPresent = 1;
          remMsgData.callProceeding.bearerCap.speechVersionInd[0].version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
  }
    
  DBG_TRACE("RIL3->CC code(Voip)(MTC): 1st(%d,%d) 2nd(%d,%d) 3rd(%d,%d)\n",
             remMsgData.callProceeding.bearerCap.speechVersionInd[0].fieldPresent,
             remMsgData.callProceeding.bearerCap.speechVersionInd[0].version,
             remMsgData.callProceeding.bearerCap.speechVersionInd[1].fieldPresent,
             remMsgData.callProceeding.bearerCap.speechVersionInd[1].version,
             remMsgData.callProceeding.bearerCap.speechVersionInd[2].fieldPresent,
             remMsgData.callProceeding.bearerCap.speechVersionInd[2].version);

  bearCap1speechVersionInd_[0] = remMsgData.callProceeding.bearerCap.speechVersionInd[0];
  bearCap1speechVersionInd_[1] = remMsgData.callProceeding.bearerCap.speechVersionInd[1];
  bearCap1speechVersionInd_[2] = remMsgData.callProceeding.bearerCap.speechVersionInd[2];

  // Always do sendRemote at the end of the handler.
  // Now send setup to the other half call.
  if (sendRemote(HC_REMOTE_CALL_PROGRESS) == false)
    // failed commn. with the remote side - treat as a release complete
    // from the remote side and cleanup - skip event loop.
    {
      DBG_LEAVE();
      return (handleMobRemoteAbnormalRelCom());
    }

  // EFR Change
  // Start Res. Assignment 
  // Very Early Assignment - Begin
  //    DBG_LEAVE();
  // return (CC_RESOURCE_ASSIGN_REQ);
  // Very Early Assignment - End
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleMobAnswer(void)
{

  DBG_FUNC("CCHalfCall::handleMobAnswer", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // cancel alerting timer
  hcTimer->cancelTimer();

  // !< handle subaddress may be needed.

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  callState_ = HC_ST_ACTIVE;
  
  // Now send the internal Answer message to the other H.323 half call - 
  if (sendRemote(OHC_REMOTE_ANSWER) == false)
    // failed commn. with the remote side - treat as a release complete
    // from the remote side and cleanup - skip event loop.
    {
      rlsCause.causeValue = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
      voipReleaseCause_ = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;

      DBG_LEAVE();
      return (handleMobRemoteAbnormalRelCom());
    }

  // Build the Connect Ack. Message towards the MS. 
  if ( (msOutResult = buildConnectAck (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Release RR resources

      rlsCause.causeValue = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
      voipReleaseCause_ = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;

      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }

  if (!parent->waitingCallLeg)
    {
      // Very Early Assignment - Begin
      // Synchronize with the Channel state machine
      if (channState_ != RES_ST_ASSIGNED)
        {
          // Channel is not assigned. Cleanup.
          // This way, you don't accept a late msg. accidentally.
          channState_ = RES_ST_IDLE;
          
          // Print a log for warning
          JCCLog1("CC Warning: H.245 channel selection for (call = %d) too late/missing?\n",
                  parent->callIndex);
          DBG_WARNING("CC Warning: H.245 channel selection for (call = %d) too late/missing?\n",
                  parent->callIndex);

          //CDR <xxu:08-21-00> BEGIN
          rlsCause.causeValue = CNI_RIL3_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
          voipReleaseCause_ = CNI_RIL3_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;

          DBG_LEAVE();
          return (CC_TWO_STAGE_DISCONNECT);
        }
       
      // Very Early Assignment - End

      // CDR <xxu:08-21-00>
      parent->ccCdr.answer((unsigned char)BEARER_SPEECH, (unsigned char)currSpeechCh_.version);
      parent->ccCdrCallState = true;
      parent->ccCdrTimer->setTimer(CALL_CDR_10M);
    
    }
  else
    {
      if (parent->parent->isOtherLegHeld(parent->callLegNum))
        {
          // No longer waiting
          parent->waitingCallLeg = false;
          DBG_LEAVE();
          return (CC_RESOURCE_ASSIGN_REQ);
        }
      else
        {
          // Do not allow this call

          //CAUSE<xxu:08-21-00> BEGIN

          rlsCause.causeValue = CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
          voipReleaseCause_ = CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;

          //CAUSE<xxu:08-21-00> END

          DBG_LEAVE();
          return (CC_TWO_STAGE_DISCONNECT);
        }
    }

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleConnectAck(void)
{

  DBG_FUNC("CCHalfCall::handleConnectAck", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // cancel connect timer
  hcTimer->cancelTimer();

  // CDR <xxu:08-21-00>
  parent->ccCdr.answer((unsigned char)BEARER_SPEECH, (unsigned char)currSpeechCh_.version);
  parent->ccCdrCallState = true;
  parent->ccCdrTimer->setTimer(CALL_CDR_10M);

  if (!isEmergencyCall)
    {
      PM_CCMeasurement.ansMobileOriginatingCalls.increment();
    }
  else
    {
      PM_CCMeasurement.ansMobileEmergencyCalls.increment();
    }

  callState_ = HC_ST_ACTIVE;
  
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleMobTermAddress(void)
{

  DBG_FUNC("CCHalfCall::handleMobTermAddress", CC_HALFCALL_LAYER);
  DBG_ENTER();

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;
  T_SUBSC_IE_MOBILE_ID_IMSI     *imsiPtr;
  T_SUBSC_IE_ISDN               *msisdnPtr;
  int i;

  // declare a pointer to the setup portion of the message
  T_CNI_RIL3CC_MSG_SETUP *setup  = &(msDecodedMsg->setup);

  // Copy the transaction identifier
  ti = (msDecodedMsg->header.si_ti | ORIG_OR_MASK);

  // Cancel MM Conn Est timer
  hcTimer->cancelTimer();

  setupTime_ = tickGet();
   
  // !< BC1 is mandatory. BC2 may be included. - only look at the 
  // message data to see what the MS is saying.
  // It shoudl be capable of Full Rate Speech, come on!

  // Copy locally the above
  cpn = setup->calledBCD;

  //CDR <xxu:08-29-00>
  parent->ccCdr.setCalledPartyNumber(&cpn);

  //    DBG_TRACE("CC Call Log: (called party number = %d,%d,%d,%d) \n",
  //              cpn.digits[0],
  //              cpn.digits[1],
  //              cpn.digits[2],
  //              cpn.digits[3]);

  // !< CLIR suppression, invocation - 4.81
  // !< CC Capabilities may be included.

  // setup remote data struct
  remMsgData.setup.CalledPartyNumber = cpn;
  for (i=0; i < 3; i++)
    {

        remMsgData.setup.bearerCap.speechVersionInd[i] = 
                          setup->bearCap1.speechVersionInd[i];

    }

  //PR1475 FR only case

  DBG_TRACE("RIL3->CC code(Rx): 1st(%d,%d) 2nd(%d,%d) 3rd(%d,%d)\n",
             setup->bearCap1.speechVersionInd[0].fieldPresent,
             setup->bearCap1.speechVersionInd[0].version,
             setup->bearCap1.speechVersionInd[1].fieldPresent,
             setup->bearCap1.speechVersionInd[1].version,
             setup->bearCap1.speechVersionInd[2].fieldPresent,
             setup->bearCap1.speechVersionInd[2].version);

  if (!(setup->bearCap1.ie_present))
   {
      setup->bearCap1.ie_present = true;
      setup->bearCap1.speechVersionInd[0].fieldPresent = true;
      setup->bearCap1.speechVersionInd[0].version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
      setup->bearCap1.speechVersionInd[1].fieldPresent = false;
      setup->bearCap1.speechVersionInd[2].fieldPresent = false;
   }
  

  if ( !( setup->bearCap1.speechVersionInd[0].fieldPresent ||
          setup->bearCap1.speechVersionInd[1].fieldPresent ||
          setup->bearCap1.speechVersionInd[2].fieldPresent  ) )
  {
          remMsgData.setup.bearerCap.speechVersionInd[0].fieldPresent = 1;
          remMsgData.setup.bearerCap.speechVersionInd[0].version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
  }
    
  DBG_TRACE("RIL3->CC code(Tx): 1st(%d,%d) 2nd(%d,%d) 3rd(%d,%d)\n",
             remMsgData.setup.bearerCap.speechVersionInd[0].fieldPresent,
             remMsgData.setup.bearerCap.speechVersionInd[0].version,
             remMsgData.setup.bearerCap.speechVersionInd[1].fieldPresent,
             remMsgData.setup.bearerCap.speechVersionInd[1].version,
             remMsgData.setup.bearerCap.speechVersionInd[2].fieldPresent,
             remMsgData.setup.bearerCap.speechVersionInd[2].version);

  bearCap1speechVersionInd_[0] = remMsgData.setup.bearerCap.speechVersionInd[0];
  bearCap1speechVersionInd_[1] = remMsgData.setup.bearerCap.speechVersionInd[1];
  bearCap1speechVersionInd_[2] = remMsgData.setup.bearerCap.speechVersionInd[2];

  remMsgData.setup.IsEmergencyCall = false;

  callState_ = OHC_ST_OFFERING_CALL;

  // !< Need to check with the LUDB that the call be allowed.
  // check subscriber's permissions & other SS interactions - future
  // Authorize the origination attempt
  if (! gLUDB.authOrig(parent->ludbIndex(), cpn))
    {
      JCCLog1("Origination Authorization failed, (LUDB Index = %d)\n",
              parent->ludbIndex());
      DBG_TRACE("Origination Authorization failed, (LUDB Index = %d)\n",
                parent->ludbIndex());
      DBG_LEAVE();

      return (handleMobRemoteAbnormalRelCom());
    }

  //CLIR <xxu:07-26-01> Begin
  T_SS_SUBSC_CLIR_OPTION option;
  bool                   isProvCLIR;
  int                    ret;

  if (ret=ludbGetCLIROption(parent->ludbIndex(), &option, &isProvCLIR))
  {
      DBG_ERROR("CC Half Call Log: ludbGetCLIROption() Failed (error=%d) for (ludbId=%d)\n",
                 ret, parent->ludbIndex());
      DBG_LEAVE();
      return (handleMobRemoteAbnormalRelCom());
  }

  DBG_TRACE("CC Half Call Log: ludbGetCLIROption() returned data (isProvCLIR=%d,option=%d,ret=%d,ludbId=%d)\n",
             isProvCLIR, option, ret, parent->ludbIndex());

  if (isProvCLIR)
  {
      if (setup->clirInvocation.ie_present || setup->clirSuppression.ie_present)
      {
          //Reject the call
          DBG_TRACE("CC Half Call Log: Call rejected due to illegal attempt of CLIR featire clirInv(%d) clirSup(%d)!\n",
                     setup->clirInvocation.ie_present, setup->clirSuppression.ie_present);
          rlsCause.causeValue = CNI_RIL3_CAUSE_REQUESTED_SERVICE_NOT_SUBSCRIBED;
          DBG_LEAVE();
          return (handleMobRemoteAbnormalRelCom());
      }

      remMsgData.setup.clir.ie_present  = true;
      remMsgData.setup.clir.clirSetting = CNI_RIL3_PRESENTATION_ALLOWED;

  } else
  {
      switch(option)
      {
      case CLIR_PERMANENT:
           //if (setup->clirSuppression.ie_present) Need to notify subscriber later
           remMsgData.setup.clir.ie_present  = true;
           remMsgData.setup.clir.clirSetting = CNI_RIL3_PRESENTATION_RESTRICTED;
           break;
           
      case CLIR_TEMP_DFT_RESTRICTED:
           remMsgData.setup.clir.ie_present  = true;
           if (setup->clirSuppression.ie_present)
               remMsgData.setup.clir.clirSetting = CNI_RIL3_PRESENTATION_ALLOWED;
           else
               remMsgData.setup.clir.clirSetting = CNI_RIL3_PRESENTATION_RESTRICTED;
           break;
      
      case CLIR_TEMP_DFT_ALLOWED:
           remMsgData.setup.clir.ie_present  = true;
           if (setup->clirInvocation.ie_present)
               remMsgData.setup.clir.clirSetting = CNI_RIL3_PRESENTATION_RESTRICTED;
           else
               remMsgData.setup.clir.clirSetting = CNI_RIL3_PRESENTATION_ALLOWED;
           break;

      default:
           DBG_ERROR("CC Half Call Log: ludbGetCLIROption() Incorrect Option(%d) for (ludbId=%d)\n",
                      option, parent->ludbIndex());
           break;
      }
  }
  
  DBG_TRACE("CC Half Call Log: CLIR sent to VoIP (ie_present=%d,clirSetting=%d,ludbId=%d)\n",
             remMsgData.setup.clir.ie_present,
             remMsgData.setup.clir.clirSetting, parent->ludbIndex());
  
  //CLIR <xxu:07-26-01> End

  // Build Call Proceeding as soon as the origination authorization 
  // is successful. Don't have to wait for a Call Proceeding from the 
  // H323 side.
  // Build the Call Proceeding Message towards the MS. 
  if ( (msOutResult = buildCallProceeding (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Release RR resources
 
      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }
 
  // BCT <xxu:07-11-00> BEGIN
  if ( parent->getBctInvoke() )
  { 
      hcTimer->setTimer(CALL_BCT_NUMBER_TVB);
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
  }
  // BCT <xxu:07-11-00> END
   
  // copy the call reference value towards the VOIP side.
  // remMsgData.callSetup.callRefValue = parent->callRefValue;

  // Always do sendRemote at the end of the handler.
  // Now send setup to the other half call.
  if (sendRemote(THC_REMOTE_TERM_ADDRESS) == false)
    // failed commn. with the remote side - treat as a release complete
    // from the remote side and cleanup - skip event loop.
    {

      DBG_LEAVE();
      return (handleMobRemoteAbnormalRelCom());
    }

  // set this boolean to true until we actually send a setup request to VOIP.
  parent->disconnectSentToVoip = false;

  // EFR Change
  // Also send a request to RRM for channel assignment.
  // Very early assignment - begin
  // DBG_LEAVE();
  // return (CC_RESOURCE_ASSIGN_REQ);
  // Very early assignment - end

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleMobEmergencyOrigination(void)
{
  DBG_FUNC("CCHalfCall::handleMobEmergencyOrigination", CC_HALFCALL_LAYER);
  DBG_ENTER();
  T_SUBSC_IE_MOBILE_ID_IMSI     *imsiPtr;
  T_SUBSC_IE_ISDN               *msisdnPtr;
  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;
  int i;

  // declare a pointer to the setup portion of the message
  T_CNI_RIL3CC_MSG_EMERGENCY_SETUP *setup  = &(msDecodedMsg->emergencySetup);

  // Copy the transaction identifier
  ti = (msDecodedMsg->header.si_ti | ORIG_OR_MASK);

  // Cancel MM Conn Est timer
  hcTimer->cancelTimer();

  setupTime_ = tickGet();
      
  cpn.ie_present = false;

  cpn.numDigits = 1;

  // setup remote data struct
  remMsgData.setup.CalledPartyNumber = cpn;

  // Bearer Capability is optional.
  // If BC not present, initialize with FR 

  if (!(setup->bearCap.ie_present))
   {
      setup->bearCap.ie_present = true;
      setup->bearCap.speechVersionInd[0].fieldPresent = true;
      setup->bearCap.speechVersionInd[0].version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
      setup->bearCap.speechVersionInd[1].fieldPresent = false;
      setup->bearCap.speechVersionInd[2].fieldPresent = false;
   }

   if ( !( setup->bearCap.speechVersionInd[0].fieldPresent ||
           setup->bearCap.speechVersionInd[1].fieldPresent ||
           setup->bearCap.speechVersionInd[2].fieldPresent  ) )
   {
          setup->bearCap.speechVersionInd[0].fieldPresent = 1;
          setup->bearCap.speechVersionInd[0].version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
   }

   for (i=0; i < 3; i++)
    {
      remMsgData.setup.bearerCap.speechVersionInd[i] = 
        setup->bearCap.speechVersionInd[i];
    }

  //PR1508
  DBG_TRACE("RIL3->CC code(Voip)(Em): 1st(%d,%d) 2nd(%d,%d) 3rd(%d,%d)\n",
             remMsgData.setup.bearerCap.speechVersionInd[0].fieldPresent,
             remMsgData.setup.bearerCap.speechVersionInd[0].version,
             remMsgData.setup.bearerCap.speechVersionInd[1].fieldPresent,
             remMsgData.setup.bearerCap.speechVersionInd[1].version,
             remMsgData.setup.bearerCap.speechVersionInd[2].fieldPresent,
             remMsgData.setup.bearerCap.speechVersionInd[2].version);

  remMsgData.setup.IsEmergencyCall = true;

  callState_ = OHC_ST_OFFERING_CALL;

  // Build Call Proceeding as soon as the origination authorization 
  // is successful. Don't have to wait for a Call Proceeding from the 
  // H323 side.
  // Build the Call Proceeding Message towards the MS. 
  if ( (msOutResult = buildCallProceeding (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Release RR resources

      //CAUSE<xxu:03-21-00> BEGIN

      rlsCause.causeValue = CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
      voipReleaseCause_ = CNI_RIL3_CAUSE_TEMPORARY_FAILURE;

      //CAUSE<xxu:03-21-00> END

      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }
  
  // copy the call reference value towards the VOIP side.
  // remMsgData.callSetup.callRefValue = parent->callRefValue;

  // Always do sendRemote at the end of the handler.
  // Now send setup to the other half call.
  if (sendRemote(THC_REMOTE_TERM_ADDRESS) == false)
    // failed commn. with the remote side - treat as a release complete
    // from the remote side and cleanup - skip event loop.
    {
      DBG_LEAVE();
      return (handleMobRemoteAbnormalRelCom());
    }

  // set this boolean to true until we actually send a setup request to VOIP.
  parent->disconnectSentToVoip = false;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

// disconnect handler 
JCCEvent_t 
CCHalfCall::handleDisconnectReq(void)
{

  DBG_FUNC("CCHalfCall::handleDisconnectReq", CC_HALFCALL_LAYER);
  DBG_ENTER();

  if ((currEvent == HC_LOCAL_DISCONNECT_REQ) && // Mobile disconnected
      (msDecodeResult == CNI_RIL3_RESULT_SUCCESS)) // not caused by protocol errors
  {
      // declare a pointer to the disconnect portion of the message
      T_CNI_RIL3CC_MSG_DISCONNECT *msInMsg  = &(msDecodedMsg->disconnect);
      
      if (callState_ != HC_ST_DISCONNECT_IND)
          rlsCause.causeValue = msInMsg->cause.causeValue;

      // Make sure that release cause is not sent in the release message.
      rlsCause.ie_present = false;

      if (rlsCause.causeValue == 0)
        {
          DBG_TRACE("RIL3->CC Warning: cause value from the Mobile is 0!\n");
          rlsCause.causeValue = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
        }
  }

  //CAUSE<xxu:03-21-00> BEGIN

  else if (currEvent == HC_LOCAL_PROTOCOL_TIMEOUT)
  {
      if (callState_ != THC_ST_ALERTING)
          rlsCause.causeValue = CNI_RIL3_CAUSE_RECOVERY_ON_TIMER_EXPIRY;
  }
  else
  {
      if (currEvent != CC_TWO_STAGE_DISCONNECT)
          rlsCause.causeValue = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
  }

  voipReleaseCause_ = rlsCause.causeValue;

  // Always do sendRemote at the end of the handler.
  // Now send the internal Rel Complt message to the other H.323 half call - 
  if (sendRemote(HC_REMOTE_RELEASE_COMPLETE) == false)
  // failed commn. with the remote side - treat as a release complete
  // from the remote side and cleanup - skip event loop.
  {
      DBG_LEAVE();
      return (handleMobRemoteAbnormalRelCom());
  }

  //switch (rlsCause.causeValue)
  //  {
  //  case CNI_RIL3_CAUSE_USER_BUSY:
  //    voipReleaseCause_ = MOBILE_CAUSE_USER_BUSY;
  //    break;
  //
  //  case CNI_RIL3_CAUSE_CALL_REJECTED:
  //    voipReleaseCause_ = MOBILE_CAUSE_CALL_REJECTED;
  //    break;
  //
  //  case CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING:
  //    voipReleaseCause_ = MOBILE_CAUSE_NORMAL_CALL_CLEARING;
  //    break;
  //      
  //  case CNI_RIL3_CAUSE_CHANNEL_UNACCEPTABLE:
  //    voipReleaseCause_ = MOBILE_CAUSE_CHANNEL_UNACCEPTABLE;
  //    break;
  //
  //  case CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED:
  //    voipReleaseCause_ = MOBILE_CAUSE_UNSPECIFIED;
  //    break;
  //
  //  default:
  //    switch((rlsCause.causeValue) >> 4)
  //      {
  //      case CNI_RIL3_CAUSE_CLASS_RESOURCE_UNAVAILABLE:
  //        voipReleaseCause_ = MOBILE_CAUSE_RESOURCES_UNAVAILABLE;
  //        break;
  //        
  //      case CNI_RIL3_CAUSE_CLASS_PROTOCOL_ERROR:
  //        voipReleaseCause_ = MOBILE_CAUSE_UNSPECIFIED;
  //        break;
  //        
  //      default:
  //        voipReleaseCause_ = MOBILE_CAUSE_UNSPECIFIED;          
  //        break;
  //      }
  //  }
    
  //CAUSE<xxu:03-21-00> END
    
  // cancel any timers
  hcTimer->cancelTimer();

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  // Build the Release Message towards the MS. 
  if ( (msOutResult = buildReleaseReq (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Release RR resources
      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }
  

  // start the release timer 
  hcTimer->setTimer(CALL_RELEASE_T308);
  releaseCounter = 1;
 
  callState_ = HC_ST_RELEASE_REQ;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

// Same handler can also be used for when an abnormal Rls is received
JCCEvent_t 
CCHalfCall::handleReleaseReq(void)
{

  DBG_FUNC("CCHalfCall::handleReleaseReq", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // Look at Cause value?
  if ((currEvent == HC_LOCAL_RELEASE_REQ) &&       // Mobile sent release
      (msDecodeResult == CNI_RIL3_RESULT_SUCCESS)) // not caused by protocol errors
  {
       //CAUSE<xxu:03-21-00> BEGIN 

       DBG_TRACE("CC Info: handleReleaseReq, ie_present %d, causeValue %d\n",
                  msDecodedMsg->release.cause.ie_present,
                  msDecodedMsg->release.cause.causeValue);

       if (msDecodedMsg->release.cause.ie_present && 
           callState_ != HC_ST_DISCONNECT_IND)
       {
           rlsCause.causeValue = msDecodedMsg->release.cause.causeValue;
       }

       //CAUSE<xxu:03-21-00> END

       // Make sure that cause is not sent in the release complete message.
       rlsCause.ie_present = false;
  }

  //CAUSE<xxu:03-21-00> BEGIN

  voipReleaseCause_ = rlsCause.causeValue;

  //CAUSE<xxu:03-21-00> END

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
}

// Not be used for when an abnormal Rls Complete
JCCEvent_t 
CCHalfCall::handleMobReleaseComplete(void)
{

  DBG_FUNC("CCHalfCall::handleMobReleaseComplete", CC_HALFCALL_LAYER);
  DBG_ENTER();

  //CAUSE<xxu:03-21-00> BEGIN

  if ( (msDecodeResult == CNI_RIL3_RESULT_SUCCESS) &&
       (msDecodedMsg->header.message_type==CNI_RIL3CC_MSGID_RELEASE_COMPLETE) &&
       (msDecodedMsg->releaseComplete.cause.ie_present) )
  {
      rlsCause.causeValue = msDecodedMsg->releaseComplete.cause.causeValue;
      voipReleaseCause_ = msDecodedMsg->releaseComplete.cause.causeValue;
  }
       
  //CAUSE<xxu:03-21-00> END

  // No need to look at Cause value for now.
  // !< Transaction Id. has to be released - in res_release
  
  callState_ = HC_ST_NULL;
  DBG_LEAVE();
  return (CC_RES_RELEASE);
}

//CAUSE<xxu:03-21-00> BEGIN
JCCEvent_t 
CCHalfCall::handleMobReleaseCompleteUtil(void)
{

  DBG_FUNC("CCHalfCall::handleMobReleaseCompleteUtil", CC_HALFCALL_LAYER);
  DBG_ENTER();

  callState_ = HC_ST_NULL;

  // No need to look at Cause value for now.
  // !< Transaction Id. has to be released - in res_release

  // Always do sendRemote at the end of the handler.
  // Now send the internal Rel Complt message to the other H.323 half call - 

  if (sendRemote(HC_REMOTE_RELEASE_COMPLETE) == false)
  // failed commn. with the remote side - treat as a release complete
  // from the remote side and cleanup - skip event loop.
  {
      DBG_LEAVE();
      return (handleMobRemoteAbnormalRelCom());
  }

  DBG_LEAVE();
  return (CC_RELEASE_CALL);
}

//CAUSE<xxu:03-21-00> END

JCCEvent_t 
CCHalfCall::handleStartDtmf(void)
{

  DBG_FUNC("CCHalfCall::handleStartDtmf", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // 
  // declare a pointer to the start dtmf portion of the message
  T_CNI_RIL3CC_MSG_START_DTMF *msInMsg  = &(msDecodedMsg->startDTMF);

  currKeypadFacility_ = msInMsg->keypadFacility;

  // Send an Ack. back to the MS.

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  // Build the Ack Message towards the MS. 
  if ( (msOutResult = buildStartDtmfAck (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Not critical to the call.
      JCCLog1("RIL3->CC: Build Start DTMF Ack problem, (RIL3 result = %d)\n",
              msOutResult);
      DBG_ERROR("RIL3->CC: Build Start DTMF Ack problem, (RIL3 result = %d)\n",
                msOutResult);

      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }

  DBG_TRACE ("CC DTMF Log: (keypad info. = %d)  received.\n",
             msInMsg->keypadFacility.keypadInfo);

  // Nullify the digit
  remMsgData.dtmfDigit = 0;

  remMsgData.dtmfDigit = (short)(msInMsg->keypadFacility.keypadInfo);

  if (remMsgData.dtmfDigit == 0x23)
    {
      remMsgData.dtmfDigit = 0x0b;
    }
  else
    {
      remMsgData.dtmfDigit = (remMsgData.dtmfDigit & 0x0f);
    }

  // Send an Ack. back to the MS.
  if (sendRemote(HC_REMOTE_DTMF_REQ) == false)
    // failed commn. with the remote side 
    {
      // Log the error. Not critical to the call.
      JCCLog("CC->VOIP: Start DTMF problem. Send to remote(H323) failed\n");
      DBG_ERROR("CC->VOIP: Start DTMF problem. Send to remote(H323) failed\n");
    }


  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleStopDtmf(void)
{

  DBG_FUNC("CCHalfCall::handleStopDtmf", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // 
  // Send an Ack. back to the MS.

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  // Build the Ack Message towards the MS. 
  if ( (msOutResult = buildStopDtmfAck (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Not critical to the call.
      JCCLog1("RIL3->CC: Build Stop DTMF Ack problem, (RIL3 result = %d)\n",
              msOutResult);
      DBG_ERROR("RIL3->CC: Build Stop DTMF Ack problem, (RIL3 result = %d)\n",
                msOutResult);
    }

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

// !< For all msgs,local and remote, User-to-User support can be added later,
// connected subaddress is related to this.

//CH<xxu:11-10-99>
JCCEvent_t 
CCHalfCall::handleHoldAct(void)
{
   
  T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

  DBG_FUNC("CCHalfCall::handleHoldAct", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE ("CC CallLog @HoldAct: callId=%d, callState=%d, chState=%d\n",
              parent->callIndex, callState_, chState_);

  //Calling this method implies the call's main state is active right now
  //Accept the HoldAct from mobile and behave consistently inside network
  chState_ = CH_ST_HELD;
  
  // Build the Hold Ack Message towards the MS. 
  if ( (msOutResult = buildHoldAck(&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
        // Log the software design bug, no workaround is proper here.
	  JCCLog1("RIL3->CC: Build HoldAck problem, (RIL3 result = %d)\n",
		     msOutResult);
	  DBG_ERROR("RIL3->CC: Build HoldAck problem, (RIL3 result = %d)\n",
		       msOutResult);
        chState_ = CH_ST_IDLE;
        return (CC_MESSAGE_PROCESSING_COMPLT);
    }

    //Hold the call's traffic path 
    T_CSU_PORT_ID srcPort, dstPort;
    T_CSU_RESULT_CON csuResult;

    srcPort = parent->parent->mobileCsuPort();
    dstPort.portType = CSU_RTP_PORT;
    dstPort.portId.rtpHandler = VoipCallGetRTPHandle(parent->h323CallHandle);
	  
    if ((csuResult = csu_DuplexBreak(&srcPort, &dstPort))
        != CSU_RESULT_CON_SUCCESS)
	  {
	     JCCLog2("CSU Error : Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
	  	        csuResult, parent->entryId());
   	     DBG_ERROR("CSU Error : Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
			    csuResult, parent->entryId());

  	  }

   parent->voiceConnected = false;

   return (CC_MESSAGE_PROCESSING_COMPLT);
}

//CH<xxu:11-24-99>
JCCEvent_t 
CCHalfCall::handleHoldRej(void)
{
   
  T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

  DBG_FUNC("CCHalfCall::handleHoldRej", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE ("CC CallLog @HoldRej: callId=%d, callState=%d, chState=%d\n",
              parent->callIndex, callState_, chState_);

  //Only build and send out HOLD REJ message, nothing else!
  
  // Build the Hold Rej Message towards the MS. 
  if ( (msOutResult = buildHoldRej(&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
  {
        // Log the software design bug, no workaround is proper here.
	JCCLog1("RIL3->CC: Build HoldRej problem, (RIL3 result = %d)\n",
	         msOutResult);
	DBG_ERROR("RIL3->CC: Build HoldRej problem, (RIL3 result = %d)\n",
		   msOutResult);
  }
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

//CH<xxu:11-10-99>
JCCEvent_t 
CCHalfCall::handleRetvAct(void)
{
   
  T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

  DBG_FUNC("CCHalfCall::handleRetvAct", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE ("CC CallLog @RetvAct: callId=%d, callState=%d, chState=%d\n",
              parent->callIndex, callState_, chState_);

  //Calling this method implies the call's main state is active right now
  //Accept the RetvAct from mobile and behave consistently inside network
  chState_ = CH_ST_IDLE;

  // Build the Retv Ack Message towards the MS. 
  if ( (msOutResult = buildRetvAck(&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
        // Log the software design bug, no workaround is proper here.
      JCCLog1("RIL3->CC: Build RetvAck problem, (RIL3 result = %d)\n",
	     msOutResult);
	  DBG_ERROR("RIL3->CC: Build RetvAck problem, (RIL3 result = %d)\n",
		       msOutResult);
        chState_ = CH_ST_IDLE;
        return (CC_MESSAGE_PROCESSING_COMPLT);
    }

    //Unhold the call's traffic path 
    T_CSU_PORT_ID srcPort, dstPort;
    T_CSU_RESULT_CON csuResult;

    srcPort = parent->parent->mobileCsuPort();
    dstPort.portType = CSU_RTP_PORT;
    dstPort.portId.rtpHandler = VoipCallGetRTPHandle(parent->h323CallHandle);
	  
    if ((csuResult = csu_DuplexConnect(&srcPort, &dstPort))
        != CSU_RESULT_CON_SUCCESS)
	  {
	     JCCLog2("CSU Error : Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
	  	        csuResult, parent->entryId());
   	     DBG_ERROR("CSU Error : Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
			    csuResult, parent->entryId());
  	  }

    parent->voiceConnected = true;

    return (CC_MESSAGE_PROCESSING_COMPLT);
}

//CH<xxu:11-24-99>
JCCEvent_t 
CCHalfCall::handleRetvRej(void)
{
   
  T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

  DBG_FUNC("CCHalfCall::handleRetvRej", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE ("CC CallLog @RetvRej: callId=%d, callState=%d, chState=%d\n",
              parent->callIndex, callState_, chState_);

  //Calling this method implies the call's main state is active right now
  //Only build and send out RETRIEVE REJ message, nothing else!

  // Build the Retv Reg Message towards the MS. 
  if ( (msOutResult = buildRetvRej(&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
        // Log the software design bug, no workaround is proper here.
	  JCCLog1("RIL3->CC: Build RetvRej problem, (RIL3 result = %d)\n",
		     msOutResult);
	  DBG_ERROR("RIL3->CC: Build RetvRej problem, (RIL3 result = %d)\n",
		       msOutResult);
    }
    return (CC_MESSAGE_PROCESSING_COMPLT);
}

//CH<xxu:11-10-99>
JCCEvent_t 
CCHalfCall::handleHoldReq(void)
{
   
  T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

  DBG_FUNC("CCHalfCall::handleHoldReq", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE ("CC CallLog @HoldReq: callId=%d, callState=%d, chState=%d\n",
              parent->callIndex, callState_, chState_);

  //Calling this method implies the call's main state is active right now
  //Accept the HoldReq from mobile and behave consistently inside network
  chState_ = CH_ST_HREQ;

  return (CC_MESSAGE_PROCESSING_COMPLT);
}

//CH<xxu:11-10-99>
JCCEvent_t 
CCHalfCall::handleSwapTimerExpiry(void)
{
   
  T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

  DBG_FUNC("CCHalfCall::handleSwapTimerExpiry", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE ("CC CallLog @SwapTimerExpiry: callId=%d, callState=%d, chState=%d\n",
              parent->callIndex, callState_, chState_);

  //Calling this method implies the call's main state is active right now
  //Accept the RetvAct from mobile and behave consistently inside network
  chState_ = CH_ST_IDLE;

  // Build the Hold Rej Message towards the MS. 
  if ( (msOutResult = buildHoldRej(&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
        // Log the software design bug, no workaround is proper here.
	  JCCLog1("RIL3->CC: Build HoldRej problem, (RIL3 result = %d)\n",
		     msOutResult);
	  DBG_ERROR("RIL3->CC: Build HoldRej problem, (RIL3 result = %d)\n",
		       msOutResult);
        //return (CC_MESSAGE_PROCESSING_COMPLT); //Allow voice is better
    }

  //Resume traffic path blindly
  T_CSU_PORT_ID srcPort,dstPort;

  srcPort = parent->parent->mobileCsuPort();
  dstPort.portType = CSU_RTP_PORT;
  dstPort.portId.rtpHandler = VoipCallGetRTPHandle(parent->h323CallHandle);
  
  csu_DuplexConnect(&srcPort, &dstPort);

  parent->voiceConnected = true;

  return (CC_MESSAGE_PROCESSING_COMPLT);
}

//CH<xxu:Mod 12-13-99>
JCCEvent_t 
CCHalfCall::handleRetvReq(void)
{

  DBG_FUNC("CCHalfCall::handleRetvReq", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE ("CC CallLog @handleRetvReq: callId=%d, callState=%d, chState=%d\n",
              parent->callIndex, callState_, chState_);

  // If external handover has occurred
  if (parent->parent->handoverSession->hoState_ == HAND_ST_ANCH_EHO_COMPLT)
    {
       T_CNI_LAPDM_L3MessageUnit  msEncodedMsg;
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

       // Build the Retv Ack Message towards the MS. 
       if ( (msOutResult = buildRetvAck(&msEncodedMsg))
             != CNI_RIL3_RESULT_SUCCESS )
         {
            // Log the software design bug, no workaround is proper here.
            JCCLog1("RIL3->CC: Build RetvAck problem, (RIL3 result = %d)\n",
                     msOutResult);
            DBG_ERROR("RIL3->CC: Build RetvAck problem, (RIL3 result = %d)\n",
            msOutResult);
         }

       //Voice path connected successfully for Retrive operation, send back RETV ACK
       parent->msSide->chState_ = CH_ST_IDLE;

       return (CC_MESSAGE_PROCESSING_COMPLT);
    }


  //Change chState to CH_ST_RREQ and return CC_RESOURCE_ASSIGN_REQ to reconnect
  //traffic channel back

  chState_ = CH_ST_RREQ;

  return (CC_RESOURCE_ASSIGN_REQ);
}

JCCEvent_t
CCHalfCall::handleMobStatusInfo(void)
{
  DBG_FUNC("CCHalfCall::handleMobStatusInfo", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // declare a pointer to the status portion of the message
  T_CNI_RIL3CC_MSG_STATUS *statusMsg  = &(msDecodedMsg->status);

  // let the developer/tester/fields person decide.
  // The user of the system needs to check this out to find the  problem.
  DBG_WARNING("MS->CC Status: (coding = %d, cause = %d),(coding = %d, call state = %d) \n",  
              statusMsg->cause.codingStandard,
              statusMsg->cause.causeValue, 
              statusMsg->callState.codingStandard,
              statusMsg->callState.state);
  if (statusMsg->auxStates.ie_present)
    {
      DBG_WARNING("MS->CC Status: Aux. states(hold state = %d),(mpty state = %d) \n",  
                  statusMsg->auxStates.holdState,
                  statusMsg->auxStates.mptyState);
    }
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
  
}

JCCEvent_t
CCHalfCall::handleMobStatusEnquiry(void)
{
  DBG_FUNC("CCHalfCall::handleMobStatusEnquiry", CC_HALFCALL_LAYER);
  DBG_ENTER();

  T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

  // Build the Status Message towards the MS. 
  if ( (msOutResult = buildStatusInfo (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Don't Release Channel resources. It is not a critical problem.
    }
  
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
  
}

