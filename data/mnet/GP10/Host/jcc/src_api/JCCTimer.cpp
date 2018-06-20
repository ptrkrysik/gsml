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

#include "JCC/JCCTimer.h"

#include "JCC/JCCLog.h"

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

JCCTimer::JCCTimer(  FUNCPTR                    fPtr,
		     int                        timerData )
  :timerData_(timerData),
   funcPtr_(fPtr),
   timerSet_(false)
{

  if ((timerId_ = wdCreate()) == NULL)
    {
      // Handle Operating System Resource Allocation Error
      JCCLog("JCC Error - Operating System Resource Allocation Error:\n");
      JCCLog("          - WatchDog Timer could not be created.\n");
    }
}

JCCTimer::JCCTimer(  FUNCPTR                    fPtr)
  :timerData_(-1),
   funcPtr_(fPtr),
   timerSet_(false)
{

  if ((timerId_ = wdCreate()) == NULL)
    {
      // Handle Operating System Resource Allocation Error
      JCCLog("JCC Error - Operating System Resource Allocation Error:\n");
      JCCLog("          - WatchDog Timer could not be created.\n");
    }
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

void JCCTimer::setTimer(int howLong, int timerData) 
{
  if (timerSet_ == true)
    {
      cancelTimer();
    }

  timerData_ = timerData;
  wdStart(timerId_, howLong, funcPtr_, timerData_);
  timerSet_ = true;

}

		
