#ifndef CallConfig_H
#define CallConfig_H

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
// File        : CallConfig.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : Configuration Data for an individual Call task 
//               - to be set through OA&M means. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "vxWorks.h"
#include "msgQLib.h"
#include "jcc\JCCComIE.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************

//	defines the size of VxWorks Message queue
//      The value of this parameter is system dependent
//
const int CALL_MAX_L3_MSGS         = 32;

// defines the priority of the VxWorks message queue
// 
const int CALL_L3_MSG_Q_PRIORITY   = MSG_Q_PRIORITY;

// Timers 
//
// The timer value is based on VxWorks "tick" which is BSP (Board Support
// Package) dependent. A translation between real-world time unit (seconds
// or millisecond) and VxWorks ticks muct be done first to set the proper
// timer values.
//

extern int CALL_MM_CONN_EST_T999;    //     = 2000      MM Connection Established - Network (MO)
extern int CALL_ALERTING_T301;       //     = 18000      Alerting received Timer - Network (MT)
extern int CALL_SETUP_T303;          //     = 2000      Setup sent - Network (MT)
extern int CALL_CALL_CONFIRMED_T310; //     = 2000      Call Confirmed received Timer - Network (MT)
extern int CALL_CONNECT_T313;        //     = 2000      Connect sent Timer - Network (MO)
extern int CALL_DISCONNECT_T305;     //     = 3000      Disconnect sent Timer - Network (MO and MT)
extern int CALL_RELEASE_T308;        //     = 2000      Release sent Timer - Network (MO and MT)

extern int SMS_WAIT_CP_ACK_TC1;      //     = 1000      Wait for CP-ACK (MO and MT) 5-15 sec 
extern int SMS_WAIT_DISCONNECT;      //     = 150       Allows to send the last CP-ACK on SACCH/TCH 

extern int CALL_SAFETY_TIMER;        //     = 500      Call Task running too long

// Handover Config
extern int CALL_HAND_SRC_THO;        //     = 1000      Handover - waiting for Target Perform Ho Req Ack (ANCH)
extern int CALL_HAND_SRC_T103;       //     = 2000      Handover - waiting for Target Ho Complt (ANCH)
extern int CALL_HAND_TRG_T204;       //     = 2000      Handover - waiting for Mobile Handover Complt (TRG)


//ext-HO <xxu:07-01-01>
extern int CALL_HAND_TRG_T211;       //     = 2000      Handover - waiting for A-HO_REQUEST_ACK/A-HO_FAILURE) (TRG)
extern int CALL_HAND_TRG_T202;       //     = 500       Handover - waiting for Mobile Handover Complt btw BSSs under MSC-B

extern int CALL_HAND_SRC_TSHO;       //     = 1000      Handover - waiting for Target Perform Hb Req Ack (TRG)
extern int CALL_HAND_SRC_T203;       //     = 2000      Handover - waiting for Target Hb Complt (TRG)
extern int CALL_HAND_TRG_T104;       //     = 2000      Handover - waiting for Mobile Handover Complt (ANCH)

//CDR <xxu:08-24-00>
extern int CALL_CDR_10M;		     //     = 39528		10m call duration timer for CDR

//CH<xxu:11-24-99>
extern int CALL_HOLD_RETV_TIMER;	 //	  = 5000      Time control for call swapping

//RETRY<xxu:05-01-00> BEGIN
extern int CALL_HAND_SRC_TVB;	 	 //     = 10000	  Time control for shake-hand btw anchor and VB
//RETRY<xxu:05-01-00> END

extern SpeechChann_t currChannDef;

// Default values
const int CALL_MM_CONN_EST_T999_DEF_VAL = 2000;       // MM Connection Established - Network (MO)
const int CALL_ALERTING_T301_DEF_VAL = 18000;         // Alerting received Timer - Network (MT)
const int CALL_SETUP_T303_DEF_VAL = 2000;             // Setup sent - Network (MT)
const int CALL_CALL_CONFIRMED_T310_DEF_VAL = 2000;    // Call Confirmed received Timer - Network (MT)
const int CALL_CONNECT_T313_DEF_VAL = 2000;           // Connect sent Timer - Network (MO)
const int CALL_DISCONNECT_T305_DEF_VAL = 3000;        // Disconnect sent Timer - Network (MO and MT)
const int CALL_RELEASE_T308_DEF_VAL = 2000;           // Release sent Timer - Network (MO and MT)

// CH<xxu:11-24-99
const int CALL_HOLD_RETV_TIMER_DEF_VAL  = 5000;       // Time out for control call swap

// CDR <xxu:08-24-00>
const int CALL_CDR_10M_DEF_VAL  = 35928;			  // 10M call duration timer for CDR

// RETRY<xxu:05-01-00> BEGIN
const int CALL_HAND_SRC_TVB_DEF_VAL = 10000;          // Time out for shake hand btw anchor and target
// RETRY<xxu:05-01-00> END
const int CALL_SAFETY_TIMER_DEF_VAL = 500;            // Call Task running too long

// Handover Config Default Values
const int CALL_HAND_SRC_THO_DEF_VAL  = 1000;          // Handover - waiting for Target Perform Ho Req Ack (ANCH)
const int CALL_HAND_SRC_T103_DEF_VAL = 2000;          // Handover - waiting for Target Ho Complt (ANCH)
const int CALL_HAND_TRG_T204_DEF_VAL = 2000;          // Handover - waiting for Mobile Handover Complt (TRG)

//ext-HO <xxu:07-01-01>
const int CALL_HAND_TRG_T211_DEF_VAL = 2000;          // Handover - waiting for A-HO_REQUEST_ACK/A-HO_FAILURE) (TRG)
const int CALL_HAND_TRG_T202_DEF_VAL = 500;           // Handover - waiting for Mobile Handover Complt btw BSSs under MSC-B

const int CALL_HAND_SRC_TSHO_DEF_VAL = 1000;          // Handover - waiting for Target Perform Hb Req Ack (TRG)
const int CALL_HAND_SRC_T203_DEF_VAL = 2000;          // Handover - waiting for Target Hb Complt (TRG)
const int CALL_HAND_TRG_T104_DEF_VAL = 2000;          // Handover - waiting for Mobile Handover Complt (ANCH)

const int SMS_WAIT_CP_ACK_TC1_DEF_VAL= 1200;          // Wait for CP-ACK (MO and MT) 10 sec (5-15 sec)
const int SMS_WAIT_DISCONNECT_DEF_VAL= 150 ;          // Allows to send CP-ACK on SACCH/TCH before channel release
                                                      // at the end of MT SMS-PP scenario (2 sec)
// Typedefs
//

#endif                                 // CallConfig_H
