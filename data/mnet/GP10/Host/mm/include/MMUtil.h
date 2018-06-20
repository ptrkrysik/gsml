#ifndef MMUtil_H
#define MMUtil_H

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
// File        : MMUtil.h
// Author(s)   : Kevin Lim
// Create Date : 05-25-99
// Description : MM module utilities 
//
// *******************************************************************

// *******************************************************************
// function prototypes
// *******************************************************************

#include "ril3/ril3_common.h"

void mmConnDataInit(void);
void irtMMDataInit(void);
short MM_FindIrtMMId(T_CNI_IRT_ID entryId);
void mmDataInit(void);
void entryPrint(MMEntry_t &mmEntry);
void entryInit(MMEntry_t &mmEntry);
short checkExistingEntry(const T_CNI_IRT_ID newEntryId);
short getEntry(const T_CNI_RIL3_IE_MOBILE_ID &mobId);
short getEntry(const short ludbId);
short getMobileId(short mmId, T_CNI_RIL3_IE_MOBILE_ID &mobId);
T_CNI_RIL3_IE_MOBILE_ID getMobileId(short mmId);
short getMMId(const short ludbId);
short entryAlloc();
void entryDealloc(short i);

void mmDataPrint(int number);
char *getMMEntryState(short mmId);
char *getMMProcStateString(MMProcedureType_t proc_type);
char *getMMStateString(MMState_t state);
char *getMMPrimStr(IntraL3PrimitiveType_t prim);
char *getMMMsgTypeStr(IntraL3MsgType_t msgType);
char *getRIL3MMTypeStr(T_CNI_RIL3_MESSAGE_TYPE msgType);
char *getMMRejectCauseStr(T_CNI_RIL3_REJECT_CAUSE_VALUE cause);
void mmBlank();

#endif                                 // MMUtil_H
