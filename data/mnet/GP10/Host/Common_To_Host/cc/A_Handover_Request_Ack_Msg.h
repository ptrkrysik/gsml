// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Request_Ack_Msg.h
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

#ifndef A_HANDOVER_REQUEST_ACK_MSG_H
#define A_HANDOVER_REQUEST_ACK_MSG_H

#include "CC\A_Handover_Ie.h"

// 3.2.1.10		Handover Request Acknowledge

typedef struct {
	// 3.2.2.1 Mandatory
	A_Ie_Message_Type_t						msgType;
	// 3.2.2.24 Mandatory
	A_Ie_Layer3_Information_t				layer3Information;
	// 3.2.2.33 Optional
	A_Ie_Chosen_Channel_t					chosenChannel;
	// 3.2.2.44 Optional
	A_Ie_Chosen_Encrypt_Algorithm_t			chosenEncryptAlgorithm;
	// 3.2.2.45 Optional
	A_Ie_Circuit_Pool_t						circuitPool;
	// 3.2.2.51 Optional
	A_Ie_Speech_Version_t					speechVersionChosen;
	// 3.2.2.2 Optional
	A_Ie_Circuit_Identity_Code_t			circuitIdentityCode;
} A_Handover_Request_Ack_t;


#include "CC\A_Handover_Common.h"

extern A_Result_t
A_EncodeHandoverRequestAck(
	A_Handover_Request_Ack_t &handoverRequestAck,
	unsigned char *buffer, 
	int &length
	);

extern A_Result_t 
A_DecodeHandoverRequestAck(
	unsigned char *buffer, 
	int length,
	A_Handover_Request_Ack_t &handoverRequestAck
	);

#endif // A_HANDOVER_REQUEST_ACK_MSG_H

