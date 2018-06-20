// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************
// PRODUCT:		GSM 08.08 A Interface ED
// PLATFORM:	COMMON CORE
// FILE:		($CC)/include/A_Handover_Common.h
// RELEASE:		
//	
// FILE DESCRIPTION:
//
// 
// CHANGE HISTORY:
//
//		Date		Description
//		----------	--------------------------------------------
//		2001-05-18	First Incarnation
//
// =================================================================

#ifndef A_HANDOVER_COMMON_H
#define A_HANDOVER_COMMON_H

#include "CC\A_Handover_Ie.h"
#include "CC\A_Handover_Request_Msg.h"
#include "CC\A_Handover_Request_Ack_Msg.h"
#include "CC\A_Handover_Complete_Msg.h"
#include "CC\A_Handover_Detect_Msg.h"
#include "CC\A_Handover_Failure_Msg.h"


#define A_CHECK_MANDATORY_IE_RETURN_CODE(resultCode)	\
	if(resultCode != A_RESULT_SUCCESS)	\
	{	\
		if(resultCode == A_IE_MISSING)	\
			return A_MANDATORY_IE_MISSING;	\
		else if(resultCode == A_MISC_IE_ERROR)	\
			return A_NON_SEMANTICAL_MANDATORY_IE_ERROR;	\
		else	\
			return resultCode;\
	}


#define A_CHECK_OPTIONAL_IE_RETURN_CODE(resultCode)	\
	if(resultCode != A_RESULT_SUCCESS)	\
	{	\
		if(resultCode == A_MISC_IE_ERROR)	\
			return A_NON_IMPERATIVE_MESSAGE_PART_ERROR;	\
		else	\
			return resultCode;\
	}

// 3.2.2.1
typedef enum
{
// Assignment Messages
	A_ASSIGNMENT_REQUEST_TYPE			= 0x1,
	A_ASSIGNMENT_COMPLETE_TYPE			= 0x2,
	A_ASSIGNMENT_FAILURE_TYPE			= 0x3,
// Handover Message
	A_HANDOVER_REQUEST_TYPE				= 0x10,
	A_HANDOVER_REQUIRED_TYPE			= 0x11,
	A_HANDOVER_REQUEST_ACK_TYPE			= 0x12,
	A_HANDOVER_COMMAND_TYPE				= 0x13,
	A_HANDOVER_COMPLETE_TYPE			= 0x14,
	A_HANDOVER_SUCCEEDED_TYPE			= 0x15,
	A_HANDOVER_FAILURE_TYPE				= 0x16,
	A_HANDOVER_PERFORMED_TYPE			= 0x17,
	A_HANDOVER_CANDIDATE_ENQUIRE_TYPE	= 0x18,
	A_HANDOVER_CANDIDATE_RESPONSE_TYPE	= 0x19,
	A_HANDOVER_REQUIRED_REJECT_TYPE		= 0x1A,
	A_HANDOVER_DETECT_TYPE				= 0x1B,
// Release Messages
	A_CLEAR_COMMAND_TYPE				= 0x20,
	A_CLEAR_COMPLETE_TYPE				= 0x21,
	A_CLEAR_REQUEST_TYPE				= 0x22,
	A_SAPI_N_REJECT_TYPE				= 0x25,
	A_CONFUSION_TYPE					= 0x26,
// Other Connection Related Messages
	A_SUSPEND_TYPE						= 0x28,
	A_RESUME_TYPE						= 0x29,
// General Messages
	A_RESET_TYPE						= 0x30,
	A_RESET_ACK_TYPE					= 0x31,
	A_OVERLOAD_TYPE						= 0x32,
	A_RESET_CIRCUIT_TYPE				= 0x34,
	A_RESET_CIRCUIT_ACK_TYPE			= 0x35,
	A_MSC_INVOKE_TRACE_TYPE				= 0x36,
	A_BSS_INVOKE_TRACE_TYPE				= 0x37,
// Terrestrial Resource Messages
	A_BLOCK_TYPE						= 0x40,
	A_BLOCKING_ACK_TYPE					= 0x41,
	A_UNBLOCK_TYPE						= 0x42,
	A_UNBLOCK_ACK_TYPE					= 0x43,
	A_CIRCUIT_GROUP_BLOCK_TYPE			= 0x44,
	A_CIRCUIT_GROUP_BLOCKING_ACK_TYPE	= 0x45,
	A_CIRCUIT_GROUP_UNBLOCKING_TYPE		= 0x46,
	A_CIRCUIT_GROUP_UNBLOCKING_ACK_TYPE = 0x47,
	A_UNEQUIPPED_CIRCUIT_TYPE			= 0x48,
	A_CHANGE_CIRCUIT_TYPE				= 0x4E,
	A_CHANGE_CIRCUIT_ACK_TYPE			= 0x4F,
// Radio Resource Messages
	A_RESOURCE_REQUEST_TYPE				= 0x50,
	A_RESORUCE_INDICATION_TYPE			= 0x51,
	A_PAGING_TYPE						= 0x52,
	A_CIPHER_MODE_COMMAND_TYPE			= 0x53,
	A_CLASSMARK_UPDATE_TYPE				= 0x54,
	A_CIPHER_MODE_COMPLETE_TYPE			= 0x55,
	A_QUEUING_INDICATION_TYPE			= 0x56,
	A_COMPLETE_LAYER3_INFORMATION_TYPE	= 0x57,
	A_CLASSMARK_REQUEST_TYPE			= 0x58,
	A_CIPHER_MODE_REJECT_TYPE			= 0x59,
	A_LOAD_INDICATION_TYPE				= 0x5A,
// VGCS/VBS
	A_VGCS_VBS_SETUP_TYPE				= 0x4,
	A_VGCS_VBS_SETUP_ACK_TYPE			= 0x5,
	A_VGCS_VBS_SETUP_REFUSE_TYPE		= 0x6,
	A_VGCS_VBS_ASSIGNMENT_REQUEST_TYPE	= 0x7,
	A_VGCS_VBS_ASSIGNMENT_RESULT_TYPE	= 0x1C,
	A_VGCS_VBS_ASSIGNMENT_FAILURE_TYPE	= 0x1D,
	A_VGCS_VBS_QUEUING_INDICATION_TYPE	= 0x1E,
	A_UPLINK_REQUEST_TYPE				= 0x1F,
	A_UPLINK_REQUEST_ACK_TYPE			= 0x27,
	A_UPLINK_REQUEST_CONFIRMATION_TYPE	= 0x49,
	A_UPLINK_RELEASE_INDICATION_TYPE	= 0x4A,
	A_UPLINK_REJECT_COMMAND_TYPE		= 0x4B,
	A_UPLINK_RELEASE_COMMAND_TYPE		= 0x4C,
	A_UPLINK_SEIZED_COMMAND_TYPE		= 0x4D,
} A_IE_MESSAGE_TYPE;

typedef struct
{
    A_Ie_Message_Type_t   msgType;
} AIF_Header_t;

#define AIF_MSG_LENGTH	251	
typedef struct
{
	unsigned short	msgLength;
	unsigned char	buffer[AIF_MSG_LENGTH];
} T_AIF_MessageUnit;

typedef union 
{
	A_Ie_Message_Type_t   				msgType;               //ext-HO <xxu:06-08-01>
	AIF_Header_t    							header;
	A_Handover_Request_t    					handoverRequest;
	A_Handover_Request_Ack_t    				handoverRequestAck;
	A_Handover_Complete_t    					handoverComplete;
	A_Handover_Failure_t    					handoverFailure;
	A_Handover_Detect_t    					    handoverDetect;
} T_AIF_MSG;

extern A_Result_t 
AIF_Encode(
	T_AIF_MSG *, T_AIF_MessageUnit *);

extern A_Result_t
AIF_Decode(
	T_AIF_MessageUnit *, T_AIF_MSG *);

#endif // A_HANDOVER_COMMON_H
