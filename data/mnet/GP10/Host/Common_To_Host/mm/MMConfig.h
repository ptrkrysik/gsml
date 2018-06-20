#ifndef MMConfig_H
#define MMConfig_H

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
// File        : MMConfig.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : message and ie interface among Application (Layer 3) 
//               modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************

// Maximum number of MM entry
#define MM_MAXNUM_ENTRY		100          // 20  08/27/01 shmin : load problem

//	defines the size of VxWorks Message queue
//      The value of this parameter is system dependent
//
const int MM_MAX_L3_MSGS           = 32;

// defines the priority of the VxWorks message queue
// 
const int MM_L3_MSG_Q_PRIORITY	  = 10;

// Timers 
//
// The timer value is based on VxWorks "tick" which is BSP (Board Support
// Package) dependent. A translation between real-world time unit (seconds
// or millisecond) and VxWorks ticks muct be done first to set the proper
// timer values.
//

// Default value if it fails to get values from OAM MIB data
const int MM_DEFAULT_MAX_MM_CONNECTION				= 99; // 19  08/28/01 shmin : load problem
const int MM_DEFAULT_PAGING_T3113					= 600;
const Boolean_t MM_DEFAULT_AUTHENTICATION_REQUIRED	= bfalse;
const Boolean_t MM_DEFAULT_CIPHERING_REQUIRED		= bfalse;
const Boolean_t MM_DEFAULT_IMEI_CHECK_REQUIRED		= bfalse;

const char MM_DEFAULT_BTS_MCC[3] 	= {0, 0, 1};
const char MM_DEFAULT_BTS_MNC[2] 	= {0, 1};
const short MM_DEFAULT_BTS_LAC		= 2;

void mmCfgSetMaxMMConnections(INT_32_T val);
void mmCfgSetPagingT3113(INT_32_T val);
void mmCfgSetAuthenticationRequired(INT_32_T val);
void mmCfgSetCipheringRequired(INT_32_T val);
void mmCfgSetIMEICheckRequired(INT_32_T val);
void mmCfgSetIMSIRequired(INT_32_T val);
void mmConvertMCC(INT_32_T oam_mcc);
void mmConvertMNC(INT_32_T oam_mnc);
void mmConvertLAC(INT_32_T oam_lac);
void mmCfgRead();

#endif                                 // MMConfig_H
