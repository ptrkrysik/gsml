#ifndef MMRmMsgProc_H
#define MMRmMsgProc_H

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
// File        : MMRmMsgProc.h
// Author(s)   : Kevin Lim
// Create Date : 07-26-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// function prototypes
// *******************************************************************


void MM_RM_RRPageRspProc(IntraL3Msg_t *rrInMsg);

void MMRRPageRspCleanUp(short mmId);
void MMProceedRRPageRspAuth(short mmId);
void MMProceedRRPageRspCipher(short mmId);
void MMProceedRRPageRspToCC(short mmId);
void MMProceedRRPageRspToVBLINK(short mmId);

void MM_RM_RRHOAckProc(IntraL3Msg_t *rrInMsg);

void MM_RM_RREstabCnfProc(IntraL3Msg_t *rrInMsg);

void MM_RM_RRHONackProc(IntraL3Msg_t *rrInMsg);

void MM_RM_RREstabRejProc(IntraL3Msg_t *rrInMsg);

void MM_RM_RRRelIndProc(IntraL3Msg_t *rrInMsg);

void MM_RM_MsgProc(IntraL3Msg_t *rrInMsg);


#endif                                 // MMRmMsgProc_H
