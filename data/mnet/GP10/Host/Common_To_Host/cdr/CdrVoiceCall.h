// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************
// CdrVoiceCall.h: interface for the CCdrVoiceCall class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _CDR_VOICECALL_H_
#define _CDR_VOICECALL_H_

#ifdef VXWORKS
#include  "cdr\include\CdrBase.h"

class CCdrVoiceCall : public CCdrBase
{
private:
	bool                   isInit              ;
//    bool                   is
    CCdrFieldIMEI          served_imei         ;    // fix string length = 15
    CCdrFieldE164          calling_party_number;    // max string length = 23 
    CCdrFieldE164          called_party_number ;    // max string length = 23
    CCdrFieldLACID         la_cell_id          ;    // max string length = 14
    unsigned char          loc_change_count    ;    // 0..5
    CCdrFieldLACID         loc_change_la_cid[5];
    CCdrFieldDateTime      loc_change_time  [5];
    CCdrFieldDateTime      seisure_time        ;    // fix string length = 15
    CCdrFieldDateTime      answer_time         ;    // fix string length = 15
    CCdrFieldDateTime      release_time        ;    // fix string length = 15
    CCdrFieldDateTime      begin_time          ;    // fix string length = 15
    CCdrFieldDateTime      end_time            ;    // fix string length = 15
    T_CNI_RIL3_CAUSE_VALUE cc_cause            ;
    CdrRecCauseTerm_t      cdr_cause           ;
    unsigned long          call_duration       ;
    int                    transaction_id      ;
    unsigned short         q931_crv            ;
    unsigned short         seq_number          ;
    bool                   emergency_call      ;
    T_CDR_FIELD_STATUS     radioChannelUsedStatus        ;
    unsigned char          radio_channel_used_bearer_type;
    unsigned char          radio_channel_used_codec      ;

    void                   seisure      (int newTI);
    void                   initLaChange ();

public:
             CCdrVoiceCall              ();
    virtual ~CCdrVoiceCall              () {};

    virtual void initialize             ();

    void    setServedIMEI               (T_CNI_RIL3_IE_MOBILE_ID *p_imei  );
    void    setCallingPartyNumber       (T_SUBSC_IE_ISDN         *p_CgPN  );
    void    setCallingPartyNumber       (T_CNI_RIL3_IE_CALLING_PARTY_BCD_NUMBER *);
    void    setCalledPartyNumber        (T_SUBSC_IE_ISDN         *p_CdPN  );
    void    setCalledPartyNumber        (T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  *);
    void    setLocationAreaCID          (T_CNI_RIL3_IE_LOCATION_AREA_ID * , 
                                        T_CNI_RIL3_IE_CELL_ID           * );

    void    setQ931Crv                  (unsigned short           crv     ) { q931_crv = crv; };
    void    seisure_NormalOrigination   (int                      trans_id);
    void    seisure_EmergencyOrigination(int                      trans_id);
    void    seisure_NormalTermination   (int                      trans_id);
    void    answer                      (unsigned char            bear_type,
                                         unsigned char            codec   );
    void    generateCallRecord          (bool partial);
    void    release                     (T_CNI_RIL3_CAUSE_VALUE   cc_cause_value ,
                                         CdrRecCauseTerm_t        cdr_cause_value);
    void    handover                    (T_CNI_RIL3_IE_LOCATION_AREA_ID *p_la_id  ,
                                         T_CNI_RIL3_IE_CELL_ID          *p_cell_id);
};


//    Sample usage of VoiceCall record for normal orignation case
//
//    #include "CdrVoiceCall.h"
//    ....
//    {
//        CCdrVoiceCall        moc_record;
//        .....
//        .....    // when call get originated
//        moc_record.setServedIMSI( <pointer-to-IMSI> );
//        moc_record.setServedMSISDN( <pointer-to-MSISDN> );
//        moc_record.setServedIMEI( <pointer-to-IMEI> );
//        moc_record.setCallingPartyNumber( <pointer-to-E164> );
//        moc_record.setCalledNumber( <pointer-to-E164> );
//        moc_record.seisure_NormalOrigination( <trana-id from RIL3 header> )
//        .....
//        .....
//        .....    // when receiving answer (connect_ack) from MS
//        moc_record.answer( <bear-type>, <codec> );
//        // need to start a 10 miniutes timer for long duration call
//        .....
//        .....    // when perfoming a handover to a new cell
//        moc_record.handover( <pointer-to-new-la-id>, <pointer-to-new-cell-id> );
//        .....
//        .....
//        .....    // when long duration timer expires
//        moc_record.generateCallRecord(true);
//        // re-start 10-minute long duration timer 
//        .....
//        .....    // when call is disconnected (from either sides) or aborted duye to error
//        moc_record.release( <cc-cause-value>, <cdr-cause-value> );
//        
//        // at this point, the record is generated, and the moc_record can be safely discard!
//
//    }





#endif // VXWORKS
#endif // _CDR_VOICECALL_H_
