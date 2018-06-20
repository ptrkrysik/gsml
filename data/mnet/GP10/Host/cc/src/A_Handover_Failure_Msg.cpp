// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Failure_Msg.cpp
// Author(s)   : Tai Yu
// Create Date : 05-15-01
// Description : Source Vipercell Exteral Handover Failure Message 
//               Encode and Decode Functions
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

#include "CC\A_Handover_Failure_Msg.h"

// 3.2.1.16
A_Result_t
A_EncodeHandoverFailure(
	A_Handover_Failure_t &handoverFailure,
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
	A_EncodeMessageType(A_HANDOVER_FAILURE_TYPE, 
		ieLength,
		&buffer[0]);

	if(handoverFailure.cause.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCause(
		    handoverFailure.cause,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
    }
    else
        return A_MANDATORY_IE_MISSING;

	if(handoverFailure.rrCause.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeRRCause(
		    handoverFailure.rrCause,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverFailure.circuitPool.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCircuitPool(
		    handoverFailure.circuitPool,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverFailure.circuitPoolList.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCircuitPoolList(
		    handoverFailure.circuitPoolList,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

    length = bufferIndex;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeHandoverFailure(
	unsigned char	*buffer, 
	int				length,
	A_Handover_Failure_t &handoverFailure
	)
{	
	if(length < 4)
	{
		return A_MANDATORY_IE_MISSING;
	}

	A_Result_t resultCode;
	int ieLength = 0;
	int bufferIndex = 0;

	resultCode = A_DecodeCause(
		&buffer[bufferIndex],
		ieLength,
		handoverFailure.cause);

	A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);

	bufferIndex += ieLength;

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_RR_CAUSE_TYPE))
	{
	    resultCode = A_DecodeRRCause(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverFailure.rrCause);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CIRCUIT_POOL_TYPE))
	{
	    resultCode = A_DecodeCircuitPool(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverFailure.circuitPool);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CIRCUIT_POOL_LIST_TYPE))
	{
	    resultCode = A_DecodeCircuitPoolList(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverFailure.circuitPoolList);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	return A_RESULT_SUCCESS;
}
