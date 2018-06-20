// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMABitMap.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEMABITMAP_H__
#define __IEMABITMAP_H__

#include "IeBase.h"
#include "bitstream.h"
#include "RlcMacCommon.h"

// *******************************************************************
// class IeMABitMap
//
// Description:
//    MA Bit Map IE -- GSM04.60 12.10a
//
// *******************************************************************

    
class IeMABitMap : public IeBase {
public:

    IeMABitMap() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetMABitMap(unsigned char len, unsigned char *map);   
    
    void GetMABitMap(unsigned char *len, unsigned char *map)
    {
        *len = maLength;
        for (int i = 0; i < ((maLength+1)>>3); i++)
        {
            map[i] = maBitMap[i];
        }
    }
   
    void ClearMABitMap() { isValid = FALSE; }
   
private:
    unsigned char           maLength;
    unsigned char           maBitMap[RLC_MAC_MAX_RF_FREQS>>3];
};

#endif