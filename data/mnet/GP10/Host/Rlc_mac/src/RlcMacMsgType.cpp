// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacMsgType.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "RlcMacMsgType.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char NonDistributedDownlinkMsgNames[][MAX_RLC_MAC_MSG_NAME_SIZE] = 
{
    {"UNUSED_MSG_TYPE"},
    {"PACKET_CELL_CHANGE_ORDER"},
    {"PACKET_DOWNLINK_ASSIGNMENT"},         
    {"PACKET_MEASUREMENT_ORDER"},           
    {"PACKET_POLLING_REQUEST"},             
    {"PACKET_POWER_CONTROL_TIMING_ADVANCE"},
    {"PACKET_QUEUEING_NOTIFICATION"},       
    {"PACKET_TIMESLOT_RECONFIGURE"},        
    {"PACKET_TBF_RELEASE"},                 
    {"PACKET_UPLINK_ACK_NACK"},             
    {"PACKET_UPLINK_ASSIGNMENT"}
};

           
char DistributedDownlinkMsgNames[][MAX_RLC_MAC_MSG_NAME_SIZE] = 
{
    {"UNUSED_MSG_TYPE"},
    {"PACKET_ACCESS_REJECT"},               
    {"PACKET_PAGING_REQUEST"},              
    {"PACKET_PDCH_RELEASE"},                
    {"PACKET_PRACH_PARAMETERS"},            
    {"PACKET_DOWNLINK_DUMMY_CONTROL_BLOCK"}
};


char SysInfoDownlinkMsgNames[][MAX_RLC_MAC_MSG_NAME_SIZE] = 
{
    {"UNUSED_MSG_TYPE"},
    {"PACKET_SYSTEM_INFO_1"},               
    {"PACKET_SYSTEM_INFO_2"},               
    {"PACKET_SYSTEM_INFO_3"},               
    {"PACKET_SYSTEM_INFO_3_BIS"},           
    {"PACKET_SYSTEM_INFO_4"},              
    {"PACKET_SYSTEM_INFO_5"},               
    {"PACKET_SYSTEM_INFO_13"}
};


char UplinkMsgNames[][MAX_RLC_MAC_MSG_NAME_SIZE] = 
{
    {"PACKET_CELL_CHANGE_FAILURE"},       
    {"PACKET_CONTROL_ACK"},               
    {"PACKET_DOWNLINK_ACK_NACK"},         
    {"PACKET_UPLINK_DUMMY_CONTROL_BLOCK"},
    {"PACKET_MEASUREMENT_REPORT"},        
    {"PACKET_RESOURCE_REQUEST"},          
    {"PACKET_MOBILE_TBF_STATUS"},         
    {"PACKET_PSI_STATUS"},
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"UNKOWN_MESSAGE"},         
    {"PACKET_DOWNLINK_ACK_NACK_8_BIT"}         
};         



RlcMacResult RlcMacMsgType::EncodeMsgType(BitStreamOut &dataStream)
{    
    DBG_FUNC("RlcMacMsgType::EncodeMsgType", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result;
    
    // Pack the Message Type bits into the output bit stream.
    if ((result = dataStream.InsertBits8((unsigned char)msgType, 6)) != RLC_MAC_SUCCESS)
    {
        DBG_ERROR("RlcMacMsgType::EncodeMsgType msgType failure %x\n", result);
    }
    
    DBG_LEAVE();
    return (result); 
}



RlcMacResult RlcMacMsgType::DecodeMsgType(BitStreamIn &dataStream)
{
    DBG_FUNC("RlcMacMsgType::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Extract message type bits from the input bit stream.
    if ((result = dataStream.ExtractBits8(msgType, 6)) != RLC_MAC_SUCCESS)
    {
        DBG_ERROR("RlcMacMsgType::DecodeIe msgType extraction failed\n");
    }
    
    DBG_LEAVE();
    return (result);
}
