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
// File        : CCMsgAnal.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : CC types shared with other modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (MM, RR)
#include "CC/CCMsgAnal.h"

#include "jcc/JCCLog.h"
#include "logging/VCLOGGING.h"

#if defined(_NO_ED) || defined(_SOFT_MOB)
    #include "JCC/JCC_ED.h"
#else
    #include "ril3/ril3_common.h"
    #define JCC_RIL3CC_Decode  CNI_RIL3CC_Decode
    #define JCC_RIL3SMS_Decode CNI_RIL3SMS_CP_Decode
#endif

#ifdef _SOFT_MOB
  extern T_CNI_LAPDM_OID softMobLapdmOid[];
#endif

#include "taskLib.h"

// *******************************************************************
// forward declarations.
// *******************************************************************


CCMsgAnalyzer::CCMsgAnalyzer()
  : msDecodeResult(CNI_RIL3_RESULT_SUCCESS)
{
}


JCCEvent_t
CCMsgAnalyzer::mapToCcEvent(T_CNI_RIL3MD_CCMM_MSG      *msInMsg)
{

  DBG_FUNC (taskName(taskIdSelf()), CC_LAYER);
  DBG_ENTER();

  if (msInMsg->primitive_type != L23PT_DL_DATA_IND)
  {
      DBG_ERROR("CC Error : Only Data Ind expected. Bad (Primitive = %d) \n",
                msInMsg->primitive_type);
      return (JCC_NULL_EVENT);
  }

  // call the Decoder function to decode the message
  if ((msDecodeResult = 
       JCC_RIL3CC_Decode 
    (
#ifdef _NO_ED
     TRUE,
#else
  #ifdef _SOFT_MOB
     !((msInMsg->lapdm_oid) < softMobLapdmOid[0]),
  #endif
#endif
        &msInMsg->l3_data,  // input encoded layer-3 msg
        &msDecodedMsg       // output decoded message 
        ))
      != CNI_RIL3_RESULT_SUCCESS)
    {
      
      DBG_ERROR("CC Error : Decoding MS Message (Problem = %d). \n ", msDecodeResult);

      // decoding error. Something wrong with the message. Switch on
      // the return code to handle different types of error
      switch(msDecodeResult) 
      {
      case CNI_RIL3_MANDATORY_IE_MISSING:
      case CNI_RIL3_NON_SEMANTICAL_MANDATORY_IE_ERROR:
        // mandatory IE error. This is a protocol violation from the
        // the peer layer-3 entity 
        causeValue = CNI_RIL3_CAUSE_INVALID_MANDATORY_INFO;
        
        switch(msDecodedMsg.header.message_type) 
        {
          // RIL3CC Call establishment messages:
        case CNI_RIL3CC_MSGID_SETUP:
          return (HC_LOCAL_RELEASE_REQ);
          break;
          
          // RIL3CC Call clearing messages:
        case CNI_RIL3CC_MSGID_DISCONNECT:
          return (HC_LOCAL_DISCONNECT_REQ);
          break;
        case CNI_RIL3CC_MSGID_RELEASE:
          return (HC_LOCAL_RELEASE_REQ);
          break;
        case CNI_RIL3CC_MSGID_RELEASE_COMPLETE:
          return (HC_LOCAL_RELEASE_COMPLETE);
          break;

        case CNI_RIL3CC_MSGID_START_DTMF:
          // Let the MS timeout
          return (JCC_NULL_EVENT);
          break;
          
        case CNI_RIL3CC_MSGID_STATUS:
          return (HC_LOCAL_RELEASE_REQ);
          break;

        default:
          // ignore the message and return status message.
          return(HC_LOCAL_STATUS_ENQUIRY);
          break;
          
        }
        break;

      case CNI_RIL3_INCORRECT_LENGTH:
        // message is too short. Ignore the message. Currently RIL3 does not disntinguish between 
        // this and incorrect length for a particular i.e.. Need to fix.
        return (JCC_NULL_EVENT);
        break;
        
      case CNI_RIL3_UNKNOWN_MESSAGE_TYPE:
        // ignore the message and return status.
        causeValue = CNI_RIL3_CAUSE_MESSAGE_TYPE_NON_EXISTENT;
        return (HC_LOCAL_STATUS_ENQUIRY);
        break;
        
      case CNI_RIL3_NON_IMPERATIVE_MESSAGE_PART_ERROR:
        // not critical. ignore the error and continue as if the ie did not exist.
        break;
        

      default:
        //CAUSE<xxu:03-27-00> BEGIN

        // internal error. continuing may lead to other problems.
        //causeValue = CNI_RIL3_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
        //return (HC_LOCAL_PROTOCOL_PROBLEM);

        causeValue = CNI_RIL3_CAUSE_PROTOCOL_ERROR_UNSPECIFIED;
        return (JCC_NULL_EVENT);
        //CAUSE<xxu:03-27-00> END

        break;
      }
                
    }

  PostL3RecvMsLog(msInMsg->lapdm_oid, msInMsg->sapi, &msInMsg->l3_data);
  // decoded layer-3 message correctly.
  // switch on the message type
  switch(msDecodedMsg.header.message_type) {
    // RIL3CC Call establishment messages:
  case CNI_RIL3CC_MSGID_ALERTING:
    return (THC_LOCAL_ALERTING);
    break;
  case CNI_RIL3CC_MSGID_CALL_CONFIRMED:
    return (HC_LOCAL_CALL_PROGRESS);
    break;
  case CNI_RIL3CC_MSGID_CONNECT:
    return (THC_LOCAL_ANSWER);
    break;
  case CNI_RIL3CC_MSGID_CONNECT_ACKNOWLEDGE:
    return (OHC_LOCAL_CONNECT_ACK);
    break;
  case CNI_RIL3CC_MSGID_EMERGENCY_SETUP:  
    return (OHC_LOCAL_EMERGENCY_ORIGINATION);
    break;
  case CNI_RIL3CC_MSGID_SETUP:
    return (OHC_LOCAL_TERM_ADDRESS);
    break;

  case CNI_RIL3CC_MSGID_START_DTMF:
    return (HC_LOCAL_START_DTMF);
    break;
  case CNI_RIL3CC_MSGID_STOP_DTMF:
    return (HC_LOCAL_STOP_DTMF);
    break;

    // RIL3CC Call clearing messages:
  case CNI_RIL3CC_MSGID_DISCONNECT:
    return (HC_LOCAL_DISCONNECT_REQ);
    break;
  case CNI_RIL3CC_MSGID_RELEASE:
    return (HC_LOCAL_RELEASE_REQ);
    break;
  case CNI_RIL3CC_MSGID_RELEASE_COMPLETE:
    return (HC_LOCAL_RELEASE_COMPLETE);
    break;

    //CH<xxu:11-24-99
  case CNI_RIL3CC_MSGID_HOLD:
    return (HC_EXTNL_HOLD_REQ);
    break;

  case CNI_RIL3CC_MSGID_RETRIEVE:
    return (HC_EXTNL_RETV_REQ);
    break;

  case CNI_RIL3CC_MSGID_STATUS:
    return (HC_LOCAL_STATUS_INFO);
    break;

  case CNI_RIL3CC_MSGID_STATUS_ENQUIRY:
    return (HC_LOCAL_STATUS_ENQUIRY);
    break;

  default:
    // log error
    DBG_ERROR("CC Error: Unexpected CC Message, (Msg Type = %d)\n",  
              msDecodedMsg.header.message_type);
    return (JCC_NULL_EVENT);
    break;
  }

  return (JCC_NULL_EVENT);
}


JCCEvent_t
CCMsgAnalyzer::mapToSmsEvent (T_CNI_RIL3MD_CCMM_MSG      *msInMsg)
{
  JCCEvent_t retVal;
  int        temp  ;
  char       buffer [256];
  unsigned char * pTemp;

  sprintf (buffer, "CCMsgAnalyzer::mapToSmsEvent %s\n", taskName(taskIdSelf()));

  DBG_FUNC (buffer, CC_SMS_LAYER);
  DBG_ENTER();

  if (msInMsg->primitive_type != L23PT_DL_DATA_IND)
  {
      DBG_ERROR("CC Error : Only Data Ind expected. Bad (Primitive = %d) \n",
                msInMsg->primitive_type);
      DBG_LEAVE();
      return   (JCC_NULL_EVENT);
  }

  DBG_TRACE ("SMS PDU received in decoder. Oid = %d, SAPI = %d, Msg lenght = %d\n", 
             msInMsg->lapdm_oid, msInMsg->sapi, msInMsg->l3_data.msgLength        );

//  DBG_HEXDUMP ((UINT8 *)&msInMsg->l3_data, msInMsg->l3_data.msgLength);
/*
  temp  = msInMsg->l3_data.msgLength;
  pTemp = (UINT8 *)&msInMsg->l3_data;

  while (temp > 0)
  {
      DBG_HEXDUMP (pTemp, ((temp <= 16) ? temp : 16)  );
      temp  -= 16;
      pTemp += 16;
  }
*/
  // call the Decoder function to decode the message
  if ((msDecodeResult = 
       JCC_RIL3SMS_Decode 
    (
#ifdef _NO_ED
     TRUE,
#else
  #ifdef _SOFT_MOB
     !((msInMsg->lapdm_oid) < softMobLapdmOid[0]),
  #endif
#endif
     &msInMsg->l3_data,  // input encoded layer-3 msg
     &msDecodedSmsMsg       // output decoded message 
     ))
        != CNI_RIL3_RESULT_SUCCESS)
  {
      DBG_ERROR ("SMS PDU decode error %d\n", msDecodeResult);

      if (msDecodeResult == CNI_RIL3_INCORRECT_LENGTH)
          retVal = JCC_NULL_EVENT; // Discard short message 04.11 9.2.1
      else
          retVal = SMS_EVENT_PROTOCOL_ERROR; // Investigate the nature 
                                             // of the error and act accordinately
                                             // Use SmsLeg3 for it?
  }
  else
  {
      // decoded layer-3 message correctly.
      // switch on the message type
      switch(msDecodedSmsMsg.header.message_type) 
      {
      case CNI_RIL3SMS_MSGID_CP_DATA :
          DBG_TRACE ("Uplink CP-DATA Data: PD = %d, TI = %d, RPDU length=%d:\n",
                     msDecodedSmsMsg.cp_data.header.protocol_descriminator,
                     msDecodedSmsMsg.cp_data.header.si_ti                 ,
                     msDecodedSmsMsg.cp_data.cp_user_data.length          );
          
          temp  = msDecodedSmsMsg.cp_data.cp_user_data.length;
          pTemp = msDecodedSmsMsg.cp_data.cp_user_data.rpdu_data;

          while (temp > 0)
          {
              DBG_HEXDUMP (pTemp, ((temp <= 16) ? temp : 16)  );
              temp  -= 16;
              pTemp += 16;
          }

          retVal = SMS_EVENT_CP_DATA;
          break;

      case CNI_RIL3SMS_MSGID_CP_ACK  :
          DBG_TRACE ("Uplink CP-ACK Info: PD = %d, TI = %d\n"            ,
                     msDecodedSmsMsg.cp_ack.header.protocol_descriminator,
                     msDecodedSmsMsg.cp_ack.header.si_ti                 );
          
          retVal = SMS_EVENT_CP_ACK ;
          break;

      case CNI_RIL3SMS_MSGID_CP_ERROR:
          DBG_TRACE ("Uplink CP-ERROR Data: PD = %d, TI = %d. Cause is %d - %d\n"  ,
                     msDecodedSmsMsg.cp_error.header.protocol_descriminator        ,
                     msDecodedSmsMsg.cp_error.header.si_ti                         ,
                     msDecodedSmsMsg.cp_error.cp_cause.ie_present                  ,
                     msDecodedSmsMsg.cp_error.cp_cause.causeValue                  );

          retVal = SMS_EVENT_CP_ERROR;
          break;

      default:
          // log error
          DBG_ERROR ("CC Session Error: Unexpected SMS Message, (Msg Type = %d)\n",  
                     msDecodedMsg.header.message_type);
          retVal = SMS_EVENT_PROTOCOL_ERROR;
          break;
      }
  }

  DBG_WARNING ("SMS PDU decode result. Event %d\n", retVal);

  PostL3RecvMsLog (msInMsg->lapdm_oid, msInMsg->sapi, &msInMsg->l3_data);

  DBG_LEAVE();
  return   (retVal);
}

