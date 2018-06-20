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
// File        : RootTask.h
// Author(s)   : Tim Olson
// Create Date : 9/18/2000
// Description : 
//
// *******************************************************************
#ifndef _ROOTTASK_H_
#define _ROOTTASK_H_  /* include once only */

#include "JCErr.h"
#include "AlarmCode.h"


class RootTask {
public:

    ~RootTask();

    static int InitRootTask();
    static void RootRebootHook();
    static int RootMain(void);
    
    void InitializeSystemModuleList();
    JC_STATUS SetSelfRebootMode(bool mode);
    void EngageSelfReboot(int i);
    void UpdateRadioBoardEeprom();
    void WaitForModuleStatusCheck();
    short CheckForMCHTest();
    void SendAlarm(Alarm_errorCode_t err_code, int arg1, int arg2);

    static RootTask *theRootTask;

private:
    static bool isOneCreated;
    
    // Disallow the following functions
    RootTask();
    RootTask(const RootTask&);
    RootTask& operator=(const RootTask&);
    int operator== (const RootTask&) const;
};


#endif
