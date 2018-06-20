#ifndef CCTypes_H
#define CCTypes_H

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
// File        : CCTypes.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : CC types shared between all CC modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "semLib.h"

#include "JCC/JCCTimer.h"

#include "RIL3/ie_called_bcd.h"

#include "VOIP/voipapi.h"

#include "oam_api.h"


// *******************************************************************
// forward declarations.
// *******************************************************************
class CCSessionHandler;

//typedefs

// Data Structures needed inside the CC Module
typedef struct 
{
  MSG_Q_ID                     msgQId;
  int                          taskId;
  int                          callIndex;

  SEM_ID                       semId;
  
  bool                         free;

  // Timer to watch for the calls not to run off in a loop.
  JCCTimer                     * safetyTimer;

  CCSessionHandler             * sessionHandler;

} CCSession_t;

// Remote Message Data Structure
typedef struct 
{
  // this is the only data needed for remote setup msg 
  // - THC_REMOTE_TERM_ADDRESS event
  union 
  {
	//BCT <xxu:07-13-00> BEGIN
	VOIP_CALL_TRANSFER_MSG					   callTransfer;
	//BCT <xxu:07-13-00> END

    //BCT <xxu:09-21-00>
    VOIP_CALL_UPDATE_HANDLES_MSG               callUpdateHandles;

    VOIP_API_CALL_PROCEEDING_MSG               callProceeding;
    VOIP_API_CALL_SETUP_MSG                    setup;
    VOIP_API_CALL_OFFERING_MSG                 callOffering;
    VOIP_API_CODEC_SELECTED_MSG                codecSelected;
    VOIP_API_CALL_RELEASE_MSG                  callRelease;
    short                                      cellId;
    short                                      dtmfDigit;
    VOIP_API_CALL_EVENT_MSG                    basicCallInMsg;
  };

} CCRemMsgData_t;

// call timer action - with the Qid  
extern FUNCPTR                      callTimerExpiry; 

// allow multiple calls
extern CCSession_t ccSession[];

// 
extern INT_32_T ccOldBTSState;

#endif                                       // CCTypes_H
