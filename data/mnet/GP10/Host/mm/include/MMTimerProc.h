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
// File        : MMTimerProc.h
// Author(s)   : Kevin Lim
// Create Date : 07-26-99
// Description : 
//
// *******************************************************************
#ifndef MMTimerProc_H
#define MMTimerProc_H

extern MobilityConfigData	mmCfg;

// *******************************************************************
// constant declarations
// *******************************************************************

const int   mmPAGE_REQUEST   = 1;
const int   mmAUTH_REQUEST   = 2;
const int   mmMAX_PG_REQUEST = 2;
const int   mmIDENT_REQUEST  = 3;
const int   mmPAGE_CC_REQUEST = 4;

// *******************************************************************
// function prototypes
// *******************************************************************

void MM_TIMER_MsgProc(IntraL3Msg_t *mmInMsg);

#endif                                 // MMTimerProc_H
