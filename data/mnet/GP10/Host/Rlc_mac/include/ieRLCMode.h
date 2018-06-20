// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieRLCMode.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IERLCMODE_H__
#define __IERLCMODE_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeRlcMode
//
// Description:
//    RLC Mode IE -- GSM04.60 11.2.7
// *******************************************************************


class IeRlcMode : public IeBase {
public:

    typedef enum
    {
        RLC_ACKNOWLEDGED,
        RLC_UNACKNOWLEDGED
    } RLC_MODE;
    
    IeRlcMode() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    RlcMacResult DecodeIe(BitStreamIn &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetRlcMode(RLC_MODE mode)
    {
        rlcMode = mode;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    RLC_MODE GetRlcMode()
    {
        return (rlcMode);
    }
   
    void ClearRlcMode() { isValid = FALSE; }
   
private:
    RLC_MODE rlcMode;
};

#endif