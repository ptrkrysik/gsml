// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MacHeader.cpp
// Author(s)   : Tim Olson
// Create Date : 11/22/2000
// Description : 
//
// *******************************************************************

#include "MacHeader.h"

// Static logger object used to display RLC/MAC header contents.
DbgOutput ULMacHeader::HdrDisplayDetailObj("RLC/MAC Msg Detail", RLC_MAC_MSG_DETAIL);
DbgOutput DLMacHeader::HdrDisplayDetailObj("RLC/MAC Msg Detail", RLC_MAC_MSG_DETAIL);


char payloadTypeName[][64] =
{
    "RLC Data Block",
    "RLC Control Block - No optional octets",
    "RLC Control Block with optional octets",
    "Reserved"
};

char rrbpName[][64] =
{
    "TDMA frame number = (N+13) mod 2715648",
    "TDMA frame number = (N+17 or N+18) mod 2715648",
    "TDMA frame number = (N+21 or N+22) mod 2715648 ",
    "TDMA frame number = (N+26) mod 2715648"
};

char spName[][64] =
{
    "RRBP field not valid",
    "RRBP field valid"
};

char finalSegName[][64] =
{
    "Not final segment",
    "Final segment"
};

char addressControlName[][64] =
{
    "TFI/D octet is not present",
    "TFI/D octet is present"
};

char powerReductionName[][64] =
{
    "0-2 dB (modeA) or 0-6 db (modeB) less than BCCH level",
    "4-6 dB (modeA) or 8-14 db (modeB) less than BCCH level",
    "8-10 dB (modeA) or 16-22 db (modeB) less than BCCH level",
    "Not Usable (modeA) or 24-30 db (modeB) less than BCCH level"
};

char directionBitName[][64] =
{
    "Uplink TBF",
    "Downlink TBF"
};

char fbiName[][64] =
{
    "Not the last RLC block",
    "Last RLC block"
};

char extensionName[][64] =
{
    "Extension octet follows",
    "No extension octet"
};

char moreName[][64] =
{
    "No more LLC data",
    "More LLC data"
};

char siName[][64] =
{
    "MS RLC transmit window is NOT stalled",
    "MS RLC trnsmit window is stalled"
};

char retryName[][64] =
{
    "MS sent channel request once",
    "MS sent channel request twice or more"
};

char tlliIndName[][64] =
{
    "TLLI field is not present",
    "TLLI filed is present"
};



// *******************************************************************
// class DLMacHeader
//
// Description:
//    Downlink MAC/RLC header
// *******************************************************************

RlcMacResult DLMacHeader::EncodeDLMacHeader(BitStreamOut &dataStream)
{
    DBG_FUNC("MacMode::EncodeDLMacHeader", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack payload bits into the output bit stream.
    dataStream.InsertBits8((unsigned char)payloadType, 2);

    // Pack RRBP bits into the output bit stream.
    dataStream.InsertBits8((unsigned char)rrbp, 2);
            
    // Pack S/P bit into the output bit stream.
    dataStream.InsertBits8((unsigned char)spBit, 1);
            
    // Pack USF bits into the output bit stream.
    dataStream.InsertBits8(usf, 3);

    switch (payloadType)
    {
        case RLC_DATA_BLOCK:
        {
            // Pack PR bits into the output bit stream.
            dataStream.InsertBits8((unsigned char)powerReduction, 2);
                
            // Pack TFI bits into the output bit stream.
            dataStream.InsertBits8(tfi, 5);
            
            // Pack FBI bit into the output bit stream.
            dataStream.InsertBits8((unsigned char)fbiBit, 1);
            
            // Pack BSN bits into the output bit stream.
            dataStream.InsertBits8(bsn, 7);
            
            // Pack Extension bit into the output bit stream.
            dataStream.InsertBits8((unsigned char)extensionBit, 1);
            
            if (extensionBit == EXTENSION_OCTET_FOLLOWS)
            {
                int i = 0;
                do
                {      
                    // Pack length bits into the output bit stream.
                    dataStream.InsertBits8(LengthOctets[i].length, 6);
                    
                    // Pack More bit into the output bit stream.
                    dataStream.InsertBits8((unsigned char)LengthOctets[i].moreBit, 1);
            
                    // Pack Extension bit into the output bit stream.
                    dataStream.InsertBits8((unsigned char)LengthOctets[i].extensionBit, 1);
                    
                } while (LengthOctets[i++].extensionBit == EXTENSION_OCTET_FOLLOWS); 
            }
        }
        break;
        
        case RLC_CTRL_BLOCK_OPT_OCTETS:
        {
            // Pack RBSN bit into the output bit stream.
            dataStream.InsertBits8((unsigned char)rbsn, 1);
            
            // Pack RTI bits into the output bit stream.
            dataStream.InsertBits8(rti, 5);
            
            // Pack FS bit into the output bit stream.
            dataStream.InsertBits8((unsigned char)finalSegBit, 1);
            
            // Pack AC bit into the output bit stream.
            dataStream.InsertBits8((unsigned char)acBit, 1);
            
            if (acBit == TFI_D_PRESENT)
            {
                // Pack PR bits into the output bit stream.
                dataStream.InsertBits8((unsigned char)powerReduction, 2);
                
                // Pack TFI bits into the output bit stream.
                dataStream.InsertBits8(tfi, 5);
                
                // Pack D bit into the output bit stream.
                dataStream.InsertBits8((unsigned char)directionBit, 1);
            }
        }
        break;
    }
    
    DBG_LEAVE();
    return (result);
}


void DLMacHeader::DisplayDetails()
{
    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        HdrDisplayDetailObj.Trace("MAC Header\n");
        
        // Display Payload Type
        HdrDisplayDetailObj.Trace("\tPayload Type ---> %s\n", payloadTypeName[payloadType]);
    
        // Display RRBP bits .
        HdrDisplayDetailObj.Trace("\tRRBP ---> %s\n", rrbpName[rrbp]);
            
        // Display S/P bit.
        HdrDisplayDetailObj.Trace("\tS/P Bit ---> %s\n", spName[spBit]);
            
        // Display USF bits.
        HdrDisplayDetailObj.Trace("\tUSF ---> %d\n", usf);
    
        switch (payloadType)
        {
            case RLC_DATA_BLOCK:
            {
                // Display PR bits.
                HdrDisplayDetailObj.Trace("\tPR ---> %s\n", powerReductionName[powerReduction]);
                
                // Display TFI bits.
                HdrDisplayDetailObj.Trace("\tTFI ---> %d\n", tfi);
            
                // Display FBI bit.
                HdrDisplayDetailObj.Trace("\tFBI ---> %s\n", fbiName[fbiBit]);
            
                // Display BSN bits.
                HdrDisplayDetailObj.Trace("\tBSN ---> %d\n", bsn);
            
                // Display Extension.
                HdrDisplayDetailObj.Trace("\tExtension ---> %s\n", extensionName[extensionBit]);
            
                if (extensionBit == EXTENSION_OCTET_FOLLOWS)
                {
                    int i = 0;
                    do
                    {      
                        // Display length bits.
                        HdrDisplayDetailObj.Trace("\tlength ---> %d\n", LengthOctets[i].length);
                    
                        // Display More bit.
                        HdrDisplayDetailObj.Trace("\tMore ---> %s\n", moreName[LengthOctets[i].moreBit]);
            
                        // Display Extension bit.
                        HdrDisplayDetailObj.Trace("\tExtension ---> %s\n", 
                            extensionName[LengthOctets[i].extensionBit]);
                    
                    } while (LengthOctets[i++].extensionBit == EXTENSION_OCTET_FOLLOWS); 
                }
            }
            break;
        
            case RLC_CTRL_BLOCK_OPT_OCTETS:
            {
                // Display RBSN bit.
                HdrDisplayDetailObj.Trace("\tRBSN ---> %d\n", rbsn);
            
                // Display RTI bits.
                HdrDisplayDetailObj.Trace("\tRTI ---> %d\n", rti);
            
                // Display FS bit.
                HdrDisplayDetailObj.Trace("\tFS ---> %s\n", finalSegName[finalSegBit]);
            
                // Display AC bit.
                HdrDisplayDetailObj.Trace("\tAC ---> %s\n", addressControlName[acBit]);
            
                if (acBit == TFI_D_PRESENT)
                {
                    // Display PR bits.
                    HdrDisplayDetailObj.Trace("\tPR ---> %s\n", powerReductionName[powerReduction]);
                
                    // Display TFI bits.
                    HdrDisplayDetailObj.Trace("\tTFI ---> %d\n", tfi);
                
                    // Display D bit.
                    HdrDisplayDetailObj.Trace("\tD ---> %s\n", directionBitName[directionBit]);
                }
            }
            break;
        }
    }
}



// *******************************************************************
// class ULMacHeader
//
// Description:
//    Uplink MAC/RLC header
// *******************************************************************

RlcMacResult ULMacHeader::DecodeULMacHeader(BitStreamIn &dataStream)
{
    DBG_FUNC("GlobalTFI::DecodeULMacHeader", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;

    // Extract Payload bits from the input bit stream.
    result = dataStream.ExtractBits8(val, 2);
    RLC_MAC_RESULT_CHECK(result);
    payloadType = (PAYLOAD_TYPE)val;
    
    switch (payloadType)
    {
        case RLC_CTRL_BLOCK_NO_OPT_OCTETS:
        {
            // Extract spare bits from the input bit stream.
            result = dataStream.ExtractBits8(val, 5);
            RLC_MAC_RESULT_CHECK(result);
            
            // Extract Retry bit from the input bit stream.
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            retryBit = (RETRY_BIT)val;
             
        }
        break;
        case RLC_DATA_BLOCK:
        {
            // Extract countdown value bits from the input bit stream.
            result = dataStream.ExtractBits8(countdownVal, 4);
            RLC_MAC_RESULT_CHECK(result);
        
            // Extract Stall Indicator bit from the input bit stream.
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            stallInd = (STALL_INDICATOR)val;
            
            // Extract Retry bit from the input bit stream.
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            retryBit = (RETRY_BIT)val;
            
            // Extract spare bits from the input bit stream.
            result = dataStream.ExtractBits8(val, 2);
            RLC_MAC_RESULT_CHECK(result);
            
            // Extract TFI bits from the input bit stream.
            result = dataStream.ExtractBits8(tfi, 5);
            RLC_MAC_RESULT_CHECK(result);
            
            // Extract TI bit from the input bit stream.
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            tlliInd = (TLLI_INDICATOR)val;
            
            // Extract BSN bits from the input bit stream.
            result = dataStream.ExtractBits8(bsn, 7);
            RLC_MAC_RESULT_CHECK(result);
            
            // Extract Extension bit from the input bit stream.
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            extensionBit = (EXTENSION_BIT)val;
            
            // Build up the list of extension octets if included.
            // In either case set the current length octet to be the first.
            currLengthOctet = 0;
            if (extensionBit == EXTENSION_OCTET_FOLLOWS)
            {
                int i = 0;
                do 
                {
                    // Extract length bits from the input bit stream.
                    result = dataStream.ExtractBits8(LengthOctets[i].length, 6);
                    RLC_MAC_RESULT_CHECK(result);
                
                    // Extract More bit from the input bit stream.
                    result = dataStream.ExtractBits8(val, 1);
                    RLC_MAC_RESULT_CHECK(result);
                    LengthOctets[i].moreBit = (MORE_BIT)val;
                
                    // Extract Extension bit from the input bit stream.
                    result = dataStream.ExtractBits8(val, 1);
                    RLC_MAC_RESULT_CHECK(result);
                    LengthOctets[i].extensionBit = (EXTENSION_BIT)val;
                
                } while (LengthOctets[i++].extensionBit == EXTENSION_OCTET_FOLLOWS);
                
                // Set the length value following the last valid length octet
                // to an invalid setting.
                LengthOctets[i].length = INVALID_LENGTH_VAL;
            }
            
            if (tlliInd == TLLI_PRESENT)
            {
                // Extract TLLI bits from the input bit stream.
                result = dataStream.ExtractBits32(tlli, 32);
                RLC_MAC_RESULT_CHECK(result);
            }
        }
        break;
    }
    DBG_LEAVE();
    return (result);
}



void ULMacHeader::DisplayDetails()
{
    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        HdrDisplayDetailObj.Trace("MAC Header\n");
        
        // Display Payload Type
        HdrDisplayDetailObj.Trace("\tPayload Type ---> %s\n", payloadTypeName[payloadType]);
    
        switch (payloadType)
        {
            case RLC_CTRL_BLOCK_OPT_OCTETS:
            {
                // Display Retry bit .
                HdrDisplayDetailObj.Trace("\tR ---> %s\n", retryName[retryBit]);
            }
            break;
        
            case RLC_DATA_BLOCK:
            {
                // Display Countdown Value bits.
                HdrDisplayDetailObj.Trace("\tCountdown Value ---> %d\n", countdownVal);
        
                // Display Stall Indicator bit.
                HdrDisplayDetailObj.Trace("\tSI ---> %s\n", siName[stallInd]);
            
                // Display Retry bit .
                HdrDisplayDetailObj.Trace("\tR ---> %s\n", retryName[retryBit]);
            
                // Display TFI bits.
                HdrDisplayDetailObj.Trace("\tTFI ---> %d\n", tfi);
            
                // Display TI bit .
                HdrDisplayDetailObj.Trace("\tTI ---> %s\n", tlliIndName[tlliInd]);
            
                // Display BSN bits.
                HdrDisplayDetailObj.Trace("\tBSN ---> %d\n", bsn);
            
                // Display Extension.
                HdrDisplayDetailObj.Trace("\tExtension ---> %s\n", extensionName[extensionBit]);

                if (extensionBit == EXTENSION_OCTET_FOLLOWS)
                {
                    int i = 0;
                    do 
                    {
                        // Display length bits.
                        HdrDisplayDetailObj.Trace("\tLength ---> %d\n", LengthOctets[i].length);
                
                        // Display More bit.
                        HdrDisplayDetailObj.Trace("\tMore ---> %s\n", moreName[LengthOctets[i].moreBit]);
                
                        // Display Extension bit.
                        HdrDisplayDetailObj.Trace("\tExtension ---> %s\n", extensionName[LengthOctets[i].extensionBit]);
                    
                    } while (LengthOctets[i++].extensionBit == EXTENSION_OCTET_FOLLOWS);
                }
            
                if (tlliInd == TLLI_PRESENT)
                {
                    // Display TLLI bits.
                    HdrDisplayDetailObj.Trace("\ttlli ---> %#x\n", tlli);
                }          
            }        
        }
    }
}
