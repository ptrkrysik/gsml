#ifndef __CDRDEFS_H__
#define __CDRDEFS_H__

// *******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : CdrDefs.h
// Author(s)   : Igal Gutkin
// Create Date : 8/22/00
// Description : Application (GP10) dependant settings
//
// *******************************************************************

#include "GP10OsTune.h"

// CDR Module ID specific for GP10 application
#define CDR_MODULE_ID MODULE_CDR

// CDR Client alarms
#define CDR_CLIENT_ALARM_ERRCODE MIB_errorCode_CDR

// GP10 uses a local version of the logger
#include "logging/VCLOGGING.h"


// Debug interface definitions

// Use stdiout as a debug output
//#define __STDIOUT 

#ifdef __STDIOUT
 #define _AM_TRACE_
 #define _AM_DEBUG_
 #define _AM_WARNING_
#endif // __STDIOUT


// specific value for GP10
#define LAYER_ID    CDR_LAYER


#endif //__CDRDEFS_H__