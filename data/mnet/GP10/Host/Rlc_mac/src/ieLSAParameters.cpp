// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieLSAParameters.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieLSAParameters.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class IeLSAParameters
//
// Description:
//    LSA Parameters IE -- GSM04.60 12.28
//
//  < LSA Parameters IE > ::=
//            < NR_OF_FREQ_OR_CELLS : bit (5) >:
//              < LSA ID information : < LSA ID information struct >> } *(val (NR_OF_FREQ_OR_CELLS)) };
//
//  < LSA ID information struct > ::=
//            { 1 { 0 < LSA_ID : bit (24) >
//                    |1 < ShortLSA_ID : bit (10) >} } ** 0 ;
//
// *******************************************************************

RlcMacResult IeLSAParameters::SetLSAParameters(unsigned char numIds, unsigned int *ids)
{
    if (numIds <= MAX_FREQ_OR_CELLS)
    {
        numFreqOrCells = numIds;
    }
    else
    {
        return (RLC_MAC_PARAMETER_RANGE_ERROR);
    }
    
    for (int i = 0; i < numIds; i++)
    {
        LSAIdInfo.lsaIds[i] = ids[i];
    }
    isValid = TRUE;
    type = LONG;
    return (RLC_MAC_SUCCESS);
}


RlcMacResult IeLSAParameters::SetLSAParameters(unsigned char numIds, unsigned short *shortIds)
{
    if (numIds <= MAX_FREQ_OR_CELLS)
    {
        numFreqOrCells = numIds;
    }
    else
    {
        return (RLC_MAC_PARAMETER_RANGE_ERROR);
    }
    
    for (int i = 0; i < numIds; i++)
    {
        LSAIdInfo.shortLsaIds[i] = shortIds[i];
    }
    isValid = TRUE;
    type = SHORT;
    return (RLC_MAC_SUCCESS);
}


RlcMacResult IeLSAParameters::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeLSAParameters::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack LSA Parameters bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8(numFreqOrCells, 5);
        if (numFreqOrCells > 0)
        {
            dataStream.InsertBits8(1, 1);
            for (int i = 0; i < numFreqOrCells; i++)
            {
                if (type == LONG)
                {
                    dataStream.InsertBits8(0, 1);
                    dataStream.InsertBits32(LSAIdInfo.lsaIds[i], 24);
                }
                else
                {
                    dataStream.InsertBits8(1, 1);
                    dataStream.InsertBits16(LSAIdInfo.shortLsaIds[i], 10);
                }
            }
        }
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeLSAParameters::EncodeIe numFreqOrCells not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}


void IeLSAParameters::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tLSA Parameters\n"
                      "\t\tNumber of frequencies or cells --> %#x\n", numFreqOrCells);
        if (numFreqOrCells > 0)
        {
            for (int i = 0; i < numFreqOrCells; i++)
            {
                if (type == LONG)
                {
                    outObj->Trace("\t\tLSA ID %d ---> %x\n", LSAIdInfo.lsaIds[i]);
                }
                else
                {
                    outObj->Trace("\t\tShort LSA ID %d ---> %x\n", LSAIdInfo.shortLsaIds[i]);
                }
            }
        }
    }
}