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
// File        : CCMobIntHnd.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : CC Handlers for events triggered internal to ViperCell.
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// 
#include "CC/CCHalfCall.h"
#include "CC/CallLeg.h"
#include "CC/CCSessionHandler.h"

#include "CC/CCUtil.h"
#include "CC/CallConfig.h"

#include "logging/VCLOGGING.h"
#include "jcc/JCCLog.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"

#include "stdio.h"

// Temporarily use csu_head instead of csu_intf
#include "csunew/csu_head.h"

#include "JCC/LUDBapi.h"

#include "JCC/JCCUtil.h"

bool  forcedCodec = false;

extern bool callTrace;

INT16  forcedCodecSpeechVersion = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_2;

#include "rm/rm_head.h"
extern u8 rm_EntryIdToTrxSlot(T_CNI_IRT_ID entryId, u16 *gsmHandler);

// *******************************************************************
// forward declarations.
// *******************************************************************

// Timeout during THC_ST_CALL_CONFIRMED, THC_ST_ALERTING,
// THC_ST_PRESENTING_CALL and OHC_ST_CONNECT_INDICATION
// are all handled the same as CC_TWO_STAGE_DISCONNECT
// event, which is basically the logic in handleDisconnectReq
// !< But in these cases, Release Cause is needed when used to 
// initiate call clearing. Set it in the timeout handlers.

void 
forcedCodecOn (void)
{
  forcedCodec = true;
}

void 
forcedCodecOff (void)
{
  forcedCodec = false;
}

void 
setCodecSpeechVersion (INT16 speechVersion)
{
  forcedCodecSpeechVersion = speechVersion;
}

// Connection Manager functionality
JCCEvent_t 
CCHalfCall::handleConnReq(void)
{

  DBG_FUNC("CCHalfCall::handleConnReq", CC_HALFCALL_LAYER);
  DBG_ENTER();

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

  if ( (callState_==HC_ST_ACTIVE && chState_==CH_ST_RREQ) || (parent->bctArrival_) )
  {   
      //BCT <xxuL09-22-00> BEGIN
      if (parent->bctArrival_)
      {
          //Bridge voice path for BCT
          DBG_TRACE("CC->BCT(csuDconn): chState(%d,%d) callSt(%d) bct(%d,%d) callHndl(%x) snk(%d,%x)\n",
                     parent->h245ChannState, channState_,callState_,
                     parent->getBctInvoke(), parent->bctArrival_,
                     parent->h323CallHandle,
                     rfSourcePort.portType, rfSourcePort.portId.rtpHandler);
          parent->bctArrival_ = false;
      }

      DBG_LEAVE();
      return (CC_CONNECT_COMPLT);
  } else
  {
      // CDR <xxu:08-21-00> BEGIN
      parent->ccCdr.answer((unsigned char)BEARER_SPEECH, (unsigned char)currSpeechCh_.version);
      parent->ccCdrCallState = true;
      parent->ccCdrTimer->setTimer(CALL_CDR_10M);

      // CDR <xxu:08-21-00> END

      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
  }
}

JCCEvent_t 
CCHalfCall::handleNullEvent(void)
{

  DBG_FUNC("CCHalfCall::handleNullEvent", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // Do Nothing.
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

// Disconnect Timeout
JCCEvent_t 
CCHalfCall::handleDisconnectTimeout(void)
{

  DBG_FUNC("CCHalfCall::handleDisconnectTimeout", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // !< Release Cause (same as the Disconnect Cause) needs to be included.
  // !< A second Release Cause may be included(timer expiry)

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
      
      DBG_WARNING("CC Log: Disconnect got no response from MS on (TRX=%d),(Slot=%d),(entryId=%d)\n",
                  trx, slot, parent->entryId());
      if ((isdnPtr = ludbGetMSISDNPtr(parent->ludbIndex())) != NULL)
        {
          DBG_WARNING("CC Log: MS (DN=%s)\n",
                      isdnPtr->digits);
        }
    }


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

JCCEvent_t 
CCHalfCall::handleReleaseTimeout(void)
{

  DBG_FUNC("CCHalfCall::handleReleaseTimeout", CC_HALFCALL_LAYER);
  DBG_ENTER();

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
      
      DBG_WARNING("CC Log: Release got no response from MS on (TRX=%d),(Slot=%d),(entryId=%d),(releaseCount=%d)\n",
                  trx, slot, parent->entryId(), releaseCounter);
      if ((isdnPtr = ludbGetMSISDNPtr(parent->ludbIndex())) != NULL)
        {
          DBG_WARNING("CC Log: MS (DN=%s)\n",
                      isdnPtr->digits);
        }
    }


  if (releaseCounter == 0)
    {
      // already repeated release msg.
      // !< Transaction Id. has to be released, maybe in res_release? - Yes!

      // Release RR resources
      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }

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
  releaseCounter = 0;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleMobResRelease(void)
{

  DBG_FUNC("CCHalfCall::handleMobResRelease", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // Release Local Resources
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
                  csuResult,  (int)rtpSourcePort.portId.rtpHandler);
          DBG_ERROR("CSU Error : Disconn. Failure, (Result = %d) for (RTPHandle = %p)\n",  
                    csuResult, rtpSourcePort.portId.rtpHandler);

          // Can only generate OA&M log. 
        }

      parent->voiceConnected = false;   

    }

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

JCCEvent_t 
CCHalfCall::handleChannAssignReq(void)
{

  DBG_FUNC("CCHalfCall::handleChannAssignReq", CC_HALFCALL_LAYER);
  DBG_ENTER();


  IntraL3Msg_t rrOutMsg;

  if (forcedCodec)
    {
      currSpeechCh_.version = 
        (T_CNI_RIL3_SPEECH_VERSION)forcedCodecSpeechVersion;
    }

  // Assign Local Resources
  // Send internal RR msg
  rrOutMsg.l3_data.channAssignCmd.channelType.bearerType = BEARER_SPEECH;

  rrOutMsg.l3_data.channAssignCmd.channelType.speechChann
    = currSpeechCh_;

  DBG_TRACE("CC->RR: Chann Assignment Request with Speech Version: %d",  
            rrOutMsg.l3_data.channAssignCmd.channelType.speechChann.version);

  parent->parent->sendRRMsg((IntraL3PrimitiveType_t)(parent->callLegNum), INTRA_L3_RR_CHANN_ASSIGN_CMD, &rrOutMsg);

  channState_ = RES_ST_ASS_REQ;
  
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCHalfCall::handleChannAssignComplt(void)
{

  DBG_FUNC("CCHalfCall::handleChannAssignComplt", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // Synchronize with the main state machine
  channState_ = RES_ST_ASSIGNED;

  if (parent->procType == CC_PROC_MOB_ORIG) 
    {
      T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

      ULONG setupCompltTime;
      
      if (!alertingSent_)
        {
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
        }
      
      if ((callState_ == OHC_ST_CONNECT_INDICATION) &&
          !connectSent_ )
        {
          
          // Build the Connect Message towards the MS. 
          if ( (msOutResult = buildConnect (&msEncodedMsg))
               != CNI_RIL3_RESULT_SUCCESS )
            {
              // Log the error. Release RR resources
              callState_ = HC_ST_NULL;
              DBG_LEAVE();
              return (CC_RES_RELEASE);
            }
          
          // start the connect timer 
          hcTimer->setTimer(CALL_CONNECT_T313);
          
          connectSent_ = true;
        }
    }
  else if (parent->procType == CC_PROC_MOB_TERM)
    {
      PM_CCMeasurement.succMobileTerminatingCalls.increment();      
    }

  // Connection Manager functionality
  if (parent->h245ChannState == RES_ST_ASSIGNED)
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
CCHalfCall::handleChannAssignFail(void)
{

  DBG_FUNC("CCHalfCall::handleChannAssignFail", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // Synchronize with the main state machine  
  channState_ = RES_ST_IDLE;
  
  rlsCause.causeValue = CNI_RIL3_CAUSE_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE;

  //CAUSE<xxu:03-21-00> BEGIN

    voipReleaseCause_ = CNI_RIL3_CAUSE_REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE;
          
  //CAUSE<xxu:03-21-00> END

  DBG_LEAVE();
  return (CC_TWO_STAGE_DISCONNECT);
}

JCCEvent_t 
CCHalfCall::handleMMRelInd(void)
{

  DBG_FUNC("CCHalfCall::handleMMRelInd", CC_HALFCALL_LAYER);
  DBG_ENTER();


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
          DBG_ERROR("CSU Error : Disconn. Failure, (Result = %d) for (RTPHandle = %p)\n",  
                    csuResult, rtpSourcePort.portId.rtpHandler);

          // Can only generate OA&M log. 
        }

      parent->voiceConnected = false;   

    }

  // Always do sendRemote at the end of the handler.
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

JCCEvent_t 
CCHalfCall::handleMMEstCnf(void)
{

  DBG_FUNC("CCHalfCall::handleMMEstCnf", CC_HALFCALL_LAYER);
  DBG_ENTER();

  T_CNI_LAPDM_L3MessageUnit     msEncodedMsg;

  // !! For Setup, BC 1 may be ommitted if only one directory number
  // is allocated for all services. - 9.07 - build one BC from the 4.08
  // first simple example

  // !< For Setup, Called party subaddress may be included, if Orig. included it
  // Build the Setup Message towards the MS. - No Mandatory IEs
  
  char callTaskName[20];
  
  sprintf(&callTaskName[0], 
          "Call-%d EntryId-%d\0", 
          parent->callIndex,
          parent->entryId()); 

  if ( (msOutResult = buildSetup (&msEncodedMsg))
       != CNI_RIL3_RESULT_SUCCESS )
    {
      // Log the error. Release RR resources
      callState_ = HC_ST_NULL;
      DBG_LEAVE();
      return (CC_RES_RELEASE);
    }
  
  // start the disconnect timer 
  hcTimer->setTimer(CALL_SETUP_T303);

  callState_ = THC_ST_PRESENTING_CALL;

  DBG_LEAVE();

  if (callTrace)
    {
      Dbg.Func(&callTaskName[0], CC_LAYER);
    }

  return (CC_MESSAGE_PROCESSING_COMPLT);

}

JCCEvent_t 
CCHalfCall::handleMMEstRej(void)
{


  DBG_FUNC("CCHalfCall::handleMMEstRej", CC_HALFCALL_LAYER);
  DBG_ENTER();

  // Page Timeout

  if (mmInMsg->l3_data.pageReq.imsi == ludbGetMobileId(parent->ludbIndex()))
    {
      // continue
    }
  else
    {
//TEST
//printf("CC Log: receive handleMMEstRej for (ludbIndex = %d)\n", parent->ludbIndex());
//T_CNI_RIL3_IE_MOBILE_ID imsi_t1, imsi_t2;
//imsi_t1 = ludbGetMobileId(parent->ludbIndex());   
//imsi_t2 = mmInMsg->l3_data.pageReq.imsi;
//char *p_t1, *p_t2;
//p_t1 = (char*) &imsi_t1;
//p_t2 = (char*) &imsi_t2;
//printf("IMSI from MM message:\n");
//int i;
//for (i=0;i<9;i++)
//{ 
//     printf("%2x", (*p_t1++));
//}
//printf("\n");
//printf("IMSI retv from LUDB:\n");
//for (i=0;i<9;i++)
//{ 
//     printf("%2x", (*p_t2++));
//}
//printf("\n");

//ccEntryPrint(ccSession[mobInCall(parent->ludbIndex())], JCCPrintf);

      // late message for a different page request
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }

  //CAUSE<xxu:03-21-00> BEGIN

  // Always do sendRemote at the end of the handler.
  //voipReleaseCause_ = MOBILE_CAUSE_USER_NO_PAGE_RESPONSE;

  //rlsCause.causeValue = CNI_RIL3_CAUSE_NOT_ROUTE_TO_DESTINATION;

  voipReleaseCause_ = mmInMsg->cause;
  rlsCause.causeValue = mmInMsg->cause;

  //CAUSE<xxu:03-21-00> END

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

// !< For all msgs,local and remote, User-to-User support can be added later,
// connected subaddress is related to this.

JCCEvent_t
CCHalfCall::handleConnComplt(void)
{
  T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

  DBG_FUNC("CCHalfCall::handleConnComplt", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE ("CC CallLog @handleConnComplt: callId=%d, callState=%d, chState=%d\n",
              parent->callIndex, callState_, chState_);

  if (callState_ == HC_ST_ACTIVE && chState_ == CH_ST_RREQ)
  {
      //Voice path connected successfully for Retrive operation, send back RETV ACK
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
      }

      chState_ = CH_ST_IDLE;
      return (CC_MESSAGE_PROCESSING_COMPLT);
  }

  //Here handling other jobs triggered by CC_CONNECT_COMPLT in the future
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t
CCHalfCall::handleConnFail(void)
{
  T_CNI_LAPDM_L3MessageUnit	msEncodedMsg;

  DBG_FUNC("CCHalfCall::handleConnFail", CC_HALFCALL_LAYER);
  DBG_ENTER();

  DBG_TRACE ("CC CallLog @handleConnComplt: callId=%d, callState=%d, chState=%d\n",
              parent->callIndex, callState_, chState_);

  if (callState_ == HC_ST_ACTIVE && chState_ == CH_ST_RREQ)
  {
      // Log the software design bug, no workaround is proper here.
	JCCLog("CC->CC: Voice path connection failed\n");
	DBG_ERROR("CC->CC: Voice path connection failed\n");
  
      // Build the Retv Rej Message towards the MS. 
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

   //Here handling other jobs triggered by CC_CONNECT_FAILED in the future
   return (CC_MESSAGE_PROCESSING_COMPLT);
}
