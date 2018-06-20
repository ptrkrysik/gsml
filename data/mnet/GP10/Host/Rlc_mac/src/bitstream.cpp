// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : bitstream.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#include "bitstream.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class BitStreamOut
//
// Description:
//
// *******************************************************************

// Pack up to 8 bits of data.
RlcMacResult BitStreamOut::InsertBits8(unsigned char bits, int numBits)
{
    RlcMacResult result = RLC_MAC_SUCCESS;
   
    if (numBits <= 8)
    {
        // Figure out how much shifting is needed to get the significant
        // bits in the byte starting at the current bit pointer.
        int bitShift = (8 - (bitPointer % 8)) - numBits;
      
        // If bitShift is positive then there is all the bits are
        // in the current byte.
        if (bitShift >= 0)
        {
            dataStream[streamLen] |= (bits << bitShift);
        }
        // Some of the bits needed are in the next byte.
        else
        {
            dataStream[streamLen] |= (bits >> (-bitShift));
            dataStream[streamLen+1] |= (bits << (8 + bitShift)); 
        }
      
        bitPointer += numBits;
        streamLen = bitPointer >> 3;
    }
    else
    {
        result = RLC_MAC_INVALID_NUM_BITS;
    }
   
    return (result);
}


// Pack up to 16 bits of data.
RlcMacResult BitStreamOut::InsertBits16(unsigned short bits, int numBits)
{
    RlcMacResult result = RLC_MAC_SUCCESS;
   
    if (numBits <= 16)
    {
        while (numBits)
        {
            int currNumBits = (numBits % 8) ? numBits % 8 : 8;
            result = InsertBits8((unsigned char)(bits >> (numBits - currNumBits)), 
                currNumBits); 
            numBits -= currNumBits;
        }
    }
    else
    {
        result = RLC_MAC_INVALID_NUM_BITS;
    }
   
    return (result);
}


RlcMacResult BitStreamOut::InsertBits32(unsigned long bits, int numBits)
{
    RlcMacResult result = RLC_MAC_SUCCESS;
   
    if (numBits <= 32)
    {
        while (numBits)
        {
            int currNumBits = (numBits % 8) ? numBits % 8 : 8;
            result = InsertBits8((unsigned char)(bits >> (numBits - currNumBits)), 
                currNumBits); 
            numBits -= currNumBits;
        }
    }
    else
    {
        result = RLC_MAC_INVALID_NUM_BITS;
    }
   
    return (result);
}


RlcMacResult BitStreamOut::InsertBytesN(unsigned char *bytes, int numBytes)
{
    RlcMacResult result = RLC_MAC_SUCCESS;
   
    for (int i = 0; i < numBytes; i++)
    {
        result = InsertBits8(bytes[i], 8); 
    }
   
    return (result);
}



void BitStreamOut::PadToOctetBoundary()
{
    dataStream[streamLen] |= ((0xff >> (bitPointer % 8)) & 0x2b);
    bitPointer += (bitPointer % 8) ? (8 - (bitPointer % 8)) : 0;
    streamLen = bitPointer >> 3;
}


void BitStreamOut::PadWithSparePadding(int len)
{
    // Make sure the stream is on an octet boundary and then pad with 0x2b
    PadToOctetBoundary();
    while (streamLen < len)
    {
        dataStream[streamLen++] = 0x2b;
        bitPointer += 8;
    };
}


// *******************************************************************
// class BitStreamIn
//
// Description:
//
// *******************************************************************



// Extract up to 8 bits of data.
RlcMacResult BitStreamIn::ExtractBits8(unsigned char &bits, int numBits)
{
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    if ((bitPointer + numBits) >> 3 > streamLen)
    {
        return (RLC_MAC_ERROR);
    }
   
    if (numBits <= 8)
    {
        // First zero out the return value
        bits = 0;
        
        // Figure out how much shifting is needed to get the significant
        // bits in the byte starting at the current bit pointer.
        int bitShift = (8 - (bitPointer % 8)) - numBits;
      
        // If bitShift is positive then all the bits are extracted from
        // the current byte.
        if (bitShift >= 0)
        {
            bits |= ((dataStream[currByte] >> bitShift) & (0xff >> (8 - numBits)));
        }
        // Some of the bits need to come from the next byte.
        else
        {
            bits |= (dataStream[currByte] << (-bitShift));
            bits |= (dataStream[currByte+1] >> (8 + bitShift));
            bits &= (0xff >> (8 - numBits)); 
        }
      
        bitPointer += numBits;
        currByte = bitPointer >> 3;
    }
    else
    {
        result = RLC_MAC_INVALID_NUM_BITS;
    }
   
    return (result);
}


// Extract up to 16 bits of data.
RlcMacResult BitStreamIn::ExtractBits16(unsigned short &bits, int numBits)
{
    RlcMacResult result = RLC_MAC_SUCCESS;
   
    if ((bitPointer + numBits) >> 3 > streamLen)
    {
        return (RLC_MAC_ERROR);
    }
   
    if (numBits <= 16)
    {
        bits = 0;
        while (numBits)
        {
            int currNumBits = (numBits % 8) ? numBits % 8 : 8;
            unsigned char aByte;
            result = ExtractBits8(aByte, currNumBits); 
            bits |= ((unsigned short)aByte) << (8 * ((numBits-1) / 8));
            numBits -= currNumBits;
        }
    }
    else
    {
        result = RLC_MAC_INVALID_NUM_BITS;
    }
   
    return (result);
}


// Extract up to 32 bits of data.
RlcMacResult BitStreamIn::ExtractBits32(unsigned long &bits, int numBits)
{
    RlcMacResult result = RLC_MAC_SUCCESS;
   
    if ((bitPointer + numBits) >> 3 > streamLen)
    {
        return (RLC_MAC_ERROR);
    }
   
    if (numBits <= 32)
    {
        bits = 0;
        while (numBits)
        {
            int currNumBits = (numBits % 8) ? numBits % 8 : 8;
            unsigned char aByte;
            result = ExtractBits8(aByte, currNumBits); 
            bits |= ((unsigned long)aByte) << (8 * ((numBits-1) / 8));
            numBits -= currNumBits;
        }
    }
    else
    {
        result = RLC_MAC_INVALID_NUM_BITS;
    }
   
    return (result);
}


RlcMacResult BitStreamIn::ExtractBytesN(unsigned char *bytes, int numBytes)
{
    RlcMacResult result = RLC_MAC_SUCCESS;
   
    if ((bitPointer + numBytes*8) >> 3 > streamLen)
    {
        return (RLC_MAC_ERROR);
    }
   
    for (int i = 0; i < numBytes; i++)
    {
        result = ExtractBits8(bytes[i], 8); 
    }
   
    return (result);
}
