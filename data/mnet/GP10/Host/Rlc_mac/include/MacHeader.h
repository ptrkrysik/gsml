// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MacHeader.h
// Author(s)   : Tim Olson
// Create Date : 11/22/2000
// Description : 
//
// *******************************************************************


#ifndef __MACHEADER_H__
#define __MACHEADER_H__

#include "bitstream.h"
#include "RlcMacCommon.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

#define MAX_RLC_LENGTH_OCTETS   10
#define INVALID_LENGTH_VAL      0xff

// *******************************************************************
// class MacHeader
//
// Description:
//    MAC layer header(s)
//
// *******************************************************************

class ULMacHeader {
public:

    typedef enum
    {
        RLC_DATA_BLOCK,
        RLC_CTRL_BLOCK_NO_OPT_OCTETS,
        RLC_CTRL_BLOCK_OPT_OCTETS,
        RESERVED
    } PAYLOAD_TYPE;
    
    typedef enum
    {
        ONE_CHAN_REQ,
        TWO_OR_MORE_CHAN_REQ
    } RETRY_BIT;
    
    typedef enum
    {
        NOT_STALLED,
        STALLED,
    } STALL_INDICATOR;
    
    typedef enum
    {
        TLLI_NOT_PRESENT,
        TLLI_PRESENT
    } TLLI_INDICATOR;
   
    typedef enum
    {
        EXTENSION_OCTET_FOLLOWS,
        NO_EXTENSION_OCTET
    } EXTENSION_BIT;

    typedef enum
    {
        NO_MORE_LLC_DATA,
        MORE_LLC_DATA
    } MORE_BIT;
    
    ULMacHeader() {}
   
    RlcMacResult DecodeULMacHeader(BitStreamIn &dataStream);
    void DisplayDetails();
   
    RlcMacResult SetPayloadType(PAYLOAD_TYPE val)
    {
        payloadType = val;
        return (RLC_MAC_SUCCESS);
    }
   
    PAYLOAD_TYPE GetPayloadType()
    {
        return (payloadType);
    }
    
    RlcMacResult SetRetryBit(RETRY_BIT val)
    {
        retryBit = val;
        return (RLC_MAC_SUCCESS);
    }
   
    RETRY_BIT GetRetryBit()
    {
        return (retryBit);
    }
  
    RlcMacResult SetStallIndicator(STALL_INDICATOR val)
    {
        stallInd = val;
        return (RLC_MAC_SUCCESS);
    }
   
    STALL_INDICATOR GetStallIndicator()
    {
        return (stallInd);
    }
   
    RlcMacResult SetCountdownVal(unsigned char val)
    {
        countdownVal = val;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetCountdownVal()
    {
        return (countdownVal);
    }
   
    RlcMacResult SetTFI(unsigned char val)
    {
        tfi = val;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetTFI()
    {
        return (tfi);
    }
    
    RlcMacResult SetTLLIIndicator(TLLI_INDICATOR val)
    {
        tlliInd = val;
        return (RLC_MAC_SUCCESS);
    }
   
    TLLI_INDICATOR GetTLLIIndicator()
    {
        return (tlliInd);
    }
   
    RlcMacResult SetBSN(unsigned char val)
    {
        bsn = val;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetBSN()
    {
        return (bsn);
    }
   
    RlcMacResult SetExtensionBit(EXTENSION_BIT val)
    {
        extensionBit = val;
        return (RLC_MAC_SUCCESS);
    }
   
    EXTENSION_BIT GetExtensionBit()
    {
        return (extensionBit);
    }
   
    RlcMacResult SetTLLI(unsigned long val)
    {
        tlli = val;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned long GetTLLI()
    {
        return (tlli);
    }
   
    RlcMacResult AddLengthOctet(unsigned char length, MORE_BIT more, EXTENSION_BIT extBit)
    {
        if (++currLengthOctet < MAX_RLC_LENGTH_OCTETS)
        {
            LengthOctets[currLengthOctet].length = length;
            LengthOctets[currLengthOctet].moreBit = more;
            LengthOctets[currLengthOctet].extensionBit = extBit;
        }
        
        return (RLC_MAC_SUCCESS);
    }
    
    bool GetCurrentLengthOctet(unsigned char *length, MORE_BIT *more, EXTENSION_BIT *extBit)
    {
        if (currLengthOctet < MAX_RLC_LENGTH_OCTETS)
        {
            *length = LengthOctets[currLengthOctet].length;
            *more = LengthOctets[currLengthOctet].moreBit;
            *extBit = LengthOctets[currLengthOctet].extensionBit;
            return (TRUE);
        }
        else
        {
            return (FALSE);
        }
    }
    
    bool GetNextLengthOctet(unsigned char *length, MORE_BIT *more, EXTENSION_BIT *extBit)
    {
        if (++currLengthOctet < MAX_RLC_LENGTH_OCTETS)
        {
            *length = LengthOctets[currLengthOctet].length;
            *more = LengthOctets[currLengthOctet].moreBit;
            *extBit = LengthOctets[currLengthOctet].extensionBit;
            return (TRUE);
        }
        else
        {
            return (FALSE);
        }
    }
    
    void ResetCurrLengthOctet()
    {
        currLengthOctet=0;
    }
   
private:
    PAYLOAD_TYPE                payloadType;
    RETRY_BIT                   retryBit;
    STALL_INDICATOR             stallInd;
    unsigned char               countdownVal;
    unsigned char               tfi;
    TLLI_INDICATOR              tlliInd;
    unsigned char               bsn;
    EXTENSION_BIT               extensionBit;
    struct 
    {
        unsigned char           length;
        MORE_BIT                moreBit;
        EXTENSION_BIT           extensionBit;
    } LengthOctets[MAX_RLC_LENGTH_OCTETS];
    int                         currLengthOctet;
    unsigned long               tlli;
    
    // Logger object for displayig header.
   static DbgOutput HdrDisplayDetailObj;
};


class DLMacHeader {
public:

    typedef enum
    {
    
        RLC_DATA_BLOCK,
        RLC_CTRL_BLOCK_NO_OPT_OCTETS,
        RLC_CTRL_BLOCK_OPT_OCTETS,
        RESERVED
    } PAYLOAD_TYPE;
    
    typedef enum
    {
        FN_N_13,
        FN_N_17_OR_N_18,
        FN_N_21_OR_N_22,
        FN_N_26
    } RRBP;
    
    typedef enum
    {
        RRBP_NOT_VALID,
        RRBP_VALID
    } SUPPLEMENTARY_POLLING_BIT;
    
    typedef enum
    {
        NOT_FINAL_SEGMENT,
        FINAL_SEGMENT
    } FINAL_SEGMENT_BIT;
    
    typedef enum
    {
        TFI_D_NOT_PRESENT,
        TFI_D_PRESENT
    } ADDRESS_CONTROL_BIT;
    
    typedef enum
    {
        MODEA_0_2_OR_MODEB_0_6_DB,
        MODEA_4_6_OR_MODEB_8_14_DB,
        MODEA_8_10_OR_MODEB_16_22_DB,
        MODEA_NOT_USABLE_OR_MODEB_24_30_DB
    } POWER_REDUCTION;
    
    typedef enum
    {
        UPLINK_TBF,
        DOWNLINK_TBF
    } DIRECTION_BIT;
    
    typedef enum
    {
        NOT_LAST_RLC_BLOCK,
        LAST_RLC_BLOCK
    } FINAL_BLOCK_INDICATOR;
    
    typedef enum
    {
        EXTENSION_OCTET_FOLLOWS,
        NO_EXTENSION_OCTET
    } EXTENSION_BIT;
    
    typedef enum
    {
        NO_MORE_LLC_DATA,
        MORE_LLC_DATA
    } MORE_BIT;
        
    DLMacHeader() : currLengthOctet(0) {}
   
    RlcMacResult EncodeDLMacHeader(BitStreamOut &dataStream);
    void DisplayDetails();
   
    RlcMacResult SetPayloadType(PAYLOAD_TYPE val)
    {
        payloadType = val;
        return (RLC_MAC_SUCCESS);
    }
   
    PAYLOAD_TYPE GetPayloadType()
    {
        return (payloadType);
    }
    
    RlcMacResult SetRRBP(RRBP val)
    {
        rrbp = val;
        return (RLC_MAC_SUCCESS);
    }
   
    RRBP GetRRBP()
    {
        return (rrbp);
    }
    
    RlcMacResult SetSPBit(SUPPLEMENTARY_POLLING_BIT val)
    {
        spBit = val;
        return (RLC_MAC_SUCCESS);
    }
   
    SUPPLEMENTARY_POLLING_BIT GetSPBit()
    {
        return (spBit);
    }
   
    RlcMacResult SetUSF(unsigned char val)
    {
        usf = val;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetUSF()
    {
        return (usf);
    }
   
    RlcMacResult SetRBSN(unsigned char val)
    {
        rbsn = val;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetRBSN()
    {
        return (rbsn);
    }
   
    RlcMacResult SetRTI(unsigned char val)
    {
        rti = val;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetRTI()
    {
        return (rti);
    }
   
    RlcMacResult SetFinalSegment(FINAL_SEGMENT_BIT val)
    {
        finalSegBit = val;
        return (RLC_MAC_SUCCESS);
    }
   
    FINAL_SEGMENT_BIT GetFinalSegment()
    {
        return (finalSegBit);
    }
 
    RlcMacResult SetAddressControl(ADDRESS_CONTROL_BIT val)
    {
        acBit = val;
        return (RLC_MAC_SUCCESS);
    }
   
    ADDRESS_CONTROL_BIT GetAddressControl()
    {
        return (acBit);
    }
  
    RlcMacResult SetPowerReduction(POWER_REDUCTION val)
    {
        powerReduction = val;
        return (RLC_MAC_SUCCESS);
    }
   
    POWER_REDUCTION GetPowerReduction()
    {
        return (powerReduction);
    }
   
    RlcMacResult SetTFI(unsigned char val)
    {
        tfi = val;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetTFI()
    {
        return (tfi);
    }
   
    RlcMacResult SetDirectionBit(DIRECTION_BIT val)
    {
        directionBit = val;
        return (RLC_MAC_SUCCESS);
    }
   
    DIRECTION_BIT GetDirectionBit()
    {
        return (directionBit);
    }
   
    RlcMacResult SetBSN(unsigned char val)
    {
        bsn = val;
        return (RLC_MAC_SUCCESS);
    }
   
    unsigned char GetBSN()
    {
        return (bsn);
    }
   
    RlcMacResult SetExtensionBit(EXTENSION_BIT val)
    {
        extensionBit = val;
        return (RLC_MAC_SUCCESS);
    }
   
    EXTENSION_BIT GetExtensionBit()
    {
        return (extensionBit);
    }
    
    RlcMacResult SetFinalBlockIndicator(FINAL_BLOCK_INDICATOR val)
    {
        fbiBit = val;
        return (RLC_MAC_SUCCESS);
    }
   
    FINAL_BLOCK_INDICATOR GetFinalBlockIndicator()
    {
        return (fbiBit);
    }
    
    bool SetLengthOctet(unsigned char length, MORE_BIT more, EXTENSION_BIT extBit)
    {
        if (currLengthOctet < MAX_RLC_LENGTH_OCTETS)
        {
            LengthOctets[currLengthOctet].length = length;
            LengthOctets[currLengthOctet].moreBit = more;
            LengthOctets[currLengthOctet].extensionBit = extBit;
            return (TRUE);
        }
        else
        {
            return (FALSE);
        }
    }
    
    RlcMacResult SetCurrLengthOctetExtension(EXTENSION_BIT extBit)
    {
        if (currLengthOctet < MAX_RLC_LENGTH_OCTETS)
        {
            LengthOctets[currLengthOctet].extensionBit = extBit;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_ERROR);
        }
    }
    
    bool GetNextLengthOctet(unsigned char *length, MORE_BIT *more, EXTENSION_BIT *extBit)
    {
        if (++currLengthOctet < MAX_RLC_LENGTH_OCTETS)
        {
            *length = LengthOctets[currLengthOctet].length;
            *more = LengthOctets[currLengthOctet].moreBit;
            *extBit = LengthOctets[currLengthOctet].extensionBit;
            return (TRUE);
        }
        else
        {
            return (FALSE);
        }
    }
    
    int GetCurrLengthOctetIndex() { return (currLengthOctet); }
    void IncrementCurrLengthOctetIndex() 
    {
        if (currLengthOctet < MAX_RLC_LENGTH_OCTETS) currLengthOctet++;  
    }
    void DecrementCurrLengthOctetIndex() 
    {
        if (currLengthOctet) currLengthOctet--;  
    }
    
    void ResetCurrLengthOctet()
    {
        currLengthOctet=0;
    }
   
    static int GetDataHdrSize()
    {
        return(3);      // Base header size for a data block
    }
    
private:
    PAYLOAD_TYPE                payloadType;
    RRBP                        rrbp;
    SUPPLEMENTARY_POLLING_BIT   spBit;
    unsigned char               usf;
    unsigned char               rbsn;
    unsigned char               rti;
    FINAL_SEGMENT_BIT           finalSegBit;
    ADDRESS_CONTROL_BIT         acBit;
    POWER_REDUCTION             powerReduction;
    unsigned char               tfi;
    FINAL_BLOCK_INDICATOR       fbiBit;
    DIRECTION_BIT               directionBit;
    unsigned char               bsn;
    EXTENSION_BIT               extensionBit;
    struct 
    {
        unsigned char           length;
        MORE_BIT                moreBit;
        EXTENSION_BIT           extensionBit;
    } LengthOctets[MAX_RLC_LENGTH_OCTETS];
    int                         currLengthOctet;
        
    // Logger object for displayig header.
   static DbgOutput HdrDisplayDetailObj;
};
#endif