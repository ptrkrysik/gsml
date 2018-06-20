// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieGlobalPacketTimingAdvance.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEGLOBALPACKETTIMINGADVANCE_H__
#define __IEGLOBALPACKETTIMINGADVANCE_H__

#include "IeBase.h"
#include "bitstream.h"

#define MAX_TA_VALUE        63
#define MAX_TA_INDEX        15
#define MAX_TA_TIMESLOT     7

// *******************************************************************
// class IeGlobalPacketTimingAdvance
//
// Description:
//    Timing Advance IE -- GSM04.60 12.12a
//
// < Global Packet Timing Advance IE > ::=
//			{ 0 | 1 < TIMING_ADVANCE_VALUE : bit (6) > }
//			{ 0 | 1 < UPLINK_TIMING_ADVANCE_INDEX : bit (4) >
//					< UPLINK_TIMING_ADVANCE_TIMESLOT_NUMBER : bit (3) > }
//			{ 0 | 1 < DOWNLINK_TIMING_ADVANCE_INDEX : bit (4) >
//					< DOWNLINK_TIMING_ADVANCE_TIMESLOT_NUMBER : bit (3) > }
//
// *******************************************************************

class IeGlobalPacketTimingAdvance : public IeBase {
public:

    IeGlobalPacketTimingAdvance() : IeBase(), isTaValueValid(FALSE), isUplinkTaIndexValid(FALSE),
        isDownlinkTaIndexValid(FALSE) {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetGlobalPacketTimingAdvance(unsigned char val)
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
    
    RlcMacResult SetGlobalPacketTimingAdvanceUplink(unsigned char index, unsigned char timeSlot)
    {
        if ((index <= MAX_TA_INDEX) && (timeSlot <= MAX_TA_TIMESLOT))
        {
            uplinkTaIndex = index;
            uplinkTsNumber = timeSlot;
            isUplinkTaIndexValid = TRUE;
            isValid = TRUE;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
    
    RlcMacResult SetGlobalPacketTimingAdvanceDownlink(unsigned char index, unsigned char timeSlot)
    {
        if ((index <= MAX_TA_INDEX) && (timeSlot <= MAX_TA_TIMESLOT))
        {
            downlinkTaIndex = index;
            downlinkTsNumber = timeSlot;
            isDownlinkTaIndexValid = TRUE;
            isValid = TRUE;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
    
   
    unsigned char GetGlobalPacketTimingAdvance()
    {
        return (taValue);
    }
    
    void GetGlobalPacketTimingAdvanceUplink(unsigned char *index, unsigned char *timeSlot)
    {
        *index = uplinkTaIndex;
        *timeSlot = uplinkTsNumber;
    }
   
    void GetGlobalPacketTimingAdvanceDownlink(unsigned char *index, unsigned char *timeSlot)
    {
        *index = downlinkTaIndex;
        *timeSlot = downlinkTsNumber;
    }
   
    void ClearGlobalPacketTimingAdvance() 
    { 
        isValid = FALSE; 
        isTaValueValid=FALSE; 
        isUplinkTaIndexValid=FALSE;
        isDownlinkTaIndexValid=FALSE;
    }
   
private:
    unsigned char   taValue;
    bool            isTaValueValid;
    unsigned char   uplinkTaIndex;
    unsigned char   uplinkTsNumber;
    bool            isUplinkTaIndexValid;    
    unsigned char   downlinkTaIndex;
    unsigned char   downlinkTsNumber;
    bool            isDownlinkTaIndexValid;    
};

#endif