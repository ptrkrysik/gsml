#ifndef LUDBMmMsgProc_H
#define LUDBMmMsgProc_H

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
// File        : LUDBMmMsgProc.h
// Author(s)   : Kevin Lim 
// Create Date : 07-31-99
// Description : Local User Database 
//
// *******************************************************************

#include "JCC/LudbApi.h"

bool sendLudbRspToMM(LudbMsgStruct *rsp);
bool sendMobileRelReqToMM(short mmId);
bool sendMobileRegCnfToMM(short mmId, short ludbId, short forceAuth);
bool sendRemEmergRspToMM(short mmId);
bool sendMobileSecRspToMM(short mmId, short ludbId);
bool sendMobileRegRejToMM(short mmId, T_CNI_RIL3_REJECT_CAUSE_VALUE cause);
bool sendMobileUnRegCnfToMM(short mmId);
bool sendMobileUnRegRejToMM(short mmId);
void LUDB_MM_EmergReqProc(T_CNI_RIL3_IE_MOBILE_ID& mobile_id, short mmId);
void LUDB_MM_RemEmergReqProc(short mmId, short ludb_id);
void LUDB_MM_RegReqProc(T_CNI_RIL3_IE_MOBILE_ID& mobile_id, short mmId);
void LUDB_MM_UnRegReqProc(T_CNI_RIL3_IE_MOBILE_ID& mobile_id, short mmId);
void LUDB_MM_SecReqProc(short mm_id, short ludb_id);
void LUDB_MM_CipherSetProc(short mm_id, short ludb_id, short algo);
void LUDB_MM_MsgProc(LudbMsgStruct *inMsg);



#endif                                       // LUDBMmMsgProc_H



