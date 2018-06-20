#ifndef JCCTimer_H
#define JCCTimer_H

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
// File        : JCCTimer.h
// Author(s)   : Bhava Nelakanti
// Create Date : 10-19-98
// Description : interface specification for -  JCCTimer
//
//  REVISION HISTORY:
// __________________________________________________________________
//| Name     |  Date  |  Reason
//+__________+________+_______________________________________________
//| Igal     |11/29/00| Added support to set timer in 100 mSec resolution
//| Shiv     |01/14/00| Added support to enable windows applications to 
//|          |        | use the same APIs provided by JCCTimer
//+__________+________+______________________________________________
//
// *******************************************************************
// Include Files.
// *******************************************************************

#if defined(__VXWORKS__)
    #include "vxWorks.h"
    #include "wdLib.h"
#elif (defined(WIN32) || defined(_WINDOWS_))
    //#include "stdAfx.h"
    #include <mmsystem.h>
    #define FUNCPTR LPTIMECALLBACK
    #define WDOG_ID MMRESULT
    #define TARGET_RESOLUTION 1  // 1-millisecond target resolution
#endif

// *******************************************************************
// forward declarations.
// *******************************************************************

class JCCTimer
{

public:

  // Destructor
  ~JCCTimer() ;

  // Constructors
  JCCTimer (FUNCPTR fPtr,
            int     timerData);  

  JCCTimer (FUNCPTR fPtr);

  // Operators

  // primary behaviour methods
  void cancelTimer ();
  void setTimer    (int howLong);
  void setTimer    (int howLong, int timerData)
      { timerData_ = timerData;
        setTimer (howLong);    }

  // set timer in resolution of 100 mSec
  void setTimerMSec(unsigned mSec) 
        { setTimer (tickPerSec*mSec/10);}

  // set timer in resolution of 100 mSec
  void setTimerMSec(unsigned mSec, int timerData)
        { setTimer ((tickPerSec*mSec/10), timerData);}

  static int getTickRate () { return (tickPerSec); }

  // maintenance methods
  
  // get and set for private data members 

  // boolean to indicate if timer is active
  bool  timerSet_;

protected:

private:

  // hide the assignment, and copy ctor and other defaults as needed.
  JCCTimer();  

  JCCTimer   (const JCCTimer& rhs) ;
  
  JCCTimer&  operator= (const JCCTimer& rhs) ;

  int        operator==(const JCCTimer& rhs) const ;

  void       init ();

private:

  static int                 tickPerSec;

  // data members
  FUNCPTR                    funcPtr_  ;
  int                        timerData_; // data for timer call-back
  WDOG_ID                    timerId_  ; // VxWorks Watchdog Timer Id
#if (defined(WIN32) || defined(_WINDOWS_))
  unsigned int     wTimerRes;
#endif
};


#endif                        // JCCTimer_H
