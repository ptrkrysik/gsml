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
// File        : RlcMacConfig.h
// Author(s)   : Tim Olson
// Create Date : 10/31/00
// Description : 
//
// *******************************************************************

#ifndef __RLCMACCONFIG_H__
#define __RLCMACCONFIG_H__

#include "bssgp/bssgp_api.h"

// Maximum message size passed to the RlcMacTask
const int RLCMAC_MAX_MSG_LENGTH        = sizeof(BSSGP_API_MSG);

// Maximum number of messages to the RlcMacTask.
const int RLCMAC_MAX_MSGS              = 100;

#define RLCMAC_TASK_NAME  "RlcMacTask"



#endif