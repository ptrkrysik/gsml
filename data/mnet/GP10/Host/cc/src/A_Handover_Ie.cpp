// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Ie.cpp
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

#include "CC\A_Handover_Ie.h"

// 3.2.2.1 Message_Type
/*
typedef unsigned char A_Ie_Message_Type_t;
*/

A_Result_t 
A_EncodeMessageType(
	A_Ie_Message_Type_t messageType,
	int             &ieLength,
	unsigned char	*buffer)
{
    buffer[0] = messageType;

    ieLength = 1;

    return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeMessageType(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Message_Type_t &messageType)
{

	messageType = buffer[0];

    ieLength = 1;

	return A_RESULT_SUCCESS;
}

// 3.2.2.2 Circuit_Identity_Code
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned short						pcmMultiplex:11;
	unsigned short						timeslot:5;
} A_Ie_Circuit_Identity_Code_t;
*/

A_Result_t 
A_EncodeCircuitIdentityCode(
	A_Ie_Circuit_Identity_Code_t &cirIdenCode,
	int &ieLength,
	unsigned char	*buffer)
{
    if(cirIdenCode.ie_present != true)
	{
	    return A_IE_MISSING;
	}

    buffer[0] = A_CIRCUIT_IDENTITY_CODE_TYPE;

    buffer[1] = (unsigned char)(cirIdenCode.pcmMultiplex >> 3);

    buffer[2] = ((unsigned char)((cirIdenCode.pcmMultiplex & 0x07) << 5) | 
	    (unsigned char)(cirIdenCode.timeslot & 0x1F));

    ieLength = 3;

    return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeCircuitIdentityCode(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Circuit_Identity_Code_t &cirIdenCode)
{
	cirIdenCode.ie_present = true;

	cirIdenCode.elementIdentifier = buffer[0];

	cirIdenCode.pcmMultiplex = ((buffer[1] << 3) | ((buffer[2] >> 5) & 0x7));

	cirIdenCode.timeslot = (buffer[2] & 0x1F);

    ieLength = 3;

	return A_RESULT_SUCCESS;
}

// 3.2.2.5 Cause
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						cause;
	unsigned char						cause2;
} A_Ie_Cause_t;
*/

A_Result_t 
A_EncodeCause(
	A_Ie_Cause_t &cause,
	int &ieLength,
	unsigned char	*buffer)
{
	if(cause.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CAUSE_TYPE;

	buffer[1] = (cause.cause & 0x8 == 0x8) ? 1 : 2;

	buffer[2] = cause.cause;

    ieLength = 3;
    if (cause.cause & 0x8 == 0x8)
    {
        buffer[3] = cause.cause2;
        ieLength = 4;
    }

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeCause(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Cause_t &cause)
{
	cause.ie_present = true;

	cause.elementIdentifier = buffer[0];

	cause.length = buffer[1];

	cause.cause = buffer[2];

	if (cause.cause & 0x8 == 0x8)
	    cause.cause2 = buffer[3];

	ieLength = cause.length + 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.10 Encryption_Information
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						permittedAlgorithm;
	unsigned char						key[8];
} A_Ie_Encryption_Information_t;
*/

A_Result_t 
A_EncodeEncryptInformation(
	A_Ie_Encryption_Information_t &encryptInfo,
	int &ieLength,
	unsigned char	*buffer)
{
	if(encryptInfo.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_ENCRYPTION_INFORMATION_TYPE;

	buffer[1] = encryptInfo.length;

	buffer[2] = encryptInfo.permittedAlgorithm;

	if (encryptInfo.length > 1)
		memcpy(&buffer[3], encryptInfo.key, 8);

    ieLength = 11;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeEncryptInformation(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Encryption_Information_t &encryptInfo)
{
	encryptInfo.ie_present = true;

	encryptInfo.elementIdentifier = buffer[0];

	encryptInfo.length = buffer[1];

	encryptInfo.permittedAlgorithm = buffer[2];

	if (encryptInfo.length > 1)
		memcpy(encryptInfo.key, &buffer[3], 8);

	ieLength = encryptInfo.length + 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.11
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						speechDataIndicator;
	unsigned char						channelRateAndType;
	unsigned char						permittedIndicator[8];
} A_Ie_Channel_Type_t;
*/

A_Result_t 
A_EncodeChannelType(
	A_Ie_Channel_Type_t &channelType,
	int &ieLength,
	unsigned char	*buffer)
{
	if(channelType.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CHANNEL_TYPE_TYPE;

	buffer[1] = channelType.length;

	buffer[2] = channelType.speechDataIndicator & 0x0F;

	buffer[3] = channelType.channelRateAndType;

    if (channelType.length > 2)
    {
		memcpy(&buffer[4], channelType.permittedIndicator, channelType.length - 2);
    }

    ieLength = channelType.length + 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeChannelType(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Channel_Type_t &channelType)
{
	channelType.ie_present = true;

	channelType.elementIdentifier = buffer[0];

	channelType.length = buffer[1];

	channelType.speechDataIndicator = buffer[2];

	channelType.channelRateAndType = buffer[3];

	if (channelType.length > 2)
    {
		memcpy(channelType.permittedIndicator, &buffer[4], channelType.length - 2);
    }

	ieLength = channelType.length + 2;

	return A_RESULT_SUCCESS;
}


// 3.2.2.17
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						cellIdDescriminator;
	unsigned char						mcc[3];
	unsigned char						mnc[3];
	unsigned short						lac;
	unsigned short						ci;
} A_Ie_Cell_Identifier_t;
*/

A_Result_t 
A_EncodeCellIdentifier(
	A_Ie_Cell_Identifier_t &cellIdentifier,
	int &ieLength,
	unsigned char	*buffer)
{
	if(cellIdentifier.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CELL_IDENTIFIER_TYPE;

	buffer[1] = cellIdentifier.length;

	buffer[2] = cellIdentifier.cellIdDescriminator & 0xF;

	switch (cellIdentifier.cellIdDescriminator)
	{
	case 0:
		buffer[3] = ((unsigned char)(cellIdentifier.mcc[1]) << 4 | 
			         (unsigned char)(cellIdentifier.mcc[0]));
		buffer[4] = ((unsigned char)(cellIdentifier.mnc[2]) << 4 | 
			         (unsigned char)(cellIdentifier.mcc[2]));
		buffer[5] = ((unsigned char)(cellIdentifier.mnc[1]) << 4 | 
			         (unsigned char)(cellIdentifier.mnc[0]));
		buffer[6] = (unsigned char)(cellIdentifier.lac >> 8);
		buffer[7] = (unsigned char)(cellIdentifier.lac & 0xFF);
		buffer[8] = (unsigned char)(cellIdentifier.ci >> 8);
		buffer[9] = (unsigned char)(cellIdentifier.ci & 0xFF);

		ieLength = 10;
		break;
	case 1:
		buffer[3] = (unsigned char)(cellIdentifier.lac >> 8);
		buffer[4] = (unsigned char)(cellIdentifier.lac & 0xFF);
		buffer[5] = (unsigned char)(cellIdentifier.ci >> 8);
		buffer[6] = (unsigned char)(cellIdentifier.ci & 0xFF);

		ieLength = 7;
		break;
	case 2:
		buffer[3] = (unsigned char)(cellIdentifier.ci >> 8);
		buffer[4] = (unsigned char)(cellIdentifier.ci & 0xFF);

		ieLength = 5;
		break;
	default:
		break;
	}

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeCellIdentifier(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Cell_Identifier_t &cellIdentifier)
{
	cellIdentifier.ie_present = true;

	cellIdentifier.elementIdentifier = buffer[0];

	cellIdentifier.length = buffer[1];

	cellIdentifier.cellIdDescriminator = buffer[2] & 0x0F;

	switch (cellIdentifier.cellIdDescriminator)
	{
	case 0:
		cellIdentifier.mcc[1] = (buffer[3] >> 4);
		cellIdentifier.mcc[0] = (buffer[3] & 0xF);
		cellIdentifier.mcc[2] = (buffer[4] & 0xF);
		cellIdentifier.mnc[2] = (buffer[4] >> 4);
		cellIdentifier.mnc[1] = (buffer[5] >> 4);
		cellIdentifier.mnc[0] = (buffer[5] & 0xF);
		cellIdentifier.lac = (unsigned short)(buffer[6] << 8) | (unsigned short)buffer[7];
		cellIdentifier.ci  = (unsigned short)(buffer[8] << 8) | (unsigned short)buffer[9];

		ieLength = 10;
		break;
	case 1:
		cellIdentifier.lac = (unsigned short)(buffer[3] << 8) | (unsigned short)buffer[4];
		cellIdentifier.ci  = (unsigned short)(buffer[5] << 8) | (unsigned short)buffer[6];

		ieLength = 7;
		break;
	case 2:
		cellIdentifier.ci  = (unsigned short)(buffer[3] << 8) | (unsigned short)buffer[4];

		ieLength = 5;
		break;
	default:
		break;
	}

	return A_RESULT_SUCCESS;
}

// 3.2.2.18
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						priority;
} A_Ie_Priority_t;
*/

A_Result_t 
A_EncodePriority(
	A_Ie_Priority_t &priority,
	int &ieLength,
	unsigned char	*buffer)
{
	if(priority.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_PRIORITY_TYPE;

	buffer[1] = priority.length;

	buffer[2] = priority.priority;

	ieLength = 3;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodePriority(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Priority_t &priority)
{
	priority.ie_present = true;

	priority.elementIdentifier = buffer[0];

	priority.length = buffer[1];

	priority.priority = buffer[2];

    ieLength = 3;

	return A_RESULT_SUCCESS;
}

// 3.2.2.9
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						protocolDisc;
	unsigned char						transactionID;
} A_Ie_Layer3_Header_Information_t;
*/

A_Result_t 
A_EncodeLayer3HeaderInformation(
	A_Ie_Layer3_Header_Information_t &layer3HeaderInfo,
	int &ieLength,
	unsigned char	*buffer)
{
	if(layer3HeaderInfo.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_LAYER3_HEADER_INFORMATION_TYPE;

	buffer[1] = layer3HeaderInfo.length;

	buffer[2] = layer3HeaderInfo.protocolDisc;

    buffer[3] = layer3HeaderInfo.transactionID;

	ieLength = 4;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeLayer3HeaderInformation(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Layer3_Header_Information_t &layer3HeaderInfo)
{
	layer3HeaderInfo.ie_present = true;

	layer3HeaderInfo.elementIdentifier = buffer[0];

	layer3HeaderInfo.length = buffer[1];

	layer3HeaderInfo.protocolDisc = buffer[2];

	layer3HeaderInfo.transactionID = buffer[3];

	ieLength = layer3HeaderInfo.length + 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.19
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						classmark[3];
} A_Ie_Classmark_Information2_t;
*/

A_Result_t 
A_EncodeClassmarkInformation2(
	A_Ie_Classmark_Information2_t &classmarkInfo2,
	int &ieLength,
	unsigned char	*buffer)
{
	if(classmarkInfo2.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CLASSMARK_INFORMATION2_TYPE;

	buffer[1] = classmarkInfo2.length;

	memcpy(&buffer[2], classmarkInfo2.classmark, classmarkInfo2.length);

	ieLength = classmarkInfo2.length + 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeClassmarkInformation2(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Classmark_Information2_t &classmarkInfo2)
{
	classmarkInfo2.ie_present = true;

	classmarkInfo2.elementIdentifier = buffer[0];

	classmarkInfo2.length = buffer[1];

	if (classmarkInfo2.length > 0)
		memcpy(classmarkInfo2.classmark, &buffer[2], classmarkInfo2.length);

	ieLength = classmarkInfo2.length + 2;

    return A_RESULT_SUCCESS;
}

// 3.2.2.20
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						classmark[12];
} A_Ie_Classmark_Information3_t;
*/

A_Result_t 
A_EncodeClassmarkInformation3(
	A_Ie_Classmark_Information3_t &classmarkInfo3,
	int &ieLength,
	unsigned char	*buffer)
{
	if(classmarkInfo3.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CLASSMARK_INFORMATION3_TYPE;

	buffer[1] = classmarkInfo3.length;

	memcpy(&buffer[2], classmarkInfo3.classmark, classmarkInfo3.length);

	ieLength = classmarkInfo3.length + 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeClassmarkInformation3(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Classmark_Information3_t &classmarkInfo3)
{
	classmarkInfo3.ie_present = true;

	classmarkInfo3.elementIdentifier = buffer[0];

	classmarkInfo3.length = buffer[1];

	if (classmarkInfo3.length > 0)
		memcpy(classmarkInfo3.classmark, &buffer[2], classmarkInfo3.length);

	ieLength = classmarkInfo3.length + 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.21
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						bandToBeUsed;
} A_Ie_Interference_Band_To_Be_Used_t;
*/

A_Result_t 
A_EncodeInterferenceBandToBeUsed(
	A_Ie_Interference_Band_To_Be_Used_t &interferenceBandToBeUsed,
	int &ieLength,
	unsigned char	*buffer)
{
	if(interferenceBandToBeUsed.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_INTERFERENCE_BAND_TO_BE_USED_TYPE;

	buffer[1] = interferenceBandToBeUsed.bandToBeUsed;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeInterferenceBandToBeUsed(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Interference_Band_To_Be_Used_t &interferenceBandToBeUsed)
{
	interferenceBandToBeUsed.ie_present = true;

	interferenceBandToBeUsed.elementIdentifier = buffer[0];

	interferenceBandToBeUsed.bandToBeUsed = buffer[1];

	ieLength = 2;

    return A_RESULT_SUCCESS;
}

// 3.2.2.22
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						rrCause;
} A_Ie_RR_Cause_t;
*/

A_Result_t 
A_EncodeRRCause(
	A_Ie_RR_Cause_t &rrCause,
	int &ieLength,
	unsigned char	*buffer)
{
	if(rrCause.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_RR_CAUSE_TYPE;

	buffer[1] = rrCause.rrCause;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeRRCause(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_RR_Cause_t &rrCause)
{
	rrCause.ie_present = true;

	rrCause.elementIdentifier = buffer[0];

	rrCause.rrCause = buffer[1];

	ieLength = 2;

    return A_RESULT_SUCCESS;
}

// 3.2.2.24
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						layer3Info[256];
} A_Ie_Layer3_Information_t;
*/

A_Result_t 
A_EncodeLayer3Information(
	A_Ie_Layer3_Information_t &layer3Info,
	int &ieLength,
	unsigned char	*buffer)
{
	if(layer3Info.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_LAYER3_INFORMATION_TYPE;

	buffer[1] = layer3Info.length;

	memcpy(&buffer[2], layer3Info.layer3Info, layer3Info.length);

	ieLength = layer3Info.length + 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeLayer3Information(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Layer3_Information_t &layer3Info)
{
	layer3Info.ie_present = true;

	layer3Info.elementIdentifier = buffer[0];

	layer3Info.length = buffer[1];

	if (layer3Info.length > 0)
		memcpy(layer3Info.layer3Info, &buffer[2], layer3Info.length);

	ieLength = layer3Info.length + 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.26
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						downlinkDTXFlag;
} A_Ie_Downlink_DTX_Flag_t;
*/

A_Result_t 
A_EncodeDownlinkDTXFlag(
	A_Ie_Downlink_DTX_Flag_t &downlinkDTXFlag,
	int &ieLength,
	unsigned char	*buffer)
{
	if(downlinkDTXFlag.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_DOWNLINK_DTX_FLAG_TYPE;

	buffer[1] = downlinkDTXFlag.downlinkDTXFlag;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeDownlinkDTXFlag(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Downlink_DTX_Flag_t &downlinkDTXFlag)
{
	downlinkDTXFlag.ie_present = true;

	downlinkDTXFlag.elementIdentifier = buffer[0];

	downlinkDTXFlag.downlinkDTXFlag = buffer[1];

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.30 (One of Information1 or Information2)
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						classmark;
} A_Ie_Classmark_Information1_t;
*/

A_Result_t 
A_EncodeClassmarkInformation1(
	A_Ie_Classmark_Information1_t &classmarkInfo1,
	int &ieLength,
	unsigned char	*buffer)
{
	if(classmarkInfo1.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CLASSMARK_INFORMATION1_TYPE;

	buffer[1] = classmarkInfo1.classmark;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeClassmarkInformation1(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Classmark_Information1_t &classmarkInfo1)
{
	classmarkInfo1.ie_present = true;

	classmarkInfo1.elementIdentifier = buffer[0];

	classmarkInfo1.classmark = buffer[1];

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.33
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						channelMode:4;
	unsigned char						channel:4;
} A_Ie_Chosen_Channel_t;
*/

A_Result_t 
A_EncodeChosenChannel(
	A_Ie_Chosen_Channel_t &chosenChannel,
	int &ieLength,
	unsigned char	*buffer)
{
	if(chosenChannel.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CHOSEN_CHANNEL_TYPE;

	buffer[1] = ((chosenChannel.channelMode << 4) & 0xF0) | (chosenChannel.channel & 0x0F);

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeChosenChannel(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Chosen_Channel_t &chosenChannel)
{
	chosenChannel.ie_present = true;

	chosenChannel.elementIdentifier = buffer[0];

	chosenChannel.channelMode = buffer[1] >> 4;

	chosenChannel.channel = buffer[1] & 0x0F;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.44
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						algorithmId;
} A_Ie_Chosen_Encrypt_Algorithm_t;
*/

A_Result_t 
A_EncodeChosenEncryptAlorithm(
	A_Ie_Chosen_Encrypt_Algorithm_t &chosenAlgorithm,
	int &ieLength,
	unsigned char	*buffer)
{
	if(chosenAlgorithm.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CHOSEN_ENCRYPTION_ALGORITHM_TYPE;

	buffer[1] = chosenAlgorithm.algorithmId;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeChosenEncryptAlorithm(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Chosen_Encrypt_Algorithm_t &chosenAlgorithm)
{
	chosenAlgorithm.ie_present = true;

	chosenAlgorithm.elementIdentifier = buffer[0];

	chosenAlgorithm.algorithmId = buffer[1];

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.45
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						circuitPoolNumber;
} A_Ie_Circuit_Pool_t;
*/

A_Result_t 
A_EncodeCircuitPool(
	A_Ie_Circuit_Pool_t &circuitPool,
	int &ieLength,
	unsigned char	*buffer)
{
	if(circuitPool.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CIRCUIT_POOL_TYPE;

	buffer[1] = circuitPool.circuitPoolNumber;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeCircuitPool(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Circuit_Pool_t &circuitPool)
{
	circuitPool.ie_present = true;

	circuitPool.elementIdentifier = buffer[0];

	circuitPool.circuitPoolNumber = buffer[1];

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.46
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						circuitPoolList[32];
} A_Ie_Circuit_Pool_List_t;
*/

A_Result_t 
A_EncodeCircuitPoolList(
	A_Ie_Circuit_Pool_List_t &circuitPoolList,
	int &ieLength,
	unsigned char	*buffer)
{
	if(circuitPoolList.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CIRCUIT_POOL_LIST_TYPE;

	buffer[1] = circuitPoolList.length;

	memcpy(&buffer[2], circuitPoolList.circuitPoolList, circuitPoolList.length);

	ieLength = circuitPoolList.length + 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeCircuitPoolList(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Circuit_Pool_List_t &circuitPoolList)
{
	circuitPoolList.ie_present = true;

	circuitPoolList.elementIdentifier = buffer[0];

	circuitPoolList.length = buffer[1];

	if (circuitPoolList.length > 0)
		memcpy(circuitPoolList.circuitPoolList, &buffer[2], circuitPoolList.length);

	ieLength = circuitPoolList.length + 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.49
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						channelMode:4;
	unsigned char						channel:4;
} A_Ie_Current_Channel_Type1_t;
*/

A_Result_t 
A_EncodeCurrentChannelType1(
	A_Ie_Current_Channel_Type1_t &currentChannelType,
	int &ieLength,
	unsigned char	*buffer)
{
	if(currentChannelType.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_CURRENT_CHANNEL_TYPE1_TYPE;

	buffer[1] = ((currentChannelType.channelMode << 4) & 0xF0) | (currentChannelType.channel & 0x0F);

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeCurrentChannelType1(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Current_Channel_Type1_t &currentChannelType)
{
	currentChannelType.ie_present = true;

	currentChannelType.elementIdentifier = buffer[0];

	currentChannelType.channelMode = buffer[1] >> 4;

	currentChannelType.channel = buffer[1] & 0x0F;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.51
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						speechVersionId;
} A_Ie_Speech_Version_t;
*/

A_Result_t 
A_EncodeSpeechVersion(
	A_Ie_Speech_Version_t &speechVersion,
	int &ieLength,
	unsigned char	*buffer)
{
	if(speechVersion.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_SPEECH_VERSION_TYPE;

	buffer[1] = speechVersion.speechVersionId & 0x7F;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeSpeechVersion(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Speech_Version_t &speechVersion)
{
	speechVersion.ie_present = true;

	speechVersion.elementIdentifier = buffer[0];

	speechVersion.speechVersionId = buffer[1];

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.54
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
} A_Ie_Talker_Flag_t;
*/

A_Result_t 
A_EncodeTalkerFlag(
	A_Ie_Talker_Flag_t &talkerFlag,
	int &ieLength,
	unsigned char	*buffer)
{
	if(talkerFlag.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_SPEECH_VERSION_TYPE;

	ieLength = 1;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeTalkerFlag(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Talker_Flag_t &talkerFlag)
{
	talkerFlag.ie_present = true;

	talkerFlag.elementIdentifier = buffer[0];

	ieLength = 1;

	return A_RESULT_SUCCESS;
}

// 3.2.2.55
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						groupDesc[4];
} A_Ie_Group_Call_Reference_t;
*/

A_Result_t 
A_EncodeGroupCallReference(
	A_Ie_Group_Call_Reference_t &groupCallReference,
	int &ieLength,
	unsigned char	*buffer)
{
	if(groupCallReference.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_GROUP_CALL_REFERENCE_TYPE;

	buffer[1] = groupCallReference.length;

	memcpy(&buffer[2], groupCallReference.groupDesc, groupCallReference.length);

	ieLength = groupCallReference.length + 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeGroupCallReference(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Group_Call_Reference_t &groupCallReference)
{
	groupCallReference.ie_present = true;

	groupCallReference.elementIdentifier = buffer[0];

	groupCallReference.length = buffer[1];

	if (groupCallReference.length > 0)
		memcpy(groupCallReference.groupDesc, &buffer[2], groupCallReference.length);

	ieLength = groupCallReference.length + 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.57
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						SMI;
} A_Ie_Config_Evolution_Indication_t;
*/

A_Result_t 
A_EncodeConfigEvolutionIndication(
	A_Ie_Config_Evolution_Indication_t &configEvolutionIndication,
	int &ieLength,
	unsigned char	*buffer)
{
	if(configEvolutionIndication.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_GROUP_CALL_REFERENCE_TYPE;

	buffer[1] = configEvolutionIndication.SMI & 0xF;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeConfigEvolutionIndication(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Config_Evolution_Indication_t &configEvolutionIndication)
{
	configEvolutionIndication.ie_present = true;

	configEvolutionIndication.elementIdentifier = buffer[0];

	configEvolutionIndication.SMI = buffer[1] & 0xF;

	ieLength = 2;

	return A_RESULT_SUCCESS;
}

// 3.2.2.59
/*
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						oldBssToNewBssIe[256];
} A_Ie_Old_BSS_To_New_BSS_Info_t;
*/

A_Result_t 
A_EncodeBssToNewBssInfo(
	A_Ie_Old_BSS_To_New_BSS_Info_t &oldBssToNewBssInfo,
	int &ieLength,
	unsigned char	*buffer)
{
	if(oldBssToNewBssInfo.ie_present != true)
	{
		return A_IE_MISSING;
	}

	buffer[0] = A_OLD_BSS_TO_NEW_BSS_INFORMATION_TYPE;

	buffer[1] = oldBssToNewBssInfo.length;

	memcpy(&buffer[2], oldBssToNewBssInfo.oldBssToNewBssIe, oldBssToNewBssInfo.length);

	ieLength = oldBssToNewBssInfo.length + 2;

	return A_RESULT_SUCCESS;
}

A_Result_t 
A_DecodeBssToNewBssInfo(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Old_BSS_To_New_BSS_Info_t &oldBssToNewBssInfo)
{
	oldBssToNewBssInfo.ie_present = true;

	oldBssToNewBssInfo.elementIdentifier = buffer[0];

	oldBssToNewBssInfo.length = buffer[1];

	if (oldBssToNewBssInfo.length > 0)
		memcpy(oldBssToNewBssInfo.oldBssToNewBssIe, &buffer[2], oldBssToNewBssInfo.length);

	ieLength = oldBssToNewBssInfo.length + 2;

	return A_RESULT_SUCCESS;
}
