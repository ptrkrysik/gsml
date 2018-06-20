// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieAckNackDescription.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEACKNACKDESCRIPTION_H__
#define __IEACKNACKDESCRIPTION_H__

#include "IeBase.h"
#include "bitstream.h"

#define RRB_SIZE        8
#define MAX_SSN_SIZE    127

// *******************************************************************
// class IeAckNackDescription
//
// Description:
//    Ack Nack Dscription IE -- GSM04.60 12.3
//
// < Ack/Nack Description IE > ::=
//   < FINAL_ACK_INDICATION : bit (1) >
//   < STARTING_SEQUENCE_NUMBER : bit (7) >
//   < RECEIVED_BLOCK_BITMAP : bit (64) > ;
// *******************************************************************

class IeAckNackDescription : public IeBase {
public:
    
    typedef enum
    {
        TBF_INCOMPLETE,
        TBF_COMPLETE
    } FINAL_ACK_INDICATION;
    
    IeAckNackDescription() : IeBase() {} 
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    RlcMacResult DecodeIe(BitStreamIn &dataStream);
    void DisplayDetails(DbgOutput *outObj);
    
    RlcMacResult SetAckNackDescription(FINAL_ACK_INDICATION finalAck, 
        unsigned char ssn, unsigned char rrb[RRB_SIZE])
    {
        if (ssn <= MAX_SSN_SIZE)
        {
            finalAckIndication = finalAck;
            startingSeqNum = ssn;
            for (int i = 0; i < RRB_SIZE; i++)
            {
                recBlockBitmap[i] = rrb[i];
            }
            isValid = TRUE;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    void GetAckNackDescription(FINAL_ACK_INDICATION *finalAck,
        unsigned char *ssn, unsigned char *rrb)
    {
        *finalAck = finalAckIndication;
        *ssn = startingSeqNum;
        for (int i = 0; i < RRB_SIZE; i++)
        {
            rrb[i] = recBlockBitmap[i];
        }
    }
    
    FINAL_ACK_INDICATION GetFinalAckInd() { return(finalAckIndication); }
    
    void ClearAckNackDescription() { isValid = FALSE; }
    
private:
    FINAL_ACK_INDICATION            finalAckIndication;
    unsigned char                   startingSeqNum;
    unsigned char                   recBlockBitmap[RRB_SIZE];
};

#endif