// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieRFLNumberList.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieRFLNumberList.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


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

RlcMacResult IeRFLNumberList::SetRFLNumberList(unsigned char count, unsigned char *list)
{
    if (count <= RLC_MAC_MAX_RF_FREQS)
    {
        numNums = count;
    }
    else
    {
        return (RLC_MAC_PARAMETER_RANGE_ERROR);
    }
    
    for (int i = 0; i < numNums; i++)
    {
        numberList[i] = list[i];
    }
    isValid = TRUE;
    return (RLC_MAC_SUCCESS);
}




RlcMacResult IeRFLNumberList::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeRFLNumberList::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack RFL Number List bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8(numberList[0], 6);
        for (int i = 1; i < numNums; i++)
        {
            dataStream.InsertBits8(1, 1);
            dataStream.InsertBits8(numberList[i], 6); 
        }
        dataStream.InsertBits8(0, 1);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeRFLNumberList::EncodeIe numberList not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}


void IeRFLNumberList::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tRFL Number List\n");
        for (int i = 0; i < numNums; i++)
        {
            outObj->Trace("\t\t%d\n", numberList[i]);
        }
    }
}