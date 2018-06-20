// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMobileIdentity.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieMobileIdentity.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char miNames[][64] =
{
    "No Identity",
    "IMSI",
    "IMEI",
    "IMEISV",
    "TMSI/P-TMSI"
};


// *******************************************************************
// class IeMobileIdentity
//
// Description:
//    Mobile Identity IE -- GSM04.60 11.2.10
//                          GSM04.08 10.1.5.4
// *******************************************************************

RlcMacResult IeMobileIdentity::SetMobileIdentity(MOBILE_IDENTITY_TYPE type, 
    int numDigits, unsigned char *digits)
{
    if (numDigits <= MAX_MOBILE_ID_LEN)
    {
        numMiDigits = numDigits;
    }
    else
    {
        return (RLC_MAC_PARAMETER_RANGE_ERROR);
    }
    
    miType = type;
    
    for (int i = 0; i < numDigits; i++)
    {
        miDigits[i] = digits[i];
    }
    isValid = TRUE;
    return (RLC_MAC_SUCCESS);
}


RlcMacResult IeMobileIdentity::SetMobileIdentity(MOBILE_IDENTITY_TYPE type, 
    unsigned long tmsi)
{    
    miType = type;
    p_tmsi = tmsi;
    isValid = TRUE;
    return (RLC_MAC_SUCCESS);
}




RlcMacResult IeMobileIdentity::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeMobileIdentity::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Mobile Identity bits into the output bit stream.
    if (isValid)
    {
        if (miType == TMSI_PTMSI)
        {
            // For TMSI or PTMSI the length of Mobile Identity octet will be 5.
            dataStream.InsertBits8(5, 4);
            // Bits 5-8 are "1111"
            dataStream.InsertBits8(0xf, 4);
            // Odd/even indicator set to 0 for (p)tmsi
            dataStream.InsertBits8(0, 1);
            
            dataStream.InsertBits8(miType, 3);
            
            dataStream.InsertBits32(p_tmsi, 32);
        }
        else if (miType == NO_IDENTITY)
        {
            // For no identity the length of Mobile Identity octet will be 1.
            dataStream.InsertBits8(1, 4);
            // Bits 5-8 are "1111"
            dataStream.InsertBits8(0xf, 4);
            // Odd/even indicator set to 0 for (p)tmsi
            dataStream.InsertBits8(0, 1);
            
            dataStream.InsertBits8(miType, 3);
        }
        else if (miType == IMEISV)
        {
            // For IMEISV the length of Mobile Identity octet will be 9.
            dataStream.InsertBits8(8, 4);
            // Bits 5-8 are are "1111".
            dataStream.InsertBits8(0xf, 4);
            // Odd/even indicator set to 0 to indicate even number of digits
            dataStream.InsertBits8(0, 1);
            
            dataStream.InsertBits8(miType, 3);
            
            for (int i = 0; i < numMiDigits; i++)
            {
                dataStream.InsertBits8(miDigits[i], 4);
            }
        }
        else
        {
            // For MSI or IMEI the length of Mobile Identity octet will be 8.
            dataStream.InsertBits8(8, 4);
            // Bits 5-8 are the first identity digit.
            dataStream.InsertBits8(miDigits[0], 4);
            // Odd/even indicator set to 1 to indicate odd number of digits
            dataStream.InsertBits8(1, 1);
            
            dataStream.InsertBits8(miType, 3);
            
            for (int i = 1; i < numMiDigits; i++)
            {
                dataStream.InsertBits8(miDigits[i], 4);
            }          
        }
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IeMobileIdentity::EncodeIe arfcnIndex not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}


void IeMobileIdentity::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tMobile Identity\n");
        if (miType == TMSI_PTMSI)
        {
            outObj->Trace("\t\t%s ---> %#x\n", miNames[miType], p_tmsi);
        }
        else if (miType == NO_IDENTITY)
        {
            outObj->Trace("\t\t%s ---> none\n", miNames[miType]);
        }
        else
        {
            char id[MAX_MOBILE_ID_LEN+1];
            int i;
            for (i = 0; i < numMiDigits; i++)
            {
                sprintf(id+i, "%01d", miDigits[i]);
            }
            id[i] = 0;
            outObj->Trace("\t\t%s ---> %s\n", miNames[miType], id);
        }
    }
}