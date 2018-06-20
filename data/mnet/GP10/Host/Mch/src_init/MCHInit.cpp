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
// File        : MCHInit.cpp
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description : 
//
// *******************************************************************
#include <msgQLib.h>

#include "MCHTask.h"

// Global variable for Message Queue
MSG_Q_ID MCHMsgQId;
MCHTask *MCHTask::theMCHTask = 0;
