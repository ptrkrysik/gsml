#ifndef __CDRDBG_H__
#define __CDRDBG_H__

// *******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : CdrDbg.h
// Author(s)   : Igal Gutkin
// Create Date : 8/22/00
// Description :  
//
// *******************************************************************


#include "dbgfunc.h"


/* Printing and\or sending to the logging module */

#define AM_RETURN(x)        {AM_LEAVE();return(x);}


// Disable all the logging output
#if (defined (__DISABLE_OUTPUT))

 #define LAYER_ID               0
 #define AM_ERROR(XX)
 #define AM_WARNING(XX)
 #define AM_HEXDUMP(x,y)
 #define AM_TRACE(XX)
 #define AM_DEBUG(XX)
 #define AM_FUNC(x,y)
 #define AM_ENTER()   
 #define AM_LEAVE()

// GP10, GMC and GS application on VxWorks standard logging
#elif (!defined(__STDIOUT) && defined(__VXWORKS__) && (defined(MNET_GP10) || defined(MNET_GMC) || defined(MNET_GS) ))

 #ifdef _MNET_COMMON_LOGGING_MODULE_
  #define AM_HEXDUMP(x,y)    DBG_HEXDUMP_FMT(x,y)
 #else
  #define AM_HEXDUMP(x,y)    DBG_HEXDUMP(x,y)
 #endif // _MNET_COMMON_LOGGING_MODULE_

 #define AM_ERROR(XX)       DBG_ERROR   XX
 #define AM_WARNING(XX)     DBG_WARNING XX
 #define AM_TRACE(XX)       DBG_TRACE   XX
 #define AM_DEBUG(XX)       DBG_TRACE   XX

 #define AM_ENTER           DBG_ENTER
 #define AM_LEAVE           DBG_LEAVE
 #define AM_FUNC(x,y)       DBG_FUNC (x,y)

// Windows application using event logging
#elif !(defined (_WIN_STDIO) || defined (__STDIOUT))

// Support for the GMC Windows application debug output
#if (defined(_WINDOWS_) || defined(WIN32)) && !(defined(_WIN_STDIO) || defined(__STDIOUT))
#include "support.h" // fetch GMC on Windows debug interface definitions 
#endif // _WIN_STDIO

 #define AM_HEXDUMP(x,y)    
 #define AM_ERROR(XX)       DebugLogCritical XX

 #ifdef _AM_WARNING_
  #define AM_WARNING(XX)    DebugLogWarning XX
 #else
  #define AM_WARNING(XX)
 #endif

 #ifdef _AM_TRACE_
  #define AM_TRACE(XX)      DebugLogInfo XX
 #else
  #define AM_TRACE(XX)
 #endif

 #ifdef _AM_DEBUG_
  #define AM_DEBUG(XX)      DebugLog XX
  #define AM_FUNC(x,y)      DebugLog("Entering function: %s\n",x)
 #else
  #define AM_DEBUG(XX)
  #define AM_FUNC(x,y)
 #endif

 #define LAYER_ID           0
 #define AM_ENTER()   
 #define AM_LEAVE()

//Unspecified system. Use own debug system
#else 

 #define AM_ERROR(XX)       {printf("ERROR: ");dbg_print XX;putchar('\n');}

 #ifdef _AM_WARNING_
  #define AM_WARNING(XX)    {printf("WARNING: ");dbg_print XX;putchar('\n');}
 #else
  #define AM_WARNING(XX)
 #endif

 #ifdef _AM_TRACE_
  #define AM_TRACE(XX)      {printf("TRACE: ");dbg_print XX;putchar('\n');}
  #define AM_HEXDUMP(x,y)   HexDump (x,y)
 #else
  #define AM_TRACE(XX)
  #define AM_HEXDUMP(x,y)
 #endif

 #ifdef _AM_DEBUG_
  #define AM_DEBUG(XX)      {printf("DEBUG: ");dbg_print XX;putchar('\n');}
  #define AM_FUNC(x,y)      printf("Entering function: %s\n",x)
 #else
  #define AM_DEBUG(XX)
  #define AM_FUNC(x,y)
 #endif

 #define LAYER_ID           0
 #define AM_ENTER()   
 #define AM_LEAVE()

#endif


#endif //__CDRDBG_H__