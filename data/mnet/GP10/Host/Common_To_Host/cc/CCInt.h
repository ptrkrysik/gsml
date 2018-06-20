#ifndef CCInt_H
#define CCInt_H

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
// File        : CCInt.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : CC types shared with other modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (MM, RR)
#include "JCC/JCCL3Msg.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// defines the message queue ID for CC interface
//
#include "vxWorks.h"
#include "msgQLib.h"
extern MSG_Q_ID ccMsgQId;
extern T_CNI_L3_ID cc_id;

// *******************************************************************
// Constant Definitions
// *******************************************************************

// defines the size of buffer for holding each message
//
const int CC_MAX_MSG_LENGTH	   = sizeof(CCMMInputMsg_t);

#endif                                       // CCInt_H
