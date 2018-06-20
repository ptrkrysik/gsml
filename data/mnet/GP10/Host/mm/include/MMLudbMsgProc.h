#ifndef MMLudbMsgProc_H
#define MMLudbMsgProc_H

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
// File        : MMLudbMsgProc.h
// Author(s)   : Kevin Lim
// Create Date : 07-26-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// function prototypes
// *******************************************************************

#include "JCC/LudbApi.h"

bool sendMMRequestToLudb(LudbMsgStruct *request);
bool sendCipherCmdRecordToLudb(short mmId, short algo);
bool sendMobileSecInfoReqToLudb(short mmId, short ludbId);
bool sendMobileRegReqToLudb( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId);
bool sendMobileUnRegReqToLudb( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId);
bool sendInsertEmergencyToLudb(short mmId);
bool sendRemoveEmergencyToLudb(short mmId);
void MM_Ludb_SecRspProc(short mmId, short ludbId);
void MM_Ludb_RegCnfProc(short mmId, short ludbId, short forceAuth);
void MMProceedLocUpdateAuth(short mmId, short ludbId, short forceAuth);
void MMProceedLocUpdateCipher(short mmId, short ludbId);
void MMProceedLocUpdateRegCnf(short mmId, short ludbId);
void MM_Ludb_RegRejProc(short mmId, T_CNI_RIL3_REJECT_CAUSE_VALUE cause);
void MM_Ludb_UnRegCnfProc(short mmId);
void MM_Ludb_UnRegRejProc(short mmId);
void MM_Ludb_MsgProc(LudbMsgStruct *LudbInMsg);

#endif                                 // MMLudbMsgProc_H
