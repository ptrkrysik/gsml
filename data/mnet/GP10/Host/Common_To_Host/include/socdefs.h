#ifndef __SOCDEFS_H__
#define __SOCDEFS_H__
//*******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : SocDefs.h
// Author(s)   : Igal Gutkin
// Create Date : 8/22/00
// Description : Application (GP10) dependant settings
//
// *******************************************************************

// CDR Module ID specific for GP10 application
#include "logging/VCLOGGING.h"


// Debug interface definitions

// Use stdiout for debug output
//#define __STDIOUT 

#ifdef __STDIOUT 
 #define _AM_TRACE_
 #define _AM_DEBUG_
 #define _AM_WARNING_
#endif //__STDIOUT

// specific value for GMC
#define SOC_LAYER_ID    SOCKET_LAYER

// use RSA SSL library
#define _RSA_SSL_LIB_


#endif //__SOCDEFS_H__