#ifndef SMSCOMMONDEF_H
#define SMSCOMMONDEF_H

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
// File        : SmsCommonDef.h
// Author(s)   : Igal Gutkin
// Create Date : 24-01-00
// Description : SMS specific definitions
//
// *******************************************************************


#define NULL_PTR ((void *)0)

//Leg number allocated for the MO SMS
#define SMS_MO_LEG_NUM 5

// TI values allocated for the MT SMS 
#define SMS_TI_1        4
#define SMS_TI_2        5
#define SMS_TI_3        6
#define SMS_TI_4        7
#define SMS_TI_INVALID    255

#define TI_ORIG_OR_MASK  ((UINT8)0x08)
#define TI_TERM_AND_MASK ((UINT8)0x07)

//  List of the CM common events:

#define SMS_EVENT_MM_EST_CNF    CC_MM_CONNECTION_CNF
#define SMS_EVENT_MM_EST_REJ    CC_MM_CONNECTION_REJ
#define SMS_EVENT_MM_REL_IND    CC_MM_CONNECTION_REL
#define SMS_EVENT_MM_EST_IND    CC_MM_CONNECTION_IND

#define SMS_LEGNUM_OFFSET       3

#endif // SMSCOMMONDEF_H

