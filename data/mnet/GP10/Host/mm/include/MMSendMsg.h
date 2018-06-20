#ifndef MMSendMsg_H
#define MMSendMsg_H

// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : MMSendMsg.h
// Author(s)   : Kevin Lim
// Create Date : 07-26-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// function prototypes
// *******************************************************************


void buildMSMsgCommon();

T_CNI_RIL3_RESULT sendMSMsg(T_CNI_LAPDM_OID            oid,
							T_CNI_LAPDM_SAPI           sapi);

void sendIntRRMsg(	T_CNI_IRT_ID               entryId,
					IntraL3PrimitiveType_t     prim,
					IntraL3MsgType_t           msgType);

void sendIntRRDataMsg(	T_CNI_IRT_ID               entryId,
					IntraL3PrimitiveType_t     prim,
					IntraL3MsgType_t           msgType,
					IntraL3MsgData_t		   l3_data);

void sendIntCCMsg(	short                      mmId,
					IntraL3PrimitiveType_t     prim,
					IntraL3MsgType_t           msgType);

void bounceIntCCMsg(MSG_Q_ID                   msgQforCC,
					IntraL3PrimitiveType_t     prim,
					IntraL3MsgType_t           msgType);


#endif                                 // MMSendMsg_H
