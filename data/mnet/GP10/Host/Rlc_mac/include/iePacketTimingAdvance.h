// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePacketTimingAdvance.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEPACKETTIMINGADVANCE_H__
#define __IEPACKETTIMINGADVANCE_H__

#include "IeBase.h"
#include "bitstream.h"

#define MAX_TA_VALUE        63
#define MAX_TA_INDEX        15
#define MAX_TA_TIMESLOT     7

// *******************************************************************
// class IePacketTimingAdvance
//
// Description:
//    Packet Timing Advance IE -- GSM04.60 12.12
//
// < Packet Timing Advance IE > ::=
//	{ 0 | 1	< TIMING_ADVANCE_VALUE : bit (6) > }
//	{ 0 | 1	< TIMING_ADVANCE_INDEX : bit (4) >
//			< TIMING_ADVANCE_TIMESLOT_NUMBER : bit (3) > } ;
//
// *******************************************************************

class IePacketTimingAdvance : public IeBase {
public:

    IePacketTimingAdvance() : IeBase(), isTaValueValid(FALSE), isTaIndexValid(FALSE) {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetPacketTimingAdvance(unsigned char val)
    {
        if (val <= MAX_TA_VALUE)
        {
            taValue = val;
            isTaValueValid = TRUE;
            isValid = TRUE;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
    
    RlcMacResult SetPacketTimingAdvance(unsigned char index, unsigned char timeSlot)
    {
        if ((index <= MAX_TA_INDEX) && (timeSlot <= MAX_TA_TIMESLOT))
        {
            taIndex = index;
            tsNumber = timeSlot;
            isTaIndexValid = TRUE;
            isValid = TRUE;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    unsigned char GetPacketTimingAdvance()
    {
        return (taValue);
    }
    
    void GetPacketTimingAdvance(unsigned char *index, unsigned char *timeSlot)
    {
        *index = taIndex;
        *timeSlot = tsNumber;
    }
   
    void ClearPacketTimingAdvance() 
    { 
        isValid = FALSE; 
        isTaValueValid=FALSE; 
        isTaIndexValid=FALSE;
    }
   
private:
    unsigned char   taValue;
    bool            isTaValueValid;
    unsigned char   taIndex;
    unsigned char   tsNumber;
    bool            isTaIndexValid;    
};

#endif