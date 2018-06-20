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
// File        : MMPmProc.h
// Author(s)   : Kevin Lim
// Create Date : 07-27-99
// Description : 
//
// *******************************************************************

#ifndef MMPmProc_H
#define MMPmProc_H

// *******************************************************************
// function prototypes
// *******************************************************************

void mmIncAttTrnPaging();
void mmIncUnsuccTrnPaging();
void mmUpdMeanSuccPaging(int val);
void mmIncAttMOCalls();
void mmIncAttMOEmergencyCalls();
void mmIncAttCipher();
void mmIncSuccCipher();
void mmUpdMeanLocUpTime(int val);
void mmIncTrnSubIdIMSI();
void mmIncIMSIDetach();
void mmIncIMSIAttach();

#endif                                 // MMPmProc_H
