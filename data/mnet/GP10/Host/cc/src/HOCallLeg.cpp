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
// File        : HOCallLeg.cpp
// Author(s)   : Joe Chen
// Create Date : 05-15-2001
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
//

#include <vxworks.h>
#include <string.h>
#include <taskLib.h>

#include "CC/CCUtil.h"


#include "Logging\vcmodules.h"

#include "CC/CCSessionHandler.h"

#include "CC/HOCallLeg.h"

#include "new.h"

extern bool callTrace;

// *******************************************************************
// 
// *******************************************************************


// *******************************************************************
// HOCallLeg class
// *******************************************************************


HOCallLeg::HOCallLeg (CCSessionHandler *session,
                      int               callId ,
                      MSG_Q_ID          qid    ,
                      short             legNum )
{
  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::HOCallLeg): entering......\n}\n");

    // Init parent class data
    HOCallLeg::parent     = session;
    HOCallLeg::callIndex  = callId ;
    HOCallLeg::msgQId     = qid    ;
    HOCallLeg::callLegNum = legNum ;

	msSide = NULL;
	voiceConnected = false;

	// allocate half call store to the maximum size possible.
	halfExtHoCallStorage = 
		operator new[] ((sizeof(HOMoHalfCall) > sizeof(HOMtHalfCall)) 
                    ? sizeof(HOMoHalfCall)
                    : sizeof(HOMtHalfCall));

    UINT16 uint16TimerValue;
    int timerValue;

    uint16TimerValue = (((UINT16)(legNum)) & (0x000F)) << 8;
    uint16TimerValue = uint16TimerValue | (((UINT16)(callIndex)) & 0x00FF);
    timerValue = (short)uint16TimerValue;
 
    tclTimer = new JCCTimer(callTimerExpiry, timerValue);

   initData ();

  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::HOCallLeg): leaving.\n}\n");
}


void
HOCallLeg::cleanup (T_CNI_RIL3_CAUSE_VALUE cause)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::cleanup): entering......\n}\n");

  if (msSide != NULL)
    {
      msSide->rlsCause.causeValue = cause;

      msSide->voipReleaseCause_ = cause;
           
      msSide->cleanup();
    }

  initData();

  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::cleanup): leaving.\n}\n");
}

void
HOCallLeg::initData ()
{
  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::cleanup): entering......\n}\n");

  tclTimer->cancelTimer();

  h323CallHandle = VOIP_NULL_CALL_HANDLE;
  disconnectSentToVoip = false;

  procType = CC_PROC_NULL;
  msSide = NULL;

  // Do the following only if entry id is valid. 
  // In terminations and cleanups, it may not be setup yet.
  // Release Connection resources, if there are any
  if (voiceConnected)
    {
      T_CSU_PORT_ID rtpSourcePort;
      T_CSU_RESULT_CON csuResult;
      
      rtpSourcePort.portType = CSU_RTP_PORT;
      rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(h323CallHandle);
      
      if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort))
          != CSU_RESULT_CON_SUCCESS)
        {
          DBG_ERRORho("{\nMNEThoTRACE(HOCallLeg::cleanup):CSU Error : Disconn. Failure, (Result = %d) for (RTPHandle = %p)\n}\n",  
                      csuResult, (int)rtpSourcePort.portId.rtpHandler);
              
          // Can only generate OA&M log. 
        }
      
      voiceConnected = false;   
    }

  CallLeg::initData ();

  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::cleanup): leaving.\n}\n");

}

void
HOCallLeg::handleReleaseCall()
{
  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::handleReleaseCall): entering......\n}\n");

  DBG_TRACEho("   CC Info: CallLeg releaseCall cause(%d)\n}\n",
                  msSide->rlsCause.causeValue);

  initData();

  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::handleReleaseCall): leaving.\n}\n");
}

void
HOCallLeg::printData (JCCPrintStFn fnPtr)
{
  char * tpClStr1Ptr = "HO Call Leg :\n";
  char tpClStr2[120];
  
  // Pre-format the call data Strings.
  sprintf(&tpClStr2[0],
          "(proc %d) (conn. %d) (discSenttoVoip %d) (callHnd %p) (callLegNum %p)\n",
          procType,
          voiceConnected,
          disconnectSentToVoip,
          h323CallHandle,
          callLegNum
          );

  (*fnPtr)(tpClStr1Ptr);
  (*fnPtr)(&tpClStr2[0]);

  if (msSide != NULL)
    {
      msSide->printData(fnPtr);
    }

}

void
HOCallLeg::setupOrigExtHo (void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::setupOrigExtHo): entering......\n}\n");

  idle = false;
  procType = CC_PROC_MOB_ORIG;

  origSide = new (halfExtHoCallStorage) HOMoHalfCall(this , tclTimer); 

  msSide = origSide;

  // set this boolean to true until we actually send a setup request to VOIP.
  disconnectSentToVoip = true;

  origSide->setupNewCall();

  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::setupOrigExtHo): leaving.\n}\n");

}

void
HOCallLeg::disconnectOrigExtHo (void)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::disconnectOrigExtHo): entering......\n}\n");

  origSide->disconnectNewCall();

  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::disconnectOrigExtHo): leaving.\n}\n");
}

void
HOCallLeg::setupTermExtHo (IntraL3Msg_t* h323InMsg)
{
  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::setupTermExtHo): entering......\n}\n");

  
  DBG_TRACEho("{\nMNETeiTRACE(hoLeg::setupTermExtHo): setup hoCall (hoAgId=%d, callId=%d)\n}\n",
	         hoAgId(),callIndexEHO());

  idle = false;
  procType = CC_PROC_MOB_TERM;

  termSide = new (halfExtHoCallStorage) HOMtHalfCall(this, tclTimer); 

  msSide = termSide;
  h323CallHandle = h323InMsg->call_handle;

  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::setupTermExtHo): leaving.\n}\n");
}

int
HOCallLeg::handleTimeoutMsg(IntraL3Msg_t    *ccInMsg)
{
  int retVal;

  retVal = 0;

  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::handleTimeoutMsg): entering......\n}\n");

  if (msSide != NULL)
    {
      retVal = msSide->handleTimeoutMsg(ccInMsg);

      DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::handleTimeoutMsg): leaving.\n}\n");
      return (retVal);
    }
  
  DBG_ERRORho("   Call Leg ERROR : Unexpected Timeout Message\n");
  DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::handleTimeoutMsg): leaving.\n}\n");
  return(retVal);
}


//ext-HO <xxu:06-07-01>
JcSubId_t 
HOCallLeg::hoAgId(void)
{
   if (parent->handoverSession != NULL)
       return(parent->handoverSession->hoAgId_);
   else
   {
	   DBG_ERRORho("{\nMNEThoERROR(holeg:hoAgId): retrieve hoAgId but no handover session\n}\n");
	   return (-1);

   }
}

//ext-HO <xxu:06-07-01>
int 
HOCallLeg::callIndexEHO(void)
{
   if (parent->handoverSession != NULL)
       return(parent->handoverSession->callIndex);
   else
   {
	   DBG_ERRORho("{\nMNEThoERROR(holeg:callIndex): retrieve callIndex but no handover session\n}\n");
	   return (-1);
   }
}

//ext-HO <xxu:06-07-01>
BOOL
HOCallLeg::authTerm(void)
{
	return ((parent->handoverSession==NULL) ? true:false);
}

//ext-HO <xxu:06-07-01>
T_CNI_RIL3_SPEECH_VERSION 
HOCallLeg::speechVersion(void)
{
	if (parent->handoverSession != NULL)
		return (parent->handoverSession->currSpeechCh_.version);
	else
	{
	   DBG_ERRORho("{\nMNEThoERROR(holeg:speechVersion): retrieve current speech version but no handover session\n}\n");
	   return (CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1);
	}
}

//ext-HO <xxu:06-07-01>
BOOL
HOCallLeg::cancelHoTimerIfNeeded(void)
{
   DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::cancelHoTimerIfNeeded): entering......\n}\n");

   BOOL ret = true;
   
   if  ( (parent->handoverSession->hoState_ == HAND_ST_TRG_HO_COMPLT)   && 
	     (parent->handoverSession ==parent->targetHandoverSessionExtHo)  )
   {
	   //external handover--handin scenario
	   DBG_TRACEho("MNETeiTRACE(hoLeg::cancelHoTimerIfNeeded): [PLMN->MNET HANDOVER COMPLETE](hoAgId=%d,callId=%d,entryId=%d)\n}\n",
		               hoAgId(),callIndexEHO(),entryId());

	   parent->sessionTimer->cancelTimer();

   } else if ( (parent->handoverSession->hoState_ == HAND_ST_ANCH_EHO_COMPLT) && 
	           (parent->handoverSession ==parent->anchorHandoverSession)      )
   {
  	   DBG_TRACEho("MNETeoTRACE(hoLeg::cancelHoTimerIfNeeded): [MNET->PLMN HANDOVER COMPLETE](hoAgId=%d,callId=%d,entryId=%d)\n}\n",
		               hoAgId(),callIndexEHO(),entryId());

       parent->sessionTimer->cancelTimer();

       // Swap HO data if all is completed and we had handover to THIRD
       if ( ( parent->anchorHandoverSession->hoType_ == HO_TY_EXT_HANDMSC3 ) ||
            ( parent->anchorHandoverSession->hoType_ == HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3 ) )
         {
            parent->anchorHandoverSession->swapH3Data();
            parent->anchorHandoverSession->hoType_ = HO_TY_EXT_HANDOVER;
         }
   } else
   {
 	   DBG_WARNINGho("MNETeoTRACE(hoLeg::cancelHoTimerIfNeeded): Handover Call established but waiting for HANDOVER COMPLETE(hoAgId=%d,callId=%d,entryId=%d)\n}\n",
                      hoAgId(),callIndexEHO(),entryId());
       
	   ret = true; //false;
   }

   DBG_TRACEho("{\nMNEThoTRACE(HOCallLeg::cancelHoTimerIfNeeded): leaving.\n}\n");
   return (ret);
}


