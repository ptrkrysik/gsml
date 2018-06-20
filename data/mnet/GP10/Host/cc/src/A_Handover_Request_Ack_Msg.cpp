// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Request_Ack_Msg.cpp
// Author(s)   : Tai Yu
// Create Date : 05-15-01
// Description : Source Vipercell Exteral Handover Request Ack Message 
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

#include "CC\A_Handover_Request_Ack_Msg.h"

// 3.2.1.16
A_Result_t
A_EncodeHandoverRequestAck(
	A_Handover_Request_Ack_t &handoverRequestAck,
	unsigned char *buffer, 
	int &length
	)
{	
	A_Result_t resultCode;
	int ieLength = 1;
	int bufferIndex = 1;

	if (length < 12)
	{
		return A_INCORRECT_LENGTH;
	}
	
	// Encode message header
	A_EncodeMessageType(A_HANDOVER_REQUEST_ACK_TYPE, 
		ieLength,
		&buffer[0]);


	if(handoverRequestAck.layer3Information.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeLayer3Information(
		    handoverRequestAck.layer3Information,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
    }
    else
        return A_MANDATORY_IE_MISSING;

	if(handoverRequestAck.chosenChannel.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeChosenChannel(
		    handoverRequestAck.chosenChannel,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequestAck.chosenEncryptAlgorithm.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeChosenEncryptAlorithm(
		    handoverRequestAck.chosenEncryptAlgorithm,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequestAck.circuitPool.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCircuitPool(
		    handoverRequestAck.circuitPool,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequestAck.speechVersionChosen.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeSpeechVersion(
		    handoverRequestAck.speechVersionChosen,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequestAck.circuitIdentityCode.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCircuitIdentityCode(
		    handoverRequestAck.circuitIdentityCode,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

    length = bufferIndex;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeHandoverRequestAck(
	unsigned char	*buffer, 
	int				length,
	A_Handover_Request_Ack_t &handoverRequestAck
	)
{	
	if(length < 12)
	{
		return A_MANDATORY_IE_MISSING;
	}

	A_Result_t resultCode;
	int ieLength = 0;
	int bufferIndex = 0;

	resultCode = A_DecodeLayer3Information(
		&buffer[bufferIndex],
		ieLength,
		handoverRequestAck.layer3Information);

	A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);

	bufferIndex += ieLength;

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CHOSEN_CHANNEL_TYPE))
	{
	    resultCode = A_DecodeChosenChannel(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequestAck.chosenChannel);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CHOSEN_ENCRYPTION_ALGORITHM_TYPE))
	{
	    resultCode = A_DecodeChosenEncryptAlorithm(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequestAck.chosenEncryptAlgorithm);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CIRCUIT_POOL_TYPE))
	{
	    resultCode = A_DecodeCircuitPool(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequestAck.circuitPool);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_SPEECH_VERSION_TYPE))
	{
	    resultCode = A_DecodeSpeechVersion(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequestAck.speechVersionChosen);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CIRCUIT_IDENTITY_CODE_TYPE))
	{
	    resultCode = A_DecodeCircuitIdentityCode(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequestAck.circuitIdentityCode);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	return A_RESULT_SUCCESS;
}
