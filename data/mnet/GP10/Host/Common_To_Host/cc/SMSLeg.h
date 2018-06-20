#ifndef SmsLeg_H
#define SmsLeg_H


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
// File        : SmsLeg.h
// Author(s)   : Igal Gutkin
// Create Date : 01-19-00
// Description : class specification for -  SmsLeg
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************


// Parent Class
#include "CC/CallLeg.h"

#include "ril3\ril3_sms_cp.h"
#include "ril3\ril3_sms_rp.h"

#include "CC/SmsHandler.h"
#include "CC/SmsMsgBuilder.h"
#include "CC/SmsHalfCall.h"

typedef enum 
{
    SMS_NONE_LEG,
    SMS_MO_LEG  ,
    SMS_MT_LEG
} SmsLegType_t;

class SmsLeg : public CallLeg
{

public:

    // Destructor
    ~SmsLeg();

    // Constructors
    SmsLeg (CCSessionHandler *session, int callId, MSG_Q_ID qid, short callLegNum);

    // Operators

    // primary behaviour methods
    virtual int  handleTimeoutMsg (IntraL3Msg_t *);

    bool handleMsMsg    (T_CNI_RIL3SMS_CP_MSG &, JCCEvent_t);
    bool handleL2Msg    (T_CNI_RIL3MD_CCMM_MSG&, JCCEvent_t);
    bool handleMmMsg    (IntraL3Msg_t         &, JCCEvent_t);
    bool handleVbMsg    (SmsVblinkMsg_t       &, JCCEvent_t);

    bool setupOrig      (T_CNI_RIL3_SI_TI ti = 0           );
    void setupTerm      (VOIP_CALL_HANDLE, int, int, bool  );

    bool isHoAllowed    ();

    // maintenance methods
    virtual void cleanup          (T_CNI_RIL3_CAUSE_VALUE  cause  );
    
    virtual void printData        (JCCPrintStFn            fnPtr  );

    // get and set for private data members 
    SmsHalfCall * getHc() {return (smsFsm_);};

    // idle is a member of the parent class CallLeg
    bool isIdle  () {return (idle);}
    void setBusy () {idle = false ;}  
    void setIdle () {idle = true  ;}

    void setRemRelCause (T_CNI_RIL3_RP_CAUSE_VALUE cause) {remCause_ = cause;}
    T_CNI_RIL3_RP_CAUSE_VALUE getRemRelCause ()           {return remCause_ ;}

    void setTi   (T_CNI_RIL3_SI_TI nt) {mainTi_ = nt            ;}
    void resetTi                    () {mainTi_ = SMS_TI_INVALID;}
    T_CNI_RIL3_SI_TI getTi          () {return (mainTi_ )       ;}
    void allocateTi                 ();

    SmsLegType_t     getSmsLegType  () {return (legType_)       ;}

    void   setVbRefNum              (UINT32 refNum = INVALID_VC_SMS_REF_NUM);
    UINT32 getVbRefNum              () {return (vbRefNum_)                ;}
    void   resetVbRefNum            () {vbRefNum_ = INVALID_VC_SMS_REF_NUM;}

protected:
    void initData ();

    bool InitMoLeg ();
    bool InitMtLeg ();

private:

    // hide the assignment, and copy ctor and other defaults as needed.
    SmsLeg ();

    bool handleMsg                 (JCCEvent_t, void *, INT32);
    bool releaseRemSide            (T_CNI_RIL3_RP_CAUSE_VALUE);

    static UINT32 allocateVbRefNum ();

    SmsLeg& operator =  (const SmsLeg& rhs)      ;
    int     operator == (const SmsLeg& rhs) const;

public:
    // SMS message build & send class
    SmsMsgBuilder    *msgBuilder;

private:

    // SMS Leg Type 
    SmsLegType_t      legType_;

    // SMS FSM instance
    SmsHalfCall      *smsFsm_ ;

    // Transaction Identifier associated with the leg
    T_CNI_RIL3_SI_TI  mainTi_ ;

    // ViperCell 1 - 32767, ViperBase 32768 - 65535 
    UINT32            vbRefNum_;

    //Disconnect cause
    T_CNI_RIL3_RP_CAUSE_VALUE remCause_;

    // Vb Reference number counter
    static UINT32     lastVbRefNum_;

};


#endif //SmsLeg_h

