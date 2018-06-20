// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Ie.h
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

#ifndef A_HANDOVER_IE_H
#define A_HANDOVER_IE_H

#include <stdio.h>
#include <stdlib.h>
#include <vxworks.h>
#include <string.h>

typedef enum 
{
	A_RESULT_SUCCESS						= 0, 
	A_UNKNOWN_MESSAGE_TYPE				= 1,
	A_MANDATORY_IE_MISSING				= 2,
	A_INCORRECT_LENGTH					= 3,
	A_NON_SEMANTICAL_MANDATORY_IE_ERROR	= 4,
	A_NON_IMPERATIVE_MESSAGE_PART_ERROR	= 5,
	A_SKIP								= 6,
	A_NON_RRM_MESSAGE					= 7,
	A_NON_MM_MESSAGE						= 8,
	A_NON_CC_MESSAGE						= 9,
	A_NON_SS_MESSAGE						= 10,
	// below are used for internal encoding/decoding purpose
	A_IE_MISSING							= 11,
	A_MISC_IE_ERROR						= 12,
	A_CONTINUE							= 13
} A_RESULT_t;

typedef unsigned int A_Result_t;

typedef unsigned char A_Ie_Message_Type_t;

typedef enum
{
	A_CIRCUIT_IDENTITY_CODE_TYPE		= 0x1,			// 3.2.2.2
	A_RESOURCE_AVAILABLE_TYPE			= 0x3,			// 3.2.2.4
	A_CAUSE_TYPE						= 0x4,			// 3.2.2.5
	A_CELL_IDENTIFIER_TYPE				= 0x5,			// 3.2.2.17
	A_PRIORITY_TYPE						= 0x6,			// 3.2.2.18
	A_LAYER3_HEADER_INFORMATION_TYPE	= 0x7,			// 3.2.2.9
	A_IMSI_TYPE							= 0x8,			// 3.2.2.6
	A_TMSI_TYPE							= 0x9,			// 3.2.2.7
	A_ENCRYPTION_INFORMATION_TYPE		= 0xA,			// 3.2.2.10
	A_CHANNEL_TYPE_TYPE					= 0xB,			// 3.2.2.11
	A_PERIODITY_TYPE					= 0xC,			// 3.2.2.12
	A_EXTENDED_RESOURCE_INDICATOR_TYPE	= 0xD,			// 3.2.2.13
	A_NUMBER_OF_MSS_TYPE				= 0xE,			// 3.2.2.8
	A_CLASSMARK_INFORMATION2_TYPE		= 0x12,			// 3.2.2.19
	A_CLASSMARK_INFORMATION3_TYPE		= 0x13,			// 3.2.2.20
	A_INTERFERENCE_BAND_TO_BE_USED_TYPE	= 0x14,			// 3.2.2.21
	A_RR_CAUSE_TYPE						= 0x15,			// 3.2.2.22
	A_LAYER3_INFORMATION_TYPE			= 0x17,			// 3.2.2.24
	A_DLCI_TYPE							= 0x18,			// 3.2.2.25
	A_DOWNLINK_DTX_FLAG_TYPE			= 0x19,			// 3.2.2.26
	A_CELL_IDENTIFIER_LIST_TYPE			= 0x1A,			// 3.2.2.27
	A_RESPONSE_REQUEST_TYPE				= 0x1B,			// 3.2.2.28
	A_RESORUCE_INDICATION_METHOD_TYPE	= 0x1C,			// 3.2.2.29
	A_CLASSMARK_INFORMATION1_TYPE		= 0x1D,			// 3.2.2.30
	A_CIRCUIT_IDENTITY_CODE_LIST_TYPE	= 0x1E,			// 3.2.2.31
	A_DIAGNOSTIC_TYPE					= 0x1F,			// 3.2.2.32
	A_LAYER3_MESSAGE_CONTENTS_TYPE		= 0x20,			// 3.2.2.35
	A_CHOSEN_CHANNEL_TYPE				= 0x21,			// 3.2.2.33
	A_TOTAL_RESOURCE_ACCESSIBLE_TYPE	= 0x22,			// 3.2.2.14
	A_CIPHER_RESPONSE_MODE_TYPE			= 0x23,			// 3.2.2.34
	A_CHANNEL_NEEDED_TYPE				= 0x24,			// 3.2.2.36
	A_TRACE_TYPE_TYPE					= 0x25,			// 3.2.2.37
	A_TRIGGERID_TYPE					= 0x26,			// 3.2.2.38
	A_TRACE_REFERENCT_TYPE				= 0x27,			// 3.2.2.39
	A_TRANSACTIONID_TYPE				= 0x28,			// 3.2.2.40
	A_MOBILE_IDENTITY_TYPE				= 0x29,			// 3.2.2.41
	A_OMCID_TYPE						= 0x2A,			// 3.2.2.42
	A_FORWARD_INDICATOR_TYPE			= 0x2B,			// 3.2.2.43
	A_CHOSEN_ENCRYPTION_ALGORITHM_TYPE	= 0x2C,			// 3.2.2.44
	A_CIRCUIT_POOL_TYPE					= 0x2D,			// 3.2.2.45
	A_CIRCUIT_POOL_LIST_TYPE			= 0x2E,			// 3.2.2.46
	A_TIME_INDICATION_TYPE				= 0x2F,			// 3.2.2.47
	A_RESOURCE_SITUATION_TYPE			= 0x30,			// 3.2.2.48
	A_CURRENT_CHANNEL_TYPE1_TYPE		= 0x31,			// 3.2.2.49
	A_QUEUEING_INDICATOR_TYPE			= 0x32,			// 3.2.2.50
	A_SPEECH_VERSION_TYPE				= 0x40,			// 3.2.2.51
	A_ASSIGNMENT_REQUIREMENT_TYPE		= 0x33,			// 3.2.2.52
	A_TALKER_FLAG_TYPE					= 0x35,			// 3.2.2.54
	A_CONNECTION_RELEASE_REQUESTED_TYPE	= 0x36,			// 3.2.2.3
	A_GROUP_CALL_REFERENCE_TYPE			= 0x37,			// 3.2.2.55
	A_EMLPPP_PRIORITY_TYPE				= 0x38,			// 3.2.2.56
	A_CONFIGURATION_EVOLUTION_INDICATION	= 0x39,			// 3.2.2.57
	A_OLD_BSS_TO_NEW_BSS_INFORMATION_TYPE	= 0x3A			// 3.2.2.59
} A_ELEMENT_IDENTIFIER_TYPE;

// 3.2.2.2
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned short						pcmMultiplex:11;
	unsigned short						timeslot:5;
} A_Ie_Circuit_Identity_Code_t;

// 3.2.2.5
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						cause;
	unsigned char						cause2;
} A_Ie_Cause_t;

// 3.2.2.10
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						permittedAlgorithm;
	unsigned char						key[8];
} A_Ie_Encryption_Information_t;

// 3.2.2.11
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						speechDataIndicator;
	unsigned char						channelRateAndType;
	unsigned char						permittedIndicator[8];
} A_Ie_Channel_Type_t;

// 3.2.2.17
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						cellIdDescriminator;
	unsigned char						mcc[3];
	unsigned char						mnc[3];
	unsigned short						lac;
	unsigned short						ci;
} A_Ie_Cell_Identifier_t;

// 3.2.2.18
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						priority;
} A_Ie_Priority_t;

// 3.2.2.9
typedef struct {
	bool								ie_present:1;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						protocolDisc;
	unsigned char						transactionID;
} A_Ie_Layer3_Header_Information_t;

// 3.2.2.19
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						classmark[3];
} A_Ie_Classmark_Information2_t;

// 3.2.2.20
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						classmark[12];
} A_Ie_Classmark_Information3_t;

// 3.2.2.21
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						bandToBeUsed;
} A_Ie_Interference_Band_To_Be_Used_t;

// 3.2.2.22
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						rrCause;
} A_Ie_RR_Cause_t;

// 3.2.2.24
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						layer3Info[256];
} A_Ie_Layer3_Information_t;

// 3.2.2.26
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						downlinkDTXFlag;
} A_Ie_Downlink_DTX_Flag_t;

// 3.2.2.30 (One of Information1 or Information2)
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						classmark;
} A_Ie_Classmark_Information1_t;

// 3.2.2.33
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						channelMode:4;
	unsigned char						channel:4;
} A_Ie_Chosen_Channel_t;

// 3.2.2.44
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						algorithmId;
} A_Ie_Chosen_Encrypt_Algorithm_t;

// 3.2.2.45
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						circuitPoolNumber;
} A_Ie_Circuit_Pool_t;

// 3.2.2.46
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						circuitPoolList[32];
} A_Ie_Circuit_Pool_List_t;

// 3.2.2.49
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						channelMode:4;
	unsigned char						channel:4;
} A_Ie_Current_Channel_Type1_t;

// 3.2.2.51
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						speechVersionId;
} A_Ie_Speech_Version_t;

// 3.2.2.54
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
} A_Ie_Talker_Flag_t;

// 3.2.2.55
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						groupDesc[4];
} A_Ie_Group_Call_Reference_t;

// 3.2.2.57
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						SMI;
} A_Ie_Config_Evolution_Indication_t;

// 3.2.2.59
typedef struct {
	unsigned char						ie_present;
	unsigned char						elementIdentifier;
	unsigned char						length;
	unsigned char						oldBssToNewBssIe[256];
} A_Ie_Old_BSS_To_New_BSS_Info_t;


extern A_Result_t 
A_EncodeMessageType(
	A_Ie_Message_Type_t messageType,
	int             &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeMessageType(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Message_Type_t &messageType);

extern A_Result_t 
A_EncodeCircuitIdentityCode(
	A_Ie_Circuit_Identity_Code_t &cirIdenCode,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeCircuitIdentityCode(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Circuit_Identity_Code_t &cirIdenCode);

extern A_Result_t 
A_EncodeCause(
	A_Ie_Cause_t &cause,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeCause(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Cause_t &cause);

extern A_Result_t 
A_EncodeEncryptInformation(
	A_Ie_Encryption_Information_t &encryptInfo,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeEncryptInformation(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Encryption_Information_t &encryptInfo);

extern A_Result_t 
A_EncodeChannelType(
	A_Ie_Channel_Type_t &channelType,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeChannelType(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Channel_Type_t &channelType);

extern A_Result_t 
A_EncodeCellIdentifier(
	A_Ie_Cell_Identifier_t &cellIdentifier,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeCellIdentifier(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Cell_Identifier_t &cellIdentifier);

extern A_Result_t 
A_EncodePriority(
	A_Ie_Priority_t &priority,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodePriority(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Priority_t &priority);

extern A_Result_t 
A_EncodeLayer3HeaderInformation(
	A_Ie_Layer3_Header_Information_t &layer3HeaderInfo,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeLayer3HeaderInformation(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Layer3_Header_Information_t &layer3HeaderInfo);

extern A_Result_t 
A_EncodeClassmarkInformation2(
	A_Ie_Classmark_Information2_t &classmarkInfo2,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeClassmarkInformation2(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Classmark_Information2_t &classmarkInfo2);

extern A_Result_t 
A_EncodeClassmarkInformation3(
	A_Ie_Classmark_Information3_t &classmarkInfo3,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeClassmarkInformation3(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Classmark_Information3_t &classmarkInfo3);

extern A_Result_t 
A_EncodeInterferenceBandToBeUsed(
	A_Ie_Interference_Band_To_Be_Used_t &interferenceBandToBeUsed,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeInterferenceBandToBeUsed(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Interference_Band_To_Be_Used_t &interferenceBandToBeUsed);

extern A_Result_t 
A_EncodeRRCause(
	A_Ie_RR_Cause_t &rrCause,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeRRCause(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_RR_Cause_t &rrCause);

extern A_Result_t 
A_EncodeLayer3Information(
	A_Ie_Layer3_Information_t &layer3Info,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeLayer3Information(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Layer3_Information_t &layer3Info);

extern A_Result_t 
A_EncodeDownlinkDTXFlag(
	A_Ie_Downlink_DTX_Flag_t &downlinkDTXFlag,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeDownlinkDTXFlag(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Downlink_DTX_Flag_t &downlinkDTXFlag);

extern A_Result_t 
A_EncodeClassmarkInformation1(
	A_Ie_Classmark_Information1_t &classmarkInfo1,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeClassmarkInformation1(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Classmark_Information1_t &classmarkInfo1);

extern A_Result_t 
A_EncodeChosenChannel(
	A_Ie_Chosen_Channel_t &chosenChannel,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeChosenChannel(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Chosen_Channel_t &chosenChannel);

extern A_Result_t 
A_EncodeChosenEncryptAlorithm(
	A_Ie_Chosen_Encrypt_Algorithm_t &chosenAlgorithm,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeChosenEncryptAlorithm(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Chosen_Encrypt_Algorithm_t &chosenAlgorithm);

extern A_Result_t 
A_EncodeCircuitPool(
	A_Ie_Circuit_Pool_t &circuitPool,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeCircuitPool(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Circuit_Pool_t &circuitPool);

extern A_Result_t 
A_EncodeCircuitPoolList(
	A_Ie_Circuit_Pool_List_t &circuitPoolList,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeCircuitPoolList(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Circuit_Pool_List_t &circuitPoolList);

extern A_Result_t 
A_EncodeCurrentChannelType1(
	A_Ie_Current_Channel_Type1_t &currentChannelType,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeCurrentChannelType1(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Current_Channel_Type1_t &currentChannelType);

extern A_Result_t 
A_EncodeSpeechVersion(
	A_Ie_Speech_Version_t &speechVersion,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeSpeechVersion(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Speech_Version_t &speechVersion);

extern A_Result_t 
A_EncodeTalkerFlag(
	A_Ie_Talker_Flag_t &talkerFlag,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeTalkerFlag(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Talker_Flag_t &talkerFlag);

extern A_Result_t 
A_EncodeGroupCallReference(
	A_Ie_Group_Call_Reference_t &groupCallReference,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeGroupCallReference(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Group_Call_Reference_t &groupCallReference);

extern A_Result_t 
A_EncodeConfigEvolutionIndication(
	A_Ie_Config_Evolution_Indication_t &configEvolutionIndication,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeConfigEvolutionIndication(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Config_Evolution_Indication_t &configEvolutionIndication);

extern A_Result_t 
A_EncodeBssToNewBssInfo(
	A_Ie_Old_BSS_To_New_BSS_Info_t &oldBssToNewBssInfo,
	int &ieLength,
	unsigned char	*buffer);

extern A_Result_t 
A_DecodeBssToNewBssInfo(
	unsigned char	*buffer,
	int				&ieLength,
	A_Ie_Old_BSS_To_New_BSS_Info_t &oldBssToNewBssInfo);

#endif // A_HANDOVER_REQUEST_ACK_MSG_H

