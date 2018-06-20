// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Complete_Msg.cpp
// Author(s)   : Tai Yu
// Create Date : 05-22-01
// Description : Source Vipercell Exteral Handover Complete Message 
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

#include "CC\A_Handover_Complete_Msg.h"

// 3.2.1.12
A_Result_t
A_EncodeHandoverComplete(
	A_Handover_Complete_t &handoverComplete,
	unsigned char *buffer, 
	int &length
	)
{	
	A_Result_t resultCode;
	int ieLength = 1;
	int bufferIndex = 1;

	if (length < 4)
	{
		return A_INCORRECT_LENGTH;
	}
	
	// Encode message header
	A_EncodeMessageType((A_Ie_Message_Type_t)A_HANDOVER_COMPLETE_TYPE, 
		ieLength,
		&buffer[0]);

	if(handoverComplete.rrCause.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeRRCause(
		    handoverComplete.rrCause,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

    length = bufferIndex;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeHandoverComplete(
	unsigned char	*buffer, 
	int				length,
	A_Handover_Complete_t &handoverComplete
	)
{	
	if(length < 4)
	{
		return A_MANDATORY_IE_MISSING;
	}

	A_Result_t resultCode;
	int ieLength = 0;
	int bufferIndex = 0;

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_RR_CAUSE_TYPE))
	{
	    resultCode = A_DecodeRRCause(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverComplete.rrCause);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	return A_RESULT_SUCCESS;
}
