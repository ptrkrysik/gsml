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
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "vxWorks.h"
#include "wdLib.h"


// *******************************************************************
// forward declarations.
// *******************************************************************

class JCCTimer
{

public:

  // Destructor
  ~JCCTimer() ;

  // Constructors
  JCCTimer(FUNCPTR                    fPtr,
	   int                        timerData);  

  JCCTimer(FUNCPTR                    fPtr);

  // Operators

  // primary behaviour methods
  void cancelTimer();
  void setTimer(int howLong);
  void setTimer(int howLong, int timerData);

  // maintenance methods
  
  // get and set for private data members 

  // boolean to indicate if timer is active
  bool                              timerSet_;  
protected:

private:

  // hide the assignment, and copy ctor and other defaults as needed.
  JCCTimer();  

  JCCTimer (const JCCTimer& rhs) ;
  
  JCCTimer&  operator= (const JCCTimer& rhs) ;

  int               operator==(const JCCTimer& rhs) const ;

private:
  
  // data members
  FUNCPTR                    funcPtr_;
  int                        timerData_; // data for timer call-back
  WDOG_ID                    timerId_;   // VxWorks Watchdog Timer Id
};


#endif                        // JCCTimer_H
