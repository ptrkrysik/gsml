#ifndef __APMDEFS_H__
#define __APMDEFS_H__

// *******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : apmdefs.h
// Author(s)   : MindTree
// Create Date : 3/12/00
// Description :  Common definitions to be used by APM
//
// *******************************************************************

// this port numbers are used by PM and Alarm Module resp.
#define PM_SERVER_PORT      11789
#define AM_SERVER_PORT      11788

// this new port numbers are used betw, APM1, APM2, APM2Gui ...

//March 29 fix
//#define APM2_SERVER_PORT    11795
#define APM3_SERVER_PORT    11796
#define APM_GUI_PORT        11797
#define AM_APPLET_PORT      11798



// TODO: this enumeration clashes with another definition
// needs to be cleaned up by the next build
typedef enum RET_STATUS{
    STATUS_OK = 0    ,
    STATUS_ERROR = -1
    };

//Used for Windows Services.
#if defined(_WINDOWS_) || defined(WIN32)
#define kMaxEventMsgLen 1024
#define APP_APPROX_MAX_INIT_TIME  2000 /* milliseconds */
typedef RET_STATUS STATUS;
#endif


#endif /* __APMDEFS_H__ */
