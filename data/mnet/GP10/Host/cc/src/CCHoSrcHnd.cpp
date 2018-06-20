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
// File        : CCHoSrcHnd.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 05-01-99
// Description : Source Vipercell Handover Event handlers 
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
#include "CC/CCInt.h"
#include "CC/CCH323Util.h"

#include "jcc/JCCLog.h"
#include "logging/VCLOGGING.h"

#include "stdio.h"

// Temporarily use csu_head instead of csu_intf
#include "csunew/csu_head.h"

#include "CC/hortp.h"

#include "lapdm/cni_debug.h"

#include "logging/VCLOGGING.h"

#include "strLib.h"

#include "string.h"

#include "pm/pm_class.h"

#include "voip/vblink.h"

//ext-HO <chenj:05-29-01>
#include "voip/exchangeho.h"
#include "CC/A_Handover_Request_Msg.h"

//CDR <xxu:08-22-00> BEGIN
#include "CDR/CdrVoiceCall.h"
extern BtsBasicPackage ccBtsBasicPackage;
extern AdjacentCellPackage    ccAdjacentCellPackage;

//CDR <xxu:08-22-00> END

//GCELL<tyu:06-01-01> BEGIN
extern GlobalCellId_t MY_GLOBALCELL_ID;
const GlobalCellId_t OTHER_GLOBALCELL_ID_DEF_VAL = {0, {'0','0','0'}, {'0','0','0'}, 1, 1};

GlobalCellId_t ccHoTrgGlobalCellId = OTHER_GLOBALCELL_ID_DEF_VAL;
//GCELL<tyu:06-01-01> END


bool forcedHandover = false;

bool useViperBase = false;

bool forcedHandmsc3 = false;

UINT32 ccHoTrgCellIpAddress = inet_addr(GetViperCellIpAddress());

// *******************************************************************
// forward declarations.
// *******************************************************************

    //CIPH<xxu:02-10-00>
    int ludbGetCipherAlgo(short ludbIndex, IntraL3CipherModeCommand_t *cipherCmd);
    
    void 
    forcedHandoverOn (void)
    {
      forcedHandover = true;
    }
    
    void 
    forcedHandoverOff (void)
    {
      forcedHandover = false;
    }
    
    void 
    forcedHandmsc3On (void)
    {
      forcedHandmsc3 = true;
    }
    
    void 
    forcedHandmsc3Off (void)
    {
      forcedHandmsc3 = false;
    }
    
    void 
    useViperBaseOn (void)
    {
      useViperBase = true;
    }
    
    void 
    useViperBaseOff (void)
    {
      useViperBase = false;
    }
    
//GCELL<tyu:06-01-01> BEGIN
    void
    setTargetCellId(short newVal)
    {
      ccHoTrgGlobalCellId.ci = newVal;
    }
    
    void
    setTargetGlobalCellId(char mcc0, char mcc1, char mcc2, char mnc0, char mnc1, char mnc2, short lac, short ci)
    {
	  ccHoTrgGlobalCellId.ci_disc = 0; // Whole Cell Id
      ccHoTrgGlobalCellId.mcc[0] = mcc0;
      ccHoTrgGlobalCellId.mcc[1] = mcc1;
      ccHoTrgGlobalCellId.mcc[2] = mcc2;
      ccHoTrgGlobalCellId.mnc[0] = mnc0;
      ccHoTrgGlobalCellId.mnc[1] = mnc1;
      ccHoTrgGlobalCellId.mnc[2] = mnc2;
      ccHoTrgGlobalCellId.lac = lac;
      ccHoTrgGlobalCellId.ci = ci;
    }
//GCELL<tyu:06-01-01> END

    void
    setTargetIpAddress(char *trgIpAddress)
    {
      ccHoTrgCellIpAddress = inet_addr(trgIpAddress);
    }
    
    //HO<xxu:02-15-00> add test code for H3
    JCCEvent_t 
    CCAnchorHandover::handleAnchHoReqd(void)
    {
      int i;
    
      DBG_FUNC("CCAnchorHandover::handleAnchHoReqd", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // get handover vipercell id
      
      //RETRY<xxu:04-21-00> BEGIN
      //VBLINK_API_CELL_LOCATION_REQUEST_MSG cellLocationRequest;
      //RETRY<xxu:04-21-00> END
    
      IntraL3HandoverRequired_t *inMsg = &rrInMsg->l3_data.handReqd;
    
      //initData();
    
//GCELL<tyu:06-01-01> BEGIN
      hoGlobalCellId_ = ccHoTrgGlobalCellId;
//GCELL<tyu:06-01-01> END
    
      hoVcIpAddress_ = ccHoTrgCellIpAddress; 
    
      //RETRY<xxu:04-24-00> BEGIN
      hoNextCandidate_ = false;
      //RETRY<xxu:04-24-00> END
    
      handReqd         = *inMsg;

      if (!forcedHandover)
        {
          if ( (i=getNextHoCandidate()) == (-1) )
            {
               // Ignore the message 
               JCCLog1 ("RR->CC Error: Handover Required, bad (numCandidates = %d)\n", 
                        inMsg->numCandidates);
               DBG_ERROR ("RR->CC Error: Handover Required, bad (numCandidates = %d)\n", 
                          inMsg->numCandidates);
               //initData();
               DBG_LEAVE();
               return (CC_MESSAGE_PROCESSING_COMPLT);
            }

          //GCELL<tyu:06-01-01> BEGIN
          hoGlobalCellId_ = inMsg->candGlobalCellId[i].candGlobalCellId;

          DBG_TRACE("CC HO Info @anch-HoReqd: #ofCand %d, candVcId(%d,%x%x%x,%x%x%x,%d,%d) ho(type %d, state %d)\n",
                     inMsg->numCandidates, i,
                     hoGlobalCellId_.mcc[0],hoGlobalCellId_.mcc[1],hoGlobalCellId_.mcc[2],
                     hoGlobalCellId_.mnc[0],hoGlobalCellId_.mnc[1],hoGlobalCellId_.mnc[2],
                     hoGlobalCellId_.lac, hoGlobalCellId_.ci,
                     hoType_, hoState_);
                    

          // ext-HO <chenj: 05-29-01>  
          if (!inMsg->candGlobalCellId[i].candIsExternal)
          {  // Candidate is internal to MNET

             hoType_ = HO_TY_HANDOVER;

             sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", hoGlobalCellId_.lac,hoGlobalCellId_.ci);

             cellLocationRequest.TxnId = callIndex;
    
             if (VBLinkLocationRequest(cellLocationRequest)
                 == false)
               {
                 // Ignore the message 
                 DBG_ERROR ("CC->VB Link Error: Failed on Cell Location Request Message\n");
    
                 initData();
                 DBG_LEAVE();
                 return (CC_MESSAGE_PROCESSING_COMPLT);
               }

             hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;

          } else { // Candidate is external to MNET
             JCCEvent_t  event_result;

             hoType_ = HO_TY_EXT_HANDOVER;

             // Send Perform HO to external PLMN
             event_result = ExtPerformHO();
             if ( event_result != CC_MESSAGE_PROCESSING_COMPLT )
               {
                  DBG_LEAVE();
                  return event_result;
               }
           }
    
          //RETRY<xxu:04-21-00> BEGIN
          // start the VB timer 
                    
          hoRetry_ = 1;
          parent->sessionTimer->setTimer(CALL_HAND_SRC_THO);
          //RETRY<xxu:04-21-00> END
    
          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);
    
        }
    
      else
        {
          if (useViperBase)
            {
               // ext-HO <chenj: 05-29-01> 
              if (!inMsg->candGlobalCellId[i].candIsExternal)
                {  // Candidate is internal to MNET
                   hoType_ = HO_TY_HANDOVER;

                   sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", hoGlobalCellId_.lac,hoGlobalCellId_.ci);
    
                   cellLocationRequest.TxnId = callIndex;
    
                   if (VBLinkLocationRequest(cellLocationRequest)
                       == false)
                   {
                      // Ignore the message 
                     DBG_ERROR ("CC->VB Link Error: Failed on Cell Location Request Message\n");

                     initData();
                     DBG_LEAVE();
                     return (CC_MESSAGE_PROCESSING_COMPLT);
                   }
    
                   DBG_TRACE("CC->VB Link Info: viperCellId[0] '%s', lac '%d', cellId '%d', callIndex '%d'\n",
                            (&cellLocationRequest.ViperCellId[0]), 
                            hoGlobalCellId_.lac,
                            hoGlobalCellId_.ci,
                            cellLocationRequest.TxnId);

                   hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;

                } else { // Candidate is external to MNET
                  JCCEvent_t  event_result;

                  hoType_ = HO_TY_EXT_HANDOVER;

                  // Send Perform HO to external PLMN
                  event_result = ExtPerformHO();
                  if ( event_result != CC_MESSAGE_PROCESSING_COMPLT )
                    {
                      DBG_LEAVE();
                      return event_result;
                    }
                }
    
              //RETRY<xxu:04-21-00> BEGIN
              hoRetry_ = 1;
              parent->sessionTimer->setTimer(CALL_HAND_SRC_THO);
              //RETRY<xxu:04-21-00> END
    
              DBG_LEAVE();
              return (CC_MESSAGE_PROCESSING_COMPLT);
    
            }
          else 
            {
              hoType_ = HO_TY_HANDOVER;
              hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;
              DBG_LEAVE();
              return (handleVcAddressRsp());
            }
        }
    }
    
    
    JCCEvent_t 
    CCAnchorHandover::handleAnchHoFail(void)
    {
      IntraL3HandoverRequired_t *inMsg = &handReqd;
      
      DBG_FUNC("CCAnchorHandover::handleAnchHoFail", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // cancel T103 timer
      parent->sessionTimer->cancelTimer();
    
      // Send Abort Handover to the Target ViperCell.
    
      vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
      vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_REVERT_TO_OLD;
    
      sendVcMsg(MODULE_CC, MODULE_CC);
    
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
                      (int)rtpSourcePort.portId.rtpHandler);
              DBG_ERROR("CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                        csuResult,
                        (int)rtpSourcePort.portId.rtpHandler);
    		  
              // Can only generate OA&M log. 
            }
    
          hoSimplexConnected_ = false;
        }
    
      //RETRY<xxu:04-24-00> BEGIN
      // retry next candidate 
      int i;
      if ( (i=getNextHoCandidate()) != (-1) )
      {
          //Try next candidate
          hoNextCandidate_ = true;
          candIndex_ = i;
    
          //GCELL<tyu:06-01-01> BEGIN
		  hoGlobalCellId_ = inMsg->candGlobalCellId[i].candGlobalCellId;

          DBG_TRACE("CC HO Info @handleAnchHoFail(ho): #ofCand %d, candVcId(%d,%x%x%x,%x%x%x,%d,%d) ho(type %d, state %d)\n",
                     inMsg->numCandidates, i,
                     hoGlobalCellId_.mcc[0],hoGlobalCellId_.mcc[1],hoGlobalCellId_.mcc[2],
                     hoGlobalCellId_.mnc[0],hoGlobalCellId_.mnc[1],hoGlobalCellId_.mnc[2],
                     hoGlobalCellId_.lac, hoGlobalCellId_.ci,
                     hoType_, hoState_);
          
          sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", hoGlobalCellId_.lac,hoGlobalCellId_.ci);
          //GCELL<tyu:06-01-01> END
    
          cellLocationRequest.TxnId = callIndex;
    
          if (VBLinkLocationRequest(cellLocationRequest)
              == false)
          {
              // Ignore the message 
              DBG_ERROR ("CC->VB Link Error: Failed on Cell Location Request Message\n");
    
              initData();
              DBG_LEAVE();
              return (CC_MESSAGE_PROCESSING_COMPLT);
          }
    
          // start the VB timer 
          hoRetry_ = 0;
          parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);
    
          hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;
    
          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);
                
      } else
      //RETRY<xxu:04-25-00> END
    
      {
         //RETRY<xxu:04-25-00> BEGIN
         //GCELL<tyu:06-01-01> END
         DBG_TRACE("CC HO Info@handleAnchHoFail(ho): #ofCand %d, lastCandVcId(%d,%x%x%x,%x%x%x,%d,%d) ho(type %d, state %d)\n",
                    inMsg->numCandidates, candIndex_,
                    hoGlobalCellId_.mcc[0],hoGlobalCellId_.mcc[1],hoGlobalCellId_.mcc[2],
                    hoGlobalCellId_.mnc[0],hoGlobalCellId_.mnc[1],hoGlobalCellId_.mnc[2],
                    hoGlobalCellId_.lac, hoGlobalCellId_.ci,
                    hoType_, hoState_);
         //GCELL<tyu:06-01-01> END
         //RETRY<xxu:04-25-00> END
    
         // Release the Ho RTP channel
         ReleaseHandoverRtpChannel(hoRtpSession_);
         hoRtpSession_ = NULL;
    
         initData();
    
         // empty the queues now that the handover failed
         emptyRRQueue      ();
         emptyDownlinkQueue();
         emptyMMQueue      ();
      }
      
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    JCCEvent_t 
    CCAnchorHandover::handleAnchDuringHoRls(void)
    {
      DBG_FUNC("CCAnchorHandover::handleAnchDuringHoRls", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // cancel any timers running
      parent->sessionTimer->cancelTimer();
    
      if (hoState_ != HAND_ST_ANCH_VC_IP_ADDRESS_REQ)
        {
          // Send Abort Handover to the Target ViperCell.
    	  
          vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
          vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_BASE;
    	  
          sendVcMsg(MODULE_CC, MODULE_CC);
    
        }
    
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
                      (int)rtpSourcePort.portId.rtpHandler);
              DBG_ERROR("CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                        csuResult,
                        (int)rtpSourcePort.portId.rtpHandler);
    
              // Can only generate OA&M log. 
            }
    
          hoSimplexConnected_ = false;
    
        }
    
      // Release the Ho RTP channel
      ReleaseHandoverRtpChannel(hoRtpSession_);
      hoRtpSession_ = NULL;
    
      initData();
    
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    
    }
    
    
    // utility for Mobile and Connection Resource cleanup
    void 
    CCAnchorHandover::releaseMobAndConnRes(void)
    {
      DBG_FUNC("CCAnchorHandover::releaseMobAndConnRes", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      if (parent->entryId != (T_CNI_IRT_ID) JCC_UNDEFINED )
        {
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
                  JCCLog2("CSU Error : Ho Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
                          csuResult,
                          parent->entryId);
                  DBG_ERROR("CSU Error : Ho Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
                            csuResult,
                            parent->entryId);
    			  
                  // Can only generate OA&M log. 
                }
    		  
              hoSimplexConnected_ = false;
    		  
            }
        }
    
      releaseMobRes();
      DBG_LEAVE();
    }
    
    // utility for cleanup
    void 
    CCAnchorHandover::releaseMobRes(void)
    {
      DBG_FUNC("CCAnchorHandover::releaseMobRes", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      if (parent->entryId != (T_CNI_IRT_ID) JCC_UNDEFINED )
        {
          // Send internal MM msg - release channel
          sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, parent->entryId, &mmOutMsg);
    
          // <Igal 1-6-00> Updated to standard IRT reset
          parent->resetIrt();
        }
      
      parent->entryId = (T_CNI_IRT_ID) JCC_UNDEFINED;
      parent->oid = CNI_LAPDM_NULL_OID;
      DBG_LEAVE();
    }
    
    //HO<xxu:02-17-00> handle both ho and h3 cases
    JCCEvent_t 
    CCAnchorHandover::handleTrgToAnchCompltHo(void)
    {
      DBG_FUNC("CCAnchorHandover::handleTrgToAnchCompltHo", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // cancel T103 timer
      parent->sessionTimer->cancelTimer();
    
      TwoPartyCallLeg *whichLeg;
    
      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
        {
          // internal error condition
          // should not have ended up in signaling only mode.
          // abort handover and session
          DBG_LEAVE();
          return (CC_RELEASE_CALL);
        }
    
      //CDR <xxu:08-22-00>
      //GCELL<tyu:06-01-01> END
      GlobalCellId_t  globalCellId, globalCellId_t;
    
      T_CSU_PORT_ID    oldPort, srcPort, snkPort;
      T_CSU_RESULT_CON csuResult;
    
      srcPort.portType = CSU_RTP_PORT;
      srcPort.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->h323CallHandle);
    
      switch (hoType_)
      {
      case HO_TY_HANDOVER:
           oldPort.portType = CSU_GSM_PORT;
           oldPort.portId.gsmHandler = parent->entryId;
           snkPort.portType = CSU_RTP_PORT;
           snkPort.portId.rtpHandler = hoRtpSession_;
           //GCELL<tyu:06-01-01> END
           globalCellId = hoGlobalCellId_;
    
           break;
    
      case HO_TY_HANDMSC3:
           oldPort.portType = CSU_RTP_PORT;
           oldPort.portId.rtpHandler = hoRtpSession_;
           snkPort.portType = CSU_RTP_PORT;
           snkPort.portId.rtpHandler = h3RtpSession_;
           //GCELL<tyu:06-01-01> END
           globalCellId = h3GlobalCellId_;
    
           break;
    
      default:
           // Own messup happened. Need immediate attendance thus don't go further.
           // Also release any resource engaged. 
    
           JCCLog2("CC Error internal messed up: (hoType = %d) (hoState = %d)\n",
                    hoType_, hoState_);
           DBG_ERROR("CC Error internal messed up: (hoType = %d) (hoState = %d)\n",
                    hoType_, hoState_);
    
           releaseMobRes();
           handleAnchPostH3Rls();
    
           DBG_LEAVE();
           return(handleAnchPostHoRls());
      }
    
      //CDR <xxu:08-22-00> BEGIN
      T_CNI_RIL3_IE_LOCATION_AREA_ID lai;
      T_CNI_RIL3_IE_CELL_ID ci;
    
      for (int i=0; i<16; i++)
      {
           int mccInteger, mncInteger;

           mccInteger = ((unsigned int)(((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc));
           //GCELL<tyu:06-01-01> BEGIN
           globalCellId_t.mcc[0] = ((char*)&(mccInteger))[0];
           globalCellId_t.mcc[1] = ((char*)&(mccInteger))[1];
           globalCellId_t.mcc[2] = ((char*)&(mccInteger))[2];

           mncInteger = ((unsigned int)(((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mnc));
           globalCellId_t.mnc[0] = ((char*)&(mncInteger))[0];
           globalCellId_t.mnc[1] = ((char*)&(mncInteger))[1];
           globalCellId_t.mnc[2] = ((char*)&(mncInteger))[2];

           globalCellId_t.lac = ((unsigned short)(((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_lac));;
           globalCellId_t.ci = ((unsigned short)(((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_ci));;

           if (isEqualGlobalCellId(&globalCellId, &globalCellId_t))
                 break;
           //GCELL<tyu:06-01-01> END
      }
     
      if (i>=16)
           //GCELL<tyu:06-01-01> 
           DBG_WARNING("CC WARNING (ho-cdr): (hoType = %d) (hoState = %d), vc(%d,%d)\n",
                        hoType_, hoState_, globalCellId.lac, globalCellId.ci);
      else
      {
           lai.ie_present = true;
           lai.mcc[0] = ((unsigned char*)&(((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc))[0];
           lai.mcc[1] = ((unsigned char*)&(((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc))[1];
           lai.mcc[2] = ((unsigned char*)&(((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc))[2];
           lai.mnc[0] = ((unsigned char*)&(((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mnc))[0];
           lai.mnc[1] = ((unsigned char*)&(((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mnc))[1];
           lai.lac    = ((short)((((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_lac)));
    
           ci.ie_present = true;
           ci.value   = ((short)((((AdjCell_HandoverEntry*)
                         (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_ci)));
    
           if ( (parent->callLeg1->msSide != NULL) &&
                (parent->callLeg1->msSide->callState_ == HC_ST_ACTIVE) )
                 parent->callLeg1->ccCdr.handover(&lai, &ci);
           if ( (parent->callLeg2->msSide != NULL) &&
                (parent->callLeg2->msSide->callState_ == HC_ST_ACTIVE) )
                 parent->callLeg2->ccCdr.handover(&lai, &ci);
      }
      
      //CDR <xxu:08-22-00> END
    
      DBG_TRACE("CC->@anch:handleTrgToAnchCompltHo: voConn %d, ho(%d,%d) src(%d,%x) old(%d,%x) snk(%d,%x)\n",
                 whichLeg->voiceConnected,
                 hoType_, hoState_,
                 srcPort.portType, srcPort.portId.rtpHandler,
                 oldPort.portType, oldPort.portId.rtpHandler,
                 snkPort.portType, snkPort.portId.rtpHandler );
    
      if (whichLeg->voiceConnected)
      {
           // Break old connections
    
           if ( (csuResult = csu_OrigTermBreakAll(&oldPort))
                 != CSU_RESULT_CON_SUCCESS)
           {
                 JCCLog3("CSU Error : Break old conn failure, (Result = %d) for oldPort(%d, %x)\n",  
                          csuResult,
                          oldPort.portType,
                          (int) oldPort.portId.rtpHandler);
                 DBG_ERROR("CSU Error : Break old conn failure, (Result = %d) for oldPort(%d, %x)\n",  
                           csuResult,
                           oldPort.portType,
                           oldPort.portId.rtpHandler);
    	  
                 // Can only generate OA&M log. 
           }
                
           // Allow going along since ho is complete already
     
           whichLeg->voiceConnected = false;
       
           // Make duplex ho or h3 connection between src and snk rtp ports
          
           if ((csuResult = csu_DuplexConnect(&snkPort, &srcPort))
                != CSU_RESULT_CON_SUCCESS)
           {
                 JCCLog3("CSU Error : Make conn failure, (Result = %d) for (snk %p, src %p)\n",  
                          csuResult,
                          (int)snkPort.portId.rtpHandler, (int)srcPort.portId.rtpHandler);
                 DBG_ERROR("CSU Error : Make conn failure, (Result = %d) for (snk %p, src %p)\n",  
                          csuResult,
                          snkPort.portId.rtpHandler, srcPort.portId.rtpHandler);
             
                 // Treat this as internal messup, thus don't go further either ! 
                 // 
    
                 if (hoType_ == HO_TY_HANDOVER)
                 {
                     // ho: Cleanup!
                     releaseMobRes();
    
                     DBG_LEAVE();
                     return(handleAnchPostHoRls());
                     
                 } else
                 {
                     // h3: Cleanup!
                     handleAnchPostH3Rls();
    
                     DBG_LEAVE();
                     return(handleAnchPostHoRls());
                 }
            }
    
            // 
            whichLeg->voiceConnected = true;
    
      } else
      {
          // break the simplex connection setup earlier.
    	  
          if ((csuResult = csu_OrigTermBreakAll(&srcPort))
              != CSU_RESULT_CON_SUCCESS)
          {
    
              JCCLog2("CSU Error : Break src conn failure, (Result = %d) for vpPortHndl %p)\n",  
                       csuResult,
                       (int)srcPort.portId.rtpHandler);
              DBG_ERROR("CSU Error : Break src conn failure, (Result = %d) for vpPortHndl %p)\n",  
                       csuResult,
                       srcPort.portId.rtpHandler);
    	  
              // Can only generate OA&M log. 
          }
      }
    
      //GCELL<tyu:06-01-01> BEGIN
      DBG_TRACE("CC->@anch:handleTrgToAnchCompltHo bf. B'->B: hoType %d, ho(rtp %p, vcIp %x vcId %d, callId %d, rePort %d)\n",
                 hoType_,
                 hoRtpSession_,
                 hoVcIpAddress_,
                 hoGlobalCellId_.ci,
                 hoVcCallIndex_,
                 otherHoIncomingRtpPort_);
    
      DBG_TRACE("CC->@anch:handleTrgToAnchCompltHo bf. B'->B: hoType %d, h3(rtp %p, vcIp %x vcId %d, callId %d, rePort %d)\n",
                 hoType_,
                 h3RtpSession_,
                 h3VcIpAddress_,
                 h3GlobalCellId_.ci,
                 h3VcCallIndex_,
                 otherH3IncomingRtpPort_);
      //GCELL<tyu:06-01-01> END
    
      // h3: update MSC-B' as MSC-B
    
      if (hoType_ == HO_TY_HANDMSC3)
      {       
          // Send the Complete Handback message to vcB
    
          vcOutMsg.msgType = INTER_VC_CC_COMPLETE_HANDBACK;
    
          sendVcMsg(MODULE_CC, MODULE_CC);
    
          // Release old tgt rtpHndle
    
          if ( hoRtpSession_ != NULL )
          {
               if ( (csuResult = csu_OrigTermBreakAll(&oldPort))
                    != CSU_RESULT_CON_SUCCESS )
               {
                    // This failure may not be a real failure, thus only treating it as INFO type
    
                    JCCLog2("CSU INFO : Break old conn failure, (Result = %d) for oldRtpHndl %p)\n",  
                             csuResult,
                             (int)oldPort.portId.rtpHandler);
                    DBG_TRACE("CSU INFO : Break old conn failure, (Result = %d) for oldRtptHndl %p)\n",  
                              csuResult,
                              oldPort.portId.rtpHandler);
               }
    
               ReleaseHandoverRtpChannel(hoRtpSession_);
          }
    
          // Update vcB'->vcB
    
          hoRtpSession_  		 = h3RtpSession_;
          h3RtpSession_  		 = NULL;
          hoVcIpAddress_ 		 = h3VcIpAddress_;
          h3VcIpAddress_ 		 = 0;
          hoVcCallIndex_ 		 = h3VcCallIndex_;
          h3VcCallIndex_ 		 = CC_MAX_CC_CALLS_DEF_VAL;
          otherHoIncomingRtpPort_  = otherH3IncomingRtpPort_;
          otherH3IncomingRtpPort_  = 0;
          //GCELL<tyu:06-01-01> BEGIN
          hoGlobalCellId_ = h3GlobalCellId_;
          memset(&h3GlobalCellId_, 0, sizeof(GlobalCellId_t));
          //GCELL<tyu:06-01-01> END
          hoSimplexConnected_      = false;
          h3SimplexConnected_      = false;
          hoSrcLinkLost_           = false;
          h3SrcLinkLost_           = false;
    
      } else
      {
          // Release rf
          releaseMobRes();
    
          hoSimplexConnected_ = false;
          hoSrcLinkLost_      = false;
      }
    
      //GCELL<tyu:06-01-01> BEGIN
      DBG_TRACE("CC->@anch:handleTrgToAnchCompltHo af. B'->B: hoType %d, ho(rtp %p, vcIp %x vcId %d, callId %d, rePort %d)\n",
                 hoType_,
                 hoRtpSession_,
                 hoVcIpAddress_,
                 hoGlobalCellId_.ci,
                 hoVcCallIndex_,
                 otherHoIncomingRtpPort_);
      
    
      DBG_TRACE("CC->@anch:handleTrgToAnchCompltHo af. B'->B: hoType_ %d, h3(rtp %p, vcIp %x vcId %d, callId %d, rePort %d)\n",
                 hoType_,
                 h3RtpSession_,
                 h3VcIpAddress_,
                 h3GlobalCellId_.ci,
                 h3VcCallIndex_,
                 otherH3IncomingRtpPort_);
      //GCELL<tyu:06-01-01> END
    
      hoType_   = HO_TY_HANDOVER;
      hoState_  = HAND_ST_ANCH_HO_COMPLT;
    
      //BCT <xxu:09-22-00> Begin
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
                                                   &whichLeg->msSide->annSnkPort_) )
                     != CSU_RESULT_CON_SUCCESS)
               {
                     DBG_WARNING("CC->BCT(csuSConnFailedAfterHo):xlst(%d) ann(%d,%x) snk(%d,%x,%d)\n",
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
      //BCT <xxu:09-21-00> END
  
      // empty the queues now that the handover is successful
      // Do this only at the end.
    
      emptyRRQueue      ();
      emptyDownlinkQueue();
      emptyMMQueue      ();
    
      //
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    
    JCCEvent_t 
    CCAnchorHandover::handleAnchT104TimerExpiry(void)
    {
      DBG_FUNC("CCAnchorHandover::handleAnchT104TimerExpiry", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // Send  Handback Nack to the Target ViperCell. 
      // Ack was already sent! Is that Ok?
    
      vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
      vcOutMsg.msgData.perfHandbackNack.cause = JC_FAILURE_T104_TIMEOUT;
    
      sendVcMsg(MODULE_CC, MODULE_CC);
    
      releaseMobAndConnRes();
    
      initHandbackData();
    
      // empty the queues now that the handover failed
      emptyRRQueue      ();
      emptyDownlinkQueue();  
      emptyMMQueue      ();
      
      //
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    JCCEvent_t 
    CCAnchorHandover::handleAnchHoReqAck(void)
    {
      DBG_FUNC("CCAnchorHandover::handleAnchHoReqAck", CC_HANDOVER_LAYER);
      DBG_ENTER();
      
      TwoPartyCallLeg *whichLeg;
    
      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
        {
          // signaling activity (CISS, SMS Point to Point, etc)
          // abort handover
          DBG_LEAVE();
          return (handleAnchHoReqNack());
        }
    
      T_CNI_L3_ID newId;
      newId.msgq_id =  msgQId;
      newId.sub_id = 0;                 // No sub id needed here
    
    
      // <Igal 1-6-00> Updated to standard IRT setup
      if (parent->setIrt (mmInMsg->entry_id, newId) == false)
      {
          // Treat it like a Ho. Req. Nack.
          releaseMobAndConnRes();	  
          DBG_LEAVE();
          return   (handleAnchHoReqNack());
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
    
          releaseMobAndConnRes();
          DBG_LEAVE();
          return (handleAnchHoReqNack());
        }
    
      parent->oid = irtEntry.lapdm_oid;
    
      // Make a simplex connection between the remote rtp channel and the rf channel
      T_CSU_PORT_ID rfSinkPort, rtpSourcePort;
      T_CSU_RESULT_CON csuResult;
    
      rfSinkPort.portType = CSU_GSM_PORT;
      rtpSourcePort.portType = CSU_RTP_PORT;
    
      rfSinkPort.portId.gsmHandler = parent->entryId;
      rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->h323CallHandle);
    
      if ((csuResult = csu_SimplexConnect(&rtpSourcePort, &rfSinkPort))
          != CSU_RESULT_CON_SUCCESS)
        {
          JCCLog3("CSU Error : Ho Simplex Conn. Failure, (Result = %d) for (rtp port = %x) and (entryId = %d)\n",  
                  csuResult,
                  (int)rtpSourcePort.portId.rtpHandler,
                  parent->entryId);
          DBG_ERROR("CSU Error : Ho Simplex Conn. Failure, (Result = %d) for (rtp port = %x) and (entryId = %d)\n",  
                    csuResult,
                    (int)rtpSourcePort.portId.rtpHandler,
                    parent->entryId);
    
          releaseMobAndConnRes();
          DBG_LEAVE();
          return (handleAnchHoReqNack());
        }
    
      hoSimplexConnected_ = true;
    
      // Send the Perform Ack message.
    
      vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_ACK;
      vcOutMsg.msgData.perfHandbackAck.handCmd  = mmInMsg->l3_data.handReqAck.handCmd;
    
      sendVcMsg(MODULE_CC, MODULE_CC);
    
      // start the T104 timer 
      parent->sessionTimer->setTimer(CALL_HAND_TRG_T104);
    
      hoState_ = HAND_ST_ANCH_SND_HB_REQ_ACK;
    
      // 
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    JCCEvent_t 
    CCAnchorHandover::handleAnchHoReqNack(void)
    {
      DBG_FUNC("CCAnchorHandover::handleAnchHoReqNack", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // Send the Perform Nack message.
    
      vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
      vcOutMsg.msgData.perfHandbackNack.cause  = rrInMsg->l3_data.handReqNack.cause;
    
      sendVcMsg(MODULE_CC, MODULE_CC);
    
      initHandbackData();
    
      //
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    JCCEvent_t 
    CCAnchorHandover::handleAnchHoComplt(void)
    {
      DBG_FUNC("CCAnchorHandover::handleAnchHoComplt", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // cancel T104 timer
      parent->sessionTimer->cancelTimer();
    
      TwoPartyCallLeg *whichLeg;
    
      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
        {
          // internal error condition
          // should not have ended up in signaling only mode.
          // abort handover and session
          DBG_LEAVE();
          return (CC_RELEASE_CALL);
        }
    
      //CDR <xxu:08-22-00> BEGIN
      T_CNI_RIL3_IE_LOCATION_AREA_ID lai;
      T_CNI_RIL3_IE_CELL_ID ci;
    
      ci.ie_present = true;
      ci.value = ((short)(ccBtsBasicPackage.bts_ci));
    
      lai.ie_present = true;
      lai.mcc[0] = ((unsigned char*)&(ccBtsBasicPackage.bts_mcc))[0];
      lai.mcc[1] = ((unsigned char*)&(ccBtsBasicPackage.bts_mcc))[1];
      lai.mcc[2] = ((unsigned char*)&(ccBtsBasicPackage.bts_mcc))[2];
      lai.mnc[0] = ((unsigned char*)&(ccBtsBasicPackage.bts_mnc))[0];
      lai.mnc[1] = ((unsigned char*)&(ccBtsBasicPackage.bts_mnc))[1];
      lai.lac    = ((short)(ccBtsBasicPackage.bts_lac));
    
      if ( (parent->callLeg1->msSide != NULL) &&
           (parent->callLeg1->msSide->callState_ == HC_ST_ACTIVE) )
            parent->callLeg1->ccCdr.handover(&lai, &ci);
      if ( (parent->callLeg2->msSide != NULL) &&
           (parent->callLeg2->msSide->callState_ == HC_ST_ACTIVE) )
            parent->callLeg2->ccCdr.handover(&lai, &ci);
      //CDR <xxu:08-22-00> END
    
      // Send the Complete Handback message.
      vcOutMsg.msgType = INTER_VC_CC_COMPLETE_HANDBACK;
    
      sendVcMsg(MODULE_CC, MODULE_CC);
    
      if (whichLeg->voiceConnected)
        {
          // Call is not held at this point for some reason.
    
          // First take down the earlier connection
          
          // Break the connections with the RTP Port
          T_CSU_PORT_ID hoRtpSourcePort;
          T_CSU_RESULT_CON csuResult;
          
          hoRtpSourcePort.portType = CSU_RTP_PORT;
          hoRtpSourcePort.portId.rtpHandler = hoRtpSession_;
          
          if ((csuResult = csu_OrigTermBreakAll(&hoRtpSourcePort))
              != CSU_RESULT_CON_SUCCESS)
            {
    	  
              JCCLog2("CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                      csuResult,
                      (int)hoRtpSourcePort.portId.rtpHandler);
              DBG_ERROR("CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                        csuResult,
                    (int)hoRtpSourcePort.portId.rtpHandler);
              
              // Can only generate OA&M log. 
              
              // Don't continue
              
              hoSimplexConnected_ = false;
              
              // Release the Ho RTP channel
              ReleaseHandoverRtpChannel(hoRtpSession_);
              hoRtpSession_ = NULL;
              
              initData();
              
              DBG_LEAVE();
              return(CC_TWO_STAGE_DISCONNECT);
            }
      
          // voice is broken
          whichLeg->voiceConnected = false;
          
          // Complete the connection
          // Make a simplex connection between the remote rtp channel and the rf channel
          T_CSU_PORT_ID rfSinkPort, rtpSourcePort;
          
          rfSinkPort.portType = CSU_GSM_PORT;
          rtpSourcePort.portType = CSU_RTP_PORT;
          
          rfSinkPort.portId.gsmHandler = parent->entryId;
          rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->h323CallHandle);
          
          if ((csuResult = csu_DuplexConnect(&rfSinkPort, &rtpSourcePort))
              != CSU_RESULT_CON_SUCCESS)
            {
              
              JCCLog3("CSU Error : Ho Duplex Conn. Failure, (Result = %d) for (rtp port = %x) and (entryId = %d)\n",  
                      csuResult,
                      (int)rtpSourcePort.portId.rtpHandler,
                      parent->entryId);
              DBG_ERROR("CSU Error : Ho Duplex Conn. Failure, (Result = %d) for (rtp port = %x) and (entryId = %d)\n",  
                        csuResult,
                        (int)rtpSourcePort.portId.rtpHandler,
                        parent->entryId);
              
              // Skip event loop.
              // No recovery is possible
              DBG_LEAVE();
              return(CC_TWO_STAGE_DISCONNECT);
              
            }
          whichLeg->voiceConnected = true;
          
        }
      
      else
        {
          // break the simplex connection that was setup earlier.
          // Break the connections with the RF Port
          T_CSU_PORT_ID rfSourcePort;
          T_CSU_RESULT_CON csuResult;
          
          rfSourcePort.portType = CSU_GSM_PORT;
          rfSourcePort.portId.gsmHandler = parent->entryId;
          
          if ((csuResult = csu_OrigTermBreakAll(&rfSourcePort))
              != CSU_RESULT_CON_SUCCESS)
            {
              JCCLog2("CSU Error : Ho Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
                      csuResult,
                      parent->entryId);
              DBG_ERROR("CSU Error : Ho Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
                        csuResult,
                        parent->entryId);
              
              // Can only generate OA&M log. 
            }
          
        }
    
      hoSimplexConnected_ = false;
      
      // Release the Ho RTP channel
      ReleaseHandoverRtpChannel(hoRtpSession_);
      hoRtpSession_ = NULL;
      
      initData();
      
      //BCT <xxu:09-22-00> Begin
      if ( (whichLeg->msSide != NULL) &&
           (whichLeg->msSide->annState_ == ANN_PLAY_WAITING) )
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
                  DBG_WARNING("CC->BCT(csuSConnFailedAfterHb):xlst(%d) ann(%d,%x) snk(%d,%x,%d)\n",
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
      //BCT <xxu:09-22-00> END
 
   
      // empty the queues now that the handover is successful
      // Do this only at the end.
      emptyRRQueue      ();
      emptyDownlinkQueue();  
      emptyMMQueue      ();
    
      //
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    //HO<xxu:01-24-00> Handle both H3 and HO
    JCCEvent_t 
    CCAnchorHandover::handleVcAddressRsp(void)
    {
      DBG_FUNC("CCAnchorHandover::handleVcAddressRsp", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      TwoPartyCallLeg *whichLeg;

      parent->sessionTimer->cancelTimer();
    
      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
      {
          // Either ho or h3: only h3 needs to abort mscB here
          // signaling activity (CISS, SMS Point to Point, etc)
          // cleanup mscA and abort handover junk @ target cell 
         
          switch (hoState_)
          {
          case HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ:
               //H3      
               DBG_TRACE("vcA<-VB: got vcB' IP Addr Rsp but no active leg thus aborting mscB\n");
    
               vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
               vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_CALL_ALREADY_DROPPED;
               sendVcMsg(MODULE_CC, MODULE_CC);
    
               //Cleanup h3
               initHandmsc3Data();

               //clear ho
               initData();
    
               break;
             
          case HAND_ST_ANCH_VC_IP_ADDRESS_REQ:
               //HO
               DBG_TRACE("vcA<-VB: got vcB IP Addr Rsp but no active leg HO case\n");

               //clear ho
               initData();
    
               break;

          case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
               DBG_TRACE("vcA<-VB: got vcB IP Addr Rsp but no active leg HO case. (hoType:%d, hoState:%d)\n",
                          hoType_, hoState_);

               hoEndCause_ = MC_HO_SUBHO_FAILURE;
               sendEndHandover();
               initData();

               break;
    
          default:
               JCCLog2 ("CC Error: messeup hoType %d, hoState %d\n", hoType_, hoState_);
               DBG_ERROR ("CC Error: messeup hoType %d, hoState %d\n", hoType_, hoState_);
    
               //Internal messup bu cleanup is still better while trouble shooting it!
               initHandmsc3Data();

               //clear ho
               initData();

               break;
          }

          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);
      }
    
      // Got active leg, assign a new Ho RTP channel
      HJCRTPSESSION reRtpSession;
    
      //RETRY<xxu:04-24-00> BEGIN
      if (hoNextCandidate_)
      {
          if (hoType_ == HO_TY_HANDOVER)
              reRtpSession = hoRtpSession_;
          else if ( (hoType_ == HO_TY_HANDMSC3) ||
                    (hoType_ ==  HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR) )
              reRtpSession = h3RtpSession_;
          else
          {
              DBG_LEAVE();
              return (CC_MESSAGE_PROCESSING_COMPLT);
          }
    
          //PR1521 <xxu:09-12-00> 
          // Release the Ho RTP channel
          if (reRtpSession != NULL)
              ReleaseHandoverRtpChannel(reRtpSession);
          else
              DBG_WARNING("CC Ho(retry): ho(%d,%d), last rtpHandle is NULL (%p)\n",
                           hoType_, hoState_, reRtpSession);
      }
    
      //RETRY<xxu:04-24-00> END
      {
          if ((reRtpSession = AssignHandoverRtpChannel())
               == NULL)
          {
               // Do not proceed with ho or h3
               JCCLog2 ("CC Error: rtp channel assignment failed (hoType %d, hoState %d)\n",
                        hoType_, hoState_);
               DBG_ERROR ("CC Error: rtp channel assignment failed (hoType %d, hoState %d)\n",
                        hoType_, hoState_);
    
               switch (hoState_)
               {
               case HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ:
                    // Give up the h3 effort but maintain the current ho
                    initHandmsc3Data();
                    break;
    
               case HAND_ST_ANCH_VC_IP_ADDRESS_REQ:
                    // Change the state back to no handover
                    initData();
                    break;

               case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:

                    hoEndCause_ = MC_HO_SUBHO_FAILURE;
                    sendHandbackNack();
                    initExtHandmsc3Data();
                    break;
    
               default:
                    //Deadly error! Troubleshooting needed!
                    JCCLog2 ("CC Error: messed up hoState '%d'. (hoType %d)\n",
                            hoState_, hoType_);
                    DBG_ERROR ("CC Error: messed up hoState '%d'. (hoType %d)\n",
                            hoState_, hoType_);
                    break;
               } 
           
               DBG_LEAVE();
               return (CC_MESSAGE_PROCESSING_COMPLT);
           }
      }
    
      vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDOVER;
      vcOutMsg.msgData.perfHandoverReq.sourceInRtpPort 
                       = GetIncomingHandoverRtpPort(reRtpSession);
      vcOutMsg.msgData.perfHandoverReq.channelType.bearerType
                       = BEARER_SPEECH;
      vcOutMsg.msgData.perfHandoverReq.channelType.speechChann
                       = parent->currSpeechCh();
      vcOutMsg.msgData.perfHandoverReq.globalCellId
                       = MY_GLOBALCELL_ID;

      //CIPH<xxu:02-10-00>Begin
    
      IntraL3CipherModeCommand_t	cipherCmd;
      if (ludbGetCipherAlgo(parent->ludbIndex, &cipherCmd))
      {
          //handling error return
          JCCLog1 ("CC->Handover Error: ludbGetCipherAlgo() failed, (ludbIndex = %d)\n",
                    parent->ludbIndex);
          DBG_ERROR ("CC->Handover Error: ludbGetCipherAlgo() failed, (ludbIndex = %d)\n",
                    parent->ludbIndex);
          cipherCmd.cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
      }
      
      DBG_TRACE("CC->@anch:PerfHo(a2t): ludbIndex = %d, bearer %d, spchVer %d, ciphSet %d, algo %d, kc(%x,%x)\n",
                 parent->ludbIndex,
                 vcOutMsg.msgData.perfHandoverReq.channelType.bearerType,
                 vcOutMsg.msgData.perfHandoverReq.channelType.speechChann.version,
                 cipherCmd.cmd.cipherModeSetting.ciphering,
                 cipherCmd.cmd.cipherModeSetting.algorithm,
                 cipherCmd.Kc[0],cipherCmd.Kc[1],
                 cipherCmd.Kc[0],cipherCmd.Kc[2]);
    
      memcpy( (unsigned char*) &(vcOutMsg.msgData.perfHandoverReq.cipherCmd),
              (unsigned char*) &(cipherCmd),
              sizeof (IntraL3CipherModeCommand_t) );
    
      //CIPH<xxu:02-10-00>End
    
    
      // Send Perform Handover to the Target ViperCell.
      switch (hoState_)
      {
      case HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ:
           // H3 case
    
           DBG_TRACE("CC->@anch:handleVcAddressRsp: loRtpHndl %p, loPort %d, h3VcIpAddress %d\n",
                      reRtpSession, vcOutMsg.msgData.perfHandoverReq.sourceInRtpPort, h3VcIpAddress_);
    
           sendVcMsgH3(MODULE_CC, MODULE_CC);
    
           h3RtpSession_ = reRtpSession;
           hoState_ = HAND_ST_ANCH_SND_H3_REQ;
           break;
    
      case HAND_ST_ANCH_VC_IP_ADDRESS_REQ:
           // HO case
    
           DBG_TRACE("CC->@anch:handleVcAddressRsp: loRtpHndl %p, loPort %d, hoVcIpAddress %d\n",
                      reRtpSession, vcOutMsg.msgData.perfHandoverReq.sourceInRtpPort, hoVcIpAddress_);
    
           sendVcMsg(MODULE_CC, MODULE_CC);
    
           hoRtpSession_ = reRtpSession;
           hoState_ = HAND_ST_ANCH_SND_HO_REQ;
           break;

      case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
           DBG_TRACE("CC->@anch:handleVcAddressRsp: loRtpHndl %p, loPort %d, hoVcIpAddress %d\n",
                      reRtpSession, vcOutMsg.msgData.perfHandoverReq.sourceInRtpPort, hoVcIpAddress_);
    
           sendVcMsgH3(MODULE_CC, MODULE_CC);
    
           h3RtpSession_ = reRtpSession;
           hoState_ = HAND_ST_ANCH_EHO_SND_H3_REQ;
           break;
      }
    
      // start the Tho timer 
      parent->sessionTimer->setTimer(CALL_HAND_SRC_THO);
    
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    
    //HO<xxu:01-31-00> extend to handle both HO & H3
    JCCEvent_t 
    CCAnchorHandover::handleTrgToAnchPerfHoNack(void)
    {
      int i;
      IntraL3HandoverRequired_t *inMsg = &handReqd;
    
      DBG_FUNC("CCAnchorHandover::handleTrgToAnchPerfHoNack", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // cancel Tho timer
      parent->sessionTimer->cancelTimer();
    
      // Only retry next candidate if available in this case
      switch(hoType_)
      {
      case HO_TY_HANDOVER:
           //RETRY<xxu:04-24-00> BEGIN retry next candidate 
           if ( (i=getNextHoCandidate()) != (-1) )
           {
               //Try next candidate
               hoNextCandidate_ = true;
               candIndex_ = i;
    
               //GCELL<tyu:06-01-01> BEGIN
               hoGlobalCellId_ = inMsg->candGlobalCellId[i].candGlobalCellId;
               
               DBG_TRACE("CC HO Info @handleTrgToAnchPerfHoNack(ho): #ofCand %d, candVcId(%d,%x%x%x,%x%x%x,%d,%d) ho(type %d, state %d)\n",
                          inMsg->numCandidates, i,
                          hoGlobalCellId_.mcc[0],hoGlobalCellId_.mcc[1],hoGlobalCellId_.mcc[2],
                          hoGlobalCellId_.mnc[0],hoGlobalCellId_.mnc[1],hoGlobalCellId_.mnc[2],
                          hoGlobalCellId_.lac, hoGlobalCellId_.ci,
                          hoType_, hoState_);
          
               sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", hoGlobalCellId_.lac,hoGlobalCellId_.ci);
               //GCELL<tyu:06-01-01> END
    
               cellLocationRequest.TxnId = callIndex;
    
               if (VBLinkLocationRequest(cellLocationRequest)
                   == false)
               {
                   // Ignore the message 
                   DBG_ERROR ("CC->VB Link Error: Failed on Cell Location Request Message\n");
    
                   initData();
                   DBG_LEAVE();
                   return (CC_MESSAGE_PROCESSING_COMPLT);
               }
    
               // start the VB timer 
               parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);
    
               hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;
    
               DBG_LEAVE();
               return (CC_MESSAGE_PROCESSING_COMPLT);
                
           } else
           //RETRY<xxu:04-24-00> END
    
           {
               // ho failed
               if (hoSrcLinkLost_)
               {
    	       // peg 
                   PM_CCMeasurement.unsuccExternHDOsWithLossOfConnectionPerMSC.increment();
    	  
                   initData();
                   DBG_LEAVE();
                   return(CC_RELEASE_CALL);					  
               } else
               {
                   initData();
               }
           }
           break;
    
      case HO_TY_HANDMSC3:
           //Abort action @ MSC-B'
           vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
           vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_ANCH_V3_FAILED;
    
           sendVcMsgH3(MODULE_CC, MODULE_CC);
    
           // Abort HB part at MSC-B
           vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
           vcOutMsg.msgData.perfHandbackNack.cause = JC_FAILURE_ANCH_V3_FAILED;
    
           sendVcMsg(MODULE_CC, MODULE_CC);
     
           initHandmsc3Data();
     
           break;

      case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
           //Abort action @ MSC-B'
           vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
           vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_ANCH_V3_FAILED;
    
           sendVcMsgH3(MODULE_CC, MODULE_CC);
    
           // Abort HB part at MSC-B
           vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
           vcOutMsg.msgData.perfHandbackNack.cause = JC_FAILURE_ANCH_V3_FAILED;
    
           sendVcMsg(MODULE_CC, MODULE_CC);
     
           initHandmsc3Data();
     
           break;
    
      default:
           //Deadly problem, need shoubleshooting first before running ahead !
           JCCLog2 ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_ERROR ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_LEAVE();
           return (CC_MESSAGE_PROCESSING_COMPLT);
      }
    
      // empty the queues now that the handover failed
      emptyRRQueue      ();
      emptyDownlinkQueue();  
      emptyMMQueue      ();
      
      //
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    //HO<xxu:01-28-00> handle both h3 and ho casse!
    JCCEvent_t 
    CCAnchorHandover::handleTrgToAnchPerfHoAck(void)
    {
      DBG_FUNC("CCAnchorHandover::handleTrgToAnchPerfHoAck", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // cancel Tho timer
      parent->sessionTimer->cancelTimer();
    
      TwoPartyCallLeg *whichLeg;
    
      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
      {
          // signaling activity (CISS, SMS Point to Point, etc)
          // abort handover
          JCCLog2 ("CC Trace: no active leg (hoType %d, hoState %d)\n", hoType_, hoState_);
          DBG_TRACE ("CC Trace: no active leg (hoType %d, hoState %d)\n", hoType_, hoState_);
          DBG_LEAVE();
          return (handleAnchAbortWiCos(JC_FAILURE_CALL_ALREADY_DROPPED));
      }
    
      T_CSU_PORT_ID  rtpSinkPort;
      bool           need_simplex_connection;
    
      switch(hoType_)
      {
      case HO_TY_HANDMSC3:
           DBG_TRACE("CC->@anch(h3):handleTrgToAnchPerfHoAck: loRtpHndl %p, reVcIpAddr %x, rePort %d\n",
                     h3RtpSession_,
                     h3VcIpAddress_,
                     vcInMsg->msgData.perfHandoverAck.targetInRtpPort);
    
           h3VcCallIndex_ = vcInMsg->origSubId;
    
           if (h3RtpSession_ == NULL)
           {
               DBG_WARNING("CC->@anch(h3):handleTrgToAnchPerfHoAck: ho(%d,%d) null-RtpHandle(%p)\n",
                            hoType_, hoState_, h3RtpSession_);
               DBG_LEAVE();
               return (handleAnchAbortWiCos(JC_FAILURE_ANCH_HO_DISALOWED));
           }
 
           SetRemoteHandoverRtpPort(h3RtpSession_,
                               h3VcIpAddress_,
                               vcInMsg->msgData.perfHandoverAck.targetInRtpPort);
    
           rtpSinkPort.portType = CSU_RTP_PORT;
           rtpSinkPort.portId.rtpHandler = h3RtpSession_;
    
           DBG_TRACE("CC->@anch(h3):handleTrgToAnchPerfHoAck: (hoType %d, hoState_ %d)\n",
                      hoType_, hoState_);

           need_simplex_connection = true;
                    
           break;
    
      case HO_TY_HANDOVER:
           hoVcCallIndex_ = vcInMsg->origSubId;
    
           //PR1521 <xxu:09-12-00>
           DBG_TRACE("CC->@anch(ho):handleTrgToAnchPerfHoAck: loRtpHndl %p, reVcIpAddr %x, rePort %d callIndex %d\n",
                     hoRtpSession_,
                     hoVcIpAddress_,
                     vcInMsg->msgData.perfHandoverAck.targetInRtpPort,
                     hoVcCallIndex_);
    
           if (hoRtpSession_ == NULL)
           {
               DBG_WARNING("CC->@anch(ho):handleTrgToAnchPerfHoAck: ho(%d,%d) null-RtpHandle(%p)\n",
                            hoType_, hoState_, hoRtpSession_);
               DBG_LEAVE();
               return (handleAnchAbortWiCos(JC_FAILURE_ANCH_HO_DISALOWED));
           }
  
           SetRemoteHandoverRtpPort(hoRtpSession_,
                               hoVcIpAddress_,
                               vcInMsg->msgData.perfHandoverAck.targetInRtpPort);
    
           rtpSinkPort.portType = CSU_RTP_PORT;
           rtpSinkPort.portId.rtpHandler = hoRtpSession_;

           need_simplex_connection = true;

           break;

      case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
           DBG_TRACE("CC->@anch(h3):handleTrgToAnchPerfHoAck: loRtpHndl %p, reVcIpAddr %x, rePort %d\n",
                     h3RtpSession_,
                     h3VcIpAddress_,
                     vcInMsg->msgData.perfHandoverAck.targetInRtpPort);
    
           h3VcCallIndex_ = vcInMsg->origSubId;
    
           if (h3RtpSession_ == NULL)
           {
               DBG_WARNING("CC->@anch(h3):handleTrgToAnchPerfHoAck: ho(%d,%d) null-RtpHandle(%p)\n",
                            hoType_, hoState_, h3RtpSession_);
               hoEndCause_ = MC_HO_SUBHO_FAILURE;
               sendEndHandover();
               initData();

               DBG_LEAVE();
               return (CC_RELEASE_HOCALLLEG);
           }
 
           SetRemoteHandoverRtpPort(h3RtpSession_,
                               h3VcIpAddress_,
                               vcInMsg->msgData.perfHandoverAck.targetInRtpPort);
    
           rtpSinkPort.portType = CSU_RTP_PORT;
           rtpSinkPort.portId.rtpHandler = h3RtpSession_;
    
           DBG_TRACE("CC->@anch(h3):handleTrgToAnchPerfHoAck: (hoType %d, hoState_ %d)\n",
                      hoType_, hoState_);

           need_simplex_connection = false;
                    
           break;

      default:
           //Deadly problem, need shoubleshooting first before running ahead !
           JCCLog2 ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_ERROR ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_LEAVE();
           return (CC_MESSAGE_PROCESSING_COMPLT);
      }
    

      if (need_simplex_connection)
        {
          // simplex conn for ho or h3
          T_CSU_PORT_ID  rtpSourcePort;
          T_CSU_RESULT_CON csuResult;
     
          rtpSourcePort.portType = CSU_RTP_PORT;
          rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->h323CallHandle);
    
          DBG_TRACE("CC->@anch PerfHock(t2a): (hoType %d,hoState %d) (vpRtpHndl %p->reRtpHndl %p)\n",
                     hoType_, hoState_, rtpSourcePort.portId.rtpHandler, rtpSinkPort.portId.rtpHandler);
    
          if ((csuResult = csu_SimplexConnect(&rtpSourcePort, &rtpSinkPort))
              != CSU_RESULT_CON_SUCCESS)
            {
              JCCLog2("CSU Error : Ho or h3 Simplex Conn. Failure, (hoType %d, hoState %)\n",
                       hoType_, hoState_);
              JCCLog3("CSU Error : Ho or h3 Simplex Conn. Failure, (Result = %d) for (rtpPort %x --> rtpPort %x)\n",  
                      csuResult,
                      (int)rtpSourcePort.portId.rtpHandler,
                      (int)rtpSinkPort.portId.rtpHandler);
              DBG_ERROR("CSU Error : Ho or h3 Simplex Conn. Failure, (hoType %d, hoState %) (Result = %d) for (rtpPort %x --> rtpPort %x)\n",  
                        hoType_,hoState_,
                        csuResult,
                        (int)rtpSourcePort.portId.rtpHandler,
                        (int)rtpSinkPort.portId.rtpHandler);
    
              // Treating like a  Handover Failure will behave as required.
              // Skip event loop.
              DBG_LEAVE();
              return (handleAnchAbortWiCos(JC_FAILURE_CSU_SIMPLEX_CONNETION));
            }
          }

     switch(hoType_)
     {
      case HO_TY_HANDOVER:
           // Send internal RR msg
           rrOutMsg.l3_data.handCmd.handCmd =  vcInMsg->msgData.perfHandoverAck.handCmd;
    
           // Dump the handover Command msg.
           // CNI_DumpHex(&rrOutMsg.l3_data.handCmd.handCmd.buffer[0], 30);
    
           sendRR(INTRA_L3_DATA, INTRA_L3_RR_HANDOVER_COMMAND, parent->entryId, &rrOutMsg);
    
           hoSimplexConnected_ = true;
           hoState_ = HAND_ST_ANCH_RCV_HO_REQ_ACK;
    
           break;
    
      case HO_TY_HANDMSC3:
           //Send HB ack to MSC-B based on HO ack from MSC-B'
           vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_ACK;
           vcOutMsg.msgData.perfHandbackAck.handCmd  = vcInMsg->msgData.perfHandoverAck.handCmd;
    
           sendVcMsg(MODULE_CC, MODULE_CC);
    
           h3SimplexConnected_ = true;
           hoState_ = HAND_ST_ANCH_RCV_H3_REQ_ACK;
    
           break;

      case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:

           // Send HANDBACK ACK to external MSC which originated the PERFORM HANDBACK
           sendHandbackAck();
    
           hoState_ = HAND_ST_ANCH_EHO_RCV_H3_REQ_ACK;
    
           break;
         
         
      //default: no other case check needed again!
    
      }
    
      // start the T103 timer 
      parent->sessionTimer->setTimer(CALL_HAND_SRC_T103);
    
      //
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    //HO<xxu:01-24-00> handle both HB and H3 cases!
    JCCEvent_t 
    CCAnchorHandover::handleTrgToAnchPerfHbReq(void)
    {
      JCCEvent_t   event_result;

      DBG_FUNC("CCAnchorHandover::handleTrgToAnchPerfHbReq", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // Use the same criteria as when triggered on the anchor side.
      if (!parent->isHandoverAllowed())
      {
          // Need to fix the cause value for the nack.
          DBG_ERROR("vcA->vcB: HB or H3 failed on handover disallowed at vcA\n");
          return(handleAnchHoReqNackWiCos(JC_FAILURE_ANCH_HO_DISALOWED));
      }
    
      //HO<xxu:01-24-00> Handle both HB and H3 scenarioes
      //xxuHoTbd: need to check networkId to handle multiple VBs!
    
      //GCELL<tyu:06-01-01> BEGIN
      DBG_TRACE("CC HO Info @anch-PerfHbReq: Mcc(%x%x%x), Mnc(%x%x%x), candVcId(%d,%d), ho(type %d, state %d)\n",
                 vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mcc[0],
                 vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mcc[1],
                 vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mcc[2],
                 vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mnc[0],
                 vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mnc[1],
                 vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mnc[2],
                 vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.lac,
                 vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.ci,
                 hoType_, hoState_);


      if (vcInMsg->msgData.perfHandbackReq.globalCellId.candIsExternal)
        {
           event_result = do_MNET_Handback_To_External_PLMN_scenario();
        }
      else
        {
          if ( isEqualGlobalCellId(&(vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId), &MY_GLOBALCELL_ID)
               && !forcedHandmsc3 )
            { 
               event_result = do_MNET_Handback_scenario();
            }
          else
            {
               event_result =  do_MNET_Handback_To_Third_MNET_scenario();
            }
        }

      DBG_LEAVE();
      return(event_result);
    }
    
    
    //HO<xxu:01-31-00> extend to both HB and H3
    JCCEvent_t 
    CCAnchorHandover::handleTrgToAnchAbortHb(void)
    {
      DBG_FUNC("CCAnchorHandover::handleTrgToAnchAbortHb", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      DBG_TRACE("CC Info: (hoType '%d', hoState %d') aborted by MSC-B\n", 
                hoType_, hoState_);
    
      // cancel T104 (hb) or T103 (h3)
      parent->sessionTimer->cancelTimer();
    
      switch (hoType_)
      {
      case HO_TY_HANDBACK:
           // Cleanup hb at anch
    
           releaseMobAndConnRes();
           initHandbackData();
    
           break;
    	
      case HO_TY_HANDMSC3:
           // Abort HO part at MSC-B'
    
           vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
           vcOutMsg.msgData.perfHandbackNack.cause =   JC_FAILURE_REVERT_TO_OLD;
     
           sendVcMsgH3(MODULE_CC, MODULE_CC);
          
           initHandmsc3Data();
    
           break; 
    
      default:
           //Deadly problem, need shoubleshooting first before running ahead !
           JCCLog2 ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_ERROR ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_LEAVE();
           return (CC_MESSAGE_PROCESSING_COMPLT);
      }
    
      // empty the queues now that the handover failed
      emptyRRQueue      ();
      emptyDownlinkQueue();  
      emptyMMQueue      ();
      
      //
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    //HO<xxu:01-28-00> handle both h3 and ho cases!
    JCCEvent_t 
    CCAnchorHandover::handleAnchThoTimerExpiry(void)
    {
      int i;
      IntraL3HandoverRequired_t *inMsg = &handReqd;
    
      DBG_FUNC("CCAnchorHandover::handleAnchThoTimerExpiry", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      //Retry next candidate if there is, otherwise give up handover effort
    
      switch (hoType_)
      {
      case HO_TY_HANDOVER:
           //RETRY<xxu:04-21-00> BEGIN, not retry to the same cell assumed
           //if retry to the same cell needed, getNextHoCandidate could be
           //modified to support this in the future.
           if ( (i=getNextHoCandidate()) != (-1) )
           {
               //Try next candidate
               hoNextCandidate_ = true;
               candIndex_ = i;
    
               //GCELL<tyu:06-01-01> BEGIN
               hoGlobalCellId_ = inMsg->candGlobalCellId[i].candGlobalCellId;

               DBG_TRACE("CC HO Info @handleAnchThoTimerExpiry(ho): #ofCand %d, candVcId(%d,%x%x%x,%x%x%x,%d,%d) ho(type %d, state %d)\n",
                          inMsg->numCandidates, i,
                          hoGlobalCellId_.mcc[0],hoGlobalCellId_.mcc[1],hoGlobalCellId_.mcc[2],
                          hoGlobalCellId_.mnc[0],hoGlobalCellId_.mnc[1],hoGlobalCellId_.mnc[2],
                          hoGlobalCellId_.lac, hoGlobalCellId_.ci,
                          hoType_, hoState_);
          
               sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", hoGlobalCellId_.lac,hoGlobalCellId_.ci);
               //GCELL<tyu:06-01-01> END
    
               cellLocationRequest.TxnId = callIndex;
    
               if (VBLinkLocationRequest(cellLocationRequest)
                   == false)
               {
                   // Ignore the message 
                   DBG_ERROR ("CC->VB Link Error: Failed on Cell Location Request Message\n");
    
                   //Release the Ho data @ MSC-A
                   ReleaseHandoverRtpChannel(hoRtpSession_);
                   hoRtpSession_ = NULL;
    
                   initData();
                   DBG_LEAVE();
                   return (CC_MESSAGE_PROCESSING_COMPLT);
               }
    
               // start the VB timer 
               parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);
    
               hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;
    
           //RETRY<xxu:04-21-00> END
           } else
           {
               //Abort action @ MSC-B
               sendVcMsg(MODULE_CC, MODULE_CC);
    
               //Release the Ho data @ MSC-A
               ReleaseHandoverRtpChannel(hoRtpSession_);
               hoRtpSession_ = NULL;
               initData();
               hoType_ = (HandoverTypes_t )0;
           }
    
           break;
    
      case HO_TY_HANDMSC3:
           //Abort action @ MSC-B'
           vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
           vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_THO_TIMEOUT;
    
           sendVcMsgH3(MODULE_CC, MODULE_CC);
    
           //Release the H3 data @ MSC-A
           initHandmsc3Data();
    
           //Abort HB from MSC-B to MSC-A
           vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
           vcOutMsg.msgData.perfHandbackNack.cause = JC_FAILURE_THO_TIMEOUT;
           sendVcMsg(MODULE_CC, MODULE_CC);
    
           break;
    
      default:
           //Deadly problem, release call and troubleshoot asap!
           JCCLog1("CC Error: messed up hoType_ '%d'\n", hoType_);
           DBG_ERROR("CC Error: messed up hoType_ '%d'\n", hoType_);
           DBG_LEAVE();
           return(CC_RELEASE_CALL);
           break;
      }
       
      // Treating like a  Handover Failure will behave as required.
      // Skip event loop.
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    //HO<xxu:01-31-00> extend to handle ho and h3
    JCCEvent_t 
    CCAnchorHandover::handleAnchT103TimerExpiry(void)
    {
      DBG_FUNC("CCAnchorHandover::handleAnchT103TimerExpiry", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // May consider to retry the same candidate later but right now just send
      // Abort Handover to the Target ViperCell: MSC-B (ho); MSC-B' (h3) etc.
    
      DBG_TRACE("CC Info: T103 expiry during (hoType %d, hoState)\n", hoType_, hoState_);
    
      switch(hoType_)
      {
      case HO_TY_HANDOVER:
           // Abort ho
    
           vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
           vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_T103_TIMEOUT;
    
           sendVcMsg(MODULE_CC, MODULE_CC);
           initData();
    
           return(CC_RELEASE_CALL);	
    
           break;
    
      case HO_TY_HANDMSC3:
           // Abort h3 at MSC-B'
    
           vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
           vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_T103_TIMEOUT;
    
           sendVcMsgH3(MODULE_CC, MODULE_CC);
    
           // Abort hb at MSC-B
    
           vcOutMsg.msgData.perfHandbackNack.cause = JC_FAILURE_T103_TIMEOUT;
    
           sendVcMsg(MODULE_CC, MODULE_CC);
     
           initHandmsc3Data();
           initData();
    
           return(CC_RELEASE_CALL);	
    
           break;

      case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:

           vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
           vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_TVB_TIMEOUT;
    
           sendVcMsgH3(MODULE_CC, MODULE_CC);

           //Send HANDBACK Nack to MSC which triggered PERFORM HANDBACK
           hoEndCause_ = MC_HO_T204_TIMEOUT;
           sendEndHandover();
           initData();

           DBG_LEAVE();
           return (CC_RELEASE_HOCALLLEG);
    
      default:
           //Deadly problem, need shoubleshooting first before running ahead !
           JCCLog2 ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_ERROR ("CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_LEAVE();
           return (CC_MESSAGE_PROCESSING_COMPLT);
      }
    
      // empty the queues now that the handover failed
      emptyRRQueue      ();
      emptyDownlinkQueue();  
      emptyMMQueue      ();
      
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    //HO<xxu:02-23-00> handle h3 release
    JCCEvent_t 
    CCAnchorHandover::handleAnchPostH3Rls(void)
    {
    
      DBG_FUNC("CCAnchorHandover::handleAnchPostH3Rls", CC_HANDOVER_LAYER);
      DBG_ENTER();

      switch(hoType_)
      {
      case HO_TY_HANDOVER:
      case HO_TY_HANDBACK:
      case HO_TY_HANDMSC3:

      // cancel any timers running
      parent->sessionTimer->cancelTimer();
    
      // Send Abort Handover to new Target ViperCell.
    
      vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
      vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_BASE;
    
      T_CSU_PORT_ID rtpSourcePort;
      T_CSU_RESULT_CON csuResult;
      
      //Cleanup of MSC-A to MSC-B' part
    
      sendVcMsgH3(MODULE_CC, MODULE_CC);
    
      rtpSourcePort.portType = CSU_RTP_PORT;
      rtpSourcePort.portId.rtpHandler = h3RtpSession_;
    
      if (h3RtpSession_ != NULL)
      {
          if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort))
               != CSU_RESULT_CON_SUCCESS)
          {
               JCCLog3("CSU Error @anch-PostH3Rls : break h3RtpHandle Failure, (hoType:%d) (Result:%d) (h3Port:%x)\n",  
                        hoType_,
                        csuResult,
                       (int)rtpSourcePort.portId.rtpHandler);
               DBG_ERROR("CSU Error @anch-PostH3Rls : break h3RtpHandle Failure, (hoType:%d) (Result:%d) (h3Port:%x)\n",  
                        hoType_,
                        csuResult,
                        (int)rtpSourcePort.portId.rtpHandler);
    
               // Can only generate OA&M log. 
          }
    
          // Release the H3 RTP channel
      
          ReleaseHandoverRtpChannel(h3RtpSession_);
      } else
      {
               JCCLog2("CSU Warning @anch-PostH3Rls : h3RtpHandle is NULL (hoType:%d, hoState %d)\n", 
                        hoType_, hoState_);
               DBG_WARNING("CSU Warning @anch-PostH3Rls : h3RtpHandle is NULL (hoType:%d, hoState %d)\n", 
                        hoType_, hoState_);
      }
     
    
      h3SimplexConnected_ = false;
      h3RtpSession_ = NULL;
    
      initHandmsc3Data();

      default:
         {
           DBG_TRACE ("CC Info @anch-handleAnchPostH3Rls: doing nothing (hoType: %d) (hoState: %d)\n", 
                      hoType_, hoState_);
           DBG_LEAVE();
           return(CC_MESSAGE_PROCESSING_COMPLT);
         }
      }
       
      DBG_LEAVE();
      return (CC_RELEASE_CALL);
    }
    
    //HO<xxu:02-01-00>
    JCCEvent_t 
    CCAnchorHandover::handleAnchPostHoRls(void)
    {
    
      DBG_FUNC("CCAnchorHandover::handleAnchPostHoRls", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // cancel any timers running
      parent->sessionTimer->cancelTimer();
    
      switch(hoType_)
      {
      case HO_TY_HANDOVER:
           // Send Abort Handover to the Target ViperCell and release call
    
           DBG_TRACE("CC HO INFO @anch-postHoRls: got RLS during HO (hoType %d) (hoState %d)\n",
                      hoType_, hoState_);
    
           cleanHoCallWiCause(INTER_VC_CC_ABORT_HANDOVER, JC_FAILURE_OTGT_CALL_RLS);
           
           DBG_LEAVE();
           return (CC_RELEASE_CALL);
    
           break;
    
      case HO_TY_HANDBACK:
    
           // Send Abort Handback to the Anchor ViperCell and release call
    
           DBG_TRACE("CC HO INFO @anch-postHoRls: got RLS during HB (hoType %d) (hoState %d)\n",
                      hoType_, hoState_);
    
           cleanHoCallWiCause(INTER_VC_CC_ABORT_HANDOVER, JC_FAILURE_OTGT_CALL_RLS);
    
           DBG_LEAVE();
           return (CC_RELEASE_CALL);
    
           break;
    
      case HO_TY_HANDMSC3:
           // Process in terms of where H3 is progressing now
    
           switch(hoState_)
           {
           case HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ:
                // Just give up H3 effort and release call
    
                initHandmsc3Data();
                cleanHoCallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_OTGT_CALL_RLS);
    
                DBG_LEAVE();
                return (CC_RELEASE_CALL);
    
                break;
    
           case HAND_ST_ANCH_SND_H3_REQ:
                // Proceed in terms of where RLS is from old or new tgt cell
    
                //GCELL<tyu:06-01-01>
                if (vcInMsg->origVcId.cellId == h3GlobalCellId_.ci && vcInMsg->origVcId.networkId == h3GlobalCellId_.lac)
                { 
                     // Give up H3 effort but maintain current HO
    
                     cleanH3CallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_NTGT_CALL_RLS);
                     cleanHoCallWiCause(INTER_VC_CC_PERFORM_HANDBACK_NACK,JC_FAILURE_NTGT_CALL_RLS);
    
                     DBG_LEAVE();
                     return (CC_MESSAGE_PROCESSING_COMPLT);
    
                       //GCELL<tyu:06-01-01>
                } else if (vcInMsg->origVcId.cellId == hoGlobalCellId_.ci && vcInMsg->origVcId.networkId == hoGlobalCellId_.lac)
                {
                     // Release this call and cleanup call environment
    
                     // Abort both the H3 effort and current HO, release call
                     cleanH3CallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_NTGT_CALL_RLS);
                     cleanHoCallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_NTGT_CALL_RLS);
    
                     DBG_LEAVE();
                     return (CC_RELEASE_CALL);
    
                } else
                {
                     // Log this wrong landed message and release call for trouble shoot
    
                     //GCELL<tyu:06-01-01>
                     DBG_ERROR ("CC Error @anch-postHoRls: messup ho(%d, %d) cellId(rx %d, ho %d, h3 %d)\n", 
                               hoType_, hoState_, 
                               vcInMsg->origVcId.cellId,
                               hoGlobalCellId_.ci, h3GlobalCellId_.ci);
    
                     // Abort both the H3 effort and current HO, release call
                     cleanH3CallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_NTGT_CALL_RLS);
                     cleanHoCallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_NTGT_CALL_RLS);
    
                     DBG_LEAVE();
                     return (CC_RELEASE_CALL);
    
                }
                 
                break;
              
           case HAND_ST_ANCH_RCV_H3_REQ_ACK:
                // Simply abort both H3 and HO now, also release call. Improve later!
                cleanH3CallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_NTGT_CALL_RLS);
                cleanHoCallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_NTGT_CALL_RLS);
    
                DBG_LEAVE();
                return (CC_RELEASE_CALL);
    
                break;
                
           default:
                // Ignore this H3 effort but maintain the cur HO
                JCCLog3 ("CC Error @anch-postHoRls: messed up (hoType: %d) (hoState: %d) (cellId: %d\n", 
                          hoType_, hoState_, vcInMsg->origVcId.cellId);
                DBG_ERROR ("CC Error @anch-postHoRls: messed up (hoType: %d) (hoState: %d) (cellId: %d\n", 
                           hoType_, hoState_, vcInMsg->origVcId.cellId);
    
                // Abort both the H3 effort and current HO, release call
                cleanH3CallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_NTGT_CALL_RLS);
                cleanHoCallWiCause(INTER_VC_CC_ABORT_HANDOVER,JC_FAILURE_NTGT_CALL_RLS);
    
                DBG_LEAVE();
                return (CC_RELEASE_CALL);
           }
    
           break;
    
      default:
           DBG_TRACE ("CC Info @anch-postHoRls: doing nothing (hoType: %d) (hoState: %d)\n", 
                      hoType_, hoState_);
           DBG_LEAVE();
           return (CC_MESSAGE_PROCESSING_COMPLT);
           
      }
    
    }
    
    //HO<xxu:02-01-00> extend to cover H3 case 
    JCCEvent_t 
    CCAnchorHandover::handleAnchPostHoMobEvent(void)
    {
    
      DBG_FUNC("CCAnchorHandover::handleAnchPostHoMobEvent", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      switch (vcInMsg->origModuleId)
        {
        case MODULE_RM:
          // Need to call the handler in the session
          parent->handleRRMsg(&vcInMsg->msgData.postHoMobEventData.intraL3Msg);
          break;
          
        case MODULE_MD:
          // Need to call the handler in the session
          parent->handleMobMsg(&vcInMsg->msgData.postHoMobEventData.msInMsg);
          break;
         
        default:
          // late event
          DBG_ERROR("EXT VC->CC Error: Unexpected event from (module id=%d)\n",
                    vcInMsg->origModuleId);
          
        }
    
      DBG_LEAVE();
      return(CC_MESSAGE_PROCESSING_COMPLT);
    
    }
    
    //HO<xxu:01-27-00> sendback HoReqNack with failure cause
    JCCEvent_t 
    CCAnchorHandover::handleAnchHoReqNackWiCos(JcFailureCause_t cause)
    {
      DBG_FUNC("CCAnchorHandover::handleAnchHoReqNackWiCos", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      DBG_TRACE("CC hb or h3 failed (cause: %d)\n", cause);
    
      // Send the Perform Nack message.
    
      vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
      vcOutMsg.msgData.perfHandbackNack.cause  = cause;
    
      sendVcMsg(MODULE_CC, MODULE_CC);
    
      switch (hoType_)
      {
      case HO_TY_HANDMSC3:
      case HO_TY_EXT_HANDMSC3:
      case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
           initHandmsc3Data();

      case HO_TY_HANDBACK:
      case HO_TY_EXT_HANDBACK:
           initHandbackData();
           break;
      default:
           JCCLog2 ("CC Error: call dropped or messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_ERROR ("CC Error: call dropped or messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           break;
      }
      //
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    //HO<xxu:01-28-00> Abort ho or h3 with a specific cause given
    JCCEvent_t 
    CCAnchorHandover::handleAnchAbortWiCos(JcFailureCause_t cause)
    {
      DBG_FUNC("CCAnchorHandover::handleAnchAbortWiCos", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // Assume no retry is needed based on candidate list
    
      // Send Abort Handover to the Target ViperCell.
    
      vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
      vcOutMsg.msgData.abortHandover.cause = cause;
    
      switch(hoType_)
      {
    
      case HO_TY_HANDMSC3:
           sendVcMsgH3(MODULE_CC, MODULE_CC);
    
           // Release the H3 RTP channel
           ReleaseHandoverRtpChannel(h3RtpSession_);
           h3RtpSession_ = NULL;
           initHandmsc3Data();
     
      case HO_TY_HANDOVER:
           sendVcMsg(MODULE_CC, MODULE_CC);
    
           // Release the Ho RTP channel
           ReleaseHandoverRtpChannel(hoRtpSession_);
           hoRtpSession_ = NULL;
           initData();
            
           break;
    
      default:
           JCCLog2 ("CC Error: call dropped or messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           DBG_ERROR ("CC Error: call dropped or messed up (hoType: %d) (hoState: %d)\n", 
                    hoType_, hoState_);
           break;
      }
    
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    //HO<xxu:02-06-00>
    JCCEvent_t 
    CCAnchorHandover::cleanHoCallWiCause(InterVcMsgType_t msg, JcFailureCause_t cause)
    {
      DBG_FUNC("CCAnchorHandover::cleanHoCallWiCause", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // Send Abort Handover to the Target ViperCell.
    
      vcOutMsg.msgType = msg;
      vcOutMsg.msgData.abortHandover.cause = cause;
    
      sendVcMsg(MODULE_CC, MODULE_CC);
    
      // break connection
      if (msg==INTER_VC_CC_ABORT_HANDOVER)
      {
          T_CSU_PORT_ID rtpSourcePort;
          T_CSU_RESULT_CON csuResult;
      
          rtpSourcePort.portType = CSU_RTP_PORT;
          rtpSourcePort.portId.rtpHandler = hoRtpSession_;
    
          if (hoRtpSession_ != NULL)
          {
              if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort))
                   != CSU_RESULT_CON_SUCCESS)
              {
                   JCCLog3("CSU Error @anch-cleanHoCallWiCause : break hoRtpHandle Failure, (hoType:%d) (Result:%d) (hoPort:%x)\n",  
                            hoType_,
                            csuResult,
                            (int)rtpSourcePort.portId.rtpHandler);
                   DBG_ERROR("CSU Error @anch-cleanHoCallWiCause : break hoRtpHandle Failure, (hoType:%d) (Result:%d) (hoPort:%x)\n",  
                            hoType_,
                            csuResult,
                            (int)rtpSourcePort.portId.rtpHandler);
    
                   // Can only generate OA&M log. 
              }
    
              // Release the Ho RTP channel
      
              ReleaseHandoverRtpChannel(hoRtpSession_);
    
          } else
          {
               JCCLog2("CSU Warning @anch-cleanHoCallWiCause : hoRtpHandle is NULL (hoType:%d, hoState %d)\n", 
                        hoType_, hoState_);
               DBG_WARNING("CSU Warning @anch-cleanHoCallWiCause : hoRtpHandle is NULL (hoType:%d, hoState %d)\n", 
                        hoType_, hoState_);
          }
    
          hoSimplexConnected_ = false;
          hoRtpSession_ = NULL;
    
          initData();
      }
    
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    
    }
    
    //HO<xxu:02-06-00>
    JCCEvent_t 
    CCAnchorHandover::cleanH3CallWiCause(InterVcMsgType_t msg, JcFailureCause_t cause)
    {
      DBG_FUNC("CCAnchorHandover::cleanH3CallWiCause", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      // Send Abort Handover to the Target ViperCell.
    
      vcOutMsg.msgType = msg;
      vcOutMsg.msgData.abortHandover.cause = cause;
    
      sendVcMsgH3(MODULE_CC, MODULE_CC);
    
      // break connection
      if (msg==INTER_VC_CC_ABORT_HANDOVER)
      {
          T_CSU_PORT_ID rtpSourcePort;
          T_CSU_RESULT_CON csuResult;
      
          rtpSourcePort.portType = CSU_RTP_PORT;
          rtpSourcePort.portId.rtpHandler = h3RtpSession_;
    
          if (hoRtpSession_ != NULL)
          {
              if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort))
                   != CSU_RESULT_CON_SUCCESS)
              {
                   JCCLog3("CSU Error @anch-cleanH3CallWiCause : break h3RtpHandle Failure, (hoType:%d) (Result:%d) (hoPort:%x)\n",  
                            hoType_,
                            csuResult,
                            (int)rtpSourcePort.portId.rtpHandler);
                   DBG_ERROR("CSU Error @anch-cleanH3CallWiCause : break h3RtpHandle Failure, (hoType:%d) (Result:%d) (hoPort:%x)\n",  
                            hoType_,
                            csuResult,
                            (int)rtpSourcePort.portId.rtpHandler);
    
                   // Can only generate OA&M log. 
              }
    
              // Release the Ho RTP channel
      
              ReleaseHandoverRtpChannel(h3RtpSession_);
    
          } else
          {
               JCCLog2("CSU Warning @anch-cleanH3CallWiCause : h3RtpHandle is NULL (hoType:%d, hoState %d)\n", 
                        hoType_, hoState_);
               DBG_WARNING("CSU Warning @anch-cleanH3CallWiCause : h3RtpHandle is NULL (hoType:%d, hoState %d)\n", 
                        hoType_, hoState_);
          }
    
          h3SimplexConnected_ = false;
          h3RtpSession_ = NULL;
    
          initHandmsc3Data();
      }
    
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
    
    }
    
    //HO<xxu:05-01-00> handle to btw anchor and VB
    JCCEvent_t 
    CCAnchorHandover::handleAnchTvbTimerExpiry(void)
    {
      DBG_FUNC("CCAnchorHandover::handleAnchTvbTimerExpiry", CC_HANDOVER_LAYER);
      DBG_ENTER();
    
      switch (hoType_)
      {
      case HO_TY_HANDOVER:
           //handover failed at the stage of target IP address resolution
    
           DBG_ERROR ("CC->VB Link Timeout: Failed on Tvb expiry (hoSt %d, hoTyp %d)\n",
                               hoState_, hoType_);
           initData();
           DBG_LEAVE();
    
           return (CC_MESSAGE_PROCESSING_COMPLT);
    
           break;
    
      case HO_TY_HANDMSC3:
           //Handmsc3 failed at the stage of target IP-address resolution
    
           vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
           vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_TVB_TIMEOUT;
    
           sendVcMsg(MODULE_CC, MODULE_CC);
    
           //Cleanup h3
           initHandmsc3Data();
    
           DBG_LEAVE();
           return (CC_MESSAGE_PROCESSING_COMPLT);
    
           break;

      case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
           //Handmsc3 (to MNET) failed at the state of target IP-address resolution

           vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
           vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_TVB_TIMEOUT;
    
           sendVcMsgH3(MODULE_CC, MODULE_CC);

           //Send HANDBACK Nack to MSC which triggered PERFORM HANDBACK
           hoEndCause_ = MC_HO_T204_TIMEOUT;
           sendHandbackNack();
           initExtHandbackData();
    
           DBG_LEAVE();
           return (CC_MESSAGE_PROCESSING_COMPLT);
    
      default:
           //Deadly problem, release call and troubleshoot asap!
           DBG_ERROR("CC Ho Error: messed up at (hoSt %d, hoTyp %d, rc %d)\n",
                      hoState_, hoType_, hoRetry_);
    
           return(CC_RELEASE_CALL);
           break;
      }
       
      // Treating like a  Handover Failure will behave as required.
      // Skip event loop.
      DBG_LEAVE();
      return (CC_MESSAGE_PROCESSING_COMPLT);
           
    }

    // ext-HO <chenj:05-29-01>
    int
    CCAnchorHandover::handleHoaCcMsg(InterHoaVcMsg_t      *hoaCcInMsg)
    {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleHoaCcMsg): entering......\n}\n");
      
      CCHandover::handleHoaCcMsg(hoaCcInMsg); // Set currEvent according to msg received

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleHoaCcMsg): leaving.\n}\n");
      return (smHandleEvent());
    }

   // ext-HO <chenj:06-29-01>
   JCCEvent_t
   CCAnchorHandover::do_MNET_Handback_scenario(void)
   {
      DBG_FUNC("CCAnchorHandover::do_MNET_Handback_scenario", CC_HANDOVER_LAYER);
      DBG_ENTER();

      initHandbackData();

      DBG_TRACE("vcB->vcA: hb is requested (hoType:%d, hoState:%d)\n",
                 hoType_, hoState_);

      // Build a message for MM for Handover Req.
      // Fill the data for MM message
      mmOutMsg.l3_data.handReq.channelType.bearerType = BEARER_SPEECH;
      mmOutMsg.l3_data.handReq.channelType.speechChann = parent->currSpeechCh();

      //CIPH<xxu:02-10-00>Begin

      IntraL3CipherModeCommand_t	cipherCmd;
      if (ludbGetCipherAlgo(parent->ludbIndex, &cipherCmd))
      {
          //handling error return
          JCCLog1 ("CC->Handover Error: ludbGetCipherAlgo() failed, (ludbIndex = %d)\n",
                    parent->ludbIndex);
          DBG_ERROR ("CC->Handover Error: ludbGetCipherAlgo() failed, (ludbIndex = %d)\n",
                    parent->ludbIndex);
          cipherCmd.cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
      }
 
      DBG_TRACE("CC->@anch:PerfHb(a2r): ludbIndex %d, bearer %d, spchVer %d, ciphSet %d, algo %d, kc(%x,%x)\n",
            parent->ludbIndex,
            vcOutMsg.msgData.perfHandoverReq.channelType.bearerType,
            vcOutMsg.msgData.perfHandoverReq.channelType.speechChann.version,
            cipherCmd.cmd.cipherModeSetting.ciphering,
            cipherCmd.cmd.cipherModeSetting.algorithm,
            cipherCmd.Kc[0],cipherCmd.Kc[1],
            cipherCmd.Kc[0],cipherCmd.Kc[2]);

      memcpy( (unsigned char*) &(mmOutMsg.l3_data.handReq.cipherCmd),
               (unsigned char*) &(cipherCmd),
                sizeof (IntraL3CipherModeCommand_t) );

      //CIPH<xxu:02-10-00>End

      mmOutMsg.l3_data.handReq.qid = msgQId;

      // Send internal MM msg - 
      sendMM(INTRA_L3_MM_EST_REQ, INTRA_L3_RR_HANDOVER_REQ, parent->entryId, &mmOutMsg);

      // Mark handover type and relevant state
      hoType_  = HO_TY_HANDBACK;
      hoState_ = HAND_ST_ANCH_RCV_HB_REQ;
      DBG_LEAVE();
   }

   // ext-HO <chenj:06-29-01>
   JCCEvent_t
   CCAnchorHandover::do_MNET_Handback_To_Third_MNET_scenario(void)
   {
     VBLINK_API_CELL_LOCATION_REQUEST_MSG t_cellLocationRequest;

     DBG_FUNC("CCAnchorHandover::do_MNET_Handback_To_Third_MNET_scenario", CC_HANDOVER_LAYER);
     DBG_ENTER();

     initHandmsc3Data();

     if (forcedHandmsc3)
     {
         //GCELL<tyu:06-01-01> 
         h3GlobalCellId_ = ccHoTrgGlobalCellId;
         h3VcIpAddress_  = ccHoTrgCellIpAddress; 
   
     } else
         //GCELL<tyu:06-01-01> 
         h3GlobalCellId_ = vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId;

     if ( (!forcedHandmsc3) || (forcedHandmsc3&&useViperBase) )
     {
         //H3 scenario. Go ahead to inquire IP address of mscB', later check
         //if the mscB' is known or not.

         //GCELL<tyu:06-01-01> BEGIN
         DBG_TRACE("vcB->vcA: h3 is requested (hoType %d; hoState %d) (mcc %x%x%x, mnc %x%x%x, lac %d; ci %d)\n",
                    hoType_, hoState_, h3GlobalCellId_.mcc[0],h3GlobalCellId_.mcc[1],h3GlobalCellId_.mcc[2],
                    h3GlobalCellId_.mnc[0],h3GlobalCellId_.mnc[1],h3GlobalCellId_.mnc[2],h3GlobalCellId_.lac, h3GlobalCellId_.ci);

         sprintf(&(t_cellLocationRequest.ViperCellId[0]), "%d:%d", h3GlobalCellId_.lac,h3GlobalCellId_.ci);
         //GCELL<tyu:06-01-01> END
      
         t_cellLocationRequest.TxnId = callIndex;

         if (VBLinkLocationRequest(t_cellLocationRequest)
             == false)
         {
             // Ignore the message 
             DBG_ERROR ("vcA->VB Link Error: h3 failed on calling VBLinkLocationRequest()(%d,%d)\n",
                         hoType_, hoState_);
             DBG_LEAVE();
             return (handleAnchHoReqNackWiCos(JC_FAILURE_ANCH_VB_LINK_PROBLEM));
         }

         //RETRY<xxu:04-21-00> BEGIN
         // start the VB timer 
         hoRetry_ = 0;
         parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);
         //RETRY<xxu:04-21-00> END

         //MSC-B' born here while MSC-B is still alive. Mark handover type and relevant state
         //GCELL<tyu:06-01-01>
         h3GlobalCellId_ = vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId;
         hoType_  = HO_TY_HANDMSC3;
         hoState_ = HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ;

     } else 
     {
         //MSC-B' born here while MSC-B is still alive. Mark handover type and relevant state
         hoType_  = HO_TY_HANDMSC3;
         hoState_ = HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ;

         DBG_LEAVE();
         return (handleVcAddressRsp());
     }

     DBG_LEAVE();
     return (CC_MESSAGE_PROCESSING_COMPLT); 
   }

   // ext-HO <chenj:06-29-01>
   JCCEvent_t
   CCAnchorHandover::do_MNET_Handback_To_External_PLMN_scenario(void)
   {
      A_Result_t                           A_result;
      NonStdRasMessagePerformHandoverRQ_t  Msg;
      T_AIF_MessageUnit                    a_msg;

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_MNET_Handback_To_External_PLMN_scenario): entering......\n}\n");
      initHandmsc3Data();

      DBG_TRACEho("   Handback to External THIRD PLMN is requested (hoType:%d, hoState:%d)\n",
                      hoType_, hoState_);

      h3GlobalCellId_ = vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId;

      // Set aifMsg's HANDOVER REQUEST A INTERFACE values
      // JOE: Inter-GP10 handback currently does not give SERVING CELL ID...
      //      the code needs to be enhanced to pass back this value.
      //      For now, use MY_GLOBALCELL_ID as the serving_cell id.
      populateAHandoverRequest( &MY_GLOBALCELL_ID, &h3GlobalCellId_);

      if ((A_result = AIF_Encode( &aifMsg, &a_msg )) != A_RESULT_SUCCESS)
        {
          // Couldn't encode Perform Handover A-Interface msg. 
          DBG_ERRORho ("   A Interface encoding Error: Perform Handver Request. Result (%d)\n",
                           (int) A_result);

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_MNET_Handback_To_External_PLMN_scenario): leaving.\n}\n");
          return (handleAnchHoReqNackWiCos(JC_FAILURE_A_INTERFACE_MESSAGE_ENCODING_FAILED));
        }

      memset( (char*)&Msg, 0, sizeof(NonStdRasMessagePerformHandoverRQ_t));

      Msg.A_HANDOVER_REQUEST.msglength = a_msg.msgLength;
      memcpy(Msg.A_HANDOVER_REQUEST.A_INTERFACE_DATA, 
             (unsigned char*)a_msg.buffer, MAX_A_INTERFACE_LEN);

      Msg.header.origSubId          = callIndex;
      // Note: need to put -1 as hoAgId since this requires GMC to create a new HOA
      Msg.header.destSubId          = -1;              // NULL (unknown hoAgId - GMC will assign)
      Msg.globalCellID              = h3GlobalCellId_;
      Msg.hoCause                   = vcInMsg->msgData.perfHandbackReq.hoCause;
      Msg.hoNumberReqd              = true;            // Always TRUE
      Msg.handoverNumber.ie_present = false;           // NULL (unknown handoverNumber - GMC will assign)
      Msg.mscNumber.ie_present      = false;           // NULL (unknown mscNumber - GMC will find out)

      DBG_TRACEho("   MESSAGE DATA:\n");
      DBG_TRACEho("       : hoAgId_=%d, callId=%d\n",hoAgId_, callIndex);
      DBG_TRACEho("       : h3GlobalCellId_ (type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
                            h3GlobalCellId_.ci_disc,hoGlobalCellId_.mcc[0],h3GlobalCellId_.mcc[1],h3GlobalCellId_.mcc[2],
                            h3GlobalCellId_.mnc[0],h3GlobalCellId_.mnc[1],h3GlobalCellId_.mnc[2],h3GlobalCellId_.lac,hoGlobalCellId_.ci);
      DBG_TRACEho("       : hoCause=SET TO 0 FOR NOW\n");


      DBG_TRACEho("   a_msg.buffer POST-AIF PROCESSING MESSAGE DATA: A_HANDOVER_REQUEST_TYPE\n");
      DBG_TRACEho("      {\n");
      DBG_TRACEho("        \na_msg.buffer hexdump: ");
      DBG_HEXDUMPho((unsigned char*) a_msg.buffer, a_msg.msgLength);
      DBG_TRACEho("\n      }\n");


      DBG_TRACEho("   MSG POST-AIF PROCESSING MESSAGE DATA: A_HANDOVER_REQUEST_TYPE\n");
      DBG_TRACEho("      {\n");
      DBG_TRACEho("        \nMsg hexdump: ");
      DBG_HEXDUMPho((unsigned char*) &Msg, sizeof (NonStdRasMessagePerformHandoverRQ_t) );
      DBG_TRACEho("\n      }\n");


      if (VBLinkPerformHandoverRequest(&Msg)
          == false)
        {
          // Ignore the message 
          DBG_ERRORho ("    CC->VB Link ERROR: Failed on Perform Handver Request Message\n}\n");

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_Handover_To_Third_PLMN_scenario): leaving.\n}\n");
          return (handleAnchHoReqNackWiCos(JC_FAILURE_ANCH_VB_LINK_PROBLEM));
        }

      parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);

      hoType_  = HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3;
      hoState_ = HAND_ST_ANCH_PERF_EHO_H3_REQ;

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_MNET_Handback_To_External_PLMN_scenario): leaving.\n}\n");
   }

