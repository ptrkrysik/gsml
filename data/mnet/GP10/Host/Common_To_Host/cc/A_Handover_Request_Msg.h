// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Request_Msg.h
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

#ifndef A_HANDOVER_REQUEST_MSG_H
#define A_HANDOVER_REQUEST_MSG_H

#include "CC\A_Handover_Ie.h"

// 3.2.1.8		Handover Request

typedef struct {
	// 3.2.2.1 Mandatory
	A_Ie_Message_Type_t						msgType;
	// 3.2.2.11 Mandatory
	A_Ie_Channel_Type_t						channelType;
	// 3.2.2.10 Mandatory
	A_Ie_Encryption_Information_t			encryptionInformation;
	// 3.2.2.30 Mandatory (One of Information1 or Information2)
	A_Ie_Classmark_Information1_t			classmarkInformation1;
	// 3.2.2.19 Mandatory (One of Information1 or Information2)
	A_Ie_Classmark_Information2_t			classmarkInformation2;
	// 3.2.2.17 Mandatory
	A_Ie_Cell_Identifier_t					cellIdentifierServing;
	// 3.2.2.18 Optional
	A_Ie_Priority_t							priority;
	// 3.2.2.2 Optional
	A_Ie_Circuit_Identity_Code_t			circuitIdentityCode;
	// 3.2.2.26 Optional
	A_Ie_Downlink_DTX_Flag_t				downlinkDTXFlag;
	// 3.2.2.17 Mandatory
	A_Ie_Cell_Identifier_t					cellIdentifierTarget;
	// 3.2.2.21 Optional
	A_Ie_Interference_Band_To_Be_Used_t		interferenceBandToBeUsed;
	// 3.2.2.5 Optional
	A_Ie_Cause_t							cause;
	// 3.2.2.20 Optional
	A_Ie_Classmark_Information3_t			classmarkInformation3;
	// 3.2.2.49 Optional
	A_Ie_Current_Channel_Type1_t			currentChannelType1;
	// 3.2.2.51 Optional
	A_Ie_Speech_Version_t					speechVersionUsed;
	// 3.2.2.55 Optional
	A_Ie_Group_Call_Reference_t				groupCallReference;
	// 3.2.2.54 Optional
	A_Ie_Talker_Flag_t						talkerFlag;
	// 3.2.2.57 Optional
	A_Ie_Config_Evolution_Indication_t		configEvolIndication;
	// 3.2.2.44 Optional
	A_Ie_Chosen_Encrypt_Algorithm_t			chosenEncryptAlgorithmServing;
	// 3.2.2.59 Optional
	A_Ie_Old_BSS_To_New_BSS_Info_t			oldBSSToNewBSSInfo;
} A_Handover_Request_t;

#include "CC\A_Handover_Common.h"

extern A_Result_t
A_EncodeHandoverRequest(
	A_Handover_Request_t &handoverRequest,
	unsigned char *buffer, 
	int &length
	);

extern A_Result_t 
A_DecodeHandoverRequest(
	unsigned char *buffer,
	int length,
	A_Handover_Request_t &handoverRequest
	);
#endif // A_HANDOVER_REQUEST_MSG_H

