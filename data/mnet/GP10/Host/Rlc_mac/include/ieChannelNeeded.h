// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieChannelNeeded.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IECHANNELNEEDED_H__
#define __IECHANNELNEEDED_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeChannelNeeded
//
// Description:
//    Channel Needed IE -- GSM04.60 11.2.10
// *******************************************************************


class IeChannelNeeded : public IeBase {
public:

    typedef enum
    {
        ANY_CHANNEL,
        SDCCH,
        TCH_F_FULL_RATE,
        TCH_H_or_TCH_F_DUAL_MODE
    } CHANNEL_NEEDED;
    
    IeChannelNeeded() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetChannelNeeded(CHANNEL_NEEDED chan)
    {
        chanNeeded = chan;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    CHANNEL_NEEDED GetChannelNeeded()
    {
        return (chanNeeded);
    }
   
    void ClearChannelNeeded() { isValid = FALSE; }
   
private:
    CHANNEL_NEEDED chanNeeded;
};

#endif