#ifndef JCC_ED_H
#define JCC_ED_H

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
// File        : JCC_ED.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : message and ie interface among Application (Layer 3) 
//               modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "ril3/ril3_cc_msg.h"
#include "ril3/ril3_sms_cp.h"
#include "ril3/ril3_mm_msg.h"
#include "ril3/ril3_rrm_msg.h"
#include "ril3/ril3_common.h"
#include "ril3/ril3_result.h"

#include "lapdm/lapdm_l3intf.h"

extern void CNI_RIL3_ZAPMSG(void *, int length);

extern T_CNI_RIL3_RESULT 
JCC_RIL3CC_Decode(
        bool isSoftMob,
	T_CNI_LAPDM_L3MessageUnit *, T_CNI_RIL3CC_MSG *
);

extern T_CNI_RIL3_RESULT 
JCC_RIL3SMS_Decode (bool isSoftMob, T_CNI_LAPDM_L3MessageUnit *l3MsgUnit, 
									T_CNI_RIL3SMS_CP_MSG      *smsMsg   );

extern T_CNI_RIL3_RESULT 
JCC_RIL3MM_Decode(
        bool isSoftMob,
	T_CNI_LAPDM_L3MessageUnit *, T_CNI_RIL3MM_MSG *
);

#endif                                       // JCC_ED_H
