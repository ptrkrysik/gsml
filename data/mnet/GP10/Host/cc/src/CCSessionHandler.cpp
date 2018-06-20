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
// File        : CCSessionHandler.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-99
// Description : CC Session Handler
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

//
#include "taskLib.h"

#include "jcc/JCCLog.h"

#include "JCC/JCCUtil.h"

#include "logging/VCLOGGING.h"

//CH<xxu:11-24-99>
#include "jcc/LUDBApi.h"
#include "ril3/ie_cause.h"
#include "ril3/ril3_sms_rp.h"
#include "rm/rm_ccintf.h"

#include "CC/CallConfig.h"
#include "CC/CCSessionHandler.h"

//CDR <xxu:08-21-00>
#include "CDR/CdrVoiceCall.h"
#include "CDR/CdrSSA.h"

#include "CC/CCUtil.h"

#include "CC/CCInt.h"

#include "stdio.h"

#include "CC/CCHandover.h"

bool smsSendVbLinkRelease(LUDB_ID, UINT32, T_CNI_RIL3_RP_CAUSE_VALUE cause = 
                          CNI_RIL3_RP_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE);

extern int findCallUsingCallHandle( VOIP_CALL_HANDLE callHandle);

extern void sessionDataInit(int);

//BCT <xxu:07-06-00> BEGIN
extern char CALL_BCT_PREFIX[10];	//Prefix defined for blind call transfer invocation 
extern char CALL_BCT_PREFIX_LEN;	//	     
extern bool sendH323Msg(TwoPartyCallLeg *, JCCEvent_t, CCRemMsgData_t);
extern bool VBLinkTransferRequest(unsigned short,
                                  T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER,
                                  T_CNI_RIL3_IE_CALLING_PARTY_BCD_NUMBER);

//BCT <xxu:07-06-00> END

#include "rm/rm_head.h"
extern u8 rm_EntryIdToTrxSlot(T_CNI_IRT_ID entryId, u16 *gsmHandler);

extern bool callTrace;

// *******************************************************************
// forward declarations.
// *******************************************************************

CCSessionHandler::CCSessionHandler (CCSession_t *session)
  :parent     (session), 
   sapi3Queue_(this   )
{

  entryId = (T_CNI_IRT_ID) JCC_UNDEFINED;

  //BCT <xxu:09-25-00>
  //anchorHandoverSession = new CCAnchorHandover(this, parent->callIndex, parent->msgQId);
  //targetHandoverSession = new CCTargetHandover(this, parent->callIndex, parent->msgQId);

  // Initialize the handover to NULL value, which means that no handover is going on
  //handoverSession = NULL;

  ma = new CCMsgAnalyzer();

  callLeg1 = new TwoPartyCallLeg(this, parent->callIndex, parent->msgQId, 1);
  callLeg2 = new TwoPartyCallLeg(this, parent->callIndex, parent->msgQId, 2);

  // Placeholder to refuse a third callleg request from MS.
  callLeg3 = new TwoPartyCallLeg(this, parent->callIndex, parent->msgQId, 3);

  smsLeg1 = new SmsLeg (this, parent->callIndex, parent->msgQId, 4); //MT SMS leg
  smsLeg2 = new SmsLeg (this, parent->callIndex, parent->msgQId, 5); //M0 SMS leg
  smsLeg3 = new SmsLeg (this, parent->callIndex, parent->msgQId, 6); //MT SMS leg

  cissCallLeg = new CISSCallLeg(this, parent->callIndex, parent->msgQId, 0);

  //ext-HO <xxu:06-08-01>
  hoCallLeg = new HOCallLeg(this, parent->callIndex, parent->msgQId, 7);
  h3CallLeg = new HOCallLeg(this, parent->callIndex, parent->msgQId, 8);

  //BCT <xxu:09-25-00>
  anchorHandoverSession      = new CCAnchorHandover(this, parent->callIndex, parent->msgQId);
  targetHandoverSession      = new CCTargetHandover(this, parent->callIndex, parent->msgQId);
  // ext-ho <xxu:06-14-01>
  targetHandoverSessionExtHo = new CCTargetHandoverExtHo(this, parent->callIndex, parent->msgQId);

  // Initialize the handover to NULL value, which means that no handover is going on
  handoverSession = NULL;


  UINT16 uint16TimerValue;
  int timerValue;

  uint16TimerValue = (((UINT16)(3)) & (0x000F)) << 8;
  uint16TimerValue = uint16TimerValue | (((UINT16)(parent->callIndex)) & 0x00FF);

  timerValue = (short)uint16TimerValue;
  sessionTimer = new JCCTimer(callTimerExpiry, timerValue);

  initData();
}


//ext-HO <xxu:06-08-01>
void
CCSessionHandler::handleReleaseCallLegExtHo(CallLeg *leg)
{

  DBG_FUNC("CCSessionHandler::handleReleaseCallLegExtHo", CC_HO_LAYER);
  DBG_ENTER();

  DBG_TRACE("{\nMNEThoTRACE(cs::handleReleaseCallLegExtHo): ho-Call Leg released\n}\n");

  // Need to wait until all the Call Legs are released to release the session
  if ((callLeg1->idle)    && (callLeg2->idle)    && (cissCallLeg->idle) &&
      (smsLeg1->isIdle()) && (smsLeg2->isIdle()) && (smsLeg3->isIdle()) &&
	  (hoCallLeg->idle)   && (h3CallLeg->idle)                           )
  {
	  DBG_TRACE("{\nMNEThoTRACE(cs::handleReleaseCallLegExtHo): ho-Call Leg is released last!\n}\n");
      cleanup();
  }

  DBG_LEAVE();
}

//ext-HO <xxu:06-08-01>
void
CCSessionHandler::handleReleaseCallLegExtH3(CallLeg *leg)
{

  DBG_FUNC("CCSessionHandler::handleReleaseCallLegExtH3", CC_HO_LAYER);
  DBG_ENTER();

  DBG_TRACE("{\nMNEThoTRACE(cs::handleReleaseCallLegExtH3): h3-Call Leg released\n}\n");

  // Need to wait until all the Call Legs are released to release the session
  if ((callLeg1->idle)    && (callLeg2->idle)    && (cissCallLeg->idle) &&
      (smsLeg1->isIdle()) && (smsLeg2->isIdle()) && (smsLeg3->isIdle()) &&
	  (hoCallLeg->idle)   && (h3CallLeg->idle)                           )
  {
	  DBG_TRACE("{\nMNEThoTRACE(cs::handleReleaseCallLegExtH3): h3-Call Leg is released last!\n}\n");
      cleanup();
  }

  DBG_LEAVE();
}


void
CCSessionHandler::handleReleaseCallLeg (CallLeg *leg)
{

  DBG_FUNC("CCSessionHandler::handleReleaseCallLeg", CC_SESSION_LAYER);
  DBG_ENTER();

  // Need to wait until all the Call Legs are released to release the session
  if ((callLeg1->idle)    && (callLeg2->idle)    && (cissCallLeg->idle) &&
      (smsLeg1->isIdle()) && (smsLeg2->isIdle()) && (smsLeg3->isIdle()))
  {
      DBG_TRACE("handleReleaseCallLeg:performing CCSession cleanup\n");
      cleanup();
  }
  else if (isMmSetupTermActive() && getMmTermSetupLeg() == leg)
  { // Reset the Mobile Termination Leg if there is another active Leg in a session
      DBG_TRACE("handleReleaseCallLeg:performing CCSession resetMmTermSetup\n");
      resetMmTermSetup();
  }

  DBG_LEAVE();
}

void
CCSessionHandler::initData ()
{
  DBG_FUNC("CCSessionHandler::initData", CC_SESSION_LAYER);
  DBG_ENTER();

  // Do the following only if entry id is valid. 
  // In terminations and cleanups, it may not be setup yet.
  // Check the case below to take care of paging failure cases.

  if (entryId != (T_CNI_IRT_ID) JCC_UNDEFINED )
    {

      IntraL3Msg_t mmOutMsg;

      // Send internal MM msg - no data needed
      sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, entryId, &mmOutMsg);

      resetIrt ();

      entryId = (T_CNI_IRT_ID) JCC_UNDEFINED;
    }

  oid = CNI_LAPDM_NULL_OID;

  ludbIndex = JCC_UNDEFINED;

  releaseCause = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;

  isEmergencyCall = false;

  resetSapi3       ();
  resetMmTermSetup ();

  sapi3Queue_.reset();

  handleReleaseSession(parent);

  //BCT <xxu:07-06-00> BEGIN
  bctInvoke_ = false;
  bctTi_ = ORIG_OR_MASK;
  bctLeg_ = NULL;
  //BCT <xxu:07-06-00> END

  //CDR <xxu:08-23-00>
  mobileId.mobileIdType = CNI_RIL3_NO_MOBILE_ID;

  //ext-HO <xxu:06-08-01>
  hoCallHandle = VOIP_NULL_CALL_HANDLE;

  DBG_LEAVE();
}

bool CCSessionHandler::resetIrt ()
{
  bool retVal;

  DBG_FUNC ("CCSessionHandler::resetIrt", CC_SESSION_LAYER);
  DBG_ENTER();

    retVal = setIrt (entryId, cc_id);

  DBG_LEAVE();
  return   (retVal);
}


// Sets all the required IRTs for CC
bool CCSessionHandler::setIrt (T_CNI_IRT_ID inEntryId, T_CNI_L3_ID& newId)
{
  bool retVal = (inEntryId != (T_CNI_IRT_ID) JCC_UNDEFINED);

  DBG_FUNC ("CCSessionHandler::setIrt", CC_SESSION_LAYER);
  DBG_ENTER();

      // Check validity of the received entry ID and Update CC entry for FACCH IRT entry
  if ( retVal                                            == true  && 
      (retVal = CNI_RIL3_IRT_Set_CC_Id (inEntryId, newId)) == true    )
  {
      // Update CC entry for SACCH IRT entry
      retVal = CNI_RIL3_IRT_Set_CC_Id (rm_GetSacchEntryId (inEntryId), newId);
  }

  if (retVal == false)
      DBG_WARNING ("CC Call Error : IRT Table set problem for (entry id = %d)\n ", inEntryId);

  DBG_LEAVE();
  return   (retVal);
}


void
CCSessionHandler::printData (JCCPrintStFn fnPtr)
{
  char * shStr1Ptr = "Session Handler Data : ";
  char shStr2[120];
 
  //BCT <xxu:07-12-00> BEGIN add BCT info
  sprintf(&shStr2[0],
          "lapId( %d), irtId( %d), (ludbId %d) (emergency %d) bct(inv %d, ti %d, leg %p)\n",
          oid,
          entryId,
          ludbIndex,
          isEmergencyCall,
          bctInvoke_,
          bctTi_,
          bctLeg_
          );
  //BCT <xxu:07-12-00> END add BCT info

  (*fnPtr)(shStr1Ptr);
  (*fnPtr)(&shStr2[0]);
   
  if (handoverSession == anchorHandoverSession)
    {
      anchorHandoverSession->printData(fnPtr);
    }
  else if (handoverSession == targetHandoverSession)
    {
      targetHandoverSession->printData(fnPtr);
    }

  callLeg1->printData    (fnPtr);
  callLeg2->printData    (fnPtr);
  cissCallLeg->printData (fnPtr);
  smsLeg1->printData     (fnPtr);
  smsLeg2->printData     (fnPtr);
  smsLeg3->printData     (fnPtr);

}

bool
CCSessionHandler::setupOrig(T_CNI_IRT_ID               irtId     ,
                            T_CNI_RIL3_CM_SERVICE_TYPE cmServType)
{
    /* Called ONLY for the very first MO setup (either call or SMS-PP */
  bool retVal = true;

  DBG_FUNC("CCSessionHandler::setupOrig", CC_SESSION_LAYER);
  DBG_ENTER();
  entryId = irtId;
  // Also initialize other variables in the session


  T_CNI_RIL3_IRT_ENTRY  irtEntry;
  irtEntry.entry_id = entryId;

  if  ( (CNI_RIL3_IRT_Get_Entry_by_Entry_ID(&irtEntry)) == false ) 
  {
      DBG_ERROR("IRT Error : IRT Table problem, (Entry Id = %d) \n ",
                entryId);

      // Cleanup action ??  - Release all resources
      initData();
      DBG_LEAVE();
      return (false);
  }

  oid = irtEntry.lapdm_oid;

  if (cmServType == CNI_RIL3_CM_SERVICE_TYPE_SHORT_MESSAGE_SERVICE)
  {
      DBG_TRACE ("Init MO SMS connection.\n");

      //Call to MO SMS setup: smsLeg2 is designated for MO SMS
      retVal = smsLeg2->setupOrig();
  }
  else if (cmServType != CNI_RIL3_CM_SERVICE_TYPE_SUPPLEMENTARY_SERVICE)
  {
      isEmergencyCall = (cmServType == CNI_RIL3_CM_SERVICE_TYPE_EMERGENCY_CALL); 

      callLeg1->setupOrig(0, cmServType);
      retVal = true;
  }

  DBG_LEAVE();
  return (retVal);
}


SpeechChann_t
CCSessionHandler::currSpeechCh(void) 
{
  DBG_FUNC("CCSessionHandler::currSpeechCh", CC_SESSION_LAYER);
  DBG_ENTER();
  
  DBG_LEAVE();
  if ( (activeLeg == NULL) || (activeLeg()->msSide == NULL) )
	  return (currChannDef);
  else
      return(activeLeg()->msSide->currSpeechCh_);
}

//Igal: Updated for SMS HO
bool
CCSessionHandler::isHandoverAllowed(void) 
{
  DBG_FUNC("CCSessionHandler::isHandoverAllowed", CC_SESSION_LAYER);
  DBG_ENTER();

  bool callLeg1Ok, callLeg2Ok = false, smsHoAllowed;
  
  if ((!callLeg1->idle) && (callLeg1->msSide != NULL))
    {
      DBG_TRACE("isHandoverAllowed Info: callLeg1's channState_(%d) callState_(%d)\n",
                 callLeg1->msSide->channState_,
                 callLeg1->msSide->callState_);

      if (((callLeg1->msSide->channState_ == RES_ST_ASSIGNED) && // Do not allow SDCCH (or) TCH signaling mode handovers
           ((callLeg1->msSide->callState_ != HC_ST_RELEASE_REQ) &&
            (callLeg1->msSide->callState_ != HC_ST_DISCONNECT_IND))) ||
          (callLeg1->waitingCallLeg))
//
//      if ((callLeg1->msSide->callState_ == HC_ST_ACTIVE) ||
//          (callLeg1->msSide->callState_ == OHC_ST_ANSWER_PENDING) ||
//          (callLeg1->msSide->callState_ == THC_ST_ALERTING))


        {
          // Don't allow handovers during NON-STABLE call state!
          if (callLeg1->isActive())
            {
              callLeg1Ok = true;
            }
        }
    }
  else if (!callLeg2->idle)
    {
      callLeg1Ok = true;
    }

  if ((!callLeg2->idle) && (callLeg2->msSide != NULL))
    {
      DBG_TRACE("isHandoverAllowed Info: callLeg2's channState_(%d) callState_(%d)\n",
                 callLeg2->msSide->channState_,
                 callLeg2->msSide->callState_);

      if (((callLeg2->msSide->channState_ == RES_ST_ASSIGNED) &&
           ((callLeg2->msSide->callState_ != HC_ST_RELEASE_REQ) &&
            (callLeg2->msSide->callState_ != HC_ST_DISCONNECT_IND))) ||
          (callLeg2->waitingCallLeg))
//
//   if ((callLeg2->msSide->callState_ == HC_ST_ACTIVE) ||
//          (callLeg2->msSide->callState_ == OHC_ST_ANSWER_PENDING) ||
//          (callLeg2->msSide->callState_ == THC_ST_ALERTING))
        {

          // Don't allow handovers during NON-STABLE call state!
          if (callLeg2->isActive())
            {
              callLeg2Ok = true;
            }
        }
    }
  else if (!callLeg1->idle)
    {
      callLeg2Ok = true;
    }

  //Igal: SMS Handover conditions
  smsHoAllowed = isSmsHoAllowed();

  DBG_TRACE("isHandoverAllowed Info: callLeg1Ok(%d) callLeg2Ok(%d) smsHoAllowed(%d) bctInvoke_(%d) getBctArrival(%d)\n",
             callLeg1Ok, callLeg2Ok, smsHoAllowed, bctInvoke_, getBctArrival());

  DBG_LEAVE();

  //BCT <xxu:07-14-00> BEGIN
  return   (callLeg1Ok && callLeg2Ok && smsHoAllowed && !bctInvoke_ && !getBctArrival());
  //BCT <xxu:07-14-00> BEGIN

}

bool
CCSessionHandler::isOtherLegHeld(short legNum) 
{
  DBG_FUNC("CCSessionHandler::isOtherLegHeld", CC_SESSION_LAYER);
  DBG_ENTER();

  bool returnValue = false;
  TwoPartyCallLeg *otherLeg;

  if (legNum == 1)
    {
      otherLeg = callLeg2;
    }
  else 
    {
      otherLeg = callLeg1;
    }

  if ((otherLeg->idle) ||
      (otherLeg->msSide == NULL))
    {
      returnValue = true;
    }
  else if (otherLeg->msSide->chState_ == CH_ST_HELD)
    {
      returnValue = true;
    }

  DBG_LEAVE();
  return(returnValue);
}


bool CCSessionHandler::isSmsHoAllowed ()
{
  DBG_FUNC ("CCSessionHandler::isSmsHoAllowed", CC_SESSION_LAYER);
  DBG_ENTER();

  bool retVal = (smsLeg1->isHoAllowed() &&
                 smsLeg2->isHoAllowed() &&
                 smsLeg3->isHoAllowed()   );

  DBG_LEAVE();
  return   (retVal);
}


TwoPartyCallLeg *
CCSessionHandler::activeLeg(void) 
{
  DBG_FUNC("CCSessionHandler::activeLeg", CC_SESSION_LAYER);
  DBG_ENTER();

  TwoPartyCallLeg *returnValue = NULL;
  
  //Igal: update this message, change return type to CallLeg?

  if ((!callLeg1->idle) && (callLeg1->msSide != NULL))
    {
      if (((callLeg2->idle) ||
           (callLeg1->msSide->chState_  == CH_ST_IDLE) ||
           (callLeg1->msSide->chState_  == CH_ST_HREQ)) &&
          (!callLeg1->waitingCallLeg))
        {
          returnValue = callLeg1;
        }
    }

  if ((!callLeg2->idle) && (callLeg2->msSide != NULL))
    {
      if (((callLeg1->idle) ||
           (callLeg2->msSide->chState_  == CH_ST_IDLE) ||
           (callLeg2->msSide->chState_  == CH_ST_HREQ)) &&
          (!callLeg2->waitingCallLeg))
        {
          returnValue = callLeg2;
        }
    }

  // Otherwise, we are doing a signaling activity (CISS, SMS Point to Point, etc)

  DBG_LEAVE();
  return(returnValue);
}

CallLeg *
CCSessionHandler::getCallLegForNewTermination(bool &waitingCallLeg)
{
  DBG_FUNC("CCSessionHandler::getCallLegForNewTermination", CC_SESSION_LAYER);
  DBG_ENTER();
  
  TwoPartyCallLeg *whichLeg = NULL;
  bool waitingAllowed = (ludbIsProvCW(ludbIndex) &&
                         ludbIsActiveCW(ludbIndex) &&
                         !isEmergencyCall );             // Override features for emergency call

  if ((callLeg1->idle) &&
      ((callLeg2->idle) || ((callLeg2->isActive()) && waitingAllowed)))
  {
      whichLeg = callLeg1;
      waitingCallLeg = (!(callLeg2->idle));
  }
  else if ((callLeg2->idle) &&
           ((callLeg1->idle) || ((callLeg1->isActive()) && waitingAllowed)))
  {
      whichLeg = callLeg2;
      waitingCallLeg = (!(callLeg1->idle));
  }

  DBG_LEAVE();

  return (whichLeg);
}

CallLeg *
CCSessionHandler::getSmsLegForNewTermination (bool &waitingCallLeg)
{
  DBG_FUNC("CCSessionHandler::getSmsLegForNewTermination", CC_SESSION_LAYER);
  DBG_ENTER();
  
  SmsLeg *whichLeg = NULL_PTR;
  
   if (smsLeg1->isIdle())
       whichLeg = smsLeg1;

  waitingCallLeg = false;

  DBG_LEAVE();
  return (whichLeg);
}


bool
CCSessionHandler::setupTerm(VOIP_CALL_HANDLE voipCallHandle,
                            int              cdrIndex      ,
                            int              callRefValue  ,
                            bool             isFirst       )
{
  DBG_FUNC("CCSessionHandler::setupTerm", CC_SESSION_LAYER);
  DBG_ENTER();

  bool waitingCallLeg = false    ;
  bool returnValue    = true     ;

  CallLeg *whichLeg;

  if (isMmSetupTermActive() == true)
  {  // Only one MT MM connection setup in progress is allowed per MS
     // Second attempt is rejected
      whichLeg = NULL;
      DBG_TRACE ("CM Log: second MT setup attempt while ongoing MT setup (%s) in progress\n",
                 (voipCallHandle != VOIP_NULL_CALL_HANDLE) ? "Voice Call" : "SMS");
  }
  else if (voipCallHandle != VOIP_NULL_CALL_HANDLE)
      // Get termination leg for the Voice call
      whichLeg = getCallLegForNewTermination (waitingCallLeg);
  else
      // Get termination for the SMS 
      whichLeg = getSmsLegForNewTermination  (waitingCallLeg);

  if (whichLeg != NULL)
  {
      if (isFirst)
          setMmTermSetup (whichLeg); // Mark beginning of the MM Connection Setup

      // Invoke virtual function
      whichLeg->setupTerm (voipCallHandle, cdrIndex, callRefValue, waitingCallLeg);

      if ( waitingCallLeg && (!isFirst) )
      { 
           // CDR <xxu:08-21-00> BEGIN: for Voice call only
           CCdrSSA  cwCdr;
           if (ludbGetSubscInfoExists(ludbIndex))
           {
               cwCdr.setServedIMSI(ludbGetMobileIdPtr(ludbIndex));
               cwCdr.setServedMSISDN(ludbGetMSISDNPtr(ludbIndex));
           }
           cwCdr.setCorrelatedEventRefNumber(whichLeg->ccCdr.getEventRefNumber());
           cwCdr.generateRecord(CDR_SS_ACTION_INVOCATION,SUPP_SERV_CW,CNI_RIL3_CAUSE_SUCCESS);
           // CDR <xxu:08-21-00> END
      }
      returnValue = true;
  }
  else if (voipCallHandle == VOIP_NULL_CALL_HANDLE)
  { // SMS MT setup during ongoing MT MM connection esteblishment
      returnValue = false;
  }
  else
  {   // Either MT call setup during ongoing MT MM connection esteblishment or 
      // the call is rejected
  
      // print out the trx, timeslot for better debugging
      UINT16 gsmHandler;
      UINT8 trx, slot;
      T_SUBSC_IE_ISDN* isdnPtr;
      
      if (rm_EntryIdToTrxSlot(entryId, 
                              &gsmHandler) ==
          RM_TRUE)
        {
          
          trx  = (UINT8)(gsmHandler>>8);
          slot = (UINT8)(gsmHandler);
          
          if (callTrace)
            {
              DBG_WARNING("CC Log: User Busy on (TRX=%d),(Slot=%d),(entryId=%d)\n",
                          trx, slot, entryId);
              if ((isdnPtr = ludbGetMSISDNPtr(ludbIndex)) != NULL)
                {
                  DBG_WARNING("CC Log: MS (DN=%s)\n",
                              isdnPtr->digits);
                }
            }
          else
            {
              DBG_TRACE("CC Log: User Busy on (TRX=%d),(Slot=%d),(entryId=%d)\n",
                        trx, slot, entryId);
              if ((isdnPtr = ludbGetMSISDNPtr(ludbIndex)) != NULL)
                {
                  DBG_TRACE("CC Log: MS (DN=%s)\n",
                            isdnPtr->digits);
                }
            }
        }
      
      returnValue = false;
  }

  DBG_LEAVE();
  return(returnValue);
}

bool
CCSessionHandler::setupTargetHandover(void)
{
  DBG_FUNC("CCSessionHandler::setupTargetHandover", CC_SESSION_LAYER);
  DBG_ENTER();

  handoverSession = targetHandoverSession;

  DBG_LEAVE();
  return(true);

}

//ext-HO <xxu:05-16-01>
bool
CCSessionHandler::setupTargetHandoverExtHo(void)
{
  DBG_TRACEei("{\nMNETeiTRACE(cs::setupTargetHandoverExtHo): entering......\n}\n");
  
  handoverSession = targetHandoverSessionExtHo;

  return(true);

}

bool
CCSessionHandler::findHandle(VOIP_CALL_HANDLE voipCallHandle)
{
  DBG_FUNC("CCSessionHandler::findHandle", CC_SESSION_LAYER);
  DBG_ENTER();

  if (((!callLeg1->idle) && (callLeg1->h323CallHandle == voipCallHandle)) ||
      ((!callLeg2->idle) && (callLeg2->h323CallHandle == voipCallHandle)))
    {
      DBG_LEAVE();
      return(true);
    }

  DBG_LEAVE();
  return(false);

}

bool
CCSessionHandler::routeFromVoipToCallLeg(IntraL3Msg_t *h323InMsg,
                                         TwoPartyCallLeg *&whichLeg)
{
  DBG_FUNC("CCSessionHandler::routeFromVoipToCallLeg", CC_SESSION_LAYER);
  DBG_ENTER();

  bool ret;  //BCT <xxu:09-21-00>
  ret = false;

  whichLeg = NULL;

  if ((!callLeg1->idle) && 
      ((callLeg1->h323CallHandle == h323InMsg->call_handle) ||
       (callLeg1->h323CallHandle == VOIP_NULL_CALL_HANDLE)))
    {
      whichLeg = callLeg1;
      callLeg1->h323CallHandle = h323InMsg->call_handle;
      ret = true; //BCT <xxu:09-21-00>
    }
  else if ((!callLeg2->idle) && 
           ((callLeg2->h323CallHandle == h323InMsg->call_handle) ||
            (callLeg2->h323CallHandle == VOIP_NULL_CALL_HANDLE)))
    {
      whichLeg = callLeg2;
      callLeg2->h323CallHandle = h323InMsg->call_handle;
      ret = true; //BCT <xxu:09-21-00>
    }
  
  // This is a VOIP error. Put in a temporary fix such that we are not left with half calls
  else if ((h323InMsg->entry_id = findCallUsingCallHandle( h323InMsg->call_handle))
           == (CC_MAX_CC_CALLS))
    {
      DBG_WARNING("H323->CC: Late Msg(%d) h323CallHandle(%p) SessionId(%d)\n",
                   h323InMsg->message_type,
                  (int)h323InMsg->call_handle,
                  parent->callIndex );
    }
  else
    {
      if (ERROR == msgQSend( ccSession[h323InMsg->entry_id].msgQId,
                             (char *) h323InMsg, 
                             sizeof(struct  IntraL3Msg_t), 
                             NO_WAIT,		//PR1553 <xxu> WAIT_FOREVER
                             MSG_PRI_NORMAL
                             ) )
        {
          DBG_ERROR("CC Call -> CC Call Error : sendCC msgQSend (QID = %p) error\n ", 
                    (int)ccSession[h323InMsg->entry_id].msgQId);
          
        }
    }

  //BCT <xxu:09-21-00> BEGIN
  if ( (whichLeg != NULL) && 
       (h323InMsg->message_type == INTRA_L3_CC_BCT_UPDATE_HANDLES) )
  { 
        whichLeg->bctArrival_ = true;
        whichLeg->oH323CallHandle = h323InMsg->call_handle;
        whichLeg->h323CallHandle = h323InMsg->l3_data.voipMsg.callUpdateHandles.NewHandle;
    
        //break whatever traffic connections sunk to old rtp handler
        bool hoSimplexConnected = false;
		bool h3SimplexConnected = false;
		
        if (!h323InMsg->l3_data.voipMsg.callUpdateHandles.ReconnectOnTransferFailure)
		{
            if ( (handoverSession != NULL)&&(handoverSession == anchorHandoverSession) )
			{
			    hoSimplexConnected = anchorHandoverSession->hoSimplexConnected_;
  		        h3SimplexConnected = anchorHandoverSession->h3SimplexConnected_;
				anchorHandoverSession->hoSimplexConnected_ = false;
				anchorHandoverSession->h3SimplexConnected_ = false;
			}
			 
            if ( whichLeg->voiceConnected || hoSimplexConnected || h3SimplexConnected )
			{
                 T_CSU_PORT_ID rtpSourcePort;
                 T_CSU_RESULT_CON csuResult;
          
                 rtpSourcePort.portType = CSU_RTP_PORT;
                 rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->oH323CallHandle);
          
                 if ( (csuResult = csu_OrigTermBreakAll(&rtpSourcePort) )
                      != CSU_RESULT_CON_SUCCESS)
				 {
                       DBG_ERROR("CC->CSU Error(bct): Disconn Failed (Result=%d)for(RTPHandle=%p)\n",  
                                  csuResult, rtpSourcePort.portId.rtpHandler);
                       //Can only generate OAM report
				 }
			}

			whichLeg->voiceConnected = false;
		} else
		{
		    //Reconnect the call between transferring and transferred party due to BCT failure
			if (whichLeg->msSide->chState_!=CH_ST_HELD)
			{
                T_CSU_PORT_ID srcPort, snkPort;
                T_CSU_RESULT_CON result;

				srcPort.portType = CSU_RTP_PORT;
                srcPort.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->h323CallHandle);
				snkPort.portType = CSU_NIL_PORT;
          
				if (handoverSession==NULL)
				{
					//connect to gsm port
					snkPort.portType = CSU_GSM_PORT;
					snkPort.portId.gsmHandler = entryId;
				}

				if ((handoverSession!=NULL)&&(handoverSession==anchorHandoverSession)&&
					(anchorHandoverSession->hoState_==HAND_ST_ANCH_HO_COMPLT) )
				{
					//connect to rtp port
					snkPort.portType = CSU_RTP_PORT;
					snkPort.portId.rtpHandler = anchorHandoverSession->hoRtpSession_;
					
				}

				if ((handoverSession!=NULL)&&(handoverSession==anchorHandoverSession)&&
					(anchorHandoverSession->hoState_==HAND_ST_ANCH_EHO_COMPLT) )
				{
					//connect to rtp port
					snkPort.portType = CSU_RTP_PORT;
					snkPort.portId.rtpHandler = VoipCallGetRTPHandle(hoCallLeg->h323CallHandle);
					
				}

				if (snkPort.portType!=CSU_NIL_PORT)
				{
					//make duplex connection
				    if ((result = csu_DuplexConnect(&srcPort, &snkPort))
						!= CSU_RESULT_CON_SUCCESS)
					{
	       	  	        DBG_ERROR("CSU Error(BCT): duplex conn (Result=%d) for src(%d,%x) snk(%d,%x)\n",
			                       result,
								   srcPort.portType, srcPort.portId.rtpHandler,
								   snkPort.portType, snkPort.portId.rtpHandler );
					}
				}

				whichLeg->voiceConnected = true;

			}
		}
		
   }

   //BCT <xxu:09-21-00> END
  
   DBG_LEAVE();
   return(ret);
}

SmsLeg * CCSessionHandler::findSmsReference (UINT32 refNum)
{
  SmsLeg * whichLeg = NULL_PTR;

  DBG_FUNC ("CCSessionHandler::findSmsReference", CC_SESSION_LAYER);
  DBG_ENTER();

    if (refNum == smsLeg1->getVbRefNum())
        whichLeg = smsLeg1;
    else if (refNum == smsLeg2->getVbRefNum())
        whichLeg = smsLeg2;
    else
        whichLeg = NULL_PTR; /* If there is no SMS leg to forward the message
                                return NULL_PTR */
  DBG_LEAVE();
  return   (whichLeg);
}


bool
CCSessionHandler::routeFromVoipToSmsLeg (IntraL3Msg_t * h323InMsg,
                                         SmsLeg       *&whichLeg )
{
  DBG_FUNC ("CCSessionHandler::routeFromVoipToSmsLeg", CC_SESSION_LAYER);
  DBG_ENTER();

    SMS_VBLINK_MSGTYPE type = (SMS_VBLINK_MSGTYPE) h323InMsg->l3_data.smsVbMsg.hdr.msgType;

    whichLeg = NULL_PTR;

    if (type == MNSMS_EST_Req) // always forwarded to the MT Setup
    {
        /* Race condition could happend here: 
           MNSMS-EST Req is sent to the active MS but by the time Call task is ready to 
           process the message all the activities are complited and the session is released */

        whichLeg = smsLeg1;
    }
    else                       // Otherwise, is routed according to Vb Reference number
        whichLeg = findSmsReference (h323InMsg->l3_data.smsVbMsg.hdr.refNum);

  DBG_LEAVE();
  return   (!(whichLeg == NULL_PTR));
}


bool
CCSessionHandler::routeFromMsToCallLeg(T_CNI_RIL3CC_MSG      *msInDecodedMsg,
                                       TwoPartyCallLeg       *&whichLeg)
{
  DBG_FUNC("CCSessionHandler::routeFromMsToCallLeg", CC_SESSION_LAYER);
  DBG_ENTER();

  whichLeg = NULL;

  if ((!callLeg1->idle) && (callLeg1->msSide != NULL))
    {
      if (((callLeg1->msSide->callState_ == HC_ST_NULL) &&
           ((msInDecodedMsg->header.message_type == CNI_RIL3CC_MSGID_SETUP) ||
            (msInDecodedMsg->header.message_type == CNI_RIL3CC_MSGID_EMERGENCY_SETUP)) && // can we get a disconnect here?
           (callLeg1->procType == CC_PROC_MOB_ORIG))|| // waiting for origination setup
          (((msInDecodedMsg->header.si_ti >> 3) == 0) && // Mobile Origination
           (callLeg1->msSide->ti == (msInDecodedMsg->header.si_ti | ORIG_OR_MASK)) &&
           (callLeg1->procType == CC_PROC_MOB_ORIG)) || // any origination protocol message
          (((msInDecodedMsg->header.si_ti >> 3) == 1) && // Mobile Termination
           (callLeg1->msSide->ti == (msInDecodedMsg->header.si_ti & TERM_AND_MASK)) &&
           (callLeg1->procType == CC_PROC_MOB_TERM))) // any termination protocol message
        
        {
          whichLeg = callLeg1;
          DBG_LEAVE();
          return(true);
        }
    }

  if ((!callLeg2->idle) && (callLeg2->msSide != NULL))
    {
      if ((((msInDecodedMsg->header.si_ti >> 3) == 0) && // Mobile Origination
           (callLeg2->msSide->ti == (msInDecodedMsg->header.si_ti | ORIG_OR_MASK)) &&
           (callLeg2->procType == CC_PROC_MOB_ORIG)) || // any origination protocol message
          (((msInDecodedMsg->header.si_ti >> 3) == 1) && // Mobile Termination
           (callLeg2->msSide->ti == (msInDecodedMsg->header.si_ti & TERM_AND_MASK)) &&
           (callLeg2->procType == CC_PROC_MOB_TERM))) // any termination protocol message
        {
          whichLeg = callLeg2;
          DBG_LEAVE();
          return(true);
        }
    }

  // Take care of the setup situations
  if ( (msInDecodedMsg->header.message_type == CNI_RIL3CC_MSGID_SETUP) ||
       (msInDecodedMsg->header.message_type == CNI_RIL3CC_MSGID_EMERGENCY_SETUP) )
  {
       //BCT <xxu:07-12-00> BEGIN
         
       if ( ( (!callLeg1->idle) && (!callLeg2->idle)) ||
              (bctInvoke_) ||
              ((handoverSession != NULL) && 
                    ( (handoverSession->hoState_ != HAND_ST_ANCH_HO_COMPLT) &&
                      (handoverSession->hoState_ != HAND_ST_ANCH_EHO_COMPLT) ) ) )
       {
            //Ignore any subsequent call setup while a BCT is in progress or both call leg in use
            //"handoverSession" supposed to be NULL while receiving a bct SETUP

            DBG_WARNING("MS->CC Warning: block bct call setup for sessionId(%d) due to (bctInv %d,hoSession %p)\n",
                         parent->callIndex, bctInvoke_, handoverSession);

            callLeg3->setupOrig(msInDecodedMsg->header.si_ti);

            callLeg3->msSide->handleMobMsg(msInDecodedMsg,
                                           CNI_RIL3_RESULT_SUCCESS, // decoding result
                                           CNI_RIL3_CAUSE_RESOURCES_UNAVAILABLE_UNSPECIFIED,
                                           HC_LOCAL_RELEASE_REQ);   // release request
            DBG_LEAVE();
            return(false);
       } else
       {
            if ( (msInDecodedMsg->header.message_type == CNI_RIL3CC_MSGID_SETUP) &&
                 (!memcmp(msInDecodedMsg->setup.calledBCD.digits, CALL_BCT_PREFIX, CALL_BCT_PREFIX_LEN))  )
            {
                  //BCT invocation, and check if BCT is provisioned

                  DBG_TRACE("CC Call Log: (bct prefix = %d,%d,%d,%d)\n",
                             msInDecodedMsg->setup.calledBCD.digits[0],
                             msInDecodedMsg->setup.calledBCD.digits[1],
                             msInDecodedMsg->setup.calledBCD.digits[2],
                             msInDecodedMsg->setup.calledBCD.digits[3]);

                  msInDecodedMsg->setup.calledBCD.numDigits -= CALL_BCT_PREFIX_LEN;
                  memcpy(msInDecodedMsg->setup.calledBCD.digits,
                         &msInDecodedMsg->setup.calledBCD.digits[CALL_BCT_PREFIX_LEN],
                         msInDecodedMsg->setup.calledBCD.numDigits);

                  DBG_TRACE("CC Call Log: (bct-to number = %d,%d,%d,%d)\n",
                             msInDecodedMsg->setup.calledBCD.digits[0],
                             msInDecodedMsg->setup.calledBCD.digits[1],
                             msInDecodedMsg->setup.calledBCD.digits[2],
                             msInDecodedMsg->setup.calledBCD.digits[3]);

                  if ( !ludbIsProvECT(ludbIndex) )
                  //if (0)
                  {
                       // Handle its release through the placeholder for the third call leg.
 
                       DBG_TRACE("LUDB->CC BCT: not provisioned for (ludbIndex = %d, sessionId = %d)\n",
                                  ludbIndex, parent->callIndex);
 
                       callLeg3->setupOrig(msInDecodedMsg->header.si_ti);

                       callLeg3->msSide->handleMobMsg(msInDecodedMsg,
                                                      CNI_RIL3_RESULT_SUCCESS, // decoding result
                                                      CNI_RIL3_CAUSE_REQUESTED_SERVICE_NOT_SUBSCRIBED	,
                                                      HC_LOCAL_RELEASE_REQ);   // release request
                       DBG_LEAVE();
                       return(false);
                   }

                  //BCT race condition
                  if (getBctArrival())
                  {
                       // Handle its release through the placeholder for the third call leg.
                        DBG_TRACE("CC->BCT: a bct call arriving, block bct invoke(ludbId=%d, sessId=%d)\n",
                                  ludbIndex, parent->callIndex);
 
                       callLeg3->setupOrig(msInDecodedMsg->header.si_ti);

                       callLeg3->msSide->handleMobMsg(msInDecodedMsg,
                                                      CNI_RIL3_RESULT_SUCCESS, // decoding result
                                                      CNI_RIL3_CAUSE_TEMPORARY_FAILURE,
                                                      HC_LOCAL_RELEASE_REQ);   // release request
                       DBG_LEAVE();
                       return(false);
                   }

                   bctInvoke_ = true;
            }     

            TwoPartyCallLeg  *otherLeg;

            if ( callLeg1->idle ) { whichLeg = callLeg1; otherLeg = callLeg2; }
            else                  { whichLeg = callLeg2; otherLeg = callLeg1; }
            
            if ( bctInvoke_ )
            {
                 if ( otherLeg->msSide->callState_ != HC_ST_ACTIVE )
                 {
                      // Reject this BCT invocation since the call to bct is being released now

                      bctInvoke_ = false;
  
                      DBG_TRACE("CC BCT: call to bct is being released now (ludbIndex = %d, sessionId = %d)\n",
                                 ludbIndex, parent->callIndex);

                      callLeg3->setupOrig(msInDecodedMsg->header.si_ti);
   
                      callLeg3->msSide->handleMobMsg(msInDecodedMsg,
                                                     CNI_RIL3_RESULT_SUCCESS, // decoding result
                                                     CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE,
                                                     HC_LOCAL_RELEASE_REQ);   // release request
                      DBG_LEAVE();
                      return(false);
                 } else
                 {
                      //Inquire VB-TTF function for xfered-to number
                      T_SUBSC_IE_ISDN* isdnPtr;
                      if ((isdnPtr = ludbGetMSISDNPtr(ludbIndex)) != NULL)
                      {
                           //DBG_TRACE("CC Log: MS (DN=%s)\n",
                           //           isdnPtr->digits);
                           msInDecodedMsg->setup.callingBCD.ie_present = true;
                           msInDecodedMsg->setup.callingBCD.numberType = CNI_RIL3_NUMBER_TYPE_INTERNATIONAL;
                           msInDecodedMsg->setup.callingBCD.numberingPlan = (T_CNI_RIL3_NUMBERING_PLAN_ID)isdnPtr->num_plan[0];
                           msInDecodedMsg->setup.callingBCD.numDigits = isdnPtr->num_digit;
                           for (int i=0;i<isdnPtr->num_digit;i++)
                           {
                                if ( isdnPtr->digits[i] >= 0x30 && isdnPtr->digits[i] < 0x40 )
                                     msInDecodedMsg->setup.callingBCD.digits[i] = isdnPtr->digits[i] - 0x30;
                                else
                                     DBG_ERROR("CC LOG LUDB(bct): invalid msisdn digit (%d,%c)\n",
                                                i, (char) isdnPtr->digits[i]);
                           }
                           DBG_TRACE("CC Call Log: (calling Party BCD = %s)\n",
                                      msInDecodedMsg->setup.callingBCD.digits);
                      } 


                      if (!VBLinkTransferRequest(parent->callIndex,
                                                 msInDecodedMsg->setup.calledBCD,
                                                 msInDecodedMsg->setup.callingBCD))
                      {
                          // Reject this BCT invocation since the call to bct is being released now
                          DBG_ERROR ("CC->VB Link Err(bct): Failed on VBLinkTransferRequest\n");

                          bctInvoke_ = false;
  
                          DBG_TRACE("CC BCT: call to bct is being released now (ludbIndex = %d, sessionId = %d)\n",
                                     ludbIndex, parent->callIndex);

                          callLeg3->setupOrig(msInDecodedMsg->header.si_ti);
   
                          callLeg3->msSide->handleMobMsg(msInDecodedMsg,
                                                     CNI_RIL3_RESULT_SUCCESS, // decoding result
                                                     CNI_RIL3_CAUSE_REQUESTED_SERVICE_NOT_SUBSCRIBED	,
                                                     HC_LOCAL_RELEASE_REQ);   // release request
                          DBG_LEAVE();
                          return(false);
                     }

                     //Go ahead to do bct
                     DBG_TRACE("CC Call Log: (calling Party BCD = %d,%d,%d,%d)\n",
                                msInDecodedMsg->setup.callingBCD.digits[0],
                                msInDecodedMsg->setup.callingBCD.digits[1],
                                msInDecodedMsg->setup.callingBCD.digits[2],
                                msInDecodedMsg->setup.callingBCD.digits[3]);

                     bctTi_ = (msInDecodedMsg->header.si_ti | ORIG_OR_MASK); 
                     bctLeg_ = whichLeg;
                     bctMainLeg_ = otherLeg;

                     CalledPartyNumber = msInDecodedMsg->setup.calledBCD;

 					 //BCTCalling Party Number shalle be transferred party's number
					 //CallingPartyNumber = msInDecodedMsg->setup.callingBCD;
					 if (bctMainLeg_->procType==CC_PROC_MOB_ORIG)
					 {
						 CallingPartyNumber.ie_present = true;
						 CallingPartyNumber.numberType = bctMainLeg_->msSide->cpn.numberType;
						 CallingPartyNumber.numberingPlan = bctMainLeg_->msSide->cpn.numberingPlan;
						 CallingPartyNumber.presentationInd = CNI_RIL3_PRESENTATION_ALLOWED;
                         CallingPartyNumber.screeningInd = CNI_RIL3_SCREENING_NETWORK_PROVIDED; //TBD
                         CallingPartyNumber.numDigits = bctMainLeg_->msSide->cpn.numDigits;
                         memcpy(CallingPartyNumber.digits,bctMainLeg_->msSide->cpn.digits,bctMainLeg_->msSide->cpn.numDigits);
					 }
					 else
                         CallingPartyNumber = bctMainLeg_->msSide->callingPartyNum;

		        }
            } 

            whichLeg->setupOrig(msInDecodedMsg->header.si_ti);
            DBG_LEAVE();
            return(true);
       }

       //BCT <xxu:07-12-00> END
  }

  DBG_WARNING("MS->CC Warning: Late Msg. for (ti = %p) (Session Id = %d)\n",
               msInDecodedMsg->header.si_ti,
               parent->callIndex );

  DBG_LEAVE();
  return(false);
}

bool CCSessionHandler::routeFromMsToSmsLeg (T_CNI_RIL3_CP_HEADER& msInDecodedHdr, SmsLeg *&whichLeg)
{
    DBG_FUNC("CCSessionHandler::routeFromMsToSmsLeg", CC_SESSION_LAYER);
    DBG_ENTER();

    unsigned newTi = msInDecodedHdr.si_ti;

    if (newTi <= 7                                                       ||
        (newTi == smsLeg2->getTi() & TI_TERM_AND_MASK &&
         (smsLeg1->getTi() == SMS_TI_INVALID || newTi != smsLeg1->getTi()))) //CP-ERROR correction
        whichLeg = smsLeg2;  // Transaction is originated by MS
    else
        whichLeg = smsLeg1;

    DBG_LEAVE();
    return (false);
}


void
CCSessionHandler::cleanup (T_CNI_RIL3_CAUSE_VALUE cause, BOOL normClear)
{
  // This function has defaul value defined for the both parameters.
  // T_CNI_RIL3_CAUSE_VALUE cause     = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING
  // BOOL                   normClear = TRUE,
  //  should be set FALSE only when called as a result of remove ViperCell failure

  BOOL isMigratedCall = FALSE;

  DBG_FUNC  ("CCSessionHandler::cleanup", CC_SESSION_LAYER);
  DBG_ENTER ();

  // <Igal: 04-10-01>
  if (handoverSession != NULL)
  {
    isMigratedCall = handoverSession->cleanup (normClear);
  }

  if (normClear == TRUE || isMigratedCall == TRUE)
    {
      // Either normal call session release or migrated call release

      // Otherwise, if the reason is external ViperCell failure:
      // Incomplete HO is canceled due to failure of the target ViperCell (above).
      // Call session continues on the current cell.

      if (entryId != (T_CNI_IRT_ID) JCC_UNDEFINED )
        {
          IntraL3Msg_t mmOutMsg    ;
          T_CNI_IRT_ID tempEntryId = entryId;

          // Send internal MM msg - no data needed
          sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, entryId, &mmOutMsg);

          resetIrt ();

          // do not set entryId to null here. 
          // It might be used below in one of the components
        }
      callLeg1->cleanup    (cause);
      callLeg2->cleanup    (cause);

      //ext-HO <xxu:06-07-01>
      hoCallLeg->cleanup   (cause);
      h3CallLeg->cleanup   (cause);

      cissCallLeg->cleanup (cause);

      if (smsLeg1)
          smsLeg1->cleanup (cause);
      if (smsLeg2)
          smsLeg2->cleanup (cause);
      if (smsLeg3)
          smsLeg3->cleanup (cause);

      // set entryId to null here
      entryId = (T_CNI_IRT_ID) JCC_UNDEFINED;
      initData();
    }

  DBG_LEAVE();

}

int
CCSessionHandler::handleMMMsg(IntraL3Msg_t    *mmMsg)
{
  DBG_FUNC("CCSessionHandler::handleMMMsg", CC_SESSION_LAYER);
  DBG_ENTER();

  int        retVal   ;
  bool       hoEvent  ;
  JCCEvent_t currEvent;

  mmInMsg = mmMsg;
 
  switch (mmInMsg->primitive_type)
    {
    case INTRA_L3_MM_EST_CNF:

      switch (mmInMsg->message_type)
        {
        case INTRA_L3_RR_PAGE_RSP:
          currEvent = CC_MM_CONNECTION_CNF;
          hoEvent = false;
          break;

        case INTRA_L3_RR_HANDOVER_REQ_ACK:
          currEvent = HAND_TRG_HO_REQ_ACK;
          hoEvent = true;
          break;

        default:
          DBG_ERROR("CC Session Error: Unexpected (Msg Type = %d) from MM.\n",  
                    mmInMsg->message_type);
          currEvent = JCC_NULL_EVENT;

        }
      break;

    case INTRA_L3_MM_EST_IND:
      {
          currEvent = CC_MM_CONNECTION_IND;
          hoEvent = false;
      }
      break;

    case INTRA_L3_MM_EST_REJ:
      switch (mmInMsg->message_type)
        {
        case INTRA_L3_RR_PAGE_REQ:
          currEvent = CC_MM_CONNECTION_REJ;
          hoEvent = false;
          break;

        case INTRA_L3_RR_HANDOVER_REQ_NACK:
          currEvent = HAND_TRG_HO_REQ_NACK;
          hoEvent = true;
          break;

        default:
          DBG_ERROR("CC Call Error: Unexpected (Msg Type = %d) from MM.\n",  
                    mmInMsg->message_type);
          currEvent = JCC_NULL_EVENT;

        }
      break;


    case INTRA_L3_MM_REL_IND:
      currEvent = CC_MM_CONNECTION_REL;
      hoEvent = false;
      break;

    default:
      DBG_ERROR("CC Call Error: Unexpected (Msg Primitive = %d) from MM.\n",  
                mmInMsg->primitive_type);
      currEvent = JCC_NULL_EVENT;

    }

  // For call events:
  if (currEvent != JCC_NULL_EVENT)
    {
      // For Handover events:
      if (hoEvent)
        {
          if (handoverSession != NULL)
            {
              int causeValue;

              if (handoverSession == anchorHandoverSession)
                {
                  causeValue = anchorHandoverSession->handleMMMsg(mmMsg, currEvent);
                }
              
			  //ext-HO <xxu:06-08-01>
              else if (handoverSession == targetHandoverSession)
                {
                  causeValue = targetHandoverSession->handleMMMsg(mmMsg, currEvent);

			  } else if (handoverSession == targetHandoverSessionExtHo)
			  {
				  causeValue = targetHandoverSessionExtHo->handleMMMsg(mmMsg, currEvent);
			  } else
			  {
				  DBG_ERROR("{\nMNEThoERROR(cs::handleMMMsg): invalid handoverSession (hoSession=%p, anchSession=%p, trgSession=%p, trgSessionExtHo=%p)\n}\n",
					  handoverSession, anchorHandoverSession, targetHandoverSession, targetHandoverSessionExtHo);
				  causeValue = -1;
			  }



              if (causeValue != 0)
                {
                  retVal = smHandleEvent(CC_RELEASE_CALL)

                  DBG_LEAVE();
                  return   (retVal);
                }
              else
                {
                  DBG_LEAVE();
                  return (0);
                }
            }

          if (currEvent == HAND_TRG_HO_REQ_ACK)
            {
              // handle the case where a late establish confirm comes in 
              // for a handover request that was previously sent.
              IntraL3Msg_t      mmOutMsg;
              sendMM   (INTRA_L3_MM_REL_REQ,
                        INTRA_L3_NULL_MSG  ,
                        mmMsg->entry_id    ,
                        &mmOutMsg          );
            }
          else
            {
              DBG_WARNING("CC Call Warning: Late Handover Msg? (MM Event=%d), (session id=%d).\n",
                          currEvent,
                          parent->callIndex);
            }

          DBG_LEAVE();
          return   (0);
        }

      else
      { // non-handover events MM handling
          int causeValue;

          if (handoverSession == targetHandoverSession &&
              currEvent == CC_MM_CONNECTION_IND          )
          {
              /* Forwarding MM-EST Ind to the anchor side */

              DBG_TRACE ("CM Session: Forward secondary MM-EST Ind to the anchor.\n");

              sendMMMsg (mmMsg);
              retVal = 0;
          
		  //ext-HO <xxu:07-01-01> 
          } else if ((handoverSession == targetHandoverSessionExtHo) &&
                     (currEvent == CC_MM_CONNECTION_IND            ))
		  {
			  causeValue = targetHandoverSessionExtHo->handleMMMsg(mmMsg, currEvent);
			  if (causeValue) 
				  retVal = smHandleEvent(CC_RELEASE_CALL);
		  }

          else
              retVal = smHandleEvent (currEvent);
      }

      DBG_LEAVE();
      return   (retVal);
    }

  DBG_LEAVE();
  return   (0);
}

int
CCSessionHandler::handleRRMsg(IntraL3Msg_t      *rrMsg)
{
  DBG_FUNC("CCSessionHandler::handleRRMsg", CC_SESSION_LAYER);
  DBG_ENTER();

  bool hoEvent;
  JCCEvent_t currEvent;

  rrInMsg = rrMsg;

  switch (rrInMsg->message_type)
    {
    case INTRA_L3_RR_CHANN_ASSIGN_COM:
      currEvent = CC_RESOURCE_ASSIGN_COMPLT;
      hoEvent = false;
      break;
      
    case INTRA_L3_RR_CHANN_ASSIGN_FAIL:
      currEvent = CC_RESOURCE_ASSIGN_FAIL;
      hoEvent = false;
      break;
      
    case INTRA_L3_RR_HANDOVER_REQUIRED:
      currEvent = JCC_NULL_EVENT;

      if (handoverSession == NULL)
        {
          // Ignore any Handover Required messages in anchor, if they need to be blocked.
          if (!isHandoverAllowed())
            {
              break;
            }

          handoverSession = anchorHandoverSession;

          //SAPI=3 connection is local. Reset the flag
          resetSapi3 ();
        }

      DBG_TRACE("CC->Handover Info: received Intra L3 RR Handover Required message\n");
      //printData(JCCPrintf);
      currEvent = HAND_SRC_HO_REQD;
      hoEvent = true;
      break;

    case INTRA_L3_RR_HANDOVER_COMPLETE:
      currEvent = HAND_TRG_HO_COMPLT;
      hoEvent = true;
      break;

	//ext-HO <xxu:06-08-01>
    case INTRA_L3_RR_HANDOVER_ACCESS:
      currEvent = HAND_HO_ACCESS;
      hoEvent = true;
      break;


    case INTRA_L3_RR_HANDOVER_FAILURE:
      currEvent = HAND_SRC_HO_FAIL;
      PM_CCMeasurement.unsuccExternHDOsWithReconnectionPerMSC.increment();
      hoEvent = true;
      break;
      
    default:
      DBG_ERROR("CC Call Error: Unexpected (Msg Type = %d) from RR.\n",  
                rrInMsg->message_type);
      currEvent = JCC_NULL_EVENT;
      
    }
  
  // For Channel States:
  if (currEvent != JCC_NULL_EVENT)
    {
      // For Handover States:
      if (hoEvent)
        {
          if (handoverSession != NULL)
            {
              int causeValue;
              
              if (handoverSession == anchorHandoverSession)
                {
                  causeValue = anchorHandoverSession->handleRRMsg(rrMsg, currEvent);
                }
			  //ext-HO <xxu:06-08-01>
              else if (handoverSession == targetHandoverSession)
                {
                  causeValue = targetHandoverSession->handleRRMsg(rrMsg, currEvent);

			  } else if (handoverSession == targetHandoverSessionExtHo)
			  {
				  causeValue = targetHandoverSessionExtHo->handleRRMsg(rrMsg, currEvent);
			  } else
			  {
				  DBG_ERROR("{\nMNEThoERROR(cs::handleRRMsg): invalid handoverSession (hoSession=%p, anchSession=%p, trgSession=%p, trgSessionExtHo=%p)\n}\n",
					  handoverSession, anchorHandoverSession, targetHandoverSession, targetHandoverSessionExtHo);
				  causeValue = -1;
			  }

              if (causeValue != 0)
                {
                  DBG_LEAVE();
                  return(smHandleEvent(CC_RELEASE_CALL));
                }
              else
                {
                  DBG_LEAVE();
                  return (0);
                }
                  
            }

          DBG_WARNING("CC Call Warning: Late Handover Msg? (RR Event=%d), (session id=%d).\n",
                      currEvent,
                      parent->callIndex);

          DBG_LEAVE();
          return(0);
          
        }
      else
        {
          // non-handover events
          int causeValue;

          TwoPartyCallLeg *whichLeg;

          if (handoverSession == targetHandoverSession)
            {
              // handover on target side.
              targetHandoverSession->sendRRMsgToAnchorCC(rrMsg);
              DBG_LEAVE();
              return(0);
            }

          switch (rrInMsg->primitive_type)
            {
            case 1:
              whichLeg = callLeg1;
              break;

            case 2:
              whichLeg = callLeg2;
              break;

            default:
              DBG_ERROR("RR->CC ERROR: Unexpected (Call Leg Num=%d) from RR , (event=%d) (session id=%d)\n",
                        rrInMsg->primitive_type,
                        currEvent,
                        parent->callIndex);
              DBG_LEAVE();
              return(0);
            }
          
          if ((!(whichLeg->idle)) && (whichLeg->msSide != NULL))
            {
              if ((causeValue = whichLeg->msSide->handleRRMsg(rrMsg, currEvent)) != 0)
                {
                  handleReleaseCallLeg(whichLeg);
                  DBG_LEAVE();
                  return(causeValue);
                }
            }
          else
            {
              // late event
              DBG_WARNING("RR->CC Warning: Unexpected/Late RR event for (call leg Num=%d), (event=%d) (session id=%d)\n",
                          rrInMsg->primitive_type,
                          currEvent,
                          parent->callIndex);
            }
        }
    }
  else
    {
    }

  //Do Nothing
  DBG_LEAVE();
  return(0);
}

int
CCSessionHandler::handleTimeoutMsg(IntraL3Msg_t    *ccInMsg)
{
  DBG_FUNC("CCSessionHandler::handleTimeoutMsg", CC_SESSION_LAYER);
  DBG_ENTER();

  CallLeg *whichLeg;
  // We already know that it is a timeout msg 

  switch (ccInMsg->entry_id)
    {
    case 0:
      whichLeg = cissCallLeg;
      break;
      
    case 1:
      whichLeg = callLeg1;
      break;
      
    case 2:
      whichLeg = callLeg2;
      break;
      
    case 3:
      whichLeg = NULL;
      break;

    case 4:
      whichLeg = smsLeg1;
      break;

    case 5:
      whichLeg = smsLeg2;
      break;

    case 6:
      whichLeg = smsLeg3;
      break;

    case 7:
      whichLeg = hoCallLeg;
      break;

    case 8:
      whichLeg = h3CallLeg;
      break;

    default:
      DBG_ERROR("CC Error: Invalid Timeout (call leg number=%d)\n",
                ccInMsg->entry_id);
      DBG_LEAVE();
      return(0);
    }      

  int causeValue;

  if (whichLeg != NULL)
    {
      // Not Session related
      if (whichLeg->idle == false)
        {
          if ((causeValue = whichLeg->handleTimeoutMsg(ccInMsg)) != 0)
            {
              handleReleaseCallLeg(whichLeg);
              DBG_LEAVE();
              
              return(causeValue);
            }
          else
            {
              DBG_LEAVE();
              return(0);
              
            }
        }
      
      // late event
      DBG_WARNING("CC Warning: Unexpected/Late Timeout event for (call leg=%d), (session id=%d)\n",
                  ccInMsg->entry_id,
                  parent->callIndex);
    }
  else
    {
      if (handoverSession != NULL)
        {
          int causeValue = 0;   //ext-HO <xxu:06-07-01>

          if (handoverSession == anchorHandoverSession)
            {
              causeValue = anchorHandoverSession->handleTimeoutMsg(ccInMsg);
            }
          else if (handoverSession == targetHandoverSession)
            {
              causeValue = targetHandoverSession->handleTimeoutMsg(ccInMsg);

		  } //ext-HO <xxu:06-07-01>
		  else if (handoverSession == targetHandoverSessionExtHo)
          {
              causeValue = targetHandoverSessionExtHo->handleTimeoutMsg(ccInMsg);
		  } else
		  {
              DBG_ERROR("CC Error: No handover session is linked to the fired session timer (callId=%d)\n",
				        parent->callIndex);
		  }
		  
          if (causeValue != 0)
            {
              DBG_LEAVE();
              return(smHandleEvent(CC_RELEASE_CALL));
            }
          else
            {
              DBG_LEAVE();
              return (0);
            }
        }
      // Call Hold feature goes here
    }

  DBG_LEAVE();
  return(0);
}

//ext-HO <xxu:05-16-01>
int
CCSessionHandler::handleHoaCcMsg (InterHoaVcMsg_t *ccInMsg)
{
  int causeValue = 0;

  DBG_FUNC("CCSessionHandler::handleHoaCcMsg", CC_HO_LAYER);
  DBG_ENTER();

  if (handoverSession != NULL)
  {
  	  if (handoverSession == targetHandoverSessionExtHo)
      {
		  causeValue = targetHandoverSessionExtHo->handleHoaCcMsg(ccInMsg);
		  
	  } else if (handoverSession == anchorHandoverSession)
      {
          causeValue = anchorHandoverSession->handleHoaCcMsg(ccInMsg);
      }
      else
      {
		  DBG_WARNING("{\nMNEThoWARNING(cs::handleHoaCcMsg): late ho msg from HOA (msgType=%d,callId=%d)\n}\n",
			             ccInMsg->msgType, parent->callIndex);
	  }
  
  } else
  {
	  DBG_WARNING("{\nMNEThoTRACE(cs::handleHoaCcMsg): late msg from HOA (msgType=%d,callId=%d,cause=%d)\n}\n",
			          ccInMsg->msgType, parent->callIndex);
  }

  if (causeValue != 0)
  {
 	  DBG_WARNING("{\nMNEThoWARNING(cs::handleHoaCcMsg):(cause=%d,msgType=%d,callId=%d)......\n}\n",
		             causeValue, ccInMsg->msgType, parent->callIndex);

	  cleanup();
	  //cleanup((T_CNI_RIL3_CAUSE_VALUE)causeValue,true);
  }
  

  DBG_LEAVE();
    
  return (0);
}

int
CCSessionHandler::handleVcCcMsg (InterVcMsg_t *vcCcInMsg)
{
  DBG_FUNC("CCSessionHandler::handleVcCcMsg", CC_SESSION_LAYER);
  DBG_ENTER();

  if (handoverSession != NULL)
  {
      int causeValue = 0;
      
      if (handoverSession == anchorHandoverSession)
      {
          causeValue = anchorHandoverSession->handleVcCcMsg(vcCcInMsg);
      }
	  //ext-HO <xxu:05-17-01> BEGIN
      else if (handoverSession == targetHandoverSessionExtHo)
	  {
		  //This is only expected for post-handin scenario and this GP10 functions as anchor for the remote GP10 and
		  //as target for the PLMN anchor MSC.

		  DBG_TRACEei("{\nMNETeiTRACE(cs::handleVcCcMsg): recv a message from another GP (msgType=%d, callId=%d):\n",
	  		        vcCcInMsg->msgType, parent->callIndex);
		  DBG_TRACEei("                                 : origGP(IPAddr=%x, subId=%d) DestGP(IPAddr=%x,subId=%d)\n}\n",
			        vcCcInMsg->origVcAddress, vcCcInMsg->origSubId,
			        vcCcInMsg->destVcAddress, vcCcInMsg->destSubId );

		  causeValue = targetHandoverSessionExtHo->handleVcCcMsg(vcCcInMsg);

      } //ext-HO <xxu:05-17-01> END
	  else
      {   // <Igal 1-6-00> Updated for handling PDUs forwarded between ViperCells after HO
          if (vcCcInMsg->msgType == INTER_VC_CC_POST_HANDOVER_DATA_EVENT)
          { // PDU forwarded from anchor side CM to target LAPDm via CM Session
              sendL2Msg (&vcCcInMsg->msgData.postHoMobEventData.msEncodedMsg);
              causeValue = 0;
          }
          else
              causeValue = targetHandoverSession->handleVcCcMsg(vcCcInMsg);
      }

      if (causeValue != 0)
      {
          DBG_LEAVE();
          return   (smHandleEvent(CC_RELEASE_CALL));
      }
      else
      {
          DBG_LEAVE();
          return (0);
      }
  }

  DBG_WARNING ("CC Call Warning: Late Handover Msg from other ViperCell, (session id=%d).\n",
               parent->callIndex);

  DBG_LEAVE();
  return   (0);
}


int
CCSessionHandler::handleVbCissMsg(CISSVblinkMsg_t &cissInMsg)
{
  DBG_FUNC("CCSessionHandler::handleVbCissMsg", CC_SESSION_LAYER);
  DBG_ENTER();

  if (!cissCallLeg->idle)
    {
      int causeValue;
      if ((causeValue = cissCallLeg->handleVbMsg(cissInMsg)) != 0)
        {
          handleReleaseCallLeg(cissCallLeg);
          DBG_LEAVE();
          return(causeValue);
        }
      else
        {
          DBG_LEAVE();
          return(0);
        }
    }

  // late event
  DBG_WARNING("MS->CC Warning: Unexpected/Late MS CISS event for (session id=%d)\n",
              parent->callIndex);
  DBG_LEAVE();
  return(0);
}

//CH<xxu:11-22-99>
int
CCSessionHandler::handleMobHoldRetvReq(JCCEvent_t& event, TwoPartyCallLeg* leg)
{
  TwoPartyCallLeg *otherLeg;
  
  DBG_FUNC("handleMobHoldRetvReq", CC_SESSION_LAYER);
  DBG_ENTER();

  //CDR <xxu:08-21-00> BEGIN
  CCdrSSA	chCdr;
  if ( ludbGetSubscInfoExists(ludbIndex) && !isEmergencyCall && leg != NULL)
  {
     chCdr.setServedIMSI(ludbGetMobileIdPtr(ludbIndex));
     chCdr.setServedMSISDN(ludbGetMSISDNPtr(ludbIndex));
     chCdr.setCorrelatedEventRefNumber(leg->ccCdr.getEventRefNumber());
  }
  //CDR <xxu:08-21-00> END

  //Check if HOLD service provisioned or not
  if ( !ludbIsProvHOLD(this->ludbIndex) ||
       isEmergencyCall )                         // Override features for emergency call
    {  
      //HOLD service is not provisioned, send back HOLD REJ msg
      DBG_WARNING("Hold service not provisioned for (%d, %d)\n",
                  this->ludbIndex,event);
      if (event==HC_EXTNL_HOLD_REQ)
        {
          event = HC_LOCAL_HOLD_REJ;
          leg->msSide->chCause_ = CNI_RIL3_CAUSE_REQUESTED_SERVICE_NOT_SUBSCRIBED;

          //CDR <xxu:08-21-00> BEGIN
          if (!isEmergencyCall)
               chCdr.generateRecord( CDR_SS_ACTION_INVOCATION,
                                     (unsigned char)SUPP_SERV_HOLD, 
                                     CNI_RIL3_CAUSE_REQUESTED_SERVICE_NOT_SUBSCRIBED );
          //CDR <xxu:08-21-00> END

          DBG_LEAVE();
          return (0);
        }
      //Retrieval of a held call is always allowed, go retrieve!
    }
      
  //HOLD service is provisioned and go ahead  
  if (leg==callLeg1)       otherLeg=callLeg2;
  else if (leg==callLeg2)  otherLeg=callLeg1;
  else
    {
      DBG_ERROR("Internal logic messed up: got invalid callLeg (%d,%d,%d)\n",
                this->entryId, parent->callIndex, leg);
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }

  //Hold/Retv Ev
  switch ( event )
    {
    case HC_EXTNL_HOLD_REQ:
         //Check if only one call now
         if (otherLeg->msSide == NULL)
         {
            //Only one call existing now
            if (leg->msSide->callState_==HC_ST_ACTIVE &&
                leg->msSide->chState_==CH_ST_IDLE      )
            {
                //this one call is qualified for being held
                event = HC_LOCAL_HOLD_ACT;
                
                //CDR <xxu:08-21-00>
                chCdr.generateRecord(  CDR_SS_ACTION_INVOCATION,
                                      (unsigned char)SUPP_SERV_HOLD, 
                                       CNI_RIL3_CAUSE_SUCCESS);
                DBG_LEAVE();
                return (0);
            } else
            {  
                //this one call is not qualified for being held
                event = HC_LOCAL_HOLD_REJ;
                leg->msSide->chCause_ = CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;

                //CDR <xxu:08-21-00>
                chCdr.generateRecord(  CDR_SS_ACTION_INVOCATION,
                                      (unsigned char)SUPP_SERV_HOLD, 
                                       CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE);

                DBG_LEAVE();
                return (0);
            }
         }  

         //Sanitory check: make sure that CALL HOLD timer should not be running!
         if (leg->msSide->hcTimer->timerSet_ || // Need to change the usage of timers
             (otherLeg->msSide->callState_== HC_ST_ACTIVE &&
              otherLeg->msSide->hcTimer->timerSet_)) // other services may also need it
        {
           //Received two HOLD REQ messages in a row, log and reject this 2nd!
           DBG_WARNING("Received two HOLD REQs in a row, curSession(%d,%d)\n",
                        this->entryId,parent->callIndex);
           event = HC_LOCAL_HOLD_REJ;
           leg->msSide->chCause_ = CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;

           //CDR <xxu:08-21-00>
           chCdr.generateRecord(  CDR_SS_ACTION_INVOCATION,
                                 (unsigned char)SUPP_SERV_HOLD, 
                                  CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE);
   
           DBG_LEAVE();
           return (0);
        }

      //No HOLD REQ is pending right now. Go ahead to handle it. First of all
      //Check if this leg is staying in (active, idle), otherwise reject it.
      if (leg->msSide->callState_ != HC_ST_ACTIVE || 
          leg->msSide->chState_   != CH_ST_IDLE    )
        {
          DBG_WARNING("Mobile tries HOLD a non (active,idle) call:(%d,%d)\n",
                      this->entryId, parent->callIndex);
          event = HC_LOCAL_HOLD_REJ;
          leg->msSide->chCause_ = CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;

           //CDR <xxu:08-22-00>
          chCdr.generateRecord(  CDR_SS_ACTION_INVOCATION,
                                 (unsigned char)SUPP_SERV_HOLD, 
                                  CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE);
          DBG_LEAVE();
          return (0);
        }

        //Sanitory check: got two HolDReqs!
        if (leg->msSide->hcTimer->timerSet_)
        {
            //Received two HOLD REQ messages in a row, log and reject this 2nd!
            DBG_WARNING("Received two HOLD REQs in a row, curSession(%d,%d)\n",
                         this->entryId,parent->callIndex);
            event = HC_LOCAL_HOLD_REJ;
            leg->msSide->hcTimer->cancelTimer(); //doesn't make sense to keep it running!
            leg->msSide->chCause_ = CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;

            //CDR <xxu:08-22-00>
            chCdr.generateRecord(  CDR_SS_ACTION_INVOCATION,
                                  (unsigned char)SUPP_SERV_HOLD, 
                                   CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE);
            DBG_LEAVE();
            return (0);
         }

         //Current leg is qualified for HOLD,process in terms of two HOLD cases
         if (otherLeg->msSide->callState_ == HC_ST_ACTIVE)
         {
             if (otherLeg->msSide->chState_ == CH_ST_HELD)
             {
              //SWAP case: handle HOLD part, start timer!
              leg->msSide->hcTimer->setTimer(CALL_HOLD_RETV_TIMER);
              event = HC_LOCAL_HOLD_REQ;

              //CDR <xxu:08-22-00>
              chCdr.generateRecord(  CDR_SS_ACTION_INVOCATION,
                                    (unsigned char)SUPP_SERV_HOLD, 
                                     CNI_RIL3_CAUSE_SUCCESS);
              DBG_LEAVE();
              return (0);
            } else
            {
                //Kind of mess-up happened, reject and log!
                DBG_WARNING("Mess-up occurs btw two legs in proc HOLD REQ\n");
                event = HC_LOCAL_HOLD_REJ;
                leg->msSide->chCause_ = CNI_RIL3_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;

                //CDR <xxu:08-22-00>
                chCdr.generateRecord(  CDR_SS_ACTION_INVOCATION,
                                      (unsigned char)SUPP_SERV_HOLD, 
                                       CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE);
                DBG_LEAVE();
                return (0);
              }
        } else
        {
	      //the other leg is not active, go ahead to HOLD the cur leg
            event = HC_LOCAL_HOLD_ACT;

            //CDR <xxu:08-22-00>
            chCdr.generateRecord(  CDR_SS_ACTION_INVOCATION,
                                  (unsigned char)SUPP_SERV_HOLD, 
                                   CNI_RIL3_CAUSE_SUCCESS);
            DBG_LEAVE();
	      return (0);
        }
 
      break;       

    case HC_EXTNL_RETV_REQ:
        //Check if only one call now
        if (otherLeg->msSide == NULL)
        {
	      //Only one call existing now
            if (leg->msSide->callState_==HC_ST_ACTIVE &&
                leg->msSide->chState_==CH_ST_HELD      )
            {
                //this one call is qualified for being retved
                //CH<xxu:Mod 12-13-99>
                //event = HC_LOCAL_RETV_ACT;
                event = HC_LOCAL_RETV_REQ;
                DBG_LEAVE();
              return (0);
            } else
            {  
                //this one call is not qualified for being held
		    event = HC_LOCAL_RETV_REJ;
                leg->msSide->chCause_ = CNI_RIL3_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
                DBG_LEAVE();
                return (0);
            }
         }  

      //If the leg is not on hold, log and reject this message. Sort of messed-up!
      if (leg->msSide->callState_!=HC_ST_ACTIVE||leg->msSide->chState_!=CH_ST_HELD)
        {
          DBG_WARNING("RETRIEVE a non-(active, held) call (%d,%d), messed up!\n",
                      this->entryId, parent->callIndex);
          event = HC_LOCAL_RETV_REJ;
          leg->msSide->chCause_ = CNI_RIL3_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
          DBG_LEAVE();
          return (0);
        }

      //If the otherLeg is in (active, holdReq) and call hold is running, do swap!
      if (otherLeg->msSide->callState_ == HC_ST_ACTIVE)
        {
          if (otherLeg->msSide->chState_ == CH_ST_HREQ)
            {
              //Do SWAP now: send the HREQ leg to HELD and pull back the HELD active!
              otherLeg->msSide->hcTimer->cancelTimer();
              otherLeg->msSide->handleHoldAct();
              //CH<xxu:Mod 12-13-99>
              //event = HC_LOCAL_RETV_ACT;
              event = HC_LOCAL_RETV_REQ;

              DBG_LEAVE();
              return (0);
            } else
              {
                //Kind of mess-up happened, reject and log!
                DBG_WARNING("Mess-up occurs btw two legs in proc RETV REQ\n");
                event = HC_LOCAL_RETV_REJ;
                leg->msSide->chCause_ = CNI_RIL3_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE;
                DBG_LEAVE();
                return (0);
              }
        } else
          {
            //the other leg is not active, go ahead to RETV the cur leg
            //CH<xxu:Mod 12-13-99>
            //event = HC_LOCAL_RETV_ACT;
            event = HC_LOCAL_RETV_REQ;
            DBG_LEAVE();
            return (0);
          }

      break;
 
    default:
      //Sanitory check, may be removed in the future
      DBG_ERROR("Logic messed up! Got  non HOLD/RETRIEVE event\n");
      break;
    }

  event = CC_MESSAGE_PROCESSING_COMPLT;
  DBG_LEAVE();
  return (0);
}

int
CCSessionHandler::handleMobMsg(T_CNI_RIL3MD_CCMM_MSG      *msInMsg)
// This function handles not only the MS PDUs but local DL management messages as well 
{
  JCCEvent_t  currEvent ;
  CallLeg    *whichLeg  = NULL_PTR;
  int         retVal    = 0       ;

  T_CNI_RIL3_PROTOCOL_DISCRIMINATOR msgPd     ;
  T_CNI_LAPDM_L2L3PrimitiveType     l2MsgType = msInMsg->primitive_type;

  DBG_FUNC ("CCSessionHandler::handleMobMsg", CC_SESSION_LAYER);
  DBG_ENTER();

  if (l2MsgType == L23PT_DL_DATA_IND || l2MsgType == L23PT_DL_UDATA_IND)
  {
      if (handoverSession == targetHandoverSession)
      { // handover on target side          
          targetHandoverSession->sendMSMsgToAnchorCC(msInMsg);
          DBG_LEAVE();
          return   (0);
      }

	  //ext-HO <xxu:05-31-01>
	  if (handoverSession == targetHandoverSessionExtHo)
	  {
          targetHandoverSessionExtHo->sendMSMsgToAnchorCC(msInMsg);
          DBG_LEAVE();
          return   (0);
	  }

     msgPd = extractPd (&msInMsg->l3_data);

     switch (msgPd)
     {
     case CNI_RIL3_PD_NON_CALL_RELATED_SS: // CISS message

        if (msInMsg->l3_data.msgLength < 2)
        { // Msg is too short. Discard it
            retVal = 0;
        }
        else 
        {
            whichLeg = cissCallLeg;
            retVal   = cissCallLeg->handleMsMsg (msInMsg);
        }
        break;

     case CNI_RIL3_PD_SMS                : // SMS message

        if ((currEvent = ma->mapToSmsEvent(msInMsg)) != JCC_NULL_EVENT)
        {
            routeFromMsToSmsLeg (ma->msDecodedSmsMsg.header, (SmsLeg *&)whichLeg);

            retVal = ((SmsLeg *)whichLeg)->handleMsMsg (ma->msDecodedSmsMsg, currEvent);
        }
        else
        {   // garbage Message received from MS will be discarded without reaction
            retVal = 0;
        }
        break;

     case CNI_RIL3_PD_CC                 : // CC  message

        currEvent = ma->mapToCcEvent (msInMsg);

        if (routeFromMsToCallLeg (&ma->msDecodedMsg, (TwoPartyCallLeg *&)whichLeg))
        {
            //CH<xxu:11-22-99>: convert to the two events to internal events
            if (currEvent==HC_EXTNL_HOLD_REQ || currEvent==HC_EXTNL_RETV_REQ)
            {
                //Got a Hold or Retv msg for 1 of 2 legs
                handleMobHoldRetvReq(currEvent, (TwoPartyCallLeg *)whichLeg);
            }

            retVal = ((TwoPartyCallLeg *)whichLeg)->msSide->handleMobMsg (&ma->msDecodedMsg  ,
                                                                           ma->msDecodeResult,
                                                                           ma->causeValue    ,
                                                                           currEvent         );
        }
        else
        {
            // late event
            // Warning messages printed in the routing method above.
        }
        break;

     default:
        DBG_ERROR ("CC Session error: Message with unknown PD = %d\n", msgPd);
        retVal = 0;
        break;
     }
  }
  else if (msInMsg->sapi == SAPI_SMS                                       &&
           (l2MsgType == L23PT_DL_EST_CONF || l2MsgType == L23PT_DL_EST_IND)  )
  {  // SAPI3 (SMS) connection management messages (setup)

      // SAPI3 connection is created and stays valid until MM connection is released
      setSapi3 (); // SAPI3 connection is established

      flashSapi3Queue ();

      // SAPI=3 connection is handled by the Session Handler and 
      // will not be forwarded to the Leg level
  }
  else if (msInMsg->sapi == SAPI_SMS                                       &&
           (l2MsgType == L23PT_DL_REL_IND  || l2MsgType == L23PT_MDL_ERROR)  )
  {
      // SAPI3 (SMS) connection management messages (release)
      // Properly release all the active SMS connections

      currEvent = SMS_EVENT_MD_ERROR_IND;

      if (handoverSession == targetHandoverSession)
      { // handover on target side
          targetHandoverSession->sendMSMsgToAnchorCC(msInMsg);
          DBG_LEAVE();
          return   (0);
      }
      else
      {
          resetSapi3       (); // Unconditional clear SAPI=3 flag and 
          sapi3Queue_.reset(); // clean the queue

          // Notify SMS legs about connection failure
          if (!smsLeg1->isIdle())
          {
              smsLeg1->handleL2Msg (*msInMsg, currEvent);
              handleReleaseCallLeg (smsLeg1);
          }

          if (!smsLeg2->isIdle())
          {
              smsLeg2->handleL2Msg (*msInMsg, currEvent);
              handleReleaseCallLeg (smsLeg2);
          }

          if (!smsLeg3->isIdle())
          {
              smsLeg3->handleL2Msg (*msInMsg, currEvent);
              handleReleaseCallLeg (smsLeg3);
          }
      }

      DBG_LEAVE ();
      return    (1); // have to do this because more than one SMS leg could be released
  }
  else
  {
      DBG_ERROR ("CM Session: Unexpected Msg received from MD (L2 type = %d)\n", l2MsgType);
      retVal = 0;
  }

  if (retVal != 0)
  {
      handleReleaseCallLeg (whichLeg);
  }

  DBG_LEAVE ();
  return    (retVal);
}


int
CCSessionHandler::smHandleEvent(JCCEvent_t currEvent)
{
  DBG_FUNC("CCSessionHandler::smHandleEvent", CC_SESSION_LAYER);
  DBG_ENTER();

  JCCEvent_t  resultEvent;

  switch (currEvent)
    {
    case CC_MM_CONNECTION_CNF:
      resultEvent = handleMMEstCnf();
      break;
      
    case CC_MM_CONNECTION_REL:
      resultEvent = handleMMRelInd();
      break;

    case CC_MM_CONNECTION_REJ:
    case CC_RELEASE_CALL:
    case JCC_NULL_EVENT:
      resultEvent = currEvent;
      break;

    case CC_MM_CONNECTION_IND:
        // Igal: Take care of the new SMS Serv Req during a call or 
        // concatinated MO SMS-PP
      resultEvent = handleMMEstInd();
      break;

    default:
      // Should not come here
      resultEvent = JCC_NULL_EVENT;
      DBG_ERROR("CC Session error : Invalid event in smHandleEvent(%d)\n", currEvent);

    }
  
  switch (resultEvent)
    {
    case CC_MM_CONNECTION_CNF:
    case CC_MM_CONNECTION_REJ:
      { /* Response to the MM connection Esteblish request.
             Requesting Leg is stored and canbe accessed using getMmTermSetupLeg() */
        int       causeValue = 0;
        CallLeg * tempLeg    = getMmTermSetupLeg ();

        if (tempLeg == smsLeg1 && !smsLeg1->isIdle())
        { // This is SMS initiated setup and it's still active
            if (causeValue = smsLeg1->handleMmMsg (*mmInMsg, resultEvent))
            {
                handleReleaseCallLeg (smsLeg1);
                DBG_LEAVE();
                return   (causeValue);
            }
            // Allows another CM level MT SETUP when MM connection setup is complete
            resetMmTermSetup ();
        }
        else if (tempLeg == callLeg1 && ((!(callLeg1->idle)) && (callLeg1->msSide != NULL_PTR)))
        { // This is a response for MM-EST Req (Page Request) for MT voice or data call
            if ((causeValue = callLeg1->msSide->handleMMMsg(mmInMsg, resultEvent)) != 0)
            {
                    handleReleaseCallLeg (callLeg1);
                    DBG_LEAVE();
                    return   (causeValue);
            }
            // Allows another CM level MT SETUP when MM connection setup is complete
            resetMmTermSetup ();
        }
        else
        { // late event
            int legNum = (!tempLeg) ? (-1) : tempLeg->callLegNum;

            DBG_WARNING ("MM->CC Warning: Unexpected/Late MM Est Response event for leg %d,\
             (event=%d) (session id=%d)\n",
                         legNum, resultEvent, parent->callIndex);
        }
      }
      break;

    case CC_MM_CONNECTION_IND:
      {
        if (isMmConnected()) // Avoiding possible race condition
        {
            if (smsLeg2->setupOrig() == false) // smsLeg2 is designated for MO SMS support
                smsLeg2->cleanup((T_CNI_RIL3_CAUSE_VALUE)smsLeg2->getRemRelCause());
        }
        else
            DBG_WARNING ("Unexpected late MM-EST Ind for SAPI=3 received by the CM session %d.\n",
                         parent->callIndex);
      }
      break;

    case JCC_NULL_EVENT:
    case CC_MESSAGE_PROCESSING_COMPLT:  
      //Do Nothing
      break;

    case CC_RELEASE_CALL:
      cleanup(releaseCause, TRUE);
      DBG_LEAVE();
      return(-1);
      break;

    default:
      // Should not come here
      DBG_ERROR("CC Session error : Invalid event in smHandleEvent(%d)\n", currEvent);

    }  

  DBG_LEAVE();
  return (0);
}


JCCEvent_t 
CCSessionHandler::handleMMRelInd(void)
{
  DBG_FUNC("CCSessionHandler::handleMMRelInd", CC_SESSION_LAYER);
  DBG_ENTER();
  JCCEvent_t currEvent;

  //CAUSE<xxu:03-21-00>
  //releaseCause = CNI_RIL3_CAUSE_TEMPORARY_FAILURE;
  releaseCause = mmInMsg->cause;

  DBG_TRACE("CC <- MM: MmRelInd contained release cause %d\n", mmInMsg->cause);

  // check for handover
  if (handoverSession != NULL)
    {
      if (handoverSession == anchorHandoverSession)
        {
          currEvent = anchorHandoverSession->handleInternalEvent(CC_MM_CONNECTION_REL);
        }
      else
        {
          currEvent = targetHandoverSession->handleInternalEvent(CC_MM_CONNECTION_REL);
        }
      
      DBG_LEAVE();
      return(currEvent);
    }
  
  DBG_LEAVE();
  return (CC_RELEASE_CALL);
}


JCCEvent_t 
CCSessionHandler::handleMMEstCnf(void)
{
  DBG_FUNC("CCSessionHandler::handleMMEstCnf", CC_SESSION_LAYER);
  DBG_ENTER();

  // MM uses the msg Q to echo back to the correct task.

  if ((mmInMsg->l3_data.pageRsp.mobileId == ludbGetMobileId(ludbIndex)) && // for this mobile
      (entryId == (T_CNI_IRT_ID) JCC_UNDEFINED)) // we are currently waiting for a confirm
    {
      // continue
    }
  else
    {
      // handle late page response from MM for a different Mobile.
      
      IntraL3Msg_t mmOutMsg;

      // Send internal MM msg for release
      sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, mmInMsg->entry_id, &mmOutMsg);
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
  
  // !< Need to generate a transaction identifer to use
  // For now, always use 0 - No Supplementary Services

  T_CNI_L3_ID  newId    ;
  T_CNI_IRT_ID newEntryId = mmInMsg->entry_id;
  bool         retVal   ;

  newId.msgq_id =  parent->msgQId;
  newId.sub_id  = 0;                 // No sub id needed here

  if (setIrt (newEntryId, newId) == false)
  {
      DBG_LEAVE();
      return   (CC_RELEASE_CALL);
  }

  // Setup the entry id, oid, and sapi
  entryId = mmInMsg->entry_id;

  // Store Mobile Classmark Info <chenj:07-24-01>
  classmark2Info = mmInMsg->l3_data.pageRsp.classmark2;

  // GET OID AND SAPI
  T_CNI_RIL3_IRT_ENTRY  irtEntry;
  irtEntry.entry_id = mmInMsg->entry_id;
  
  if  ( (CNI_RIL3_IRT_Get_Entry_by_Entry_ID(&irtEntry))
        == false )
    {
      DBG_ERROR("CC Call Error : IRT Table Problem for (entry Id = %d)\n",  
                mmInMsg->entry_id);
      DBG_LEAVE();
      return (CC_RELEASE_CALL);
    }
  oid = irtEntry.lapdm_oid;
  // sapi = SAPI_MAIN;  

  DBG_LEAVE();
  return (CC_MM_CONNECTION_CNF);
}


JCCEvent_t CCSessionHandler::handleMMEstInd (void)
{
  JCCEvent_t event ;
  bool       result;
  T_CNI_RIL3_CM_SERVICE_TYPE cmServType =
                    mmInMsg->l3_data.servReq.cmServiceType.serviceType;

  DBG_FUNC("CCSessionHandler::handleMMEstInd", CC_SESSION_LAYER);
  DBG_ENTER();

    if (cmServType == CNI_RIL3_CM_SERVICE_TYPE_SHORT_MESSAGE_SERVICE)
    {
        event = CC_MM_CONNECTION_IND;

        DBG_TRACE ("CC Session: Secondary MO SMS Serv. Request received.\n");
    }
    else
    { // This CM Service Type is not expected
      // Previously was blocked by the MM
        DBG_ERROR ("CC Session Error: Unexpected MM_EST_IND, service type %d - releasing call!\n",
                    cmServType);
        // PR CSCdv12866 <chenj:08-21-01>
        // A 2nd MM establishment is sent to CC for the same mobile.  This is an 
        // error condition and the MM/RR states need to be reset.
        event = CC_RELEASE_CALL;
     }

  DBG_LEAVE();
  return   (event);
}


JCCEvent_t 
h323MapToEvent(IntraL3Msg_t* h323InMsg, 
               CCRemMsgData_t &remMsgData) 
{

  DBG_FUNC("CCSessionHandler::h323MapToEvent", CC_SESSION_LAYER);
  DBG_ENTER();

  JCCEvent_t remoteEvent;
          
  // Copy the incoming VOIP message to the remote message data structure 
  remMsgData = h323InMsg->l3_data.voipMsg;

  switch (h323InMsg->message_type)
    {
	//<xxu:03-22-01> Handle Progress Message
    case INTRA_L3_CC_Q931_PROGRESS:
		  remoteEvent = HC_REMOTE_CALL_PROGRESS_1;
		  break;

    //BCT <xxu:09-21-00>
    case INTRA_L3_CC_BCT_UPDATE_HANDLES:
         remoteEvent = HC_REMOTE_BCT_UPDATE_HANDLES;
         break;

    //BCT <xxu:07-13-00> BEGIN
    case INTRA_L3_CC_H450_BCT_STATUS:
         remoteEvent = OHC_REMOTE_BCT_STATUS;
         break;
    //BCT <xxu:07-13-00> END

    case INTRA_L3_CC_Q931_SETUP:
      remoteEvent = THC_REMOTE_TERM_ADDRESS;
      remMsgData.callOffering = h323InMsg->l3_data.setup.callOffering;
      break;
      
    case INTRA_L3_CC_Q931_CALL_PROC:
      remoteEvent = HC_REMOTE_CALL_PROGRESS;
      break;
      
    case INTRA_L3_CC_Q931_ALERTING:
      remoteEvent = OHC_REMOTE_ALERTING;
      break;
      
    case INTRA_L3_CC_Q931_CONNECT:
      remoteEvent = OHC_REMOTE_ANSWER;
      break;
      
    case INTRA_L3_CC_Q931_DISCONNECT:
      remoteEvent = HC_REMOTE_DISCONNECT_REQ;
      break;
      
    case INTRA_L3_CC_H245_CHANN_READY:
      remoteEvent = HC_REMOTE_CONNECTION_CONTROL;
      break;

    case INTRA_L3_CC_H245_CHANN_SELECT:
      remoteEvent = HC_REMOTE_CHANN_SELECT;
      break;

    case INTRA_L3_CC_RAS_IP_ADDRESS_RESPONSE:
      remoteEvent = HC_VB_GET_VC_ADDRESS_RSP;
      break;

    //BCT <xxu:08-11-00> BEGIN
    case INTRA_L3_CC_RAS_BCT_NUMBER_RESPONSE:
      remoteEvent = HC_VB_GET_BCT_NUMBER_RSP;
      break;
    //BCT <xxu:08-11-00> END

    case INTRA_L3_CC_VC_UPDATE_INFO: 
      // Check ViperCell Status          
      if (h323InMsg->l3_data.statusVCMsg.status == VC_STATUS_DOWN)
            remoteEvent = EXT_VC_FAIL;
      else
      {
          remoteEvent = JCC_NULL_EVENT;

          DBG_ERROR ("CC Session Error: VOIP ViperCell update with unknown status %d.\n",
                                                   h323InMsg->l3_data.statusVCMsg.status);
      }
      break;

    case INTRA_L3_SMS_MESSAGE:
      { // SMS-PP related message
        switch (h323InMsg->l3_data.smsVbMsg.hdr.msgType)
        {
        case MNSMS_EST_Req  :
            remoteEvent = SMS_EVENT_MNSMS_EST_REQ  ;
            break;

        case MNSMS_DATA_Req :
            remoteEvent = SMS_EVENT_MNSMS_DATA_REQ ;
            break;

        case MNSMS_ABORT_Req:
            remoteEvent = SMS_EVENT_MNSMS_ABORT_REQ;
            break;

        case MNSMS_REL_Req  :
            remoteEvent = SMS_EVENT_MNSMS_REL_REQ  ;
            break;

        default:
            DBG_ERROR ("CM SESSION : Invalid SMS message (type = %d) from VOIP.\n",  
                                           h323InMsg->l3_data.smsVbMsg.hdr.msgType);
            remoteEvent = SMS_EVENT_PROTOCOL_ERROR ;
            break;
        }
      }
      break;

    default:
      DBG_ERROR("CC Call Error: Unexpected (Msg Type = %d) from VOIP.\n",  
                h323InMsg->message_type);
      remoteEvent = JCC_NULL_EVENT;
     
    }

  DBG_LEAVE();
  return (remoteEvent);
}

int               
CCSessionHandler::handleH323Msg(IntraL3Msg_t* h323InMsg) 
{
  DBG_FUNC("CCSessionHandler::handleH323Msg", CC_SESSION_LAYER);
  DBG_ENTER();

  JCCEvent_t      remoteEvent  ;
  CCRemMsgData_t  remMsgData   ;
  CallLeg        *leg          ;
  bool            needResponse = true;
  int             causeValue   = 0   ;

  remoteEvent = h323MapToEvent (h323InMsg, remMsgData);

  switch (remoteEvent)
  {
  //BCT <xxu: 08-11-00> BEGIN
  case HC_VB_GET_BCT_NUMBER_RSP:
       if (h323InMsg->l3_data.bctNumberResponse.rejectCause)
       {
           //Release BCT leg but keep the other
           if (bctLeg_ != NULL && bctInvoke_ && bctLeg_->msSide != NULL)
           {
               //CDR <xxu:08-22-00> BEGIN
               if ( ludbGetSubscInfoExists(ludbIndex) && bctMainLeg_ != NULL )
               {
                    CCdrSSA	bctCdr;
                    bctCdr.setServedIMSI(ludbGetMobileIdPtr(ludbIndex));
                    bctCdr.setServedMSISDN(ludbGetMSISDNPtr(ludbIndex));
                    bctCdr.setCorrelatedEventRefNumber(bctMainLeg_->ccCdr.getEventRefNumber());
                    bctCdr.generateRecord( CDR_SS_ACTION_INVOCATION, SUPP_SERV_ECT, 
                                           (T_CNI_RIL3_CAUSE_VALUE)remMsgData.basicCallInMsg.ReasonCode);
               }

               CCRemMsgData_t  remMsgData;
               remMsgData.basicCallInMsg.ReasonCode = h323InMsg->l3_data.bctNumberResponse.rejectCause;
               bctLeg_->msSide->handleRemoteEvent(h323InMsg, HC_VB_GET_BCT_NUMBER_RSP, remMsgData);

           }
          
       } else
       {
           //Continue current BCT process
           if (bctLeg_->msSide != NULL)
               bctLeg_->msSide->hcTimer->cancelTimer();

           DBG_TRACE("VBLink->CC Log(bct): Bct Num Rsp(reqSeqNo %d, rejCause %d) bctNo(%d,%d,%d,%d,%d)\n",
                      h323InMsg->l3_data.bctNumberResponse.reqSeqNumber,
                      h323InMsg->l3_data.bctNumberResponse.rejectCause,
                      h323InMsg->l3_data.bctNumberResponse.cpn.digits[0],
                      h323InMsg->l3_data.bctNumberResponse.cpn.digits[1],
                      h323InMsg->l3_data.bctNumberResponse.cpn.digits[2],
                      h323InMsg->l3_data.bctNumberResponse.cpn.digits[3],
                      h323InMsg->l3_data.bctNumberResponse.cpn.digits[4] );

           if (bctMainLeg_ != NULL && bctLeg_ != NULL )
           {
			   //Release the bct leg only
               CCRemMsgData_t  remMsgData;
               remMsgData.basicCallInMsg.ReasonCode = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
               bctLeg_->msSide->handleRemoteEvent(h323InMsg, HC_VB_GET_BCT_NUMBER_RSP, remMsgData);

               XltCalledPartyNumber = h323InMsg->l3_data.bctNumberResponse.cpn;
               CCRemMsgData_t bctMsgData;
               bctMsgData.callTransfer.CalledPartyNumber = h323InMsg->l3_data.bctNumberResponse.cpn;
               bctMsgData.callTransfer.CallingPartyNumber = CallingPartyNumber;

               sendH323Msg(bctMainLeg_, OHC_REMOTE_BCT_INVOKE, bctMsgData);
           } else
           {
               DBG_TRACE("VBLink->CC Log(bct): BCT not going any more when received BCT Num Rsp\n");
           }  
       }
       break;
  //BCT <xxu: 08-11-00> END
    
  case HC_VB_GET_VC_ADDRESS_RSP:
      {
          if (handoverSession == anchorHandoverSession)
              causeValue = anchorHandoverSession->handleVbCcMsg(h323InMsg);
          else 
              causeValue = targetHandoverSessionExtHo->handleVbCcMsg(h323InMsg);
      }
      break;

  case EXT_VC_FAIL:
      {
          DBG_TRACE("CC Session: Received external ViperCell failure notification.\n");

          // Cancel handover in process to the failed cell but keep the call.
          // Always disconnect calls already migrated to the failed remote cell
          cleanup (CNI_RIL3_CAUSE_RESOURCES_UNAVAILABLE_UNSPECIFIED, FALSE);
          causeValue = 0;
      }
      break;

  case SMS_EVENT_MNSMS_REL_REQ  :
  case SMS_EVENT_MNSMS_ABORT_REQ:
      {
          needResponse = false;
      }
  case SMS_EVENT_MNSMS_EST_REQ  :
  case SMS_EVENT_MNSMS_DATA_REQ :
      {
          SmsLeg * whichLeg;

          DBG_TRACE ("CM Session: SMS message (event = %d) received from VBLink.\n", 
                                                                      remoteEvent);
          if (routeFromVoipToSmsLeg (h323InMsg, whichLeg))
          {  
              causeValue = whichLeg->handleVbMsg (h323InMsg->l3_data.smsVbMsg, remoteEvent);
              leg        = whichLeg;
          }
          else
          {
              DBG_WARNING ("CM Session: Unable to forward a late message (event = %d) to SMS leg\n",
                           remoteEvent);
              if (needResponse) // Respond with MNSMS-ERROR Ind. Otherwise, discard it
                  smsSendVbLinkRelease (ludbIndex, h323InMsg->l3_data.smsVbMsg.hdr.refNum);

              causeValue = 0; // No need to release a leg
          }
      }
      break;

  case SMS_EVENT_PROTOCOL_ERROR :
      { // Invalid SMS message type. Discard it
        // Should never reach this point (the check is implemented in the unpacking method.
          DBG_ERROR ("CM Session: Discard invalid SMS message received from VOIP.\n");
          causeValue = 0;
      }
      break;

  default:
      {
        //if (remoteEvent == THC_REMOTE_TERM_ADDRESS)
        //{
          TwoPartyCallLeg *whichLeg  ;

          if (routeFromVoipToCallLeg (h323InMsg, whichLeg))
          {
              //ProgressIE
              

              //CDR <xxu:08-23-00>
              whichLeg->ccCdr.setQ931Crv(h323InMsg->H323CRV);

              //BCT <xxu: 07-13-00> BEGIN
              if (bctInvoke_)
              {
                  //Internal messup, debugging asap!
                  if (bctLeg_ == whichLeg)                  {
                      DBG_ERROR("H323->CC Warning: event(%d) VOIP msg(%d) for bctLeg (%p) (sessId=%d)\n",
                                 remoteEvent, h323InMsg->message_type, bctLeg_, parent->callIndex);
                      return(0);
                  }

                  remoteEvent = HC_REMOTE_DISCONNECT_REQ;
                  switch(h323InMsg->message_type)
                  {
                  case INTRA_L3_CC_H450_BCT_STATUS:
                       {
                         //CDR <xxu:08-22-00> BEGIN
                         CCdrSSA  bctCdr;
                         if (ludbGetSubscInfoExists(ludbIndex))
                         {
                             bctCdr.setServedIMSI(ludbGetMobileIdPtr(ludbIndex));
                             bctCdr.setServedMSISDN(ludbGetMSISDNPtr(ludbIndex));
                         }
                         bctCdr.setCorrelatedEventRefNumber(bctMainLeg_->ccCdr.getEventRefNumber());
                         //CDR <xxu:08-22-00> END

                         if (!remMsgData.basicCallInMsg.ReasonCode)
                         {
                              //CDR <xxu:08-22-00>
                              bctCdr.generateRecord( CDR_SS_ACTION_INVOCATION, SUPP_SERV_ECT, 
						                 CNI_RIL3_CAUSE_SUCCESS );

                              //BCT success, disconnect bct call first if not released yet
                              remMsgData.basicCallInMsg.ReasonCode = 
                                         (unsigned char) CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
                              if ( (bctLeg_ != NULL) && (bctLeg_->msSide != NULL) &&
                                   (bctLeg_->msSide->callState_ == OHC_ST_OFFERING_CALL) )
                                    bctLeg_->msSide->handleRemoteEvent(h323InMsg,remoteEvent, remMsgData);

                              //BCT success, disconnect the original call if it is active
                              remMsgData.basicCallInMsg.ReasonCode = 
                                         (unsigned char) CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
                              if (whichLeg->msSide->callState_ == HC_ST_ACTIVE)
                                  whichLeg->msSide->handleRemoteEvent(h323InMsg,remoteEvent, remMsgData);

                          } else
                          {
				  //CDR <xxu:08-22-00>
                              bctCdr.generateRecord( CDR_SS_ACTION_INVOCATION, SUPP_SERV_ECT, 
                                                     (T_CNI_RIL3_CAUSE_VALUE)remMsgData.basicCallInMsg.ReasonCode );

                              //BCT failed, disconnect the bct call only
                              remMsgData.basicCallInMsg.ReasonCode = 
                                         (unsigned char) CNI_RIL3_CAUSE_NORMAL_UNSPECIFIED;
                              if ( (bctLeg_ != NULL) && (bctLeg_->msSide != NULL) &&
                                   (bctLeg_->msSide->callState_ == OHC_ST_OFFERING_CALL) )
                                   bctLeg_->msSide->handleRemoteEvent(h323InMsg,remoteEvent, remMsgData);
                          }
 	               }
                       break;
                
                  //Clean itself if senseing two side doing BCT at the same time
                  case INTRA_L3_CC_BCT_UPDATE_HANDLES:
                  case INTRA_L3_CC_Q931_DISCONNECT:
                       //BCT does not make any sense, disconnect first call
                       if ( (bctLeg_ != NULL ) && (bctLeg_->msSide != NULL) &&
                            (bctLeg_->msSide->callState_ == OHC_ST_OFFERING_CALL) )
                           bctLeg_->msSide->handleRemoteEvent(h323InMsg,remoteEvent, remMsgData);

                       //BCT does not make any sense, disconnect second call
                       if ( (whichLeg->msSide->callState_ == HC_ST_ACTIVE) )
                           whichLeg->msSide->handleRemoteEvent(h323InMsg,remoteEvent, remMsgData);

                       break;

                  default:
                       //Wrong landing from VoIP, ignore this message
                       DBG_WARNING ("H323->CC Warning: Unexpected VOIP event for (session id=%d) (ev = %d)\n",
                                     parent->callIndex, h323InMsg->message_type);
                       break;
                  }
                  return (0);
              }
                   
              //BCT <xxu: 07-13-00> END

              if (whichLeg->msSide != NULL)
              {
                  causeValue = whichLeg->msSide->handleRemoteEvent(h323InMsg,remoteEvent, remMsgData);
                  leg        = whichLeg;
              }
          }
          else
          {  
              // late event
              DBG_WARNING ("H323->CC: Unexpected/Late msg(%d) h323CallHandle(%d) sessionId(%d)\n",
                            h323InMsg->message_type,
                            h323InMsg->call_handle, 
                            parent->callIndex);

              //BCT <xxu:07-13-00> BEGIN
              causeValue = 0;            
              //BCT <xxu:07-13-00> END
 
          }
      }
  }

  if (causeValue)
  {
      handleReleaseCallLeg (leg);
  }

  DBG_LEAVE();
  return   (causeValue);
}


//HO<xxu:01-31-00> extend to handle H3
void
CCSessionHandler::sendRRMsg(IntraL3PrimitiveType_t     primType,
                            IntraL3MsgType_t           msgType,
                            IntraL3Msg_t               *rrOutMsg)
{
  DBG_FUNC("CCSessionHandler::sendRRMsg", CC_SESSION_LAYER);
  DBG_ENTER();

  // if handover is in progress, then we need to send it to the target vipercell.
  if (handoverSession == anchorHandoverSession) 
    {
      rrOutMsg->primitive_type = primType; 
      rrOutMsg->message_type = msgType;

      if (( anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_HO_REQ_ACK) ||
          ( anchorHandoverSession->hoState_ == HAND_ST_ANCH_SND_HB_REQ_ACK) ||
          ( anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_H3_REQ_ACK) ||

          // ext-HO <chenj:06-12-01>
          // JOE: need to add handback and h3 states for external handovers
          ( anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_EHO_REQ_ACK) )
        {
          // Handover Command is sent. Queue messages
          anchorHandoverSession->addToRRQueue(rrOutMsg);
          DBG_LEAVE();
          return;
        }
      else if(anchorHandoverSession->hoState_ == HAND_ST_ANCH_HO_COMPLT)
        {
          // handover on anchor side.
          anchorHandoverSession->sendRRMsgToTargetCC(rrOutMsg);
          DBG_LEAVE();
          return;
        }
      // ext-HO <chenj:06-12-01>
      else if(anchorHandoverSession->hoState_ == HAND_ST_ANCH_EHO_COMPLT)
        {
          // handover on anchor side.
          // send message to target PLMN using POST HANDOVER MOB EVENT MAP message
          anchorHandoverSession->sendRRMsgToTargetPLMN (rrOutMsg);
        }
      else
        {
          DBG_TRACE("sendRRMsg ERROR: incorrect hoState_(%d)\n",
                     anchorHandoverSession->hoState_);
        }
    }

  sendRR   (primType, msgType, entryId, rrOutMsg);
  DBG_LEAVE();
}


void CCSessionHandler::sendMMMsg (IntraL3Msg_t *mmOutMsg)
{
  /* Forward MM messages between target and anchor CM */

  DBG_FUNC("CCSessionHandler::sendMMMsg", CC_SESSION_LAYER);
  DBG_ENTER();

    if (handoverSession == targetHandoverSession)
    {
        handoverSession->sendMMMsgToRemoteCC (mmOutMsg);
    }

    else if (handoverSession == anchorHandoverSession)
    {
      if ((anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_HO_REQ_ACK) ||
          (anchorHandoverSession->hoState_ == HAND_ST_ANCH_SND_HB_REQ_ACK) ||
          (anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_H3_REQ_ACK) ||

          // ext-HO <chenj:06-12-01>
          // JOE: need to add handback and h3 states for external handovers
          ( anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_EHO_REQ_ACK) )
      {
          // Handover Command is sent. Queue messages
          handoverSession->mmQueue_.add (mmOutMsg);
      }
      else if(anchorHandoverSession->hoState_ == HAND_ST_ANCH_HO_COMPLT)
      {
          // handover on anchor side.
          handoverSession->sendMMMsgToRemoteCC (mmOutMsg);
      }
      // ext-HO <chenj:06-12-01>
      else if(anchorHandoverSession->hoState_ == HAND_ST_ANCH_EHO_COMPLT)
        {
          // handover on anchor side.
          // send message to target PLMN using POST HANDOVER MOB EVENT MAP message
          anchorHandoverSession->sendMMMsgToRemotePLMN (mmOutMsg);
        }
      else
        {
          DBG_TRACE("sendMMMsg ERROR: incorrect hoState_(%d)\n",
                     anchorHandoverSession->hoState_);
        }
    }

    else
    {
        DBG_ERROR ("CM Session: Request to send MM message to non-existing remote CC.\n");
    }

  DBG_LEAVE();
}


void
CCSessionHandler::sendDLMsg (L3L2MsgType_t    msgType = L3L2_DL_EST_REQ,
                             T_CNI_LAPDM_SAPI sapi    = SAPI_SMS       )
{ // currently handling only DL-EST Req (SAPI3) from SMS

  DBG_FUNC ("CCSessionHandler::sendDLMsg", CC_SESSION_LAYER);
  DBG_ENTER();

  T_CNI_LAPDM_OID msgOid = rm_GetOid (entryId, sapi);

  if (msgType != L3L2_DL_EST_REQ || sapi != SAPI_SMS)
  {
      DBG_ERROR ("CM session error: Request to send unknown msg (type %d, SAPI %d) to LAPDm\n",
                 msgType, sapi);
  }
  else
  {
      // if handover is in progress, then we need to send it to the target vipercell.
      if (handoverSession == anchorHandoverSession) 
      {
          if ((anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_HO_REQ_ACK) ||
              (anchorHandoverSession->hoState_ == HAND_ST_ANCH_SND_HB_REQ_ACK) ||
              (anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_H3_REQ_ACK)   )
          {
              /* Handover Command is sent. Not allowed to setup SAPI=3 connection.
                 IT will be set on the new cell */
              DBG_WARNING (" Session Manager: Late request for SAPI=3 establishment is ignored.\n Handover is in process.\n");
          }
          else if(anchorHandoverSession->hoState_ >= HAND_ST_ANCH_HO_COMPLT)
          {
              // handover on anchor side.
              DBG_ERROR (" Session Manager: Unexpected request for SAPI=3 establishment is ignored.\n Handover is completed.\n");
          }
          else
              CNI_LAPDM_Dl_Establish_Request (msgOid, sapi, EM_NORMAL, NULL_PTR);
      }
      else
          CNI_LAPDM_Dl_Establish_Request (msgOid, sapi, EM_NORMAL, NULL_PTR);
  }

  DBG_LEAVE();
  return     ;
}


//HO<xxu:01-31-00> extend to handle H3 
void
CCSessionHandler::sendL2Msg (T_CNI_LAPDM_L3MessageUnit *msEncodedMsg)
{
  DBG_FUNC ("CCSessionHandler::sendL2Msg", CC_SESSION_LAYER);
  DBG_ENTER();

  // if handover is in progress, then we need to send it to the target vipercell.
  if (handoverSession == anchorHandoverSession) 
  {
      if ( ( anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_HO_REQ_ACK)  ||
           ( anchorHandoverSession->hoState_ == HAND_ST_ANCH_SND_HB_REQ_ACK)  ||
           ( anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_H3_REQ_ACK)  ||

           // ext-HO <chenj:06-12-01>
           // JOE: need to add handback and h3 states for external handovers
           ( anchorHandoverSession->hoState_ == HAND_ST_ANCH_RCV_EHO_REQ_ACK) )
      {
          // Handover Command is sent. Queue messages
          anchorHandoverSession->addToDownlinkQueue(msEncodedMsg);
      }
      else if ((anchorHandoverSession->hoState_ >= HAND_ST_ANCH_HO_COMPLT) &&
               (anchorHandoverSession->hoState_ <= HAND_ST_ANCH_RCV_H3_REQ_ACK) )
      {
          // handover on anchor side.
          anchorHandoverSession->sendL2MsgToTargetCC (msEncodedMsg);
      }
      // ext-HO <chenj:06-12-01>
      else if ( (anchorHandoverSession->hoState_ >= HAND_ST_ANCH_RCV_EHO_HB_REQ) &&
                (anchorHandoverSession->hoState_ <= HAND_ST_ANCH_EHO_COMPLT) )
      {
          // handover on anchor side.
          // send message to target PLMN using POST HANDOVER MOB EVENT MAP message
          anchorHandoverSession->sendL2MsgToTargetPLMN (msEncodedMsg);
      }
      else
      { // send PDU or if SAPI=3 is not ready establish it and queue message
          forwardPdu (msEncodedMsg);
      }
  }
  else
  { // send PDU or if SAPI=3 is not ready establish it and queue message
      forwardPdu (msEncodedMsg);
  }

  DBG_LEAVE();
}


bool CCSessionHandler::forwardPdu (T_CNI_LAPDM_L3MessageUnit *msEncodedMsg)
{
  DBG_FUNC ("CCSessionHandler::forwardPdu", CC_SESSION_LAYER);
  DBG_ENTER();

  bool retVal = true;

  T_CNI_RIL3_PROTOCOL_DISCRIMINATOR msgPd = extractPd (msEncodedMsg);

  T_CNI_LAPDM_SAPI sapi  = (msgPd == CNI_RIL3_PD_SMS) ? SAPI_SMS : SAPI_MAIN;

  // Retrieve oid for the channel according to SAPI from RR
  T_CNI_LAPDM_OID msgOid;

    if (sapi == SAPI_MAIN || getSapi3Status())
    { // SAPI=0 message or SAPI=3 connection is already established
        if ((msgOid = rm_GetOid (entryId, sapi)) == CNI_LAPDM_NULL_OID)
        {
            DBG_ERROR ("CM Session: Invalid OID received from RM (entryId = %d, sapi = %d)\n",
                       entryId, sapi);
            retVal = false;
        }

        CNI_LAPDM_Dl_Data_Request (msgOid, sapi, msEncodedMsg);

        PostL3SendMsLog (msgOid, sapi, msEncodedMsg);
    }
    else
    { 
        // Establish SAPI=3 before sending SMS messages to LAPDm
        sendDLMsg       (L3L2_DL_EST_REQ, SAPI_SMS);
        // Queue message till connection is established
        sapi3Queue_.add (msEncodedMsg             );
    }

  DBG_LEAVE();
  return (retVal);
}


T_CSU_PORT_ID
CCSessionHandler::mobileCsuPort(void) 
{

  DBG_FUNC("CCSessionHandler::mobileCsuPort", CC_SESSION_LAYER);
  DBG_ENTER();

  // if handover is in progress, then we need to send it to the target vipercell.
  if ((handoverSession == anchorHandoverSession) &&
       ( (anchorHandoverSession->hoState_ == HAND_ST_ANCH_HO_COMPLT) ||
         (anchorHandoverSession->hoState_ == HAND_ST_ANCH_EHO_COMPLT) ) )
    {
      // handover on anchor side. Get CSU port for handover.
      DBG_LEAVE();
      return(anchorHandoverSession->mobileCsuPort());
    }
  else
    {
      T_CSU_PORT_ID rfSourcePort;
      rfSourcePort.portType = CSU_GSM_PORT;

      rfSourcePort.portId.gsmHandler = entryId;

      DBG_LEAVE();
      return(rfSourcePort);
    }
      
}


void CCSessionHandler::flashSapi3Queue ()
{
    sapi3Queue_.flash();
}


void
sendStatusMsg(int sessionNum,
              int callLeg)
{
  if ((0 <= sessionNum) &&
      (sessionNum < CC_MAX_CC_CALLS))
    {
      switch (callLeg) {
      case 1:
        ccSession[sessionNum].sessionHandler->callLeg1->msSide->handleMobStatusEnquiry();
        break;
      case 2:
        ccSession[sessionNum].sessionHandler->callLeg2->msSide->handleMobStatusEnquiry();
        break;
      default:
        break;
      }
    }
}


//BCT <xxu:09-22-00>
bool
CCSessionHandler::getBctArrival(void)
{
   if (callLeg1 != NULL)
   {  
       if (callLeg1->bctArrival_)
           return true;
   }

   if (callLeg2 != NULL)
   {
       if (callLeg2->bctArrival_)
          return true;
   }

   return false;
}

//ext_HO <xxu:05-16-01>
int               
CCSessionHandler::handleH323MsgExtH3(IntraL3Msg_t* h323InMsg) 
{
  JCCEvent_t      remoteEvent  ;
  CCRemMsgData_t  remMsgData   ;
  int             causeValue=0 ;

  DBG_FUNC("CCSessionHandler::handleH323MsgExtH3", CC_EO_LAYER);
  DBG_ENTER();

  remoteEvent = h323MapToEventExtH3(h323InMsg, remMsgData);

  if (h3CallLeg->msSide != NULL)
  {
      causeValue = h3CallLeg->msSide->handleRemoteEvent(h323InMsg, remoteEvent, remMsgData);
  }

  if (causeValue)
  {
  	  DBG_ERROR("{\nMNEThoERROR(cs::handleH323MsgExtH3): release h3CallLeg due to h3Call issue\n}\n");
      handleReleaseCallLegExtH3(h3CallLeg);
  }

  DBG_LEAVE();
  return causeValue;
}

//ext-HO <xxu:05-16-01>
int               
CCSessionHandler::handleH323MsgExtHo(IntraL3Msg_t* h323InMsg) 
{
  JCCEvent_t      remoteEvent  ;
  CCRemMsgData_t  remMsgData   ;
  int             causeValue=0 ;

  DBG_FUNC("CCSessionHandler::handleH323MsgExtHo", CC_HO_LAYER);
  DBG_ENTER();

  remoteEvent = h323MapToEventExtHo(h323InMsg, remMsgData);

  if (hoCallLeg->msSide != NULL)
  {
      causeValue = hoCallLeg->msSide->handleRemoteEvent(h323InMsg, remoteEvent, remMsgData);
  }

  if (causeValue)
  {
	  DBG_ERROR("{\nMNEThoERROR(cs::handleH323MsgExtHo): release hoCallLeg due to hoCall issue\n}\n");
      handleReleaseCallLegExtHo(hoCallLeg);
  }

  DBG_LEAVE();
  return causeValue;
}

//ext-HO <xxu:05-10-01>
bool
CCSessionHandler::setupTermExtHo(IntraL3Msg_t* h323InMsg)
{
  DBG_FUNC("CCSessionHandler::setupTermExtHo", CC_EI_LAYER);
  DBG_ENTER();

  if ( (handoverSession == NULL) || (handoverSession !=targetHandoverSessionExtHo) )
  {
      DBG_ERROR("{\nMNETeiERROR(cs::setupTermExtHo): no call session for hoCall SETUP (callId=%d,h323Handle=%x)\n}\n",
                 parent->callIndex, h323InMsg->call_handle);
      return false;
  }
     
  //Invoke virtual function
  hoCallLeg->setupTermExtHo(h323InMsg);
      
  DBG_LEAVE();
  return(true);
}

//ext-HO <xxu:05-16-01>
JCCEvent_t 
CCSessionHandler::h323MapToEventExtHo(IntraL3Msg_t* h323InMsg, 
                                      CCRemMsgData_t &remMsgData) 
{
  JCCEvent_t remoteEvent;

  DBG_FUNC("CCSessionHandler::h323MapToEventExtHo", CC_HO_LAYER);
  DBG_ENTER();

  // Copy the incoming VOIP message to the remote message data structure 
  remMsgData = h323InMsg->l3_data.voipMsg;

  switch (h323InMsg->message_type)
  {
  case INTRA_L3_CC_Q931_SETUP:
       remoteEvent = THC_EHO_REMOTE_TERM_ADDRESS;
       remMsgData.callOffering = h323InMsg->l3_data.setup.callOffering;
       break;
      
  case INTRA_L3_CC_Q931_CALL_PROC:
       remoteEvent = OHC_EHO_REMOTE_CALL_PROGRESS;
       break;
      
  case INTRA_L3_CC_Q931_CONNECT:
       remoteEvent = OHC_EHO_REMOTE_ANSWER;
       break;
      
  case INTRA_L3_CC_Q931_DISCONNECT:
       remoteEvent = HC_EHO_REMOTE_DISCONNECT_REQ;
       break;
      
  case INTRA_L3_CC_H245_CHANN_READY:
       remoteEvent = HC_EHO_REMOTE_CONNECTION_CONTROL;
       break;

  case INTRA_L3_CC_H245_CHANN_SELECT:
       remoteEvent = HC_EHO_REMOTE_CHANN_SELECT;
       break;

  default:
       remoteEvent = JCC_NULL_EVENT;
  }

  if (remoteEvent==JCC_NULL_EVENT)
  {
      DBG_WARNING("{\nMNEThoWARNING(cs::h323MapToEventExtHo): unexpected h323 message (msgType=%d,callId=%d)\n}\n",
                   h323InMsg->message_type,parent->callIndex);
  } else
  {
      DBG_TRACE("{\nMNEThoTRACE(cs::h323MapToEventExtHo): received H323 Message (msgType=%d,event=%d,callId=%d)\n}\n",
		  	     h323InMsg->message_type, remoteEvent, parent->callIndex);
  }

  DBG_LEAVE();
  return (remoteEvent);
}

//ext-HO <xxu:05-16-01>
JCCEvent_t 
CCSessionHandler::h323MapToEventExtH3(IntraL3Msg_t* h323InMsg, 
                                      CCRemMsgData_t &remMsgData) 
{
  JCCEvent_t remoteEvent;

  DBG_FUNC("CCSessionHandler::h323MapToEventExtH3", CC_HO_LAYER);
  DBG_ENTER();

  // Copy the incoming VOIP message to the remote message data structure 
  remMsgData = h323InMsg->l3_data.voipMsg;

  DBG_TRACE("{\nMNEThoTRACE(cs::h323MapToEventExtH3): received H323 Message (msgType=%d,callId=%d)\n}\n",
	         h323InMsg->message_type, parent->callIndex);

  switch (h323InMsg->message_type)
  {
  case INTRA_L3_CC_Q931_SETUP:
       remoteEvent = THC_EHO_REMOTE_TERM_ADDRESS;
       remMsgData.callOffering = h323InMsg->l3_data.setup.callOffering;
       break;
      
  case INTRA_L3_CC_Q931_CALL_PROC:
       remoteEvent = OHC_EHO_REMOTE_CALL_PROGRESS;
       break;
      
  case INTRA_L3_CC_Q931_CONNECT:
       remoteEvent = OHC_EHO_REMOTE_ANSWER;
       break;
      
  case INTRA_L3_CC_Q931_DISCONNECT:
       remoteEvent = HC_EHO_REMOTE_DISCONNECT_REQ;
       break;
      
  case INTRA_L3_CC_H245_CHANN_READY:
       remoteEvent = HC_EHO_REMOTE_CONNECTION_CONTROL;
       break;

  case INTRA_L3_CC_H245_CHANN_SELECT:
       remoteEvent = HC_EHO_REMOTE_CHANN_SELECT;
       break;

  default:
       remoteEvent = JCC_NULL_EVENT;
  }

  if (remoteEvent==JCC_NULL_EVENT)
  {
      DBG_WARNING("{\nMNEThoWARNING(cs::h323MapToEventExtH3): unexpected h323 message (msgType=%d,callId=%d)\n}\n",
                   h323InMsg->message_type,parent->callIndex);
  } else
  { 
      DBG_TRACE("{\nMNEThoTRACE(cs::h323MapToEventExtH3): received H323 Message (msgType=%d, event=%d, callId=%d)\n}\n",
	             h323InMsg->message_type, remoteEvent, parent->callIndex);
  }

  DBG_LEAVE();
  return (remoteEvent);
}


void
CCSessionHandler::setReleaseCause(T_CNI_RIL3_CAUSE_VALUE cause)
{
   releaseCause = cause;
}


//ext-HO <xxu:07-08-01>
BOOL
CCSessionHandler::hoCallVoiceConnected(void)
{
	if (hoCallLeg->msSide != NULL)
	{
		return (hoCallLeg->voiceConnected);
	} else
	{
		DBG_WARNINGho("{\nMNEThoWARNING(cs::hoCallVoiceConnected): no hoCall context any more (hoAgId=%d,callId=%d)\n}\n",
			           handoverSession->hoAgId_, handoverSession->callIndex);
		return false;
	}
}

HalfCallStates_t
CCSessionHandler::hoCallState(void)
{
	if (hoCallLeg->msSide != NULL)
	{
		return (hoCallLeg->msSide->callState_);
	} else
	{
		DBG_WARNINGho("{\nMNEThoWARNING(cs::hoCallState): no hoCall context found! (hoAgId=%d,callId=%d)\n}\n",
			           handoverSession->hoAgId_, handoverSession->callIndex);
		return (EHO_ST_NULL);
	}
}

//ext-HO <xxu:06-08-01>
BOOL
CCSessionHandler::h3CallVoiceConnected(void)
{
	if (hoCallLeg->msSide != NULL)
	{
		return (hoCallLeg->voiceConnected);
	} else
	{
		DBG_WARNINGho("{\nMNEThoWARNING(cs::h3CallVoiceConnected): no h3Call context any more (hoAgId=%d,callId=%d)\n}\n",
			           handoverSession->hoAgId_, handoverSession->callIndex);
		return false;
	}
}


//ext-HO <chenj:06-11-01>
bool
CCSessionHandler::setupOrigExtHo(void)
{
  DBG_FUNC("CCSessionHandler::setupOrigExtHo", CC_EO_LAYER);
  DBG_ENTER();

  if ( (handoverSession == NULL) || (handoverSession != anchorHandoverSession) )
  {
      DBG_ERROR("{\nMNETeoERROR(cs::setupOrigExtHo): no call session for hoCall SETUP (callId=%d)\n}\n",
                 parent->callIndex);
      return false;
  }
     
  hoCallLeg->setupOrigExtHo();
      
  DBG_LEAVE();
  return(true);
}

//ext-HO <chenj:06-11-01>
bool
CCSessionHandler::setupOrigExtHo3(void)
{
  DBG_FUNC("CCSessionHandler::setupOrigExtHo3", CC_EO_LAYER);
  DBG_ENTER();

  if ( (handoverSession == NULL) || (handoverSession != anchorHandoverSession) )
  {
      DBG_ERROR("{\nMNETeoERROR(cs::setupOrigExtHo): no call session for hoCall SETUP (callId=%d)\n}\n",
                 parent->callIndex);
      return false;
  }
     
  h3CallLeg->setupOrigExtHo();
      
  DBG_LEAVE();
  return(true);
}