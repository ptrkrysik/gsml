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
// File        : SmsLeg.cpp
// Author(s)   : Igal Gutkin
// Create Date : 01-20-2000
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
//

#include <vxworks.h>
#include <string.h>
#include <taskLib.h>


#include "Logging\vcmodules.h"

#include "CC/CCSessionHandler.h"

#include "CC/SmsCommonDef.h"
#include "CC/SmsLeg.h"

extern bool callTrace;

// *******************************************************************
// 
// *******************************************************************


// *******************************************************************
// SmsLeg class
// *******************************************************************

// Definition of the static data member
UINT32 SmsLeg::lastVbRefNum_ (0);


SmsLeg::SmsLeg (CCSessionHandler *session,
                int               callId ,
                MSG_Q_ID          qid    ,
                short             legNum )
            : CallLeg ()
{
  DBG_FUNC ("SmsLeg::SmsLeg (...)", CC_SMS_LAYER);
  DBG_ENTER();

    msgBuilder = new SmsMsgBuilder (*this);

    initData ();

    // Init parent class data
    CallLeg::parent     = session;
    CallLeg::callIndex  = callId ;
    CallLeg::msgQId     = qid    ;
    CallLeg::callLegNum = legNum ;

    if (callLegNum == SMS_MO_LEG_NUM)
        InitMoLeg (); // MO SMS Leg
    else
        InitMtLeg (); // MT SMS leg

  DBG_LEAVE();
}


void SmsLeg::initData ()
{
  DBG_FUNC  ("SmsLeg::initData", CC_SMS_LAYER);
  DBG_ENTER ();

        //Init parent class
        CallLeg::initData ();

        // Reset local data
        resetTi        ();
        resetVbRefNum  ();
        setRemRelCause (CNI_RIL3_RP_CAUSE_PROTOCOL_ERROR_UNSPECIFIED);

  DBG_LEAVE();
}


bool SmsLeg::InitMoLeg ()
{
  DBG_FUNC  ("SmsLeg::InitMoLeg", CC_SMS_LAYER);
  DBG_ENTER ();

    legType_ = SMS_MO_LEG;
    smsFsm_  = new SmsMoHalfCall (*this);

  DBG_LEAVE();
  return true;
}


bool SmsLeg::InitMtLeg ()
{
  DBG_FUNC ("SmsLeg::InitMtLeg", CC_SMS_LAYER);
  DBG_ENTER();

    legType_ = SMS_MT_LEG;
    smsFsm_  = new SmsMtHalfCall (*this);

  DBG_LEAVE();
  return   (true);
}


SmsLeg::~SmsLeg ()
{
    // Allocated memory cleanup

    if (smsFsm_)
        delete smsFsm_;
    if (msgBuilder)
        delete msgBuilder;
}


void SmsLeg::cleanup (T_CNI_RIL3_CAUSE_VALUE cause)
{
  DBG_FUNC  ("SmsLeg::cleanup", CC_SMS_LAYER);
  DBG_ENTER ();

    smsFsm_->cleanup (cause);

    initData();

  DBG_LEAVE();
  return;
}


bool SmsLeg::setupOrig (T_CNI_RIL3_SI_TI ti)
{
  // false to reject the setup
  bool retVal;

  DBG_FUNC ("SmsLeg::setupOrig", CC_SMS_LAYER);
  DBG_ENTER();

    if (getSmsLegType() != SMS_MO_LEG)
    {
        DBG_ERROR ("SMS Error: SMS leg %d is of wrong type (MT).\n", 
                   (callLegNum - SMS_LEGNUM_OFFSET));
        return (isIdle()); // keep current state of the Leg
    }

    if (smsFsm_->invokeHandler (SMS_EVENT_MM_EST_IND, NULL_PTR, ti) == false)
    {  // returns true if the connection is rejected and the leg is released (reverse logic), 
       // otherwise mark the leg as Busy

        setBusy ();
        retVal = true;

        if (callTrace)
        {
            char callTaskName[20];

            sprintf (callTaskName, "Sms-%d EntryId-%d\0 MO", callIndex, entryId()); 

            DBG_FUNC(callTaskName, CC_LAYER);
        }
    }
    else
        retVal = false;

  DBG_LEAVE();
  return   (retVal);
}


void SmsLeg::setupTerm (VOIP_CALL_HANDLE voipCallHandle, int  index      ,
                        int              refValue      , bool callWaiting)
{
  DBG_FUNC ("SmsLeg::setupTerm", CC_SMS_LAYER);
  DBG_ENTER();

    if (isIdle())
    {
        /* voipCallHandle and callWaiting are not in use for the SMS */
        /* CDR parameters */
        cdrIndex     = index   ;
        callRefValue = refValue;

        setBusy ();
    }

  DBG_LEAVE();
}


bool SmsLeg::isHoAllowed ()
{
    // HO is allowed when SMS is Idle or in the stage of active signalling
    return (isIdle() || getHc()->isActive());
}


int SmsLeg::handleTimeoutMsg (IntraL3Msg_t *smsTimerMsg)
{
  int retVal = 0;

  DBG_FUNC ("SmsLeg::handleTimeoutMsg", CC_SMS_LAYER);
  DBG_ENTER();

    if (!isIdle())
    {
        /* Pass timer message to the FSM
           Returns 0 if the leg is keep going, 
           otherwise returns disconnect cause value  */
        retVal = handleMsg (SMS_EVENT_TIMEOUT, NULL_PTR, 0);
    }
    else
    {
        DBG_ERROR ("SMS Leg %d Error: Unexpected Timeout Message.\n", 
                   (callLegNum - SMS_LEGNUM_OFFSET)                  );
    }

  DBG_LEAVE();
  return   (retVal);
}


bool SmsLeg::handleL2Msg (T_CNI_RIL3MD_CCMM_MSG& l2InMsg, JCCEvent_t event)
{  // L2 service messages (without PDU)
  bool retVal;

  DBG_FUNC  ("SmsLeg::handleL2Msg", CC_SMS_LAYER);
  DBG_ENTER ();

    retVal = handleMsg (event, &l2InMsg, 0);

  DBG_LEAVE ();
  return    (retVal);
}


bool SmsLeg::handleMmMsg (IntraL3Msg_t& msInMsg, JCCEvent_t event)
{  // Handle MM service messages
  bool retVal = false;

  DBG_FUNC  ("SmsLeg::handleMmMsg", CC_SMS_LAYER);
  DBG_ENTER ();

    // don't forward late MM Release Ind (in fact confirmation)
    if (!(isIdle() && event == SMS_EVENT_MM_REL_IND))
    {
        // Don't check TI here 
        retVal = handleMsg (event, &msInMsg, 0);
    }

  DBG_LEAVE ();
  return    (retVal);
}


bool SmsLeg::handleMsMsg (T_CNI_RIL3SMS_CP_MSG& msInMsg, JCCEvent_t event)
{
  bool retVal;

  DBG_FUNC  ("SmsLeg::handleMsMsg", CC_SMS_LAYER);
  DBG_ENTER ();

    // Don't check TI here 
    retVal = handleMsg (event, &msInMsg, 0);

  DBG_LEAVE ();
  return    (retVal);
}


bool SmsLeg::handleVbMsg (SmsVblinkMsg_t& netInMsg, JCCEvent_t event)
{
  bool retVal;
    
  DBG_FUNC  ("SmsLeg::handleVbMsg", CC_SMS_LAYER);
  DBG_ENTER ();

    retVal = handleMsg (event, &netInMsg, netInMsg.hdr.status);

  DBG_LEAVE ();
  return    (retVal);
}


bool SmsLeg::handleMsg (JCCEvent_t event, void *data, INT32 param1)
{
  bool                      retVal;
  T_CNI_RIL3_RP_CAUSE_VALUE cause ;

  DBG_FUNC  ("SmsLeg::handleMsg", CC_SMS_LAYER);
  DBG_ENTER ();

    if (isIdle()) // Protection: The Leg is not initiated
    {
        retVal = true; // The leg is already inactive
        DBG_WARNING ("Idle %s SMS Leg %d received event %d. The message will be discarded.\n",
                     (getSmsLegType() == SMS_MO_LEG) ? "MO":"MT", 
                     (callLegNum - SMS_LEGNUM_OFFSET)           , event);
    }
    else if (retVal = smsFsm_->invokeHandler (event, data, param1))
    {
        cause = getRemRelCause ();

        releaseRemSide (cause);
        cleanup        ((T_CNI_RIL3_CAUSE_VALUE)cause);
    }

  DBG_LEAVE ();
  return    (retVal);
}


bool SmsLeg::releaseRemSide   (T_CNI_RIL3_RP_CAUSE_VALUE cause)
{
  DBG_FUNC  ("SmsLeg::releaseRemSide", CC_SMS_LAYER);
  DBG_ENTER ();

    /* There is no Network side if reference number isn't allocated */
    if (getVbRefNum() != INVALID_VC_SMS_REF_NUM)
    { // release remote side on low layer error
        msgBuilder->sendMnSmsErrorInd (cause);
        resetVbRefNum ();
    }

  DBG_LEAVE ();
  return    (true);
}


/*===============================================================*/

void SmsLeg::printData (JCCPrintStFn fnPtr)
{
  char strLeg []   = "Sms Leg:\n";
  char strInfo[120];
    
    sprintf (strInfo, 
    "(%s SMS Leg number = %d) (Idle - %s) (SMS FSM State = %d) (TI= %d) (VB ref. num %lu).\n",
            (legType_ == SMS_MO_LEG) ? "MO"  : "MT", callLegNum         , 
            (isIdle()              ) ? "Yes" : "No", 
            smsFsm_->getState(), getTi(), getVbRefNum());

    (*fnPtr)(strLeg );
    (*fnPtr)(strInfo);
}

//static 
UINT32 SmsLeg::allocateVbRefNum ()
{
    if (++lastVbRefNum_ > MAX_VC_SMS_REF_NUM)
        lastVbRefNum_ = MIN_VC_SMS_REF_NUM;

    return (lastVbRefNum_);
}


void SmsLeg::setVbRefNum (UINT32 refNum = INVALID_VC_SMS_REF_NUM)
{
  DBG_FUNC ("SmsLeg::setVbRefNum", CC_SMS_LAYER);
  DBG_ENTER();

    // Store new ref. number in the object
    vbRefNum_ = (refNum != INVALID_VC_SMS_REF_NUM)
              ? refNum 
              : allocateVbRefNum ();

    DBG_TRACE ("SMS: Set VBLink ref.num = %u by the task %s , SMS Leg %d\n", 
               vbRefNum_, taskName(taskIdSelf()), (callLegNum - SMS_LEGNUM_OFFSET));
  DBG_LEAVE();
}


void SmsLeg::allocateTi ()
{
  /* Should be used for MT leg only */
  DBG_FUNC ("SmsLeg::AllocateTi", CC_SMS_LAYER);
  DBG_ENTER();

    if (getSmsLegType() == SMS_MT_LEG)
    {
        setTi     (callLegNum); 
        DBG_TRACE ("SMS: Allocate Ti = %d by the task %s , SMS Leg %d\n", 
                   getTi(), taskName(taskIdSelf()), (callLegNum - SMS_LEGNUM_OFFSET));
    }
    else
        DBG_ERROR ("SMS: MO Leg should use received TI. TI is not allocated.\n");

  DBG_LEAVE();
}

