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
// File        : CCHoTrgHnd.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 05-01-99
// Description : Target Vipercell Handover Event handlers 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// 

#include "CC/CCHandover.h"

#include "CC/CCSessionHandler.h"

#include "CC/CCUtil.h"

#include "CC/CallConfig.h"

#include "jcc/JCCLog.h"
#include "logging/VCLOGGING.h"

#include "stdio.h"

// Temporarily use csu_head instead of csu_intf
#include "csunew/csu_head.h"

#include "CC/hortp.h"

#include "lapdm/cni_debug.h"

#include "pm/pm_class.h"

#ifdef _SOFT_MOB
extern T_CNI_LAPDM_OID softMobLapdmOid[];
#endif

extern MSG_Q_ID ccMsgQId;
extern INT32 forcedHandover;

//GCELL<tyu:06-01-01>
extern GlobalCellId_t MY_GLOBALCELL_ID;

//Ciphering <chenj:07-17-01>
extern unsigned char rm_DspCiphCap;
#define RM_DSPA52(X)   (X & 2)
#define RM_DSPA51(X)   (X & 1)

// *******************************************************************
// forward declarations.
// *******************************************************************

JCCEvent_t 
CCTargetHandover::handleTrgHoReqAck(void)
{
  
  DBG_FUNC("CCTargetHandover::handleTrgHoReqAck", CC_SESSION_LAYER);
  DBG_ENTER();

  T_CNI_L3_ID newId;
  newId.msgq_id =  msgQId;
  newId.sub_id = 0;                 // No sub id needed here

  // <Igal 1-6-00> Updated to standard IRT setup
  if (parent->setIrt(mmInMsg->entry_id, newId) == false)
  {
      sendMM (INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, parent->entryId, &mmOutMsg);

      DBG_LEAVE();
      return   (handleTrgHoReqNack());
  }

  // Setup the entry id, oid, and sapi
  parent->entryId = mmInMsg->entry_id;

  // GET OID AND SAPI
  T_CNI_RIL3_IRT_ENTRY	irtEntry;
  irtEntry.entry_id = mmInMsg->entry_id;
  
  if  ( (CNI_RIL3_IRT_Get_Entry_by_Entry_ID(&irtEntry))
	== false )
    {
      JCCLog1("CC Call Error : IRT Table problem for (entry id = %d)\n ", mmInMsg->entry_id);
      DBG_ERROR("CC Call Error : IRT Table problem for (entry id = %d)\n ", mmInMsg->entry_id);

      sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, parent->entryId, &mmOutMsg);
      DBG_LEAVE();
      return (handleTrgHoReqNack());
    }

  parent->oid = irtEntry.lapdm_oid;

  // Assign a new Ho RTP channel
  if ((hoRtpSession_ = AssignHandoverRtpChannel())
      == NULL)
    {
      // Do not proceed with handover
      JCCLog ("CC->Handover RTP Pool Error: assignment fail\n");
      DBG_ERROR ("CC->Handover RTP Pool Error: assignment fail\n");
      sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, parent->entryId, &mmOutMsg);
      DBG_LEAVE();
      return (handleTrgHoReqNack());
    }

  // Setup the remote RTP Port 
  SetRemoteHandoverRtpPort(hoRtpSession_,
                           hoVcIpAddress_,
                           otherHoIncomingRtpPort_);

  DBG_TRACE("CC HO tgt-handleTrgHoReqAck Info: loRtpHndl %p, reVcIpAddr %x, rePort %d\n",
            hoRtpSession_, hoVcIpAddress_, otherHoIncomingRtpPort_);

  // Make a connection between the rtp channel and the rf channel
  T_CSU_PORT_ID rfSourcePort, rtpSinkPort;
  T_CSU_RESULT_CON csuResult;

  rfSourcePort.portType = CSU_GSM_PORT;
  rtpSinkPort.portType = CSU_RTP_PORT;

  rfSourcePort.portId.gsmHandler = parent->entryId;
  rtpSinkPort.portId.rtpHandler = hoRtpSession_;

  if ((csuResult = csu_DuplexConnect(&rfSourcePort, &rtpSinkPort))
      != CSU_RESULT_CON_SUCCESS)
    {
      JCCLog3("CSU Error : Ho Duplex Conn. Failure, (Result = %d) for (rtp port = %x) and (entryId = %d)\n",  
              csuResult,
              (int)rtpSinkPort.portId.rtpHandler,
              parent->entryId);
      DBG_ERROR("CSU Error : Ho Duplex Conn. Failure, (Result = %d) for (rtp port = %x) and (entryId = %d)\n",  
                csuResult,
                (int)rtpSinkPort.portId.rtpHandler,
                parent->entryId);
      sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, parent->entryId, &mmOutMsg);
      DBG_LEAVE();
      return (handleTrgHoReqNack());

    }

  voiceConnected = true;

  // Send the Perform Ack message.

  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDOVER_ACK;
  vcOutMsg.msgData.perfHandoverAck.targetInRtpPort 
    = GetIncomingHandoverRtpPort(hoRtpSession_);
  vcOutMsg.msgData.perfHandoverAck.handCmd  = mmInMsg->l3_data.handReqAck.handCmd;

  // Dump the handover Command msg.
  // CNI_DumpHex(&vcOutMsg.msgData.perfHandoverAck.handCmd.buffer[0], 30);

  sendVcMsg(MODULE_CC, MODULE_CC);

  // start the T204 timer 
  parent->sessionTimer->setTimer(CALL_HAND_TRG_T204);

  hoState_ = HAND_ST_TRG_SND_HO_REQ_ACK;

  // 
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandover::handleTrgHoReqNack(void)
{

  DBG_FUNC("CCTargetHandover::handleTrgHoReqNack", CC_SESSION_LAYER);
  DBG_ENTER();

  // Send the Perform Nack message.

  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDOVER_NACK;
  vcOutMsg.msgData.perfHandoverNack.cause  = rrInMsg->l3_data.handReqNack.cause;

  sendVcMsg(MODULE_CC, MODULE_CC);

  initData();

  DBG_LEAVE();
  return (CC_RELEASE_CALL);
  //
}

JCCEvent_t 
CCTargetHandover::handleTrgHoComplt(void)
{
  
  DBG_FUNC("CCTargetHandover::handleTrgHoComplt", CC_SESSION_LAYER);
  DBG_ENTER();

  // cancel T204 timer
  parent->sessionTimer->cancelTimer();

  // Send the Complete Handover message.

  vcOutMsg.msgType = INTER_VC_CC_COMPLETE_HANDOVER;

  sendVcMsg(MODULE_CC, MODULE_CC);

  hoState_ = HAND_ST_TRG_HO_COMPLT;

  //
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandover::handleTrgT204TimerExpiry(void)
{
  DBG_FUNC("CCTargetHandover::handleTrgT204TimerExpiry", CC_SESSION_LAYER);
  DBG_ENTER();

  // Send  Handover Nack to the Source ViperCell. 
  // Ack was already sent! Is that Ok?

  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDOVER_NACK;
  vcOutMsg.msgData.perfHandoverNack.cause = JC_FAILURE_T204_TIMEOUT;

  sendVcMsg(MODULE_CC, MODULE_CC);

  if (voiceConnected)
    {
      // break connection
      T_CSU_PORT_ID rtpSourcePort;
      T_CSU_RESULT_CON csuResult;
	  
      rtpSourcePort.portType = CSU_RTP_PORT;
      rtpSourcePort.portId.rtpHandler = hoRtpSession_;
	  
      if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort))
          != CSU_RESULT_CON_SUCCESS)
        {
          JCCLog2("CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                  csuResult,
                  (int)rtpSourcePort.portId.rtpHandler);
          DBG_ERROR("CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                    csuResult,
                    (int)rtpSourcePort.portId.rtpHandler);
	  
          // Can only generate OA&M log. 
        }

      voiceConnected = false;
    }

  // Release the Ho RTP channel
  ReleaseHandoverRtpChannel(hoRtpSession_);
  hoRtpSession_ = NULL;

  initData();

  DBG_LEAVE();
  return (CC_RELEASE_CALL);
}

JCCEvent_t 
CCTargetHandover::handleTrgPostHoRls(void)
{
  DBG_FUNC("CCTargetHandover::handleTrgPostHoRls", CC_SESSION_LAYER);
  DBG_ENTER();

  vcOutMsg.msgType = INTER_VC_CC_RLS_CALL;
  vcOutMsg.msgData.rlsCall.cause = JC_FAILURE_BASE;

  sendVcMsg(MODULE_CC, MODULE_CC);

  DBG_LEAVE();
  return (handleAnchToTrgAbortHo());
}

JCCEvent_t 
CCTargetHandover::handleAnchToTrgPerfHoReq(void)
{

  DBG_FUNC("CCTargetHandover::handleAnchToTrgPerfHoReq", CC_SESSION_LAYER);
  DBG_ENTER();

  // Copy the Anchor information.
  hoVcIpAddress_ = vcInMsg->origVcAddress;
  hoVcCallIndex_ = vcInMsg->origSubId;

//GCELL<tyu:06-01-01> BEGIN
  hoGlobalCellId_ =  vcInMsg->msgData.perfHandoverReq.globalCellId;
//GCELL<tyu:06-01-01> END

  otherHoIncomingRtpPort_ = vcInMsg->msgData.perfHandoverReq.sourceInRtpPort;

  // Build a message for MM for Handover Req.
  // Fill the data for MM message
  currSpeechCh_ = vcInMsg->msgData.perfHandoverReq.channelType.speechChann;
  mmOutMsg.l3_data.handReq.channelType = vcInMsg->msgData.perfHandoverReq.channelType;
  mmOutMsg.l3_data.handReq.qid = msgQId;

  //CIPH<xxu:02-10-00> begin

  memcpy( (unsigned char*) &(mmOutMsg.l3_data.handReq.cipherCmd),
          (unsigned char*) &(vcInMsg->msgData.perfHandoverReq.cipherCmd),
          sizeof (IntraL3CipherModeCommand_t) );

  // Make sure ciphering algorithm is supported on target GP10
  if (mmOutMsg.l3_data.handReq.cipherCmd.cmd.cipherModeSetting.ciphering == 
      CNI_RIl3_CIPHER_START_CIPHERING)
    {
      switch (mmOutMsg.l3_data.handReq.cipherCmd.cmd.cipherModeSetting.algorithm)
        {
           case CNI_RIL3_CIPHER_ALGORITHM_A51:
             if (!RM_DSPA51(rm_DspCiphCap) )
               {  // Algorithm not supported, turn off ciphering

                  DBG_TRACE("handleAnchToTrgPerfHoReq TURNING CIPHERING OFF: previous algorithm %d\n",
                             mmOutMsg.l3_data.handReq.cipherCmd.cmd.cipherModeSetting.ciphering);

                  mmOutMsg.l3_data.handReq.cipherCmd.cmd.cipherModeSetting.ciphering =
                           CNI_RIl3_CIPHER_NO_CIPHERING;
               }
             break;

           case CNI_RIL3_CIPHER_ALGORITHM_A52:
             if ( !RM_DSPA52(rm_DspCiphCap) )
               {  // Algorithm not supported, turn off ciphering
                  DBG_TRACE("handleAnchToTrgPerfHoReq TURNING CIPHERING OFF: previous algorithm %d\n",
                             mmOutMsg.l3_data.handReq.cipherCmd.cmd.cipherModeSetting.ciphering);

                  mmOutMsg.l3_data.handReq.cipherCmd.cmd.cipherModeSetting.ciphering =
                           CNI_RIl3_CIPHER_NO_CIPHERING;
               }
             break;

           default:
             DBG_TRACE("handleAnchToTrgPerfHoReq TURNING CIPHERING OFF: previous algorithm %d\n",
                        mmOutMsg.l3_data.handReq.cipherCmd.cmd.cipherModeSetting.ciphering);

             mmOutMsg.l3_data.handReq.cipherCmd.cmd.cipherModeSetting.ciphering =
                      CNI_RIl3_CIPHER_NO_CIPHERING;
        }
    }

  IntraL3CipherModeCommand_t	*p;
  p = (IntraL3CipherModeCommand_t *) &(vcInMsg->msgData.perfHandoverReq.cipherCmd);


  DBG_TRACE("CC->@tgt:HoReq(t2r): ludbIndex = %d, bearer %d, spchVer %d, ciphSet %d, algo %d, kc(%x,%x)\n",
             parent->ludbIndex,
             vcOutMsg.msgData.perfHandoverReq.channelType.bearerType,
             vcOutMsg.msgData.perfHandoverReq.channelType.speechChann.version,
             p->cmd.cipherModeSetting.ciphering,
             p->cmd.cipherModeSetting.algorithm,
             p->Kc[0], p->Kc[1]);

  //CIPH<xxu:02-10-00> End

  DBG_TRACE("CC->@tgt:HoReq(t2r): callId %d, callQid(%p, %p), ccMsgQId %p\n",
             callIndex, msgQId,
             ccSession[callIndex].msgQId,
             ccMsgQId);

  // Send internal MM msg - 
  sendMM(INTRA_L3_MM_EST_REQ, INTRA_L3_RR_HANDOVER_REQ, parent->entryId, &mmOutMsg);

  hoState_ = HAND_ST_TRG_RCV_HO_REQ;

  //
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandover::handleAnchToTrgAbortHo(void)
{
  DBG_FUNC("CCTargetHandover::handleAnchToTrgAbortHo", CC_SESSION_LAYER);
  DBG_ENTER();

  // cancel T204 timer
  parent->sessionTimer->cancelTimer();

  if (voiceConnected)
    {
      // break connection
      T_CSU_PORT_ID rtpSourcePort;
      T_CSU_RESULT_CON csuResult;
	  
      rtpSourcePort.portType = CSU_RTP_PORT;
      rtpSourcePort.portId.rtpHandler = hoRtpSession_;
	  
      if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort))
          != CSU_RESULT_CON_SUCCESS)
        {
		  
          JCCLog2("CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                  csuResult,
                  (int)rtpSourcePort.portId.rtpHandler);
          DBG_ERROR("CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                    csuResult,
                    (int)rtpSourcePort.portId.rtpHandler);

          // Can only generate OA&M log. 
        }

      voiceConnected = false;

    }

  // Release the Ho RTP channel
  ReleaseHandoverRtpChannel(hoRtpSession_);
  hoRtpSession_ = NULL;

  initData();

  DBG_LEAVE();
  return (CC_RELEASE_CALL);

}


JCCEvent_t 
CCTargetHandover::handleAnchToTrgPerfHbAck(void)
{
  DBG_FUNC("CCTargetHandover::handleAnchToTrgPerfHbAck", CC_SESSION_LAYER);
  DBG_ENTER();

  // cancel Tsho timer
  parent->sessionTimer->cancelTimer();

  // Send internal RR msg
  rrOutMsg.l3_data.handCmd.handCmd =  vcInMsg->msgData.perfHandbackAck.handCmd;

#ifdef _SOFT_MOB
  if (!(parent->oid < softMobLapdmOid[0]))
    {
      printf("Soft Mob Log: Hand Cmd. sent, Send Hand Cmd. Complete now\n");
    }
  else
#endif
    {
      sendRR(INTRA_L3_DATA, INTRA_L3_RR_HANDOVER_COMMAND, parent->entryId, &rrOutMsg);
    }

  // start the T203 timer 
  parent->sessionTimer->setTimer(CALL_HAND_SRC_T203);

  hoState_ = HAND_ST_TRG_RCV_HB_REQ_ACK;

  //
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

//RETRY<xxu:04-27-00>
JCCEvent_t 
CCTargetHandover::handleAnchToTrgPerfHbNack(void)
{
  DBG_FUNC("CCTargetHandover::handleAnchToTrgPerfHbNack", CC_SESSION_LAYER);
  DBG_ENTER();

  // cancel Tsho timer
  parent->sessionTimer->cancelTimer();

  //RETRY<xxu:04-25-00> BEGIN
  //Retry next candidate if there is another left
  int i;
  IntraL3HandoverRequired_t *inMsg = &handReqd;

  if ( (i=getNextHoCandidate()) != (-1) )
  {
     //Try next candidate
     //GCELL<tyu:06-01-01> BEGIN
     candGlobalCellId_t candidate;

     hoNextCandidate_ = true;
     candIndex_ = i;

     candidate = inMsg->candGlobalCellId[i];
     //GCELL<tyu:06-01-01> END

     DBG_TRACE("Next Candidate (trg::handleTrgHoReqd): Selected %d-th of Candidates(%d) (isExt=%d, isTried=%d, mcc:%x%x%x mnc:%x%x%x lac:%d ci:%d) (hoState %d)\n",
                i, inMsg->numCandidates,
                candidate.candIsExternal, candidate.candIsTried, 
                candidate.candGlobalCellId.mcc[0],candidate.candGlobalCellId.mcc[1],candidate.candGlobalCellId.mcc[2],
                candidate.candGlobalCellId.mnc[0],candidate.candGlobalCellId.mnc[1],candidate.candGlobalCellId.mnc[2],
                candidate.candGlobalCellId.lac, candidate.candGlobalCellId.ci, hoType_, hoState_);

     // Send Perform Handback to the Anchor ViperCell.
     vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK;
     vcOutMsg.msgData.perfHandbackReq.channelType.bearerType = BEARER_SPEECH;
     vcOutMsg.msgData.perfHandbackReq.channelType.speechChann = currSpeechCh_;
     //GCELL<tyu:06-01-01> END
     vcOutMsg.msgData.perfHandbackReq.globalCellId = candidate;
  
     sendVcMsg(MODULE_CC, MODULE_CC);

     // start the Tsho timer 
     parent->sessionTimer->setTimer(CALL_HAND_SRC_TSHO);

     hoState_ = HAND_ST_TRG_SND_HB_REQ;
           
  } else
  {
     initHandbackData();

     if (hoSrcLinkLost_)
     {
         // peg
         PM_CCMeasurement.unsuccExternHDOsWithLossOfConnectionPerMSC.increment();
	  
         vcOutMsg.msgType = INTER_VC_CC_RLS_CALL;
         vcOutMsg.msgData.rlsCall.cause = JC_FAILURE_BASE;
	
         sendVcMsg(MODULE_CC, MODULE_CC);
         DBG_LEAVE();
         return (handleAnchToTrgAbortHo());
     }
  }

  //
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandover::handleAnchToTrgCompltHb(void)
{
  DBG_FUNC("CCTargetHandover::handleAnchToTrgCompltHb", CC_SESSION_LAYER);
  DBG_ENTER();

  // cancel T203 timer
  parent->sessionTimer->cancelTimer();

  // Release the Ho RTP channel
  ReleaseHandoverRtpChannel(hoRtpSession_);
  hoRtpSession_ = NULL;

  initData();

  DBG_LEAVE();
  return (CC_RELEASE_CALL);
}

JCCEvent_t 
CCTargetHandover::handleTrgTshoTimerExpiry(void)
{
  DBG_FUNC("CCTargetHandover::handleTrgTshoTimerExpiry", CC_SESSION_LAYER);
  DBG_ENTER();

  // Assume no retry is needed based on candidate list

  // Send Abort Handback to the Anchor ViperCell.

  vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDBACK;
  vcOutMsg.msgData.abortHandback.cause = JC_FAILURE_TSHO_TIMEOUT;

  sendVcMsg(MODULE_CC, MODULE_CC);

  initHandbackData();

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandover::handleTrgT203TimerExpiry(void)
{
  DBG_FUNC("CCTargetHandover::handleTrgT203TimerExpiry", CC_SESSION_LAYER);
  DBG_ENTER();

  // Assume no retry is needed based on candidate list

  // Send Abort Handback to the Anchor ViperCell.

  vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDBACK;
  vcOutMsg.msgData.abortHandback.cause = JC_FAILURE_T203_TIMEOUT;

  sendVcMsg(MODULE_CC, MODULE_CC);

  //ext-HO <xxu:07-09-01>
  //initHandbackData();
  initData();

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);

}

JCCEvent_t 
CCTargetHandover::handleTrgHoFail(void)
{
  DBG_FUNC("CCTargetHandover::handleTrgHoFail", CC_SESSION_LAYER);
  DBG_ENTER();

  // cancel T203 timer
  parent->sessionTimer->cancelTimer();

  // Assume no retry is needed based on candidate list

  // Send Abort Handback to the Anchor ViperCell.

  vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDBACK;
   
  //ext-HO <xxu:06-28-01>
  vcOutMsg.msgData.abortHandback.cause = rrInMsg->l3_data.handFail.cause;

  sendVcMsg(MODULE_CC, MODULE_CC);

  initHandbackData();

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}


//HO<xxu:02-01-00> extend to cover H3
JCCEvent_t 
CCTargetHandover::handleTrgHoReqd(void)
{
  int i;
 
  DBG_FUNC("CCTargetHandover::handleTrgHoReqd", CC_SESSION_LAYER);
  DBG_ENTER();

  IntraL3HandoverRequired_t *inMsg = &rrInMsg->l3_data.handReqd;
  //GCELL<tyu:06-01-01> END
  candGlobalCellId_t candidate;

  DBG_TRACE("CC Info: Received Handover Required (numCandidates = %d)\n", 
             inMsg->numCandidates);

  //Assume go back to anchor. If not update candidate to MSC-B' (3rd party cell)
  //GCELL<tyu:06-01-01> END
  candidate.candIsExternal   = false;
  candidate.candIsTried      = true; 
  candidate.candGlobalCellId = hoGlobalCellId_;
  
  handReqd = *inMsg;

  if (!forcedHandover)
  {
      if ((inMsg->numCandidates) <= 0)
      {
          // Ignore the message 
          DBG_ERROR ("RR->CC Error (trg::handleTrgHoReqd): Handover Required, bad (numCandidates = %d)\n", 
                     inMsg->numCandidates);
          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);
      }

      // Make sure a candidate to try is a valid one
      if ( (i=getNextHoCandidate()) == (-1) ) 
      {
          DBG_ERROR ("RR->CC (trg::handleTrgHoReqd): Handover Required Error (all bad #candidates = %d)\n",
                      inMsg->numCandidates);
          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);
      }

      candIndex_ = i;
      candidate = inMsg->candGlobalCellId[i];

      DBG_TRACE("RR->CC (trg::handleTrgHoReqd): Selected %d-th of Candidates(%d) (isExt=%d, isTried=%d, mcc:%x%x%x mnc:%x%x%x lac:%d ci:%d) (hoState %d)\n}\n",
                 i, inMsg->numCandidates,
                 candidate.candIsExternal, candidate.candIsTried,
                 candidate.candGlobalCellId.mcc[0],candidate.candGlobalCellId.mcc[1],
	             candidate.candGlobalCellId.mcc[2],
                 candidate.candGlobalCellId.mnc[0],candidate.candGlobalCellId.mnc[1],
		         candidate.candGlobalCellId.mnc[2],
                 candidate.candGlobalCellId.lac, candidate.candGlobalCellId.ci, hoState_);
  
      // Save candidate list for future retry
      handReqd = *inMsg;
      candIndex_ = i;
      //GCELL<tyu:06-01-01> END

  }

  // Send Perform Handback to the Anchor ViperCell.
  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK;
  vcOutMsg.msgData.perfHandbackReq.channelType.bearerType = BEARER_SPEECH;
  vcOutMsg.msgData.perfHandbackReq.channelType.speechChann = currSpeechCh_;
  vcOutMsg.msgData.perfHandbackReq.globalCellId = candidate;
  //GCELL<tyu:06-01-01> END
  
  sendVcMsg(MODULE_CC, MODULE_CC);

  // start the Tsho timer 
  parent->sessionTimer->setTimer(CALL_HAND_SRC_TSHO);

  hoState_ = HAND_ST_TRG_SND_HB_REQ;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

