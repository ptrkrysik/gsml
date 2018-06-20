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
// File        : SmsHalfCall.cpp
// Author(s)   : Igal Gutkin
// Create Date : 01-20-2000
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************


#include "CC/CCSessionHandler.h"
#include "CC/CallConfig.h"
#include "CC/SmsHalfCall.h"

extern const smsEventTableEntry_t *smsMoTable [], 
                                  *smsMtTable [];

// *******************************************************************
// 
// *******************************************************************
#define SMS_EVENT_NUMBER (CC_MM_CONNECTION_IND - CC_MM_CONNECTION_REQ + SMS_EVENT_DEFAULT-SMS_EVENT_BASE)

const char * SmsEventNames[SMS_EVENT_NUMBER] =
{
    {"Invalid"           }, //    0
    {"MM-EST Cnf"        }, //92  1
    {"MM-Est Rej"        }, //93  2
    {"MM-REL Ind"        }, //94  3
    {"MM-EST Ind"        }, //95  4

    {"DL-ERROR Ind"      }, //301 5

    {"CP-DATA"           }, //302 6
    {"CP-ACK"            }, //303 7
    {"CP-ERROR"          }, //304 8
    {"MNSMS-EST Req"     }, //305 9
    {"MNSMS-DATA Req"    }, //306 10
    {"MNSMS-REL Req"     }, //307 11
    {"MNSMS-ABORT Req"   }, //308 12
    {"TIMEOUT"           }, //309 13
    {"SMS-PROTOCOL ERROR"}  //310 14
};


/*******************************************************************/

SmsHalfCall::SmsHalfCall (SmsLeg& smsLeg, smsEventTableEntry_t **table)
             : parent_  (smsLeg),
               smsTable_(table )
{
  UINT16 uint16TimerValue;
  int    timerValue      ;

    // Timer Data below is used for sending timeout to the correct task.
    uint16TimerValue = (((UINT16)(parent_.callLegNum)) & (0x000F)) << 8;
    uint16TimerValue = uint16TimerValue | (((UINT16)(parent_.callIndex)) & 0x00FF);

    timerValue = (short)uint16TimerValue;

    smsTimer_ = new JCCTimer(callTimerExpiry, timerValue);

    initData ();
}


SmsHalfCall::~SmsHalfCall ()
{
    delete smsTimer_;
}


void SmsHalfCall::initData ()
{
    currentState_ = SMS_STATE_IDLE;
    
    resetNextState  ();
    resetLastAttemt ();
    resetRpdu       ();
}


void SmsHalfCall::cleanup (T_CNI_RIL3_CAUSE_VALUE  cause)
{
    // Not used upon VBLink abnormal release!!!
    initData ();
}


void SmsHalfCall::resetRpdu ()
{
    CNI_RIL3_ZAPMSG ((void *)&rpdu_, sizeof(T_CNI_RIL3_IE_CP_USER_DATA));
}


void SmsHalfCall::saveRpdu  (SmsVblinkMsg_t *vbMsgData)
{
  DBG_FUNC  ("SmsHalfCall::saveRpdu", CC_SMS_LAYER);
  DBG_ENTER ();

    if (vbMsgData->hdr.msgLen > 0)
    {
        rpdu_.ie_present = true;
        rpdu_.length     = vbMsgData->hdr.msgLen;

        memcpy ((void *)&rpdu_.rpdu_data, vbMsgData->msg, rpdu_.length);
    }
    else
        DBG_WARNING ("SMS: RP data is not presented.\n");

  DBG_LEAVE();
}


void SmsHalfCall::setNewState (SmsState_t state)
{
  DBG_FUNC  ("SmsHalfCall::setNewState", CC_SMS_LAYER);
  DBG_ENTER ();

    /*
    New state is set according to the following rules:
    1. Equal to nextState_ (override the table default)
       if it's set to a legal state [0, SMS_STATE_MAX)    , else
    2. Stay without change if nextState_ == SMS_STATE_NONE, else
    3. Equal to required state, if it's a legal state     , else
    4. Stay without change if state == SMS_STATE_NONE, 
    */

    currentState_ = (nextState_ < SMS_STATE_MAX) 
                  ? nextState_
                  : (nextState_ != SMS_STATE_NONE && state < SMS_STATE_MAX ) 
                  ? state 
                  : currentState_;

    nextState_ = SMS_STATE_MAX;  // Reset it

    DBG_WARNING ("SMS leg %d, new state = %d\n", 
                 (parent_.callLegNum - SMS_LEGNUM_OFFSET), currentState_);

  DBG_LEAVE();
}


// SMS FSM main access method
bool SmsHalfCall::invokeHandler (JCCEvent_t newEvent, void *data, INT32 param1)
{
    bool retVal ;
    int  index  = 0;

    DBG_FUNC  ("SmsHalfCall::invokeHandler", CC_SMS_LAYER);
    DBG_ENTER ();

    const smsEventTableEntry_t *stateTable = smsTable_[getState()];

    if (validateEvent (newEvent))
    {

        DBG_TRACE ("invokeHandler (%s SMS Leg %d).\n\
        Current State = %d, event %s (%d)\n",
                   (parent_.getSmsLegType() == SMS_MO_LEG) ? "MO" : "MT", 
                   (parent_.callLegNum - SMS_LEGNUM_OFFSET), getState(), 
                   getEventName(newEvent), newEvent);

        while ((stateTable + index)->event != newEvent        && 
               (stateTable + index)->event != SMS_EVENT_DEFAULT )
        {
            ++index;
        }

        DBG_WARNING ("SMS FSM table entry num %d is selected for state %d.\n", 
                     index, getState());

        // Invoke an appropriate handler
        retVal = (stateTable + index)->handler (this, data, param1);

        // Set new state
        // This state could be overrided by the handler!
        if (retVal == false) // Call Continues
            setNewState ((stateTable + index)->nextState);
        else
            setNewState (SMS_STATE_IDLE); // force state to IDLE

        if (getState() == SMS_STATE_IDLE)
            retVal = true;  // safety measure
    }

    else
    {  // Invalid event (out of the range). No change in a state of the half call.
        DBG_ERROR ("SMS Error: (SmsHalfCall::smsInvokeHandler) Event = %d is out of range\n", 
                   newEvent);
        retVal = false;
    }

    DBG_LEAVE ();
    return    (retVal);
}

const char * SmsHalfCall::getEventName  (JCCEvent_t event)
{
    static char buff [50];

    unsigned index;

    switch (event)
    {
    case SMS_EVENT_MM_EST_CNF     :
    case SMS_EVENT_MM_EST_REJ     :
    case SMS_EVENT_MM_REL_IND     :
    case SMS_EVENT_MM_EST_IND     :
        index = event - CC_INTERNAL_CALL_EVENT_BASE -1;
        break;

    case SMS_EVENT_MD_ERROR_IND   :
    case SMS_EVENT_CP_DATA        :
    case SMS_EVENT_CP_ACK         :
    case SMS_EVENT_CP_ERROR       :
    case SMS_EVENT_MNSMS_EST_REQ  :
    case SMS_EVENT_MNSMS_DATA_REQ :
    case SMS_EVENT_MNSMS_REL_REQ  :
    case SMS_EVENT_MNSMS_ABORT_REQ:
    case SMS_EVENT_TIMEOUT        :
    case SMS_EVENT_PROTOCOL_ERROR :
        index = event - SMS_EVENT_BASE + 4;
        break;

    default:
        sprintf (buff, "Unknown SMS event %d", event);
        return (buff);
    }

    return (SmsEventNames [index]);
}


/*******************************************************************/
SmsMoHalfCall::SmsMoHalfCall (SmsLeg& smsLeg) 
               : SmsHalfCall (smsLeg, (smsEventTableEntry_t **)smsMoTable)
{
    ;
}




/*******************************************************************/
SmsMtHalfCall::SmsMtHalfCall (SmsLeg& smsLeg) 
              : SmsHalfCall (smsLeg, (smsEventTableEntry_t **)smsMtTable)
{
    ;
}

