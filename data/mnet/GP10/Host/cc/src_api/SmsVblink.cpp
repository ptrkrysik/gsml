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
// File        : SmsVblink.cpp
// Author(s)   : Igal Gutkin
// Create Date : 31-03-2000
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
//

#include <vxworks.h>
#include <string.h>

#include "logging\vcmodules.h"
#include "logging\vclogging.h"

#include "jcc\jcccommsg.h"

//#include "voip\vblink.h"

#include "CC\ccint.h"
#include "CC\ccutil.h"

bool smsUseVblink = true; /* Controls whether to forward MNSMS msg's from SMS to 
                             ViperBase and visa versa. */

// Local declarations
static bool smsUnpackVblinkMsg (SmsVblinkMsg_t&, const SmsVblinkMsg_t *, unsigned int);


/* Parser and routing function for the MNSMS messages received from SMR entity */
bool smsReceiveSmrMsg (LUDB_ID ludbIdx, unsigned char *msgData, unsigned int msgLen)
{
  MSG_Q_ID           qid      ;
  bool               retVal   ;
  int                sessionId;
  IntraL3Msg_t       l3MsgOut ;
  SMS_VBLINK_MSGTYPE msgType  ;

  SmsVblinkMsg_t& smsMsgOut = l3MsgOut.l3_data.smsVbMsg;

  DBG_FUNC ("smsReceiveSmrMsg", CC_SMS_LAYER);
  DBG_ENTER();

    DBG_TRACE   ("VBLINK->SMS: received message from ViperBase.\n");
//    DBG_HEXDUMP (msgData, msgLen);

    if (smsUseVblink == false)
    {
        DBG_WARNING ("VBLink->SMS: VBLink interface is OFF. The message will not be forwarded to SMS.\n");
        DBG_LEAVE   ();
        return      (false);
    }

    if (msgLen < SMS_VBLINK_HDRSIZE || msgLen > sizeof (SmsVblinkMsg_t))
    {
        DBG_ERROR ("VBLink->SMS Error : Invalid message length = %d \n", msgLen);        
        DBG_LEAVE ();
        return    (false);
    }

    // Reset the output message pool
	memset (&l3MsgOut, 0, sizeof(IntraL3Msg_t));

    if (smsUnpackVblinkMsg (smsMsgOut, (SmsVblinkMsg_t *)msgData, msgLen) == false)
    {
        DBG_LEAVE ();
        return    (false);
    }

    msgType = ( SMS_VBLINK_MSGTYPE)smsMsgOut.hdr.msgType;

    if (msgType <= MNSMS_MSG_BASE || msgType >= MNSMS_MSG_MAX)
    {
        DBG_ERROR ("VBLink->SMS Error : Unknown message type = %d \n", smsMsgOut.hdr.msgType);        
        DBG_LEAVE ();
        return    (false);
    }

    l3MsgOut.module_id    = MODULE_H323          ;
    l3MsgOut.message_type = INTRA_L3_SMS_MESSAGE ;
    l3MsgOut.entry_id     = ludbIdx              ;

    // The following parameters are not in use
    l3MsgOut.call_handle  = VOIP_NULL_CALL_HANDLE;

    // Forward the message to the appropriate CC entity (either CC main or Call-x task)
    // MNSMS-EST Req always sent to CC Main task in order to avoid race conditions    
    if (msgType != MNSMS_EST_Req && (sessionId = mobInCall(ludbIdx)) != JCC_ERROR)
        qid = ccSession[sessionId].msgQId; // MS is already engaged
    else
        qid = ccMsgQId; // MS is not involved in any, forward to the CC main task

    // send the message
    if (ERROR == msgQSend (qid                  ,
                           (char *)&l3MsgOut    ,
                           sizeof (IntraL3Msg_t), 
                           	NO_WAIT,    //PR1553 <xxu> WAIT_FOREVER,
                           MSG_PRI_NORMAL       ))
    {
      DBG_ERROR ("VBLINK->SMS Error : sendCC msgQSend (QID = %p) error\n ", 
                 (int)qid);
      retVal = false;
    }
    else
    {
      DBG_TRACE ("VBLINK->SMS: Sent Message to SMS, (LUDB = %d), (Msg Type = %d)\n", 
                 ludbIdx, smsMsgOut.hdr.msgType);
      retVal = true;
    }

  DBG_LEAVE();
  return (retVal);
}


static bool smsUnpackVblinkMsg (SmsVblinkMsg_t&       smsMsgOut,
                                const SmsVblinkMsg_t *smsMsgIn ,
                                unsigned int          msgLen   )
{
  DBG_FUNC("smsUnpackVblinkMsg", CC_SMS_LAYER);
  DBG_ENTER();

    // Check integrity of the message
    if ((smsMsgOut.hdr.magic = ntohl (smsMsgIn->hdr.magic)) != SMS_MAGIC_NUMBER)
    {
        // corrupted or non-SMS message
        DBG_ERROR ("VBLINK->SMS Error: Corrupted message is received (magic number = %#x)\n", 
                   smsMsgOut.hdr.magic);
        DBG_LEAVE ();
        return    (false);
    }

    // Check the associated RP message data length
    if ((smsMsgOut.hdr.msgLen = ntohl(smsMsgIn->hdr.msgLen)) > SMS_MAX_RP_MSG_LEN)
    {
        DBG_ERROR ("VBLINK->SMS Error: Invalid RP message data length = %ul is received\n",
                   smsMsgOut.hdr.msgLen);
        DBG_LEAVE ();
        return    (false);
    }

    smsMsgOut.hdr.refNum  = ntohl (smsMsgIn->hdr.refNum );
    smsMsgOut.hdr.msgType = ntohl (smsMsgIn->hdr.msgType);
    smsMsgOut.hdr.status  = ntohl (smsMsgIn->hdr.status );

    memcpy (smsMsgOut.msg, smsMsgIn->msg, smsMsgOut.hdr.msgLen);

    DBG_TRACE   ("VBLINK->SMS: Received message type %d, ref.num = %d, status = %d\n RPDU Data: ", 
                 smsMsgOut.hdr.msgType, smsMsgOut.hdr.refNum, smsMsgOut.hdr.status);
    DBG_HEXDUMP (smsMsgOut.msg, smsMsgOut.hdr.msgLen);

  DBG_LEAVE();
  return   (true);
}

