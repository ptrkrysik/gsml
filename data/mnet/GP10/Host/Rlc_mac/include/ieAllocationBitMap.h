// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieAllocationBitMap.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEALLOCATIONBITMAP_H__
#define __IEALLOCATIONBITMAP_H__

#include "IeBase.h"
#include "bitstream.h"

#define MAX_ALLOCATION_BITMAP_LEN   128

// *******************************************************************
// class IeAllocationBitMap
//
// Description:
//    Allocation Bit Map IE -- GSM04.60 12.4
//
// *******************************************************************

    
class IeAllocationBitMap : public IeBase {
public:

    IeAllocationBitMap() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetAllocationBitMap(unsigned char len, unsigned char *map);   
    
    void GetAllocationBitMap(unsigned char *len, unsigned char *map)
    {
        *len = allocLength;
        for (int i = 0; i < allocLength; i++)
        {
            map[i] = allocBitMap[i];
        }
    }
   
    void ClearAllocationBitMap() { isValid = FALSE; }
   
private:
    unsigned char           allocLength;
    unsigned char           allocBitMap[MAX_ALLOCATION_BITMAP_LEN>>3];
};

#endif