// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTimeslotAllocation.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IETIMESLOTALLOCATION_H__
#define __IETIMESLOTALLOCATION_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeTimeslotAllocation
//
// Description:
//    RLC Mode IE -- GSM04.60 12.18
// *******************************************************************


class IeTimeslotAllocation : public IeBase {
public:

    IeTimeslotAllocation() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetTimeslotAllocation(unsigned char alloc)
    {
        tsAllocation = alloc;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetTimeslotAllocation()
    {
        return (tsAllocation);
    }
   
    void ClearTimeslotAllocation() { isValid = FALSE; }
   
private:
    unsigned char tsAllocation;
};

#endif