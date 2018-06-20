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
// File        : CCapiH323.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 02-15-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include <stdlib.h>

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "logging/VCLOGGING.h"

#include "voip/voipapi.h"
#include "jcc/JCCUtil.h"
#include "jcc/JCCVcMsg.h"
#include "CC/CCInt.h"
#include "CC/CCconfig.h"
#include "CC/CCTypes.h"
#include "voip/vblink.h"

// Global Variables for current Call Sessions
CCSession_t ccSession[CC_MAX_CC_CALLS_DEF_VAL];
short CC_MAX_CC_CALLS;    //   

bool
checkTransactionId( VOIP_CALL_HANDLE callHandle, 
                    T_CNI_IRT_ID txnId,
                    IntraL3Msg_t *ccOutMsg)
{
  DBG_FUNC("checkTransactionId", CC_LAYER);
  DBG_ENTER();

  short callId;
  short callLegId;

  // get the call id and call leg id bits out
  callId    = (txnId & 0xff);
  callLegId = ((txnId >> 8) & 0xff);

  // Get the Message Queue Id for the Call Control Task
  if ( (0 > callId) ||
       ( callId >= CC_MAX_CC_CALLS) )
    {
      if (callId  == VOIP_API_NULL_TXN_ID)
        {
          // Null value is received for transaction id.
          // send this message to the parent cc task
                  
        }
      else
        {
          DBG_ERROR("H323->CC Error : Bad (txnId = %d) with (Handle = %p)\n",  
                    txnId,
                    (int)callHandle
                    );
          DBG_LEAVE();
          return (false);
        }
    }
  else
    {
      //        if (ccSession[transactionId].h323CallHandle != VOIP_NULL_CALL_HANDLE)
      //              {
      //                if ((ccSession[transactionId].sessionHandler)->findHandle(callHandle))
      //                      {
      //                        // Transaction id is not valid.
      //                        DBG_WARNING("H323->CC Warning: Late Msg. for (Handle = %p) (Txn.Id = %d)\n",
      //                                                (int)callHandle,
      //                                                transactionId );
      //                        DBG_LEAVE();
      //                        return (false);
      //                      }
      //              }
    };

  // check call leg id range
  if ( (callLegId < 0) ||  // When callLegId == 0 signifies non-external handover callp event
       (callLegId > 8) )   // JOE: Replace hardcoded values with CONSTANT.
    {
       DBG_ERROR("H323->CC Error : Bad (txnId = %d) with (Handle = %p)\n",  
                  txnId,
                 (int)callHandle
                    );
       DBG_LEAVE();
       return (false);
    };

  ccOutMsg->entry_id       = callId;
  DBG_LEAVE();
  return (true);
}

bool
CcVoipMessageHandler(VOIP_API_MESSAGE *voipInMsg)
{
  DBG_FUNC("CcVoipMessageHandler", CC_LAYER);
  DBG_ENTER();

  MSG_Q_ID qid;
  IntraL3Msg_t ccOutMsg;
  short callId;
  short callLegId;

  ccOutMsg.module_id      = MODULE_H323;
  ccOutMsg.call_handle    = voipInMsg->VoipCallHandle; 

  //CDR <xxu:08-24-00>
  ccOutMsg.H323CRV = voipInMsg->H323CRV;

  // EHO <chenj:06-25-01>
  // get the call id and call leg id bits out
  callId    = (voipInMsg->TxnId & 0xff);
  callLegId = ((voipInMsg->TxnId >> 8) & 0xff);

  ccOutMsg.callleg_id = callLegId;

  DBG_TRACE("H323->CC API LOG: Sending CC a Msg(%d) callHandle(%d) ludbId(%d) sessionId(%d) callleg (%d)\n",
            (int)voipInMsg->MessageType,(int)voipInMsg->VoipCallHandle, (int)voipInMsg->LudbId,
            (int)callId,
            (int)callLegId);

  switch (voipInMsg->MessageType) {

  //BCT <xxu:09-21-00> BEGIN
  case VOIP_API_CALL_UPDATE_HANDLES:
    {
      if (callId == VOIP_API_NULL_TXN_ID)
        {
          qid = ccMsgQId;
        }
      else
        {
          qid = ccSession[callId].msgQId;
        }

      ccOutMsg.l3_data.voipMsg.callUpdateHandles.NewHandle = voipInMsg->CallUpdateHandles.NewHandle;

      //DBG_TRACE("VOIP->CC(api): BCT update call handles at transferred party:\n");
      //DBG_TRACE("VOIP->CC(api): oldHandle(%x), newHandle(%x), sessionId(%d), ludbId(%d) qId(%x)\n",
      //                          voipInMsg->VoipCallHandle,
      //                          voipInMsg->CallUpdateHandles.NewHandle,
	//				  voipInMsg->TxnId,
	//				  voipInMsg->LudbId,
	//				  qid);
                                
      ccOutMsg.entry_id       = voipInMsg->LudbId;

      ccOutMsg.message_type   = (IntraL3MsgType_t)INTRA_L3_CC_BCT_UPDATE_HANDLES;
      break;
    }
  //BCT <xxu:09-21-00> END

  case VOIP_API_CALL_OFFERING:
    {
      qid = ccMsgQId;

      ccOutMsg.l3_data.setup.callOffering = voipInMsg->CallOffering;

      //        DBG_TRACE("API: calling party number received from Voip:\n");
      //        DBG_TRACE(
      //                          "(ie_present=%d),(numberType=%d),(plan=%d),(numDigits=%d)\n",
      //                          voipInMsg->CallOffering.CallingPartyNumber.ie_present,
      //                          voipInMsg->CallOffering.CallingPartyNumber.numberType,
      //                          voipInMsg->CallOffering.CallingPartyNumber.numberingPlan,
      //                          voipInMsg->CallOffering.CallingPartyNumber.numDigits);

      //        DBG_TRACE("API: called party number received from Voip:\n");
      //        DBG_TRACE("(numDigits=%d), (%d), (%d), (%d), (%d), (%d)\n",
      //                          voipInMsg->CallOffering.CalledPartyNumber.numDigits,
      //                          voipInMsg->CallOffering.CalledPartyNumber.digits[0],
      //                          voipInMsg->CallOffering.CalledPartyNumber.digits[1],
      //                          voipInMsg->CallOffering.CalledPartyNumber.digits[2],
      //                          voipInMsg->CallOffering.CalledPartyNumber.digits[3],
      //                          voipInMsg->CallOffering.CalledPartyNumber.digits[4]
      //                          );

      ccOutMsg.entry_id       = voipInMsg->LudbId;

      ccOutMsg.message_type   = 
        (IntraL3MsgType_t) 
        INTRA_L3_CC_Q931_SETUP;
      break;
    }
        
  case VOIP_API_CODEC_SELECTED:
    {
      if (!(checkTransactionId(voipInMsg->VoipCallHandle,
                               voipInMsg->TxnId,
                               &ccOutMsg)))
        {
          DBG_LEAVE();
          return (false);
        }

      if (ccOutMsg.entry_id == VOIP_API_NULL_TXN_ID)
        {
          qid = ccMsgQId;
        }
      else
        {
          qid = ccSession[ccOutMsg.entry_id].msgQId;
        }

      ccOutMsg.message_type   = INTRA_L3_CC_H245_CHANN_SELECT;
      ccOutMsg.l3_data.voipMsg.codecSelected = voipInMsg->CodecSelected;
      break;
    }

  case VOIP_API_CALL_EVENT:
    {
      if (!(checkTransactionId(voipInMsg->VoipCallHandle,
                               voipInMsg->TxnId,
                               &ccOutMsg)))
        {
          DBG_LEAVE();
          return (false);
        }

      if (ccOutMsg.entry_id == VOIP_API_NULL_TXN_ID)
        {
          qid = ccMsgQId;
        }
      else
        {
          qid = ccSession[ccOutMsg.entry_id].msgQId;
        }

      ccOutMsg.message_type   = (IntraL3MsgType_t)
        (INTRA_L3_CC_Q931_BASE +  (voipInMsg->CallEvent.CallEvent - VOIP_MOBILE_CALL_EVENT_BASE));
      ccOutMsg.l3_data.voipMsg.basicCallInMsg = voipInMsg->CallEvent;
      break;
    }

  default:
    {
      DBG_ERROR("H323->CC API ERROR: Unexpected Msg(%d) callHandle(%d) ludbId(%d) sessionId(%d) callleg(%d)\n",
            (int)voipInMsg->MessageType, (int)voipInMsg->VoipCallHandle, (int)voipInMsg->LudbId,
            (int)callId,
            (int)callLegId);

      DBG_LEAVE();
      return(false);
    }

  }

  // send the message.
  if (ERROR == msgQSend( qid,
                         (char *) &ccOutMsg, 
                         sizeof(struct  IntraL3Msg_t), 
                         NO_WAIT,		//PR1553 <xxu> WAIT_FOREVER,
                         MSG_PRI_NORMAL
                         ) )
    {
      DBG_ERROR("H323->CC API ERROR: sendCC msgQSend (QID = %p) error\n ", 
                (int)qid);
      DBG_LEAVE();
      return (false);
    }
  else
    {
      //DBG_TRACE("H323 Log: Sent Message to CC, (Call Handle = %p), (Msg Type = %d), (reason = %d)\n",  
      //          (int)ccOutMsg.call_handle, 
      //          ccOutMsg.message_type,
      //          voipInMsg->CallEvent.ReasonCode);
      DBG_LEAVE();
      return (true);
    }
}

bool
CcVBLinkMessageHandler(char *cellId, char *ipAddress, TXN_ID txnId)
{
  
  DBG_FUNC("CcVBLinkMessageHandler", CC_LAYER);
  DBG_ENTER();

  MSG_Q_ID qid;
  IntraL3Msg_t ccOutMsg;

  ccOutMsg.module_id      = MODULE_H323;

  DBG_TRACE("VBL->CC API LOG: Sending CC IP Addr Rsp Msg(%d) cellId(%s) ipAddr(%s) sessionId(%d)\n",
             (int)INTRA_L3_CC_RAS_IP_ADDRESS_RESPONSE,
              cellId, ipAddress, txnId);

  if ( (0 > txnId) ||
       (txnId >= CC_MAX_CC_CALLS) )
    {
      DBG_ERROR("VBL->CC API ERROR: Bad txnId(%d)\n",  
                txnId);
      DBG_LEAVE();
      return(false);
    }
  
  ccOutMsg.entry_id = txnId;

  qid = ccSession[ccOutMsg.entry_id].msgQId;

  ccOutMsg.message_type   = INTRA_L3_CC_RAS_IP_ADDRESS_RESPONSE;

  strcpy(&(ccOutMsg.l3_data.ipAddressResponse.ViperCellId[0]), cellId);

  strcpy(&(ccOutMsg.l3_data.ipAddressResponse.ViperCellIpAddress[0]), ipAddress);

  // send the message.
  if (ERROR == msgQSend( qid,
                         (char *) &ccOutMsg, 
                         sizeof(struct  IntraL3Msg_t), 
                         NO_WAIT, 		//PR1553 <xxu> WAIT_FOREVER,
                         MSG_PRI_NORMAL
                         ) )
    {
      DBG_ERROR("VBL->CC API ERROR: sendCC msgQSend (QID = %p) error\n ", 
                (int)qid);
      DBG_LEAVE();
      return(false);
    }
  else
    {
      //DBG_TRACE("VB Link Log: Sent Message to CC (cell id requested = %s)",
      //          cellId);
      DBG_LEAVE();
      return(true);
    }

}


BOOL CcVBLinkCellStatusUpdate (char *pCellId, VBLINK_VC_STATUS_INFO status)
{
  IntraL3Msg_t ccOutMsg;
  int          cellNum ;

	DBG_FUNC  ("CcVBLinkCellStatusUpdate", CC_LAYER);
	DBG_ENTER ();

	ccOutMsg.module_id    = MODULE_H323               ;
	ccOutMsg.call_handle  = 0                         ;
	ccOutMsg.message_type = INTRA_L3_CC_VC_UPDATE_INFO;

	if ((cellNum = atoi (pCellId)) <= 0)
	{
		// Invalid ViperCell ID
		DBG_ERROR ("VBL->CC API ERROR: Sending CC a Msg(%d) w. Invalid CellID(%s) cellNum(%d) status(%d)\n",
                        (int)INTRA_L3_CC_VC_UPDATE_INFO, pCellId, cellNum, status);
  	      DBG_LEAVE ();
		return (FALSE);
	}

	DBG_TRACE("VBL->CC API LOG: Sending CC GP StatusUpdate Msg(%d) cellId(%s) cellNum(%d) StatusUpdate(%d)\n",
                 (int)INTRA_L3_CC_VC_UPDATE_INFO, pCellId, cellNum, status);

	// Correct Network ID is not provided at this time
	ccOutMsg.l3_data.statusVCMsg.btsID.networkId = 0      ;
	ccOutMsg.l3_data.statusVCMsg.btsID.cellId    = cellNum;
	ccOutMsg.l3_data.statusVCMsg.status          = status ;

	// send the message.
	if (ERROR == msgQSend (ccMsgQId                 ,
                          (char *) &ccOutMsg            ,
                           sizeof (struct  IntraL3Msg_t),
                           NO_WAIT			,   //PR1553 <xxu> WAIT_FOREVER
                           MSG_PRI_NORMAL               ) )
    {
      DBG_ERROR ("VBL->CC API ERROR : sendCC msgQSend (QID = %p) error.\n", 
                (int)ccMsgQId);
      DBG_LEAVE ();

      return (FALSE);
    }

    //DBG_TRACE ("ViperLink Log: Sent VC update message to CC, ID = %d, status = %d .\n",  
    //            cellNum, status);
    DBG_LEAVE ();

    return (TRUE);
}


//BCT <xxu:08-11-00> BEGIN
bool
CcVBLinkTransferResponse (
          unsigned short reqSeqNumber, 
          unsigned char  rejectCause,
          T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER   cpn )
{
  
  DBG_FUNC("CCVBLinkTransferResponse", CC_LAYER);
  DBG_ENTER();

  MSG_Q_ID qid;
  IntraL3Msg_t ccOutMsg;

  ccOutMsg.module_id      = MODULE_H323;

  DBG_TRACE("VBL->CC API LOG: Sending BCT No. Rsp reqSeqNo(%d) rejectCause(%d) bctNo(%d,%d,%d,%d,%d,%d)\n",
             reqSeqNumber, rejectCause, cpn.digits[0], cpn.digits[1], cpn.digits[2],  cpn.digits[3], 
             cpn.digits[4], cpn.digits[5]);

  if ( reqSeqNumber >= CC_MAX_CC_CALLS )
  {
      DBG_ERROR("VBL->CC API ERROR: Bad reqSegNumber(%d)\n",  
                 reqSeqNumber);
      DBG_LEAVE();
      return(false);
  }
  
  ccOutMsg.entry_id = reqSeqNumber;

  qid = ccSession[ccOutMsg.entry_id].msgQId;
  ccOutMsg.message_type   = INTRA_L3_CC_RAS_BCT_NUMBER_RESPONSE;
  ccOutMsg.l3_data.bctNumberResponse.reqSeqNumber = reqSeqNumber;
  ccOutMsg.l3_data.bctNumberResponse.rejectCause = rejectCause;
  memcpy(&(ccOutMsg.l3_data.bctNumberResponse.cpn), &cpn, 
                            sizeof(T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER));

  // send the message.
  if (ERROR == msgQSend( qid,
                         (char *) &ccOutMsg, 
                         sizeof(struct  IntraL3Msg_t), 
                         NO_WAIT,					//PR1553 <xxu> WAIT_FOREVER,
                         MSG_PRI_NORMAL
                         ) )
    {
      DBG_ERROR("VBL->CC API ERROR: sendCC msgQSend (QID = %p) error\n ", 
                (int)qid);
      DBG_LEAVE();
      return(false);
    }

    DBG_LEAVE();
    return(true);
}
//BCT <xxu:08-11-00> END
