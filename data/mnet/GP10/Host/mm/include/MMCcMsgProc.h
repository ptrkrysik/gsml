#ifndef MMCcMsgProc_H
#define MMCcMsgProc_H

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
// File        : MMCcMsgProc.h
// Author(s)   : Kevin Lim
// Create Date : 07-26-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// function prototypes
// *******************************************************************


void sendPageReqToRRM(short mmId);
void MMPageReqCleanUp(short mmId);

void MM_CC_PageReqProc(IntraL3Msg_t *ccInMsg);

void MM_CC_HOReqProc(IntraL3Msg_t *ccInMsg);

void MM_CC_MMEstabReqProc(IntraL3Msg_t *ccInMsg);

void MM_CC_MMRelReqProc(IntraL3Msg_t *ccInMsg);

void MM_CC_MsgProc(IntraL3Msg_t *ccInMsg);

short sendLocUpInPageReq(T_CNI_RIL3_IE_MOBILE_ID mobileId, short mmId);
	
#endif                                 // MMCcMsgProc_H
