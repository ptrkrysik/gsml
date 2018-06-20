#ifndef LUDBOamMsgProc_H
#define LUDBOamMsgProc_H

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
// File        : LUDBOamMsgProc.h
// Author(s)   : Kevin Lim
// Create Date : 09-09-99
// Description : Ludb module OAM msg handler header 
//
// *******************************************************************

// *******************************************************************
// function prototypes
// *******************************************************************

void ludbHandleOpStChanged(INT32 value);
void ludbHandleAuthFreqChanged(INT32 value);
void LUDB_OAM_MsgProc(TrapMsg *trapMsg);


#endif                                 // LUDBOamMsgProc_H
