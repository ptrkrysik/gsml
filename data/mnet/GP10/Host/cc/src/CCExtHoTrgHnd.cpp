
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
// File        : CCExtHoTrgHnd.cpp
// Author(s)   : Xiaode Xu
// Create Date : 06-07-01
// Description : External Handover--Handin&PostHandin trg-GP handlers 
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

extern MSG_Q_ID ccMsgQId;

#include "cc/a_handover_common.h"
#include "voip/vblink.h"
#include "voip/exchangeho.h"

#define TICKS_PER_10MS  (sysClkRateGet()) / 100 

extern bool useViperBase; 
extern GlobalCellId_t MY_GLOBALCELL_ID;
extern INT32 forcedHandover;
extern INT32 forcedHandmsc3;
extern GlobalCellId_t ccHoTrgGlobalCellId;
extern UINT32 ccHoTrgCellIpAddress;


extern T_CNI_LAPDM_OID rm_GetOid (T_CNI_IRT_ID entryId, T_CNI_LAPDM_SAPI sapi);

// *******************************************************************
// forward declarations.
// *******************************************************************
int
CCTargetHandoverExtHo::handleHoaCcMsg(InterHoaVcMsg_t *hoaCcInMsg)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleHoaCcMsg", CC_ETRG_LAYER);
  DBG_ENTER();

  CCHandover::handleHoaCcMsg(hoaCcInMsg);
    
  DBG_LEAVE();
  return(smHandleEvent());
}

int
CCTargetHandoverExtHo::smHandleEvent(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::smHandleEvent", CC_ETRG_LAYER);
  DBG_ENTER();

  int returnValue = 0;
    
  if (currEvent != JCC_NULL_EVENT)
  {
      currEvent = 
            (this->*extHoTrgHandler[hoState_][currEvent - HAND_SRC_EVENT_BASE])();
  }
    
  switch(currEvent)
  {
  case CC_RELEASE_CALL:
	   returnValue = ccReleaseCall();
	   break;

  case CC_RELEASE_OWN:
	   returnValue = ccReleaseOwn();
	   break;

  case CC_RELEASE_GP_HO:
	   ccReleaseGpHo();
	   break;

  case CC_RELEASE_GP_HB:
	   ccReleaseGpHb();
	   break;

  case CC_RELEASE_GP_H3:
	   ccReleaseGpH3();
	   break;

  case CC_RELEASE_EH_HB:
	   ccReleaseEhHb();
	   break;
  
  case CC_RELEASE_GP_EH_HB:
	   ccReleaseGpEhHb();
	   break;
    
  case CC_MESSAGE_PROCESSING_COMPLT:
  case JCC_NULL_EVENT:
       // Do Nothing
       break;
    
  default:
       DBG_ERROR("{\nMNETeiTRACE(etgt::smHandleEvent): Got Bad Event (event=%d, hoState=%d, callId=%d)\n}\n",
                    currEvent, hoState_, callIndex);
	   break;
  }
  
  DBG_TRACE("{\nMNETeiTRACE(etrg::smHandleEvent): Event Handled (event=%d, currEvent=%d)\n}\n",
	           currEvent, returnValue);

  DBG_LEAVE();
  return(returnValue);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleAnchToTrgPerfHoReq(void)
{
  int                       i;
  A_Result_t                result;
  bool                      found = false;
  T_CNI_RIL3_SPEECH_VERSION speechVer;
  int                       choosenEncryption;

  DBG_FUNC("CCTargetHandoverExtHo::handleAnchToTrgPerfHoReq", CC_ETRG_LAYER);
  DBG_ENTER();

  //Copy the Anchor information.
  hoAgId_  = hoaInMsg->origSubId;
  hoNumber_ = hoaInMsg->msgData.perfExtHandoverReq.handoverNumber;

  DBG_CALL_TRACE(hoaInMsg->origSubId, "{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHoReq): [<==HOA PERFORM_HANDOVER_REQ] (hoAgId=%d, callId=%d)\n}\n",
	             hoaInMsg->origSubId, hoaInMsg->destSubId);

  //Decode A-interface message
  memset( (char*)&aifMsg, 0, sizeof(T_AIF_MSG));

  if ( A_RESULT_SUCCESS != (result=
	   AIF_Decode((T_AIF_MessageUnit*)&(hoaInMsg->msgData.perfExtHandoverReq.A_HANDOVER_REQUEST), &aifMsg)) )
  {
	   DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchToTrgPerfHoReq): A-MSG decoding err (err=%d, hoAgId_=%d)\nA-MSG:\n",
	                result, hoAgId_);
	   DBG_TRACE("{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHoReq) Received A-HANDOVER_REQ DUMP (length=%d): \n",
                  hoaInMsg->msgData.perfExtHandoverReq.A_HANDOVER_REQUEST.msglength);

       DBG_HEXDUMP(
 		          (unsigned char*)hoaInMsg->msgData.perfExtHandoverReq.A_HANDOVER_REQUEST.A_INTERFACE_DATA,
		          (int)hoaInMsg->msgData.perfExtHandoverReq.A_HANDOVER_REQUEST.msglength
                );
       DBG_TRACE("\n END-OF-DUMP\n}\n");

	   DBG_ERROR("\n}\n");
	   
	   //remember reason
	   hoEndCause_ = MC_HO_UNEXPECTED_DATA_VALUE;  //EHOcause

	   DBG_LEAVE();
	   return (CC_RELEASE_CALL);
  }
  
  DBG_TRACE("{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHoReq) Received A-INTERFACE-MSG DUMP (length=%d): \n",
             hoaInMsg->msgData.perfExtHandoverReq.A_HANDOVER_REQUEST.msglength);
  DBG_HEXDUMP(
 		     (unsigned char*)hoaInMsg->msgData.perfExtHandoverReq.A_HANDOVER_REQUEST.A_INTERFACE_DATA,
		     (int)hoaInMsg->msgData.perfExtHandoverReq.A_HANDOVER_REQUEST.msglength
             );
  DBG_TRACE("\n END-OF-DUMP\n}\n");

  if (aifMsg.msgType != A_HANDOVER_REQUEST_TYPE)
  {
      //Unexpected A-MSG received, ignore but report
	   DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchToTrgPerfHoReq): unexpected A-MSG (msgType=%d,hoState_=%d)\n}\n",
		             aifMsg.msgType, hoState_);
	   DBG_LEAVE();
	   return (CC_RELEASE_CALL);
  }

  //Build a message sent to MM for Handover Req. Only allow voice handover now
  A_Handover_Request_t *pRequest = (A_Handover_Request_t*)&aifMsg.handoverRequest;

  //Check if a good Current Channel IE contained in the A-MESSAGE
  if ( (pRequest->channelType.speechDataIndicator != 1) ||  //1-speech
       (pRequest->channelType.channelRateAndType     != 8) ) //8-TCH F
  {
	   DBG_WARNING("{\nMNETeiWARNING(etrg::handleAnchToTrgPerfHoReq): bad currChannel IE (iePresent=%d, speechDataIndicator=%d, channelRateAndType=%d)\n}\n",
		             pRequest->channelType.ie_present,
	                 pRequest->channelType.speechDataIndicator,
					 pRequest->channelType.channelRateAndType);

       hoEndCause_ = MC_HO_UNEXPECTED_DATA_VALUE; //EHOcause

	   DBG_LEAVE();
	   return (CC_RELEASE_CALL);
  }

////Check if a good Speech Version (used) IE contained in the A-MESSAGE
//if (  !(pRequest->speechVersionUsed.ie_present) ||
//   !( (pRequest->speechVersionUsed.speechVersionId == 0x01) ||    //FR
//         (pRequest->speechVersionUsed.speechVersionId == 0x21) )  )  //EFR
//{
//  DBG_WARNING("{\nMNETeiWARNING(etrg::handleAnchToTrgPerfHoReq): bad speechVersionUsed IE (iePresent=%d, speechVerison=%x)\n}\n",
//             pRequest->speechVersionUsed.ie_present,
//                pRequest->speechVersionUsed.speechVersionId);
//  
//     hoEndCause_ = MC_HO_UNEXPECTED_DATA_VALUE; //EHOcause

//  DBG_LEAVE();
//  return (CC_RELEASE_CALL);
//}

  DBG_TRACE("{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHoReq) MESSAGE DATA:\n");
  DBG_TRACE("       : hoAgId_=%d, callId=%d\n",hoAgId_, callIndex);
  DBG_TRACE("       : hoGlobalCellId_ (type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
	                  hoGlobalCellId_.ci_disc,hoGlobalCellId_.mcc[0],hoGlobalCellId_.mcc[1],hoGlobalCellId_.mcc[2],
		  	          hoGlobalCellId_.mnc[0],hoGlobalCellId_.mnc[1],hoGlobalCellId_.mnc[2],hoGlobalCellId_.lac,hoGlobalCellId_.ci);
  DBG_TRACE("       : hoNumber_ (present=%d,numberType=%d,numberingPlan=%d,numDigits=%d)\n",
	                  hoNumber_.ie_present,hoNumber_.numberType,hoNumber_.numberingPlan,hoNumber_.numDigits);
  DBG_TRACE("                  (digits[1-10]:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x)\n",
		 			            hoNumber_.digits[0],hoNumber_.digits[1],hoNumber_.digits[1],
					            hoNumber_.digits[3],hoNumber_.digits[4],hoNumber_.digits[5],
					            hoNumber_.digits[6],hoNumber_.digits[7],hoNumber_.digits[8],
					            hoNumber_.digits[9],hoNumber_.digits[10]);
  DBG_TRACE("\n	    : Handover Number Hex Dump:\n");
  DBG_HEXDUMP(      (unsigned char*) &hoNumber_,  sizeof(T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER));
  DBG_TRACE("\n");
  DBG_TRACE("       : speechDataIndicator=%d, channelRateAndType=%d, permittedIndicator[0-7]=%x,%x,%x,%x,%x,%x,%x,%x\n",
                      pRequest->channelType.speechDataIndicator,
                      pRequest->channelType.channelRateAndType,
                      pRequest->channelType.permittedIndicator[0],
                      pRequest->channelType.permittedIndicator[1],
                      pRequest->channelType.permittedIndicator[2],
                      pRequest->channelType.permittedIndicator[3],
                      pRequest->channelType.permittedIndicator[4],
                      pRequest->channelType.permittedIndicator[5],
                      pRequest->channelType.permittedIndicator[6],
                      pRequest->channelType.permittedIndicator[7]);

  DBG_TRACE("       : ciphData (ciphAlog=%d, ciphKey[1-8]: %x,%x,%x,%x,%x,%x,%x,%x)\n}\n",
	                  pRequest->encryptionInformation.permittedAlgorithm,
                      pRequest->encryptionInformation.key[0],
	                  pRequest->encryptionInformation.key[1],
	                  pRequest->encryptionInformation.key[2],
	                  pRequest->encryptionInformation.key[3],
	                  pRequest->encryptionInformation.key[4],
                      pRequest->encryptionInformation.key[5],
                      pRequest->encryptionInformation.key[6],
                      pRequest->encryptionInformation.key[7]);

  // Store requested encryption and channel type info for negotiation
  reqEncrypt_     = pRequest->encryptionInformation;
  reqChannelType_ = pRequest->channelType;

  // Negotiate encryption
  choosenEncryption = selectEncryption();

  if (choosenEncryption == -1)
    {
      currEncrypt_.cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
    } 
  else
    {
      currEncrypt_.cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_START_CIPHERING;
      currEncrypt_.cmd.cipherModeSetting.algorithm = (T_CNI_RIL3_CIPHER_ALGORITHM) choosenEncryption;

      memcpy( (unsigned char*) currEncrypt_.Kc, (unsigned char*) pRequest->encryptionInformation.key, 8);
    }

  // Negotiate codec
  speechVer = (T_CNI_RIL3_SPEECH_VERSION) selectChannelType();

  if (speechVer == -1)
    {
       DBG_WARNING("{\nMNETeiWARNING(etrg::handleAnchToTrgPerfHoReq): bad currChannel IE (permittedIndicator[0-7]=%x,%x,%x,%x,%x,%x,%x.%x)\n}\n",
                      pRequest->channelType.permittedIndicator[0],
                      pRequest->channelType.permittedIndicator[1],
                      pRequest->channelType.permittedIndicator[2],
                      pRequest->channelType.permittedIndicator[3],
                      pRequest->channelType.permittedIndicator[4],
                      pRequest->channelType.permittedIndicator[5],
                      pRequest->channelType.permittedIndicator[6],
                      pRequest->channelType.permittedIndicator[7]);

       hoEndCause_ = MC_HO_UNEXPECTED_DATA_VALUE; //EHOcause

       DBG_LEAVE();
       return (CC_RELEASE_CALL);
     }

  currSpeechCh_.version = speechVer;
  currChannel_.bearerType = BEARER_SPEECH;
  currChannel_.speechChann.version = speechVer;
  currClassmark_ = pRequest->classmarkInformation2;

  memcpy( (unsigned char*) &(mmOutMsg.l3_data.handReq.cipherCmd), (unsigned char*) &currEncrypt_,
           sizeof(IntraL3CipherModeCommand_t) );
  mmOutMsg.l3_data.handReq.channelType = currChannel_;
  mmOutMsg.l3_data.handReq.qid = msgQId;

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHoReq): [==>MM MM_EST_REQ] (hoAgId_=%d, callId=%d)\n}\n",
                 hoAgId_, callIndex);

  // Send internal MM msg - 
  sendMM(INTRA_L3_MM_EST_REQ, INTRA_L3_RR_HANDOVER_REQ, parent->entryId, &mmOutMsg);

  hoState_ = HAND_ST_TRG_RCV_HO_REQ;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}


JCCEvent_t 
CCTargetHandoverExtHo::handleTrgHoReqAck(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgHoReqAck", CC_ETRG_LAYER);
  DBG_ENTER();

  T_CNI_L3_ID newId;
  newId.msgq_id =  msgQId;
  newId.sub_id = 0;                 // No sub id needed here

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgHoReqAck): [<==MM MM_EST_CNF] (hoAgId=%d,callIndex=%d,entryId=%d)\n}\n",
	              hoAgId_, callIndex, mmInMsg->entry_id);

  // <Igal 1-6-00> Updated to standard IRT setup
  if (parent->setIrt(mmInMsg->entry_id, newId) == false)
  {
	  DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqAck): setIrt API failed for (entryId=%d)\n}\n",
		         mmInMsg->entry_id); 
      
      hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

      DBG_LEAVE();
      return (CC_RELEASE_CALL);
  }
  
  // GET OID AND SAPI
  T_CNI_RIL3_IRT_ENTRY	irtEntry;
  irtEntry.entry_id = mmInMsg->entry_id;
  
  if  ( (CNI_RIL3_IRT_Get_Entry_by_Entry_ID(&irtEntry))
	     == false )
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqAck): getIrt API failed for entryId=%d)\n}\n",
		         mmInMsg->entry_id);
      
	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

      DBG_LEAVE();
      return (CC_RELEASE_CALL);
  }

  // Setup the entry id, oid, and sapi
  parent->entryId = mmInMsg->entry_id;
  parent->oid     = irtEntry.lapdm_oid;

  //Populate data for encoding A-HANDOVER-ACKNOWLEDGE message
  A_Handover_Request_Ack_t  *pRequestAck = (A_Handover_Request_Ack_t*) &aifMsg.handoverRequestAck;

  memset((char*) &aifMsg,0,sizeof(T_AIF_MSG));

  //Mandatory IEs
  pRequestAck->msgType = A_HANDOVER_REQUEST_ACK_TYPE;
  pRequestAck->layer3Information.ie_present = true;
  pRequestAck->layer3Information.elementIdentifier = 0x17; //layer3Information-IEI
  pRequestAck->layer3Information.length = (unsigned char) mmInMsg->l3_data.handReqAck.handCmd.msgLength;
  memcpy((char*)pRequestAck->layer3Information.layer3Info, 
	     (char*)mmInMsg->l3_data.handReqAck.handCmd.buffer,
		 (int)  mmInMsg->l3_data.handReqAck.handCmd.msgLength);

  //Optional IEs
  pRequestAck->chosenChannel.ie_present = true;
  pRequestAck->chosenChannel.elementIdentifier = 0x21;     //chosen channel-IEI
  pRequestAck->chosenChannel.channelMode = 9;              //speech (full/half)
  pRequestAck->chosenChannel.channel     = 4;              //1-TCH full

  pRequestAck->speechVersionChosen.ie_present = true;
  pRequestAck->speechVersionChosen.elementIdentifier= 0x40;//speechVersionChosen-IEI
  if (currSpeechCh_.version == 0)
      pRequestAck->speechVersionChosen.speechVersionId = 0x01;
  else if (currSpeechCh_.version == 2)
	  pRequestAck->speechVersionChosen.speechVersionId = 0x21;
  else
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqAck): CC Internal error (speechVer=%d)\n\n}\n",
		         currSpeechCh_.version);
      
	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

      DBG_LEAVE();
      return (CC_RELEASE_CALL);
  }
  
  pRequestAck->chosenEncryptAlgorithm.ie_present = true;
  pRequestAck->chosenEncryptAlgorithm.elementIdentifier = 0x2c;
  if (currEncrypt_.cmd.cipherModeSetting.ciphering == CNI_RIl3_CIPHER_NO_CIPHERING)
  {
      pRequestAck->chosenEncryptAlgorithm.algorithmId = 0x01; //no encryption used
  } else if (currEncrypt_.cmd.cipherModeSetting.algorithm == CNI_RIL3_CIPHER_ALGORITHM_A51)
  {
      pRequestAck->chosenEncryptAlgorithm.algorithmId = 0x02; //A51 used
  } else if (currEncrypt_.cmd.cipherModeSetting.algorithm == CNI_RIL3_CIPHER_ALGORITHM_A52)
  {
	  pRequestAck->chosenEncryptAlgorithm.algorithmId = 0x03; //A52 used
  } else
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqAck): CC Internal error (ciphSetting=%d, ciphAlgo=%d)\n\n}\n",
                 currEncrypt_.cmd.cipherModeSetting.ciphering,
		         currEncrypt_.cmd.cipherModeSetting.algorithm);

	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

      DBG_LEAVE();
      return (CC_RELEASE_CALL);	  
  }

  //Encode A-MSG: HANDOVER REQUEST ACKNOWLEDGE
  A_Result_t result;
  NonStdRasMessagePerformHandoverAck_t requestAck;
  
  memset( (char*) &requestAck, 0,  sizeof(NonStdRasMessagePerformHandoverAck_t) );
  if ( A_RESULT_SUCCESS != (result=
                            AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&requestAck.A_INTERFACE_MSG) ) )
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqAck): A-HANDOVER-REQUEST-ACKNOWLEDGE encoding failed (result=%d)\n\n}\n",
		         result);

	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

      DBG_LEAVE();
      return (CC_RELEASE_CALL);	  
  }

  DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgHoReqAck) Received A-HANDOVER-REQ-ACK DUMP (length=%d): \n",
             requestAck.A_INTERFACE_MSG.msglength);
  DBG_HEXDUMP(
 		     (unsigned char*)requestAck.A_INTERFACE_MSG.A_INTERFACE_DATA,
		     (int)           requestAck.A_INTERFACE_MSG.msglength
             );
  DBG_TRACE("\n END-OF-DUMP\n}\n");

  //Send PERFORM-HANDOVER-ACK (positive) to HOA
  requestAck.reason = 0;
  requestAck.header.origSubId = callIndex;
  requestAck.header.destSubId = hoAgId_;

  if (!VBLinkPerformHandoverAck(&requestAck))
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqAck): VBLinkPerformHandoverAck API failed !\n",
		         result);
	  DBG_HEXDUMP(
                (unsigned char*) &requestAck,
				(int) (requestAck.A_INTERFACE_MSG.msglength+6)
				);

	  DBG_ERROR("}\n");

	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

	  DBG_LEAVE();
      return (CC_RELEASE_CALL);	        
  }

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgHoReqAck): [==>HOA PERFORM_HANDOVER_ACK(p)] (hoAgId=%d,callIndex=%d,entryId=%d)\n}\n",
	              hoAgId_, callIndex, mmInMsg->entry_id);

  //start the T204 timer 
  parent->sessionTimer->setTimer(5000);  // CALL_HAND_TRG_T204
  DBG_CALL_TRACE(hoAgId_, "\nMNETeiTRACE(etrg::handleTrgHoReqAck):  [<==CC START TIMER T204] (hoAgId=%d, callId=%d, T204=%d msec)\n}\n",
	              hoAgId_, callIndex, (int)CALL_HAND_TRG_T204/TICKS_PER_10MS);

  hoState_ = HAND_ST_TRG_SND_HO_REQ_ACK;

  // 
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgHoReqNack(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgHoReqNack", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_,"{\nMNETeiTRACE(etrg::handleTrgHoReqNack): [<==MM MM_EST_REJ] (hoAgId=%d,callIndex=%d)\n}\n",
	              hoAgId_, callIndex);

  //Populate data for encoding A-HANDOVER-FAILURE message
  memset(&aifMsg, 0, sizeof(A_Handover_Failure_t));

  A_Handover_Failure_t  *pFailure = (A_Handover_Failure_t*) &aifMsg.handoverFailure;
 
  //Mandatory IEs
  pFailure->msgType = A_HANDOVER_FAILURE_TYPE;
  pFailure->cause.ie_present = true;
  pFailure->cause.elementIdentifier = 0x04; //cause IEI
  pFailure->cause.length = 1;
  pFailure->cause.cause = 0x21; //No radio resource available
  
  //Optional IEs

  //Encode A-MSG: HANDOVER REQUEST ACKNOWLEDGE
  A_Result_t result;
  NonStdRasMessagePerformHandoverAck_t requestAck;
  
  memset( (char*) &requestAck, 0,  sizeof(NonStdRasMessagePerformHandoverAck_t) );
  if ( A_RESULT_SUCCESS != (result=
                            AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&requestAck.A_INTERFACE_MSG) ) )
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqNack): A-HANDOVER-FAILURE encoding failed (result=%d)\n\n}\n",
		         result);

	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

      DBG_LEAVE();
      return (CC_RELEASE_CALL);	  
  }

  //Send PERFORM-HANDOVER-ACK (negative) to HOA
  //requestAck.reason = 1;
  requestAck.header.origSubId = callIndex;
  requestAck.header.destSubId = hoAgId_;

  if (!VBLinkPerformHandoverAck(&requestAck))
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqNack): VBLinkPerformHandoverAck API failed !\n",
		         result);
	  DBG_HEXDUMP(
                (unsigned char*) &requestAck,
				(int) (requestAck.A_INTERFACE_MSG.msglength+6)
				);

	  DBG_ERROR("}\n");

	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

	  DBG_LEAVE();
      return (CC_RELEASE_CALL);	        
  }

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgHoReqNack): [==>HOA PERFORM_HANDOVER_ACK(n)] (hoAgId=%d,callIndex=%d)\n}\n",
	              hoAgId_, callIndex);

  hoEndCause_ = (HandoverCause_t)0;
  DBG_LEAVE();
  return (CC_RELEASE_CALL);	        
}

JCCEvent_t 
CCTargetHandoverExtHo::handleAnchToTrgEndHandover(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleAnchToTrgEndHandover", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoaInMsg->origSubId, "{\nMNETeiTRACE(etrg::handleAnchToTrgEndHandover): [<==HOA END_HANDOVER] (hoAgId=%d,callId=%d,reason=%d)\n}\n",
	             hoaInMsg->origSubId, hoaInMsg->destSubId,
				 hoaInMsg->msgData.extEndHandover.reason);

  parent->sessionTimer->cancelTimer();

  parent->sessionTimer->cancelTimer();

  DBG_LEAVE();
  return(CC_RELEASE_OWN);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgMmRelInd(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgMmRelInd", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgMmRelInd): [<==MM MM_REL_IND (RSL)] (hoAgId=%d,callId=%d,entryId=%d)\n",
	                hoAgId_, callIndex, parent->entryId);

  //Clean up the on-going handover
  hoEndCause_ = MC_HO_RLS_FAILURE;  //EHOcause
  parent->setReleaseCause(CNI_RIL3_CAUSE_DESTINATION_OUT_OF_ORDER);
  	  
  DBG_LEAVE();
  return (CC_RELEASE_CALL);	        
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgHoComplt(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgHoComplt", CC_ETRG_LAYER);
  DBG_ENTER();

  //Populate data for encoding A-HANDOVER-COMPLETE message
  memset(&aifMsg, 0, sizeof(A_Handover_Complete_t));

  A_Handover_Complete_t  *pComplete = (A_Handover_Complete_t*) &aifMsg.handoverComplete;
 
  //Mandatory IEs
  pComplete->msgType = 	A_HANDOVER_COMPLETE_TYPE;
  
  //Optional IEs
  pComplete->rrCause.ie_present = true;
  pComplete->rrCause.elementIdentifier = 0x15;  //cause IEI
  pComplete->rrCause.rrCause = 0;				//normal event
  
  //Encode A-MSG: HANDOVER REQUEST ACKNOWLEDGE
  A_Result_t result;
  NonStdRasMessageHandoverSuccess_t hoSuccess;
  
  memset( (char*) &hoSuccess, 0,  sizeof(NonStdRasMessageHandoverSuccess_t) );
  if ( A_RESULT_SUCCESS != (result=
                            AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&hoSuccess.A_INTERFACE_MSG) ) )
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoComplt): A-HANDOVER-COMPLETE encoding failed (result=%d)\n\n}\n",
		         result);

	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

      DBG_LEAVE();
      return (CC_RELEASE_CALL);	  
  }

  //Send HANDOVER-SUCCESS to HOA
  hoSuccess.header.origSubId = callIndex;
  hoSuccess.header.destSubId = hoAgId_;

  if (!VBLinkHandoverSuccess(&hoSuccess))
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoComplt): VBLinkHandoverSuccess API failed !\n",
		         result);
	  DBG_HEXDUMP(
                (unsigned char*) &hoSuccess,
				(int) (hoSuccess.A_INTERFACE_MSG.msglength+4)
				);

	  DBG_ERROR("}\n");

	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause
	  
	  DBG_LEAVE();
      return (CC_RELEASE_CALL);	        
  }

  //Stop T204 if the voice path is connected too
  if (parent->hoCallVoiceConnected())
  {
      parent->sessionTimer->cancelTimer();
	  
	  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgHoComplt): [==>HOA HANDOVER-COMPLETE both signaling and voice] (hoAgId=%d,callId=%d,entryId=%d)\n",
	              hoAgId_, callIndex, parent->entryId);
	  DBG_CALL_TRACE(hoAgId_,"                                       : [<==CC STOP T204] (hoAgId=%d, callId=%d, T204=%d msec)\n}\n",
                      hoAgId_, callIndex, (int)CALL_HAND_TRG_T204/TICKS_PER_10MS);
  } else
  {
      DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgHoComplt): [==>HOA HANDOVER-COMPLETE signaling only] (hoAgId=%d,callId=%d,entryId=%d)\n}\n",
	                  hoAgId_, callIndex, parent->entryId);
  }

  hoState_ = HAND_ST_TRG_HO_COMPLT;

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);	        
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgHoAccess(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgHoAccess", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgHoAccess): [<==RM HANDOVER-ACCESS] (hoAgId=%d,callId=%d,entryId=%d)\n}\n",
	              hoAgId_, callIndex, parent->entryId);

  //Populate data for encoding A-HANDOVER-COMPLETE message
  memset(&aifMsg, 0, sizeof(A_Handover_Detect_t));

  A_Handover_Detect_t  *pDetect = (A_Handover_Detect_t*) &aifMsg.handoverDetect;
 
  //Mandatory IEs
  pDetect->msgType = A_HANDOVER_DETECT_TYPE;
  
  //Optional IEs
  
  //Encode A-MSG: HANDOVER DETECT
  A_Result_t result;
  NonStdRasMessageHandoverAccess_t hoAccess;
  
  memset( (char*) &hoAccess, 0,  sizeof(NonStdRasMessageHandoverAccess_t) );

  //Send HANDOVER-ACCESS to HOA
  hoAccess.header.origSubId = callIndex;
  hoAccess.header.destSubId = hoAgId_;

  if (!VBLinkHandoverAccess(&hoAccess))
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoAccess): VBLinkHandoverAccess API failed !\n",
		         result);
	  DBG_HEXDUMP(
                (unsigned char*) &hoAccess,
				(int) (hoAccess.A_INTERFACE_MSG.msglength+4)
				);

	  DBG_ERROR("}\n");

	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause
	  
	  DBG_LEAVE();
      return (CC_RELEASE_CALL);	        
  }

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgHoAccess): [==>HOA HANDOVER-DETECT] (hoAgId=%d,callId=%d,entryId=%d)\n}\n",
	              hoAgId_, callIndex, parent->entryId);

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);	        
}

void
CCTargetHandoverExtHo::sendMMMsgToRemoteCC (IntraL3Msg_t *mmMsg)
{ 
  //Send Response to CM SERVICE REQUEST (either CM SERVICE ACCEPT or REJECT)  
    
  DBG_FUNC("CCTargetHandoverExtHo::sendMMMsgToCC", CC_ETRG_LAYER);
  DBG_ENTER();
    
  DBG_TRACE("{\nMNETeiTRACE(etrg::sendMMMsgToRemoteCC): CC Internal Error (hoAgId=%d,callId=%d,hoState=%d,currEvent=%d)\n}\n",
             hoAgId_,callIndex,hoState_,currEvent);
  //vcOutMsg.msgType = INTER_VC_CC_POST_HANDOVER_MOB_EVENT ;
  //vcOutMsg.msgData.postHoMobEventData.intraL3Msg = *mmMsg;
    
  // Source, Destination
  //sendVcMsg (MODULE_CC, MODULE_MM);
    
  DBG_LEAVE();
}

void
CCTargetHandoverExtHo::sendMSMsgToAnchorCC(T_CNI_RIL3MD_CCMM_MSG *msInMsg)
{
  DBG_FUNC("CCTargetHandoverExtHo::sendMSMsgToAnchorCC", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::sendMSMsgToAnchorCC): [<==L2 MS MESSAGE] (hoAgId=%d,callId=%d,entryId=%d)\n",
	                hoAgId_, callIndex, parent->entryId);
  DBG_HEXDUMP((unsigned char*) msInMsg->l3_data.buffer, msInMsg->l3_data.msgLength);
  DBG_CALL_TRACE(hoAgId_, "}\n");

  //Relay the MS message received to HOA
  NonStdRasMessagePostHandoverMobEvent_t l3msg;

  l3msg.header.origSubId = callIndex;
  l3msg.header.destSubId = hoAgId_;

  l3msg.LAYER3PDU.LAYER3DATA[0] = 0x01;  //DTAP 1st byte of Distribution Data Unit (Discrimination)
  l3msg.LAYER3PDU.LAYER3DATA[1] = 0x80;  //DTAP 2nd byte of Distribution Data Unit (FACCH or SDCCH, SAPI0)
  l3msg.LAYER3PDU.LAYER3DATA[2] = (unsigned char) msInMsg->l3_data.msgLength; //length of L3 Msg followed
  memcpy((char*) &l3msg.LAYER3PDU.LAYER3DATA[3], (char*) msInMsg->l3_data.buffer,  //body of L3 Msg
	     (int)msInMsg->l3_data.msgLength);
  l3msg.LAYER3PDU.msglength = (unsigned short) 3 + msInMsg->l3_data.msgLength;

  if (!VBLinkPostHandoverMobEvent(&l3msg))
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::sendMSMsgToAnchorCC): VBLinkPostHandoverMobEvent API failed !\n}\n");
	  DBG_HEXDUMP((unsigned char*) &l3msg, l3msg.LAYER3PDU.msglength+2);
	
	  DBG_ERROR("}\n");

	  hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause
	
	  // send END_HANDOVER and RELEASE-HOA to HOA then cleanup self: radio resource, hoCallLeg, context
      // DBG_LEAVE();
      // return (CC_RELEASE_CALL);	        

	  return;
  }

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::sendMSMsgToAnchorCC): [==>HOA MS MESSAGE] (hoAgId=%d,callId=%d,entryId=%d)\n",   
                  hoAgId_, callIndex);
  DBG_HEXDUMP((unsigned char*) &l3msg, l3msg.LAYER3PDU.msglength+2);
  DBG_CALL_TRACE(hoAgId_, "}\n");

  DBG_LEAVE();
}

    
JCCEvent_t 
CCTargetHandoverExtHo::handleTrgT204TimerExpiry(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgT204TimerExpiry", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgT204TimerExpiry): [<==CC T204 FIRED] (hoAgId=%d,callId=%d,entryId=%d)\n",
	                hoAgId_, callIndex, parent->entryId);

  //Clean up the on-going handover
  hoEndCause_ = MC_HO_T204_TIMEOUT; //EHOcause
  parent->setReleaseCause(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);
  	  
  DBG_LEAVE();
  return (CC_RELEASE_CALL);	        
}
  

BOOL
CCTargetHandoverExtHo::cleanup(BOOL force = FALSE)
{
  DBG_FUNC("CCTargetHandoverExtHo::cleanup", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE((unsigned long)hoAgId_, "{\nMNETeiTRACE(etgt::cleanup): [<==CC T204 FIRED] (hoAgId=%d,callId=%d,entryId=%d)\n",
	                hoAgId_, callIndex, parent->entryId);

  //Send END_HANDOVER, RELEASE_HOA to HOA
  if (hoAgId_ != -1)
  {
      NonStdRasMessageEndHandover_t  endHandover;
      endHandover.header.origSubId = callIndex;
      endHandover.header.destSubId = hoAgId_;
      endHandover.reason           = hoEndCause_;

	  if (!VBLinkEndHandover(&endHandover))
      {
	  	   DBG_ERROR("{\nMNETeiERROR(etrg::cleanup): VBLinkEndHandover API failed\n}\n");
	  }
       
	  DBG_TRACE("{\nMNETeiTRACE(etrg::cleanup): [==>HOA END_HANDOVER (hoAgId=%d, callId=%d, entryId=%d)\n}\n",
		         hoAgId_, callIndex, parent->entryId);

      //NonStdRasMessageReleaseHOA_t relHoa;
	  //relHoa.header.origSubId = callIndex;
	  //relHoa.header.destSubId = hoAgId_;

	  //if (!VBLinkReleaseHOA(&relHoa))
      //{
	  //     DBG_ERROR("{\nMNETeiERROR(etrg::cleanup): VBLinkReleaseHOA API failed\n}\n");
	  //}

  	  //DBG_TRACE("{\nMNETeiTRACE(etrg::cleanup): [==>HOA RELEASE_HANDOVER_AGENT (hoAgId=%d, callId=%d, entryId=%d)\n}\n",
	  //	         hoAgId_, callIndex, parent->entryId);
  }

  initData();

  DBG_LEAVE();
  return(true);

}

CCTargetHandoverExtHo::CCTargetHandoverExtHo(CCSessionHandler *session,
                                   int              callId  ,
                                   MSG_Q_ID         qid     )
                 : CCHandover (session, callId, qid)
{
  voiceConnected      = false;

  CCHandover::initData();
}

int
CCTargetHandoverExtHo::handleTimeoutMsg (IntraL3Msg_t *ccInMsg)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTimeoutMsg", CC_ETRG_LAYER);
  DBG_ENTER();
    
  currEvent = HAND_TRG_TIMER_EXPIRY;
                      
  DBG_LEAVE();
  return(smHandleEvent());    
}
    
int
CCTargetHandoverExtHo::handleRRMsg(IntraL3Msg_t *rrMsg,
                                  JCCEvent_t   hoEvent)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleRRMsg", CC_ETRG_LAYER);
  DBG_ENTER();
    
  CCHandover::handleRRMsg(rrMsg, hoEvent);
    
  DBG_LEAVE();
  return(smHandleEvent());
}
    
JCCEvent_t 
CCTargetHandoverExtHo::handleAnchToTrgPerfHbAck(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleAnchToTrgPerfHbAck", CC_ETRG_LAYER);
  DBG_ENTER();

  //cancel T211 timer
  parent->sessionTimer->cancelTimer();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbAck): [<==HOA PERFORM_HANDBACK_ACK (p)] (callId=%d, entryId=%d)\n}\n",
	             callIndex, parent->entryId);

  //Decode A-interface message
  A_Result_t result;
  memset( (char*)&aifMsg, 0, sizeof(T_AIF_MSG));

  if ( A_RESULT_SUCCESS != (result=
	   AIF_Decode((T_AIF_MessageUnit*)&(hoaInMsg->msgData.perfExtHandbackAck.A_INTERFACE_MSG), &aifMsg)) )
  {
	   DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchToTrgPerfHbAck): A-MSG decoding err (err=%d, hoAgId=%d,callId=%d)\n",
	                result, hoAgId_,callIndex);
	   DBG_ERROR("          A-INTERFACE MESSAGE Hex Dump:\n");
	   DBG_HEXDUMP(
		   (unsigned char*)hoaInMsg->msgData.perfExtHandbackAck.A_INTERFACE_MSG.A_INTERFACE_DATA,
		   (int)hoaInMsg->msgData.perfExtHandbackAck.A_INTERFACE_MSG.msglength
		          );
	   DBG_ERROR("\n}\n");
	   
	   //remember reason
	   hoEndCause_ = JC_FAILURE_A_INTERFACE_MESSAGE_DECODING_FAILED;  //EHOcause

	   DBG_LEAVE();
	   return (CC_RELEASE_EH_HB);
  }
  
  if (aifMsg.msgType != A_HANDOVER_REQUEST_ACK_TYPE)
  {
      //Unexpected A-MSG received, ignore but report
	  DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchToTrgPerfHbAck): unexpected A-MSG (msgType=%d,hoState_=%d)\n}\n",
	             aifMsg.msgType, hoState_);

      hoEndCause_ = JC_FAILURE_A_INTERFACE_MESSAGE_TYPE_INCOMPATIBLE;

	  DBG_LEAVE();
	  return (CC_RELEASE_EH_HB);
  }

  if (!(aifMsg.handoverRequestAck.layer3Information.ie_present))
  {
      //Unexpected A-MSG received, ignore but report
	  DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchToTrgPerfHbAck): A_HANDOVER_ACK without HANDOVER COMMAND (hoAgId=%d,callId=%d,hoState=%d)\n}\n",
	             hoAgId_,callIndex,hoState_);
          hoEndCause_ = JC_FAILURE_A_HANDOVER_ACKNOWLEDGE_WITHOUT_L3_DATA;

	  DBG_LEAVE();
	  return (CC_RELEASE_EH_HB);
  }

  switch(hoState_)
  {
  case HAND_ST_ANCH_SND_HB_REQ:
       //Relay message to GP-B
       vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_ACK;
       vcOutMsg.msgData.perfHandbackAck.handCmd.msgLength =
                                        aifMsg.handoverRequestAck.layer3Information.length;
       memcpy(vcOutMsg.msgData.perfHandbackAck.handCmd.buffer, 
                                        aifMsg.handoverRequestAck.layer3Information.layer3Info,
                                        aifMsg.handoverRequestAck.layer3Information.length);
       sendVcMsg(MODULE_CC, MODULE_CC);

       DBG_CALL_TRACE(hoAgId_,"{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbAck) [==>GP-B PERFORM_HANDBACK_ACK] (callId=%d,entryId=%d)\n}\n",
                      callIndex,parent->entryId);

       hoState_ = HAND_ST_ANCH_RCV_HB_REQ_ACK;

       break;
       
  case HAND_ST_TRG_SND_HB_REQ:
       //Send internal RR msg
       rrOutMsg.l3_data.handCmd.handCmd.msgLength = aifMsg.handoverRequestAck.layer3Information.length;
       memcpy(rrOutMsg.l3_data.handCmd.handCmd.buffer, aifMsg.handoverRequestAck.layer3Information.layer3Info,
              aifMsg.handoverRequestAck.layer3Information.length);
      
       sendRR(INTRA_L3_DATA, INTRA_L3_RR_HANDOVER_COMMAND, parent->entryId, &rrOutMsg);
      
       DBG_CALL_TRACE(hoAgId_,"{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbAck) [==>RM HANDOVER_COMMAND] (callId=%d,entryId=%d)\n}\n",
                      callIndex,parent->entryId);
      
       // start the T204 timer 
       parent->sessionTimer->setTimer(CALL_HAND_TRG_T204);
      
       hoState_ = HAND_ST_TRG_RCV_HB_REQ_ACK;

       break;

  default:
 	   DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchToTrgPerfHbAck): CC Error Inproper hoState(%d) (hoAgId=%d,callId=%d\n}\n",
	              hoState_,hoAgId_,callIndex);

       hoEndCause_ = JC_FAILURE_INPROPER_HANDOVER_STATE;

	   DBG_LEAVE();
	   return (CC_RELEASE_EH_HB);
  }
      
  //
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleAnchToTrgPerfHbNack(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleAnchToTrgPerfHbNack", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbNack): [<==HOA PERFORM_HANDBACK_ACK (n)] (callId=%d, entryId=%d)\n}\n",
	             callIndex, parent->entryId);

  // cancel Tsho timer
  parent->sessionTimer->cancelTimer();

  int i;
  IntraL3HandoverRequired_t *inMsg = &handReqd;

  switch (hoState_)
  {
  case HAND_ST_TRG_SND_HB_REQ:
       if ( (i=getNextHoCandidate()) == (-1) )
       {
            DBG_TRACE("MNETeiTRACE(etrg::handleAnchToTrgPerfHbNack): no more candidate to retrytry (#ofCands=%d)\n",
                       inMsg->numCandidates);
      
            DBG_LEAVE();
            return (CC_RELEASE_EH_HB);
       } else
       {
           //Try next candidate
           candGlobalCellId_t candidate;
           candidate = inMsg->candGlobalCellId[i];
           
           DBG_TRACE("MNETeiTRACE(etrg::handleAnchToTrgPerfHbNack): try next cand cell! %d-th of Cands(%d) candCell(%x,%x%x%x,%x%x%x,%x,%x) hoState(%d)\n",
                      i,inMsg->numCandidates, 
                      candidate.candGlobalCellId.mcc[0],candidate.candGlobalCellId.mcc[1],
		      candidate.candGlobalCellId.mcc[2],
		      candidate.candGlobalCellId.mnc[0],candidate.candGlobalCellId.mnc[1],
		      candidate.candGlobalCellId.mnc[2],
                      candidate.candGlobalCellId.lac, candidate.candGlobalCellId.ci, hoState_);
            
           if (candidate.candIsExternal)
           {
               //Handback from anchor GP to PLMN case
               populateAHandoverRequest( &MY_GLOBALCELL_ID, &candidate.candGlobalCellId);
                
               A_Result_t result;
               NonStdRasMessagePerformHandbackRQ_t handbackRQ; 
                
               memset( (char*) &handbackRQ, 0,  sizeof(NonStdRasMessagePerformHandbackRQ_t) );
                
               if ( A_RESULT_SUCCESS != (result=
                    AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&handbackRQ.A_HANDOVER_REQUEST) ) )
               {
                    DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchToTrgPerfHbNack): A-HANDOVER-REQUEST encoding failed (result=%d)\n\n}\n",
                               result);
                                                           
                    DBG_LEAVE();
                    return (CC_RELEASE_EH_HB);
               }
                
               //Send PERFORM-HANDBACK-REQ to HOA
               handbackRQ.hoCause          = inMsg->hoCause;
               handbackRQ.header.origSubId = callIndex;
               handbackRQ.header.destSubId = hoAgId_;
               handbackRQ.globalCellID     = candidate.candGlobalCellId;
               handbackRQ.externalCell     = true;
                
               if (!VBLinkPerformHandback(&handbackRQ))
               {
                   DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchToTrgPerfHbNack): VBLinkPerformHandoverAck API failed !\n}\n");
                    
                   DBG_HEXDUMP( (unsigned char*) &handbackRQ,
                                (int) (handbackRQ.A_HANDOVER_REQUEST.msglength+26)
                              );
                    
                   DBG_ERROR("}\n");
                                                          
                   DBG_LEAVE();
                   return (CC_RELEASE_EH_HB);            
               }
                
                DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbNack): [==>HOA PERFORM_HANDBACK_REQ](callId=%d,entryId=%d)\n}\n",
                               callIndex, parent->entryId);
                
                DBG_TRACE("{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbNack) PERFORM_HANDBACK_REQ MESSAGE DATA:\n");
                DBG_TRACE("       : hoAgId_=%d, callId=%d\n",hoAgId_, callIndex);
                DBG_TRACE("       : target cell(type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
                            handbackRQ.globalCellID.ci_disc,
                            handbackRQ.globalCellID.mcc[0],handbackRQ.globalCellID.mcc[1],handbackRQ.globalCellID.mcc[2],
                            handbackRQ.globalCellID.mnc[0],handbackRQ.globalCellID.mnc[1],handbackRQ.globalCellID.mnc[2],
                            handbackRQ.globalCellID.lac,handbackRQ.globalCellID.ci);
                
                DBG_TRACE("    Encoded A_HANDOVER_REQ HEX DUMP:\n");
                DBG_HEXDUMP((unsigned char*) &handbackRQ, sizeof (NonStdRasMessagePerformHandoverRQ_t) );
                DBG_TRACE("      \n}\n");


                DBG_TRACE("{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbNack) Received A-INTERFACE-MSG DUMP (length=%d): \n",
                            handbackRQ.A_HANDOVER_REQUEST.msglength);
                DBG_HEXDUMP(
 		                    (unsigned char*)handbackRQ.A_HANDOVER_REQUEST.A_INTERFACE_DATA,
		                    (int)handbackRQ.A_HANDOVER_REQUEST.msglength
                           );
                DBG_TRACE("\n END-OF-DUMP\n}\n");

                parent->sessionTimer->setTimer(CALL_HAND_TRG_T211);
                
                DBG_CALL_TRACE(hoAgId_, "\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbNack):  [<==CC START TIMER T211] (callId=%d, T211=%d ticks)\n}\n",
                               callIndex, (int)CALL_HAND_TRG_T211);
                
                hoState_ = HAND_ST_TRG_SND_HB_REQ;
                
            } else
            {
                //Handover from anchor GP to another GP case
                sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", 
                        candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
                
                cellLocationRequest.TxnId = callIndex;
                
                if (false == VBLinkLocationRequest(cellLocationRequest) )
                {
                    // Ignore the message 
                    DBG_ERROR("{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbNack): VBLinkLocationRequest failed!\n");
                    DBG_ERROR("                                      (hoAgId=%d callId=%d entryId=%d,lac=%d,ci=%d)\n}\n",
                                hoAgId_, callIndex, parent->entryId, 
                                candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
                    
                    DBG_LEAVE();
                    return (CC_RELEASE_EH_HB);
                }
                
                DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbNack): [==>HOA IP_ADDRESS_REQ](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                               callIndex, parent->entryId,
                               candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
                
                DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbNack): [<==CC START TIMER Tvb](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                               callIndex, parent->entryId,
                               candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
                
                parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);
                    
                hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;
            }
            
           DBG_LEAVE();
           return (CC_MESSAGE_PROCESSING_COMPLT);
       }

       break;

  case HAND_ST_ANCH_SND_HB_REQ:
       // Abort HB part at GP-B
       vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
       vcOutMsg.msgData.perfHandbackNack.cause = JC_FAILURE_A_HANDOVER_FAILURE;
      
       sendVcMsg(MODULE_CC, MODULE_CC);

       DBG_CALL_TRACE(hoAgId_,"{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHbAck) [==>GP-B PERFORM_HANDBACK_NACK] (callId=%d,hoState=%d)\n}\n",
                      callIndex,hoState_);

       DBG_LEAVE();
       return(CC_RELEASE_GP_HB);

       break;

  default:
 	   DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchToTrgPerfHbNack): CC Error Inproper hoState(%d) (hoAgId=%d,callId=%d\n}\n",
	              hoState_,hoAgId_,callIndex);

       hoEndCause_ = JC_FAILURE_INPROPER_HANDOVER_STATE;

	   DBG_LEAVE();
	   return (JCC_NULL_EVENT);
  }

  //
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgT211TimerExpiry(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgT211TimerExpiry", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgT211TimerExpiry): [<==CC TIMER T211 FIRED](callId=%d,entryId=%d)\n}\n",
         callIndex, parent->entryId);

  hoEndCause_ = JC_FAILURE_T211_TIMEOUT;

  DBG_LEAVE();
  return (CC_RELEASE_EH_HB);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgHoReqd(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgHoReqd", CC_ETRG_LAYER);
  DBG_ENTER();

  int i;

  IntraL3HandoverRequired_t *inMsg = &rrInMsg->l3_data.handReqd;
  candGlobalCellId_t candidate;

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgHoReqd): [<==RM HANDOVER REQUIRED] (callId=%d, entryId=%d, #Cands=%d)\n}\n",
                 callIndex, parent->entryId, inMsg->numCandidates);

  candidate.candIsExternal   = false;
  candidate.candIsTried      = true; 
  candidate.candGlobalCellId = hoGlobalCellId_;

  handReqd = *inMsg;

  if (!forcedHandover)
  {
      if ((inMsg->numCandidates) <= 0)
      {
          // Ignore the message 
          DBG_ERROR ("{\nMNETeiERROR(etrg::handleTrgHoReqd): Handover Required Error (bad numCandidates = %d)\n}\n", 
                      inMsg->numCandidates);
          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);
      }

      if ( (i=getNextHoCandidate()) == (-1) ) 
      {
          DBG_ERROR ("{\nMNETeiERROR(etrg::handleTrgHoReqd): Handover Required Error (all bad #candidates = %d)\n}\n",
                      inMsg->numCandidates);
          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);
      }

      candIndex_ = i;
      candidate = inMsg->candGlobalCellId[i];

      DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgHoReqd): Selected %dth of Candidates(%d) (mcc:%x%x%x mnc:%x%x%x lac:%d ci:%d) (hoState %d)\n}\n",
                 i, inMsg->numCandidates, 
                 candidate.candGlobalCellId.mcc[0],candidate.candGlobalCellId.mcc[1],
		 candidate.candGlobalCellId.mcc[2],
                 candidate.candGlobalCellId.mnc[0],candidate.candGlobalCellId.mnc[1],
		 candidate.candGlobalCellId.mnc[2],
                 candidate.candGlobalCellId.lac, candidate.candGlobalCellId.ci, hoState_);
  }

  if (candidate.candIsExternal)
  {
      //Handback from anchor GP to PLMN
      populateAHandoverRequest(&MY_GLOBALCELL_ID, &candidate.candGlobalCellId);

      A_Result_t result;
      NonStdRasMessagePerformHandbackRQ_t handbackRQ; 

      memset( (char*) &handbackRQ, 0,  sizeof(NonStdRasMessagePerformHandbackRQ_t) );

      if ( A_RESULT_SUCCESS != (result=
           AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&handbackRQ.A_HANDOVER_REQUEST) ) )
      {
           DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqd): A-HANDOVER-REQUEST encoding failed (result=%d)\n\n}\n",
                     result);

           DBG_LEAVE();
           return (CC_MESSAGE_PROCESSING_COMPLT);
      }

      //Send PERFORM-HANDBACK-REQ to HOA
      handbackRQ.hoCause          = inMsg->hoCause;
      handbackRQ.header.origSubId = callIndex;
      handbackRQ.header.destSubId = hoAgId_;
      handbackRQ.globalCellID     = candidate.candGlobalCellId;
      handbackRQ.externalCell     = true;

      if (!VBLinkPerformHandback(&handbackRQ))
      {
          DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoReqd): VBLinkPerformHandoverAck API failed !\n}\n");
               
          DBG_HEXDUMP( (unsigned char*) &handbackRQ,
                       (int) (handbackRQ.A_HANDOVER_REQUEST.msglength+26)
                     );

          DBG_ERROR("}\n");
         
          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);            
      }

      DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoReqd): [==>HOA PERFORM_HANDBACK_REQ](callId=%d,entryId=%d)\n}\n",
                     callIndex, parent->entryId);
                           
      DBG_TRACE("{\nMNETeiTRACE(etrg::handleAnchHoReqd) PERFORM_HANDBACK_REQ MESSAGE DATA:\n");
      DBG_TRACE("       : hoAgId_=%d, callId=%d\n",hoAgId_, callIndex);
      DBG_TRACE("       : target cell(type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
                          handbackRQ.globalCellID.ci_disc,
                          handbackRQ.globalCellID.mcc[0],handbackRQ.globalCellID.mcc[1],handbackRQ.globalCellID.mcc[2],
                          handbackRQ.globalCellID.mnc[0],handbackRQ.globalCellID.mnc[1],handbackRQ.globalCellID.mnc[2],
                          handbackRQ.globalCellID.lac,handbackRQ.globalCellID.ci);

      DBG_TRACE("    Encoded A_HANDOVER_REQ HEX DUMP:\n");
      DBG_HEXDUMP((unsigned char*) &handbackRQ, sizeof (NonStdRasMessagePerformHandoverRQ_t) );
      DBG_TRACE("      \n}\n");


      DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgHoReqAck) Received A-INTERFACE-MSG DUMP (length=%d): \n",
                 handbackRQ.A_HANDOVER_REQUEST.msglength);
      DBG_HEXDUMP(
                 (unsigned char*)handbackRQ.A_HANDOVER_REQUEST.A_INTERFACE_DATA,
                 (int)handbackRQ.A_HANDOVER_REQUEST.msglength
                );
      DBG_TRACE("\n END-OF-DUMP\n}\n");

      parent->sessionTimer->setTimer(CALL_HAND_TRG_T211);
      DBG_CALL_TRACE(hoAgId_, "\nMNETeiTRACE(etrg::handleTrgHoReqAck):  [<==CC START TIMER T211] (callId=%d, T211=%d msec)\n}\n",
                     callIndex, (int)CALL_HAND_TRG_T211);

      hoState_ = HAND_ST_TRG_SND_HB_REQ;

  } else
  {
      //Handover from anchor GP to another GP case
      sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", 
              candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);

      cellLocationRequest.TxnId = callIndex;
    
      if (false == VBLinkLocationRequest(cellLocationRequest) )
      {
          // Ignore the message 
          DBG_ERROR("{\nMNETeiTRACE(etrg::handleAnchHoReqd): VBLinkLocationRequest failed!\n");
          DBG_ERROR("                                      (hoAgId=%d callId=%d entryId=%d,lac=%d,ci=%d)\n}\n",
                     hoAgId_, callIndex, parent->entryId, 
                     candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
          
          DBG_LEAVE();
          return (CC_MESSAGE_PROCESSING_COMPLT);
      }

      DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoReqd): [==>HOA IP_ADDRESS_REQ](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                     callIndex, parent->entryId,
                     candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);

      DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoReqd): [<==CC START TIMER Tvb](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                     callIndex, parent->entryId,
                     candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);

      parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);

                     
      hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;
  }
  
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}


JCCEvent_t 
CCTargetHandoverExtHo::handleTrgToAnchHoAccess(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgToAnchHoAccess", CC_ETRG_LAYER);
  DBG_ENTER();
    
  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchHoAccess): [<==GP-B HANDOVER_ACCESS] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                 callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);
    
  //Send up bss APDU to PLMN (anchor part)
  NonStdRasMessagePostHandoverMobEvent_t bssApdu;
  
  bssApdu.header.origSubId = callIndex;
  bssApdu.header.destSubId = hoAgId_;
  
  bssApdu.LAYER3PDU.LAYER3DATA[0] = 0x00;  //BSSMAP
  bssApdu.LAYER3PDU.LAYER3DATA[1] = 1;     
  bssApdu.LAYER3PDU.LAYER3DATA[2] = A_HANDOVER_DETECT_TYPE;     
  
  bssApdu.LAYER3PDU.msglength = 3;
  
  if (!VBLinkPostHandoverMobEvent(&bssApdu))
  {
	  DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgToAnchHoAccess): VBLinkPostHandoverMobEvent API failed !\n}\n");
	  DBG_HEXDUMP((unsigned char*) &bssApdu, bssApdu.LAYER3PDU.msglength+2);
	  
	  DBG_ERROR("}\n");
  } else
  {
      DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgHoFail): [==>HOA A-HANDOVER-DETECT] (callId=%d,hoState=%d)\n",   
				     callIndex,hoState_);
      DBG_HEXDUMP((unsigned char*) &bssApdu, bssApdu.LAYER3PDU.msglength+2);
      DBG_CALL_TRACE(hoAgId_, "}\n");
  }
  
  DBG_LEAVE();
  return(CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleAnchHoReqNack(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleAnchHoReqNack", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_,"{\nMNETeiTRACE(etrg::handleAnchHoReqNack): [<==RR MM_EST_REJ(hoReqNack)](callId=%d,hoState=%d)\n}\n",
				 callIndex, hoState_);
  
  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
  vcOutMsg.msgData.perfHandbackNack.cause  = rrInMsg->l3_data.handReqNack.cause;
  
  sendVcMsg(MODULE_CC, MODULE_CC);
  
    //
  DBG_LEAVE();
  return (CC_RELEASE_GP_HB);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleAnchHoReqAck(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleAnchHoReqAck", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoReqAck): [<==RR MM_EST_CNF(hoReqAck)](callId=%d,hoState=%d)\n}\n",
                 callIndex, hoState_);


  T_CNI_L3_ID newId;
  newId.msgq_id =  msgQId;
  newId.sub_id = 0;
  
  if (parent->setIrt (mmInMsg->entry_id, newId) == false)
  {
	  DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchHoReqAck) CC Call Error IRT SET problem for (entryId=%d)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
				  mmInMsg->entry_id,hoAgId_,callIndex,hoState_);

      DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoReqAck): [==>GP-B PERFORM_HANDBACK_NACK](callId=%d,entryId=%d,hoState=%d)\n}\n",
					 callIndex, mmInMsg->entry_id, hoState_);

	  // Treat it like a Ho. Req. Nack.
	  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
	  vcOutMsg.msgData.perfHandbackNack.cause  = JC_FAILURE_IRT_TABLE_SET_FAILED;
	  
	  sendVcMsg(MODULE_CC, MODULE_CC);
	  
	  DBG_LEAVE();
	  return   (CC_RELEASE_GP_HB);
  }
  
  // Setup the entry id, oid, and sapi
  parent->entryId = mmInMsg->entry_id;
  
  // GET OID AND SAPI
  T_CNI_RIL3_IRT_ENTRY	irtEntry;
  irtEntry.entry_id = mmInMsg->entry_id;
  
  if  ( (CNI_RIL3_IRT_Get_Entry_by_Entry_ID(&irtEntry))	== false )
  {
	  DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchHoReqAck) CC Call Error IRT GET problem for (entryId=%d)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
				  mmInMsg->entry_id,hoAgId_,callIndex,hoState_);
	  
	  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoReqAck): [==>GP-B PERFORM_HANDBACK_NACK](callId=%d,entryId=%d,hoState=%d)\n}\n",
					 callIndex, mmInMsg->entry_id, hoState_);
	  
	  // Treat it like a Ho. Req. Nack.
	  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
	  vcOutMsg.msgData.perfHandbackNack.cause  = JC_FAILURE_IRT_TABLE_GET_FAILED;
	  
	  sendVcMsg(MODULE_CC, MODULE_CC);
	  
	  DBG_LEAVE();
	  return (CC_RELEASE_GP_HB);
  }
  
  parent->oid = irtEntry.lapdm_oid;
  
  // Make a simplex connection between the remote rtp channel and the rf channel
  T_CSU_PORT_ID rfSinkPort, rtpSourcePort;
  T_CSU_RESULT_CON csuResult;
  
  rfSinkPort.portType = CSU_GSM_PORT;
  rtpSourcePort.portType = CSU_RTP_PORT;
  
  rfSinkPort.portId.gsmHandler = parent->entryId;
  rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(parent->hoCallLeg->h323CallHandle);
  
  if ((csuResult = csu_SimplexConnect(&rtpSourcePort, &rfSinkPort))
	  != CSU_RESULT_CON_SUCCESS)
  {
	  DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchHoReqAck) CSU UNI-CONNECT failed (entryId=%d,h323Hndl=%p)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
				 mmInMsg->entry_id,parent->hoCallLeg->h323CallHandle, hoAgId_,callIndex,hoState_);
	  
	  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoReqAck): [==>GP-B PERFORM_HANDBACK_NACK](callId=%d,entryId=%d,hoState=%d)\n}\n",
					 callIndex, mmInMsg->entry_id, hoState_);
	  
	  // Treat it like a Ho. Req. Nack.
	  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
	  vcOutMsg.msgData.perfHandbackNack.cause  = JC_FAILURE_CSU_UNI_CONNECT_FAILED;
	  
	  sendVcMsg(MODULE_CC, MODULE_CC);
	  
	  DBG_LEAVE();
	  return   (CC_RELEASE_GP_HB);
  }
  
  hoSimplexConnected_ = true;
  
  //Send the Perform Ack message.
  
  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_ACK;
  vcOutMsg.msgData.perfHandbackAck.handCmd  = mmInMsg->l3_data.handReqAck.handCmd;
 
  sendVcMsg(MODULE_CC, MODULE_CC);
 
  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoReqAck): [==>GP-B PERFORM_HANDBACK_ACK](callId=%d,entryId=%d,hoState=%d)\n}\n",
                 callIndex, mmInMsg->entry_id, hoState_);

  // start the T202 timer 
  parent->sessionTimer->setTimer(CALL_HAND_TRG_T202);  //ext-HO FFS-TIMER
 
  hoState_ = HAND_ST_ANCH_SND_HB_REQ_ACK;
			 
  // 
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleAnchHoComplt(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleAnchHoComplt", CC_ETRG_LAYER);
  DBG_ENTER();
    
  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoComplt): [<==RM HANDOVER_COMPLETE] (callIndex=%d,entryId=%d,hoState=%d)\n}\n",
      		     callIndex, parent->entryId, hoState_);
	
  //Send the Complete Handback message.
  vcOutMsg.msgType = INTER_VC_CC_COMPLETE_HANDBACK;
  sendVcMsg(MODULE_CC, MODULE_CC);


  //Make voice connection
  T_CSU_PORT_ID oldPort, srcPort, snkPort;
  T_CSU_RESULT_CON csuResult;

  oldPort.portType = CSU_RTP_PORT;
  oldPort.portId.rtpHandler = hoRtpSession_;
  
  if (hoRtpSession_ != NULL)
  {
      if ( (csuResult = csu_OrigTermBreakAll(&oldPort)) != CSU_RESULT_CON_SUCCESS)
      {
            DBG_WARNING("{\nMNETeiWARNING(etrg::handleAnchHoComplt) CSU BreakAll Error(%d) for oldPort(%d, %x)\n}\n",  
                         csuResult, oldPort.portType, oldPort.portId.rtpHandler);

            //Ignore this warning to allow call going along
      }

      ReleaseHandoverRtpChannel(hoRtpSession_);

      hoRtpSession_ = NULL;
  }

  srcPort.portType = CSU_RTP_PORT;
  srcPort.portId.rtpHandler = VoipCallGetRTPHandle(parent->hoCallLeg->h323CallHandle);

  snkPort.portType = CSU_GSM_PORT;
  snkPort.portId.gsmHandler = parent->entryId;
  if ((csuResult = csu_DuplexConnect(&srcPort, &snkPort)) != CSU_RESULT_CON_SUCCESS)
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchHoComplt) CSU 2-WAY Conn Error (%d) SrcPort(%d:%p)<->SnkPort(%d:%p)\n}\n",  
                 csuResult, srcPort.portType, srcPort.portId.rtpHandler, snkPort.portType, snkPort.portId.rtpHandler);
      
      DBG_LEAVE();
      return (CC_RELEASE_CALL);
  }
   
  //Empty the queues now. Do this only at the end.
  emptyRRQueue      ();
  emptyDownlinkQueue();  
  emptyMMQueue      ();
    
  //
  DBG_LEAVE();
  return (CC_RELEASE_GP_HO);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleAnchPostHoRls(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleAnchPostHoRls", CC_ETRG_LAYER);
  DBG_ENTER();
    
  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchPostHoRls): [<==GP-B POST_HANDOVER_RLS] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                 callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);
 
  // cancel any timers running
  parent->sessionTimer->cancelTimer();

  hoEndCause_ = MC_HO_RLS_FAILURE;
  
  DBG_LEAVE();
  return (CC_RELEASE_CALL);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgToAnchPostHoMobEvent", CC_ETRG_LAYER);
  DBG_ENTER();
    
  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPostHoMobEvent): [<==GP-B POST_HANDOVER_EVENT] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                 callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);
    
  //PostL3RecvMsLog(msInMsg->lapdm_oid, msInMsg->sapi, &msInMsg->l3_data);  
    
  switch (vcInMsg->origModuleId)
  {
  case MODULE_RM:
      //Should not happen: ignore RR message but watch out for analysis when it really occurs.
      DBG_WARNING("{\nMNETeiTRACE(etrg::handleAnchPostHoMobEvent): [<==GP-B POST_HANDOVER_EVENT (RR Message)] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                   hoAgId_, callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);

      break;
      
  case MODULE_MD:
      //Pass up to PLMN (anchor)
      sendMSMsgToAnchorCC(&vcInMsg->msgData.postHoMobEventData.msInMsg);
      break;
      
  default:
      // late event
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgToAnchPostHoMobEvent) CC Error Invalid OrigModuleId(%d) (hoAgId=%d,callId=%d,hoState=%d, reIpAddr=%x, reCallId=%d)\n}\n",
                 vcInMsg->origModuleId,  hoAgId_, callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);     
  }
  
  DBG_LEAVE();
  return(CC_MESSAGE_PROCESSING_COMPLT);
  
}


JCCEvent_t 
CCTargetHandoverExtHo::handleTrgToAnchPerfHbReq(void)
{
    DBG_FUNC("CCTargetHandoverExtHo::handleTrgToAnchPerfHbReq", CC_ETRG_LAYER);
    DBG_ENTER();
    
    DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHbReq): [<==GP-B PERFORM_SUBSEQUENT_HO_REQ] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d cand(MCC:%x%x% MNC:%x%x%x LAC:%x CI:%x)\n}\n",
                   callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId,
                   vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mcc[2],
                   vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mcc[1],
                   vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mcc[0],
                   vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mnc[2],
                   vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mnc[1],
                   vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.mnc[0],
                   vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.lac,
                   vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId.ci);
    
    // Use the same criteria as when triggered on the anchor side.
    if (!parent->hoCallVoiceConnected())
    {
        // Need to fix the cause value for the nack.
        DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHbReq): HB disallowed due to improper hoCallState(%d) (hoAgId=%d,callId=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                    parent->hoCallState(),hoAgId_,callIndex, vcInMsg->origVcAddress, vcInMsg->origSubId);

        DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHbReq): [==>GP-B PERFORM_SUBSEQUENT_HO_REQ] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                    callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);
         
        vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
        vcOutMsg.msgData.perfHandbackNack.cause  = JC_FAILURE_HANDBACK_NOT_ALLOWED;
    
        sendVcMsg(MODULE_CC, MODULE_CC);
    
        return(CC_MESSAGE_PROCESSING_COMPLT);
    }
    
    if ( isEqualGlobalCellId(&(vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId), &MY_GLOBALCELL_ID) &&
         !forcedHandmsc3 )
    { 
        //HB->anchor GP scenario
        DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHbReq): [<==GP-B HANDBACK to GP-A] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                       callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);     

        // Build MM_EST_REQ for sake of the handback  <ext-HO FFS-CIPH>
        memcpy( (unsigned char*) &(mmOutMsg.l3_data.handReq.cipherCmd),
                (unsigned char*) &currEncrypt_, sizeof(IntraL3CipherModeCommand_t));
        mmOutMsg.l3_data.handReq.channelType = currChannel_;
        mmOutMsg.l3_data.handReq.qid = msgQId;
        
        // Send internal MM msg - 
        sendMM(INTRA_L3_MM_EST_REQ, INTRA_L3_RR_HANDOVER_REQ, parent->entryId, &mmOutMsg);

        DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchToTrgPerfHoReq): [==>MM MM_EST_REQ] (hoAgId_=%d,callId=%d,speech%d:%d,ciph:%d:%d,Kc:%x%x%x%x)\n}\n",
                       hoAgId_, callIndex,
                       vcOutMsg.msgData.perfHandoverReq.channelType.bearerType,
                       vcOutMsg.msgData.perfHandoverReq.channelType.speechChann.version,
                       currEncrypt_.cmd.cipherModeSetting.ciphering,
                       currEncrypt_.cmd.cipherModeSetting.algorithm,
                       currEncrypt_.Kc[0],currEncrypt_.Kc[1],
                       currEncrypt_.Kc[2],currEncrypt_.Kc[3]);
        
		parent->sessionTimer->setTimer(CALL_HAND_TRG_T202);

        hoState_ = HAND_ST_ANCH_RCV_HB_REQ;
        return (CC_MESSAGE_PROCESSING_COMPLT);
        
    } else
    {
        VBLINK_API_CELL_LOCATION_REQUEST_MSG t_cellLocationRequest;
            
        if (forcedHandmsc3)
        {
            h3GlobalCellId_ = ccHoTrgGlobalCellId;
            h3VcIpAddress_  = ccHoTrgCellIpAddress; 
        } else
        {
            h3GlobalCellId_ = vcInMsg->msgData.perfHandbackReq.globalCellId.candGlobalCellId;

        }
        
        candGlobalCellId_t candidate;
        candidate = vcInMsg->msgData.perfHandbackReq.globalCellId;

        if (candidate.candIsExternal)
        {
            //Handback from non-anchor GP to PLMN 
            populateAHandoverRequest(&MY_GLOBALCELL_ID, &candidate.candGlobalCellId);
            
            A_Result_t result;
            NonStdRasMessagePerformHandbackRQ_t handbackRQ; 
            
            memset( (char*) &handbackRQ, 0,  sizeof(NonStdRasMessagePerformHandbackRQ_t) );

            if ( A_RESULT_SUCCESS != (result=
                 AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&handbackRQ.A_HANDOVER_REQUEST) ) )
            {
                DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgToAnchPerfHbReq): A-HANDOVER-REQUEST encoding error(%d)(hoAgId=%d,callIndex=%d)\n\n}\n",
                           result,hoAgId_,callIndex);
                
                hoEndCause_ = JC_FAILURE_A_INTERFACE_MESSAGE_ENCODING_FAILED;

                DBG_LEAVE();
                return (CC_RELEASE_EH_HB);
            }
            
            //Send PERFORM-HANDBACK-REQ to HOA
            handbackRQ.hoCause          = vcInMsg->msgData.perfHandbackReq.hoCause;
            handbackRQ.header.origSubId = callIndex;
            handbackRQ.header.destSubId = hoAgId_;
            handbackRQ.globalCellID     = h3GlobalCellId_;
            handbackRQ.externalCell     = true;
            
            if (!VBLinkPerformHandback(&handbackRQ))
            {
                DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgToAnchPerfHbReq): VBLinkPerformHandoverAck() failed! (hoAgId=%d,callId=%d,hoState=%d)\n}\n",
                           hoAgId_,callIndex,hoState_);

                DBG_HEXDUMP( (unsigned char*) &handbackRQ,
                    (int) (handbackRQ.A_HANDOVER_REQUEST.msglength+26)
                    );
                
                DBG_ERROR("}\n");

                hoEndCause_ = JC_FAILURE_A_INTERFACE_MESSAGE_ENCODING_FAILED;

                DBG_LEAVE();
                return (CC_RELEASE_EH_HB);
            }
            
            DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHbReq): [==>HOA PERFORM_HANDBACK_REQ](callId=%d,entryId=%d)\n}\n",
                           callIndex, parent->entryId);
            
            DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHbReq) PERFORM_HANDBACK_REQ MESSAGE DATA:\n");
            DBG_TRACE("       : hoAgId_=%d, callId=%d\n",hoAgId_, callIndex);
            DBG_TRACE("       : target cell(type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
                        handbackRQ.globalCellID.ci_disc,
                        handbackRQ.globalCellID.mcc[0],handbackRQ.globalCellID.mcc[1],handbackRQ.globalCellID.mcc[2],
                        handbackRQ.globalCellID.mnc[0],handbackRQ.globalCellID.mnc[1],handbackRQ.globalCellID.mnc[2],
                        handbackRQ.globalCellID.lac,handbackRQ.globalCellID.ci);
            
            DBG_TRACE("    Encoded A_HANDOVER_REQ HEX DUMP:\n");
            DBG_HEXDUMP((unsigned char*) &handbackRQ, sizeof (NonStdRasMessagePerformHandoverRQ_t) );
            DBG_TRACE("      \n}\n");
                                  
            DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHbReq) Received A-INTERFACE-MSG DUMP (length=%d): \n",
                         handbackRQ.A_HANDOVER_REQUEST.msglength);
            DBG_HEXDUMP(
                         (unsigned char*)handbackRQ.A_HANDOVER_REQUEST.A_INTERFACE_DATA,
                         (int)handbackRQ.A_HANDOVER_REQUEST.msglength
                        );
            DBG_TRACE("\n END-OF-DUMP\n}\n");

			parent->sessionTimer->setTimer(CALL_HAND_TRG_T211);
            hoState_ = HAND_ST_ANCH_SND_HB_REQ;
            
        } else
        {
            //Handover to 3rd GP10 
            if ( (!forcedHandmsc3) || (forcedHandmsc3&&useViperBase) )
            {
                //H3 scenario. Go ahead to inquire IP address of mscB'
                DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHbReq): [<==GP-B HANDTO3rd to GP-B'] (callIndex=%d,hoState=%d,B-reIpAddr=%x,B-reCallId-B=%d,B'-LAC:%x,B'-CI:%x)\n}\n",
                               callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId,     
                               h3GlobalCellId_.lac, h3GlobalCellId_.ci);
                
                sprintf(&(t_cellLocationRequest.ViperCellId[0]), "%d:%d", h3GlobalCellId_.lac,h3GlobalCellId_.ci);
                
                t_cellLocationRequest.TxnId = callIndex;
                
                if (VBLinkLocationRequest(t_cellLocationRequest) == false)
                {
                    //Ignore the message 
                    DBG_ERROR("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHbReq):VBLinkLocationRequest()failed for GP(%d:%d)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
                               h3GlobalCellId_.lac, h3GlobalCellId_.ci,hoAgId_,callIndex,hoState_);
                    
                    vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
                    vcOutMsg.msgData.perfHandbackNack.cause  = JC_FAILURE_ANCH_VB_LINK_PROBLEM;
                    
                    sendVcMsg(MODULE_CC, MODULE_CC);
                    
                    DBG_LEAVE();
                    return (CC_RELEASE_GP_HB);
                }
                
                // start the VB timer 
                parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);
                
                //MSC-B' born here while MSC-B is still alive. Mark handover type and relevant state
                hoState_ = HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ;
                
            } else 
            {
                //MSC-B' born here 
                hoState_ = HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ;
                
                DBG_LEAVE();
                return (handleVcAddressRsp());
            }
        }
    }
    
    //
    DBG_LEAVE();
    return (CC_MESSAGE_PROCESSING_COMPLT);
}
    
JCCEvent_t 
CCTargetHandoverExtHo::handleVcAddressRsp(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleVcAddressRsp", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleVcAddressRsp): [<==HOA IP ADDRESS RSP] (callIndex=%d,entryId=%d,ipAddr=%x)\n}\n",
                 callIndex, parent->entryId,hoVcIpAddress_);

  //Cancel Timer Tvb
  parent->sessionTimer->cancelTimer();

  switch (hoState_)
  {
  case HAND_ST_ANCH_VC_IP_ADDRESS_REQ:     
       //Handover from anchor GP->another GP
       if (hoRtpSession_ != NULL)
           ReleaseHandoverRtpChannel(hoRtpSession_);
       break;
                        
   case HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ:
       //Handover to 3rd GP from nonanchor GP
       if (h3RtpSession_ != NULL)
           ReleaseHandoverRtpChannel(h3RtpSession_);
       break;
  
  default:
       DBG_ERROR("{\nMNETeiTRACE(etrg::handleVcAddressRsp): CC Internal Error (hoAgId=%d,callId=%d,entryId=%d,hoState=%d)\n",
                  hoAgId_, callIndex, parent->entryId, hoState_);
       DBG_LEAVE();
       return (JCC_NULL_EVENT);
  }

  HJCRTPSESSION reRtpSession;

  if ((reRtpSession = AssignHandoverRtpChannel())
      == NULL)
  {
      // Do not proceed with ho
      DBG_ERROR ("{\nMNETeiTRACE(etrg::handleVcAddressRsp): AssignHandoverRtpChannel failed for reRtpSession(hoAgId=%d,callId=%d,entryId=%d,hoState=%d\n}\n",
                  hoAgId_,callIndex,parent->entryId,hoState_);
      DBG_LEAVE();
      return( (hoState_==HAND_ST_ANCH_VC_IP_ADDRESS_REQ) ? CC_RELEASE_GP_HO:CC_RELEASE_GP_H3);
  }

  vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDOVER;
  vcOutMsg.msgData.perfHandoverReq.sourceInRtpPort 
                   = GetIncomingHandoverRtpPort(reRtpSession);
  vcOutMsg.msgData.perfHandoverReq.channelType.bearerType
                   = BEARER_SPEECH;
  vcOutMsg.msgData.perfHandoverReq.channelType.speechChann
                   = currSpeechCh_;
  vcOutMsg.msgData.perfHandoverReq.globalCellId
                   = MY_GLOBALCELL_ID;
  
  IntraL3CipherModeCommand_t cipherCmd;
  //if (getExtHoCipherAlgo(&cipherCmd))
  {
      DBG_ERROR("{\nMNETeiTRACE(etrg::handleVcAddressRsp): getExtHoCipherAlgo() failed (hoAgId=%d,callId=%d,entryId=%d,hoState=%d)\n}\n",
                 hoAgId_,callIndex,parent->entryId,hoState_);
      cipherCmd.cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
  }
  
  DBG_TRACE("{\nMNETeiTRACE(etrg::handleVcAddressRsp): PerfHo (bearer %d, spchVer %d, ciphSet %d, algo %d, kc:%x,%x,%x,%x)\n}\n",
             vcOutMsg.msgData.perfHandoverReq.channelType.bearerType,
             vcOutMsg.msgData.perfHandoverReq.channelType.speechChann.version,
             cipherCmd.cmd.cipherModeSetting.ciphering,
             cipherCmd.cmd.cipherModeSetting.algorithm,
             cipherCmd.Kc[0],cipherCmd.Kc[1],
             cipherCmd.Kc[2],cipherCmd.Kc[3]);
  
  memcpy( (unsigned char*) &(vcOutMsg.msgData.perfHandoverReq.cipherCmd),
          (unsigned char*) &(cipherCmd),
           sizeof (IntraL3CipherModeCommand_t) );
  
  
  // Send Perform Handover to the Target ViperCell.
  switch (hoState_)
  {
  case HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ:
      // H3 case
      DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleVcAddressRsp): [==>GP-B' PERFORM_HANDOVER_REQ(h3)] (callIndex=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                 hoAgId_, callIndex, parent->entryId, hoGlobalCellId_.lac, hoGlobalCellId_.ci);

      DBG_TRACE("{\nMNETeiTRACE(etrg::handleVcAddressRsp): hand3rd info (loRtpHndl=%p, loPort=%d, h3VcIpAddress=%d)\n}\n",
                 reRtpSession, vcOutMsg.msgData.perfHandoverReq.sourceInRtpPort, h3VcIpAddress_);
      
      sendVcMsgH3(MODULE_CC, MODULE_CC);
      
      h3RtpSession_ = reRtpSession;
      hoState_ = HAND_ST_ANCH_SND_H3_REQ;
      break;
      
  case HAND_ST_ANCH_VC_IP_ADDRESS_REQ:
      // HO case
      DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleVcAddressRsp): [==>GP-B PERFORM_HANDOVER_REQ(ho)] (callIndex=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                 hoAgId_, callIndex, parent->entryId, hoGlobalCellId_.lac, hoGlobalCellId_.ci);

      DBG_TRACE("{\nNETeiTRACE(etrg::handleVcAddressRsp): handover info (loRtpHndl=%p, loPort=%d, hoVcIpAddress=%d)\n}\n",
                 reRtpSession, vcOutMsg.msgData.perfHandoverReq.sourceInRtpPort, hoVcIpAddress_);
      
      sendVcMsg(MODULE_CC, MODULE_CC);
      
      hoRtpSession_ = reRtpSession;
      hoState_ = HAND_ST_ANCH_SND_HO_REQ;
                 
      break;
  }
  
  // start the T202 timer 
  parent->sessionTimer->setTimer(CALL_HAND_TRG_T202);
  
  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}
    
JCCEvent_t 
CCTargetHandoverExtHo::handleAnchTvbTimerExpiry(void)
{ 
  JCCEvent_t retEvent = JCC_NULL_EVENT;

  DBG_FUNC("CCTargetHandoverExtHo::handleAnchTvbTimerExpiry", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchTvbTimerExpiry): [<==CC TIMER Tvb FIRED] (callIndex=%d,hoState=%d)\n}\n",
                 callIndex, hoState_);

  switch(hoState_)
  {
  case HAND_ST_ANCH_VC_IP_ADDRESS_REQ:
	   hoState_ = HAND_ST_TRG_HO_COMPLT;
	   retEvent = CC_MESSAGE_PROCESSING_COMPLT;

	   break;
	   
  case HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ:
	   retEvent = CC_RELEASE_GP_HB;
	   
	   break;

  default:
  	   DBG_ERROR("{\nMNETeiERROR(etrg::handleAnchTvbTimerExpiry): CC Error Inproper hoState(%d) (hoAgId=%d,callId=%d)\n}\n",
                 hoState_,hoAgId_,callIndex);
	   break;
  }

  DBG_LEAVE();
  return (retEvent);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleAnchHoFail(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleAnchHoFail", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchHoFail): [<==RR HANDOVER FAILURE](callId=%d,entryId=%d)\n}\n",
               callIndex, parent->entryId);

  //cancel T202 timer
  parent->sessionTimer->cancelTimer();

  vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
  vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_REVERT_TO_OLD;
  
  sendVcMsg(MODULE_CC, MODULE_CC);
  
  // break connection
  T_CSU_PORT_ID rtpSourcePort;
  T_CSU_RESULT_CON csuResult;
  
  rtpSourcePort.portType = CSU_RTP_PORT;
  rtpSourcePort.portId.rtpHandler = hoRtpSession_;
  
  if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort)) != CSU_RESULT_CON_SUCCESS)
  {
	  DBG_TRACE("{\nMNETeiTRACE(etrg::handleAnchHoFail) CSU BreakAll Error(%d) for rtpPort(%d:%p)\n}\n",
			     csuResult, rtpSourcePort.portType, (int)rtpSourcePort.portId.rtpHandler);
  }

  DBG_LEAVE();
  return (CC_RELEASE_GP_HO);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgHoFail(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgHoFail", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgHoFail): [<==RR HANDOVER FAILURE](callId=%d,entryId=%d)\n}\n",
                 callIndex, parent->entryId);

  //cancel T204
  parent->sessionTimer->cancelTimer();

  //Pass an A-HANDOVER FAILURE to anchor i.e. PLMN

  //Zero out message description 
  A_Handover_Failure_t *pFailure = (A_Handover_Failure_t*) &aifMsg.handoverFailure;
  memset((char*) &aifMsg,0,sizeof(T_AIF_MSG));
  
  //Mandatory IEs
  pFailure->msgType = A_HANDOVER_FAILURE_TYPE;
  pFailure->cause.ie_present        = true;
  pFailure->cause.elementIdentifier = 0x04;
  pFailure->cause.length            = 1;
  pFailure->cause.length            = (unsigned char) rrInMsg->l3_data.handFail.cause;
  
  //No optional IEs furnished now
  
  //Encode A-HANDOVER-FAILURE msg
  A_Result_t result;  A_INTERFACE_MSG_STRUCT_t aifData;
  memset( (char*) &aifData, 0,  sizeof(A_INTERFACE_MSG_STRUCT_t) );
  if ( A_RESULT_SUCCESS != (result=
	   AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&aifData) ) )
  {
	  DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoFail): A-HANDOVER-FAILURE encoding failed (result=%d)\n\n}\n",
		  result);
	  
	  DBG_LEAVE();
	  return (CC_RELEASE_EH_HB);
  }
  
  //Send up bss APDU to PLMN (anchor part)
  NonStdRasMessagePostHandoverMobEvent_t bssApdu;
  
  bssApdu.header.origSubId = callIndex;
  bssApdu.header.destSubId = hoAgId_;
  
  bssApdu.LAYER3PDU.LAYER3DATA[0] = 0x00;  //BSSMAP
  bssApdu.LAYER3PDU.LAYER3DATA[1] = (unsigned char) aifData.msglength;
  memcpy((char*)&bssApdu.LAYER3PDU.LAYER3DATA[2],(char*)aifData.A_INTERFACE_DATA,
	     aifData.msglength);
  bssApdu.LAYER3PDU.msglength = (unsigned short) (2 + aifData.msglength);
  
  if (!VBLinkPostHandoverMobEvent(&bssApdu))
  {
	  DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoFail): VBLinkPostHandoverMobEvent API failed !\n}\n");
	  DBG_HEXDUMP((unsigned char*) &bssApdu, bssApdu.LAYER3PDU.msglength+2);
	  
	  DBG_ERROR("}\n");
	  
	  DBG_LEAVE();
	  return (CC_RELEASE_EH_HB);;
  }
  
  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgHoFail): [==>HOA A-HANDOVER-FAILURE] (callId=%d,entryId=%d)\n",   
		 	     callIndex,parent->entryId);
  DBG_HEXDUMP((unsigned char*) &bssApdu, bssApdu.LAYER3PDU.msglength+2);
  DBG_CALL_TRACE(hoAgId_, "}\n");

  //Retry next candidate if there is candidate
  int i;
  IntraL3HandoverRequired_t *inMsg = &handReqd;

  if ( (i=getNextHoCandidate()) == (-1) )
  {
     DBG_TRACE("MNETeiTRACE(etrg::handleTrgHoFail): no more candidate to retrytry (#ofCands=%d)\n",
                inMsg->numCandidates);
             
     DBG_LEAVE();
     return (CC_RELEASE_EH_HB);

  } else
  {
     candGlobalCellId_t candidate;
     candidate = inMsg->candGlobalCellId[i];
     
     DBG_TRACE("MNETeiTRACE(etrg::handleTrgHoFail): try next cand cell! %d-th of Cands(%d) candCell(%x,%x%x%x,%x%x%x,%x,%x) hoState(%d)\n",
                i,inMsg->numCandidates, 
                candidate.candGlobalCellId.mcc[0],candidate.candGlobalCellId.mcc[1],
		candidate.candGlobalCellId.mcc[2],candidate.candGlobalCellId.mnc[0],
		candidate.candGlobalCellId.mnc[1],candidate.candGlobalCellId.mnc[2],
                candidate.candGlobalCellId.lac, candidate.candGlobalCellId.ci,
		hoState_);

     if (candidate.candIsExternal)
     {
         //Handback from anchor GP to PLMN case
         populateAHandoverRequest(&MY_GLOBALCELL_ID, &candidate.candGlobalCellId);

         A_Result_t result;
         NonStdRasMessagePerformHandbackRQ_t handbackRQ; 

         memset( (char*) &handbackRQ, 0,  sizeof(NonStdRasMessagePerformHandbackRQ_t) );

         if ( A_RESULT_SUCCESS != (result=
              AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&handbackRQ.A_HANDOVER_REQUEST) ) )
         {
              DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoFail): A-HANDOVER-REQUEST encoding failed (result=%d)\n\n}\n",
                         result);

              DBG_LEAVE();
              return (CC_RELEASE_EH_HB);
         }

         //Send PERFORM-HANDBACK-REQ to HOA
         handbackRQ.hoCause          = inMsg->hoCause;
         handbackRQ.header.origSubId = callIndex;
         handbackRQ.header.destSubId = hoAgId_;
         handbackRQ.globalCellID     = candidate.candGlobalCellId;
         handbackRQ.externalCell     = true;

         if (!VBLinkPerformHandback(&handbackRQ))
         {
             DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgHoFail): VBLinkPerformHandoverAck API failed !\n}\n");
               
             DBG_HEXDUMP( (unsigned char*) &handbackRQ,
                            (int) (handbackRQ.A_HANDOVER_REQUEST.msglength+26)
                        );

             DBG_ERROR("}\n");

             DBG_LEAVE();
             return (CC_RELEASE_EH_HB);            
         }

         DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgHoFail): [==>HOA PERFORM_HANDBACK_REQ](callId=%d,entryId=%d)\n}\n",
                        callIndex, parent->entryId);
                           
         DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgHoFail) PERFORM_HANDBACK_REQ MESSAGE DATA:\n");
         DBG_TRACE("       : hoAgId_=%d, callId=%d\n",hoAgId_, callIndex);
         DBG_TRACE("       : target cell(type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
                             handbackRQ.globalCellID.ci_disc,
                             handbackRQ.globalCellID.mcc[0],handbackRQ.globalCellID.mcc[1],handbackRQ.globalCellID.mcc[2],
                             handbackRQ.globalCellID.mnc[0],handbackRQ.globalCellID.mnc[1],handbackRQ.globalCellID.mnc[2],
                             handbackRQ.globalCellID.lac,handbackRQ.globalCellID.ci);

         DBG_TRACE("    Encoded A_HANDOVER_REQ HEX DUMP:\n");
         DBG_HEXDUMP((unsigned char*) &handbackRQ, sizeof (NonStdRasMessagePerformHandoverRQ_t) );
         DBG_TRACE("      \n}\n");


         DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgHoFail) Received A-INTERFACE-MSG DUMP (length=%d): \n",
                    handbackRQ.A_HANDOVER_REQUEST.msglength);
         DBG_HEXDUMP(
                    (unsigned char*)handbackRQ.A_HANDOVER_REQUEST.A_INTERFACE_DATA,
                    (int)handbackRQ.A_HANDOVER_REQUEST.msglength
                    );
         DBG_TRACE("\n END-OF-DUMP\n}\n");

         parent->sessionTimer->setTimer(CALL_HAND_TRG_T211);

         DBG_CALL_TRACE(hoAgId_, "\nMNETeiTRACE(etrg::handleTrgHoFail):  [<==CC START TIMER T211] (callId=%d, T211=%d ticks)\n}\n",
                        callIndex, (int)CALL_HAND_TRG_T211);

         hoState_ = HAND_ST_TRG_SND_HB_REQ;
     
     } else
     {
         //Handover from anchor GP to another GP case
         sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", 
                  candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);

         cellLocationRequest.TxnId = callIndex;
    
         if (false == VBLinkLocationRequest(cellLocationRequest) )
         {
             // Ignore the message 
             DBG_ERROR("{\nMNETeiTRACE(etrg::handleTrgHoFail): VBLinkLocationRequest failed!\n");
             DBG_ERROR("                                      (hoAgId=%d callId=%d entryId=%d,lac=%d,ci=%d)\n}\n",
                        hoAgId_, callIndex, parent->entryId, 
                        candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);

             DBG_LEAVE();
             return (CC_RELEASE_EH_HB);
         }

         DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgHoFail): [==>HOA IP_ADDRESS_REQ](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                        callIndex, parent->entryId,
                        candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
         
         DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgHoFail): [<==CC START TIMER Tvb](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                     callIndex, parent->entryId,
                     candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);

         parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);

         hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;
     }
  }

  DBG_LEAVE();
  return (CC_MESSAGE_PROCESSING_COMPLT);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleAnchToTrgCompltHb(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleAnchToTrgCompltHb", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchToTrgCompltHb): [<==HOA HANDBACK SUCCESS] (callIndex=%d, entryId=%d,hoState=%d)\n}\n",
                 callIndex, parent->entryId,hoState_);

  // cancel T204 timer
  parent->sessionTimer->cancelTimer();

  if (hoState_==HAND_ST_ANCH_RCV_HB_REQ_ACK)
  {
      vcOutMsg.msgType = INTER_VC_CC_COMPLETE_HANDBACK;
      sendVcMsg(MODULE_CC, MODULE_CC);

      DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleAnchToTrgCompltHb): [==>GP-B HANDBACK SUCCESS] (callIndex=%d,hoState=%d)\n}\n",
                     callIndex, hoState_);
  }

  DBG_LEAVE();
  return (CC_RELEASE_OWN);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgToAnchCompltHo(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgToAnchCompltHo", CC_ETRG_LAYER);
  DBG_ENTER();
    
  //cancel T202
  parent->sessionTimer->cancelTimer();

  GlobalCellId_t globalCellId, globalCellId_t;
    
  T_CSU_PORT_ID  oldPort, srcPort, snkPort;
  T_CSU_RESULT_CON csuResult;

  srcPort.portType = CSU_RTP_PORT;
  srcPort.portId.rtpHandler = VoipCallGetRTPHandle(parent->hoCallLeg->h323CallHandle);

  switch (hoState_)
  {
  case HAND_ST_ANCH_RCV_HO_REQ_ACK:
       DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchCompltHo): [<==GP-B HANDOVER_COMPLETE] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                 callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);
  
       oldPort.portType = CSU_GSM_PORT;
       oldPort.portId.gsmHandler = parent->entryId;
       snkPort.portType = CSU_RTP_PORT;
       snkPort.portId.rtpHandler = hoRtpSession_;

       globalCellId = hoGlobalCellId_;
      
       break;
    
  case HAND_ST_ANCH_RCV_H3_REQ_ACK:
       DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchCompltHo): [<==GP-B' HANDOVER_COMPLETE] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                 callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);
 
       oldPort.portType = CSU_RTP_PORT;
       oldPort.portId.rtpHandler = hoRtpSession_;
       snkPort.portType = CSU_RTP_PORT;
       snkPort.portId.rtpHandler = h3RtpSession_;

       globalCellId = h3GlobalCellId_;
      
       break;
      
  default:
       DBG_ERROR("{\nMNETeiTRACE(etrg::handleTrgToAnchCompltHo) CC Inproper hoState(%d)(hoAgId=%d,callId=%d)\n}\n",
                  hoState_,hoAgId_,callIndex);
                    
       DBG_LEAVE();
       return(JCC_NULL_EVENT);
  }
    
  DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchCompltHo) (hoState=%d) srcPort(%d:%p) oldPort(%d:%p) snkPort(%d,%x)\n}\n",
             hoState_,
             srcPort.portType, srcPort.portId.rtpHandler,
             oldPort.portType, oldPort.portId.rtpHandler,
             snkPort.portType, snkPort.portId.rtpHandler);
   
  if ( (csuResult = csu_OrigTermBreakAll(&oldPort)) != CSU_RESULT_CON_SUCCESS)
  {
      DBG_WARNING("{\nMNETeiWARNING(etrg::handleTrgToAnchCompltHo) CSU BreakAll Error(%d) for oldPort(%d, %x)\n}\n",  
                   csuResult, oldPort.portType, oldPort.portId.rtpHandler);

      //Ignore this warning to allow call going along
  }
  
  // Make duplex ho or h3 connection between src and snk rtp ports
  if ((csuResult = csu_DuplexConnect(&snkPort, &srcPort)) != CSU_RESULT_CON_SUCCESS)
  {
      DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgToAnchCompltHo) CSU 2-WAY Conn Error (%d) SrcPort(%d:%p)<->SnkPort(%d:%p)\n}\n",  
                 csuResult, srcPort.portType, srcPort.portId.rtpHandler, snkPort.portType, snkPort.portId.rtpHandler);
      
      DBG_LEAVE();
      return (CC_RELEASE_CALL);
  }

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchCompltHo): [==>CSU 2-WAY CONNECT] src(%d:%p) snk(%d:%p)] (callIndex=%d, hoState=%d, reIpAddr=%x,reCallId=%d)\n}\n",
                 srcPort.portType, srcPort.portId.rtpHandler, snkPort.portType, snkPort.portId.rtpHandler,
                 callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);

  DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchCompltHo) BEFORE B'->B (hoAgId=%d,callId=%d,hoState=%d) ho(rtp=%p:%d vcCc=%x:%d vcId=%d:%d) h3(rtp=%p:%d vcCc=%x:%d vcId=%d:%d)\n}\n", 
             hoAgId_,callIndex, hoState_,
             hoRtpSession_,otherHoIncomingRtpPort_, hoVcIpAddress_,hoVcCallIndex_, hoGlobalCellId_.lac,hoGlobalCellId_.ci,
             h3RtpSession_,otherH3IncomingRtpPort_, h3VcIpAddress_,h3VcCallIndex_, h3GlobalCellId_.lac,h3GlobalCellId_.ci);

  if (hoState_ == HAND_ST_ANCH_RCV_H3_REQ_ACK)
  {       
      // Send the Complete Handback message to vcB
      vcOutMsg.msgType = INTER_VC_CC_COMPLETE_HANDBACK;
      sendVcMsg(MODULE_CC, MODULE_CC);
      
      //Release hoRtpHndle
    
      if ( hoRtpSession_ != NULL ) ReleaseHandoverRtpChannel(hoRtpSession_);
        
      // Update vcB'->vcB
      hoRtpSession_  		   = h3RtpSession_;
      h3RtpSession_  		   = NULL;
      hoVcIpAddress_ 		   = h3VcIpAddress_;
      h3VcIpAddress_ 		   = 0;
      hoVcCallIndex_ 		   = h3VcCallIndex_;
      h3VcCallIndex_ 		   = CC_MAX_CC_CALLS_DEF_VAL;
      otherHoIncomingRtpPort_  = otherH3IncomingRtpPort_;
      otherH3IncomingRtpPort_  = 0;
      hoGlobalCellId_          = h3GlobalCellId_;
      memset(&h3GlobalCellId_, 0, sizeof(GlobalCellId_t));
      
      hoSimplexConnected_      = false;
      h3SimplexConnected_      = false;
      hoSrcLinkLost_           = false;
      h3SrcLinkLost_           = false;     
  } else
  {
    // Release rf
    if (parent->entryId != (T_CNI_IRT_ID) JCC_UNDEFINED )
    {
        // Send internal MM msg - release channel
        sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, parent->entryId, &mmOutMsg);
        parent->resetIrt();
    }
    
    parent->entryId = (T_CNI_IRT_ID) JCC_UNDEFINED;
    parent->oid = CNI_LAPDM_NULL_OID;
    hoSimplexConnected_ = false;
    hoSrcLinkLost_      = false;
  }

  DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchCompltHo) AFTER B'->B (hoAgId=%d,callId=%d,hoState=%d) ho(rtp=%p:%d vcCc=%x:%d vcId=%d:%d) h3(rtp=%p:%d vcCc=%x:%d vcId=%d:%d)\n}\n", 
             hoAgId_,callIndex, hoState_,
             hoRtpSession_,otherHoIncomingRtpPort_, hoVcIpAddress_,hoVcCallIndex_, hoGlobalCellId_.lac,hoGlobalCellId_.ci,
             h3RtpSession_,otherH3IncomingRtpPort_, h3VcIpAddress_,h3VcCallIndex_, h3GlobalCellId_.lac,h3GlobalCellId_.ci);

  hoState_  = HAND_ST_ANCH_HO_COMPLT;

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
CCTargetHandoverExtHo::handleTrgT202TimerExpiry(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleTrgT202TimerExpiry", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgT202TimerExpiry): [<==CC TIMER T202 FIRED] (callIndex=%d,entryId=%d)\n}\n",
                 callIndex, parent->entryId);


  DBG_LEAVE();
  return (CC_RELEASE_CALL);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgToAnchPerfHoAck(void)
{
    DBG_FUNC("CCTargetHandoverExtHo::handleTrgToAnchPerfHoAck", CC_ETRG_LAYER);
    DBG_ENTER();
    
    //cancel T202
    parent->sessionTimer->cancelTimer();
    
    T_CSU_PORT_ID  rtpSinkPort;
    
    switch(hoState_)
    {
    case HAND_ST_ANCH_SND_H3_REQ:
         DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoAck): [<==GP-B' PERFORM_HANDOVER_ACK] (callIndex=%d,entryId=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                        callIndex, parent->entryId, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);
    
         h3VcCallIndex_ = vcInMsg->origSubId;

         DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoAck) hand23rd (hoAgId=%d,callId=%d,entryId=%d,h3RtpHndl=%p) (reIpAddr=%x, reCallId=%d, reRtpPort=%d)\n}\n",
                    hoAgId_, callIndex, parent->entryId, h3RtpSession_,
                    h3VcIpAddress_, h3VcCallIndex_, vcInMsg->msgData.perfHandoverAck.targetInRtpPort);
 
         if (h3RtpSession_ == NULL)
         {
            DBG_WARNING("{\nMNETeiWARNING(etrg::handleTrgToAnchPerfHoAck) h3RtpHandle NULL (hoAgId=%d,callId=%d,entryId=%d, hoState=%d)(reIpAddr=%x,reCallId=%d)\n}\n",
                    hoAgId_,callIndex, parent->entryId, hoState_, h3VcIpAddress_, h3VcCallIndex_);

            vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
            vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_ANCH_H3_RTP_HANDLE_NULL;
            sendVcMsgH3(MODULE_CC, MODULE_CC);

            vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
            vcOutMsg.msgData.perfHandbackNack.cause = JC_FAILURE_ANCH_H3_RTP_HANDLE_NULL;

            sendVcMsg(MODULE_CC, MODULE_CC);

            DBG_LEAVE();
            return (CC_RELEASE_GP_H3);
         }

         SetRemoteHandoverRtpPort(h3RtpSession_, h3VcIpAddress_,
                                  vcInMsg->msgData.perfHandoverAck.targetInRtpPort);
        
         rtpSinkPort.portType = CSU_RTP_PORT;
         rtpSinkPort.portId.rtpHandler = h3RtpSession_;
        
         break;
        
    case HAND_ST_ANCH_SND_HO_REQ:
         DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoAck): [<==GP-B PERFORM_HANDOVER_ACK] (callIndex=%d,entryId=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
         callIndex, parent->entryId, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);
    
         hoVcCallIndex_ = vcInMsg->origSubId;
        
         DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoAck) handover (hoAgId=%d,callId=%d,entryId=%d,hoRtpHndl=%p) (reIpAddr=%x, reCallId=%d, reRtpPort=%d)\n}\n",
                    hoAgId_, callIndex, parent->entryId, hoRtpSession_,
                    hoVcIpAddress_, hoVcCallIndex_, vcInMsg->msgData.perfHandoverAck.targetInRtpPort);
                             
         if (hoRtpSession_ == NULL)
         {
            DBG_WARNING("{\nMNETeiWARNING(etrg::handleTrgToAnchPerfHoAck) hoRtpHandle NULL (hoAgId=%d,callId=%d,entryId=%d, hoState=%d)(reIpAddr=%x,reCallId=%d)\n}\n",
                    hoAgId_,callIndex, parent->entryId, hoState_, hoVcIpAddress_, hoVcCallIndex_);

            vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
            vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_ANCH_HO_RTP_HANDLE_NULL;
            sendVcMsg(MODULE_CC, MODULE_CC);

            DBG_LEAVE();
            return (CC_RELEASE_GP_HO);
         }
        
         SetRemoteHandoverRtpPort(hoRtpSession_, hoVcIpAddress_,
                                  vcInMsg->msgData.perfHandoverAck.targetInRtpPort);
        
         rtpSinkPort.portType = CSU_RTP_PORT;
         rtpSinkPort.portId.rtpHandler = hoRtpSession_;
                
         break;
        
    default:
         //Deadly problem, need shoubleshooting first before running ahead !
         DBG_ERROR ("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoAck): CC Error Inproper hoState(%d) (hoAgId=%d,callId=%d)\n}\n", 
                     hoState_,hoAgId_,callIndex);
         DBG_LEAVE();
         return (JCC_NULL_EVENT);
    }
    
    // simplex conn for ho or h3
    
    T_CSU_PORT_ID  rtpSourcePort;
    T_CSU_RESULT_CON csuResult;
    
    rtpSourcePort.portType = CSU_RTP_PORT;
    rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(parent->hoCallLeg->h323CallHandle);
    
    if ((csuResult = csu_SimplexConnect(&rtpSourcePort, &rtpSinkPort))
        != CSU_RESULT_CON_SUCCESS)
    {
        DBG_ERROR("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoAck): CSU 1-Way Conn Error(%d) (hoAgId=%d,callId=%d,hoState=%d,srcRtpHndl=%p,snkRtpHndl=%p)\n}\n",
            csuResult, hoAgId_,callIndex,hoState_,
            (int)rtpSourcePort.portId.rtpHandler,
            (int)rtpSinkPort.portId.rtpHandler);
        
		if (hoState_ == HAND_ST_ANCH_SND_H3_REQ)
		{
		    vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDBACK;
            vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_CSU_UNI_CONNECT_FAILED;
		    sendVcMsg(MODULE_CC, MODULE_CC);

		    vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
            vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_CSU_UNI_CONNECT_FAILED;
		    sendVcMsgH3(MODULE_CC, MODULE_CC);

			DBG_LEAVE();
			return (CC_RELEASE_GP_H3);
		} else
		{
			//hoState_ == HAND_ST_ANCH_SND_HO_REQ)
		    vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
            vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_CSU_UNI_CONNECT_FAILED;
		    sendVcMsg(MODULE_CC, MODULE_CC);

			DBG_LEAVE();
			return (CC_RELEASE_GP_HO);
		}
    }

    // Handover or handover to 3rd Simplex Connection is made successfully
    DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoAck): [==>CSU 1-WAY CONNECT](callId=%d,hoState=%d,src=%d:%p<-->snk=%d:%p)\n}\n",
                   callIndex, hoState_,
				   rtpSinkPort.portType, rtpSinkPort.portId.rtpHandler,
				   rtpSourcePort.portType, rtpSourcePort.portId.rtpHandler);
    
    switch(hoState_)
    {
    case HAND_ST_ANCH_SND_HO_REQ:
         // Send internal RR msg
         rrOutMsg.l3_data.handCmd.handCmd =  vcInMsg->msgData.perfHandoverAck.handCmd;
        
         sendRR(INTRA_L3_DATA, INTRA_L3_RR_HANDOVER_COMMAND, parent->entryId, &rrOutMsg);
    
         hoSimplexConnected_ = true;
         hoState_ = HAND_ST_ANCH_RCV_HO_REQ_ACK;

		 DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoAck): [==>RM HANDOVER_COMMAND](callId=%d,hoState=%d,entryId=%d)\n}\n",
                   callIndex, hoState_,parent->entryId);
         break;

    case HAND_ST_ANCH_SND_H3_REQ:        
         //Send HB ack to MSC-B based on HO ack from MSC-B'
         vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_ACK;
         vcOutMsg.msgData.perfHandbackAck.handCmd  = vcInMsg->msgData.perfHandoverAck.handCmd;
        
         sendVcMsg(MODULE_CC, MODULE_CC);
        
         h3SimplexConnected_ = true;
         hoState_ = HAND_ST_ANCH_RCV_H3_REQ_ACK;
        
	 DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoAck): [==>GP-B PERFORM_HANDBACK_ACK](callId=%d,entryId=%d,hoState=%d)\n}\n",
                        callIndex,parent->entryId,hoState_);

         break;
    }     
       
    // start the T202 timer 
    parent->sessionTimer->setTimer(CALL_HAND_TRG_T202);
    
    //
    DBG_LEAVE();
    return (CC_MESSAGE_PROCESSING_COMPLT);
}


JCCEvent_t 
CCTargetHandoverExtHo::handleTrgToAnchPerfHoNack(void)
{
    DBG_FUNC("CCTargetHandoverExtHo::handleTrgToAnchPerfHoNack", CC_ETRG_LAYER);
    DBG_ENTER();

    //cancel T202
    parent->sessionTimer->cancelTimer();

    int i;
    IntraL3HandoverRequired_t *inMsg = &handReqd;

    switch(hoState_)
    {
    case HAND_ST_ANCH_SND_HO_REQ:

         DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): [<==GP-B PERFORM_HANDOVER_NACK] (callIndex=%d,entryId=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                        callIndex, parent->entryId, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);
        
         if ( (i=getNextHoCandidate()) != (-1) )
         {
            //Try next candidate
            candGlobalCellId_t candidate;
            candidate = inMsg->candGlobalCellId[i];
            
            DBG_TRACE("MNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): try next cand cell! %d-th of Cands(%d) candCell(%x,%x%x%x,%x%x%x,%x,%x) hoState(%d)\n}\n",
                       i,inMsg->numCandidates, 
                       candidate.candGlobalCellId.mcc[0],candidate.candGlobalCellId.mcc[1],
		       candidate.candGlobalCellId.mcc[2],candidate.candGlobalCellId.mnc[0],
		       candidate.candGlobalCellId.mnc[1],candidate.candGlobalCellId.mnc[2],
                       candidate.candGlobalCellId.lac, candidate.candGlobalCellId.ci,
		       hoState_);
            
            if (candidate.candIsExternal)
            {
               //Handback from anchor GP to PLMN case
               populateAHandoverRequest(&MY_GLOBALCELL_ID, &candidate.candGlobalCellId);

               A_Result_t result;
               NonStdRasMessagePerformHandbackRQ_t handbackRQ; 

               memset( (char*) &handbackRQ, 0,  sizeof(NonStdRasMessagePerformHandbackRQ_t) );
               
               if ( A_RESULT_SUCCESS != (result=
                   AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&handbackRQ.A_HANDOVER_REQUEST) ) )
               {
                   DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgToAnchPerfHoNack): A-HANDOVER-REQUEST encoding failed (result=%d)\n\n}\n",
                       result);
                   
				   vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
				   vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_A_INTERFACE_MESSAGE_ENCODING_FAILED;
				   sendVcMsg(MODULE_CC, MODULE_CC);

                   DBG_LEAVE();
                   return (CC_RELEASE_GP_HO);
               }
               
               //Send PERFORM-HANDBACK-REQ to HOA
               handbackRQ.hoCause          = inMsg->hoCause;
               handbackRQ.header.origSubId = callIndex;
               handbackRQ.header.destSubId = hoAgId_;
               handbackRQ.globalCellID     = candidate.candGlobalCellId;
               handbackRQ.externalCell     = true;
               
               if (!VBLinkPerformHandback(&handbackRQ))
               {
                   DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgToAnchPerfHoNack): VBLinkPerformHandoverAck API failed !\n}\n");
                   
                   DBG_HEXDUMP( (unsigned char*) &handbackRQ,
                                (int) (handbackRQ.A_HANDOVER_REQUEST.msglength+26)
                              );
                   
                   DBG_ERROR("}\n");
                   
                   DBG_LEAVE();
                   return (CC_RELEASE_GP_HO);            
               }
               
               DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): [==>HOA PERFORM_HANDBACK_REQ](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                              callIndex, parent->entryId,candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
               
               DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack) PERFORM_HANDBACK_REQ MESSAGE DATA:\n");
               DBG_TRACE("       : hoAgId_=%d, callId=%d\n",hoAgId_, callIndex);
               DBG_TRACE("       : target cell(type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
                          handbackRQ.globalCellID.ci_disc,
                          handbackRQ.globalCellID.mcc[0],handbackRQ.globalCellID.mcc[1],handbackRQ.globalCellID.mcc[2],
                          handbackRQ.globalCellID.mnc[0],handbackRQ.globalCellID.mnc[1],handbackRQ.globalCellID.mnc[2],
                          handbackRQ.globalCellID.lac,handbackRQ.globalCellID.ci);
               
               DBG_TRACE("    Encoded A_HANDOVER_REQ HEX DUMP:\n");
               DBG_HEXDUMP((unsigned char*) &handbackRQ, sizeof (NonStdRasMessagePerformHandoverRQ_t) );
               DBG_TRACE("      \n}\n");
               
               DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack) Received A-INTERFACE-MSG DUMP (length=%d): \n",
                          handbackRQ.A_HANDOVER_REQUEST.msglength);
               DBG_HEXDUMP(
                           (unsigned char*)handbackRQ.A_HANDOVER_REQUEST.A_INTERFACE_DATA,
                           (int)handbackRQ.A_HANDOVER_REQUEST.msglength
                          );
               DBG_TRACE("\n END-OF-DUMP\n}\n");

               parent->sessionTimer->setTimer(CALL_HAND_TRG_T211);
               
               DBG_CALL_TRACE(hoAgId_, "\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack):  [<==CC START TIMER T211] (callId=%d, T211=%d ticks,lac=%d,ci=%d)\n}\n",
                              callIndex, (int)CALL_HAND_TRG_T211,candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
               
               hoState_ = HAND_ST_TRG_SND_HB_REQ;
               
            } else
            {
                //Handover from anchor GP to another GP case
                sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", 
                        candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
                
                cellLocationRequest.TxnId = callIndex;
                
                if (false == VBLinkLocationRequest(cellLocationRequest) )
                {
                    // Ignore the message 
                    DBG_ERROR("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): VBLinkLocationRequest failed!\n");
                    DBG_ERROR("                                      (hoAgId=%d callId=%d entryId=%d,lac=%d,ci=%d)\n}\n",
                                hoAgId_, callIndex, parent->entryId, 
                                candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
                    
                    DBG_LEAVE();
                    return (CC_RELEASE_GP_HO);
                }
                
                DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): [==>HOA IP_ADDRESS_REQ](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                               callIndex, parent->entryId,
                               candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
                
                DBG_TRACE("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): [<==CC START TIMER Tvb](hoAgId=%d,callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
                          hoAgId_, callIndex, parent->entryId,
                          candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
                
                parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);
                    
                hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ;
            }

         } else
         {
            DBG_TRACE("MNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): run-out of (#ofCands=%d) (hoAgId=%d,callId=%d,entryId=%d,hoState=%d)\n",
                       inMsg->numCandidates,hoAgId_,callIndex,parent->entryId,hoState_);
            DBG_LEAVE();
            return (CC_RELEASE_GP_HO);
         }

         break;

    case HAND_ST_ANCH_SND_H3_REQ:

         DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): [<==GP-B' PERFORM_HANDOVER_NACK] (callIndex=%d,entryId=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                        callIndex, parent->entryId, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);

         //Abort action @ MSC-B'
         vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
         vcOutMsg.msgData.abortHandover.cause = vcInMsg->msgData.perfHandoverNack.cause;
        
         sendVcMsgH3(MODULE_CC, MODULE_CC);
        
		 DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): [==>GP-B' ABORT_HANDOVER] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                        callIndex, hoState_, vcInMsg->origVcAddress, vcInMsg->origSubId);

         // Abort HB part at MSC-B
         vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_NACK;
         vcOutMsg.msgData.perfHandbackNack.cause = vcInMsg->msgData.perfHandoverNack.cause;
        
         sendVcMsg(MODULE_CC, MODULE_CC);

		 DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): [==>GP-B ABORT_HANDBACK] (callIndex=%d,hoState=%d,reIpAddr=%x,reCallId=%d)\n}\n",
                        callIndex, hoState_, hoVcIpAddress_, hoVcCallIndex_);

         DBG_LEAVE();
         return(CC_RELEASE_GP_H3);
        
         break;
        
    default:
         //Deadly problem, need shoubleshooting right away !
         DBG_ERROR ("{\nMNETeiTRACE(etrg::handleTrgToAnchPerfHoNack): CC Inproper hoState(%d) (hoAgId=%d,callId=%d)\n}\n", 
                     hoState_, hoAgId_, callIndex);
         DBG_LEAVE();
          
		 return (JCC_NULL_EVENT);
    }

    //
    DBG_LEAVE();
    return (CC_MESSAGE_PROCESSING_COMPLT);
}
    
JCCEvent_t 
CCTargetHandoverExtHo::handleTrgToAnchAbortHb(void)
{
  JCCEvent_t retEvent;

  DBG_FUNC("CCTargetHandoverExtHo::handleTrgToAnchAbortHb", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchAbortHb): [<==GP-B ABORT_HANDBACK](callId=%d,hoState=%d)\n}\n",
                 callIndex, hoState_);   

  parent->sessionTimer->cancelTimer();

  if ( (hoState_ == HAND_ST_ANCH_SND_HB_REQ) || (hoState_ ==HAND_ST_ANCH_RCV_HB_REQ_ACK) )
	    retEvent = CC_RELEASE_GP_EH_HB;
  else if ( (hoState_ == HAND_ST_ANCH_RCV_HB_REQ) || (hoState_ == HAND_ST_ANCH_SND_HB_REQ_ACK) ||
	        (hoState_ == HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ) )
		retEvent = CC_RELEASE_GP_HB;

  else if ( (hoState_ == HAND_ST_ANCH_RCV_H3_REQ_ACK) || (hoState_ == HAND_ST_ANCH_SND_H3_REQ) )
  {
		vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
	    vcOutMsg.msgData.abortHandover.cause = vcInMsg->msgData.abortHandback.cause;
		sendVcMsgH3(MODULE_CC, MODULE_CC);

		retEvent = CC_RELEASE_GP_H3;
  } else
  {
        retEvent = JCC_NULL_EVENT;
  }

  DBG_LEAVE();
  return(retEvent);
}

JCCEvent_t 
CCTargetHandoverExtHo::handleTrgToAnchHbFail(void)
{
  JCCEvent_t retEvent = JCC_NULL_EVENT;

  DBG_FUNC("CCTargetHandoverExtHo::handleTrgToAnchHbFail", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgToAnchHbFail): [<==GP-B HANDOVER FAILURE](callId=%d,hoState=%d)\n}\n",
                 callIndex, hoState_);   

  //Cancel T202 or T204 
  parent->sessionTimer->cancelTimer();
  A_Handover_Failure_t *pFailure = (A_Handover_Failure_t*) &aifMsg.handoverFailure;
  
  switch (hoState_)
  {
  case HAND_ST_ANCH_RCV_H3_REQ_ACK:
       vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
       vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_ANCH_H3_RTP_HANDLE_NULL;
       sendVcMsgH3(MODULE_CC, MODULE_CC);
       
	   retEvent = CC_RELEASE_GP_H3;

       break;
	   
  case HAND_ST_ANCH_RCV_HB_REQ_ACK:
 	   //Zero out message description 
	   memset((char*) &aifMsg,0,sizeof(T_AIF_MSG));
	  
	   //Mandatory IEs
	   pFailure->msgType = A_HANDOVER_FAILURE_TYPE;
	   pFailure->cause.ie_present        = true;
	   pFailure->cause.elementIdentifier = 0x04;
	   pFailure->cause.length            = 1;
	   pFailure->cause.length            = (unsigned char) vcInMsg->msgData.abortHandback.cause;
	  
	   //No optional IEs furnished now
	  
	   //Encode A-HANDOVER-FAILURE msg
	   A_Result_t result;  A_INTERFACE_MSG_STRUCT_t aifData;
	   memset( (char*) &aifData, 0,  sizeof(A_INTERFACE_MSG_STRUCT_t) );
	   if ( A_RESULT_SUCCESS != (result=
	 	    AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&aifData) ) )
	   {
		    DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgToAnchHbFail): A-HANDOVER-FAILURE encoding error(%d)(hoAgId=%d,callIndex=%d,hoState=%d)\n}\n",
			           result,hoAgId_,callIndex,hoState_);
		  
		    retEvent = CC_RELEASE_EH_HB;
	   }
	  
	   //Send up bss APDU to PLMN (anchor part)
	   NonStdRasMessagePostHandoverMobEvent_t bssApdu;
	  
	   bssApdu.header.origSubId = callIndex;
	   bssApdu.header.destSubId = hoAgId_;
	  
	   bssApdu.LAYER3PDU.LAYER3DATA[0] = 0x00;  //BSSMAP
	   bssApdu.LAYER3PDU.LAYER3DATA[1] = (unsigned char) aifData.msglength;
	   memcpy((char*)&bssApdu.LAYER3PDU.LAYER3DATA[2],(char*)aifData.A_INTERFACE_DATA,
		 	  aifData.msglength);
	   bssApdu.LAYER3PDU.msglength = (unsigned short) (2 + aifData.msglength);
	  
	   if (!VBLinkPostHandoverMobEvent(&bssApdu))
	   {
	 	   DBG_ERROR("{\nMNETeiERROR(etrg::handleTrgToAnchHbFail): VBLinkPostHandoverMobEvent() failed (hoAgId=%d,callIndex=%d,hoState=%d)\n}\n",
			          hoAgId_,callIndex,hoState_);
		   DBG_HEXDUMP((unsigned char*) &bssApdu, bssApdu.LAYER3PDU.msglength+2);
		  
		   DBG_ERROR("}\n");

		   retEvent = CC_RELEASE_EH_HB;
	   }
	  
	   DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etgt::handleTrgToAnchHbFail): [==>HOA A-HANDOVER-FAILURE] (callId=%d,entryId=%d,hoState=%d)\n",   
		              callIndex,parent->entryId,hoState_);
	   DBG_HEXDUMP((unsigned char*) &bssApdu, bssApdu.LAYER3PDU.msglength+2);
	   DBG_CALL_TRACE(hoAgId_, "}\n");
	  
	   retEvent = CC_RELEASE_EH_HB;

	   break;

  default:
	   DBG_ERROR("{\nMNETeiERROR)(etrg::handleTrgToAnchHbFail):CC Error Inproper hoState(%d)(hoAgId=%d,callId=%d)\n}\n",
		          hoState_,hoAgId_,callIndex);
	   break;
  }

  DBG_LEAVE();
  return (retEvent);

}

int 
CCTargetHandoverExtHo::ccReleaseCall(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::ccReleaseCall", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_TRACE("{\nMNETeiTRACE(etrg::ccReleaseCall): CC External Handover (CC_RELEASE_CALL event=%d)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
	         hoEndCause_,hoAgId_,callIndex,hoState_);

  //Cancel whatever timer in case
  parent->sessionTimer->cancelTimer();

  //Send END_HANDOVER to HOA
  NonStdRasMessageEndHandover_t  endHandover;
  endHandover.header.origSubId = callIndex;
  endHandover.header.destSubId = hoAgId_;
  endHandover.reason    = hoEndCause_;
  
  if (!VBLinkEndHandover(&endHandover))
  {
	  DBG_ERROR("{\nMNETeiERROR(etrg::smHandleEvent): VBLinkEndHandover API failed(hoAgId=%d,callId=%d,hoState=%d,cause=%d)\n}\n",
		         hoAgId_,callIndex,hoState_,hoEndCause_);
  } else
  {
	  DBG_TRACE("{\nMNETeiTRACE(etrg::smHandleEvent): [==>HOA END_HANDOVER cause=%d)](hoAgId=%d,callId=%d,hoState=%d)\n}\n",
				 hoEndCause_, hoAgId_, callIndex, hoState_);
  }

  //Send RELEASE_HOA to HOA
  //NonStdRasMessageReleaseHOA_t relHoa;
  //relHoa.header.origSubId = callIndex;
  //relHoa.header.destSubId = hoAgId_;

  //if (!VBLinkReleaseHOA(&relHoa))
  //{
  //	 DBG_ERROR("{\nMNETeiERROR(etrg::smHandleEvent): VBLinkReleaseHOA API failed\n}\n");
  //} else
  //{
  //     DBG_TRACE("{\nMNETeiTRACE(etrg::smHandleEvent): [==>HOA RELEASE_HANDOVER_AGENT (hoAgId=%d,callId=%d,entryId=%d)\n}\n",
  //				hoAgId_, callIndex, parent->entryId);
  //}

  hoAgId_ = -1;

  DBG_LEAVE();
  return(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);
}

  
int 
CCTargetHandoverExtHo::ccReleaseOwn(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::ccReleaseOwn", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_TRACE("{\nMNETeiTRACE(etrg::ccReleaseCall): CC External Handover (CC_RELEASE_OWN)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
	       hoAgId_,callIndex, hoState_);

  //Cancel whatever timer in case
  parent->sessionTimer->cancelTimer();

  hoAgId_ = -1;

  DBG_LEAVE();
  return(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);
}

void
CCTargetHandoverExtHo::ccReleaseGpHo(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::ccReleaseGpHo", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_TRACE("{\nMNETeiTRACE(etrg::ccReleaseGpHo): CC External Handover (CC_RELEASE_GP_HO)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
	         hoAgId_,callIndex,hoState_);

  //Cancel whatever timer in case
  parent->sessionTimer->cancelTimer();

  if (hoRtpSession_ != NULL)
  {
	  // break connection
	  T_CSU_PORT_ID rtpSourcePort;
	  T_CSU_RESULT_CON csuResult;
	  
	  rtpSourcePort.portType = CSU_RTP_PORT;
	  rtpSourcePort.portId.rtpHandler = hoRtpSession_;
	  
	  if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort)) != CSU_RESULT_CON_SUCCESS)
	  {
		  DBG_ERROR("{\nMNETeiERROR(etrg::ccReleaseGpHo) CC CSU OrigTerm Break All Error(%d) (hoAgId=%d,callId=%d,hoRtpSession=%x)\n",  
			         csuResult,hoAgId_,callIndex, (int)hoRtpSession_);

		  //Skip this error but may report via OAM if it cares.
	  }
	  
	  ReleaseHandoverRtpChannel(hoRtpSession_);

      hoRtpSession_ = NULL;
  }

  msMsgsQueued_			  = false;
  rrMsgsQueued_			  = false;
  hoSimplexConnected_     = false;
  hoSrcLinkLost_          = false;  //ext-HO FFS ???
  hoVcCallIndex_          = CC_MAX_CC_CALLS_DEF_VAL;
  hoVcIpAddress_          = 0;
  otherHoIncomingRtpPort_ = 0;
  memset(&hoGlobalCellId_,  0, sizeof(GlobalCellId_t));

  hoState_                = HAND_ST_TRG_HO_COMPLT;

  DBG_LEAVE();
  return; //(CC_MESSAGE_PROCESSING_COMPLT);
}      

void
CCTargetHandoverExtHo::ccReleaseGpH3(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::ccReleaseGpH3", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_TRACE("{\nMNETeiTRACE(etrg::ccReleaseGpH3): CC External Handover (CC_RELEASE_GP_H3)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
	         hoAgId_,callIndex, hoState_);

  //Cancel whatever timer in case
  parent->sessionTimer->cancelTimer();

  if (h3RtpSession_ != NULL)
  {
	  // break connection
	  T_CSU_PORT_ID rtpSourcePort;
	  T_CSU_RESULT_CON csuResult;
	  
	  rtpSourcePort.portType = CSU_RTP_PORT;
	  rtpSourcePort.portId.rtpHandler = h3RtpSession_;
	  
	  if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort)) != CSU_RESULT_CON_SUCCESS)
	  {
		  DBG_ERROR("{\nMNETeiERROR(etrg::ccReleaseGpH3) CC CSU OrigTerm Break All Error(%d) (hoAgId=%d,callId=%d,hoRtpSession=%x)\n",  
			         csuResult,hoAgId_,callIndex, (int)h3RtpSession_);

		  //Skip this error but may report via OAM if it cares.
	  }
	  
	  ReleaseHandoverRtpChannel(h3RtpSession_);

      h3RtpSession_ = NULL;
  }

  msMsgsQueued_			  = false;
  rrMsgsQueued_			  = false;

  h3SimplexConnected_     = false;
  h3VcCallIndex_          = CC_MAX_CC_CALLS_DEF_VAL;
  h3VcIpAddress_          = 0;
  otherH3IncomingRtpPort_ = 0;
  h3SrcLinkLost_          = false;  //ext-HO FFS ???
  memset(&h3GlobalCellId_,  0, sizeof(GlobalCellId_t));

  hoState_                = HAND_ST_ANCH_HO_COMPLT;

  DBG_LEAVE();
  return; //(CC_MESSAGE_PROCESSING_COMPLT);
}      

void
CCTargetHandoverExtHo::ccReleaseGpHb(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::ccReleaseGpHb", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_TRACE("{\nMNETeiTRACE(etrg::ccReleaseGpHb): CC External Handover (CC_RELEASE_GP_HB)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
	         hoAgId_,callIndex, hoState_);

  //Cancel whatever timer in case
  parent->sessionTimer->cancelTimer();

  if (parent->entryId != (T_CNI_IRT_ID) JCC_UNDEFINED)
  {
      //Break the connections with the RF Port
	  T_CSU_PORT_ID rfSourcePort;
      T_CSU_RESULT_CON csuResult;
	  
	  rfSourcePort.portType = CSU_GSM_PORT;
	  rfSourcePort.portId.gsmHandler = parent->entryId;
	  
	  if ((csuResult = csu_OrigTermBreakAll(&rfSourcePort)) != CSU_RESULT_CON_SUCCESS)
	  {
		  DBG_ERROR("CSU Error : Disconn. Failure, (Result = %d) for (entryId = %d)\n",  
			         csuResult, parent->entryId);
		  
		  // Can only generate OA&M log. 
	  }
  
      IntraL3Msg_t mmOutMsg;

      // Send internal MM msg for release
      sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, parent->entryId, &mmOutMsg);
	  parent->resetIrt();
      parent->entryId = (T_CNI_IRT_ID) JCC_UNDEFINED;
	  parent->oid     = CNI_LAPDM_NULL_OID;
  }


  hoSimplexConnected_ = false; 
  hoState_ = HAND_ST_ANCH_HO_COMPLT;
     
  DBG_LEAVE();
  return; //(CC_MESSAGE_PROCESSING_COMPLT);
}      

void
CCTargetHandoverExtHo::ccReleaseEhHb(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::ccReleaseEhHb", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_TRACE("{\nMNETeiTRACE(etrg::ccReleaseGpHb): CC External Handover (CC_RELEASE_EH_HB)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
	         hoAgId_,callIndex, hoState_);

  //Cancel whatever timer in case
  parent->sessionTimer->cancelTimer();


  hoState_ = HAND_ST_TRG_HO_COMPLT;
 
  DBG_LEAVE();
  return; //(CC_MESSAGE_PROCESSING_COMPLT);
}

void
CCTargetHandoverExtHo::ccReleaseGpEhHb(void)
{
  DBG_FUNC("CCTargetHandoverExtHo::ccReleaseGpEhHb", CC_ETRG_LAYER);
  DBG_ENTER();

  DBG_TRACE("{\nMNETeiTRACE(etrg::ccReleaseGpEhHb): CC External Handover (CC_RELEASE_GP_EH_HB)(hoAgId=%d,callId=%d,hoState=%d)\n}\n",
	         hoAgId_,callIndex, hoState_);

  //Cancel whatever timer in case
  parent->sessionTimer->cancelTimer();

  hoState_ = HAND_ST_ANCH_HO_COMPLT;
 
  DBG_LEAVE();
  return; //(CC_MESSAGE_PROCESSING_COMPLT);
}    
 
int
CCTargetHandoverExtHo::handleVcCcMsg(InterVcMsg_t    *vcCcInMsg)
{
	DBG_FUNC("CCTargetHandoverExtHo::handleVcCcMsg", CC_ETRG_LAYER);
	DBG_ENTER();
    
	CCHandover::handleVcCcMsg(vcCcInMsg);
    
	DBG_LEAVE();
	return(smHandleEvent());
}
    
void
CCTargetHandoverExtHo::initData()
{
  DBG_FUNC("CCTargetHandoverExtHo::initData", CC_ETRG_LAYER);
  DBG_ENTER();

  //Cancel whatever timer in case
  //parent->sessionTimer->cancelTimer();
  vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
  vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_REMOTE_END_HANDOVER;

  switch (hoState_)
  {
  case HAND_ST_ANCH_SND_H3_REQ:     case HAND_ST_ANCH_RCV_H3_REQ_ACK:
       sendVcMsgH3(MODULE_CC, MODULE_CC);
  case HAND_ST_ANCH_SND_HO_REQ:     case HAND_ST_ANCH_RCV_HO_REQ_ACK:
  case HAND_ST_ANCH_HO_COMPLT :     case HAND_ST_ANCH_SND_HB_REQ    :
  case HAND_ST_ANCH_RCV_HB_REQ_ACK: case HAND_ST_ANCH_RCV_HB_REQ    :
  case HAND_ST_ANCH_SND_HB_REQ_ACK: case HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ:
       sendVcMsg(MODULE_CC, MODULE_CC);
       break;

  default:
       break;
  }

  if (hoRtpSession_ != NULL)
  {
	  // break connection
	  T_CSU_PORT_ID rtpSourcePort;
	  T_CSU_RESULT_CON csuResult;
	  
	  rtpSourcePort.portType = CSU_RTP_PORT;
	  rtpSourcePort.portId.rtpHandler = hoRtpSession_;
	  
	  if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort)) != CSU_RESULT_CON_SUCCESS)
	  {
		  DBG_ERROR("{\nMNETeiERROR(etrg::initData) CC CSU OrigTerm Break All Error(%d) (hoAgId=%d,callId=%d,hoRtpSession=%x)\n",  
			         csuResult,hoAgId_,callIndex, (int)hoRtpSession_);

		  //Skip this error but may report via OAM if it cares.
	  }
	  
	  ReleaseHandoverRtpChannel(hoRtpSession_);
  }

  if (h3RtpSession_ != NULL)
  {
	  // break connection
	  T_CSU_PORT_ID rtpSourcePort;
	  T_CSU_RESULT_CON csuResult;
	  
	  rtpSourcePort.portType = CSU_RTP_PORT;
	  rtpSourcePort.portId.rtpHandler = h3RtpSession_;
	  
	  if ((csuResult = csu_OrigTermBreakAll(&rtpSourcePort)) != CSU_RESULT_CON_SUCCESS)
	  {
		  DBG_ERROR("{\nMNETeiERROR(etrg::initData) CC CSU OrigTerm Break All Error(%d) (hoAgId=%d,callId=%d,hoRtpSession=%x)\n",  
			         csuResult,hoAgId_,callIndex, (int)h3RtpSession_);

		  //Skip this error but may report via OAM if it cares.
	  }
	  
	  ReleaseHandoverRtpChannel(h3RtpSession_);
  }


  if (parent->entryId != (T_CNI_IRT_ID) JCC_UNDEFINED)
  {
      //Break the connections with the RF Port
	  T_CSU_PORT_ID rfSourcePort;
      T_CSU_RESULT_CON csuResult;
	  
	  rfSourcePort.portType = CSU_GSM_PORT;
	  rfSourcePort.portId.gsmHandler = parent->entryId;
	  
	  if ((csuResult = csu_OrigTermBreakAll(&rfSourcePort)) != CSU_RESULT_CON_SUCCESS)
	  {
		   DBG_ERROR("{\nMNETeiERROR(etrg::initData) CC CSU OrigTerm Break All Error(%d) (hoAgId=%d,callId=%d,entryId=%d)\n",  
				         csuResult, hoAgId_,callIndex, parent->entryId);
		  
		  // Can only generate OA&M log. 
	  }
  
      // Send internal MM msg for release
      IntraL3Msg_t mmOutMsg;

      sendMM(INTRA_L3_MM_REL_REQ, INTRA_L3_NULL_MSG, parent->entryId, &mmOutMsg);
	  parent->resetIrt();
      parent->entryId = (T_CNI_IRT_ID) JCC_UNDEFINED;
	  parent->oid     = CNI_LAPDM_NULL_OID;
  }

  msMsgsQueued_			  = false;
  rrMsgsQueued_			  = false;

  hoSimplexConnected_     = false;
  hoSrcLinkLost_          = false;  //ext-HO FFS ???
  hoVcCallIndex_          = CC_MAX_CC_CALLS_DEF_VAL;
  hoVcIpAddress_          = 0;
  otherHoIncomingRtpPort_ = 0;
  memset(&hoGlobalCellId_,  0, sizeof(GlobalCellId_t));

  h3SimplexConnected_     = false;
  h3VcCallIndex_          = CC_MAX_CC_CALLS_DEF_VAL;
  h3VcIpAddress_          = 0;
  otherH3IncomingRtpPort_ = 0;
  h3SrcLinkLost_          = false;  //ext-HO FFS ???
  memset(&h3GlobalCellId_,  0, sizeof(GlobalCellId_t));

  currSpeechCh_.version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
  currChannel_.bearerType = BEARER_SPEECH;
  currChannel_.speechChann.version= CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1;
  hoEndCause_ = MC_HO_SUCCESS;
  memset(&currEncrypt_, 0, sizeof(IntraL3CipherModeCommand_t));
    
  parent->handoverSession = NULL;

  hoState_                = HAND_ST_IDLE;

  DBG_LEAVE();
  return; //(CC_MESSAGE_PROCESSING_COMPLT);
}

int
CCTargetHandoverExtHo::handleMMMsg(IntraL3Msg_t *mmMsg,
                                   JCCEvent_t   hoEvent)
{
  DBG_FUNC("CCTargetHandoverExtHo::handleMMMsg", CC_ETRG_LAYER);
  DBG_ENTER();
    
  CCHandover::handleMMMsg(mmMsg, hoEvent);
    
  DBG_LEAVE();
  return(smHandleEvent());  
}

void
CCTargetHandoverExtHo::emptyDownlinkQueue (void)
{
  DBG_FUNC("CCTargetHandoverExtHo::emptyDownlinkQueue", CC_HANDOVER_LAYER);
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
CCTargetHandoverExtHo::addToRRQueue (IntraL3Msg_t      *rrMsg)
{
  DBG_FUNC("CCTargetHandoverExtHo::addToRRQueue", CC_HANDOVER_LAYER);
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
CCTargetHandoverExtHo::emptyRRQueue (void)
{   
  DBG_FUNC("CCTargetHandoverExtHo::emptyRRQueue", CC_HANDOVER_LAYER);
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


JCCEvent_t 
CCTargetHandoverExtHo::handleNullEvent(void)
{
   DBG_FUNC("CCTargetHandoverExtHo::handleNullEvent",CC_ETRG_LAYER);
   DBG_ENTER();

   if (vcInMsg->msgType==INTER_VC_CC_POST_HANDOVER_MOB_EVENT)
   {   
       DBG_CALL_TRACE(hoAgId_,"{\nMNETeiTRACE(etrg::handleNullEvent):[<==GP(%x:%x) MS MESSAGE](callId=%d,hoState=%d)\n}\n",
                      vcInMsg->origVcAddress,vcInMsg->origSubId,callIndex,hoState_);
   } else
   {
       DBG_TRACE("{\nMNETeiTRACE(etrg::handleNullEvent): CC Ignored Event(%d) (hoAgId=%d,callIndex=%d,hoState=%d)\n}\n",
                    currEvent,hoAgId_,callIndex,hoState_);
   }
   DBG_LEAVE();
   return (JCC_NULL_EVENT);
}

void
CCTargetHandoverExtHo::sendL2MsgToGpBCc(T_CNI_LAPDM_L3MessageUnit *msEncodedMsg)
{
   DBG_FUNC("CCAnchorHandover::sendL2MsgToGpBCc", CC_ETRG_LAYER);
   DBG_ENTER();
    
   vcOutMsg.msgType = INTER_VC_CC_POST_HANDOVER_DATA_EVENT;
   vcOutMsg.msgData.postHoMobEventData.msEncodedMsg = *msEncodedMsg;
    
   sendVcMsg(MODULE_CC, MODULE_CC); 
   DBG_LEAVE();
}

JCCEvent_t
CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent(void)
{
  DBG_FUNC ("CCTargetHandoverExtHo::handleAnchToTrgPostHoHoaEvent", CC_ETRG_LAYER);
  DBG_ENTER();

  T_CNI_LAPDM_L3MessageUnit msMsg;


  DBG_TRACE("{\nMNETeiTRACE(etrg::handleAnchToTrgPostHoHoaEvent) Received POST HOA EVENT DUMP (length=%d): \n",
               hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.msglength);

  DBG_HEXDUMP(
              (unsigned char*)&(hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA),
              (int)hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.msglength
                );
  DBG_TRACE("\n END-OF-DUMP\n}\n");

  msMsg.msgLength = hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA[2];
  memcpy( (char*)msMsg.buffer,
          (char*)&(hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA[3]),
          (int)  msMsg.msgLength);


  //If handing around within MNET in progress. Relay the message properly
  switch (hoState_)
  {
  case HAND_ST_TRG_HO_COMPLT:          case HAND_ST_TRG_SND_HB_REQ:
  case HAND_ST_ANCH_VC_IP_ADDRESS_REQ: case HAND_ST_ANCH_SND_HO_REQ:
       //Send MS message to GP-A 
       sendL2MsgToGpACc(&msMsg);
       break;

  case HAND_ST_ANCH_HO_COMPLT:         case HAND_ST_ANCH_SND_HB_REQ:
  case HAND_ST_ANCH_RCV_HB_REQ:        case HAND_ST_ANCH_SND_H3_REQ:
  case HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ:
       //Send MS message to GP-B
       sendL2MsgToGpBCc(&msMsg);
       break;

  case HAND_ST_ANCH_RCV_HO_REQ_ACK:    case HAND_ST_ANCH_SND_HB_REQ_ACK:
  case HAND_ST_ANCH_RCV_H3_REQ_ACK:
       //Queue up MS messages since MNET handover is in progress
       addToDownlinkQueue(&msMsg); 
       break;

  case HAND_ST_TRG_RCV_HB_REQ_ACK:     case HAND_ST_ANCH_RCV_HB_REQ_ACK:
  default:
       DBG_WARNING("{\nMNETeiTRACE(etrg::handleAnchToTrgPostHoHoaEvent): Ignore this MS message from HOA (hoAgId=%d,callId=%d,hoState=%d)\n}\n",
                   hoAgId_,callIndex,hoState_);
       break;
  }

  DBG_LEAVE();
  return(CC_MESSAGE_PROCESSING_COMPLT);
}

bool
CCTargetHandoverExtHo::sendL2MsgToGpACc (T_CNI_LAPDM_L3MessageUnit *msEncodedMsg)
{
  DBG_FUNC ("CCTargetHandoverExtHo::sendL2MsgToGpACc", CC_ETRG_LAYER);
  DBG_ENTER();

  bool retVal = true;
  
  //T_CNI_RIL3_PROTOCOL_DISCRIMINATOR msgPd = extractPd (msEncodedMsg);
  //T_CNI_LAPDM_SAPI sapi  = (msgPd == CNI_RIL3_PD_SMS) ? SAPI_SMS : SAPI_MAIN;
  T_CNI_LAPDM_SAPI sapi = SAPI_MAIN;
  // Retrieve oid for the channel according to SAPI from RR
  T_CNI_LAPDM_OID msgOid;

  //if (sapi == SAPI_MAIN || getSapi3Status())
  {  
      // SAPI=0 message or SAPI=3 connection is already established
      if ((msgOid = rm_GetOid (parent->entryId, sapi)) == CNI_LAPDM_NULL_OID)
      {
          DBG_ERROR ("{\nMNETeiTRACE(etrg::sendL2MsgToGpACc) Invalid OID received from RM (hoAgId=%d,callId=%d, hoState=%d entryId=%d sapi=%d)\n",
                      hoAgId_,callIndex, hoState_, parent->entryId, sapi);
          retVal = false;
      }

      CNI_LAPDM_Dl_Data_Request (msgOid, sapi, msEncodedMsg);
      PostL3SendMsLog (msgOid, sapi, msEncodedMsg);

  }
  //else
  { 
      // Establish SAPI=3 before sending SMS messages to LAPDm
      // sendDLMsg       (L3L2_DL_EST_REQ, SAPI_SMS);
      // Queue message till connection is established
      //sapi3Queue_.add (msEncodedMsg             );
  }

  DBG_LEAVE();
  return (retVal);
}

void
CCTargetHandoverExtHo::addToDownlinkQueue (T_CNI_LAPDM_L3MessageUnit *msEncodedMsg)
{
  DBG_FUNC("CCTargetHandoverExtHo::addToDownlinkQueue", CC_ETRG_LAYER);
  DBG_ENTER();
    
  if (msMsgsQueued_ < MAX_HO_SRC_DOWNLINK_QUEUE_LENGTH)
  {
      downlinkQueue_[msMsgsQueued_] = *msEncodedMsg;
      msMsgsQueued_++;
  }
    
  DBG_LEAVE();
}
    

int
CCTargetHandoverExtHo::handleVbCcMsg (IntraL3Msg_t *vbCcInMsg)
{
  
  JCCEvent_t retEvent;

  DBG_FUNC("CCTargetHandoverExtHo::handleVbCcMsg", CC_ETRG_LAYER);
  DBG_ENTER();
  
   // Copy the ip address from the message 
  UINT32 reVcIpAddress;
  reVcIpAddress = inet_addr(&(vbCcInMsg->l3_data.ipAddressResponse.ViperCellIpAddress[0]) );
 
  DBG_TRACE("{\nMNETeiTRACE(etrg::handleVbCcMsg): Received IP Addr Rsp Msg (hoAgId=%d,callId=%d,hoState=%d,ipAddr=%x,hoCell=%d:%d,h3CellId=%d:%d)\n}\n",
            hoAgId_, callIndex,hoState_, reVcIpAddress,hoGlobalCellId_.lac,hoGlobalCellId_.ci,h3GlobalCellId_.lac,h3GlobalCellId_.ci);
           
  if ((hoState_ != HAND_ST_ANCH_VC_IP_ADDRESS_REQ) && (hoState_ != HAND_ST_ANCH_H3_VC_IP_ADDRESS_REQ ))
  {
      // Do Nothing, late message

      DBG_TRACE("{\nMNETeiTRACE(etrg::handleVbCcMsg): Late IP Address Response ignored (callId=%d,hoState=%d,ipAddr=%x)\n}\n",
                hoAgId_,callIndex, hoState_,reVcIpAddress);

      retEvent = ( (hoState_==HAND_ST_ANCH_VC_IP_ADDRESS_REQ) ? CC_RELEASE_GP_HO:CC_RELEASE_GP_H3);
  }
      
  if (reVcIpAddress == 0) 
  {
      DBG_ERROR ("{\nMNETeiERROR(etrg::handleVbCcMsg): IP Address Failed (hoAgId=%d,callId=%d,hoState=%d)\n",
                  hoAgId_,callIndex,hoState_);
      DBG_ERROR("\n             Check if an adjacent vipercell is still up running properly\n}\n");
    
      retEvent = ( (hoState_==HAND_ST_ANCH_VC_IP_ADDRESS_REQ) ? CC_RELEASE_GP_HO:CC_RELEASE_GP_H3);
  } else
  {
      //Proceed to do handover or hand 3rd

      currEvent = HC_VB_GET_VC_ADDRESS_RSP;
      if (hoState_==HAND_ST_ANCH_VC_IP_ADDRESS_REQ)   hoVcIpAddress_ = reVcIpAddress;
      else                                            h3VcIpAddress_ = reVcIpAddress;
     
      DBG_LEAVE();
      return (smHandleEvent());
 }
    
 DBG_LEAVE();
 return (retEvent);

}
    
