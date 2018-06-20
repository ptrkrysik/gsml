#ifndef MMInt_H
#define MMInt_H

// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : MMInt.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : message and ie interface among Application (Layer 3) 
//               modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "JCC/JCCL3Msg.h"

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// defines the message queue ID for MM interface
//
#include "vxWorks.h"
#include "msgQLib.h"
extern MSG_Q_ID mmMsgQId;

// *******************************************************************
// Constant Definitions
// *******************************************************************

// defines the size of buffer for holding each message
//
#define		MM_MAX_MSG_LENGTH	1024
//const int MM_MAX_MSG_LENGTH	   = sizeof(T_CNI_RIL3MD_CCMM_MSG);

//typedefs

#endif                                       // MMInt_H


