#ifndef JCCUtil_H
#define JCCUtil_H

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
// File        : JCCUtil.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included MD and IRT headers for messages from MS
#include "ril3/ril3md.h"
#include "ril3/ril3msgheader.h"
#include "ril3/ie_mobile_id.h"
#include "ril3/ie_called_bcd.h"
#include "ril3/ie_location_area_id.h"

// *******************************************************************
// forward declarations.
// *******************************************************************

// *******************************************************************
// Constant Definitions
// *******************************************************************

//typedefs

// Utility functions

int
operator==(const T_CNI_L3_ID& lhs,  const T_CNI_L3_ID& rhs);

int
operator==(const T_CNI_RIL3_IE_MOBILE_ID& lhs,  
	   const T_CNI_RIL3_IE_MOBILE_ID& rhs);

void printMobileId(const T_CNI_RIL3_IE_MOBILE_ID&);

T_CNI_RIL3_PROTOCOL_DISCRIMINATOR extractPd (const T_CNI_LAPDM_L3MessageUnit *);

#endif                                       // JCCUtil_H
