// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacResult.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __RLCMACRESULT_H__
#define __RLCMACRESULT_H__

#include "logging\vclogging.h"

#define FALSE     0
#define TRUE      1

typedef enum {
   RLC_MAC_SUCCESS,
   RLC_MAC_ERROR,
   RLC_MAC_INVALID_NUM_BITS,
   RLC_MAC_PARAMETER_RANGE_ERROR,
   RLC_MAC_MISSING_IE,
   RLC_MAC_INVALID_IE
} RlcMacResult;

#define RLC_MAC_RESULT_CHECK(result) \
    if(result != RLC_MAC_SUCCESS)   \
    {   \
        DBG_LEAVE();\
        return (result);\
    }
    
#endif