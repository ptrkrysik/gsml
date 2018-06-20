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
// File        : CCMobUtil.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "stdio.h"

#include "jcc/JCCLog.h"

#include "logging/VCLOGGING.h"

#include "CC/CCUtil.h"

// Need to include MM Message Queue
#include "mm/MMInt.h"

// Need to include RR Message Queue
#ifndef _NO_RRM
extern MSG_Q_ID rm_MsgQId;
#endif

bool
sendRR      (IntraL3PrimitiveType_t     prim    ,
             IntraL3MsgType_t           msgType ,
             T_CNI_IRT_ID               entryId ,
             IntraL3Msg_t              *rrOutMsg)
{
  DBG_FUNC("sendRR", CC_LAYER);
  DBG_ENTER();

  rrOutMsg->module_id = MODULE_CC;
  rrOutMsg->primitive_type = prim; 
  rrOutMsg->message_type = msgType;

  rrOutMsg->entry_id = entryId;

  // send the message.
#ifndef _NO_RRM
  if (ERROR == msgQSend( rm_MsgQId,
                         (char *) rrOutMsg, 
                         sizeof(struct  IntraL3Msg_t), 
                         NO_WAIT,    //PR1553 <xxu> WAIT_FOREVER,
                         MSG_PRI_NORMAL
                         ) )
    {
      DBG_ERROR("CC Error : sendRR msgQSend (QID = %d) error\n ", 
                (int)rm_MsgQId);
      DBG_LEAVE();
      return(false);
    }
  else
    {
      DBG_TRACE("CC Call Log: Sent Message to RR, (Prim. Type = %d), (Msg Type = %d)\n",  
                prim, 
                msgType);
      if (msgType == INTRA_L3_RR_CHANN_ASSIGN_CMD)
        {
          DBG_TRACE("CC->RR: Chann Assignment Request with Speech Version: %d",  
                    rrOutMsg->l3_data.channAssignCmd.channelType.speechChann.version);
        }

      DBG_LEAVE();
      return(true);
    }

#endif
  DBG_LEAVE();
  return(true);
}  
 
bool 
sendMM      (IntraL3PrimitiveType_t     prim    ,
             IntraL3MsgType_t           msgType ,
             T_CNI_IRT_ID               entryId ,
             IntraL3Msg_t              *mmOutMsg)
{
  DBG_FUNC("sendMM", CC_LAYER);
  DBG_ENTER();

  mmOutMsg->module_id = MODULE_CC;
  mmOutMsg->primitive_type = prim; 
  mmOutMsg->message_type = msgType;

  mmOutMsg->entry_id = entryId;

  // send the message.
  if (ERROR == msgQSend( mmMsgQId,
                         (char *) mmOutMsg, 
                         sizeof(struct  IntraL3Msg_t), 
                         NO_WAIT,    //PR1553 <xxu> WAIT_FOREVER,
                         MSG_PRI_NORMAL
                         ) )
    {
      DBG_ERROR("CC Error : sendMM msgQSend (QID=%d) error\n ", 
                (int)mmMsgQId);
      DBG_LEAVE();
      return(false);
    }
  else
    {
      DBG_TRACE("CC Call Log: Sent Message to MM, Prim. Type, Msg Type: %d %d\n",  
                prim, 
                msgType);
      DBG_LEAVE();
      return(true);
    }
}  

