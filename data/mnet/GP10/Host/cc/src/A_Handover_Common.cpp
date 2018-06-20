// *******************************************************************
//
// (c) Copyright Cisco 2001
// All Rights Reserved
//
// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : ($CC)/src/A_Handover_Common.cpp
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

#include "CC\A_Handover_Common.h"

A_Result_t 
AIF_Encode(
	T_AIF_MSG *AIF_Msg,
	T_AIF_MessageUnit *AIF_MsgUnit)
{
    unsigned char *buffer = AIF_MsgUnit->buffer;
    int length = AIF_MSG_LENGTH;
    A_Result_t result;

	if (length < 2) return A_INCORRECT_LENGTH;
   
    switch(AIF_Msg->header.msgType)
    {
    case A_HANDOVER_REQUEST_TYPE:
        result = A_EncodeHandoverRequest(
                    AIF_Msg->handoverRequest,
                    buffer,
                    length);
        AIF_MsgUnit->msgLength = length;
        return result;
    case A_HANDOVER_REQUEST_ACK_TYPE:
        result = A_EncodeHandoverRequestAck(
                    AIF_Msg->handoverRequestAck,
                    buffer,
                    length);
        AIF_MsgUnit->msgLength = length;
        return result;
    case A_HANDOVER_COMPLETE_TYPE:
        result = A_EncodeHandoverComplete(
                    AIF_Msg->handoverComplete,
                    buffer,
                    length);
        AIF_MsgUnit->msgLength = length;
        return result;
    case A_HANDOVER_FAILURE_TYPE:
        result = A_EncodeHandoverFailure(
                    AIF_Msg->handoverFailure,
                    buffer,
                    length);
        AIF_MsgUnit->msgLength = length;
        return result;
    case A_HANDOVER_DETECT_TYPE:
        result = A_EncodeHandoverDetect(
                    AIF_Msg->handoverDetect,
                    buffer,
                    length);
        AIF_MsgUnit->msgLength = length;
        return result;
    case A_QUEUING_INDICATION_TYPE:
        result = A_EncodeMessageType(
	            A_RESULT_SUCCESS,
	            length,
	            buffer);
        AIF_MsgUnit->msgLength = length;
        return result;
    default:
		return A_UNKNOWN_MESSAGE_TYPE;
    }
}

A_Result_t 
AIF_Decode(
	T_AIF_MessageUnit *AIF_MsgUnit,
    T_AIF_MSG *AIF_Msg)
{
    unsigned char *buffer = AIF_MsgUnit->buffer;
    int length = AIF_MsgUnit->msgLength;

	if (length < 2) return A_INCORRECT_LENGTH;
   
    AIF_Msg->header.msgType = AIF_MsgUnit->buffer[0];

    switch(AIF_Msg->header.msgType)
    {
    case A_HANDOVER_REQUEST_TYPE:
        return A_DecodeHandoverRequest(
                &buffer[1],
                length,
                AIF_Msg->handoverRequest);
    case A_HANDOVER_REQUEST_ACK_TYPE:
        return A_DecodeHandoverRequestAck(
                &buffer[1],
                length,
                AIF_Msg->handoverRequestAck);
    case A_HANDOVER_COMPLETE_TYPE:
        return A_DecodeHandoverComplete(
                &buffer[1],
                length,
                AIF_Msg->handoverComplete);
    case A_HANDOVER_FAILURE_TYPE:
        return A_DecodeHandoverFailure(
                &buffer[1],
                length,
                AIF_Msg->handoverFailure);
    case A_HANDOVER_DETECT_TYPE:
        return A_DecodeHandoverDetect(
                &buffer[1],
                length,
                AIF_Msg->handoverDetect);
    case A_QUEUING_INDICATION_TYPE:
        AIF_Msg->msgType = A_QUEUING_INDICATION_TYPE;
        return A_RESULT_SUCCESS;
    default:
		return A_UNKNOWN_MESSAGE_TYPE;
    }
}

