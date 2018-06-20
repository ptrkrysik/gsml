// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketResourceRequest.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETRESOURCEREQUEST_H__
#define __MSGPACKETRESOURCEREQUEST_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "RlcMacCommon.h"
#include "ieAckNackDescription.h"
#include "ieChannelRequestDescription.h"
#include "ieGlobalTFI.h"
#include "ieTLLI.h"
#include "ieAccessType.h"
#include "ieMSRadioAccessCapability.h"
#include "ieChangeMark.h"


// *******************************************************************
// PACKET RESOURCE REQUEST
//
// GSM 04.60 11.2.16
//
// < Packet Resource Request message content > ::=	
//      { 0 | 1 < ACCESS_TYPE : bit (2) > }	
//      { 0 < Global TFI : < Global TFI IE > >	
//      | 1 < TLLI : < TLLI IE > > }	
//      { 0 | 1 < MS Radio Access Capability : < MS Radio Access Capability IE > > }	
//      < Channel Request Description : < Channel Request Description IE > >	
//      { 0 | 1 < CHANGE_MARK : bit (2) > }	
//      < C_VALUE : bit (6) >	
//      { 0 | 1 < SIGN_VAR : bit (6) >}	
//      { 0 | 1 < I_LEVEL_TN0 : bit (4) > }	
//      { 0 | 1 < I_LEVEL_TN1 : bit (4) > }	
//      { 0 | 1 < I_LEVEL_TN2 : bit (4) > }	
//      { 0 | 1 < I_LEVEL_TN3 : bit (4) > }	
//      { 0 | 1 < I_LEVEL_TN4 : bit (4) > }	
//      { 0 | 1 < I_LEVEL_TN5 : bit (4) > }	
//      { 0 | 1 < I_LEVEL_TN6 : bit (4) > }	
//      { 0 | 1 < I_LEVEL_TN7 : bit (4) > }	
//      < padding bits > ;
//
// *******************************************************************

class MsgPacketResourceRequest : public UplinkMsgBase {
public:

        
    // Message type is included in MsgBase.
   
    // Message contents
    IeAccessType                    accessType;
    IeGlobalTFI                     globalTfi;
    IeTLLI                          tlli;
    IeMSRadioAccessCapability       msRadioAccessCapability;
    IeChannelRequestDescription     chanReqDescription;
    IeChangeMark                    changeMark;
    unsigned char                   cValue;
    struct SignVarStruct
    {
        unsigned char               signVar;
        bool                        isValid;
    } SignVar;
    struct ILevelStruct
    {
        unsigned char               iLevel;
        bool                        isValid;
    } ILevel[MAX_TIMESLOTS];

    // Default constructor
    MsgPacketResourceRequest() : UplinkMsgBase(RlcMacMsgType::PACKET_RESOURCE_REQUEST),
        accessType(), globalTfi(), tlli(), msRadioAccessCapability(), chanReqDescription(), 
        changeMark(), cValue(0)
    {
        for (int i=0; i < MAX_TIMESLOTS; i++)
        {
            ILevel[i].isValid = FALSE;
        }
        
        SignVar.isValid = FALSE;
    }
   
    // Default destructor
    ~MsgPacketResourceRequest() {}
   
    // Decode input stream of bits into message.
    RlcMacResult DecodeMsg(BitStreamIn &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif