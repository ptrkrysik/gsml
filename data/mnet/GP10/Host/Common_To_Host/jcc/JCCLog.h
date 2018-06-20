#ifndef JCCLog_H
#define JCCLog_H

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
// File        : JCCLog.h
// Author(s)   : Bhava Nelakanti
// Create Date : 11-01-98
// Description : message and ie interface among Application (Layer 3) 
//               modules. 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "stdio.h"

//
// The following log macro is defined in such a way to
// allow printf-type capability when the _JCC_DEBUG flag is set
// and no run-time panelty when _JCC_DEBUG flag is clear.
//
// Without the ::_JCCLog line, the compiler will
// complain about the parameter (...). With this line
// and _JCC_DEBUG set to false, the compiler will
// optimize the code (1?(void)0: ::_JCCLog) out completely
// without any run time penalty.
//

typedef int (* JCCPrintStFn) (const char *);

inline int JCCPrintf(const char *str) { return (printf(str)); }

inline int _JCCLog(const char *) { return (0); }
inline int _JCCLog1(const char *, int ) { return (0); }
inline int _JCCLog2(const char *, int, int) { return (0); }
inline int _JCCLog3(const char *, int , int , int ) { return (0); }

int __JCCLog(const char *logText);
int __JCCLog1(const char *logText, int arg1);
int __JCCLog2(const char *logText, int arg1, int arg2);
int __JCCLog3(const char *logText, int arg1, int arg2, int arg3);

#ifdef _JCC_DEBUG
#define JCCLog	__JCCLog
#else
#define JCCLog	1 ? (void)0: ::_JCCLog
#endif

#ifdef _JCC_DEBUG
#define JCCLog1	__JCCLog1
#else
#define JCCLog1	1 ? (void)0: ::_JCCLog1
#endif

#ifdef _JCC_DEBUG
#define JCCLog2	__JCCLog2
#else
#define JCCLog2	1 ? (void)0: ::_JCCLog2
#endif

#ifdef _JCC_DEBUG
#define JCCLog3	__JCCLog3
#else
#define JCCLog3	1 ? (void)0: ::_JCCLog3
#endif

#endif                                       // JCCLog_H
