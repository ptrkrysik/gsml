// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************
// CdrBase.cpp: implementation of the CCdrBase class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "CdrBase.h"

bool    CDR_Enable_Debug = false;


// CDR Field E164

void CCdrFieldE164::set(T_SUBSC_IE_ISDN *p_e164)
{
    if (p_e164->num_digit)
    {
        p_e164->digits[ (p_e164->num_digit>MAX_NUM_SUBSC_ISDN)? MAX_NUM_SUBSC_ISDN : p_e164->num_digit]=0;
        sprintf (buffer, "%02X%s", (p_e164->num_plan[0] & 0x7F), p_e164->digits);
        status = CDR_FIELD_SET_BUT_NOT_PRINT;
    }
}

void CCdrFieldE164::set (T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER  *pNum)
{
  unsigned offset       ,
           index     = 0,
           numDigits = (pNum->numDigits > CNI_RIL3_MAX_BCD_DIGITS) 
                     ? CNI_RIL3_MAX_BCD_DIGITS
                     : pNum->numDigits                           ;

    // correction for zero lenght number
    if (pNum->numDigits)
    {
        offset = sprintf (buffer, "%1X%1X", pNum->numberType, pNum->numberingPlan);

        // print BCD digits
        while (index < numDigits)
        {
            *(buffer + offset + index) = 0x30 + pNum->digits [index++];
        }

        *(buffer + offset + index) = '\0';

        status=CDR_FIELD_SET_BUT_NOT_PRINT;
    }
}

void CCdrFieldE164::set (T_CNI_RIL3_IE_CALLING_PARTY_BCD_NUMBER *pNum)
{
    T_CNI_RIL3_IE_CALLED_PARTY_BCD_NUMBER shortForm;

    shortForm.ie_present    = pNum->ie_present   ;
    shortForm.numberType    = pNum->numberType   ;
    shortForm.numberingPlan = pNum->numberingPlan;
    shortForm.numDigits     = pNum->numDigits    ;
    memcpy (shortForm.digits, pNum->digits, CNI_RIL3_MAX_BCD_DIGITS);

    set (&shortForm);
}


char * CCdrFieldE164::string()
{
    status=CDR_FIELD_SET_AND_PRINTED;
    return buffer;
};


// CDR Field IMSI

void CCdrFieldIMSI::set(T_SUBSC_IE_MOBILE_ID_IMSI *p_IMSI)
{
    p_IMSI->imsi[ (p_IMSI->num_digit>MAX_NUM_SUBSC_IMSI) ? MAX_NUM_SUBSC_ISDN : p_IMSI->num_digit ]=0;
    sprintf(buffer, "%s", p_IMSI->imsi);
    status=CDR_FIELD_SET_BUT_NOT_PRINT;
};

char * CCdrFieldIMSI::string()
{
    status=CDR_FIELD_SET_AND_PRINTED;
    return buffer;
};

void CCdrFieldIMEI::set (T_CNI_RIL3_IE_MOBILE_ID *pIMEI)
{
  unsigned index = 0;

    // print BCD digits
    while (index < 15)
    {
        buffer [index] = 0x30 + pIMEI->digits [index++];
    }

    buffer[index] = '\0';

    status = CDR_FIELD_SET_BUT_NOT_PRINT;
}


char * CCdrFieldIMEI::string()
{
    status = CDR_FIELD_SET_AND_PRINTED;
    return (buffer);
}

// CDR Field LACID

void CCdrFieldLACID::set(T_CNI_RIL3_IE_LOCATION_AREA_ID *p_la_id, T_CNI_RIL3_IE_CELL_ID *p_cell_id)
{
    sprintf (buffer, "%1u%1u%1u%1u%1u%1x%04X%04X",
             p_la_id->mcc[0], p_la_id->mcc[1], p_la_id->mcc[2],
             p_la_id->mnc[0], p_la_id->mnc[1], ((unsigned)p_la_id->mnc[2] > 9) ? 0xF : p_la_id->mnc[2],
             (unsigned short)p_la_id->lac    , (unsigned short)p_cell_id->value);

    status = CDR_FIELD_SET_BUT_NOT_PRINT;
};

char * CCdrFieldLACID::string()
{
    status = CDR_FIELD_SET_AND_PRINTED;
    return buffer;
};


// CDR Field Date and Time

void CCdrFieldDateTime::stamp()
{
  // The following time function comes from standard ANSI library. 
  // Therefore, the body of the code, including type/structure definition are
  // the same for both Windows NT and VxWorks environment. No conditional
  // compilation is needed between the two operating systems.

  struct tm *gmt;

    time (&lTime);          // get local time
    gmt = gmtime (&lTime);  // convert to UTC time

    // the final format is UTC time, YYMMDDhhmmss
    sprintf(buffer,"%02d%02d%02d%02d%02d%02d",
            gmt->tm_year % 100,     // year since 1900
            gmt->tm_mon  + 1  ,     // month since Jan 0..11
            gmt->tm_mday      ,     // day of the month 1..31
            gmt->tm_hour      ,     // hours after midnight 0..23
            gmt->tm_min       ,     // minutes after hour 0..59
            gmt->tm_sec             // seconds after minutes 0..59
            );

    status = CDR_FIELD_SET_BUT_NOT_PRINT;
}


char * CCdrFieldDateTime::string()
{
    status=CDR_FIELD_SET_AND_PRINTED;
    return buffer;
};

// CDR Base class
void CCdrBase::initialize()
{
    count              =0   ;
    cdr_msg.record[0]  ='\0';
    record_type        = CDR_RECORD_TYPE_INVALID;
    event_ref_number   = cdrAllocateRef ()      ;

    served_imsi.initialize         ();
    served_msisdn.initialize       ();
}


CCdrBase::CCdrBase()
{
    initialize();
}

CCdrBase::~CCdrBase()
{
}

void CCdrBase::printBaseFields()
{
    // print all the fields of this class to the outbuffer
    // 
    // A2=IP-address field of the recording entity
    // A3=Event Reference Number
    // A4=Served IMSI
    // A5=Served MSISDN
    // A6=CDR Format Version (current = 2)
    // A7=Record Type
    count+=sprintf(&cdr_msg.record[count],"A2=%08X", cdrGetLocalIp());
    count+=sprintf(&cdr_msg.record[count],"%cA3=%d",CDR_FIELD_DELIMETER,event_ref_number);
    if(served_imsi.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count+=sprintf(&cdr_msg.record[count],"%cA4=%s",CDR_FIELD_DELIMETER,served_imsi.string());
    if(served_msisdn.status==CDR_FIELD_SET_BUT_NOT_PRINT)
        count+=sprintf(&cdr_msg.record[count],"%cA5=%s",CDR_FIELD_DELIMETER,served_msisdn.string());
    count+=sprintf(&cdr_msg.record[count],"%cA6=%u",CDR_FIELD_DELIMETER, CDR_CURRENT_VERSION_NUM);
    count+=sprintf(&cdr_msg.record[count],"%cA7=%d",CDR_FIELD_DELIMETER,(unsigned long)record_type);
}


void CCdrBase::generateRecord()
{
    // print to stdout under debug control flag
    if(CDR_Enable_Debug) printf("%s\n", cdr_msg.record);

    // send the record to the CDR client
    cdrSendLog(&cdr_msg);

    // reset the output buffer to its initial state
    count=0;
    cdr_msg.record[0]='\0';
}


