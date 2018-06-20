#ifndef CCconfig_H
#define CCconfig_H

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
// File        : CCconfig.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : Configuration Data for the parent CC task 
//               - to be set through OA&M means. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "vxWorks.h"
#include "msgQLib.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************

// Maximum number of CC Connections allowed
// Also max number of CC Call tasks
extern short CC_MAX_CC_CALLS;    //   

const int CC_MAX_CC_CALLS_DEF_VAL = 30;

extern UINT32 MY_VIPERCELL_IP_ADDRESS;

//	defines the size of VxWorks Message queue
//      The value of this parameter is system dependent
//
const int CC_MAX_L3_MSGS          = 32;

// defines the priority of the VxWorks message queue
// 
const int CC_L3_MSG_Q_PRIORITY 	  = MSG_Q_PRIORITY;

// The following two sets of constants define the priority and the
// working stack size for the CC task.
//
//const int CC_TASK_PRIORITY        = 70;
//const int CC_TASK_STACK_SIZE   = 10000;

// Timers 
//
// The timer value is based on VxWorks "tick" which is BSP (Board Support
// Package) dependent. A translation between real-world time unit (seconds
// or millisecond) and VxWorks ticks muct be done first to set the proper
// timer values.
//

// extern int CC_SAMPLE_T3113; = 10000  Sample Timer - Network


// Typedefs
//

#endif                                 // CCconfig_H
