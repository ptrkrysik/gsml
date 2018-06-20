// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Detect_Msg.cpp
// Author(s)   : Tai Yu
// Create Date : 05-22-01
// Description : Source Vipercell Exteral Handover Detect Message 
//               Encode and Decode Functions
//
// FILE DESCRIPTION:
//
// 
// CHANGE HISTORY:
//
//		Date		Description
//		----------	--------------------------------------------
//		2001-05-22	First Incarnation
//
// =================================================================

#include "CC\A_Handover_Detect_Msg.h"

// 3.2.1.40
A_Result_t
A_EncodeHandoverDetect(
	A_Handover_Detect_t &handoverDetect,
	unsigned char *buffer, 
	int &length
	)
{	
	A_Result_t resultCode;
    int ieLength = 1;

	if (length < 1)
	{
		return A_INCORRECT_LENGTH;
	}
	
	// Encode message header
	A_EncodeMessageType((A_Ie_Message_Type_t)A_HANDOVER_DETECT_TYPE, 
		ieLength,
		&buffer[0]);

    length = 1;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeHandoverDetect(
	unsigned char	*buffer, 
	int				length,
	A_Handover_Detect_t &handoverDetect
	)
{	
    int decodedLength;

	if(length < 1)
	{
		return A_MANDATORY_IE_MISSING;
	}

    if(buffer[0] != A_HANDOVER_DETECT_TYPE)
    {
        return A_UNKNOWN_MESSAGE_TYPE;
    }

	// Decode message header
	A_DecodeMessageType(&buffer[0],
		decodedLength,
        handoverDetect.msgType);

	return A_RESULT_SUCCESS;
}
