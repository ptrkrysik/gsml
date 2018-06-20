#ifndef CCMsgAnal_H
#define CCMsgAnal_H

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
// File        : CCMsgAnal.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// 
#include "JCC/JCCEvent.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"
#include "ril3/ril3_cc_msg.h"
#include "ril3/ril3_sms_cp.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

//typedefs

class CCMsgAnalyzer
{
 public:
  JCCEvent_t mapToCcEvent   (T_CNI_RIL3MD_CCMM_MSG *msInMsg);
  JCCEvent_t mapToSmsEvent  (T_CNI_RIL3MD_CCMM_MSG *msInMsg);

  T_CNI_RIL3CC_MSG		msDecodedMsg   ;
  T_CNI_RIL3SMS_CP_MSG	msDecodedSmsMsg;

  T_CNI_RIL3_RESULT		msDecodeResult ;

  T_CNI_RIL3_CAUSE_VALUE     causeValue;
};

#endif                                       // CCMsgAnal_H
