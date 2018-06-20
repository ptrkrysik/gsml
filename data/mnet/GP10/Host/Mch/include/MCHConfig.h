#ifndef MCHconfig_H
#define MCHconfig_H

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
// File        : MCHconfig.h
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description : 
//
// *******************************************************************

#include "MCH/MCHIntf.h"

// Maximum message size passed to the Maintenance Command Handler
const int MCH_MAX_MSG_LENGTH        = sizeof(MCHMessageType);

// Maximum number of messages to the Maintenance Command Handler.
const int MCH_MAX_MSGS              = 32;

#define MCH_TASK_NAME  "MCHTask"



#endif MCHconfig_H