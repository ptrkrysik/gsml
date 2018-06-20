// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Complete_Msg.h
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

#ifndef A_HANDOVER_COMPLETE_MSG_H
#define A_HANDOVER_COMPLETE_MSG_H

#include "CC\A_Handover_Ie.h"

// 3.2.1.40		Handover Complete

typedef struct {
	// 3.2.2.1 Mandatory
	A_Ie_Message_Type_t						msgType;
	// 3.2.2.22 Optional
	A_Ie_RR_Cause_t							rrCause;
} A_Handover_Complete_t;

#include "CC\A_Handover_Common.h"

extern A_Result_t
A_EncodeHandoverComplete(
	A_Handover_Complete_t &handoverComplete,
	unsigned char *buffer, 
	int &length
	);

extern A_Result_t 
A_DecodeHandoverComplete(
	unsigned char *buffer, 
	int length,
	A_Handover_Complete_t &handoverComplete
	);

#endif // A_HANDOVER_COMPLETE_MSG_H

