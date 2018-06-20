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
// File        : JCCLog.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 10-19-98
// Description : implementation for -  JCCLog
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "JCC/JCCLog.h"
#include "vxWorks.h"
#include "logLib.h"
#include "time.h"
#include "string.h"

int __JCCLog(const char *logText)
{
  struct tm currTime;

  time_t timeInSecs = time(NULL); 
  
  char outText[161];

  // Initialize to an empty string
  outText[0] = '\0';

  strcat(outText, "T::%d:%d:%d:  ");

  strcat(outText, logText);

  localtime_r(&timeInSecs, &currTime);

  logMsg(outText, currTime.tm_hour, currTime.tm_min, currTime.tm_sec, 0, 0, 0);
}

int __JCCLog1(const char *logText, int arg1)
{
  struct tm currTime;

  time_t timeInSecs = time(NULL); 
  
  char outText[161];

  // Initialize to an empty string
  outText[0] = '\0';

  strcat(outText, "T::%d:%d:%d:  ");
  
  strcat(outText, logText);

  localtime_r(&timeInSecs, &currTime);

  logMsg(outText, currTime.tm_hour, currTime.tm_min, currTime.tm_sec, arg1, 0, 0);
}

int __JCCLog2(const char *logText, int arg1, int arg2)
{
  struct tm currTime;

  time_t timeInSecs = time(NULL); 
  
  char outText[161];

  // Initialize to an empty string
  outText[0] = '\0';

  strcat(outText, "T::%d:%d:%d:  ");
  
  strcat(outText, logText);

  localtime_r(&timeInSecs, &currTime);

  logMsg(outText, currTime.tm_hour, currTime.tm_min, currTime.tm_sec, arg1, arg2, 0);
}

int __JCCLog3(const char *logText, int arg1, int arg2, int arg3)
{
  struct tm currTime;

  time_t timeInSecs = time(NULL); 
  
  char outText[161];

  // Initialize to an empty string
  outText[0] = '\0';

  strcat(outText, "T::%d:%d:%d:  ");
  
  strcat(outText, logText);

  localtime_r(&timeInSecs, &currTime);

  logMsg(outText, currTime.tm_hour, currTime.tm_min, currTime.tm_sec, arg1, arg2, arg3);
}
