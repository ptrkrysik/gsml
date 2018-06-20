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
// File        : SmsMsgBuilder.cpp
// Author(s)   : Igal Gutkin
// Create Date : 02-10-2000
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

#include "smspp\smspp_inf.h"

#include "CC/CCSessionHandler.h"
#include "CC/SmsLeg.h"
#include "CC/SmsMsgBuilder.h"

bool VBLinkMsToNetSmspp (LUDB_ID, unsigned char *, int);

extern bool smsUseVblink    ; // Controls whether to forward MNSMS msg's to ViperBase
       int  smsDamageCpData = 0; // For Test purposes only!!!

/*===============================================================================*/
/* Ud SMS-related messages pack and send methods */


T_CNI_RIL3_RESULT SmsMsgBuilder::sendCpAck ()
{
  T_CNI_RIL3_RESULT retVal;

  DBG_FUNC ("SmsMsgBuilder::buildCpAck", CC_SMS_LAYER);
  DBG_ENTER();

    if (buildMsMsgCommon ())
    {
        msOutMsg_.header.message_type = CNI_RIL3SMS_MSGID_CP_ACK;
        retVal = sendMsMsg ();
    }
    else
        retVal = CNI_RIL3_NON_CC_MESSAGE; // Used to indicate an error 

  DBG_LEAVE();
  return   (retVal);
}


T_CNI_RIL3_RESULT SmsMsgBuilder::sendCpError (T_CNI_RIL3_CP_CAUSE_VALUE cause)
{
  T_CNI_RIL3_RESULT retVal;

  DBG_FUNC ("SmsMsgBuilder::buildCpError", CC_SMS_LAYER);
  DBG_ENTER();

    if (buildMsMsgCommon ())
    {
        msOutMsg_.header.message_type          = CNI_RIL3SMS_MSGID_CP_ERROR;
        msOutMsg_.cp_error.cp_cause.ie_present = true                      ;
        msOutMsg_.cp_error.cp_cause.causeValue = cause                     ;

        retVal = sendMsMsg ();
    }
    else
        retVal = CNI_RIL3_NON_CC_MESSAGE; // Used to indicate an error 

  DBG_LEAVE();
  return   (retVal);
}


T_CNI_RIL3_RESULT SmsMsgBuilder::sendCpData ()
{
  T_CNI_RIL3_RESULT retVal;

  DBG_FUNC ("SmsMsgBuilder::buildCpData", CC_SMS_LAYER);
  DBG_ENTER();

  const T_CNI_RIL3_IE_CP_USER_DATA & rpdu = parent_.getHc()->getRpdu();

    if (buildMsMsgCommon ())
    {
        msOutMsg_.header.message_type = CNI_RIL3SMS_MSGID_CP_DATA;

        if (rpdu.ie_present)
        {
            memcpy (&(msOutMsg_.cp_data.cp_user_data), &rpdu, 
                    sizeof(T_CNI_RIL3_IE_CP_USER_DATA)      );
/*
            DBG_TRACE   ("SMS CP-DATA: RPDU length = %u Is present = %d\n",
                         msOutMsg_.cp_data.cp_user_data.length            , 
                         msOutMsg_.cp_data.cp_user_data.ie_present        );

            DBG_HEXDUMP (msOutMsg_.cp_data.cp_user_data.rpdu_data, 
                         msOutMsg_.cp_data.cp_user_data.length   );
*/
            retVal = sendMsMsg ();
        }
        else
            retVal = CNI_RIL3_MANDATORY_IE_MISSING;
    }
    else
        retVal = CNI_RIL3_NON_CC_MESSAGE; // Used to indicate an error 

  DBG_LEAVE();
  return   (retVal);
}


bool SmsMsgBuilder::buildMsMsgCommon ()
{
  bool retVal = true;

  DBG_FUNC ("SmsMsgBuilder::buildMsCommon", CC_SMS_LAYER);
  DBG_ENTER();

  T_CNI_RIL3_SI_TI ti = parent_.getTi();

    if (ti == SMS_TI_INVALID)
    {
        DBG_WARNING ("SMS PDU builder error: TI isn't initialized\n");
        retVal = false;
    }
    else
    {
        // But, before we start populating the message content, 
        // zap it clean first.
        CNI_RIL3_ZAPMSG (&msOutMsg_, sizeof(T_CNI_RIL3SMS_CP_MSG));
    
        if (parent_.getSmsLegType () == SMS_MO_LEG)
            ti |= TI_ORIG_OR_MASK; 

        // CP message header portion
        msOutMsg_.header.protocol_descriminator = CNI_RIL3_PD_SMS;
        msOutMsg_.header.si_ti                  = ti             ;

        DBG_TRACE ("SMS->MS: Build downlink PDU: PD = %d, TI = %d\n",
                   msOutMsg_.header.protocol_descriminator          ,
                   msOutMsg_.header.si_ti                           );
    }

  DBG_LEAVE();
  return   (retVal);
}


T_CNI_RIL3_RESULT SmsMsgBuilder::sendMsMsg ()
{
  T_CNI_RIL3_RESULT         msOutResult ;
  T_CNI_LAPDM_L3MessageUnit msEncodedMsg;

  DBG_FUNC ("SmsMsgBuilder::sendMSMsg", CC_SMS_LAYER);
  DBG_ENTER();

    // call the Encoder function to encode the message
    msOutResult = CNI_RIL3SMS_CP_Encode (&msOutMsg_    ,  // input message definition
                                         &msEncodedMsg);  // encoded layer-3 message

    if (smsDamageCpData                                           && 
        msOutMsg_.header.message_type == CNI_RIL3SMS_MSGID_CP_DATA  )
    {  // intentional damage CP-DATA PDU for testing purpose
        if (smsDamageCpData == 1) 
        {
            msEncodedMsg.buffer[1] = 3; /* Change CP-PDU type. Fatal error */
            DBG_WARNING ("Damaging CP-DATA PDU type!!!\n");
        }
        if (smsDamageCpData == 2)
        {
            msEncodedMsg.buffer[2] = 0; /* Put RPDU lenght to 0. No RP-ACK will be sent by MS */
            DBG_WARNING ("Damaging CP-DATA PDU. Reset RPDU length to 0!!!\n");
        }
        if (smsDamageCpData == 3)
        {
            msEncodedMsg.buffer[2] = 0x03; /* Put RPDU lenght to 0. No RP-ACK will be sent by MS */
            DBG_WARNING ("Damaging CP-DATA PDU. Reset RPDU msg type to invalid value 0x03!!!\n");
        }
    }

    // check the encoding result before sending the message
    if (msOutResult != CNI_RIL3_RESULT_SUCCESS)
    {
        DBG_ERROR ("SMS Error: Encoding CP PDU (Problem = %d) \n ", msOutResult);
    }
    else
    {
        // encoding successful, send the message to LAPDm for delivery
        parent_.parent->sendL2Msg (&msEncodedMsg);

        DBG_TRACE   ("SMS->MS: Sent PDU (Type = %d, Length = %d)to LAPDm\n", 
                     msOutMsg_.header.message_type, msEncodedMsg.msgLength );
        DBG_HEXDUMP ((unsigned char *)msEncodedMsg.buffer, msEncodedMsg.msgLength);
    }

  DBG_LEAVE();
  return   (msOutResult);
} 


/*===============================================================================*/
/* VBLink SMS-related pack and send methods */

bool SmsMsgBuilder::sendMnSmsEstInd   (UINT32 dataLength, UINT8* rpdu)
{
  bool retVal;

  DBG_FUNC ("SmsMsgBuilder::sendMnSmsEstInd", CC_SMS_LAYER);
  DBG_ENTER();

    // Allocate and store new VB ref. number
    setVbRefNum ();

    retVal = sendVbMsg (MNSMS_EST_Ind, dataLength, rpdu);

  DBG_LEAVE();
  return   (retVal);
}


bool SmsMsgBuilder::sendMnSmsDataInd  (UINT32 dataLength, UINT8* rpdu)
{
  bool retVal;

  DBG_FUNC ("SmsMsgBuilder::sendMnSmsDataInd", CC_SMS_LAYER);
  DBG_ENTER();

    retVal = sendVbMsg (MNSMS_DATA_Ind, dataLength, rpdu);

  DBG_LEAVE();
  return   (retVal);
}


bool SmsMsgBuilder::sendMnSmsErrorInd (T_CNI_RIL3_RP_CAUSE_VALUE cause, UINT32 tempRefNum = 0)
{
  bool retVal = true;

  DBG_FUNC ("SmsMsgBuilder::sendMnSmsErrorInd", CC_SMS_LAYER);
  DBG_ENTER();

    retVal = sendVbMsg (MNSMS_ERROR_Ind, 0, NULL_PTR, cause, tempRefNum);

  DBG_LEAVE();
  return   (retVal);
}


bool SmsMsgBuilder::sendVbMsg (SMS_VBLINK_MSGTYPE  msgType    , 
                               UINT32              msgLength  , 
                               UINT8              *msgData    , 
                               UINT32              status     = 0,
                               UINT32              tempRefNum = 0)
{
    /* status     - disconnect cause or zero otherwise
       tempRefNum - Used to reject incorrect message 
       that doesn't belong to the current SMS session */

  int             temp   ;
  unsigned char * pTemp  ;
  bool            retVal = true;

  DBG_FUNC ("SmsMsgBuilder::sendVbMsg", CC_SMS_LAYER);
  DBG_ENTER();

  UINT32 refNum = (tempRefNum) ? tempRefNum : getVbRefNum();

    CNI_RIL3_ZAPMSG (&vbOutMsg_, sizeof(SmsVblinkMsg_t));

    if (refNum == INVALID_VC_SMS_REF_NUM)
    {
        DBG_ERROR ("SMS->VBLINK: VB ref. number is not initialized. \
 Abort sending VB Msg (type = %d)\n", msgType);

        DBG_LEAVE();
        return   (false);
    }

    if (msgLength <= SMS_MAX_RP_MSG_LEN                      &&  
        (msgType > MNSMS_MSG_BASE && msgType < MNSMS_MSG_MAX)  )
    {
        // prepare message data. Ensure the network byte order for the header elements
        vbOutMsg_.hdr.magic   = htonl (SMS_MAGIC_NUMBER);
        vbOutMsg_.hdr.refNum  = htonl (refNum          );
        vbOutMsg_.hdr.msgType = htonl (msgType         );
        vbOutMsg_.hdr.status  = htonl (status          );
        vbOutMsg_.hdr.msgLen  = htonl (msgLength       );

        if (msgLength)
        { // copy RP message raw data
            memcpy (vbOutMsg_.msg, msgData, msgLength);
        }

        DBG_TRACE   ("SMS->VBLINK: Attempt to send MNSMS message (type %u) from entity (LUDB = %d),\n\
  ref number %u, status = %u, RPDU length = %u. RPDU dump: \n", 
                     msgType, parent_.ludbIndex(), refNum, status, msgLength);

//        DBG_HEXDUMP ((unsigned char *)&vbOutMsg_.msg, msgLength);

        temp  = msgLength;
        pTemp = (unsigned char *)&vbOutMsg_.msg;

        while (temp > 0)
        {
            DBG_HEXDUMP (pTemp, ((temp <= 16) ? temp : 16)  );
            temp  -= 16;
            pTemp += 16;
        }

        // sent message to VBLink
        if (smsUseVblink)
        {
            // The total message length calculation could be wrong 
            // if ported to the different platform or 
            // compiled with the word alignment other than 4 bytes
            retVal = VBLinkMsToNetSmspp (parent_.parent->ludbIndex     ,
                                         (unsigned char *)&vbOutMsg_   ,
                                         SMS_VBLINK_HDRSIZE + msgLength); 
        }
        else
        {
            DBG_WARNING ("SMS->VBLINK: VBLink interface is OFF. Message will not be send to ViperBase.\n");
            DBG_HEXDUMP ((unsigned char *)&vbOutMsg_, sizeof (SmsVblinkMsg_t));
            retVal = false;
        }
    }
    else
    {
        DBG_ERROR ("SMS->VBLINK: VBLink message data (Msg type %d) is invalid.\n", msgType);
        retVal = false;
    }

  DBG_LEAVE();
  return   (retVal);
}


UINT32 SmsMsgBuilder::getVbRefNum   ()
{
    return (parent_.getVbRefNum());
}


// Allocate and store new VB ref. number
void   SmsMsgBuilder::setVbRefNum ()
{
  DBG_FUNC ("SmsMsgBuilder::setVbRefNum", CC_SMS_LAYER);
  DBG_ENTER();

    parent_.setVbRefNum ();

  DBG_LEAVE();
}


