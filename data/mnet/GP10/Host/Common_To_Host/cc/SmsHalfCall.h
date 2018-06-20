#ifndef SMSHALFCALL_H
#define SMSHALFCALL_H


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
// File        : SmsHalfCall.h
// Author(s)   : Igal Gutkin
// Create Date : 01-19-00
// Description : class specification for SmsHalfCall and the derived 
//                 classes SmsMoHalfCall, SmsMtHalfCall
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "ril3/ril3md.h"
#include "ril3/ril3_cc_msg.h"
#include "ril3/ril3_sms_cp.h"

#include "jcc/JCCComMsg.h"
#include "jcc/Jcctimer.h"

#include "CC/SmsHandler.h"

// *******************************************************************
// Forward declarations
// *******************************************************************
class SmsLeg;

/*******************************************************************/
class SmsHalfCall
{

    friend bool smsHandlerDef       (SmsHalfCall *, void *                     , INT32           );

    friend bool smsHandlerMdErrInd  (SmsHalfCall *, void *                     , INT32           );

    friend bool smsHandlerMmEstInd  (SmsHalfCall *, void *                     , T_CNI_RIL3_SI_TI);
    friend bool smsHandlerMmEstCnf  (SmsHalfCall *, void *                     , INT32           );
    friend bool smsHandlerMmEstRej  (SmsHalfCall *, void *                     , INT32           );
    friend bool smsHandlerMmRelInd  (SmsHalfCall *, void *                     , INT32           );

    friend bool smsHandlerCpData    (SmsHalfCall *, T_CNI_RIL3SMS_MSG_CP_DATA *, INT32           );
    friend bool smsHandlerCpAck     (SmsHalfCall *, T_CNI_RIL3SMS_MSG_CP_ACK  *, INT32           );
    friend bool smsHandlerCpError   (SmsHalfCall *, T_CNI_RIL3SMS_MSG_CP_ERROR*, INT32           );
    friend bool smsHandlerCpReject  (SmsHalfCall *, T_CNI_RIL3SMS_CP_MSG      *, INT32           );

    friend bool smsHandlerMnEstReq  (SmsHalfCall *, SmsVblinkMsg_t            *, T_CNI_RIL3_CP_CAUSE_VALUE);
    friend bool smsHandlerMnDataReq (SmsHalfCall *, SmsVblinkMsg_t            *, T_CNI_RIL3_CP_CAUSE_VALUE);
    friend bool smsHandlerMnAbortReq(SmsHalfCall *, SmsVblinkMsg_t            *, T_CNI_RIL3_CP_CAUSE_VALUE);
    friend bool smsHandlerMnRelReq  (SmsHalfCall *, SmsVblinkMsg_t            *, T_CNI_RIL3_CP_CAUSE_VALUE);

    friend bool smsHandlerTimeout   (SmsHalfCall *, void *                     , INT32           );
    friend bool smsHandlerTimeoutAck(SmsHalfCall *, void *                     , INT32           );
    friend bool smsHandlerTimerDisc (SmsHalfCall *, void *                     , INT32           );

public:
    
    // Constructors
    SmsHalfCall (class SmsLeg& smsLeg, smsEventTableEntry_t **table);

    // Destructor
    ~SmsHalfCall ();
    
    // Operators
    
    // maintenance methods

    void cleanup          (T_CNI_RIL3_CAUSE_VALUE  cause);

    //Parse Timer Message
    int handleTimeoutMsg  (IntraL3Msg_t *smsTimerMsg);

    bool validateEvent    (JCCEvent_t event)
    {
        return ((event > SMS_EVENT_BASE              && event <= SMS_EVENT_DEFAULT  ) ||
                (event > CC_INTERNAL_CALL_EVENT_BASE && event <= HC_LOCAL_EVENT_BASE)   );
    }

    // Main FSM access method Move to Protected?
    bool invokeHandler    (JCCEvent_t event, void *data, INT32 param1);

    // Retreive saved RPDU data
    const T_CNI_RIL3_IE_CP_USER_DATA & getRpdu () {return (rpdu_);}
    bool                                isRpdu () {return (rpdu_.ie_present);}

    // get and set for private data members
    SmsState_t getState   () {return (currentState_);}
    SmsLeg   & getParent  () {return (parent_      );}

    void setLastAttemt    () {lastAttempt_ = true   ;}
    void resetLastAttemt  () {lastAttempt_ = false  ;}
    bool isLastAttemt     () {return (lastAttempt_) ;}

    static const char * getEventName  (JCCEvent_t);

    bool isActive         () {return (currentState_ >= SMS_STATE_CONNECTED);}

protected:

    void setNextState     (SmsState_t state) 
                             {nextState_   = state  ;}
    void resetNextState   () {nextState_   = SMS_STATE_MAX;}

    void resetRpdu        ();
    void saveRpdu         (SmsVblinkMsg_t *);

private:

    void initData         (); 
    void setNewState      (SmsState_t);
    
protected:

    smsEventTableEntry_t       **smsTable_    ; //pointer to the FSM Table
    SmsLeg                     & parent_      ; //SmsLeg
    JCCTimer                    *smsTimer_    ; //Protocol Timer
    T_CNI_RIL3_IE_CP_USER_DATA   rpdu_        ; //PDU data storage. Contains data till 
                                                //CP PDU is finally sent
    SmsState_t                   nextState_   ; //Override default next state from FSM
    SmsState_t                   currentState_; //Current SMS FSM state
    bool                         lastAttempt_ ; //CP-DATA resend attempt flag

private:


};


/*******************************************************************/
class SmsMoHalfCall : public SmsHalfCall
{
    
public:

    // Constructors
    SmsMoHalfCall (class SmsLeg& smsLeg);

    // Destructor
    ~SmsMoHalfCall () {};

    // Operators

    // maintenance methods

    // get and set for private data members 

protected:

//private:

    // hide the assignment, and copy ctor and other defaults as needed.

public:

};


/*******************************************************************/
class SmsMtHalfCall : public SmsHalfCall
{

public:

    // Constructors
    SmsMtHalfCall (class SmsLeg& smsLeg);

    // Destructor
    ~SmsMtHalfCall () {};

    // Operators

    // maintenance methods

    // Even handler methods

    // get and set for private data members 

//protected:
    
private:

    // hide the assignment, and copy ctor and other defaults as needed.

private:

};

#endif // SMSHALFCALL_H
