// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************
// CdrBase.h: interface for the CCdrBase class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _CDR_BASE_H_
#define _CDR_BASE_H_

#include <time.h>

#include "CdrInterface.h"

#include "subsc/SubscInfoType.h"	// in common vob - klim 11/09/00
#include "ril3/ril3msgheader.h"
#include "ril3/ril3_common.h"
#include "ril3/ie_cause.h"
#include "ril3/ie_calling_bcd.h"
#include "ril3/ie_called_bcd.h"


#define CNI_RIL3_CAUSE_SUCCESS      ((T_CNI_RIL3_CAUSE_VALUE)255)
#define CDR_CURRENT_VERSION_NUM     2

typedef ubyte4 T_CDR_EVENT_REFERENCE_NUMBER;


// CDR data types
typedef enum
{
    CDR_REC_NORM_RELEASE                     = 0,
    CDR_REC_PARTIAL                          = 1,
    CDR_REC_PARTIAL_REC_CALL_REESTABLISHMENT = 2,
    CDR_REC_UNSUCCESSFUL_CALL_ATTEMPT        = 3,
    CDR_REC_STABLE_CALL_ABNORMAL_TERM        = 4,
    CDR_REC_CAMEL_INIT_CALL_RELEASE          = 5

} CdrRecCauseTerm_t;


typedef enum {
    CDR_RECORD_TYPE_MOC          =0 ,
    CDR_RECORD_TYPE_MTC          =1 ,
    CDR_RECORD_TYPE_MOSMS        =6 ,
    CDR_RECORD_TYPE_MTSMS        =7 ,
    CDR_RECORD_TYPE_SSA          =10,
    CDR_RECORD_TYPE_CTR          =11,
    CDR_RECORD_TYPE_INVALID      =99,
} T_CDR_RECORD_TYPE;


typedef enum {
    CDR_FIELD_NOT_SET            = 0,    // the field has not being set at all
    CDR_FIELD_SET_AND_PRINTED    = 1,    // the field has been set and has been printed already
    CDR_FIELD_SET_BUT_NOT_PRINT  = 2     // the field has been set but never been printed yet.
} T_CDR_FIELD_STATUS;


// CDR Field E164
class CCdrFieldE164
{
public:
    T_CDR_FIELD_STATUS status    ;
    char               buffer[23];        // pre-formated ascii string

    void               initialize   () { buffer[0]='\0'; status=CDR_FIELD_NOT_SET;};
                       CCdrFieldE164() { initialize(); };
    virtual           ~CCdrFieldE164() {};
    char             * string       ();
    void               set          (T_SUBSC_IE_ISDN *p_e164);
    void               set          (T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  *);
    void               set          (T_CNI_RIL3_IE_CALLING_PARTY_BCD_NUMBER *);
};

// CDR Field IMSI
class CCdrFieldIMSI
{
public:
    T_CDR_FIELD_STATUS status    ;
    char               buffer[16];        // pre-formated ascii string

    void               initialize   () { buffer[0]='\0'; status=CDR_FIELD_NOT_SET;};
                       CCdrFieldIMSI() { initialize(); };
    virtual           ~CCdrFieldIMSI() {};
    void               set          (T_SUBSC_IE_MOBILE_ID_IMSI *p_IMSI) ;
    char             * string       ();
};

// CDR Field IMEI
class CCdrFieldIMEI
{
public:
    T_CDR_FIELD_STATUS status    ;
    char               buffer[20];        // pre-formated ascii string

    void               initialize   () { buffer[0]='\0'; status=CDR_FIELD_NOT_SET;};
                       CCdrFieldIMEI() { initialize();};
    virtual           ~CCdrFieldIMEI() {};
    void               set          (T_CNI_RIL3_IE_MOBILE_ID *p_IMEI) ;
    char             * string       ();
};

// CDR Field LACID
class CCdrFieldLACID
{
public:
    T_CDR_FIELD_STATUS status;
    char               buffer[20];        // pre-formated ascii string

    void               initialize    () { buffer[0]='\0'; status=CDR_FIELD_NOT_SET;};
                       CCdrFieldLACID() { initialize();};
    virtual           ~CCdrFieldLACID() {};
    void               set           (T_CNI_RIL3_IE_LOCATION_AREA_ID *p_la_id  ,
                                      T_CNI_RIL3_IE_CELL_ID          *p_cell_id);
    char             * string        ();
};


// CDR FIeld Date and Time
class CCdrFieldDateTime
{
private:
    time_t             lTime     ;      // current system time (not UTC)
    char               buffer[16];      // pre-formated ascii string

public:
    T_CDR_FIELD_STATUS status;

                       CCdrFieldDateTime() { initialize();};
    virtual           ~CCdrFieldDateTime() {};

    void               initialize       () { buffer[0] = '\0'             ;
                                             lTime     = 0                ;
                                             status    = CDR_FIELD_NOT_SET;};
    time_t             getTime          () { return (lTime); }
    void               stamp            ();    // stamp the current date and time into buffer
    char             * string           ();
};


// CDR Base Class
class CCdrBase
{
protected:
    T_CDR_RECORD_TYPE             record_type;
    T_CDR_EVENT_REFERENCE_NUMBER  event_ref_number;
    CCdrFieldIMSI                 served_imsi;
    CCdrFieldE164                 served_msisdn;

public:
    unsigned short                count;
    CdrMessage_t                  cdr_msg;

             CCdrBase();
    virtual ~CCdrBase();

    virtual void      initialize     ();

    void              setRecordType  (T_CDR_RECORD_TYPE type) { cdr_msg.msgType=record_type=type; };
    T_CDR_RECORD_TYPE getRecordType  ()                       { return record_type; };
    void              setServedIMSI  (T_SUBSC_IE_MOBILE_ID_IMSI *p_imsi) 
                                                              { served_imsi.set(p_imsi); };
    void              setServedMSISDN(T_SUBSC_IE_ISDN *p_msisdn)
                                                              { served_msisdn.set(p_msisdn); };
    void              printBaseFields();    // print all the fields into outbuffer
    void              generateRecord ();    // send the record to the CDR client
    T_CDR_EVENT_REFERENCE_NUMBER 
                      getEventRefNumber()                     { return event_ref_number; };

};


// This class should NOT be used directly. This is a base class to derive
// CTR, VoiceCall, SSA, and SMS records

#endif // _CDR_BASE_H_