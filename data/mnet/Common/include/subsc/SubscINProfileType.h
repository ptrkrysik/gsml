
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
// File        : SubscINProfileType.h
// Author(s)   : Ji-Hoon Yoon
// Create Date : 09-25-01
// Description : message and ie interface among GUDB and other modules
//
// *******************************************************************

#ifndef SUBSC_IN_PROFILE_TYPE_H
#define SUBSC_IN_PROFILE_TYPE_H

#include "ril3/ril3_sm_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	unsigned char						classId;
} T_SUBSC_IN_PROFILE_CLASS_INFO;

typedef struct {
	bool								isPrepaid;
} T_SUBSC_IN_PROFILE_PREPAID_INDICATOR;

typedef struct {
	bool isPresent;
	T_SUBSC_IN_PROFILE_CLASS_INFO					classInfo;
	T_SUBSC_IN_PROFILE_PREPAID_INDICATOR			prepaidIndicator;
} T_SUBSC_IE_IN_PROFILE; // 2.1.2

#ifdef __cplusplus
}
#endif

#endif // SUBSC_IN_PROFILE_TYPE_H