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
// File        : CCHalfCall.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : CC Half call main methods
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

//
#include "taskLib.h"

#include "CC/CCHalfCall.h"

#include "CC/CCMsgAnal.h" 

#include "jcc/JCCLog.h"

#include "stdio.h"

#include "CC/CCUtil.h"

#include "CC/CCInt.h"

#include "CC/CCH323Util.h"

#include "logging/VCLOGGING.h"

#include "pm/pm_class.h"

#include "csunew/csu_head.h"

#include "CC/CallConfig.h"

#include "CC/CallLeg.h"

extern bool callTrace;

extern DbgOutput Dbg;

inline int JCCDBG_TRACE2(char *str) { DBG_TRACE(str); return true;};

// *******************************************************************
// forward declarations.
// *******************************************************************

// !> move the following?
// this goes inside the Call Half Class definition
HalfCallMsgHandler_t generatedEventHandler[MAX_GENERATED_EVENTS];

// this goes inside the Call Half Class definition
HalfCallMsgHandler_t callStateHandler[MAX_CALL_STATES][MAX_CALL_EXTERNAL_EVENTS];
 
// Channel state machine
HalfCallMsgHandler_t channStateHandler[MAX_CHANN_STATES][MAX_CHANN_EXTERNAL_EVENTS];

 CCHalfCall::CCHalfCall ()
    {
    
    }
    
    CCOrigHalfCall::CCOrigHalfCall (TwoPartyCallLeg *callLeg, JCCTimer *tclTimer)
    {
      //PR1378 <xxu:08-16-00>
      //UINT16 uint16TimerValue;
      //int timerValue;
    
      // Timer Data below is used for sending timeout to the correct task.
      CCHalfCall::parent = callLeg;
    
      //uint16TimerValue = (((UINT16)(parent->callLegNum)) & (0x000F)) << 8;
      //uint16TimerValue = uint16TimerValue | (((UINT16)(parent->callIndex)) & 0x00FF);
    
      //timerValue = (short)uint16TimerValue;
    
      hcTimer = tclTimer;
      //hcTimer = new JCCTimer(callTimerExpiry, timerValue);
    
      initData();
    }
    
    CCTermHalfCall::CCTermHalfCall (TwoPartyCallLeg *callLeg, JCCTimer *tclTimer)
    {
      //PR1378 <xxu:08-16-00>
      //UINT16 uint16TimerValue;
      //int timerValue;
    
      // Timer Data below is used for sending timeout to the correct task.
      CCHalfCall::parent = callLeg;
    
      //uint16TimerValue = (((UINT16)(parent->callLegNum)) & (0x000F)) << 8;
      //uint16TimerValue = uint16TimerValue | (((UINT16)(parent->callIndex)) & 0x00FF);
    
      //timerValue = (short)uint16TimerValue;
    
      hcTimer = tclTimer;
      //hcTimer = new JCCTimer(callTimerExpiry, timerValue);
    
      initData();
    }
    
    void
    CCOrigHalfCall::setupNewCall (T_CNI_RIL3_SI_TI   newCallTi,
                                  T_CNI_RIL3_CM_SERVICE_TYPE  serviceRequested)
    {
    
      DBG_FUNC("CCOrigHalfCall::setupNewCall", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      char callTaskName[20];
    
      sprintf(&callTaskName[0], 
              "Call-%d EntryId-%d\0", 
              parent->callIndex,
              parent->entryId()); 
      
      if (callTrace)
        {
          Dbg.Func(&callTaskName[0], CC_LAYER);
        }
    
      // set transaction Id - first message for CC
      // The starting entity uses 0. Other side uses 1.
      ti = (newCallTi | ORIG_OR_MASK);
    
      // set to null state so that we can take care of abnormal cleanups.
      callState_ = HC_ST_NULL;
    
      // start the timer when waiting for the setup message
      hcTimer->setTimer(CALL_MM_CONN_EST_T999);
    
      isEmergencyCall = (serviceRequested == CNI_RIL3_CM_SERVICE_TYPE_EMERGENCY_CALL); 
      DBG_LEAVE();
    }
    
    void
    CCHalfCall::cleanup(void)
    {
    
      DBG_FUNC("CCHalfCall::cleanup", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      currEvent = CC_MM_CONNECTION_REL;
      smHandleFirstCallEvent();
      DBG_LEAVE();
    }
    
    void
    CCOrigHalfCall::initData(void)
    {
      CCHalfCall::ti = ORIG_OR_MASK; // ti = 0 from MS
      CCHalfCall::initData();
    }
    
    void
    CCTermHalfCall::initData(void)
    {
      // This is used in terminations as is
      CCHalfCall::ti = parent->callLegNum;
      CCHalfCall::initData();
    }
    
    void
    CCHalfCall::initData(void)
    {
    
      DBG_FUNC("CCHalfCall::initData", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      // cancel any timers
      hcTimer->cancelTimer();
    
      releaseCounter = 0;
      callState_ = HC_ST_FREE;
    
      //CH<xxu:11-10-99>
      chState_ = CH_ST_IDLE;
    
	        //BCT <xxu:09-24-00>
      if (annState_ == ANN_PLAY_ON)
	  {
          //Stop inband annoucement       
          T_CSU_RESULT_CON csuResult;

          DBG_TRACE("CC->CSU(bct): call released, stop inband alert ann(%d,%x)-->snk(%d,%x,%d)\n",
                     annSrcPort_.portType,
                     (int)annSrcPort_.portId.rtpHandler,
                     annSnkPort_.portType,
                     (int)annSnkPort_.portId.rtpHandler,
                     (int)annSnkPort_.speechVersion);

           if ( (csuResult = csu_SimplexBreak(&annSrcPort_, &annSnkPort_))
                 != CSU_RESULT_CON_SUCCESS)
		   {
                 DBG_WARNING("CC->CSU(bct): call released, stop inband alert s-break failed xlst(%d) ann(%d,%x) snk(%d,%x,%d)\n",
                              csuResult,
                              annSrcPort_.portType,
                              (int)annSrcPort_.portId.rtpHandler,
                              annSnkPort_.portType,
                              (int)annSnkPort_.portId.rtpHandler,
                              (int)annSnkPort_.speechVersion);

		   }

	  }

      //BCT <xxu:09-24-00>
      annState_ = ANN_PLAY_OFF;
    
      channState_ = RES_ST_IDLE;
    
      currSpeechCh_.version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
    
      currEvent = JCC_NULL_EVENT;
      msOutResult = CNI_RIL3_RESULT_SUCCESS;
    
      // Have a standard release cause to use
      memset(&rlsCause, 0, sizeof(T_CNI_RIL3_IE_CAUSE)); // first zap this ie
      rlsCause.ie_present = true; 
      rlsCause.codingStandard = CNI_RIL3_GSMPLMN_CODING;
      rlsCause.location = CNI_RIL3_LOCATION_PRIVATE_NETWORK_LOCAL_USER;
      rlsCause.causeValue = CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED;
      rlsCause.diagnosticsLength = 0;
    
      //CAUSE<xxu:03-21-00> BEGIN
    
      // pre-initialized value
      //voipReleaseCause_ = MOBILE_CAUSE_NORMAL_CALL_CLEARING;
    
      voipReleaseCause_ = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
    
      //CAUSE<xxu:03-21-00> END
     
      // initialize cpn
      cpn.ie_present = false;
    
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
    
      alertingSent_ = false;
	  progressSent_ = false;
      connectSent_ = false;
    
      DBG_LEAVE();
    
      if (callTrace)
        {
          Dbg.Func(taskName(taskIdSelf()), CC_LAYER);
        }
    
      isEmergencyCall = false;
    }
    
    void
    CCHalfCall::printData(JCCPrintStFn fnPtr)
    {
      char * hcStr1Ptr = "Half Call Data: \n";
      char hcStr2[120];
    
      char callTaskName[22];
    
      sprintf(&callTaskName[0],
              "Call-%d EntryId-%d: \0",
              parent->callIndex,
              parent->entryId()); 
    
      //BCT <xxu:08-01-00> BEGIN
      sprintf(&hcStr2[0],
              "ti(%d),curEv(%d),decRes(%d),encRes(%d),callSt(%d),channSt(%d),chState(%d),rlsCause(%d) annSt(%d)\n",
              ti,
              currEvent,
              msDecodeResult,
              msOutResult,
              callState_,
              channState_,
              chState_,
              rlsCause.causeValue,
              annState_
              );
      //BCT <xxu:08-01-00> END
    
      (*fnPtr)(callTaskName);
      (*fnPtr)(hcStr1Ptr);
      (*fnPtr)(&hcStr2[0]);
    
    }
    
    int
    CCHalfCall::handleMobMsg(T_CNI_RIL3CC_MSG      *msInMsg,
                             T_CNI_RIL3_RESULT     decodeResult,
                             T_CNI_RIL3_CAUSE_VALUE     causeValue,
                             JCCEvent_t            inEvent)
    {
      DBG_FUNC("CCHalfCall::handleMobMsg", CC_HALFCALL_LAYER);
      DBG_ENTER();
      
      currEvent = inEvent;
      msDecodedMsg = msInMsg;
      msDecodeResult = decodeResult;
      if (msDecodeResult != CNI_RIL3_RESULT_SUCCESS)
        {
          rlsCause.causeValue = causeValue;
    
          //CAUSE<xxu:03-21-00> BEGIN
          if (causeValue==CNI_RIL3_CAUSE_INVALID_MANDATORY_INFO)
              voipReleaseCause_ = CNI_RIL3_CAUSE_NETWORK_OUT_OF_ORDER;
          else
              voipReleaseCause_ = causeValue;
    
          //CAUSE<xxu:03-21-00> END
    
        } 
    
      DBG_LEAVE();
      return (smHandleFirstCallEvent());
    }
    
    int
    CCHalfCall::handleTimeoutMsg(IntraL3Msg_t    *ccInMsg)
    {
      DBG_FUNC("CCHalfCall::handleTimeoutMsg", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      // We already know that it is a timeout msg 
      //CH<xxu:11-10-99> now CH not supported during & after HO
      if (chState_ == CH_ST_HREQ)
          {
             currEvent = HC_LOCAL_CHTR_EXP;
             DBG_LEAVE();
             return (smHandleFirstCallEvent());
          } 
      else
      {
          currEvent = HC_LOCAL_PROTOCOL_TIMEOUT;
    
          rlsCause.causeValue = CNI_RIL3_CAUSE_RECOVERY_ON_TIMER_EXPIRY;
              
          DBG_LEAVE();
          return (smHandleFirstCallEvent());
      } 
    }
    
    
    JCCEvent_t
    CCHalfCall::handleInvalidChannEvent(void)
    {
      DBG_FUNC("CCHalfCall::handleInvalidChannEvent", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      // let the developer/tester/fields person decide.
      // The user of the system needs to check this out if there was other problem.
      // Otherwise, this is just a race condition.
      JCCLog2("RR->CC Chann Warning: Invalid/Late (event = %d)  in (State = %d) \n",  
              currEvent, 
              channState_);
      DBG_WARNING("RR->CC Chann Warning: Invalid/Late (event = %d)  in (State = %d) \n",  
                  currEvent, 
                  channState_);
      DBG_LEAVE();
      return (JCC_NULL_EVENT);
      
    }
    
    int
    CCHalfCall::handleRemoteEvent(IntraL3Msg_t* h323Msg, JCCEvent_t remoteEvent,
                                  CCRemMsgData_t  inRemMsg)
    { 
      DBG_FUNC("CCHalfCall::handleRemoteEvent", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      // copy message data
      h323InMsg = h323Msg;
      remMsgData = inRemMsg;
    
      currEvent = remoteEvent;
      
      // Just call the SM Handle Event
      DBG_LEAVE();
      return (smHandleFirstCallEvent());
    }
    
    int
    CCHalfCall::smHandleEvent(void)
    {
      DBG_FUNC("CCHalfCall::smHandleEvent", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      // Call Handler and loop
      while ((currEvent != JCC_NULL_EVENT)  &&
             (currEvent != CC_MESSAGE_PROCESSING_COMPLT) &&
             (currEvent != CC_RELEASE_CALL))
        {
          // sm logic here - one-to-one mapping for internal events
          // that are generated from within.
          // state is already factored in
          currEvent = 
            (this->*generatedEventHandler[currEvent - CC_INTERNAL_GEN_EVENT_BASE])();
        }
      
      if (currEvent == CC_RELEASE_CALL)
        {
          int storedCause;
        
          parent->handleReleaseCall();
    
          // causeValue below Works because 0 is unused.  
          storedCause = (0 - rlsCause.causeValue);
    
          initData();
          DBG_LEAVE();
          return(storedCause);
        }
          
      DBG_LEAVE();
      return(0);
    }
    
    int
    CCHalfCall::smHandleFirstCallEvent(void)
    {
      DBG_FUNC("CCHalfCall::smHandleFirstCallEvent", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      // Come up with the correct handler that should handle the msg 
      // based on the state 
    
    #ifdef _JCC_DEBUG
      ccEntryPrint(ccSession[parent->callIndex], JCCLog);
    #endif
      
      if (callTrace)
        {
    
          ccEntryPrint(ccSession[parent->callIndex], JCCDBG_TRACE2);
        }
    
    //TEST
    //printf("CC Log FirstCallEv: callIndex(%d), callSt(%d), curEv(%d)\n", parent->callIndex, callState_, currEvent);
    
      // For Call States:
      if (currEvent != JCC_NULL_EVENT)
        {
          currEvent = 
            (this->*callStateHandler[callState_][currEvent - CC_INTERNAL_CALL_EVENT_BASE])();
        }
    
      DBG_LEAVE();
      return(smHandleEvent());
    }
    
    JCCEvent_t
    CCHalfCall::handleInvalidEvent(void)
    {
    
      DBG_FUNC("CCHalfCall::handleInvalidEvent", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      // let the developer/tester/fields person decide.
      // The user of the system needs to check this out if there was other problem.
      // Otherwise, this is just a race condition.
      JCCLog2("CC Call Warning: Invalid (event = %d)  in (State = %d) \n",  
              currEvent, 
              callState_);
      DBG_WARNING("CC Call Warning: Invalid (event = %d)  in (State = %d) \n",  
                  currEvent, 
                  callState_);
      DBG_LEAVE();
      return (JCC_NULL_EVENT);
      
    }
    
    bool
    CCHalfCall::sendRemote(JCCEvent_t remoteEvent)
    {
      DBG_FUNC("CCHalfCall::sendRemote", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      // Copy the disconnect reason, if it is a release message.
      if (remoteEvent == HC_REMOTE_RELEASE_COMPLETE)
        {
          remMsgData.callRelease.cause = voipReleaseCause_;
        }
    
      // send to H323 task
      DBG_LEAVE();
      return (sendH323Msg(parent, remoteEvent, remMsgData));
    }
    
    JCCEvent_t
    CCHalfCall::handleMobProtocolProblem(void)
    {
      DBG_FUNC("CCHalfCall::handleMobProtocolProblem", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      //CAUSE<xxu:03-21-00> BEGIN
    
      rlsCause.causeValue = CNI_RIL3_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
      voipReleaseCause_ = CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER;
    
      //CAUSE<xxu:03-21-00> END
    
      DBG_LEAVE();
      return (CC_TWO_STAGE_DISCONNECT);
    }
    
    int
    CCHalfCall::handleMMMsg(IntraL3Msg_t    *mmMsg,
                            JCCEvent_t      inEvent)
    {
      DBG_FUNC("CCHalfCall::handleMMMsg", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      mmInMsg = mmMsg;
    
      currEvent = inEvent;
    
      DBG_LEAVE();
      return(smHandleFirstCallEvent());
    
    }
    
    int
    CCHalfCall::handleRRMsg(IntraL3Msg_t      *rrMsg,
                            JCCEvent_t        inEvent)
    {
      DBG_FUNC("CCHalfCall::handleRRMsg", CC_HALFCALL_LAYER);
      DBG_ENTER();
    
      rrInMsg = rrMsg;
    
      currEvent = inEvent;
    
      currEvent = 
        (this->*channStateHandler[channState_][currEvent - CC_INTERNAL_CHANN_EVENT_BASE])();
    
      DBG_LEAVE();
      return(smHandleEvent());
    }
