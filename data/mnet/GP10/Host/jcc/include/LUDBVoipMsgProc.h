#ifndef LUDBVoipMsgProc_H
#define LUDBVoipMsgProc_H

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
// File        : LUDBVoipMsgProc.h
// Author(s)   : Kevin Lim 
// Create Date : 07-31-99
// Description : Local User Database 
//
// *******************************************************************

#include "JCC/Ludbapi.h"

bool sendMobileRegReq( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId, short ludbId, short fSec);
bool sendMobileUnRegReq( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId, short ludbId);
bool sendMobileSecInfoReq( T_CNI_RIL3_IE_MOBILE_ID& mobileId, short mmId, short ludbId, short fReg);
void LUDB_VBLINK_RCFMsgProc(LudbMsgStruct *inMsg);
void LUDB_VBLINK_RRJMsgProc(LudbMsgStruct *inMsg);
void LUDB_VBLINK_UCFMsgProc(LudbMsgStruct *inMsg);
void LUDB_VBLINK_URJMsgProc(LudbMsgStruct *inMsg);
void LUDB_VBLINK_SPIMsgProc(LudbMsgStruct *inMsg);
void LUDB_VBLINK_SRSMsgProc(LudbMsgStruct *inMsg);
void LUDB_VBLINK_URQMsgProc(LudbMsgStruct *inMsg);
void LUDB_VBLINK_MsgProc(LudbMsgStruct *inMsg);


#endif                                       // LUDBVoipMsgProc_H



