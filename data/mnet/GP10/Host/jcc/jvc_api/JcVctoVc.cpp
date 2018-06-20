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
// File        : JcVcToVc.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 05-01-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "cc/CCInt.h"

#include "JCC/JcVcToVc.h"
#include "JCC/viperchannel.h"
#include "JCC/JCCbcCb.h"

#include "logging/VCLOGGING.h"

#include "rm/rm_ccintf.h"

#include "cc/CCUtil.h"

#include "cc/CCSessionHandler.h"

#include "JCC/JCCUtil.h"

#include "cc/CCConfig.h"


bool JcMsgRcvByCC (InterVcMsg_t *);
bool JcMsgRcvByRM (InterVcMsg_t *);


bool JcMsgRcvByCC (InterVcMsg_t *msg)
{
  DBG_FUNC ("JcMsgRcvByCC", CC_LAYER);
  DBG_ENTER();

  MSG_Q_ID  qId     ;
  VcCcMsg_t intMsg  ;

  unsigned  buffSize;
  char *    buffer  ;

  bool      retVal = true;

  if (msg->origModuleId == MODULE_MM)
  { // MM->remote CC
      buffSize = sizeof msg->msgData.postHoMobEventData.intraL3Msg  ; // IntraL3Msg_t
      buffer   = (char *)&msg->msgData.postHoMobEventData.intraL3Msg;
  }
  else
  { // CC->remote CC
      buffSize = sizeof intMsg        ;
      buffer   = (char *)&intMsg      ;

      intMsg.module_id = MODULE_EXT_VC;
      intMsg.vcToVcMsg = *msg         ;
  }

  switch (msg->msgType) 
  {
  case INTER_VC_CC_PERFORM_HANDOVER:
    {
      qId = ccMsgQId;
      break;
    }

  case INTER_VC_CC_POST_HANDOVER_DATA_EVENT:
  default:
    {
      if ( (msg->destSubId >= CC_MAX_CC_CALLS_DEF_VAL) || 
		   (msg->origVcAddress == 0) )
		   return true;

      qId     = ccSession[msg->destSubId].msgQId;
      break;
    }
  }

  // send the message.
  if (ERROR == msgQSend (qId            ,
                         buffer         ,
                         buffSize       ,
                         NO_WAIT        ,
                         MSG_PRI_NORMAL ) )
  {
      DBG_ERROR("EXT VC->CC Error : msgQSend (QID = %p) error\n ", 
                (int)qId);
      retVal = false;
  }
  else
  {
      DBG_TRACE("EXT VC->CC Log: Sent Message to CC, (Msg Type = %d)\n",  
                msg->msgType);
      retVal = true;
  }

  DBG_LEAVE();
  return   (retVal);
}


bool JcMsgRcvByRM(InterVcMsg_t *msg)
{
  DBG_FUNC("JcMsgRcvByRM", CC_LAYER);
  DBG_ENTER();

  IntraL3Msg_t                 *rrOutMsg;

  switch (msg->msgType) { 

  case INTER_VC_CC_POST_HANDOVER_DATA_EVENT:
    {

      if ((!ccSession[msg->destSubId].free) &&
          ((ccSession[msg->destSubId].sessionHandler->handoverSession) 
           == (ccSession[msg->destSubId].sessionHandler->targetHandoverSession)))
        {
          rrOutMsg = &msg->msgData.postHoMobEventData.intraL3Msg;
          DBG_LEAVE();
          return(sendRR(rrOutMsg->primitive_type,
                        rrOutMsg->message_type,
                        ccSession[msg->destSubId].sessionHandler->entryId,
                        rrOutMsg));
        }
      else
        {
          DBG_WARNING("EXT VC CC->RM Warning : Unexpected/Late Handover (Msg. Type=%d), (Session Id=%d)\n ", 
                      msg->msgType,
                      msg->destSubId);
        }
      break;
	
    }
  default:
    DBG_ERROR("EXT VC CC->RM Error : Unexpected Handover (Msg. Type=%d), (Session Id=%d)\n ", 
              msg->msgType,
              msg->destSubId);
  }
  
  DBG_LEAVE();
  return(false);
}


void RecvViperChannelMessage(unsigned char *Packet,int DataSize, UINT32 RemoteAddress)
{
  DBG_FUNC("RecvViperChannelMessage", CC_LAYER);
  DBG_ENTER();

  InterVcMsg_t *msg = (InterVcMsg_t *) Packet;
  
  DBG_HEXDUMP(Packet, DataSize);

  switch(msg->destModuleId)
  {
    case MODULE_CC: 
          JcMsgRcvByCC (msg);
      break;

    case MODULE_RM:
      if (msg->origModuleId == MODULE_CC)
          JcMsgRcvByRM (msg);
      else
          JcMsgRcvBySmscbc ((InterRxCbcCbMsg_t*)Packet);
      break;

    default:
      DBG_ERROR ("Inter-Vipercell message received for unexpected (module = %d)\n",
                 msg->destModuleId);
      break;
  }  

  DBG_LEAVE();
}


bool JcMsgSendToVipercell(InterVcMsg_t *msg)
{
  DBG_FUNC("JcMsgSendToVipercell", CC_LAYER);
  DBG_ENTER();

  BOOL retVal;

  if ((retVal = ViperChannelSendMessage((unsigned char *) msg, 
                                        sizeof(struct  InterVcMsg_t),
                                        msg->destVcAddress
                                        ))
      == TRUE)
    {
      DBG_LEAVE();
      return(true);
    }
  else
    {
      DBG_LEAVE();
      return(false);
    }
		  
}

