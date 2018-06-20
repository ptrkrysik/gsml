// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Failure_Msg.h
// Author(s)   : Tai Yu
// Description : 
//
// FILE DESCRIPTION:
//
// 
// CHANGE HISTORY:
//
//		Date		Description
//		----------	--------------------------------------------
//		2001-05-15	First Incarnation
//
// =================================================================

#ifndef A_HANDOVER_FAILURE_H
#define A_HANDOVER_FAILURE_H

#include "CC\A_Handover_Ie.h"

// 3.2.1.16		Handover Failure

typedef struct {
	// 3.2.2.1 Mandatory
	A_Ie_Message_Type_t						msgType;
	// 3.2.2.5 Mandatory
	A_Ie_Cause_t							cause;
	// 3.2.2.22 Optional
	A_Ie_RR_Cause_t							rrCause;
	// 3.2.2.45 Optional
	A_Ie_Circuit_Pool_t						circuitPool;
	// 3.2.2.46 Optional
	A_Ie_Circuit_Pool_List_t				circuitPoolList;
} A_Handover_Failure_t;

#include "CC\A_Handover_Common.h"

extern A_Result_t
A_EncodeHandoverFailure(
	A_Handover_Failure_t &handoverFailure,
	unsigned char *buffer, 
	int &length
	);

extern A_Result_t 
A_DecodeHandoverFailure(
	unsigned char *buffer, 
	int length,
	A_Handover_Failure_t &handoverFailure
	);

#endif // A_HANDOVER_FAILURE_MSG_H

