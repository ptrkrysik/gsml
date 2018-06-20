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
// File        : CallTask.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : The Main Loop for each individual task.
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included VxWorks headers
#include "vxWorks.h"
#include "taskLib.h"

#include "logging/VCLOGGING.h"

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"

// CC Message Constants
#include "CC/CallConfig.h"
#include "CC/CCTypes.h"
#include "CC/CCInt.h"
#include "CC/CCUtil.h"
#include "CC/CCH323Util.h"
#include "CC/CCHalfCall.h"
#include "CC/CCSessionHandler.h"

#include "taskLib.h"

#include "Os/JCTask.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3_common.h"

#include "ril3/ril3md.h"

// included LAPDm header for downlink messages
#include "lapdm/lapdm_l3intf.h"

#include "oam_api.h"
#include "vipermib.h"

extern DbgOutput Dbg;

extern bool callTrace;

JCTask *CallTaskObjs[CC_MAX_CC_CALLS_DEF_VAL];

inline int JCCDBG_TRACE1(char *str) { DBG_TRACE(str); return true;};

// entry function for the Call Module (VxWorks Task)
int callMain(const short callIndex)
{
  DBG_FUNC(taskName(taskIdSelf()), CC_LAYER);
  DBG_ENTER();

  // This task's call session
  CCSession_t *callSession = &ccSession[callIndex];

  // local message queue buffer
  CCMMInputMsg_t              inMsg;

  // enter infinite loop to wait for message and process message from MS 
  // or other layer-3 modules
  CallTaskObjs[callIndex]->JCTaskEnterLoop();

      while (true)
      {
          if (msgQReceive (callSession->msgQId,
                           (char *) &inMsg    , 
                           CC_MAX_MSG_LENGTH  , 
                           WAIT_FOREVER        )
              == ERROR)
          {
              // failed to receive message. VxWorks error. Abort the task.
              printf("??->CC CALL FATAL ERROR: failed to read VxWorks message queue\n" );
              DBG_LEAVE();
              return ERROR;
          }
          
          DBG_TRACE ("M%d->CC CALL LOG: Call Task \"%s\" Received a message\n",
                      inMsg.intraL3Msg.module_id, taskName(taskIdSelf()));
    
          //ISSUE<xxu:04-12-00>
          if (callSession->free && inMsg.intraL3Msg.message_type != INTRA_L3_CC_Q931_SETUP) 
            {
              if ( (inMsg.intraL3Msg.module_id == MODULE_MM) &&
                   (inMsg.intraL3Msg.primitive_type == INTRA_L3_MM_EST_CNF) )
                {
                  // handle the case where a late establish confirm comes in 
                  // for a page request or a handover request that was previously sent.
                  IntraL3Msg_t      mmOutMsg;
                  sendMM   (INTRA_L3_MM_REL_REQ,
                            INTRA_L3_NULL_MSG,
                            inMsg.intraL3Msg.entry_id,
                            &mmOutMsg);
                }
              else
                {
                  DBG_WARNING("M%d->CC CALL WARNING: Received Unexpected/Late Msg(%d) Prim(%d) sessionId(%d)\n",
                               inMsg.intraL3Msg.module_id,
                               inMsg.intraL3Msg.message_type,
                               inMsg.intraL3Msg.primitive_type,
                               callSession->callIndex
                              );
                 DBG_WARNING("CC CALL WARNING: Check protocol at the source. \n");
                }
            }
    
          else
            {
              // take the semaphore before processing the message.
              semTake(callSession->semId, WAIT_FOREVER);
    
              //ISSUE<xxu: 04-12-00>
              callSession->free = false;
    
              // Call the safety timer before processing the message,
              // because of event loops. msg sent to mother CC task.
              callSession->safetyTimer->setTimer(CALL_SAFETY_TIMER);
    
              // action based on the sending module
              // 
              switch (inMsg.intraL3Msg.module_id) 
              { 
              case T_CNI_MD_ID:
                {
                  T_CNI_RIL3MD_CCMM_MSG      *msInMsg = &inMsg.mdMsg;
    
                  DBG_TRACE("MD->CC CALL LOG: Received Prim(%d) sessionId(%d)\n",  
                             msInMsg->primitive_type,
                             callSession->callIndex);
    
                  callSession->sessionHandler->handleMobMsg(msInMsg);
    
                  break;
                }
    
              case MODULE_MM:
                {
                  IntraL3Msg_t            *mmInMsg = &inMsg.intraL3Msg;
    
                  DBG_TRACE("MM->CC CALL LOG: Received Msg(%d) Prim(%d) entryId(%d) sessionId(%d)\n",  
                             mmInMsg->message_type,
                             mmInMsg->primitive_type, 
                             mmInMsg->entry_id,
                             callSession->callIndex);
    
                  callSession->sessionHandler->handleMMMsg(mmInMsg);
    
                  break;
                }
    
              case MODULE_RM:
                {
                  IntraL3Msg_t            *rrInMsg = &inMsg.intraL3Msg;
    
                  DBG_TRACE("RM->CC CALL LOG: Recived Msg(%d) Prim(%d) entryId(%d) sessionId(%d)\n",  
                             rrInMsg->message_type,
                             rrInMsg->primitive_type, 
                             rrInMsg->entry_id,
                             callSession->callIndex);
    
                  if (callTrace)
                    {
                      ccEntryPrint(*callSession, JCCDBG_TRACE1);
                    }
    
                  callSession->sessionHandler->handleRRMsg(rrInMsg);
    
                  break;
                }
    
              // <Igal 31-5-00> Updated for handling forwarded interVC PDUs
              case MODULE_CC:
                {
                  IntraL3Msg_t     *ccInMsg = &inMsg.intraL3Msg          ;
    
                  DBG_TRACE("CC->CC CALL LOG: Received msg(%d) callLeg(%d) signature(%d) sessionId(%d)\n",
                             ccInMsg->message_type, ccInMsg->entry_id, ccInMsg->signature,
                             callSession->callIndex);
   
                  switch (ccInMsg->message_type)
                  {
                  case INTRA_L3_TIMEOUT_MSG:
                       callSession->sessionHandler->handleTimeoutMsg (ccInMsg);
                       break;
    
                  default:
                       DBG_ERROR("CC->CC CALL ERROR: Unexpected (Msg Type = %d) from CC.\n",  
                                  ccInMsg->message_type);
                  }
    
                  break;
                }
            
              case MODULE_H323:
                {
                  IntraL3Msg_t *h323InMsg = &inMsg.intraL3Msg;

                  //ext-HO <xxu: 05-10-01>
                  if ( h323InMsg->call_handle != -1 ) // If first message from VOIP in calltask, then can NOT be related to external handover
                    {
                      if ( (callSession->sessionHandler->hoCallHandle==h323InMsg->call_handle) ||  // note: hoCallHandle is used for hand-in case only
                           (h323InMsg->callleg_id == 7) )  // JOE: Use constant instead of hard-coded value for hoCallLeg
                        {
                          DBG_TRACEho("{\nMNEThoTRACE(callMain): Recv VOIP Msg (msgType=%d, hoHndl=%p, callId=%d, ludbId=%d)\n}\n",
                                      h323InMsg->message_type,
                                      (int)(h323InMsg->call_handle), 
                                      callSession->callIndex,
                                      h323InMsg->entry_id);

                          if (callSession->sessionHandler->handleH323MsgExtHo(h323InMsg) )
                            {
                               DBG_ERRORho("{\nMNEThoERROR(callMain): Recv VOIP Msg (msgType=%d, hoHndl=%p, callId=%d, ludbId=%d)\n}\n",
                                           h323InMsg->message_type,
                                           (int)(h323InMsg->call_handle), 
                                           callSession->callIndex,
                                           h323InMsg->entry_id);
                            }
                        }
                     else if ( h323InMsg->callleg_id == 8 )  // JOE: Use constant instead of hard-coded value for h3CallLeg
                        {
                           DBG_TRACEho("{\nMNEThoTRACE(callMain): Recv VOIP Msg (msgType=%d, h3Hndl=%p, callId=%d, ludbId=%d)\n}\n",
                                       h323InMsg->message_type,
                                       (int)(h323InMsg->call_handle), 
                                       callSession->callIndex,
                                       h323InMsg->entry_id);

                           if (callSession->sessionHandler->handleH323MsgExtH3(h323InMsg) )
                             {
                                DBG_ERRORho("{\nMNEThoERROR(callMain): Recv VOIP Msg (msgType=%d, h3Hndl=%p, callId=%d, ludbId=%d)\n}\n",
                                            h323InMsg->message_type,
                                           (int)(h323InMsg->call_handle), 
                                           callSession->callIndex,
                                           h323InMsg->entry_id);
                             }
                        }
                     else
                        {
                            DBG_TRACE("H323->CC CALL LOG: Received Msg(%d) h323CallHandle(%p) sessionId(%d)\n",
                                       h323InMsg->message_type,
                                       (int)(h323InMsg->call_handle), 
                                       callSession->callIndex);

                            callSession->sessionHandler->handleH323Msg(h323InMsg);
                        }
                    }
                 else
                    {
                       DBG_TRACE("H323->CC CALL LOG: Received Msg(%d) h323CallHandle(%p) sessionId(%d)\n",
                                  h323InMsg->message_type,
                                  (int)(h323InMsg->call_handle), 
                                  callSession->callIndex);

                       callSession->sessionHandler->handleH323Msg(h323InMsg);
                    }

                    break;
                }

              //ext-HO <xxu:05-16-01>
              case MODULE_EXTHOA:
                {
                  InterHoaVcMsg_t *hoaInMsg = &inMsg.hoaCcMsg;
    
                  DBG_TRACEho("{\nMNEThoTRACE(callMain): Received Msg from HOA (msgType=%d, callId=%d)\n}\n",  
                               hoaInMsg->msgType, callSession->callIndex);
    
				  DBG_CCDATA(callSession->callIndex);

                  callSession->sessionHandler->handleHoaCcMsg(hoaInMsg);
    
                  break;
                }

              case MODULE_EXT_VC:
                {
                  InterVcMsg_t *vcInMsg = &inMsg.vcCcMsg.vcToVcMsg;
    
                  DBG_TRACE ("VC->CC CALL LOG: Received Msg(%d) from another GP\n",  
                              vcInMsg->msgType);
    
                  if (callTrace)
                  {
                      ccEntryPrint (*callSession, JCCDBG_TRACE1);
                  }
    
                  callSession->sessionHandler->handleVcCcMsg (vcInMsg);
    
                  break;
                }
    
              case MODULE_CISS:
                {
                  CISSVblinkMsg_t *cissInMsg = &inMsg.intraL3Msg.l3_data.cissMsg;
    
                  DBG_TRACE("VB->CC CISS: Received Msg(%d) from ViperBase\n",  
                             cissInMsg->msgType);
    
                  if (callTrace)
                    {
                      ccEntryPrint(*callSession, JCCDBG_TRACE1);
                    }
    
                  callSession->sessionHandler->handleVbCissMsg(inMsg.intraL3Msg.l3_data.cissMsg);
    
                  break;
                }

              case MODULE_LUDB:
                {
                  DBG_TRACE ("LUDB->CC CALL LOG: Received Msg (%d) sessionId(%d)\n",
                             inMsg.intraL3Msg.message_type, callSession->callIndex );
                  
                  if (inMsg.intraL3Msg.message_type == INTRA_L3_LUDB_PURGE_PROFILE)
                  {
                      callSession->sessionHandler->cleanup (CNI_RIL3_CAUSE_REQUESTED_SERVICE_NOT_SUBSCRIBED);
                  }
                  else
                  {
                      DBG_ERROR ("CC Call Error: Unexpected Msg [type = %d] from LUDB.\n",  
                                 inMsg.intraL3Msg.message_type                           );
                  }
                  break;
                }


              default:
                // log error
                DBG_ERROR("CC CALL ERROR: Received a Msg from unexpected (Module = %d).\n",  
                           inMsg.intraL3Msg.module_id);
              }
    
              // Now let go of the semaphores.
              semGive(callSession->semId);
    
              // Message Processing complete. Cancel the safety timer.
              callSession->safetyTimer->cancelTimer();
            }
      }
  CallTaskObjs[callIndex]->JCTaskNormExit();
}


