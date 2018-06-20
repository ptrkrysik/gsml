// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieARFCNIndexList.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEARFCNINDEXLIST_H__
#define __IEARFCNINDEXLIST_H__

#include "IeBase.h"
#include "bitstream.h"
#include "RlcMacCommon.h"

// *******************************************************************
// class IeARFCNIndexList
//
// Description:
//    ARFCN Index List IE -- GSM04.60 12.10a
//
// < ARFCN index list struct > ::=
//   < ARFCN_INDEX : bit (6) >
//   { 0 | 1 < ARFCN index list struct > } ;
//
// *******************************************************************

    
class IeARFCNIndexList : public IeBase {
public:

    IeARFCNIndexList() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetARFCNIndexList(unsigned char count, unsigned char *list);   
    
    void GetARFCNIndexList(unsigned char *len, unsigned char *list)
    {
        *len = numIndex;
        for (int i = 0; i < numIndex; i++)
        {
            list[i] = arfcnIndex[i];
        }
    }
   
    void ClearARFCNIndexList() { isValid = FALSE; }
   
private:
    unsigned char   numIndex;;
    unsigned char   arfcnIndex[RLC_MAC_MAX_RF_FREQS];
};

#endif