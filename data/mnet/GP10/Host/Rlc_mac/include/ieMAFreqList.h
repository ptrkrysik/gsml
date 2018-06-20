// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMAFreqList.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEMAFREQLIST_H__
#define __IEMAFREQLIST_H__

#include "IeBase.h"
#include "bitstream.h"
#include "RlcMacCommon.h"


// *******************************************************************
// class IeMAFreqList
//
// Description:
//    MA Frequency List IE -- GSM04.60 12.8
// *******************************************************************

#define MAX_MA_FREQ_LIST_LEN    18
#define MA_LIST_MAX_RF_FREQS    RLC_MAC_MAX_RF_FREQS

typedef enum
{
    FREQ_LIST_FORMAT_BIT_MAP_0              = 0,
    FREQ_LIST_FORMAT_1024_RANGE             = 1,
    FREQ_LIST_FORMAT_512_RANGE              = 20,
    FREQ_LIST_FORMAT_256_RANGE              = 21,
    FREQ_LIST_FORMAT_128_RANGE              = 22,
    FREQ_LIST_FORMAT_VARIABLE_BIT_MAP       = 23,
} FREQ_LIST_FORMAT_ID;   

class IeMAFreqList : public IeBase {
public:

    IeMAFreqList() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetMAFreqList(unsigned char numEntries, unsigned short *arfcns);
   
    void GetMAFreqList(unsigned char *numEntries, unsigned short *arfcns)
    {
        *numEntries = numArfcns;
        for (int i = 0; i < numArfcns; i++)
        {
            arfcns[i] = arfcnList[i];
        }
    }
   
    void ClearMAFreqList() { isValid = FALSE; }
   
private:
    unsigned char   numArfcns;
    unsigned short  arfcnList[MA_LIST_MAX_RF_FREQS];
    
    
    FREQ_LIST_FORMAT_ID SelectFrequencyListFormat();
    RlcMacResult EncodeFreqList1024Range(unsigned char &MAListLen, unsigned char *MAList);
    RlcMacResult EncodeFreqList512Range(unsigned char &MAListLen, unsigned char *MAList);
    RlcMacResult EncodeFreqList256Range(unsigned char &MAListLen, unsigned char *MAList);
    RlcMacResult EncodeFreqList128Range(unsigned char &MAListLen, unsigned char *MAList);
    RlcMacResult EncodeFreqListVariableBitMap(unsigned char &MAListLen, unsigned char *MAList);
    RlcMacResult EncodeFreqListBitMap0(unsigned char &MAListLen, unsigned char *MAList);
    int GetGreatestPowerOf2LessorOrEqualTo(int index);
    void EncodeFreqsSubTree(int index, int range,unsigned short *set,
        short numberInSet, unsigned short *W);
    void GetFreqSetFromW(unsigned short origArfcn, int original_range, 
        unsigned short *W, short numberInW, unsigned short *set);
    void SetWbits(int firstNumBits, int arrayLength, unsigned short *Wbits);
    unsigned short ReduceMinArfcn(int numFreq, unsigned short *reducedArfcn);
    void StuffArray(
        int arrayLength, 
        unsigned short *W, 
        unsigned short *Wbits,
        int &bufferLength,
        int startBit,   // 1-8; 1:lsb, 8:msb, msbs before startBit have to be preserved
                        // startBit is advanced to the next available bit before return
        unsigned char *buffer);
    
};

#endif