// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieChannelRequestDescription.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IECHANNELREQUESTDESCRIPTION_H__
#define __IECHANNELREQUESTDESCRIPTION_H__

#include "logging\vclogging.h"
#include "bitstream.h"
#include "ieRLCMode.h"


// *******************************************************************
// class IeChannelRequestDescription
//
// Description:
//    Channel Request Description IE -- GSM04.60 12.7
//
// < Channel Request Description IE > ::=   
//      < PEAK_THROUGHPUT_CLASS : bit (4) > 
//      < RADIO_PRIORITY : bit (2) >    
//      < RLC_MODE : bit (1) >  
//      < LLC_PDU_TYPE : bit (1) > 
//      < RLC_OCTET_COUNT : bit (16) > ;
// *******************************************************************

class IeChannelRequestDescription : public IeBase {
public:
    
    typedef enum
    {
        SACK_OR_ACK,
        NOT_SACK_OR_ACK
    } LLC_PDU_TYPE;
    
    // IE contents
    unsigned char           peakThroughputClass;
    unsigned char           radioPriority;
    IeRlcMode               rlcMode;
    LLC_PDU_TYPE            llcPduType;
    unsigned short          rlcOctetCount;
    
    IeChannelRequestDescription() : IeBase(), peakThroughputClass(1), radioPriority(1),
        rlcMode(), llcPduType(SACK_OR_ACK), rlcOctetCount(0) {} 
   
    RlcMacResult DecodeIe(BitStreamIn &dataStream);
    void DisplayDetails(DbgOutput *outObj);
    
	void SetChannelRequestDescriptionValid() { isValid = TRUE; }
   	void ClearChannelRequestDescription() { isValid = FALSE; }
};

#endif