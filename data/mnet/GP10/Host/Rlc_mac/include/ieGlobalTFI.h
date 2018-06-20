// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieGlobalTFI.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEGLOBALTFI_H__
#define __IEGLOBALTFI_H__

#include "IeBase.h"
#include "bitstream.h"

#define MAX_TFI   32

// *******************************************************************
// class IeGlobalTFI
//
// Description:
//    Global TFI IE -- GSM04.60 12.10
//
// < Global TFI IE > ::=
//	 { 0 < UPLINK_TFI : bit (5) >
//	 | 1 < DOWNLINK_TFI : bit (5) > } ;
//
// *******************************************************************

class IeGlobalTFI : public IeBase {
public:

    typedef enum
    {
        UPLINK_TFI,
        DOWNLINK_TFI
    } RLC_MAC_TFI_TYPE;
   
    IeGlobalTFI() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    RlcMacResult DecodeIe(BitStreamIn &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetGlobalTFI(unsigned char tfiIn, RLC_MAC_TFI_TYPE type)
    {
        if (tfiIn <= MAX_TFI)
        {
            globalTFI = tfiIn;
            tfiType = type;
            isValid = TRUE;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    unsigned int GetGlobalTFI()
    {
        return (globalTFI);
    }
   
    RLC_MAC_TFI_TYPE GetGlobalTFIType()
    {
        return (tfiType);
    }
   
    void ClearGlobalTFI() { isValid = FALSE; }
   
private:
    RLC_MAC_TFI_TYPE tfiType;
    unsigned char globalTFI;
};

#endif