// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieBSIC.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEBSIC_H__
#define __IEBSIC_H__

#include "IeBase.h"
#include "bitstream.h"


// *******************************************************************
// class IeBSIC
//
// Description:
//    BSIC IE -- GSM04.60 11.2.4
// *******************************************************************

#define MAX_BSIC   63

class IeBSIC : public IeBase {
public:

    IeBSIC() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    RlcMacResult DecodeIe(BitStreamIn &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetBSIC(unsigned char bsicVal)
    {
        if (bsicVal <= MAX_BSIC)
        {
            bsic = bsicVal;
            isValid = TRUE;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    unsigned char GetBSIC()
    {
        return(bsic);
    }
   
    void ClearBSIC() { isValid = FALSE; }
   
private:
    unsigned char bsic;
};

#endif