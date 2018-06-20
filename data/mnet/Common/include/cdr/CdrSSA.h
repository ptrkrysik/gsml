// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************
// CdrSSA.h: interface for the CCdrSSA class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _CDR_SSA_H_
#define _CDR_SSA_H_

#include "cdr\include\CdrBase.h"

typedef enum {
    CDR_SS_ACTION_REGISTRATION          = 0,
    CDR_SS_ACTION_ERASURE               = 1,
    CDR_SS_ACTION_ACTIVATION            = 2,
    CDR_SS_ACTION_DEACTIVATION          = 3,
    CDR_SS_ACTION_INTERROGATION         = 4,
    CDR_SS_ACTION_INVOCATION            = 5,
    CDR_SS_ACTION_PASSWORD_REGISTRATION = 6
} T_CDR_SS_ACTION;

class CCdrSSA : public CCdrBase
{
private:
    bool                         isInit                         ;
    bool                         basic_service_set              ;
    unsigned char                basic_service_selector         ;
    unsigned char                basic_service_code             ;
    CCdrFieldE164                redirected_dn                  ;
    CCdrFieldE164                translated_redirected_dn       ;
    CCdrFieldE164                transfer_dn                    ;
    CCdrFieldE164                translated_transfer_dn         ;
    bool                         correlated_event_ref_number_set;
    T_CDR_EVENT_REFERENCE_NUMBER correlated_event_ref_number    ;

public:
             CCdrSSA ();
    virtual ~CCdrSSA ();

    virtual void initialize               ();

    void     setTeleServiceCode           (unsigned char code);
    void     setBearerServiceCode         (unsigned char code);
    void     setCorrelatedEventRefNumber  (T_CDR_EVENT_REFERENCE_NUMBER event_number);
    void     setRedirectedNumber          (T_SUBSC_IE_ISDN * p_DN);
    void     setTranslatedRedirectedNumber(T_SUBSC_IE_ISDN * p_DN);
    void     setTransferNumber            (T_SUBSC_IE_ISDN * p_DN);
    void     setTranslatedTransferNumber  (T_SUBSC_IE_ISDN * p_DN);
    void     generateRecord               (T_CDR_SS_ACTION ss_action, 
                                           unsigned char   ss_code  ,
                                           T_CNI_RIL3_CAUSE_VALUE ss_cause);
};


//    Sample usage of SSA record
//
//    #include "CdrSSA.h"
//    ....
//    {
//        CCdrSSA        ssa_record;
//        .....
//        ssa_record.setServedIMSI( <pointer-to-IMSI> );
//        ssa_record.setServedMSISDN( <pointer-to-MSISDN> );
//        ssa_record.setTeleServiceCode( <code> );
//        ssa_record.setCorrelatedEventRefNumber( <event-ref-number> );
//        ssa_record.setRedirectedNumber( <pointer-to-E164> );
//        ssa_record.setTranslatedRedirectedNumber( <pointer-to-E164> );
//        ssa_record.generateRecord();
//        // at this point, the record is generated, and the ssa_record can be safely discard!
//
//    }


#endif
