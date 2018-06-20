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
// File        : CallLeg.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

//
#include "taskLib.h"

#include "jcc/JCCLog.h"

#include "logging/VCLOGGING.h"

#include "defs.h"
#include "oam_api.h"
#include "pm/pm_class.h"

#include "CC/CCSessionHandler.h"

#include "CC/CallLeg.h"

extern int CALL_CDR_10M;

#include "new.h"

// Temporarily use csu_head instead of csu_intf
#include "csunew/csu_head.h"

//CDR <xxu:08-24-00>
//CDR <xxu:08-21-00> BEGIN
#include "JCC/LUDBapi.h"
#include "CDR/CdrVoiceCall.h"
#include "CDR/CdrSSA.h"
extern BtsBasicPackage        ccBtsBasicPackage;

// *******************************************************************
// forward declarations.
// *******************************************************************

//BCT-TEST
//BCT <xxu:07-12-00> BEGIN

T_CNI_RIL3_SI_TI
//CallLeg::getBctTi()
TwoPartyCallLeg::getBctTi()
{
	return (parent->bctTi_);
}
bool
//CallLeg::getBctInvoke()
TwoPartyCallLeg::getBctInvoke()
{
      return (parent->bctInvoke_);
}

TwoPartyCallLeg*
//CallLeg::getBctLeg()
TwoPartyCallLeg::getBctLeg()
{
      return (parent->bctLeg_);
}

T_CNI_IRT_ID
//CallLeg::getEntryId()
TwoPartyCallLeg::getEntryId()
{
      return (parent->entryId);
}

CCHandover*
//CallLeg::getHandoverSession()
TwoPartyCallLeg::getHandoverSession()
{
      return (parent->handoverSession);
}

CCAnchorHandover*
//CallLeg::getAnchorHandoverSession()
TwoPartyCallLeg::getAnchorHandoverSession()
{
      return (parent->anchorHandoverSession);
}

HJCRTPSESSION
//CallLeg::getHoRtpSession()
TwoPartyCallLeg::getHoRtpSession()
{
      if ( (parent->handoverSession != NULL) &&
	     (parent->handoverSession == parent->anchorHandoverSession) )
            return (parent->handoverSession->hoRtpSession_);
      else
            return (NULL);
}				
//BCT <xxu:07-12-00> END

T_CNI_LAPDM_OID             
CallLeg::oid()
{
  return(parent->oid);
}
 
T_CNI_IRT_ID
CallLeg::entryId()
{
  return(parent->entryId);
}

short
CallLeg::ludbIndex()
{
  return(parent->ludbIndex);
}

CallLeg::CallLeg ()
{
}

TwoPartyCallLeg::TwoPartyCallLeg (CCSessionHandler *session, 
                                  int callId, MSG_Q_ID qid,
                                  short legNum)
{
  CallLeg::parent = session;
  CallLeg::callIndex = callId;
  CallLeg::msgQId = qid;
  CallLeg::callLegNum = legNum;

  msSide = NULL;

  voiceConnected = false;

  // allocate half call store to the maximum size possible.
  halfCallStorage = 
    operator new[] ((sizeof(CCOrigHalfCall) > sizeof(CCTermHalfCall)) 
                    ? sizeof(CCOrigHalfCall)
                    : sizeof(CCTermHalfCall));

  //PR1378 <xxu:08-16-00> Begin

  UINT16 uint16TimerValue;
  int timerValue;

  uint16TimerValue = (((UINT16)(legNum)) & (0x000F)) << 8;
  uint16TimerValue = uint16TimerValue | (((UINT16)(callIndex)) & 0x00FF);
  timerValue = (short)uint16TimerValue;
 
  tclTimer = new JCCTimer(callTimerExpiry, timerValue);

  //PR1378 <xxu:08-16-00> End

  //CDR <xxu:08-22-00> BEGIN
  uint16TimerValue = (((UINT16)(legNum)) & (0x000F)) << 8;
  uint16TimerValue = uint16TimerValue | (((UINT16)(callIndex)) & 0x00FF);
  timerValue = (short)uint16TimerValue;
  timerValue |= 0xF0000000;  //add CDR timer signature
  ccCdrTimer = new JCCTimer(callTimerExpiry, timerValue);
  //CDR <xxu:08-22-00> END

  initData();

}

void
CallLeg::initData ()
{
  DBG_FUNC("CallLeg::initData", CC_CALLLEG_LAYER);
  DBG_ENTER();

  idle = true;

  cdrIndex = 0;
  callRefValue = 0;

  DBG_LEAVE();
}

void
TwoPartyCallLeg::cleanup (T_CNI_RIL3_CAUSE_VALUE cause)
{
  DBG_FUNC("TwoPartyCallLeg::cleanup", CC_CALLLEG_LAYER);
  DBG_ENTER();

  if (msSide != NULL)
    {
      msSide->rlsCause.causeValue = cause;

      //CAUSE<xxu:03-21-00> BEGIN

      msSide->voipReleaseCause_ = cause;

      //CAUSE<xxu:03-21-00> END
           
      msSide->cleanup();
    }

  initData();

  DBG_LEAVE();
}

void
TwoPartyCallLeg::initData ()
{
  DBG_FUNC("TwoPartyCallLeg::initData", CC_CALLLEG_LAYER);
  DBG_ENTER();

  waitingCallLeg = false;

  //BCT <xxu:07-13-00> BEGIN
  if ( (parent->bctLeg_ != NULL) || (parent->bctInvoke_) )
  {
      parent->bctTi_ = ORIG_OR_MASK;
      parent->bctInvoke_ = false;
      parent->bctLeg_ = NULL;
  }
  //BCT <xxu:07-13-00> END

  //BCT <xxu:09-22-00>
  bctArrival_ = false;

  //CDR <xxu:08-22-00>
  ccCdrCallState = false;
  ccCdrTimer->cancelTimer();  
  tclTimer->cancelTimer();

  h323CallHandle = VOIP_NULL_CALL_HANDLE;
  disconnectSentToVoip = false;

  procType = CC_PROC_NULL;
  msSide = NULL;

  h245ChannState = RES_ST_IDLE;

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
          
          JCCLog2("CSU Error : Disconn. Failure, (Result = %d) for (RTPHandle = %p)\n",  
                  csuResult, (int)rtpSourcePort.portId.rtpHandler);
          DBG_ERROR("CSU Error : Disconn. Failure, (Result = %d) for (RTPHandle = %p)\n",  
                    csuResult, (int)rtpSourcePort.portId.rtpHandler);
              
          // Can only generate OA&M log. 
        }
      
      voiceConnected = false;   
    }

  isEmergencyCall = false;

  CallLeg::initData ();
  DBG_LEAVE();

}

void
TwoPartyCallLeg::handleReleaseCall()
{
  DBG_FUNC("TwoPartyCallLeg::handleReleaseCall", CC_CALLLEG_LAYER);
  DBG_ENTER();

  // check both sides -  for now no VOIP in CC.
  // if ((origSide->callState_ == HC_ST_FREE) && 
  //     (termSide->callState_ == HC_ST_FREE) )

  DBG_TRACE("CC Info: CallLeg releaseCall cause(%d) ccCdrCallState(%d)\n",
             msSide->rlsCause.causeValue, ccCdrCallState);

  //CDR <xxu:08-22-00> BEGIN
  //Collect CDR data for all call releases if its call leg already set up
  if (msSide != NULL && this != parent->bctLeg_)
  {  
     CdrRecCauseTerm_t cdrCause;
  
     if (ccCdrCallState)
     {
         if ( msSide->rlsCause.causeValue != CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING )
              cdrCause = CDR_REC_STABLE_CALL_ABNORMAL_TERM;
         else
	        cdrCause = CDR_REC_NORM_RELEASE;
     } else
         cdrCause = CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT;

     ccCdr.release(msSide->rlsCause.causeValue, cdrCause);
     ccCdrTimer->cancelTimer();
     ccCdrCallState = false;      
  }
  //CDR <xxu:08-22-00> END
             
  initData();
  DBG_LEAVE();
}

void
CallLeg::printData (JCCPrintStFn fnPtr)
{
}

void
TwoPartyCallLeg::printData (JCCPrintStFn fnPtr)
{
  char * tpClStr1Ptr = "Two Party Call Leg :\n";
  char tpClStr2[120];
  
  // Pre-format the call data Strings.
  sprintf(&tpClStr2[0],
          "(proc %d) (h245ChSt %d) (conn. %d) (discSenttoVoip %d) (callHnd %p) (callLegNum %p)\n",
          procType,
          h245ChannState,
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
TwoPartyCallLeg::setupOrig (T_CNI_RIL3_SI_TI            ti,
                            T_CNI_RIL3_CM_SERVICE_TYPE  serviceRequested)
{
  DBG_FUNC("TwoPartyCallLeg::setupOrig", CC_CALLLEG_LAYER);
  DBG_ENTER();

  idle = false;
  procType = CC_PROC_MOB_ORIG;

  //PR1378 <xxu:08-16-00>     
  origSide = new (halfCallStorage) CCOrigHalfCall(this, tclTimer);
  msSide = origSide;

  // set this boolean to true until we actually send a setup request to VOIP.
  disconnectSentToVoip = true;

  isEmergencyCall = (serviceRequested == CNI_RIL3_CM_SERVICE_TYPE_EMERGENCY_CALL); 

  origSide->setupNewCall(ti, serviceRequested);

  //CDR <xxu:08-21-00> BEGIN
  ccCdr.initialize();

  if (isEmergencyCall && parent->mobileId.mobileIdType==CNI_RIL3_IMEI)
  {
      ccCdr.setServedIMEI(&parent->mobileId);
      ccCdr.seisure_EmergencyOrigination(ti);
  }
  else
  {
      if (ludbGetSubscInfoExists(parent->ludbIndex))
      {
          ccCdr.setServedIMSI(ludbGetMobileIdPtr(parent->ludbIndex));
          T_SUBSC_IE_ISDN *msisdn;
          msisdn = ludbGetMSISDNPtr(parent->ludbIndex);
          msisdn->num_plan[0] = 0x11;
          ccCdr.setServedMSISDN(msisdn);
      }

      ccCdr.seisure_NormalOrigination(ti);
  }
  //Load up with Location area info
  T_CNI_RIL3_IE_LOCATION_AREA_ID lai;
  T_CNI_RIL3_IE_CELL_ID ci;

  ci.ie_present = true;
  ci.value = ((short)(ccBtsBasicPackage.bts_ci));

  lai.ie_present = true;
  lai.mcc[0] = ((char*)&(ccBtsBasicPackage.bts_mcc))[0];
  lai.mcc[1] = ((char*)&(ccBtsBasicPackage.bts_mcc))[1];
  lai.mcc[2] = ((char*)&(ccBtsBasicPackage.bts_mcc))[2];
  lai.mnc[0] = ((char*)&(ccBtsBasicPackage.bts_mnc))[0];
  lai.mnc[1] = ((char*)&(ccBtsBasicPackage.bts_mnc))[1];
  lai.lac    = ((short)(ccBtsBasicPackage.bts_lac));

  ccCdr.setLocationAreaCID(&lai,&ci);

  DBG_TRACE("mcc(%d,%d,%d) mnc(%d,%d) lac(%d) ci(%d)\n",
		 lai.mcc[0],lai.mcc[1],lai.mcc[2],lai.mnc[0],lai.mnc[1],lai.lac,ci.value);

  DBG_TRACE("MCC(%d,%d,%d) MNC(%d,%d)\n",
		((char*)&(ccBtsBasicPackage.bts_mcc))[0],
		((char*)&(ccBtsBasicPackage.bts_mcc))[1],
		((char*)&(ccBtsBasicPackage.bts_mcc))[2],
		((char*)&(ccBtsBasicPackage.bts_mnc))[0],
		((char*)&(ccBtsBasicPackage.bts_mnc))[1]);

  //CDR <xxu:08-21-00> END

  DBG_LEAVE();

}

bool                         
TwoPartyCallLeg::isActive(void)
{
  DBG_FUNC("TwoPartyCallLeg::isActive", CC_CALLLEG_LAYER);
  DBG_ENTER();
  
  if (msSide != NULL) {
    DBG_LEAVE();
    return(msSide->callState_==HC_ST_ACTIVE);
  }
}

void
TwoPartyCallLeg::setupTerm (VOIP_CALL_HANDLE voipCallHandle,
                            int              index,
                            int              refValue,
                            bool             callWaiting)
{
  DBG_FUNC("TwoPartyCallLeg::setupTerm", CC_CALLLEG_LAYER);
  DBG_ENTER();

  idle = false;
  procType = CC_PROC_MOB_TERM;

  //PR1378 <xxu:08-11-00>
  termSide = new (halfCallStorage) CCTermHalfCall(this, tclTimer);

  msSide = termSide;
  h323CallHandle = voipCallHandle;
  waitingCallLeg = callWaiting;

  //CDR <xxu:08-21-00>
  ccCdr.initialize();
        
  if (ludbGetSubscInfoExists(parent->ludbIndex))
  {
      ccCdr.setServedIMSI(ludbGetMobileIdPtr(parent->ludbIndex));
      T_SUBSC_IE_ISDN *msisdn;
      msisdn = ludbGetMSISDNPtr(parent->ludbIndex);
      msisdn->num_plan[0] = 0x11;
      ccCdr.setServedMSISDN(msisdn);
      //ccCdr.setServedMSISDN(ludbGetMSISDNPtr(parent->ludbIndex));
  }

  ccCdr.seisure_NormalTermination(callLegNum);
  ccCdr.setQ931Crv( (unsigned short)index );

  //ccCdr.setCallingPartyNumber(h323InMsg->setup.CallingPartyNumber);
  //ccCdr.setCalledPartyNumber(h323InMsg->setup.CalledPartyNumber);

  //Load up with Location area info
  T_CNI_RIL3_IE_LOCATION_AREA_ID lai;
  T_CNI_RIL3_IE_CELL_ID ci;

  ci.ie_present = true;
  ci.value = ((short)(ccBtsBasicPackage.bts_ci));

  lai.ie_present = true;
  lai.mcc[0] = ((char*)&(ccBtsBasicPackage.bts_mcc))[0];
  lai.mcc[1] = ((char*)&(ccBtsBasicPackage.bts_mcc))[1];
  lai.mcc[2] = ((char*)&(ccBtsBasicPackage.bts_mcc))[2];
  lai.mnc[0] = ((char*)&(ccBtsBasicPackage.bts_mnc))[0];
  lai.mnc[1] = ((char*)&(ccBtsBasicPackage.bts_mnc))[1];
  lai.lac    = ((short)(ccBtsBasicPackage.bts_lac));

  ccCdr.setLocationAreaCID(&lai,&ci);

  DBG_TRACE("mcc(%d,%d,%d) mnc(%d,%d) lac(%d) ci(%d)\n",
		 lai.mcc[0],lai.mcc[1],lai.mcc[2],lai.mnc[0],lai.mnc[1],lai.lac,ci.value);

  DBG_TRACE("MCC(%d,%d,%d) MNC(%d,%d)\n",
		((char*)&(ccBtsBasicPackage.bts_mcc))[0],
		((char*)&(ccBtsBasicPackage.bts_mcc))[1],
		((char*)&(ccBtsBasicPackage.bts_mcc))[2],
		((char*)&(ccBtsBasicPackage.bts_mnc))[0],
		((char*)&(ccBtsBasicPackage.bts_mnc))[1]);

  DBG_LEAVE();
}

int
TwoPartyCallLeg::handleTimeoutMsg(IntraL3Msg_t    *ccInMsg)
{
  DBG_FUNC("TwoPartyCallLeg::handleTimeoutMsg", CC_CALLLEG_LAYER);
  DBG_ENTER();

  if (msSide != NULL)
    {
      //CDR <xxu:08-22-00> BEGIN
 	int retVal;

      retVal = 0;

      if ( (ccInMsg->signature & 0xF0000000) == 0xF0000000 )
      {
          // handle 10min call duration timeout for CDR
          if (ccCdrCallState)
          {
              ccCdr.generateCallRecord(true);
              ccCdrTimer->setTimer(CALL_CDR_10M);
          }
      } else 
      { 
          if ( (msSide->callState_ == OHC_ST_OFFERING_CALL) && parent->bctInvoke_ && parent->bctMainLeg_)
          {
               CCdrSSA	bctCdr;
               if (ludbGetSubscInfoExists(parent->ludbIndex))
               {
                   bctCdr.setServedIMSI(ludbGetMobileIdPtr(parent->ludbIndex));
                   bctCdr.setServedMSISDN(ludbGetMSISDNPtr(parent->ludbIndex));
               }
               bctCdr.setCorrelatedEventRefNumber(parent->bctMainLeg_->ccCdr.getEventRefNumber());
               bctCdr.generateRecord(CDR_SS_ACTION_INVOCATION, SUPP_SERV_ECT, CNI_RIL3_CAUSE_TEMPORARY_FAILURE);
           }

           retVal = msSide->handleTimeoutMsg(ccInMsg);
      }

      //CDR <xxu:08-22-00> END
      
      DBG_LEAVE();
      return (retVal);
    }
  
  DBG_ERROR("Call Leg Error : Unexpected Timeout Message\n");
  DBG_LEAVE();
  return(0);
}




