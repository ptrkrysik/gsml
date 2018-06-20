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
// File        : JCCTimer.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 10-19-98
// Description : interface specification for -  JCCTimer
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include <stdio.h>
#include "Os/JCCTimer.h"

extern "C" int	sysClkRateGet (void);

#define JCCLog printf

int JCCTimer::tickPerSec = sysClkRateGet();


void JCCTimer::cancelTimer()
{
  if (timerSet_ == true)
    {
      wdCancel(timerId_);
      timerSet_ = false;
    }
}

JCCTimer::~JCCTimer()
{
  wdDelete(timerId_);
}

JCCTimer::JCCTimer (FUNCPTR fPtr     ,
                    int     timerData)
  :timerData_(timerData),
   funcPtr_  (fPtr)     ,
   timerSet_ (false)
{
    init ();
}


JCCTimer::JCCTimer (FUNCPTR fPtr)
  :timerData_(-1)   ,
   funcPtr_  (fPtr) ,
   timerSet_ (false)
{
    init ();
}


void JCCTimer::setTimer(int howLong)
{
  if (timerSet_ == true)
  {
      cancelTimer();
  }

  wdStart(timerId_, howLong, funcPtr_, timerData_);

  timerSet_ = true;
}


void JCCTimer::init()
{
  if (!tickPerSec)
      tickPerSec = sysClkRateGet ();

  if ((timerId_ = wdCreate()) == NULL)
    {
      // Handle Operating System Resource Allocation Error
      JCCLog("JCC Error - Operating System Resource Allocation Error:\n");
      JCCLog("          - WatchDog Timer could not be created.\n");
    }
}

