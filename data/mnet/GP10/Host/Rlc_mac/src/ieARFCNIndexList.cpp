// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieARFCNIndexList.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieARFCNIndexList.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


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

RlcMacResult IeARFCNIndexList::SetARFCNIndexList(unsigned char count, unsigned char *list)
{
    if (count <= RLC_MAC_MAX_RF_FREQS)
    {
        numIndex = count;
    }
    else
    {
        return (RLC_MAC_PARAMETER_RANGE_ERROR);
    }
    
    for (int i = 0; i < numIndex; i++)
    {
        arfcnIndex[i] = list[i];
    }
    isValid = TRUE;
    return (RLC_MAC_SUCCESS);
}




RlcMacResult IeARFCNIndexList::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeARFCNIndexList::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack ARFCN Index List bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(arfcnIndex[0], 6);
        for (int i = 1; i < numIndex; i++)
        {
            dataStream.InsertBits8(1, 1);
            dataStream.InsertBits8(arfcnIndex[i], 6); 
        }
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeARFCNIndexList::EncodeIe arfcnIndex not valid\n");
    }
    dataStream.InsertBits8(0, 1);
   
    DBG_LEAVE();
    return (result);
}


void IeARFCNIndexList::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tARFCN Index List\n");
        for (int i = 0; i < numIndex; i++)
        {
            outObj->Trace("\t\t%d\n", arfcnIndex[i]);
        }
    }
}