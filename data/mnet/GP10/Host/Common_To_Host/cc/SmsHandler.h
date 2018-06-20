#ifndef SmsHandler_H
#define SmsHandler_H

// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************


// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : SmsLeg.h
// Author(s)   : Igal Gutkin
// Create Date : 01-19-00
// Description : class specification for -  SmsLeg
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "jcc\jccevent.h"
#include "jcc\jccstate.h"

#include "CC/SmsCommonDef.h"

class SmsHalfCall;


typedef bool (*fsmHandle_t)(SmsHalfCall *, void *, INT32);

#define SFH     fsmHandle_t

// Event Entry
typedef struct 
{
    JCCEvent_t  event    ;
    fsmHandle_t handler  ;
    SmsState_t  nextState;
} smsEventTableEntry_t;

// State entry
typedef smsEventTableEntry_t  smsStateTableEntry_t[];

// FSM Table (array of pointers)
typedef smsStateTableEntry_t  *smsFsmTable_t      [SMS_STATE_MAX];


#endif // SmsHandler_H
