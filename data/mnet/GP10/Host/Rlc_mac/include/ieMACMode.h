// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMACMode.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEMACMODE_H__
#define __IEMACMODE_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeMacMode
//
// Description:
//    Mac Mode IE -- GSM04.60 11.2.7
// *******************************************************************

class IeMacMode : public IeBase {
public:

    typedef enum
    {
        DYNAMIC_ALLOCATION,
        EXTENDED_DYNAMIC_ALLOCATION,
        FIXED_ALLOCATION_NO_HALF_DUPLEX,
        FIXED_ALLOCATION_HALF_DUPLEX
    } MAC_MODE;
   
    IeMacMode() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetMacMode(MAC_MODE mode)
    {
        macMode = mode;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    MAC_MODE GetMacMode()
    {
        return(macMode);
    }
   
    void ClearMacMode() { isValid = FALSE; }
private:
    MAC_MODE macMode;
};



#endif