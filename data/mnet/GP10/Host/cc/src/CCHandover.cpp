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
// File        : CCHandover.cpp
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

#include "jcc/JCCUtil.h"

#include "logging/VCLOGGING.h"

#include "defs.h"
#include "oam_api.h"
#include "pm/pm_class.h"

#include "CC/CCHandover.h"

#include "CC/CCUtil.h"

#include "CC/CCSessionHandler.h"

#include "CC/hortp.h"

// ext-HO <chenj:06-06-01>
#include "jcc/JCCEHoMsg.h"

//GCELL<tyu:06-01-01> BEGIN
extern GlobalCellId_t MY_GLOBALCELL_ID;
//GCELL<tyu:06-01-01> END

//Ciphering <chenj:07-17-01>
extern unsigned char rm_DspCiphCap;
#define RM_DSPA52(X)   (X & 2)
#define RM_DSPA51(X)   (X & 1)
#include "jcc/LUDBApi.h"

// Handover state machine
AnchorHandoverMsgHandler_t hoSrcHandler[MAX_HO_SRC_STATES][MAX_HO_SRC_EVENTS];
TargetHandoverMsgHandler_t hoTrgHandler[MAX_HO_TRG_STATES][MAX_HO_TRG_EVENTS];

//ext-HO <xxu:06-07-01>
ExtHoTargetHandoverMsgHandler_t extHoTrgHandler[MAX_HO_TRG_STATES+MAX_HO_SRC_STATES][MAX_HO_TRG_EVENTS+MAX_HO_SRC_EVENTS];
// *******************************************************************
// forward declarations.
// *******************************************************************

    CCHandover::CCHandover (CCSessionHandler *session,
                            int              callId  ,
                            MSG_Q_ID         qid     )
               : mmQueue_(this)
    {
      parent        = session;
      callIndex     = callId ;
      msgQId        = qid    ;
      hoRtpSession_ = NULL   ;
    }
    
    CCAnchorHandover::CCAnchorHandover(CCSessionHandler *session,
                                       int              callId  ,
                                       MSG_Q_ID         qid     )
                     : CCHandover (session, callId, qid)
    {
      hoSimplexConnected_ = false;
	  h3SimplexConnected_ = false;
    
      initData();
    
    }
    
    CCTargetHandover::CCTargetHandover(CCSessionHandler *session,
                                       int              callId  ,
                                       MSG_Q_ID         qid     )
                     : CCHandover (session, callId, qid)
    {
      voiceConnected      = false;
    
      initData();
    }
    
    void
    CCHandover::initData (void)
    {
      DBG_FUNC ("CCHandover::initData", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      hoState_                = HAND_ST_IDLE;
      hoVcCallIndex_          = CC_MAX_CC_CALLS_DEF_VAL;
    
//GCELL<tyu:06-01-01> BEGIN
      memset(&hoGlobalCellId_, 0, sizeof(GlobalCellId_t));
//GCELL<tyu:06-01-01> END
      
      hoVcIpAddress_          = 0;
    
      otherHoIncomingRtpPort_ = 0;
    
      if (hoRtpSession_ != NULL)
        {
          // Release the Ho RTP channel
          ReleaseHandoverRtpChannel(hoRtpSession_);
          hoRtpSession_ = NULL;
        }
    
      parent->handoverSession = NULL;
          
      hoSrcLinkLost_          = false;

      //ext-HO <xxu:05-07-01>
      hoAgId_ = -1;
      h3AgId_ = -1;
      memset(&hoNumber_, 0, sizeof(T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER));
      hoEndCause_ = MC_HO_SUCCESS;
      memset(&currEncrypt_, 0, sizeof(IntraL3CipherModeCommand_t));
      reqEncrypt_.ie_present = false;
      optCurrEncrypt_.ie_present = false;
      reqChannelType_.ie_present = false;
      optCurrChannelType_.ie_present = false;
      currClassmark_.ie_present = false;
      currChannel_.bearerType = BEARER_SPEECH;
      currChannel_.speechChann.version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
      currSpeechCh_.version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;

      DBG_LEAVE();
    }
    
    
    bool
    CCHandover::isEqualGlobalCellId (const GlobalCellId_t *gCell1,const GlobalCellId_t *gCell2)
    {
        if (gCell1->mcc[0] != gCell2->mcc[0] ||
            gCell1->mcc[1] != gCell2->mcc[1] ||
            gCell1->mcc[2] != gCell2->mcc[2] ||
            gCell1->mnc[0] != gCell2->mnc[0] ||
            gCell1->mnc[1] != gCell2->mnc[1] ||
            gCell1->mnc[2] != gCell2->mnc[2] ||
            gCell1->lac != gCell2->lac ||
            gCell1->ci != gCell2->ci)
            return false;
        else
            return true;
    }   
    
    void CCHandover::emptyMMQueue (void)
    {
    
      DBG_FUNC("CCHandover::emptyMMQueue", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
        mmQueue_.flash ();
    
      DBG_LEAVE();
    }
    
    void
    CCAnchorHandover::addToDownlinkQueue (T_CNI_LAPDM_L3MessageUnit        *msEncodedMsg)
    {
    
      DBG_FUNC("CCAnchorHandover::addToDownlinkQueue", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      if (msMsgsQueued_ < MAX_HO_SRC_DOWNLINK_QUEUE_LENGTH)
      {
        downlinkQueue_[msMsgsQueued_] = *msEncodedMsg;
        msMsgsQueued_++;
      }
    
      DBG_LEAVE();
    }
    
    
    void
    CCAnchorHandover::emptyDownlinkQueue (void)
    {
    
      DBG_FUNC("CCAnchorHandover::emptyDownlinkQueue", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      short index;
    
      // Empty second level PDU Queue first
      // Messages will be send to the target Vipercell
      parent->flashSapi3Queue();
    
      if (msMsgsQueued_ > 0)
      {
          for(index=0; index< msMsgsQueued_; index++)
          {
              parent->sendL2Msg(&downlinkQueue_[index]);
          }
    
          msMsgsQueued_ = 0;
      }
    
      DBG_LEAVE();
    }
    
    
    void
    CCAnchorHandover::addToRRQueue (IntraL3Msg_t      *rrMsg)
    {
    
      DBG_FUNC("CCAnchorHandover::addToRRQueue", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      if (rrMsgsQueued_)
      {
        // Queue is full. Do Nothing.
      }
      else
      {
        savedRRMsg_ = *rrMsg;
        rrMsgsQueued_ = true;
      }
    
      DBG_LEAVE();
    }
    
    void
    CCAnchorHandover::emptyRRQueue (void)
    {
    
      DBG_FUNC("CCAnchorHandover::emptyRRQueue", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      if (!rrMsgsQueued_)
      {
        // Queue is empty. Do Nothing.
      }
      else
      {
        parent->sendRRMsg(savedRRMsg_.primitive_type,
                          savedRRMsg_.message_type,
                          &savedRRMsg_);
    
        rrMsgsQueued_ = false;
      }
    
      DBG_LEAVE();
    }
    
    void
    CCTargetHandover::initData (void)
    {
    
      DBG_FUNC("CCTargetHandover::initData", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      if (parent->entryId != (T_CNI_IRT_ID) JCC_UNDEFINED )
      {
          //ext-HO <xxu:07-09-01>
          //if (voiceConnected)
          {
             // Break the connections with the RF Port
             T_CSU_PORT_ID rfSourcePort;
             T_CSU_RESULT_CON csuResult;
    		  
             rfSourcePort.portType = CSU_GSM_PORT;
             rfSourcePort.portId.gsmHandler = parent->entryId;
    		  
             if ((csuResult = csu_OrigTermBreakAll(&rfSourcePort))
                  != CSU_RESULT_CON_SUCCESS)
             {
                 DBG_ERROR("CSU Error : Ho Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
                            csuResult, parent->entryId);
    			  
                  // Can only generate OA&M log. 
             }
    		  
             voiceConnected = false;
    		  
          }

          sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, parent->entryId, &mmOutMsg);
      }
    
      CCHandover::initData();
      DBG_LEAVE();
    
    }
    
    void
    CCTargetHandover::initHandbackData()
    {
      DBG_FUNC("CCTargetHandover::initHandbackData", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      hoState_ = HAND_ST_TRG_HO_COMPLT;
      DBG_LEAVE();
    }
    
    void
    CCAnchorHandover::initHandbackData()
    {
      DBG_FUNC("CCAnchorHandover::initHandbackData", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      hoState_ = HAND_ST_ANCH_HO_COMPLT;
    
      if (hoSimplexConnected_)
        {
          // Break the connections with the RF Port
          T_CSU_PORT_ID rfSourcePort;
          T_CSU_RESULT_CON csuResult;
    	  
          rfSourcePort.portType = CSU_GSM_PORT;
          rfSourcePort.portId.gsmHandler = parent->entryId;
    	  
          if ((csuResult = csu_OrigTermBreakAll(&rfSourcePort))
              != CSU_RESULT_CON_SUCCESS)
            {
              JCCLog2("CSU Error : Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
                      csuResult, parent->entryId);
              DBG_ERROR("CSU Error : Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
                        csuResult, parent->entryId);
    		  
              // Can only generate OA&M log. 
            }
    
          hoSimplexConnected_ = false;
        }
    
     //BCT <xxu:09-22-00> Begin
     TwoPartyCallLeg *whichLeg;
     whichLeg = parent->activeLeg();
 
     if (whichLeg != NULL)
     {
         if ( whichLeg->msSide != NULL )
          {
               if (whichLeg->msSide->annState_ == ANN_PLAY_WAITING)
               {
                   //Play inband ringback to transferred party for BCT 
                   T_CSU_RESULT_CON csuResult;
                   whichLeg->msSide->annSrcPort_.portType = CSU_ANN_PORT;
                   whichLeg->msSide->annSrcPort_.portId.annHandler = 0;  
                   whichLeg->msSide->annSnkPort_.portType = CSU_RTP_PORT;
                   whichLeg->msSide->annSnkPort_.portId.rtpHandler = hoRtpSession_;
                   whichLeg->msSide->annSnkPort_.speechVersion = whichLeg->msSide->currSpeechCh_.version;
  
                   DBG_TRACE("CC->CSU(bct): start inband alert annState(%d) ann(%d,%x) --> snk(%d,%x,%d)\n",
                              whichLeg->msSide->annState_,
                              whichLeg->msSide->annSrcPort_.portType,
                              whichLeg->msSide->annSrcPort_.portId.annHandler,
                              whichLeg->msSide->annSnkPort_.portType,
                              whichLeg->msSide->annSnkPort_.portId.rtpHandler,
                              whichLeg->msSide->annSnkPort_.speechVersion);
  
                   if ( (csuResult = csu_SimplexConnect(&whichLeg->msSide->annSrcPort_, 
                                                        &whichLeg->msSide->annSnkPort_))
                         != CSU_RESULT_CON_SUCCESS)
                   {
                         DBG_WARNING("CC->BCT(csuSConnFailedAfterHoFailed):xlst(%d) ann(%d,%x) snk(%d,%x,%d)\n",
                                      csuResult,
                                      whichLeg->msSide->annSrcPort_.portType,
                                      (int)whichLeg->msSide->annSrcPort_.portId.rtpHandler,
                                      whichLeg->msSide->annSnkPort_.portType,
                                      (int)whichLeg->msSide->annSnkPort_.portId.rtpHandler,
                                      (int)whichLeg->msSide->annSnkPort_.speechVersion);
  
                         //ignore this remote alerting
                   }
                   whichLeg->msSide->annState_ = ANN_PLAY_ON;
               }
          }
      }
      //BCT <xxu:09-21-00> END
  
      DBG_LEAVE();
    }
    
    int
    CCHandover::handleMMMsg(IntraL3Msg_t *mmMsg,
                            JCCEvent_t   hoEvent)
    {
      DBG_FUNC("CCHandover::handleMMMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      mmInMsg = mmMsg;
      currEvent = hoEvent;
      DBG_LEAVE();
      return(0);
    
    }
    
    int
    CCAnchorHandover::handleMMMsg(IntraL3Msg_t *mmMsg,
                                  JCCEvent_t   hoEvent)
    {
      DBG_FUNC("CCAnchorHandover::handleMMMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      CCHandover::handleMMMsg(mmMsg, hoEvent);
    
      DBG_LEAVE();
      return(smHandleEvent());
    
    }
    
    int
    CCTargetHandover::handleMMMsg(IntraL3Msg_t *mmMsg,
                                  JCCEvent_t   hoEvent)
    {
      DBG_FUNC("CCTargetHandover::handleMMMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      CCHandover::handleMMMsg(mmMsg, hoEvent);
    
      DBG_LEAVE();
      return(smHandleEvent());
    
    }
    
    int
    CCHandover::handleRRMsg(IntraL3Msg_t *rrMsg,
                            JCCEvent_t   hoEvent)
    {
      DBG_FUNC("CCHandover::handleRRMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      rrInMsg = rrMsg;
      currEvent = hoEvent;
      DBG_LEAVE();
      return(0);
    
    }
    
    int
    CCAnchorHandover::handleRRMsg(IntraL3Msg_t *rrMsg,
                                  JCCEvent_t   hoEvent)
    {
      DBG_FUNC("CCAnchorHandover::handleRRMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      CCHandover::handleRRMsg(rrMsg, hoEvent);
    
      DBG_LEAVE();
      return(smHandleEvent());
    
    
    }
    
    int
    CCTargetHandover::handleRRMsg(IntraL3Msg_t *rrMsg,
                                  JCCEvent_t   hoEvent)
    {
      DBG_FUNC("CCTargetHandover::handleRRMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      CCHandover::handleRRMsg(rrMsg, hoEvent);
    
      DBG_LEAVE();
      return(smHandleEvent());
    
    
    }
    
    JCCEvent_t 
    CCHandover::handleMMLateEstCnf(void)
    {
    
      DBG_FUNC("handleMMLateEstCnf", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // handle the case where a late establish confirm comes in 
      // for a handover request that was previously sent.
      sendMM   (INTRA_L3_MM_REL_REQ,
                INTRA_L3_NULL_MSG,
                mmInMsg->entry_id,
                &mmOutMsg);
    
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    JCCEvent_t
    CCHandover::handleInvalidEvent(void)
    {
    
      DBG_FUNC("CCHandover::handleInvalidEvent", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // let the developer/tester/fields person decide.
      // The user of the system needs to check this out if there was other problem.
      // Otherwise, this is just a race condition.
      JCCLog2("CC Handover Warning: (Invalid event = %d)  in (State = %d) \n",  
              currEvent, 
              hoState_);
      DBG_WARNING("CC Handover Warning: (Invalid event = %d)  in (State = %d) \n",  
                  currEvent, 
                  hoState_);
      DBG_LEAVE();
      return (JCC_NULL_EVENT);
      
    }
    
    JCCEvent_t
    CCHandover::handleInternalEvent (JCCEvent_t handEvent)
    {
      DBG_FUNC("CCHandover::handleInternalEvent", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      DBG_LEAVE();
      return(CC_RELEASE_CALL);
    
    }
    
    //HO<xxu:03-07-00>
    JCCEvent_t
    CCAnchorHandover::handleInternalEvent (JCCEvent_t handEvent)
    {
      
      DBG_FUNC("CCAnchorHandover::handleInternalEvent", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      JCCEvent_t returnValue = JCC_NULL_EVENT;
    
      DBG_TRACE("CC INFO @anch-handleInternalEvent: ho(%d, %d), event %d\n",
                    hoType_, hoState_, handEvent);
    
      // check for handover
      switch (hoState_) 
        {
        case HAND_ST_IDLE:
          // No Handover activity
          returnValue = CC_RELEASE_CALL;
          break;
    
        case HAND_ST_ANCH_VC_IP_ADDRESS_REQ:
        case HAND_ST_ANCH_SND_HO_REQ:
          // Skip event loop. cleanup handover and continue to cleanup session.
          handleAnchDuringHoRls();
          returnValue = CC_RELEASE_CALL;
          break;
    
        case HAND_ST_ANCH_RCV_HO_REQ_ACK:
          {
            // There is a risk that we never receive any message
            // from Target ViperCell. Also, we haven't released
            // the old channel and connection resources and setup the 
            // two-way connection with the new Ho RTP channel.
    		
            // Do not cancel T103 timer - timer value should be set accordingly!
            // This will let us handle the case described above.
            // hcTimer->cancelTimer();
    		
            hoSrcLinkLost_ = true;
    		
            // Wait for a Handover Complete from Target
            // Do Nothing else
            returnValue = CC_MESSAGE_PROCESSING_COMPLT;
            break;
          }
    
        case HAND_ST_ANCH_HO_COMPLT:
        case HAND_ST_ANCH_RCV_HB_REQ:
          // Should not receive this msg. in these states.
          // Late Message. Ignore!
          returnValue = CC_MESSAGE_PROCESSING_COMPLT;
          break;
              
        case HAND_ST_ANCH_SND_HB_REQ_ACK:
          // cancel T104 timer
          parent->sessionTimer->cancelTimer();
          // There is a chance for recovery.
          handleAnchT104TimerExpiry();
          returnValue = CC_MESSAGE_PROCESSING_COMPLT;
          break;
    
        default:
          DBG_ERROR("CC Handover Error: (Invalid Anchor State = %d)(hoType %d) \n",  
                    hoState_, hoType_);
        }
              
    	  
      DBG_LEAVE();
      return(returnValue);
    }
    
    JCCEvent_t
    CCTargetHandover::handleInternalEvent (JCCEvent_t handEvent)
    {
      DBG_FUNC("CCTargetHandover::handleInternalEvent", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      JCCEvent_t returnValue = JCC_NULL_EVENT;
    
      // check for handover
      switch (hoState_) 
        {
    
        case HAND_ST_TRG_RCV_HO_REQ:
        case HAND_ST_TRG_SND_HO_REQ_ACK:
        case HAND_ST_TRG_HO_COMPLT:
        case HAND_ST_TRG_SND_HB_REQ:
          handleTrgPostHoRls();
          returnValue = CC_RELEASE_CALL;
          break;
    
        case HAND_ST_TRG_RCV_HB_REQ_ACK:
          {
            // There is a risk that we never receive any message
            // from Target(Anchor) ViperCell. Also, we haven't released
            // the old channel and connection resources and setup the 
            // new connection with the new Ho RTP channel.
    		
            // Do not cancel T203 timer - timer value should be set accordingly!
            // This will let us handle the case described above.
            // hcTimer->cancelTimer();
    					  
            hoSrcLinkLost_ = true;
    		
            // Wait for a Handover Complete from Target
            // Do Nothing else
            returnValue = CC_MESSAGE_PROCESSING_COMPLT;
            break;
          }
    
        default:
          DBG_ERROR("CC Handover Error: (Invalid Target State = %d) \n",  
                    hoState_);
        }
              
      DBG_LEAVE();
      return(returnValue);
    
    }
    
    int
    CCHandover::handleTimeoutMsg (IntraL3Msg_t *ccInMsg)
    {
      DBG_FUNC("CCHandover::handleTimeoutMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      DBG_LEAVE();
      return(0);
    
    }
    
    int
    CCAnchorHandover::handleTimeoutMsg (IntraL3Msg_t *ccInMsg)
    {
      DBG_FUNC("CCAnchorHandover::handleTimeoutMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      currEvent = HAND_SRC_TIMER_EXPIRY;
                      
      DBG_LEAVE();
      return(smHandleEvent());
    
    }
    
    int
    CCTargetHandover::handleTimeoutMsg (IntraL3Msg_t *ccInMsg)
    {
      DBG_FUNC("CCTargetHandover::handleTimeoutMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      currEvent = HAND_TRG_TIMER_EXPIRY;
                      
      DBG_LEAVE();
      return(smHandleEvent());
    
    }
    
    
    int
    CCHandover::handleVcCcMsg(InterVcMsg_t    *vcCcInMsg)
    {
    
      DBG_FUNC("CCHandover::handleVcCcMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      vcInMsg = vcCcInMsg;
    
      switch (vcInMsg->msgType)
        {
        case INTER_VC_CC_PERFORM_HANDOVER:
          currEvent = HAND_PERFORM_HO_REQ;
          // peg
          PM_CCMeasurement.attIncomingInterMSCHDOs.increment();
          break;
          
        case INTER_VC_CC_ABORT_HANDOVER:
          currEvent = HAND_ABORT_HO;
          break;
          
        case INTER_VC_CC_PERFORM_HANDOVER_ACK:
          currEvent = HAND_PERFORM_HO_ACK;
          break;
          
        case INTER_VC_CC_PERFORM_HANDOVER_NACK:
          currEvent = HAND_PERFORM_HO_NACK;
          break;
          
        case INTER_VC_CC_COMPLETE_HANDOVER:
          currEvent = HAND_COMPLT_HO;
          // peg
          PM_CCMeasurement.succOutgoingInterMSCHDOs.increment();
          break;
          
        case INTER_VC_CC_RLS_CALL:
          currEvent = HAND_RLS_CALL;
          break;
          
        case INTER_VC_CC_POST_HANDOVER_MOB_EVENT:
          currEvent = HAND_POSTHO_MOB_EVENT;
          break;
          
          // Handback Events
    
        case INTER_VC_CC_PERFORM_HANDBACK:
          currEvent = HAND_PERFORM_HO_REQ;
          // peg
          PM_CCMeasurement.attIncomingInterMSCHDOs.increment();
          break;
          
        case INTER_VC_CC_ABORT_HANDBACK:
          currEvent = HAND_ABORT_HO;
          break;
          
        case INTER_VC_CC_PERFORM_HANDBACK_ACK:
          currEvent = HAND_PERFORM_HO_ACK;
          break;
          
        case INTER_VC_CC_PERFORM_HANDBACK_NACK:
          currEvent = HAND_PERFORM_HO_NACK;
          break;
          
        case INTER_VC_CC_COMPLETE_HANDBACK:
          currEvent = HAND_COMPLT_HO;
          // peg
          PM_CCMeasurement.succOutgoingInterMSCHDOs.increment();
          break;
    
        default:
          DBG_ERROR("CC Call Error: Unexpected (Msg Type = %d) from Vipercell.\n",  
                    vcInMsg->msgType);
          currEvent = JCC_NULL_EVENT;
          
        }
            
      DBG_LEAVE();
      return(0);
    }
    
    int
    CCAnchorHandover::handleVcCcMsg(InterVcMsg_t    *vcCcInMsg)
    {
      DBG_FUNC("CCAnchorHandover::handleVcCcMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      CCHandover::handleVcCcMsg(vcCcInMsg);
    
      DBG_LEAVE();
      return(smHandleEvent());
    }
    
    int
    CCTargetHandover::handleVcCcMsg(InterVcMsg_t    *vcCcInMsg)
    {
      DBG_FUNC("CCTargetHandover::handleVcCcMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      CCHandover::handleVcCcMsg(vcCcInMsg);
    
      DBG_LEAVE();
      return(smHandleEvent());
    }
    
    void
    CCHandover::printData (JCCPrintStFn fnPtr)
    {
      char * hcStr1Ptr = "Call Handover Data : ";
      char hcStr2[120];
    
      sprintf(&hcStr2[0],
              "state( %d), hoCallIndex( %d), VcMCC(%x%x%x), VcMNC(%x%x%x), VcLac(%d), VcCellId( %d), VcIpAddress( %d)\n",
              hoState_,
              hoVcCallIndex_,
              hoGlobalCellId_.mcc[0],hoGlobalCellId_.mcc[1],hoGlobalCellId_.mcc[2],
              hoGlobalCellId_.mnc[0],hoGlobalCellId_.mnc[1],hoGlobalCellId_.mnc[2],
              hoGlobalCellId_.lac,
              hoGlobalCellId_.ci,
              hoVcIpAddress_
              );
    
      //if (hoState_ != HAND_ST_IDLE)
      //  {
          (*fnPtr)(hcStr1Ptr);
          (*fnPtr)(&hcStr2[0]);
      //  }
    }
    
    void
    CCTargetHandover::printData (JCCPrintStFn fnPtr)
    {
      CCHandover::printData(fnPtr);
    
      char * hcStr1Ptr = "Call Target Handover Data : ";
      char hcStr2[120];
    
      sprintf(&hcStr2[0],
              "voiceConnected( %d)\n",
              voiceConnected
              );
    
      if (hoState_ != HAND_ST_IDLE)
        {
          (*fnPtr)(hcStr1Ptr);
          (*fnPtr)(&hcStr2[0]);
        }
    }

    //HO<xxu:02-23-00> Handle H3
    BOOL
    CCAnchorHandover::cleanup(BOOL force = FALSE)
    {
      BOOL retVal;

      DBG_FUNC("CCAnchorHandover::cleanup", CC_HANDOVER_LAYER);
      DBG_ENTER();

      if (force == FALSE)
      {
          // FALSE returned on canceling handover in process (incompleted)
          // The return value is used in the CCSessionHandler::cleanup()
          // as a condition for cleaning the call upon remote VC failure
          retVal = (hoState_ < HAND_ST_ANCH_HO_COMPLT) ? FALSE : TRUE;

          // Cleanup source
          switch (hoType_)
          {
          case HO_TY_HANDMSC3:
               //Keep ho but drop h3
               handleAnchPostH3Rls();
               hoState_ = HAND_ST_ANCH_HO_COMPLT;
               hoType_  = HO_TY_HANDOVER;
               retVal = FALSE;
               break;

          case HO_TY_HANDOVER:
          case HO_TY_HANDBACK:
               //Drop ho
               handleAnchPostHoRls();
               break;

          case HO_TY_EXT_HANDOVER:
          case HO_TY_EXT_HANDBACK:
          case HO_TY_EXT_HANDMSC3:
          case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
          case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
               // <ext-ho <chenj: 06-05-01>
               handleAnchPostExtHoRls();
               break;

          default:
               // own messup happend, trouble shooting immediately!
               DBG_ERROR ("CC Error @anch-cleanup: messed up (hoType: %d) (hoState: %d)\n", 
                          hoType_, hoState_);
               break;
          }
      }
      else
      {
          // <Igal: 04-10-01> 
          // Unconditional complete cleanup
          retVal = TRUE;

          // <ext-ho <chenj: 06-05-01>
          handleAnchPostExtHoRls();

          handleAnchPostHoRls();
          handleAnchPostH3Rls();

          initData();
      }

      DBG_LEAVE();
      return (retVal);
    }

    BOOL
    CCTargetHandover::cleanup(BOOL force = FALSE)
    {
    
      DBG_FUNC("CCTargetHandover::cleanup", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // Cleanup target
      handleTrgPostHoRls();
    
      DBG_LEAVE();
    
      // Always TRUE. Target session is always cleaned when a connection
      // with the target VC and the anchor session is lost.
      // The return value is used in the CCSessionHandler::cleanup()
      return (TRUE);
    }
    
    int
    CCHandover::smHandleEvent(void)
    {
    
      DBG_FUNC("CCHandover::smHandleEvent", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      DBG_LEAVE();
      return(0);
    }
    
    int
    CCAnchorHandover::smHandleEvent(void)
    {
      DBG_FUNC("CCAnchorHandover::smHandleEvent", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      int returnValue = 0;
    
      if (currEvent != JCC_NULL_EVENT)
        {
          currEvent = 
            (this->*hoSrcHandler[hoState_][currEvent - HAND_SRC_EVENT_BASE])();
        }
      else
        {
          DBG_LEAVE();
          return(0);
        }
    
      switch(currEvent)
        {
        case CC_RELEASE_CALL:
          returnValue = -1;  // release call

          break;

        case CC_RELEASE_HOCALLLEG:
          // ext-HO <chenj:06-11-01>
          // JOE: later, might want to pass a correct cause
          parent->hoCallLeg->cleanup( CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING );

          returnValue = 0;  // do NOT release call (just the external call leg)!

          break;

        case CC_RELEASE_H3CALLLEG:
          // ext-HO <chenj:06-11-01>
          // JOE: later, might want to pass a correct cause
          parent->h3CallLeg->cleanup( CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING );

          returnValue = 0;  // do NOT release call (just the external call leg)!

          break;

        case CC_RELEASE_ALLHOCALLLEG:
          // EHO <chenj:06-11-01>
          // JOE: later, might want to pass a correct cause
          parent->hoCallLeg->cleanup( CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING );
          parent->h3CallLeg->cleanup( CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING );

          returnValue = 0;  // do NOT release call (just the external call leg)!

          break;

        case CC_RELEASE_OWN:
          //Return to proceed with call legs and call releases
          returnValue = CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING;
          break;
    
        case CC_TWO_STAGE_DISCONNECT:
          returnValue = -1;
          break;
    
        case CC_MESSAGE_PROCESSING_COMPLT:
        case JCC_NULL_EVENT:
          // Do Nothing
          break;
    
        default:
    
          DBG_ERROR("CC Handover Internal Error: (Bad event=%d), (state=%d), (session=%d)\n",
                    currEvent,
                    hoState_,
                    callIndex);
        }
          
      DBG_LEAVE();
      return(returnValue);
    }
    
    int
    CCTargetHandover::smHandleEvent(void)
    {
      DBG_FUNC("CCTargetHandover::smHandleEvent", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      int returnValue = 0;
    
      if (currEvent != JCC_NULL_EVENT)
        {
          currEvent = 
            (this->*hoTrgHandler[hoState_][currEvent - HAND_SRC_EVENT_BASE])();
        }
      else
        {
          DBG_LEAVE();
          return(0);
        }
    
    
      switch(currEvent)
        {
        case CC_RELEASE_CALL:
          returnValue = -1;
          break;
    
        case CC_MESSAGE_PROCESSING_COMPLT:
        case JCC_NULL_EVENT:
          // Do Nothing
          break;
    
        default:
    
          DBG_ERROR("CC Handover Internal Error: (Bad event=%d), (state=%d), (session=%d)\n",
                    currEvent,
                    hoState_,
                    callIndex);
        }
          
      DBG_LEAVE();
      return(returnValue);
    }
    
    void
    CCTargetHandover::sendRRMsgToAnchorCC(IntraL3Msg_t      *rrMsg)
    {
    
      DBG_FUNC("CCTargetHandover::sendRRMsgToAnchorCC", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // Do Nothing
      vcOutMsg.msgType = INTER_VC_CC_POST_HANDOVER_MOB_EVENT;
      vcOutMsg.msgData.postHoMobEventData.intraL3Msg = *rrMsg;
      sendVcMsg(MODULE_RM, MODULE_CC);
      DBG_LEAVE();
    }
    
    void
    CCTargetHandover::sendMSMsgToAnchorCC(T_CNI_RIL3MD_CCMM_MSG      *msInMsg)
    {
      DBG_FUNC("CCTargetHandover::sendMSMsgToAnchorCC", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      vcOutMsg.msgType = INTER_VC_CC_POST_HANDOVER_MOB_EVENT;
      vcOutMsg.msgData.postHoMobEventData.msInMsg = *msInMsg;
      sendVcMsg(MODULE_MD, MODULE_CC);
      DBG_LEAVE();
    }
    
    
    void CCTargetHandover::sendMMMsgToRemoteCC (IntraL3Msg_t *mmMsg)
    { 
      /* Forward CM SERVICE REQUEST to the anchor CC (CM Session) */
    
      DBG_FUNC("CCTargetHandover::sendMMMsgToCC", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
        vcOutMsg.msgType = INTER_VC_CC_POST_HANDOVER_MOB_EVENT ;
        vcOutMsg.msgData.postHoMobEventData.intraL3Msg = *mmMsg;
    
        // Source, Destination
        sendVcMsg (MODULE_MM, MODULE_CC);
    
      DBG_LEAVE();
    }
    
    
    void
    CCAnchorHandover::sendL2MsgToTargetCC(T_CNI_LAPDM_L3MessageUnit *msEncodedMsg)
    {
      DBG_FUNC("CCAnchorHandover::sendL2MsgToTargetCC", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      vcOutMsg.msgType = INTER_VC_CC_POST_HANDOVER_DATA_EVENT;
      vcOutMsg.msgData.postHoMobEventData.msEncodedMsg = *msEncodedMsg;
    
      // Igal: PDU has to be intercepted by the CM Session on the target side
      sendVcMsg(MODULE_CC, MODULE_CC); 
      DBG_LEAVE();
    }
    
    
    void
    CCAnchorHandover::sendRRMsgToTargetCC(IntraL3Msg_t      *rrMsg)
    {
      DBG_FUNC("CCAnchorHandover::sendRRMsgToTargetCC", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      vcOutMsg.msgType = INTER_VC_CC_POST_HANDOVER_DATA_EVENT; //INTER_VC_CC_POST_HANDOVER_MOB_EVENT;
      vcOutMsg.msgData.postHoMobEventData.intraL3Msg = *rrMsg;
      sendVcMsg(MODULE_CC, MODULE_RM);
      DBG_LEAVE();
    }
    
    
    void CCAnchorHandover::sendMMMsgToRemoteCC (IntraL3Msg_t *mmMsg)
    { 
      /* Send Response to CM SERVICE REQUEST (either CM SERVICE ACCEPT or REJECT) 
         to the target MM */
    
      DBG_FUNC("CCAnchorHandover::sendMMMsgToCC", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
        vcOutMsg.msgType = INTER_VC_CC_POST_HANDOVER_MOB_EVENT ;
        vcOutMsg.msgData.postHoMobEventData.intraL3Msg = *mmMsg;
    
        // Source, Destination
        sendVcMsg (MODULE_CC, MODULE_MM);
    
      DBG_LEAVE();
    }
    
    
    T_CSU_PORT_ID
    CCAnchorHandover::mobileCsuPort(void)
    {
      DBG_FUNC("CCAnchorHandover::mobileCsuPort", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      T_CSU_PORT_ID rtpSourcePort;

      rtpSourcePort.portType = CSU_RTP_PORT;

      if (hoState_ == HAND_ST_ANCH_EHO_COMPLT) // External handover
        {
          rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(parent->hoCallLeg->h323CallHandle);
        }
      else
        {
          rtpSourcePort.portId.rtpHandler = hoRtpSession_;
        }

      return(rtpSourcePort);
    
      DBG_LEAVE();
    }
    
    
    //HO<xxu:01-31-00> give type of going handover
    JCCEvent_t 
    CCHandover::handleNullEvent(void)
    {
    
      DBG_FUNC("handleNullEvent", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      DBG_TRACE("CC Info: hoType '%d', hoState '%d'\n", hoType_, hoState_);
      // Do Nothing.
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    
    //HO<xxu:01-31-00> init h3
    void
    CCAnchorHandover::initHandmsc3Data(void)
    {
      DBG_FUNC("CCAnchorHandover::initHandmsc3Data", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      if (h3SimplexConnected_)
      {
          // Break connections with the RF Port
          T_CSU_PORT_ID dstRtpPort;
          T_CSU_RESULT_CON csuResult;
    	  
          dstRtpPort.portType = CSU_RTP_PORT;
          dstRtpPort.portId.rtpHandler = h3RtpSession_;
    	  
          if ((csuResult = csu_OrigTermBreakAll(&dstRtpPort))
              != CSU_RESULT_CON_SUCCESS)
          {
              JCCLog3("CSU Error : Disconn. Failure, (hoType|hoState: %x) (Result = %d)(rtpHndler %x) \n",
                       (hoType_<<8|hoState_), csuResult, (int) h3RtpSession_);
              DBG_ERROR("CSU Error : Disconn. Failure, (hoType %d;hoState_ %d)(Result = %d)(rtpHndler %x) \n",  
                       hoType_, hoState_, csuResult, h3RtpSession_);
    		  
              // Can only generate OA&M log. 
          }
      }
    
      if (h3RtpSession_ != NULL)
      {
          // Release the H3 RTP channel
          ReleaseHandoverRtpChannel(h3RtpSession_);
      }
    
      msMsgsQueued_           = 0;
      rrMsgsQueued_           = false;
    
      h3SimplexConnected_     = false;
      h3VcCallIndex_          = CC_MAX_CC_CALLS_DEF_VAL;
//GCELL<tyu:06-01-01> BEGIN
      memset(&h3GlobalCellId_, 0, sizeof(GlobalCellId_t));
//GCELL<tyu:06-01-01> END
      h3VcIpAddress_          = 0;
      otherH3IncomingRtpPort_ = 0;
      h3RtpSession_           = NULL;
    
      h3SrcLinkLost_          = false;
      hoType_                 = HO_TY_HANDOVER;
      hoState_                = HAND_ST_ANCH_HO_COMPLT;
    
      //BCT <xxu:09-22-00> Begin
      TwoPartyCallLeg *whichLeg;
      whichLeg = parent->activeLeg();
  
      if (whichLeg != NULL)
      {
          if ( whichLeg->msSide != NULL )
          {
               if (whichLeg->msSide->annState_ == ANN_PLAY_WAITING)
               {
                   //Play inband ringback to transferred party for BCT 
                   T_CSU_RESULT_CON csuResult;
                   whichLeg->msSide->annSrcPort_.portType = CSU_ANN_PORT;
                   whichLeg->msSide->annSrcPort_.portId.annHandler = 0;  
                   whichLeg->msSide->annSnkPort_.portType = CSU_RTP_PORT;
                   whichLeg->msSide->annSnkPort_.portId.rtpHandler = hoRtpSession_;
                   whichLeg->msSide->annSnkPort_.speechVersion = whichLeg->msSide->currSpeechCh_.version;
  
  
                   DBG_TRACE("CC->CSU(bct): start inband alert annState(%d) ann(%d,%x) --> snk(%d,%x,%d)\n",
                              whichLeg->msSide->annState_,
                              whichLeg->msSide->annSrcPort_.portType,
                              whichLeg->msSide->annSrcPort_.portId.annHandler,
                              whichLeg->msSide->annSnkPort_.portType,
                              whichLeg->msSide->annSnkPort_.portId.rtpHandler,
                              whichLeg->msSide->annSnkPort_.speechVersion);
  
                   if ( (csuResult = csu_SimplexConnect( &whichLeg->msSide->annSrcPort_,
                                                         &whichLeg->msSide->annSnkPort_))
                         != CSU_RESULT_CON_SUCCESS)
                   {
                         DBG_WARNING("CC->BCT(csuSConnFailedAfterHoFailed):xlst(%d) ann(%d,%x) snk(%d,%x,%d)\n",
                                      csuResult,
                                      whichLeg->msSide->annSrcPort_.portType,
                                      (int)whichLeg->msSide->annSrcPort_.portId.rtpHandler,
                                      whichLeg->msSide->annSnkPort_.portType,
                                      (int)whichLeg->msSide->annSnkPort_.portId.rtpHandler,
                                      (int)whichLeg->msSide->annSnkPort_.speechVersion);
  
                         //ignore this remote alerting
                   }
                   whichLeg->msSide->annState_ = ANN_PLAY_ON;
               }
          }
      }
      //BCT <xxu:09-21-00> END
  
  
      //RETRY<xxu:04-24-00> BEGIN
      hoRetry_ = 0;
      //RETRY<xxu:04-24-00> END
    
      DBG_LEAVE();
    }
    
    
    //HO<xxu:01-31-00> init ho
    void
    CCAnchorHandover::initData (void)
    {
    
      DBG_FUNC("CCAnchorHandover::initData", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      msMsgsQueued_ = 0;
    
      rrMsgsQueued_ = false;
    
      if (hoSimplexConnected_)
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
                      (int)hoRtpSession_);
              DBG_ERROR("CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                        csuResult,
                        (int)hoRtpSession_);
    		  
              // Can only generate OA&M log. 
            }
    
          hoSimplexConnected_ = false;
        }
  
	  
	  if (h3SimplexConnected_)
      {
          // Break connections with the RF Port
          T_CSU_PORT_ID dstRtpPort;
          T_CSU_RESULT_CON csuResult;
    	  
          dstRtpPort.portType = CSU_RTP_PORT;
          dstRtpPort.portId.rtpHandler = h3RtpSession_;
    	  
          if ((csuResult = csu_OrigTermBreakAll(&dstRtpPort))
              != CSU_RESULT_CON_SUCCESS)
          {
              JCCLog3("CSU Error : Disconn. Failure, (hoType|hoState: %x) (Result = %d)(rtpHndler %x) \n",
                       (hoType_<<8|hoState_), csuResult, (int) h3RtpSession_);
              DBG_ERROR("CSU Error : Disconn. Failure, (hoType %d;hoState_ %d)(Result = %d)(rtpHndler %x) \n",  
                       hoType_, hoState_, csuResult, h3RtpSession_);
    		  
              // Can only generate OA&M log. 
          }
      }
    
      if (h3RtpSession_ != NULL)
      {
          // Release the H3 RTP channel
          ReleaseHandoverRtpChannel(h3RtpSession_);
      }
    
      msMsgsQueued_           = 0;
      rrMsgsQueued_           = false;
    
      h3SimplexConnected_     = false;
      h3VcCallIndex_          = CC_MAX_CC_CALLS_DEF_VAL;
//GCELL<tyu:06-01-01> BEGIN
      memset(&h3GlobalCellId_, 0, sizeof(GlobalCellId_t));
//GCELL<tyu:06-01-01> END
      h3VcIpAddress_          = 0;
      otherH3IncomingRtpPort_ = 0;
      h3RtpSession_           = NULL;
    
      h3SrcLinkLost_          = false;

      // ext-HO <chenj:06-29-01>
      memset(&targetMscNumber_, 0, sizeof(T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER));
  
      //BCT <xxu:09-22-00> Begin
      TwoPartyCallLeg *whichLeg;
      whichLeg = parent->activeLeg();
  
      if (whichLeg != NULL)
      {
          if ( whichLeg->msSide != NULL )
          {
               if (whichLeg->msSide->annState_ == ANN_PLAY_WAITING)
               {
                   //Play inband ringback to transferred party for BCT 
                   T_CSU_RESULT_CON csuResult;
                   whichLeg->msSide->annSrcPort_.portType = CSU_ANN_PORT;
                   whichLeg->msSide->annSrcPort_.portId.annHandler = 0;  
                   whichLeg->msSide->annSnkPort_.portType = CSU_GSM_PORT;
                   whichLeg->msSide->annSnkPort_.portId.gsmHandler = parent->entryId;
                   whichLeg->msSide->annSnkPort_.speechVersion = whichLeg->msSide->currSpeechCh_.version;
  
  
                   DBG_TRACE("CC->CSU(bct): start inband alert annState(%d) ann(%d,%x) --> snk(%d,%x,%d)\n",
                              whichLeg->msSide->annState_,
                              whichLeg->msSide->annSrcPort_.portType,
                              whichLeg->msSide->annSrcPort_.portId.annHandler,
                              whichLeg->msSide->annSnkPort_.portType,
                              whichLeg->msSide->annSnkPort_.portId.rtpHandler,
                              whichLeg->msSide->annSnkPort_.speechVersion);
  
                   if ( (csuResult = csu_SimplexConnect(&whichLeg->msSide->annSrcPort_, 
                                                      &whichLeg->msSide->annSnkPort_))
                         != CSU_RESULT_CON_SUCCESS)
                   {
                         DBG_WARNING("CC->BCT(csuSConnFailedAfterHoFailed):xlst(%d) ann(%d,%x) snk(%d,%x,%d)\n",
                                      csuResult,
                                      whichLeg->msSide->annSrcPort_.portType,
                                      (int)whichLeg->msSide->annSrcPort_.portId.rtpHandler,
                                      whichLeg->msSide->annSnkPort_.portType,
                                      (int)whichLeg->msSide->annSnkPort_.portId.rtpHandler,
                                      (int)whichLeg->msSide->annSnkPort_.speechVersion);
  
                         //ignore this remote alerting
                   }
                   whichLeg->msSide->annState_ = ANN_PLAY_ON;
               }
          }
      }
      //BCT <xxu:09-21-00> END
  
      CCHandover::initData();
    
      //HO<xxu: 01-27-00>
      hoType_ = (HandoverTypes_t) 0;
    
      //RETRY<xxu:04-24-00> BEGIN
      hoRetry_ = 0;
      //RETRY<xxu:04-24-00> END
    
      DBG_LEAVE();
    }
    
    //HO<xxu:01-28-00> need handle both h3 and ho 
    int
    CCAnchorHandover::handleVbCcMsg(IntraL3Msg_t      *vbCcInMsg)
    {
      UINT32 reVcIpAddress;
      bool location_rejected = false;

      DBG_FUNC("CCAnchorHandover::handleVbCcMsg", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      DBG_TRACE("CC->Handover Info: CCAnchorHandover::handleVbCcMsg (hoType %d, hoState %d)\n",
                 hoType_, hoState_);
    
      if (hoState_ != HAND_ST_ANCH_VC_IP_ADDRESS_REQ    &&
          hoState_ != HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ  )
      {
          // Do Nothing
          // Late Message
          DBG_LEAVE();
          return 0;
      }
 
      // PR CSCdv18197 <chenj:08-27-01>
      // A string "0" instead of NULL for rejected case
      if ((vbCcInMsg->l3_data.ipAddressResponse.ViperCellIpAddress[0] == '0') &&
          (vbCcInMsg->l3_data.ipAddressResponse.ViperCellIpAddress[0] == '\n'))
        {
          location_rejected = true;
        }
      else
        {
          // Copy the ip address from the message 
          reVcIpAddress =  
              inet_addr(&(vbCcInMsg->l3_data.ipAddressResponse.ViperCellIpAddress[0]) );

          if (reVcIpAddress == 0)
            {
               location_rejected = true;
            }

          switch(hoType_)
            {
              case HO_TY_HANDOVER:
              hoVcIpAddress_ = reVcIpAddress;

              break;

              case HO_TY_HANDMSC3:
              h3VcIpAddress_ = reVcIpAddress;

              break;

              default:
                //Deadly error, can't go further. Troubleshooting needed !
                JCCLog2 ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                          hoType_, hoState_);
                DBG_ERROR ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                            hoType_, hoState_);
                location_rejected = true;
                return (0);
            }
         }

      if (location_rejected)
      {
          // Do Nothing
          JCCLog2 ("VOIP->CC Error: Ip Address failed (hoType %d, hoState %d)\n",
                   hoType_, hoState_);
          JCCLog("Check if an adjacent vipercell is down, but radio is powered on.\n");
          DBG_ERROR ("VOIP->CC Error: Ip Address failed (hoType %d, hoState %d)\n",
                      hoType_, hoState_);
          DBG_ERROR("Check if an adjacent vipercell is down, but radio is powered on.\n");
          
          // Give up this ho or h3 effort wait for next Handover Required message.
          if (hoType_==HO_TY_HANDOVER)
              initData();
          else
              initHandmsc3Data();
      }else
      {
          JCCLog3 ("VOIP->CC Log: (hoType %d, hoState %d) (Ip Address = %d) returned.\n",
                   hoType_, hoState_,
                   reVcIpAddress);
          DBG_TRACE ("VOIP->CC Log: (hoType %d, hoState %d) (Ip Address = %d) returned.\n",
                   hoType_, hoState_,
                   reVcIpAddress);
          
          // VC IP Address Request was sent out.
          currEvent = HC_VB_GET_VC_ADDRESS_RSP;
    
          DBG_LEAVE();
          return (smHandleEvent());
      }
    
      DBG_LEAVE();
      return(0);
    }
    
    //RETRY<xxu:04-25-00> BEGIN
    int
    CCHandover::getNextHoCandidate(void)
    {
      int i;
      IntraL3HandoverRequired_t *inMsg = &handReqd;
    
      DBG_FUNC("CCHandover::getNextHoCandidate", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // Assume all the candidates are already sorted in terms of perference
      // Make sure a candidate to try is a valid one
      for (i=0;i<inMsg->numCandidates;i++)
      {
           // xxuTbdHo: multiple VBs ie networkId

            DBG_TRACE("TRACE-getNextHoCandidate: Cand MCC %x%x%x MNC %x%x%x Lac %d Ci %d IsExtern %d IsTried %d\n",
                  inMsg->candGlobalCellId[i].candGlobalCellId.mcc[0], inMsg->candGlobalCellId[i].candGlobalCellId.mcc[1], inMsg->candGlobalCellId[i].candGlobalCellId.mcc[2], 
                  inMsg->candGlobalCellId[i].candGlobalCellId.mnc[0], inMsg->candGlobalCellId[i].candGlobalCellId.mnc[1], inMsg->candGlobalCellId[i].candGlobalCellId.mnc[2], 
                  inMsg->candGlobalCellId[i].candGlobalCellId.lac, inMsg->candGlobalCellId[i].candGlobalCellId.ci, inMsg->candGlobalCellId[i].candIsExternal,
                  inMsg->candGlobalCellId[i].candIsTried);

           if ((isEqualGlobalCellId(&(inMsg->candGlobalCellId[i].candGlobalCellId), &MY_GLOBALCELL_ID) == false) &&
               (inMsg->candGlobalCellId[i].candIsTried == FALSE))
           {
                inMsg->candGlobalCellId[i].candIsTried = true;
                // Found an untried candidate that is not myself, so break out of for loop.
                break;
           }
           else
           {
               DBG_WARNING("RR->CC Alert: %th of (numCandidates %d) in handReqd msg is self\n",
                            i, inMsg->numCandidates);
           }
      }
    	 
      if (i>=inMsg->numCandidates) 
      {
          DBG_ERROR ("RR->CC Error: look for next candidate, all of (numCandidates = %d) are bad \n", 
                      inMsg->numCandidates);
          DBG_LEAVE();
          return (-1);
      } 
    
      DBG_LEAVE();
      return (i);
    }
    //RETRY<xxu:04-25-00> END


//ext-HO <xxu:05-22-01>
int
CCHandover::handleHoaCcMsg(InterHoaVcMsg_t *hoaCcInMsg)
{
	DBG_TRACEho("{\nMNEThoTRACE(ho::handleHoaCcMsg): entering......\n}\n");

    hoaInMsg = hoaCcInMsg;
    
    switch (hoaInMsg->msgType)
	{
    case INTER_EHO_PERFORM_HANDOVER_REQUEST:
		 currEvent = HAND_PERFORM_HO_REQ;
		 break;

    case INTER_EHO_PERFORM_HANDOVER_ACK:
         if (hoaCcInMsg->msgData.perfExtHandoverAck.reason)
             currEvent = HAND_PERFORM_HO_NACK;
		 else 
			 currEvent = HAND_PERFORM_HO_ACK;
         
		 break;

    case INTER_EHO_END_HANDOVER:
		 currEvent = HAND_END_HO;
		 break;

    case INTER_EHO_HANDOVER_ACCESS:
		 currEvent = HAND_HO_ACCESS;
		 break;

    case INTER_EHO_HANDOVER_SUCCESS:
		 currEvent = HAND_COMPLT_HO;
         break;

    case INTER_EHO_HANDOVER_MOB_EVENT:
         // <tyy:6-29-01> POSTMOB
         // Check DTAP 1st byte of Distribution Data Unit (Discrimination)
         if (hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA[0] & 0x01 == 1) 
         {
            if (parent->handoverSession == parent->targetHandoverSessionExtHo)
                currEvent = HAND_POSTHO_HOA_EVENT;
            else
                currEvent = HAND_POSTHO_MOB_EVENT;
         }
         else { // If it's a BSSMAP message instead then check for detect or failure
             if (hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA[2] == A_HANDOVER_DETECT_TYPE)
        		 currEvent = HAND_HO_ACCESS;
             else if (hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA[2] == A_HANDOVER_FAILURE_TYPE)
                 currEvent = HAND_SRC_HO_FAIL;
             else
             {
                 DBG_ERRORho("{\nMNEThoWARNING(ho::handleHoaCcMsg): Unexpected (A-Int MsgType %d) in MOB Event from HOA\n}\n",  
                     hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA[2]);
                 currEvent = JCC_NULL_EVENT;
             }
         }
         break;

    case INTER_EHO_PERFORM_HANDBACK_REQUEST:
         currEvent = HAND_PERFORM_HO_REQ;
         break;
		 
    case INTER_EHO_PERFORM_HANDBACK_ACK:
		 if (hoaCcInMsg->msgData.perfExtHandbackAck.reason)
             currEvent = HAND_PERFORM_HO_NACK;
		 else 
			 currEvent = HAND_PERFORM_HO_ACK;
         break;

    case INTER_EHO_COMPLETE_HANDBACK:
         currEvent = HAND_COMPLT_HO;
     
    default:
         DBG_ERRORho("{\nMNEThoWARNING(ho::handleHoaCcMsg): Unexpected (msgType = %d) from HOA\n}\n",  
                     hoaInMsg->msgType);
         currEvent = JCC_NULL_EVENT;
        
    }
  
	if (currEvent != JCC_NULL_EVENT)
	{
        DBG_TRACEho("{\nMNEThoTRACE(ho::handleHoaCcMsg): received a HOA msg (msgType=%d, event=%d, callId=%d)\n}\n",
		             hoaInMsg->msgType, currEvent, callIndex);
	}

    return(0);
}


// ----------------------- External Handover Methods -------------------------------

   void 
   CCHandover::classmarkFill( T_AIF_MSG *aifMsg )
   {
      DBG_FUNC("CCHandover::classmarkFill", CC_HANDOVER_LAYER);
      DBG_ENTER();

      aifMsg->handoverRequest.classmarkInformation1.elementIdentifier  = A_CLASSMARK_INFORMATION1_TYPE;
      aifMsg->handoverRequest.classmarkInformation1.ie_present         = false;

      aifMsg->handoverRequest.classmarkInformation2.elementIdentifier  = A_CLASSMARK_INFORMATION2_TYPE;
      aifMsg->handoverRequest.classmarkInformation2.ie_present         = true;
      aifMsg->handoverRequest.classmarkInformation2.length             = 3;

      if (parent->handoverSession == parent->anchorHandoverSession)
        {  // External handover SERVING
          if (parent->classmark2Info.ie_present)
            {
               unsigned char *pclass = (unsigned char *)&aifMsg->handoverRequest.classmarkInformation2.classmark[0];

               if (parent->classmark2Info.revLevel == CNI_RIL3_REV_LEVEL_PHASE2)
                   pclass[0] |= 0x20;
               if (parent->classmark2Info.esInd == CNI_RIL3_EARLY_CLASSMARK_SENDING_IMPLEMENTED)
                   pclass[0] |= 0x10;
               if (parent->classmark2Info.a51 == CNI_RIL3_A51_NOT_AVAILABLE)
                   pclass[0] |= 0x08;

               pclass[0] |= parent->classmark2Info.rfPowerCap;

               if (parent->classmark2Info.psCap == CNI_RIL3_PS_CAPABILITY_PRESENT)
                   pclass[1] |= 0x40;
               if (parent->classmark2Info.ssScreening == CNI_RIL3_SS_SCREENING_PHASE2)
                   pclass[1] |= 0x10;
               if (parent->classmark2Info.smCap == CNI_RIL3_SM_MT_PTP_SUPPORTED)
                   pclass[1] |= 0x08;
               if (parent->classmark2Info.vbsReception == CNI_RIL3_VBS_NOTIFICATION_WANTED)
                   pclass[1] |= 0x04;
               if (parent->classmark2Info.vgcsReception == CNI_RIL3_VGCS_NOTIFICATION_WANTED)
                   pclass[1] |= 0x02;
               if (parent->classmark2Info.fcFreqCap == CNI_RIL3_GSM_EXTENTION_BAND_G1_SUPPORTED)
                   pclass[1] |= 0x01;
               if (parent->classmark2Info.classmark3 == CNI_RIL3_CLASSMARK3_INFO_AVAILABLE)
                   pclass[2] |= 0x80;
               if (parent->classmark2Info.cmsp == CNI_RIL3_NETWORK_INITIATED_MO_SUPPORTED)
                   pclass[2] |= 0x04;
               if (parent->classmark2Info.a53 == CNI_RIL3_A53_AVAILABLE)
                   pclass[2] |= 0x02;
               if (parent->classmark2Info.a52 == CNI_RIL3_A52_AVAILABLE)
                   pclass[2] |= 0x01;
            }
          else
            {
              DBG_ERROR("Error@classmarkFill: Classmark2Info ie NOT present.  Filling with default values.\n");
              // Classmark 2 Default values (see 04.08 section 10.5.1.6 Mobile Station Classmark 6)
              //     - Revision = phase 2 MS
              //     - "Controlled Early classmark sending" option is NOT implemented in the MS
              //     - Encryption algorithm A5/1 AVAILABLE
              //     - PCS 1900 CLASS 3
              //     - Pseudo-synchronization capability NOT present
              //     - Suplementary Service screening indicator = 0
              //     - SM capability SUPPORTED
              //     - VBS, VGCS, CM3, LCS VA, SoLSA, CMSP, A5/3 all NOT supported
              //     - Frequency capability = 0
              //     - A5/2 algorithm SUPPORTED

              aifMsg->handoverRequest.classmarkInformation2.classmark[0]       = 0x22; 
              aifMsg->handoverRequest.classmarkInformation2.classmark[1]       = 0x08;
              aifMsg->handoverRequest.classmarkInformation2.classmark[2]       = 0x01;
            }
         }
      else
         {  // External handover TARGET
              aifMsg->handoverRequest.classmarkInformation2 = currClassmark_;
         }

      aifMsg->handoverRequest.classmarkInformation3.elementIdentifier  = A_CLASSMARK_INFORMATION3_TYPE;
      aifMsg->handoverRequest.classmarkInformation3.ie_present         = false;

      DBG_LEAVE();
   }

   void 
   CCHandover::channelTypeFill( T_AIF_MSG *aifMsg )
   {
      int i;
      int version_count = 0;
      TwoPartyCallLeg *whichLeg;
      T_CNI_RIL3_SPEECH_VERSION version;

      aifMsg->handoverRequest.channelType.elementIdentifier = A_CHANNEL_TYPE_TYPE;
      aifMsg->handoverRequest.channelType.ie_present = true;

      aifMsg->handoverRequest.channelType.speechDataIndicator = 1;   // Speech
      aifMsg->handoverRequest.channelType.channelRateAndType  = 8;   // Full rate TCH channel Bm

      if (parent->handoverSession == parent->anchorHandoverSession)
        {  // External handover SERVING
           whichLeg = parent->activeLeg();

           for (i=0; i<3; i++)
             {
               if (whichLeg->msSide->bearCap1speechVersionInd_[i].fieldPresent)
                 {
                    version = whichLeg->msSide->bearCap1speechVersionInd_[i].version;

                    if (version == CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1) {
                      aifMsg->handoverRequest.channelType.permittedIndicator[version_count] = 0x01;  // NOTE: This is actually GSM VERSION 1 in 08.08 spec (full rate FR)
                      version_count ++;
                    } else if (version == CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_2) {
                      aifMsg->handoverRequest.channelType.permittedIndicator[version_count] = 0x21;  // NOTE: This is actually GSM VERSION 3 in 08.08 spec (full rate AMR)
                      version_count ++;
                    } else if (version == CNI_RIL3_GSM_HALF_RATE_SPEECH_VERSION_1) {
                      aifMsg->handoverRequest.channelType.permittedIndicator[version_count] = 0x05;  // NOTE: This is actually GSM VERSION 3 in 08.08 spec (full rate AMR)
                      version_count ++;
                    } else
                    {
                      DBG_ERRORho("CCHandover::channelTypeFill -- Version content error (version=%d)\n", version);
                    }
                 }
             }
         }
      else
         {  // External handover TARGET
            if (currSpeechCh_.version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1)
               {
                  aifMsg->handoverRequest.channelType.permittedIndicator[0] = 0x01;
               }
            else if (currSpeechCh_.version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_2)
               {
                  aifMsg->handoverRequest.channelType.permittedIndicator[0] = 0x21;
               }
            else if (currSpeechCh_.version = CNI_RIL3_GSM_HALF_RATE_SPEECH_VERSION_1)
               {
                  aifMsg->handoverRequest.channelType.permittedIndicator[0] = 0x05;
               }
            else
               {
                  DBG_ERRORho("CCHandover::channelTypeFill -- Version content error (version=%d)\n", version);
               }
            version_count = 1;
         }

      switch (version_count)
        {
           case 1:
             aifMsg->handoverRequest.channelType.length = 3;
             break;

           case 2:
             aifMsg->handoverRequest.channelType.length = 4;
             aifMsg->handoverRequest.channelType.permittedIndicator[0] |= 0x80; // Set extension bit 8
             break;

           case 3:
             aifMsg->handoverRequest.channelType.length = 5;
             aifMsg->handoverRequest.channelType.permittedIndicator[0] |= 0x80; // Set extension bit 8
             aifMsg->handoverRequest.channelType.permittedIndicator[1] |= 0x80; // Set extension bit 8
             break;

           case 0:
           default:
             DBG_ERRORho("CCHandover::channelTypeFill -- Content error (i=%d)\n", i);
             aifMsg->handoverRequest.channelType.length = 3;
             aifMsg->handoverRequest.channelType.permittedIndicator[0] = 0x21;
        }
   }

   void 
   CCHandover::currentChannelTypeFill( T_AIF_MSG *aifMsg )
   {
      aifMsg->handoverRequest.currentChannelType1.elementIdentifier = A_CURRENT_CHANNEL_TYPE1_TYPE;
      aifMsg->handoverRequest.currentChannelType1.ie_present = true;
      aifMsg->handoverRequest.currentChannelType1.channelMode = 1;   // Speech
      aifMsg->handoverRequest.currentChannelType1.channel  = 8;      // Full rate TCH channel Bm
   }

   unsigned char
   CCHandover::findGpCipheringCapability(void)
   {
     DBG_FUNC("CCHandover::findGpCipheringCapability", CC_HANDOVER_LAYER);
     DBG_ENTER();

     unsigned char ciphCap=0;

     if (RM_DSPA51(rm_DspCiphCap)) ciphCap |= 0x02;
     if (RM_DSPA52(rm_DspCiphCap)) ciphCap |= 0x04;

     if (ciphCap) 
       ciphCap &= 0xFE;
     else
       ciphCap  = 1;  // No Encryption

     DBG_TRACE("INFO@findGpCipheringCapability (ciphCap=0x%x)\n",ciphCap);
     DBG_LEAVE();
     return(ciphCap);
   }

   void 
   CCHandover::encryptionInformationFill( T_AIF_MSG *aifMsg )
   {
      IntraL3CipherModeCommand_t    cipherCmd;

      if (parent->handoverSession == parent->anchorHandoverSession)
        {  // External handover SERVING
          if (ludbGetCipherAlgo(parent->ludbIndex, &cipherCmd))
            {
              DBG_ERRORho ("   WARNING: ludbGetCipherAlgo() failed, (ludbIndex = %d)\n",
                               parent->ludbIndex);
              cipherCmd.cmd.cipherModeSetting.ie_present = true;
              cipherCmd.cmd.cipherModeSetting.ciphering  = CNI_RIl3_CIPHER_NO_CIPHERING;
            }
        }
      else
         {  // External handover TARGET
            cipherCmd = currEncrypt_;
         }

      aifMsg->handoverRequest.encryptionInformation.elementIdentifier = A_ENCRYPTION_INFORMATION_TYPE;
      if (cipherCmd.cmd.cipherModeSetting.ie_present)
        {
          aifMsg->handoverRequest.encryptionInformation.ie_present = true;
          aifMsg->handoverRequest.encryptionInformation.length = 9; 
          if (cipherCmd.cmd.cipherModeSetting.ciphering == CNI_RIl3_CIPHER_NO_CIPHERING)
            {
              aifMsg->handoverRequest.encryptionInformation.permittedAlgorithm = 1; // No encryption
            }
          else
            {
              aifMsg->handoverRequest.encryptionInformation.permittedAlgorithm = findGpCipheringCapability();
            }

          memcpy( (unsigned char*)aifMsg->handoverRequest.encryptionInformation.key,
                  (unsigned char*)cipherCmd.Kc, CIPHER_KC_LEN);
        }
      else
        {
          aifMsg->handoverRequest.encryptionInformation.ie_present = true;
          aifMsg->handoverRequest.encryptionInformation.permittedAlgorithm = 1;  // No Ciphering
          aifMsg->handoverRequest.encryptionInformation.length = 9;
        };
   }

   void 
   CCHandover::currentEncryptionAlgorithmFill( T_AIF_MSG *aifMsg )
   {
      IntraL3CipherModeCommand_t    cipherCmd;

      if (parent->handoverSession == parent->anchorHandoverSession)
        {  // External handover SERVING
          if (ludbGetCipherAlgo(parent->ludbIndex, &cipherCmd))
            {
              DBG_ERRORho ("   WARNING: ludbGetCipherAlgo() failed, (ludbIndex = %d)\n",
                               parent->ludbIndex);
              cipherCmd.cmd.cipherModeSetting.ie_present = true;
              cipherCmd.cmd.cipherModeSetting.ciphering  = CNI_RIl3_CIPHER_NO_CIPHERING;
            }
        }
      else
         {  // External handover TARGET
            cipherCmd = currEncrypt_;
         }

      aifMsg->handoverRequest.chosenEncryptAlgorithmServing.elementIdentifier = A_CHOSEN_ENCRYPTION_ALGORITHM_TYPE;
      if (cipherCmd.cmd.cipherModeSetting.ie_present)
        {
          aifMsg->handoverRequest.chosenEncryptAlgorithmServing.ie_present = true;
          if (cipherCmd.cmd.cipherModeSetting.ciphering == CNI_RIl3_CIPHER_NO_CIPHERING)
            {
              aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = 1; // No encryption
            }
          else
            {
               switch (cipherCmd.cmd.cipherModeSetting.algorithm)
                 {
                   case CNI_RIL3_CIPHER_ALGORITHM_A51:
                     {
                        aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = 0x02;  // GSM A5/1
                        break;
                     }

                   case CNI_RIL3_CIPHER_ALGORITHM_A52:
                     {
                        aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = 0x03;  // GSM A5/2
                        break;
                     }

                   case CNI_RIL3_CIPHER_ALGORITHM_A53:
                     {
                        aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = 0x04;  // GSM A5/3
                        break;
                     }

                   case CNI_RIL3_CIPHER_ALGORITHM_A54:
                     {
                        aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = 0x05;  // GSM A5/4
                        break;
                     }

                   case CNI_RIL3_CIPHER_ALGORITHM_A55:
                     {
                        aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = 0x06;  // GSM A5/5
                        break;
                     }

                   case CNI_RIL3_CIPHER_ALGORITHM_A56:
                     {
                        aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = 0x07;  // GSM A5/6
                        break;
                     }

                   case CNI_RIL3_CIPHER_ALGORITHM_A57:
                     {
                        aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = 0x08;  // GSM A5/7
                        break;
                     }
                 }

              aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = findGpCipheringCapability();
            }
        }
      else
        {
          aifMsg->handoverRequest.chosenEncryptAlgorithmServing.ie_present = true;
          aifMsg->handoverRequest.chosenEncryptAlgorithmServing.algorithmId = 1;  // No Ciphering
        };
   }

   void
   CCHandover::populateAHandoverRequest(GlobalCellId_t *serving_cell, GlobalCellId_t *target_cell)
   {
      memset( (char*)&aifMsg, 0, sizeof(T_AIF_MSG));
      aifMsg.header.msgType = A_HANDOVER_REQUEST_TYPE;

      // Mandatory A-interface parameters
      channelTypeFill( &aifMsg );
      encryptionInformationFill( &aifMsg );
      classmarkFill( &aifMsg );

      aifMsg.handoverRequest.cellIdentifierTarget.elementIdentifier = A_CELL_IDENTIFIER_TYPE;
      aifMsg.handoverRequest.cellIdentifierTarget.ie_present = true;
      aifMsg.handoverRequest.cellIdentifierTarget.length = 8;
      aifMsg.handoverRequest.cellIdentifierTarget.cellIdDescriminator = 0; // The whole Cell Global Id is used
      memcpy ((unsigned char *) aifMsg.handoverRequest.cellIdentifierTarget.mcc,
                       (char *) target_cell->mcc,
                                NUM_MCC_DIGITS);
      memcpy ((unsigned char *) aifMsg.handoverRequest.cellIdentifierTarget.mnc,
                       (char *) target_cell->mnc,
                                NUM_MNC_DIGITS);
      aifMsg.handoverRequest.cellIdentifierTarget.lac = target_cell->lac;
      aifMsg.handoverRequest.cellIdentifierTarget.ci  = target_cell->ci;


      aifMsg.handoverRequest.cellIdentifierServing.elementIdentifier = A_CELL_IDENTIFIER_TYPE;
      aifMsg.handoverRequest.cellIdentifierServing.ie_present = true;
      aifMsg.handoverRequest.cellIdentifierServing.length = 8;
      aifMsg.handoverRequest.cellIdentifierServing.cellIdDescriminator = 0; // The whole Cell Global Id is used
      memcpy ((unsigned char *) aifMsg.handoverRequest.cellIdentifierServing.mcc,
                       (char *) serving_cell->mcc,
                                NUM_MCC_DIGITS);
      memcpy ((unsigned char *) aifMsg.handoverRequest.cellIdentifierServing.mnc,
                       (char *) serving_cell->mnc,
                                NUM_MNC_DIGITS);
      aifMsg.handoverRequest.cellIdentifierServing.lac = serving_cell->lac;
      aifMsg.handoverRequest.cellIdentifierServing.ci  = serving_cell->ci;

      // Optional A-interface parameters
      currentChannelTypeFill( &aifMsg );
      currentEncryptionAlgorithmFill( &aifMsg );


      DBG_TRACEho("    AIFMSG PRE-AIF PROCESSING MESSAGE DATA: A_HANDOVER_REQUEST_TYPE\n");
      DBG_TRACEho("      {\n");
      DBG_TRACEho("        - channelType\n");
      DBG_TRACEho("            - speechDataIndicator=%d\n", 
                                 aifMsg.handoverRequest.channelType.speechDataIndicator);
      DBG_TRACEho("            - channelRateAndType=%d\n", 
                                 aifMsg.handoverRequest.channelType.channelRateAndType);
      DBG_TRACEho("            - permittedIndicator[0-7]=%x,%x,%x,%x,%x,%x,%x,%x\n", 
                                 aifMsg.handoverRequest.channelType.permittedIndicator[0],
                                 aifMsg.handoverRequest.channelType.permittedIndicator[1],
                                 aifMsg.handoverRequest.channelType.permittedIndicator[2],
                                 aifMsg.handoverRequest.channelType.permittedIndicator[3],
                                 aifMsg.handoverRequest.channelType.permittedIndicator[4],
                                 aifMsg.handoverRequest.channelType.permittedIndicator[5],
                                 aifMsg.handoverRequest.channelType.permittedIndicator[6],
                                 aifMsg.handoverRequest.channelType.permittedIndicator[7]);
      DBG_TRACEho("        - encryptionInformation (ie_present=%d)\n", 
                                 aifMsg.handoverRequest.encryptionInformation.ie_present);
      DBG_TRACEho("            - permittedAlgorithm=%d\n", 
                                 aifMsg.handoverRequest.encryptionInformation.permittedAlgorithm);
      DBG_TRACEho("            - ciphData (ciphKey[1-8]: %x,%x,%x,%x,%x,%x,%x,%x)\n",
                                 aifMsg.handoverRequest.encryptionInformation.key[0],
                                 aifMsg.handoverRequest.encryptionInformation.key[1],
                                 aifMsg.handoverRequest.encryptionInformation.key[2],
                                 aifMsg.handoverRequest.encryptionInformation.key[3],
                                 aifMsg.handoverRequest.encryptionInformation.key[4],
                                 aifMsg.handoverRequest.encryptionInformation.key[5],
                                 aifMsg.handoverRequest.encryptionInformation.key[6],
                                 aifMsg.handoverRequest.encryptionInformation.key[7]);
      DBG_TRACEho("        - classmarkInformation2\n");
      DBG_TRACEho("            - classmark[0-2]: %x,%x,%x\n", 
                                 aifMsg.handoverRequest.classmarkInformation2.classmark[0], 
                                 aifMsg.handoverRequest.classmarkInformation2.classmark[1],
                                 aifMsg.handoverRequest.classmarkInformation2.classmark[2]);
      DBG_TRACEho("        - cellIdentifierTarget\n");
      DBG_TRACEho("            - cellIdDescriminator=%d\n", 
                                 aifMsg.handoverRequest.cellIdentifierTarget.cellIdDescriminator);
      DBG_TRACEho("            - cellIdentifierTarget.mcc[0-2]: %x%x%x\n",
                                 aifMsg.handoverRequest.cellIdentifierTarget.mcc[0],
                                 aifMsg.handoverRequest.cellIdentifierTarget.mcc[1],
                                 aifMsg.handoverRequest.cellIdentifierTarget.mcc[2]);
      DBG_TRACEho("            - cellIdentifierTarget.mnc[0-2]: %x%x%x\n",
                                 aifMsg.handoverRequest.cellIdentifierTarget.mnc[0],
                                 aifMsg.handoverRequest.cellIdentifierTarget.mnc[1],
                                 aifMsg.handoverRequest.cellIdentifierTarget.mnc[2]);
      DBG_TRACEho("            - lac=%d\n", 
                                 aifMsg.handoverRequest.cellIdentifierTarget.lac);
      DBG_TRACEho("            - ci=%d\n", 
                                 aifMsg.handoverRequest.cellIdentifierTarget.ci);
      DBG_TRACEho("        - cellIdentifierServing\n");
      DBG_TRACEho("            - cellIdDescriminator=%d\n", 
                                 aifMsg.handoverRequest.cellIdentifierServing.cellIdDescriminator);
      DBG_TRACEho("            - cellIdentifierServing.mcc[0-2]: %x%x%x\n",
                                 aifMsg.handoverRequest.cellIdentifierServing.mcc[0],
                                 aifMsg.handoverRequest.cellIdentifierServing.mcc[1],
                                 aifMsg.handoverRequest.cellIdentifierServing.mcc[2]);
      DBG_TRACEho("            - cellIdentifierServing.mnc[0-2]: %x%x%x\n",
                                 aifMsg.handoverRequest.cellIdentifierServing.mnc[0],
                                 aifMsg.handoverRequest.cellIdentifierServing.mnc[1],
                                 aifMsg.handoverRequest.cellIdentifierServing.mnc[2]);
      DBG_TRACEho("            - lac=%d\n", 
                                 aifMsg.handoverRequest.cellIdentifierServing.lac);
      DBG_TRACEho("            - ci=%d\n", 
                                 aifMsg.handoverRequest.cellIdentifierServing.ci);
      DBG_TRACEho("        - currentChannelType1\n");
      DBG_TRACEho("            - channelMode=%d\n", 
                                 aifMsg.handoverRequest.currentChannelType1.channelMode);
      DBG_TRACEho("            - channel=%d\n", 
                                 aifMsg.handoverRequest.currentChannelType1.channel);

      DBG_TRACEho("\n            aifMsg hexdump: ");
      DBG_HEXDUMPho((unsigned char*) &aifMsg, sizeof (T_AIF_MSG) );
      DBG_TRACEho("\n      }\n");
   }

   int
   CCHandover::selectEncryption(void)
   {
     DBG_FUNC("CCHandover::selectEncryption", CC_HANDOVER_LAYER);
     DBG_ENTER();

     int ret_val = -1;

     if (optCurrEncrypt_.ie_present)
       {
          switch (optCurrEncrypt_.algorithmId)
            {
               case 0x01: // No encryption
                 ret_val = -1;
                 break;

               case 0x02: // A51 algorithm
                 if RM_DSPA51(rm_DspCiphCap)
                   ret_val = CNI_RIL3_CIPHER_ALGORITHM_A51;
                 else
                   ret_val = -1;
                 break;

               case 0x03: // A52 algorithm
                 if RM_DSPA52(rm_DspCiphCap)
                   ret_val = CNI_RIL3_CIPHER_ALGORITHM_A52;
                 else
                   ret_val = -1;
                 break;

               default:
                 ret_val = -1;
            }
       }
     else
       {
          ret_val = -1; // No encryption
       }

     DBG_TRACE("INFO@selectEncryption (ret_val=%d)\n",ret_val);
     DBG_LEAVE();
     return (ret_val);
   }

   int
   CCHandover::selectChannelType(void)
   {
     DBG_FUNC("CCHandover::selectChannelType", CC_HANDOVER_LAYER);
     DBG_ENTER();

     int ret_val = -1;
     int i;

     for (i=0;i<8;i++)
       {
         if ((aifMsg.handoverRequest.channelType.permittedIndicator[i] & 0x7F) == 0x21)
           {
              ret_val = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_2;
              break;
           }

         if ((aifMsg.handoverRequest.channelType.permittedIndicator[i] & 0x7F) == 0x01)
           {
              ret_val = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
              // Don't break -- want to search through entire list for Version 2 preference
           }

         if ((aifMsg.handoverRequest.channelType.permittedIndicator[i] & 0x7F) == 0x05)
           {
              ret_val = CNI_RIL3_GSM_HALF_RATE_SPEECH_VERSION_1;
              // Don't break -- want to search through entire list for Version 2 preference
           }
       }

     DBG_TRACE("INFO@selectChannelType (ret_val=%d)\n",ret_val);
     DBG_LEAVE();
     return (ret_val);
   }


