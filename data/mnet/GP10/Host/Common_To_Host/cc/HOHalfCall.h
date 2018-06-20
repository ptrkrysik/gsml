#ifndef HOHALFCALL_H
#define HOHALFCALL_H


// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : HOHalfCall.h
// Author(s)   : Xiaode and Joe
// Create Date : 05-15-01
// Description : Definition of HOHalfCall class and the derived 
//               classes HOMoHalfCall, HOMtHalfCall
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "ril3/ril3md.h"
#include "ril3/ril3_cc_msg.h"

#include "jcc/JCCComMsg.h"
#include "jcc/Jcctimer.h"


// *******************************************************************
// Forward declarations
// *******************************************************************
class HOCallLeg;


// *******************************************************************
// Constant definitions.
// *******************************************************************

const int MAX_EHO_CALL_STATES 
= (EHO_ST_ACTIVE + 1);

const int MAX_EHO_CALL_EXTERNAL_EVENTS    
= (CC_EHO_LAST_EVENT - CC_INTERNAL_EHO_CALL_EVENT_BASE + 1);


/*******************************************************************/
class HOHalfCall
{

 public:

  // Following covered in HOHalfCall.cpp

  HOHalfCall ();

  virtual void                cleanup(void);
  void                        printData(JCCPrintStFn);

  JCCEvent_t                  handleInvalidEvent(void);


  int                         handleTimeoutMsg(IntraL3Msg_t *ccInMsg);
  int                         handleRemoteEvent(IntraL3Msg_t* h323InMsg, JCCEvent_t remoteEvent,
                                                CCRemMsgData_t   inRemMsg);

  int                         smHandleEvent(void);

  int                         smHandleFirstCallEvent(void);

  virtual bool                sendRemote(JCCEvent_t remoteEvent);

  //ext-HO <xxu:06-07-01>
  JCCEvent_t				  fakeEHOMobCallProgress(void);
  JCCEvent_t				  handleHoCallTimeout(void);
  JCCEvent_t				  handleEHOMobRemoteTermAddress(void);
  JCCEvent_t				  handleEHOMobRemoteChannSelectedEi(void);
  JCCEvent_t                  fakeEHOMobConnect(void);
  JCCEvent_t				  handleEHOMobRemoteChannReady(void);

 public:

  JCCEvent_t                  handleEHOMobTermAddress(void);
  JCCEvent_t                  handleEHOMobRemoteChannSelected(void);
  JCCEvent_t                  handleEHOMobRemoteAnswer(void);
  JCCEvent_t                  handleEHOMobRemoteDisconnect(void);
  JCCEvent_t                  handleEHOMobCallProgress(void);
  JCCEvent_t                  handleEHOMobLocalDisconnect(void);
  JCCEvent_t                  handleNullEvent(void);

 private:

 protected:

  virtual void                initData(void);

  T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER
    cpn;
  T_CNI_RIL3_IE_CALLING_PARTY_BCD_NUMBER  
    callingPartyNum;

  short                       releaseCounter;
  
  // For building data for remote events
  CCRemMsgData_t              remMsgData;

  IntraL3Msg_t                * h323InMsg;

 public:

  //CAUSE<xxu:03-21-00> moved here from private
  T_CNI_RIL3_CAUSE_VALUE    voipReleaseCause_;

  //CH<xxu:11-10-99>
  ChStates_t                  chState_;
  T_CNI_RIL3_CAUSE_VALUE      chCause_;

  JCCTimer                    *hcTimer;  //CH<xxu:12-06-99>

  HalfCallStates_t            callState_ ;

  ResStates_t                 channState_;
  
  SpeechChann_t               currSpeechCh_;

  T_CNI_RIL3_IE_CAUSE         rlsCause;

  DbgOutput                   Dbg;

 protected:

  HOCallLeg                   *parent;

  JCCEvent_t                  currEvent;

 private:


  T_CNI_RIL3_IE_BEARER_CAPABILITY 
    bearerCap;


  T_CNI_RIL3_IE_KEYPAD_FACILITY 
    currKeypadFacility_;

 private:

  // hide the assignment, and copy ctor and other defaults as needed.

  HOHalfCall (const HOHalfCall& rhs) ;
  
  HOHalfCall&  operator= (const HOHalfCall& rhs) ;

  int               operator==(const HOHalfCall& rhs) const ;

};

/*******************************************************************/
class HOMoHalfCall : public HOHalfCall
{

public:

    // Constructors
    HOMoHalfCall (HOCallLeg *HOLeg, JCCTimer *tclTimer);

    // Destructor
    ~HOMoHalfCall () {};

    // Operators

    void  setupNewCall(void);
    void  disconnectNewCall(void);

    void  initData(void);

    // maintenance methods

    // Even handler methods

    // get and set for private data members 

//protected:
    
private:
    // hide the assignment, and copy ctor and other defaults as needed

    HOMoHalfCall();

};



/*******************************************************************/
class HOMtHalfCall : public HOHalfCall
{

public:

    // Constructors
    HOMtHalfCall (HOCallLeg  *callLeg, JCCTimer *tclTimer);

    // Destructor
    ~HOMtHalfCall () {};

    // Operators
    void  initData(void);

    // maintenance methods

    // Even handler methods

    // get and set for private data members 

//protected:
    
private:
    // hide the assignment, and copy ctor and other defaults as needed

    HOMtHalfCall();

};


// *******************************************************************
// State machine storage
// *******************************************************************


typedef JCCEvent_t (HOHalfCall::* HOHalfCallMsgHandler_t) (); 
extern HOHalfCallMsgHandler_t ehoCallStateHandler[MAX_EHO_CALL_STATES][MAX_EHO_CALL_EXTERNAL_EVENTS];

#endif // HOHALFCALL_H
