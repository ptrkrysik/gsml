// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Request_Msg.cpp
// Author(s)   : Tai Yu
// Create Date : 05-15-01
// Description : Source Vipercell Exteral Handover Request Message 
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

#include "CC\A_Handover_Request_Msg.h"

// 3.2.1.16
A_Result_t
A_EncodeHandoverRequest(
	A_Handover_Request_t &handoverRequest,
	unsigned char *buffer, 
	int &length
	)
{	
	A_Result_t resultCode;
	int ieLength = 1;
	int bufferIndex = 1;

	if (length < 23)
	{
		return A_INCORRECT_LENGTH;
	}
	
	// Encode message header
	A_EncodeMessageType(A_HANDOVER_REQUEST_TYPE, 
		ieLength,
		&buffer[0]);

	ieLength = 1;

	if(handoverRequest.channelType.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeChannelType(
		    handoverRequest.channelType,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
    }
    else
        return A_MANDATORY_IE_MISSING;

	if(handoverRequest.encryptionInformation.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeEncryptInformation(
		    handoverRequest.encryptionInformation,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
    }
    else
        return A_MANDATORY_IE_MISSING;

	if(handoverRequest.classmarkInformation1.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeClassmarkInformation1(
		    handoverRequest.classmarkInformation1,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
    }
    else
	if(handoverRequest.classmarkInformation2.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeClassmarkInformation2(
		    handoverRequest.classmarkInformation2,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
    }
    else
        return A_MANDATORY_IE_MISSING;

	if(handoverRequest.cellIdentifierServing.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCellIdentifier(
		    handoverRequest.cellIdentifierServing,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
    }
    else
        return A_MANDATORY_IE_MISSING;

	if(handoverRequest.priority.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodePriority(
		    handoverRequest.priority,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.circuitIdentityCode.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCircuitIdentityCode(
		    handoverRequest.circuitIdentityCode,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.downlinkDTXFlag.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeDownlinkDTXFlag(
		    handoverRequest.downlinkDTXFlag,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.cellIdentifierTarget.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCellIdentifier(
		    handoverRequest.cellIdentifierTarget,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
    }
    else
        return A_MANDATORY_IE_MISSING;

	if(handoverRequest.interferenceBandToBeUsed.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeInterferenceBandToBeUsed(
		    handoverRequest.interferenceBandToBeUsed,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.cause.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCause(
		    handoverRequest.cause,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.classmarkInformation3.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeClassmarkInformation3(
		    handoverRequest.classmarkInformation3,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.currentChannelType1.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeCurrentChannelType1(
		    handoverRequest.currentChannelType1,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.speechVersionUsed.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeSpeechVersion(
		    handoverRequest.speechVersionUsed,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.groupCallReference.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeGroupCallReference(
		    handoverRequest.groupCallReference,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.talkerFlag.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeTalkerFlag(
		    handoverRequest.talkerFlag,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.configEvolIndication.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeConfigEvolutionIndication(
		    handoverRequest.configEvolIndication,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.chosenEncryptAlgorithmServing.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeChosenEncryptAlorithm(
		    handoverRequest.chosenEncryptAlgorithmServing,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

	if(handoverRequest.oldBSSToNewBSSInfo.ie_present)
	{
		if(length <= bufferIndex)
		{
			return A_INCORRECT_LENGTH;
		}
	    resultCode = A_EncodeBssToNewBssInfo(
		    handoverRequest.oldBSSToNewBSSInfo,
		    ieLength,
		    &buffer[bufferIndex]);
	    bufferIndex += ieLength;

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
    }

    length = bufferIndex;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeHandoverRequest(
	unsigned char	*buffer, 
	int				length,
	A_Handover_Request_t &handoverRequest
	)
{	
	if(length < 12)
	{
		return A_MANDATORY_IE_MISSING;
	}

	A_Result_t resultCode;
	int ieLength = 0;
	int bufferIndex = 0;

	resultCode = A_DecodeChannelType(
		&buffer[bufferIndex],
		ieLength,
		handoverRequest.channelType);

	A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);

	bufferIndex += ieLength;

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_ENCRYPTION_INFORMATION_TYPE))
	{
 	    resultCode = A_DecodeEncryptInformation(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.encryptionInformation);

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }
    else
        return A_MANDATORY_IE_MISSING;

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CLASSMARK_INFORMATION1_TYPE))
	{
 	    resultCode = A_DecodeClassmarkInformation1(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.classmarkInformation1);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    } else
	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CLASSMARK_INFORMATION2_TYPE))
	{
 	    resultCode = A_DecodeClassmarkInformation2(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.classmarkInformation2);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }
    else
        return A_MANDATORY_IE_MISSING;

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CELL_IDENTIFIER_TYPE))
	{
 	    resultCode = A_DecodeCellIdentifier(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.cellIdentifierServing);

	    A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }
    else
        return A_MANDATORY_IE_MISSING;

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_PRIORITY_TYPE))
	{
	    resultCode = A_DecodePriority(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.priority);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CIRCUIT_IDENTITY_CODE_TYPE))
	{
 	    resultCode = A_DecodeCircuitIdentityCode(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.circuitIdentityCode);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_DOWNLINK_DTX_FLAG_TYPE))
	{
 	    resultCode = A_DecodeDownlinkDTXFlag(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.downlinkDTXFlag);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CELL_IDENTIFIER_TYPE))
	{
 	    resultCode = A_DecodeCellIdentifier(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.cellIdentifierTarget);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_INTERFERENCE_BAND_TO_BE_USED_TYPE))
	{
 	    resultCode = A_DecodeInterferenceBandToBeUsed(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.interferenceBandToBeUsed);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CAUSE_TYPE))
	{
 	    resultCode = A_DecodeCause(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.cause);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CLASSMARK_INFORMATION3_TYPE))
	{
	    resultCode = A_DecodeClassmarkInformation3(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.classmarkInformation3);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CURRENT_CHANNEL_TYPE1_TYPE))
	{
	    resultCode = A_DecodeCurrentChannelType1(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.currentChannelType1);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_SPEECH_VERSION_TYPE))
	{
 	    resultCode = A_DecodeSpeechVersion(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.speechVersionUsed);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_GROUP_CALL_REFERENCE_TYPE))
	{
 	    resultCode = A_DecodeGroupCallReference(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.groupCallReference);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_TALKER_FLAG_TYPE))
	{
 	    resultCode = A_DecodeTalkerFlag(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.talkerFlag);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CONFIGURATION_EVOLUTION_INDICATION))
	{
 	    resultCode = A_DecodeConfigEvolutionIndication(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.configEvolIndication);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_CHOSEN_ENCRYPTION_ALGORITHM_TYPE))
	{
 	    resultCode = A_DecodeChosenEncryptAlorithm(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.chosenEncryptAlgorithmServing);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	if((length > bufferIndex) &&
		(buffer[bufferIndex] == A_OLD_BSS_TO_NEW_BSS_INFORMATION_TYPE))
	{
 	    resultCode = A_DecodeBssToNewBssInfo(
		    &buffer[bufferIndex],
		    ieLength,
		    handoverRequest.oldBSSToNewBSSInfo);

	    A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode);
	    bufferIndex += ieLength;
    }

	return A_RESULT_SUCCESS;
}
