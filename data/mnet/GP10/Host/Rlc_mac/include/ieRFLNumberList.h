// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieRFLNumberList.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IERFLNUMBERLIST_H__
#define __IERFLNUMBERLIST_H__

#include "IeBase.h"
#include "bitstream.h"
#include "RlcMacCommon.h"

// *******************************************************************
// class IeRFLNumberList
//
// Description:
//    RFL Number List IE -- GSM04.60 12.10a
//
// < RFL number list struct > ::=
//	 < RFL_NUMBER : bit (4) >
//	 { 0 | 1 < RFL number list struct > } ;
//
// *******************************************************************

    
class IeRFLNumberList : public IeBase {
public:

    IeRFLNumberList() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetRFLNumberList(unsigned char count, unsigned char *list);   
    
    void GetRFLNumberList(unsigned char *count, unsigned char *list)
    {
        *count = numNums;
        for (int i = 0; i < numNums; i++)
        {
            list[i] = numberList[i];
        }
    }
   
    void ClearRFLNumberList() { isValid = FALSE; }
   
private:
    unsigned char   numNums;
    unsigned char   numberList[RLC_MAC_MAX_RF_FREQS];
};

#endif