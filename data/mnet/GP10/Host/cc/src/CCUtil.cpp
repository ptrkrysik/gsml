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
// File        : CCUtil.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "JCC/JCCLog.h"
#include "logging/VCLOGGING.h"

#include "stdio.h"

#include "CC/CCsessionHandler.h"

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"
#include "rm/rm_ccintf.h"

#include "ril3/ril3_sms_rp.h"
#include "voip/vblink.h"


// CC Message Constants
#include "CC/CCconfig.h"

#include "CC/CCUtil.h"

bool smsSendVbLinkRelease (LUDB_ID, UINT32, T_CNI_RIL3_RP_CAUSE_VALUE cause = 
                           CNI_RIL3_RP_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE);

extern bool smsUseVblink;


/*----------------------------------------------------------------*/

void ccEntryInit(CCSession_t *session)
{
  session->free = true;

}

// Handles the release of a session
void
handleReleaseSession(CCSession_t *session)
{
  DBG_FUNC("handleReleaseSession", CC_LAYER);
  DBG_ENTER();
  ccEntryInit(session);
  DBG_LEAVE();
}

short ccCallAlloc()
{
  DBG_FUNC("ccCallAlloc", CC_LAYER);
  DBG_ENTER();
  short i;
  for(i=0; i< CC_MAX_CC_CALLS; i++)
    {
      if (ccSession[i].free == true)
        {
          // take the semaphore for the originating session.
          semTake(ccSession[i].semId, WAIT_FOREVER);
                  
          ccSession[i].free                = false;

          semGive(ccSession[i].semId);

          DBG_LEAVE();
          return i;
        }
    }
  
  DBG_LEAVE();
  return JCC_ERROR;
}


short initNewMSConn (const T_CNI_IRT_ID  newEntryId,
                           short        &callIndex )
{
  short       retVal = JCC_SUCCESS;
  T_CNI_L3_ID newId ;

  DBG_FUNC("initNewMSConn", CC_LAYER);
  DBG_ENTER();

  // Connection Initialization - 
  // New MS MM Connection that needs a CC level service
  if ((callIndex = ccCallAlloc()) == JCC_ERROR)
  {
      // Error - No Connections left. 
      DBG_ERROR ("CC Error : No Connections left. \n ");

      retVal = JCC_ERROR;
  }
  else
  {
      DBG_TRACE ("CC Log: (call selected = %d) for (entry id = %d) \n", callIndex, newEntryId);

      newId.msgq_id = ccSession[callIndex].msgQId;
      newId.sub_id  = 0                          ; // No sub id needed here

      // Update IRT
      if (ccSession[callIndex].sessionHandler->setIrt (newEntryId, newId) == false)
      {
          semTake              ( ccSession[callIndex].semId, WAIT_FOREVER);
          handleReleaseSession (&ccSession[callIndex]      );
          semGive              ( ccSession[callIndex].semId);

          retVal = JCC_ERROR;
      }
  }

  DBG_LEAVE();
  return   (retVal);
}


void ccEntryPrint(CCSession_t &session, JCCPrintStFn fnPtr)
{
  char callStr1[90];

  // Pre-format the call data Strings.
  sprintf(&callStr1[0],
          "Sess. Data: (Qid %p), (TaskId %p) (callId %d)\n",
          session.msgQId,
          session.taskId,
          session.callIndex);
  
  (*fnPtr)(&callStr1[0]);
 
  session.sessionHandler->printData(fnPtr);

}


// Internal Data Printing 
void ccDataPrint()
{
  short i;
  for(i=0; i< CC_MAX_CC_CALLS;i++)
    {
      printf("CC Session#(%d):", i);
      if (ccSession[i].free)
        {
          printf(" is free\n");
        }
      else
        {
          ccEntryPrint(ccSession[i], JCCPrintf);
        }
    }
}


bool smsSendVbLinkRelease(LUDB_ID ludbIndex, UINT32 refNum, 
                          T_CNI_RIL3_RP_CAUSE_VALUE cause = CNI_RIL3_RP_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE)
/* Used to send MNSMS-ERROR Ind from outside SMS Leg. Mostly in error cases. */
{
  SmsVblinkMsg_t smsMsgOut;
  bool           retVal   = true;

  DBG_FUNC ("smsSendVbLinkRelease", CC_SMS_LAYER); //Igal: change it later to CC_LAYER
  DBG_ENTER();

    memset (&smsMsgOut, sizeof (SmsVblinkMsg_t), 0);

    // prepare message data. Ensure the network byte order for the header elements
    smsMsgOut.hdr.magic   = htonl (SMS_MAGIC_NUMBER);
    smsMsgOut.hdr.refNum  = htonl (refNum          );
    smsMsgOut.hdr.msgType = htonl (MNSMS_ERROR_Ind );
    smsMsgOut.hdr.status  = htonl (cause           );
    smsMsgOut.hdr.msgLen  = htonl (0               );

    // sent message to VBLink
    if (smsUseVblink)
    {
        // The total message length calculation could be wrong 
        // if ported to the different platform or 
        // compiled with the word alignment other than 4 bytes
        retVal = VBLinkMsToNetSmspp (ludbIndex                  ,
                                     (unsigned char *)&smsMsgOut,
                                     SMS_VBLINK_HDRSIZE         );
        DBG_TRACE ("Send MNSMS-ERROR Ind, ref. number %d, cause %d \n", refNum, cause);
    }
    else
    {
        DBG_TRACE   ("SMS: VBLink interface is OFF. Message will not be send to ViperBase\n");
        DBG_HEXDUMP ((unsigned char *)&smsMsgOut, sizeof (SMS_VBLINK_HDRSIZE));
    }


  DBG_LEAVE();
  return   (retVal);
}

//ext-HO <xxu:05-16-01>
void ccSetCallTrace(void)
{
	ccCallTrace=true;
}

void ccResetCallTrace(void)
{
	ccCallTrace=false;
}

