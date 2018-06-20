// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacMsgType.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __RLCMACMSGTYPE_H__
#define __RLCMACMSGTYPE_H__

#include "bitstream.h"
#include "RlcMacResult.h"

// Message types defined for RLC/MAC operation.
// GSM04.60 11.2.0

#define MAX_RLC_MAC_MSG_NAME_SIZE   128

extern char UplinkMsgNames[][MAX_RLC_MAC_MSG_NAME_SIZE];
extern char NonDistributedDownlinkMsgNames[][MAX_RLC_MAC_MSG_NAME_SIZE];
extern char DistributedDownlinkMsgNames[][MAX_RLC_MAC_MSG_NAME_SIZE];
extern char SysInfoDownlinkMsgNames[][MAX_RLC_MAC_MSG_NAME_SIZE];

// *******************************************************************
// class MsgBase
//
// Description
//    
// *******************************************************************

class RlcMacMsgType
{
public:

   // Downlink RLC/MAC messages 11.2.0.1
   typedef enum {
      PACKET_CELL_CHANGE_ORDER                      = 0x01,
      PACKET_DOWNLINK_ASSIGNMENT                    = 0x02,
      PACKET_MEASUREMENT_ORDER                      = 0x03,
      PACKET_POLLING_REQUEST                        = 0x04,
      PACKET_POWER_CONTROL_TIMING_ADVANCE           = 0x05,
      PACKET_QUEUEING_NOTIFICATION                  = 0x06,
      PACKET_TIMESLOT_RECONFIGURE                   = 0x07,
      PACKET_TBF_RELEASE                            = 0x08,
      PACKET_UPLINK_ACK_NACK                        = 0x09,
      PACKET_UPLINK_ASSIGNMENT                      = 0x0A,
      RLC_MAC_MAX_NONDISTRIBUTED_DOWNLINK_MSG
   } RLC_MAC_NONDISTRIBUTED_DOWNLINK_MSGS;
   
   typedef enum {
      PACKET_ACCESS_REJECT                          = 0x21,
      PACKET_PAGING_REQUEST                         = 0x22,
      PACKET_PDCH_RELEASE                           = 0x23,
      PACKET_PRACH_PARAMETERS                       = 0x24,
      PACKET_DOWNLINK_DUMMY_CONTROL_BLOCK           = 0x25,
      RLC_MAC_MAX_DISTRIBUTED_DOWNLINK_MSG
   } RLC_MAC_DISTRIBUTED_DOWNLINK_MSGS;
   
   typedef enum {
      PACKET_SYSTEM_INFO_1                          = 0x31,
      PACKET_SYSTEM_INFO_2                          = 0x32,
      PACKET_SYSTEM_INFO_3                          = 0x33,
      PACKET_SYSTEM_INFO_3_BIS                      = 0x34,
      PACKET_SYSTEM_INFO_4                          = 0x35,
      PACKET_SYSTEM_INFO_5                          = 0x36,
      PACKET_SYSTEM_INFO_13                         = 0x37,
      RLC_MAC_MAX_SYSINFO_MSG
   } RLC_MAC_SYSINFO_DOWNLINK_MSGS;
     
   // Uplink RLC/MAC messages 11.2.0.2
   typedef enum {   
      PACKET_CELL_CHANGE_FAILURE                    = 0x00,
      PACKET_CONTROL_ACK                            = 0x01,
      PACKET_DOWNLINK_ACK_NACK                      = 0x02,
      PACKET_UPLINK_DUMMY_CONTROL_BLOCK             = 0x03,
      PACKET_MEASUREMENT_REPORT                     = 0x04,
      PACKET_RESOURCE_REQUEST                       = 0x05,
      PACKET_MOBILE_TBF_STATUS                      = 0x06,
      PACKET_PSI_STATUS                             = 0x07,
      PACKET_CONTROL_ACK_8_BIT                      = 0x1f,
      RLC_MAC_MAX_UPLINK_MSG
   } RLC_MAC_UPLINK_MSGS;
   
   // Constructors
   RlcMacMsgType(RLC_MAC_NONDISTRIBUTED_DOWNLINK_MSGS type) : msgType(type) {}
   RlcMacMsgType(RLC_MAC_DISTRIBUTED_DOWNLINK_MSGS type) : msgType(type) {}
   RlcMacMsgType(RLC_MAC_SYSINFO_DOWNLINK_MSGS type) : msgType(type) {}
   RlcMacMsgType(RLC_MAC_UPLINK_MSGS type) : msgType(type) {}
   RlcMacMsgType(){}
   
   // Pack message type into a single ouput stream of bits
   RlcMacResult EncodeMsgType(BitStreamOut &dataStream);
   
   // Extract message type from a single input stream of bits
   RlcMacResult DecodeMsgType(BitStreamIn &dataStream);
   
   // Return a printable message type string.
   char *DisplayDownlinkMsgType()
   {
      if ((msgType < RLC_MAC_MAX_NONDISTRIBUTED_DOWNLINK_MSG) &&
          (msgType > 0))
      {
          return (NonDistributedDownlinkMsgNames[msgType]);
      }
      else if ((msgType < RLC_MAC_MAX_DISTRIBUTED_DOWNLINK_MSG) &&
          (msgType > 0x20))
      {
          return (DistributedDownlinkMsgNames[msgType & 0xDF]); 
      }
      else if ((msgType < RLC_MAC_MAX_SYSINFO_MSG) && (msgType > 0x30))
      {
         return (SysInfoDownlinkMsgNames[msgType & 0xCF]);
      }
      else
      {
         return ("Unknown RLC/MAC downlink message.");
      }
   }
   
   char *DisplayUplinkMsgType()
   {
      if (msgType < RLC_MAC_MAX_UPLINK_MSG)
         return (UplinkMsgNames[msgType]);
      else
         return ("Unknown RLC/MAC uplink message.");
   }
    
   unsigned char msgType;

};


#endif