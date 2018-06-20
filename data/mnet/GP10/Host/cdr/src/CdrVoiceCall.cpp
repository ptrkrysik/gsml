// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************
// CdrVoiceCall.cpp: implementation of the CCdrVoiceCall class.
//
//////////////////////////////////////////////////////////////////////

#ifdef VXWORKS

#include "stdafx.h"

#include "Cdr\CdrVoiceCall.h"


CCdrVoiceCall::CCdrVoiceCall()
    : CCdrBase (),
      isInit   (true)
{
    initialize ();
}


void CCdrVoiceCall::initialize()
{
    // skip it for the very first time when is called by the constructor
    if (!isInit)
        CCdrBase::initialize ();
    else
        isInit = false;

    call_duration    = 0;
    cc_cause         = (T_CNI_RIL3_CAUSE_VALUE)0;
    cdr_cause        = CDR_REC_NORM_RELEASE     ;
    emergency_call   = false;
    q931_crv         = 0;
    radioChannelUsedStatus         = CDR_FIELD_NOT_SET;
    radio_channel_used_bearer_type = 0;
    radio_channel_used_codec       = 0;
    seq_number       = 0;
    transaction_id   = 0;

    initLaChange                   ();
    answer_time.initialize         ();
    begin_time.initialize          ();
    called_party_number.initialize ();
    calling_party_number.initialize();
    end_time.initialize            ();
    la_cell_id.initialize          ();

    served_imei.initialize         ();

    release_time.initialize();
    seisure_time.initialize();
    served_imei.initialize ();
}

void CCdrVoiceCall::initLaChange ()
{
    loc_change_count = 0;

    for (int index=0; index<5; index++) 
    {
        loc_change_la_cid[index].initialize ();
        loc_change_time  [index].initialize ();
    }
}

void CCdrVoiceCall::setServedIMEI         (T_CNI_RIL3_IE_MOBILE_ID *p_imei)
                                        { served_imei.set(p_imei);         }

void CCdrVoiceCall::setCallingPartyNumber (T_SUBSC_IE_ISDN * p_CgPN)
                                        {calling_party_number.set(p_CgPN); }

void CCdrVoiceCall::setCalledPartyNumber  (T_SUBSC_IE_ISDN * p_CdPN)
                                        {called_party_number.set(p_CdPN);  }

void CCdrVoiceCall::setCallingPartyNumber (T_CNI_RIL3_IE_CALLING_PARTY_BCD_NUMBER *pNum)
                                        { calling_party_number.set (pNum); }

void CCdrVoiceCall::setCalledPartyNumber  (T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  *pNum)
                                        { called_party_number.set (pNum);  }

void CCdrVoiceCall::setLocationAreaCID (T_CNI_RIL3_IE_LOCATION_AREA_ID *pLaId, 
                                        T_CNI_RIL3_IE_CELL_ID          *pCellId)
                                        { la_cell_id.set (pLaId, pCellId); }


void CCdrVoiceCall::seisure (int newTI)
{
    seisure_time.stamp ();
    begin_time.stamp   ();

    transaction_id = newTI;
    seq_number     = 1    ;
}

void CCdrVoiceCall::seisure_NormalOrigination (int trans_id)
{
    // seisure time will not trigger sending of record
    seisure (trans_id);

    emergency_call = false;
    setRecordType (CDR_RECORD_TYPE_MOC);
}

void CCdrVoiceCall::seisure_EmergencyOrigination (int trans_id)
{
    // seisure time will not trigger sending of record
    seisure (trans_id);

    emergency_call = true;
    setRecordType (CDR_RECORD_TYPE_MOC);
}

void CCdrVoiceCall::seisure_NormalTermination (int trans_id)
{
    // seisure time will not trigger sending of record
    seisure (trans_id);

    emergency_call = false;
    setRecordType (CDR_RECORD_TYPE_MTC);
}


void CCdrVoiceCall::answer(unsigned char bear_type, unsigned char codec)
{
    // answer time will not trigger sending of record
    answer_time.stamp();

    radioChannelUsedStatus         = CDR_FIELD_SET_BUT_NOT_PRINT;
    radio_channel_used_bearer_type = bear_type                  ;
    radio_channel_used_codec       = codec                      ;
}


void CCdrVoiceCall::generateCallRecord (bool partial)
{
  T_CDR_RECORD_TYPE recType = getRecordType();

  // prepare all the data fields
    end_time.stamp();

    // Calculate the call duration according GSM 12.05
    // 1. Incomplete calls (call attempts): from seizure to call release
    // 2. Complete (answered) calls: from answer to release
    // 3. Partial record: non-cumulitive duration of the individual partial record
    // 4. Should not be 0

    if (partial) 
        cdr_cause = CDR_REC_PARTIAL;

    switch (cdr_cause)
    {
    case CDR_REC_PARTIAL:
        // Partial record
        call_duration = end_time.getTime() - begin_time.getTime();
        break;

    case CDR_REC_NORM_RELEASE:
        // Complete call
        if (seq_number > 1)
        { // Last partial record
            call_duration = release_time.getTime() - begin_time.getTime();
        }
        else
        { // complete record
            call_duration = release_time.getTime() - answer_time.getTime();
        }
        break;

    default:
        // Incomplete call attempt
        call_duration = end_time.getTime() - seisure_time.getTime();
    }

    if (!call_duration)
        ++call_duration; // zero duration correction

    // generate common header fields first to the output buffer
    printBaseFields ();

    // print all the fields of this class to the outbuffer
    // 
    // O2=Served IMEI
    // N1=Calling Party Number
    // N2=Called Party Number
    // O5=Transaction ID
    // O7=Location Area and Cell ID
    // H1=Q931 CRV (for MOC)
    // H2=Q931 CRV (for MTC)
    // L1=Location Changes (may be repeated up to 5 times)
    // T1=Seisure Time
    // T2=Answer Time
    // T3=Release Time
    // E1=Emergency Call (MOC only)
    // T9=Call Duration
    // R1=Radio Channel Used - Bearer Type
    // R2=Radio Channel Used - Codec
    // C1=CC Cause for termination
    // C2=CDR Cause for termination
    // Z1=Sequence Number
    // Z2=Begin Date and Time
    // Z3=End Date and Time

    // O2=Served IMEI
    if (served_imei.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count += sprintf(&cdr_msg.record[count],"%cO2=%s",CDR_FIELD_DELIMETER,served_imei.string());

    // N1=Calling Party Number
    if (recType == CDR_RECORD_TYPE_MTC && calling_party_number.status == CDR_FIELD_SET_BUT_NOT_PRINT)
        count += sprintf(&cdr_msg.record[count],"%cN1=%s",CDR_FIELD_DELIMETER,calling_party_number.string());

    // N2=Called Party Number
    if (recType == CDR_RECORD_TYPE_MOC && called_party_number.status  == CDR_FIELD_SET_BUT_NOT_PRINT)
        count += sprintf(&cdr_msg.record[count],"%cN2=%s",CDR_FIELD_DELIMETER,called_party_number.string());

    // O5=Transaction ID
    count += sprintf(&cdr_msg.record[count],"%cO5=%d",CDR_FIELD_DELIMETER,transaction_id);

    // O7=Location Area and Cell ID
    count += sprintf(&cdr_msg.record[count],"%cO7=%s",CDR_FIELD_DELIMETER,la_cell_id.string());

    // H1=Q931 CRV (for MOC)
    // H2=Q931 CRV (for MTC)
    count += sprintf (&cdr_msg.record[count],"%cH%c=%04X",
                      CDR_FIELD_DELIMETER, (recType == CDR_RECORD_TYPE_MOC) ? '1':'2', q931_crv);

    // L1=Location Changes (may be repeated up to 5 times)
    for (int index=0; index < loc_change_count; index++)
    {
        if (loc_change_la_cid[index].status == CDR_FIELD_SET_BUT_NOT_PRINT)
        count += sprintf (&cdr_msg.record[count],"%cL1=%s%s", CDR_FIELD_DELIMETER,
                          loc_change_la_cid[index].string() , loc_change_time[index].string());
    }

    // T1=Seisure Time
    if (seisure_time.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count += sprintf(&cdr_msg.record[count],"%cT1=%s",CDR_FIELD_DELIMETER,seisure_time.string());

    // T2=Answer Time
    if (answer_time.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count += sprintf(&cdr_msg.record[count],"%cT2=%s",CDR_FIELD_DELIMETER,answer_time.string());

    // T3=Release Time
    if (release_time.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count += sprintf(&cdr_msg.record[count],"%cT3=%s",CDR_FIELD_DELIMETER,release_time.string());

    if (recType == CDR_RECORD_TYPE_MOC)
    { // E1=Emergency Call (MOC only)
        count += sprintf(&cdr_msg.record[count],"%cE1=%d",CDR_FIELD_DELIMETER,emergency_call?1:0);
    }

    // T9=Call Duration
    count += sprintf(&cdr_msg.record[count],"%cT9=%d",CDR_FIELD_DELIMETER,call_duration);

    // R1=Radio Channel Used - Bearer Type
    // R2=Radio Channel Used - Codec
    if (radioChannelUsedStatus == CDR_FIELD_SET_BUT_NOT_PRINT)
    { // these two fields are set and print along with answer time field
        count += sprintf(&cdr_msg.record[count],"%cR1=%1d" ,CDR_FIELD_DELIMETER,radio_channel_used_bearer_type);
        count += sprintf(&cdr_msg.record[count],"%cR2=%02X",CDR_FIELD_DELIMETER,radio_channel_used_codec);
        radioChannelUsedStatus = CDR_FIELD_SET_AND_PRINTED;
    }

    // C1=CC Cause for termination
    if (release_time.status != CDR_FIELD_NOT_SET)
    {
        // this field is set and print along with release time field
        count+=sprintf(&cdr_msg.record[count],"%cC1=%d",CDR_FIELD_DELIMETER,cc_cause);
    }

    // C2=CDR Cause for termination
    count += sprintf(&cdr_msg.record[count],"%cC2=%d",CDR_FIELD_DELIMETER,cdr_cause);
    
    // Z1=Sequence Number
    count += sprintf(&cdr_msg.record[count],"%cZ1=%d",CDR_FIELD_DELIMETER, seq_number++);

    // Z2=Begin Date and Time
    count += sprintf(&cdr_msg.record[count],"%cZ2=%s",CDR_FIELD_DELIMETER,begin_time.string());

    // Z3=End Date and Time
    count += sprintf(&cdr_msg.record[count],"%cZ3=%s",CDR_FIELD_DELIMETER,end_time.string());

    // send the record off to the CDR Client
    CCdrBase::generateRecord();

    if (cdr_cause == CDR_REC_PARTIAL)
    {
        // reset Location Change history
        initLaChange     ();

        // reset begin time for the next record
        begin_time.stamp ();
    }
}

void CCdrVoiceCall::release (T_CNI_RIL3_CAUSE_VALUE cc_cause_value, CdrRecCauseTerm_t cdr_cause_value)
{
    // release time will always trigger sending of the complete record
    release_time.stamp();

    cdr_cause = cdr_cause_value;
    cc_cause  = cc_cause_value ;

    // print all the data fields
    generateCallRecord (false);
}


void CCdrVoiceCall::handover (T_CNI_RIL3_IE_LOCATION_AREA_ID *pLaId  ,
                              T_CNI_RIL3_IE_CELL_ID          *pCellId)
{
    if (loc_change_count >= 5)
    {
        generateCallRecord (true);
        initLaChange       ()    ;
    }

    loc_change_time  [loc_change_count].stamp()            ;
    loc_change_la_cid[loc_change_count].set(pLaId, pCellId);
    loc_change_count++;
}


#endif