// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacCommon.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __RLCMACCOMMON_H__
#define __RLCMACCOMMON_H__

#include "oam_api.h"
#include "JCErr.h"

#define RLC_MAC_MAX_RF_FREQS        64
#define MAX_TFI   				    32
#define MAX_NUM_RADIO_PRIORITY      4
#define MAX_ALPHA_VALUE		        16
#define MAX_TIMESLOTS               8
#define MAX_TRX                     2
#define MAX_USF_PER_TS              8
#define MAX_TAI_PER_TS              16
#define MAX_DATA_RTS_COUNT          1
#define MAX_SIGNALLING_RTS_COUNT    2
#define RLC_MOD_VAL                 128
#define MAX_RTI_VAL                 32
#define MAX_RLC_PDU_SIZE            1560
#define CTL_ACK_TAG_NOT_APPLICABLE  0xff
#define RLC_MAC_NORMAL_BURST        0
#define RLC_MAC_ACCESS_BURST        1

#define MAX_ALLOWED_INACTIVE_TIME	100	// in 100's of ms


typedef enum
{
    MS_ID_TLLI,
    MS_ID_GLOBAL_TFI
} MS_ID_TYPE;


// Common DSP and Layer one settings
#define RELATIVE_START_TIME_VAL     1

typedef enum { DL, UL } LinkDirection;

inline int GetMibValueInt(MibTag tag)
{
    long val;
    oam_getMibIntVar(tag, &val);
    return ((int)val);
}


#endif
