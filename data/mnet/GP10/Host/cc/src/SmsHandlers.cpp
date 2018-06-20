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
// File        : SmsHandlers.cpp
// Author(s)   : Igal Gutkin
// Create Date : 01-20-2000
// Description : Contains handlers for the SMS-PP FSM
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
//

#include <vxworks.h>

#include "logging\vclogging.h"
#include "jcc\LUDBApi.h"

#include "CC\CallConfig.h"
#include "CC\CCUtil.h"
#include "CC\CCSessionHandler.h"

#include "CC\SmsLeg.h"
#include "CC\SmsHalfCall.h"

#define SMS_SEND_ABORT_TIMEOUT ((int)(SMS_WAIT_DISCONNECT * 1.5))


/* SMS Event handlers */
/* Handlers will return true is the Leg should be released, otherwise false */

/* ========================================================== */
/*               SMS Default Event handler                    */

bool smsHandlerDef (SmsHalfCall *hcall, void *data, INT32 param1)
{
  DBG_FUNC  ("smsHandlerDef", CC_SMS_LAYER);
  DBG_ENTER ();

  SmsLeg & leg = hcall->getParent();

    DBG_WARNING ("SMS: Event inconsistent with protocol state %d received by %s SMS leg %d\n", 
                 hcall->getState(), (leg.getSmsLegType() == SMS_MO_LEG) ? "MO" : "MT", 
                 (leg.callLegNum - SMS_LEGNUM_OFFSET));

  DBG_LEAVE();
  return   (false);
}


/* ========================================================== */
/*               DL Layer handlers                            */

bool smsHandlerMdErrInd  (SmsHalfCall *hcall, void * data, INT32 param1)
{
  DBG_FUNC  ("smsHandlerMdErrInd", CC_SMS_LAYER);
  DBG_ENTER ();

    SmsLeg & leg = hcall->getParent();

    hcall->smsTimer_->cancelTimer();

    // take cause in account
    leg.setRemRelCause (CNI_RIL3_RP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED);

  DBG_LEAVE();
  return   (true);
}


/* ========================================================== */
/*               MM handlers                                  */

bool smsHandlerMmEstInd (SmsHalfCall *hcall, void *data, T_CNI_RIL3_SI_TI newTi)
{
  bool       retVal = false             ;
  SmsLeg &   leg    = hcall->getParent();

  DBG_FUNC  ("smsHandlerMmEstInd", CC_SMS_LAYER);
  DBG_ENTER ();

    // Currently will reject second MO SMS request !!!
    // Later will take care of concatenated MO SMS here:
    //   will not reject the message with different TI 
    //   while waiting for the final CP-ACK
    if (hcall->getState () == SMS_STATE_IDLE)
    {
        // set transaction Id - first message for SMS
        // The starting entity uses 0. Other side uses 1.
        // hcall->getParent().setTi (newTi | TI_ORIG_OR_MASK);

        DBG_TRACE ("SMS Leg %d: Set TI = %d\n", (leg.callLegNum - SMS_LEGNUM_OFFSET),
                                                 leg.getTi());

        // start the timer while waiting for the first CM level setup message
        // Wait for the following SMS PDU
        hcall->smsTimer_->setTimer (CALL_MM_CONN_EST_T999);
    }
    else
    {
        ; // Yet unexpected. Will be handled later
    }

  DBG_LEAVE();
  return   (retVal);
}


bool smsHandlerMmEstCnf  (SmsHalfCall *hcall, void * data, INT32 param1)
{
  int result;
  DBG_FUNC  ("smsHandlerMmEstCnf", CC_SMS_LAYER);
  DBG_ENTER ();

  CCSessionHandler * session = hcall->getParent().parent;
  bool               retVal  = false                    ;

    hcall->smsTimer_->cancelTimer ();

    /* MM connection has been established.
       Forward CP-DATA to the session     */
    if ((result = hcall->getParent().msgBuilder->sendCpData())
                == CNI_RIL3_RESULT_SUCCESS                    )
    {
        hcall->setNextState        (SMS_STATE_WAIT_CP_ACK);
        hcall->smsTimer_->setTimer (SMS_WAIT_CP_ACK_TC1  );
    }
    else
    {
        DBG_ERROR ("SMS: Unable to send CP-DATA. Error code %d\n", result);
        retVal = true; // Release connection on packing error
    }

  DBG_LEAVE();
  return   (retVal);
}


bool smsHandlerMmEstRej  (SmsHalfCall *hcall, void * data, INT32 param1)
{
  DBG_FUNC  ("smsHandlerMmEstRej", CC_SMS_LAYER);
  DBG_ENTER ();

    hcall->smsTimer_->cancelTimer();

    // Set error code for SMR to identify the case when Page request failed
    // This error code is used ONLY inside ViperNet between VC and VB based
    // SMS Entities and should not be forwarded to the SM-SC
    hcall->getParent().setRemRelCause (CNI_RIL3_RP_CAUSE_UNKNOWN_SUBSCRIBER);

  DBG_LEAVE();
  return   (true);
}


bool smsHandlerMmRelInd (SmsHalfCall *hcall, void *msgData, INT32 param1)
{
  DBG_FUNC  ("smsHandlerMmRelInd", CC_SMS_LAYER);
  DBG_ENTER ();

    hcall->smsTimer_->cancelTimer();

  DBG_LEAVE();
  return   (true);
}


/* ========================================================== */
/*              MS PDU handlers                               */

bool smsHandlerCpData (SmsHalfCall *hcall, T_CNI_RIL3SMS_MSG_CP_DATA *msgCpData, INT32 param1)
{
  DBG_FUNC  ("smsHandlerCpData", CC_SMS_LAYER);
  DBG_ENTER ();

  int               result ;
  bool              retVal = false                  ;
  SmsLeg          & leg    = hcall->getParent()     ;
  T_CNI_RIL3_SI_TI  oldTi  = leg.getTi()            ,
                    newTi  = msgCpData->header.si_ti & TI_TERM_AND_MASK;

    // Cancel timer T999
    hcall->smsTimer_->cancelTimer();

    DBG_TRACE ("CP-DATA received by %s SMS leg %d, TI = %d\n", 
               (leg.getSmsLegType() == SMS_MO_LEG) ? "MO" : "MT",
               (leg.callLegNum - SMS_LEGNUM_OFFSET), msgCpData->header.si_ti);

    if (leg.getSmsLegType() == SMS_MO_LEG                             &&
        (oldTi == SMS_TI_INVALID                                    ||    // Condition 1.
        (oldTi != newTi && hcall->getState() == SMS_STATE_WAIT_CP_ACK)) ) // Condition 2.
    {
        /* Valid for MO SMS Leg only!
           1. For the very first CP PDU received in the dialog OR
           2. Received next part of a concatenated MO message
              described in GSM 04.11, ver 6.0.1, 5.4, page 23.
           Accept and save received TI */
        leg.setTi (newTi);
    }
    else if (oldTi == newTi)
    { // Normal case. Consecutive PDU in the trasfer
        ;
    }
    else 
    { // Handle unexpected CP-DATA
        retVal = smsHandlerCpReject (hcall, (T_CNI_RIL3SMS_CP_MSG *)msgCpData, param1);

        DBG_LEAVE();
        return   (retVal);
    }

    // send CP-ACK
    if ((result = leg.msgBuilder->sendCpAck())
                == CNI_RIL3_RESULT_SUCCESS    )
    {
        if (leg.getSmsLegType() == SMS_MO_LEG)
        {   // MO (first uplink PDU) - send MNSMS-EST ind (msgCpData->cp_user_data) via VBLink
            leg.msgBuilder->sendMnSmsEstInd  (msgCpData->cp_user_data.length   ,
                                              msgCpData->cp_user_data.rpdu_data);
        }
        else
            leg.msgBuilder->sendMnSmsDataInd (msgCpData->cp_user_data.length   ,
                                              msgCpData->cp_user_data.rpdu_data);
    }
    else
    {
        retVal = true; // Release connection on packing error
        DBG_ERROR ("SMS: Unable to send CP-ACK. Error code %d\n", result);
    }

  DBG_LEAVE();
  return   (false);
}


// Response for CP-ACK. Valid in Wait_for_CP-ACK state
bool smsHandlerCpAck (SmsHalfCall *hcall, T_CNI_RIL3SMS_MSG_CP_ACK *msgCpAck, INT32 param1)
{
  bool    retVal = false;

  DBG_FUNC  ("smsHandlerCpAck", CC_SMS_LAYER);
  DBG_ENTER ();

    hcall->smsTimer_->cancelTimer();

    if (hcall->getState() == SMS_STATE_DISCONNECT)
    {
        /* This is an ack for the last CP-DATA in a dialog. Release the leg. */
        hcall->getParent().resetVbRefNum (); // no need to notify SMR
        retVal = true; 
    }
    else
        hcall->resetRpdu(); // Cleanup RPDU storage

  DBG_LEAVE();
  return   (retVal);
}


bool smsHandlerCpError (SmsHalfCall *hcall, T_CNI_RIL3SMS_MSG_CP_ERROR* msgCpError, INT32 param1)
{
  DBG_FUNC  ("smsHandlerCpError", CC_SMS_LAYER);
  DBG_ENTER ();

    hcall->smsTimer_->cancelTimer();

    hcall->getParent().setRemRelCause ((T_CNI_RIL3_RP_CAUSE_VALUE)
                                       msgCpError->cp_cause.causeValue);

  DBG_LEAVE();
  return   (true);
}


// Used to notify MS about receiving unexpected PDU 
bool smsHandlerCpReject (SmsHalfCall *hcall, T_CNI_RIL3SMS_CP_MSG* msgCpMsg, INT32 param1)
{
  DBG_FUNC  ("smsHandlerCpReject", CC_SMS_LAYER);
  DBG_ENTER ();

  int     result ;
  bool    retVal = false;
  SmsLeg& leg    = hcall->getParent();

    hcall->smsTimer_->cancelTimer();

    leg.setRemRelCause (CNI_RIL3_RP_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE);

    // send CP-ERROR (Error = Msg not compatible with the protocol state[98])
    if ((result = leg.msgBuilder->sendCpError(CNI_RIL3_CP_CAUSE_MSG_NOT_COMPATIBLE))
                == CNI_RIL3_RESULT_SUCCESS                                          )
    {
        hcall->setNewState         (SMS_STATE_DISCONNECT  );
        hcall->smsTimer_->setTimer (SMS_SEND_ABORT_TIMEOUT);
    }
    else
    {
        DBG_ERROR ("SMS: Unable to send CP-ERROR. Error code %d\n", result);
        retVal = true;
    }

  DBG_LEAVE();
  return   (retVal);
}


/* ========================================================== */
/*               VB Event handlers                            */

// Use it for the accepted MT setup only. Otherwise, invoke reject method 
bool smsHandlerMnEstReq  (SmsHalfCall               *hcall    ,
                          SmsVblinkMsg_t            *vbMsgData,
                          T_CNI_RIL3_CP_CAUSE_VALUE  cause    )
{
  DBG_FUNC  ("smsHandlerMnSmsEstReq", CC_SMS_LAYER);
  DBG_ENTER ();

  bool          retVal  = false             ;
  SmsLeg      & leg     = hcall->getParent();
  IntraL3Msg_t  mmOutMsg;

    // Save RP data first
    hcall->saveRpdu (vbMsgData);

    // Initiate MM Setup (send MM-Est Req)

    // Fill the data for MM message
    mmOutMsg.l3_data.pageReq.imsi = ludbGetMobileId (leg.ludbIndex());
    mmOutMsg.l3_data.pageReq.qid  = leg.msgQId;

    // Store received VB ref. number
    leg.setVbRefNum (vbMsgData->hdr.refNum);
    leg.allocateTi  ();

    if (leg.parent->isMmConnected() == false)
    { // MM connection yet to be established
        DBG_TRACE ("SMS: handover session = %p, anchor = %p, target = %p\n entryId %d\n",
                   leg.parent->handoverSession, leg.parent->anchorHandoverSession,
                   leg.parent->targetHandoverSession, leg.parent->entryId);

        sendMM (INTRA_L3_MM_EST_REQ, INTRA_L3_MM_LOCATE_MS, 
                leg.entryId()      , &mmOutMsg            );

        hcall->smsTimer_->setTimer (CALL_MM_CONN_EST_T999);
    }
    else 
    { /* Continue with the process by forwarding CP-DATA to MS */
        retVal = smsHandlerMmEstCnf (hcall, NULL_PTR, 0);
    }

  DBG_LEAVE();
  return   (retVal);
}


bool smsHandlerMnDataReq (SmsHalfCall               *hcall    , 
                          SmsVblinkMsg_t            *vbMsgData, 
                          T_CNI_RIL3_CP_CAUSE_VALUE  cause    )
{
  bool retVal = false;
  int  result;

  DBG_FUNC  ("smsHandlerMnSmsDataReq", CC_SMS_LAYER);
  DBG_ENTER ();

    hcall->smsTimer_->cancelTimer();

    // Save RP data first
    hcall->saveRpdu (vbMsgData);

    // Send CP-DATA
    if ((result = hcall->getParent().msgBuilder->sendCpData())
                == CNI_RIL3_RESULT_SUCCESS                    )
    {
        if (vbMsgData->hdr.status)
            hcall->setNextState (SMS_STATE_DISCONNECT);
                                     /* Indicates that this is a final message 
                                        from SMS-SC and the leg should be 
                                        disconnected upon completion.        */
        hcall->getParent().resetVbRefNum ();  /* prevents sending notification to SMR on delivery error */

        hcall->smsTimer_->setTimer (SMS_WAIT_CP_ACK_TC1);
    }
    else
    {
        retVal = true; // Release connection on packing error
        DBG_ERROR ("SMS: Unable to send CP-DATA. Error code %d\n", result);
    }

  DBG_LEAVE();
  return   (retVal);
}


bool smsHandlerMnAbortReq (SmsHalfCall               *hcall    ,
                           SmsVblinkMsg_t            *vbMsgData,
                           T_CNI_RIL3_CP_CAUSE_VALUE  cause    )
{
  DBG_FUNC  ("smsHandlerMnAbortReq", CC_SMS_LAYER);
  DBG_ENTER ();

  int     result ;
  bool    retVal = false;
  SmsLeg& leg    = hcall->getParent();

    hcall->smsTimer_->cancelTimer();

    // send CP-ERROR
    if ((result = leg.msgBuilder->sendCpError(cause))
                == CNI_RIL3_RESULT_SUCCESS           )
    {
        leg.resetVbRefNum ();
        hcall->smsTimer_->setTimer (SMS_SEND_ABORT_TIMEOUT);
    }
    else
    {
        DBG_ERROR ("SMS: Unable to send CP-ERROR. Error code %d\n", result);
        retVal = true;
    }

  DBG_LEAVE();
  return   (retVal);
}


bool smsHandlerMnRelReq  (SmsHalfCall               *hcall    , 
                          SmsVblinkMsg_t            *vbMsgData, 
                          T_CNI_RIL3_CP_CAUSE_VALUE  cause    )
{
  DBG_FUNC  ("smsHandlerMnRelReq", CC_SMS_LAYER);
  DBG_ENTER ();

  SmsLeg & leg = hcall->getParent();

    hcall->smsTimer_->cancelTimer   ();
    leg.resetVbRefNum();

    /* This will allow us to send the final CP-ACK before the channel is released. */
    if (leg.getSmsLegType() == SMS_MT_LEG          &&      
        hcall->getState  () == SMS_STATE_CONNECTED   )
    {
        hcall->smsTimer_->setTimer (SMS_WAIT_DISCONNECT );
    }

  DBG_LEAVE();
  return   (false);
}


/* ========================================================== */
/*               Timer handlers                               */

// Fatal SMS timeout handling
bool smsHandlerTimeout (SmsHalfCall *hcall, void *data, INT32 param1)
{
  IntraL3Msg_t mmOutMsg;

  DBG_FUNC  ("smsHandlerTimeout", CC_SMS_LAYER);
  DBG_ENTER ();

    hcall->smsTimer_->cancelTimer();

    // Igal: will take cause in account later

  DBG_LEAVE();
  return   (true);
}


// Use it in Wait for CP-ACK state
bool smsHandlerTimeoutAck (SmsHalfCall *hcall, void *data, INT32 param1)
{
  int          result  ;
  IntraL3Msg_t mmOutMsg;
  bool         retVal  = false;

  DBG_FUNC  ("smsHandlerTimeoutAck", CC_SMS_LAYER);
  DBG_ENTER ();

    if (hcall->isLastAttemt())
        retVal = smsHandlerTimeout (hcall, data, param1); // second attempt has failed
    else
    { // resend CP-DATA
        hcall->smsTimer_->cancelTimer();

        hcall->setLastAttemt();

        // send CP-DATA
        if ((result = hcall->getParent().msgBuilder->sendCpData())
                    == CNI_RIL3_RESULT_SUCCESS                    )
        {
            hcall->smsTimer_->setTimer (SMS_WAIT_CP_ACK_TC1);
        }
        else
        {
            DBG_ERROR ("SMS: Unable to send CP-DATA. Error code %d\n", result);
            retVal = true; // Release connection on packing error
        }
    }

  DBG_LEAVE();
  return   (retVal);
}


// Use it state Wait for disconnect state
bool smsHandlerTimerDisc (SmsHalfCall *hcall, void *data, INT32 param1)
{
  DBG_FUNC  ("smsHandlerTimerDisc", CC_SMS_LAYER);
  DBG_ENTER ();

  bool retVal = true;

    // No need to notify Remote side
    hcall->getParent().resetVbRefNum ();

    if (hcall->isRpdu())
    { // This timeout is due to TC1 expiry for a last CP-DATA in a dialog.
      // Possibly need to resend it.
        retVal = smsHandlerTimeoutAck (hcall, data, param1);
    }

  DBG_LEAVE();
  return   (retVal);
}


