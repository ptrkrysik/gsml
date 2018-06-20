// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************
// CdrSSA.cpp: implementation of the CCdrSSA class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "Cdr\CdrSSA.h"


CCdrSSA::CCdrSSA()
    : CCdrBase (), 
      isInit   (true )
{
    initialize ();
}

CCdrSSA::~CCdrSSA()
{
}

void CCdrSSA::initialize ()
{
    // skip in for the very first time when is called by the constructor
    if (!isInit)
        CCdrBase::initialize ();
    else
        isInit = false;

    setRecordType (CDR_RECORD_TYPE_SSA);

    basic_service_code              = 0    ;
    basic_service_selector          = 0    ;
    basic_service_set               = false;
    correlated_event_ref_number     = 0    ;
    correlated_event_ref_number_set = false;

    redirected_dn.initialize            ();
    transfer_dn.initialize              ();
    translated_redirected_dn.initialize ();
    translated_transfer_dn.initialize   ();
}

void CCdrSSA::setBearerServiceCode(unsigned char code)
{
    basic_service_set=true;
    basic_service_selector=1;
    basic_service_code=code;
}

void CCdrSSA::setTeleServiceCode(unsigned char code)
{
    basic_service_set=true;
    basic_service_selector=2;
    basic_service_code=code;
}

void CCdrSSA::setCorrelatedEventRefNumber( T_CDR_EVENT_REFERENCE_NUMBER event_number)
{
    correlated_event_ref_number_set=true;
    correlated_event_ref_number=event_number;
}

void CCdrSSA::setRedirectedNumber(T_SUBSC_IE_ISDN * p_DN) { redirected_dn.set(p_DN); }
void CCdrSSA::setTranslatedRedirectedNumber(T_SUBSC_IE_ISDN * p_DN){ translated_redirected_dn.set(p_DN); }
void CCdrSSA::setTransferNumber(T_SUBSC_IE_ISDN * p_DN) { transfer_dn.set(p_DN); }
void CCdrSSA::setTranslatedTransferNumber(T_SUBSC_IE_ISDN * p_DN) { translated_transfer_dn.set(p_DN); }

void CCdrSSA::generateRecord(
        T_CDR_SS_ACTION ss_action, 
        unsigned char ss_code, 
        T_CNI_RIL3_CAUSE_VALUE ss_cause
        )
{
  CCdrFieldDateTime ss_action_time;

    // get current time
    ss_action_time.stamp();

    // first generate common header fields
    printBaseFields();

    // print all the fields of this class to the outbuffer
    // 
    // K1=SS Action
    // K2=SS Code
    // K3=SS Cause
    // T5=SS Action Time
    // N5=Redirected Number
    // N6=Translated Redirected Number
    // N7=Transfer Number
    // N8=Translated Transfer Number
    // B1=Basic Service Code
    // R1=Correlated Event Reference Number
    //
    count+=sprintf(&cdr_msg.record[count],"%cK1=%02X",CDR_FIELD_DELIMETER,(unsigned long)ss_action);
    count+=sprintf(&cdr_msg.record[count],"%cK2=%02X",CDR_FIELD_DELIMETER, ss_code);
    count+=sprintf(&cdr_msg.record[count],"%cK3=%03d" ,CDR_FIELD_DELIMETER, ss_cause);

    count+=sprintf(&cdr_msg.record[count],"%cT5=%s",CDR_FIELD_DELIMETER,ss_action_time.string());

    if(redirected_dn.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count+=sprintf(&cdr_msg.record[count],"%cN5=%s",CDR_FIELD_DELIMETER,redirected_dn.string());
    if(translated_redirected_dn.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count+=sprintf(&cdr_msg.record[count],"%cN6=%s",CDR_FIELD_DELIMETER,translated_redirected_dn.string());
    if(transfer_dn.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count+=sprintf(&cdr_msg.record[count],"%cN7=%s",CDR_FIELD_DELIMETER,transfer_dn.string());
    if(translated_transfer_dn.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count+=sprintf(&cdr_msg.record[count],"%cN8=%s",CDR_FIELD_DELIMETER,translated_transfer_dn.string());
    if(basic_service_set)
        count+=sprintf(&cdr_msg.record[count],"%cB1=%1d%02X",CDR_FIELD_DELIMETER,
                        basic_service_selector,    basic_service_code );
    if(correlated_event_ref_number_set)
        count+=sprintf(&cdr_msg.record[count],"%cR1=%d",CDR_FIELD_DELIMETER, correlated_event_ref_number );

    // send the generated record off to the CDR client
    CCdrBase::generateRecord();
}

