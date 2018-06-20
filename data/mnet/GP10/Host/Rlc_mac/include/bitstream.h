// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : bitstream.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <string.h>
#include "RlcMacResult.h"
#include "msgL1.h"

#define MAX_BITSTREAM      MAX_RLC_DATA_MSG

// *******************************************************************
// class BitStreamOut
//
// Description:
//
// *******************************************************************

class BitStreamOut {
public:
    BitStreamOut() 
    { 
        bitPointer = 0; 
        streamLen = 0;
        bzero((char *)dataStream, MAX_BITSTREAM);
    }
   
    ~BitStreamOut(){}
   
    RlcMacResult InsertBits8(unsigned char bits, int numBits);
    RlcMacResult InsertBits16(unsigned short bits, int numBits);
    RlcMacResult InsertBits32(unsigned long bits, int numBits);
    RlcMacResult InsertBytesN(unsigned char *bytes, int numBytes);
   
    void PadToOctetBoundary();
    
    void PadWithSparePadding(int len);
   
    unsigned char *GetBitStream() { return (dataStream); }
    void ClearBitStream() 
    { 
        bzero((char *)dataStream, MAX_BITSTREAM); 
        bitPointer = 0;
        streamLen = 0;
    }
    void ResetBitPointer() { bitPointer = 0; streamLen = 0; }
    int GetStreamLen() { return (streamLen); }
 
private:
    unsigned char dataStream[MAX_BITSTREAM];
    int bitPointer;
    int streamLen;
};


// *******************************************************************
// class BitStreamIn
//
// Description:
//
// *******************************************************************

class BitStreamIn {
public:
    BitStreamIn(unsigned char *data, int dataLen) 
    { 
        bitPointer = 0; 
        streamLen = dataLen;
        currByte = 0;
        for (int i = 0; i < dataLen; i++)
        {
            dataStream[i] = data[i];
        }
    }
   
    BitStreamIn()
    {
        bitPointer = 0; 
        streamLen = 0;
        currByte = 0;
    }
   
    ~BitStreamIn(){}
   
    void InitBitStreamIn(unsigned char *data, int dataLen) 
    { 
        bitPointer = 0; 
        streamLen = dataLen;
        currByte = 0;
        for (int i = 0; i < dataLen; i++)
        {
            dataStream[i] = data[i];
        }
    }
   
    RlcMacResult ExtractBits8(unsigned char &bits, int numBits);
    RlcMacResult ExtractBits16(unsigned short &bits, int numBits);
    RlcMacResult ExtractBits32(unsigned long &bits, int numBits);
    RlcMacResult ExtractBytesN(unsigned char *bytes, int numBytes);
   
    unsigned char *GetBitStream() { return (dataStream); }
    int GetCurrentPosition() { return (currByte); }
    void ResetBitPointer() { bitPointer = 0; currByte = 0; }
    int GetStreamLen() { return (streamLen); }
 
private:
    unsigned char dataStream[MAX_BITSTREAM];
    int bitPointer;
    int streamLen;
    int currByte;
    
};

#endif
