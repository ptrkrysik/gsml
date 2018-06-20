// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMobileIdentity.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEMOBILEIDENTITY_H__
#define __IEMOBILEIDENTITY_H__

#include "IeBase.h"
#include "bitstream.h"

#define MAX_MOBILE_ID_LEN 16
// *******************************************************************
// class IeMobileIdentity
//
// Description:
//    Mobile Identity IE -- GSM04.60 11.2.10
//                          GSM04.08 10.1.5.4
// *******************************************************************

    
class IeMobileIdentity : public IeBase {
public:

    typedef enum
    {
        NO_IDENTITY,
        IMSI,
        IMEI,
        IMEISV,
        TMSI_PTMSI
    } MOBILE_IDENTITY_TYPE;

    IeMobileIdentity() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetMobileIdentity(MOBILE_IDENTITY_TYPE type, int numDigits, unsigned char *digits);   
    RlcMacResult SetMobileIdentity(MOBILE_IDENTITY_TYPE type, unsigned long tmsi);   
    
    void GetMobileIdentity(MOBILE_IDENTITY_TYPE *type, int *numDigits, unsigned char *digits)
    {
        *type = miType;
        *numDigits = numMiDigits;
        for (int i = 0; i < numMiDigits; i++)
        {
            digits[i] = miDigits[i];
        }
    }
   
    void GetMobileIdentity(MOBILE_IDENTITY_TYPE *type, unsigned long *tmsi)
    {
        *type = miType;
        *tmsi = p_tmsi;
    }
   
    void ClearMobileIdentity() { isValid = FALSE; }
   
private:
    MOBILE_IDENTITY_TYPE miType;
    unsigned long   p_tmsi;
    int             numMiDigits;
    unsigned char   miDigits[MAX_MOBILE_ID_LEN];
};

#endif