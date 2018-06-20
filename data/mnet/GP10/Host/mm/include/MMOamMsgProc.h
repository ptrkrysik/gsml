#ifndef MMOamMsgProc_H
#define MMOamMsgProc_H

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

void mmHandleOpStChanged(INT32 value);
void MM_OAM_MsgProc(TrapMsg *trapMsg);


#endif                                 // MMOamMsgProc_H
