// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieRLCBlocksGranted.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IERLCBLOCKSGRANTED_H__
#define __IERLCBLOCKSGRANTED_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeRLCBlocksGranted
//
// Description:
//    RLC Blocks Granted IE -- GSM04.60 11.2.29
//
// *******************************************************************

class IeRLCBlocksGranted : public IeBase {
public:

   
    IeRLCBlocksGranted() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetRLCBlocksGranted(unsigned char val)
    {
        rlcBlocksGranted = val;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetRLCBlocksGranted()
    {
        return (rlcBlocksGranted);
    }
   
    void ClearRLCBlocksGranted() { isValid = FALSE; }
   
private:
    unsigned char rlcBlocksGranted;
};

#endif