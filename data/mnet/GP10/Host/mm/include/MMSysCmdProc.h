#ifndef MMSysCmdProc_H
#define MMSysCmdProc_H

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
// File        : MMSysCmdProc.h
// Author(s)   : Kevin Lim
// Create Date : 07-26-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// function prototypes
// *******************************************************************


int	SysCommand_MM(T_SYS_CMD	action);

MSG_Q_ID createMMMsgQ();



#endif                                 // MMSysCmdProc_H
