// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketUplinkAssignment.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketUplinkAssignment.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char tlliCodeNames[][64] =
{
    "CS-1 for RLC block with TLLI",
    "Command Coding for RLC block with TLLI"
};

// *******************************************************************
// PACKET UPLINK ASSIGNMENT
//
// GSM 04.60 11.2.29
//
// < Packet Uplink Assignment message content > ::=
//   < PAGE_MODE : bit (2) >
//   { 0 | 1 <PERSISTENCE_LEVEL : bit (4) > * 4 }
//   {   {  0   < Global TFI : < Global TFI IE > >
//       | 10   < TLLI : bit (32) >
//       | 110  < TQI : bit (16) >
//       | 111  < Packet Request Reference : < Packet Request Reference IE > > }
//       { 0        -- Message escape
//          {   < CHANNEL_CODING_COMMAND : bit (2) >
//              < TLLI_BLOCK_CHANNEL_CODING : bit (1) >
//              < Packet Timing Advance : < Packet Timing Advance IE > >
//              { 0 | 1 < Frequency Parameters : < Frequency Parameters IE > > }
//              { 01    <Dynamic Allocation : < Dynamic Allocation struct > >
//               | 10   <Single Block Allocation : < Single Block Allocation struct > >
//               | 00   < extension >
//               | 11   < Fixed allocation : < Fixed Allocation struct > > }
//              < padding bits >
//               ! < Non-distribution part error : bit (*) = < no string > > } 
//           ! < Message escape : 1 bit (*) = <no string> > }
//       ! < Address information part error : bit (*) = < no string > > } 
//   ! < Distribution part error : bit (*) = < no string > > ;
//
// <extension> ::=  -- Future extension can be done by modifying this structure
//   null ;
//
// <Dynamic Allocation struct > ::= 
//   < Extended Dynamic Allocation : bit (1) >
//   { 0 | 1 < P0 : bit (4) > 
//          < PR_MODE : bit (1) > }
//   < USF_GRANULARITY : bit (1) >
//   { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
//   { 0 | 1 < RLC_DATA_BLOCKS_GRANTED : bit (8) > }
//   { 0 | 1 < TBF Starting Time : < Starting Frame Number Description IE > > }
//   { 0                        -- Timeslot Allocation
//      { 0 | 1 < USF_TN0 : bit (3) > }
//      { 0 | 1 < USF_TN1 : bit (3) > }
//      { 0 | 1 < USF_TN2 : bit (3) > }
//      { 0 | 1 < USF_TN3 : bit (3) > }
//      { 0 | 1 < USF_TN4 : bit (3) > }
//      { 0 | 1 < USF_TN5 : bit (3) > }
//      { 0 | 1 < USF_TN6 : bit (3) > }
//      { 0 | 1 < USF_TN7 : bit (3) > }
//   | 1                        -- Timeslot Allocation with Power Control Parameters
//      < ALPHA : bit (4) >
//      { 0 | 1 < USF_TN0 : bit (3) >
//              < GAMMA_TN0 : bit (5) > }
//      { 0 | 1 < USF_TN1 : bit (3) >
//              < GAMMA_TN1 : bit (5) > }
//      { 0 | 1 < USF_TN2 : bit (3) >
//              < GAMMA_TN2 : bit (5) > }
//      { 0 | 1 < USF_TN3 : bit (3) >
//              < GAMMA_TN3 : bit (5) > }
//      { 0 | 1 < USF_TN4 : bit (3) >
//              < GAMMA_TN4 : bit (5) > }
//      { 0 | 1 < USF_TN5 : bit (3) >
//              < GAMMA_TN5 : bit (5) > }
//      { 0 | 1 < USF_TN6 : bit (3) >
//              < GAMMA_TN6 : bit (5) > }
//      { 0 | 1 < USF_TN7 : bit (3) >
//              < GAMMA_TN7 : bit (5) > } } ;
//
// <Single Block Allocation struct > ::=
//   < TIMESLOT_NUMBER : bit (3) > 
//   { 0 | 1 < ALPHA : bit (4) >
//   < GAMMA_TN : bit (5) >}
//   { 0 | 1 < P0 : bit (4) >
//           < BTS_PWR_CTRL_MODE : bit (1) > 
//           < PR_MODE : bit (1) > }
//   < TBF Starting Time : < Starting Frame Number Description IE > > ;
//
// <Fixed Allocation struct > ::= 
//   { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
//   < FINAL_ALLOCATION : bit (1) >
//   < DOWNLINK_CONTROL_TIMESLOT: bit (3) >
//   { 0 | 1 < P0 : bit (4) >
//          < BTS_PWR_CTRL_MODE : bit (1) > 
//          < PR_MODE : bit (1) > }
//   { 0    < TIMESLOT_ALLOCATION : bit (8) >
//   | 1    < Power Control Parameters : < Power Control Parameters IE > > }
//   < HALF_DUPLEX_MODE : bit (1) >
//   < TBF Starting Time : < Starting Frame Number Description IE > > 
//   { 0    { 0         -- with length of Allocation Bitmap
//          < BLOCKS_OR_BLOCK_PERIODS : bit (1) >
//          < ALLOCATION_BITMAP_LENGTH : bit (7) > 
//          < ALLOCATION_BITMAP : bit (val(ALLOCATION_BITMAP_LENGTH)) >
//       | 1            -- without length of Allocation Bitmap (fills remainder of the message)
//          < ALLOCATION_BITMAP : bit ** > } 
//       ! < Message escape : 1 bit (*) = <no string> > } ;
//
// *******************************************************************



RlcMacResult MsgPacketUplinkAssignment::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketUplinkAssignment::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //   < PAGE_MODE : bit (2) >
    result = pageMode.EncodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
      
    // Encode Persistence Level ie.
    //   { 0 | 1 <PERSISTENCE_LEVEL : bit (4) > * 4 }
    if (PersistenceLevelParams.isValid)
    {
        for (int i = 0; i < 4; i++)
        {
            dataStream.InsertBits8(1, 1);
            result = PersistenceLevelParams.persistenceLevel[i].EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
        } 
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Global TFI, TLLI, TQI, or Packet Request Ref.
    //   {   {  0   < Global TFI : < Global TFI IE > >
    //       | 10   < TLLI : bit (32) >
    //       | 110  < TQI : bit (16) >
    //       | 111  < Packet Request Reference : < Packet Request Reference IE > > }
    if (globalTFI.IsValid())
    {
        dataStream.InsertBits8(0, 1);
        result = globalTFI.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else if (tlli.IsValid())
    {
        dataStream.InsertBits8(2, 2);
        result = tlli.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else if (tqi.IsValid())
    {
        dataStream.InsertBits8(6, 3);
        result = tqi.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else if (packetReqRef.IsValid())
    {
        dataStream.InsertBits8(7, 3);
        result = packetReqRef.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("MsgPacketUplinkAssignment::EncodeMsgPacketUplinkAssignment Global TFI, "
            "TLLI, TQI or Packet Req Ref must be valid\n");
    }

    //       { 0        -- Message escape
    dataStream.InsertBits8(0, 1);
    
    // Encode Channel Coding Command ie.
    //          {   < CHANNEL_CODING_COMMAND : bit (2) >
    result = chanCoding.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode TLLI Block Channel Coding.
    //              < TLLI_BLOCK_CHANNEL_CODING : bit (1) >
    dataStream.InsertBits8((unsigned char)tlliBlockCoding, 1);
    
    // Encode Packet Timing Advance ie.
    //              < Packet Timing Advance : < Packet Timing Advance IE > >
    result = timingAdvance.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    // Encode Frequency Parameters ie.
    //              { 0 | 1 < Frequency Parameters : < Frequency Parameters IE > > }
    if (frequencyParameters.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = frequencyParameters.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Dynamic Allocation, Single Block Allocation, or Fixed Allocation.
    //              { 01    <Dynamic Allocation : < Dynamic Allocation struct > >
    //               | 10   <Single Block Allocation : < Single Block Allocation struct > >
    //               | 00   < extension >
    //               | 11   < Fixed allocation : < Fixed Allocation struct > > }
    if (dynamicAllocation.isValid)
    {
        dataStream.InsertBits8(1, 2);
        result = dynamicAllocation.EncodeDynamicAllocationStruct(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else if (singleBlockAllocation.isValid)
    {
        dataStream.InsertBits8(1, 2);
        result = singleBlockAllocation.EncodeSingleBlockAllocationStruct(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else if (fixedAllocation.isValid)
    {
        dataStream.InsertBits8(1, 2);
        result = fixedAllocation.EncodeFixedAllocationStruct(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("MsgPacketUplinkAssignment::EncodeMsgPacketUplinkAssignment Dynamic, "
            "Single Block, or Fixed Allocation must be valid\n");
    }
    
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return (result);
}



void MsgPacketUplinkAssignment::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketUplinkAssignment::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayDownlinkMsgType());
      
        // Print out the Page Mode.
        pageMode.DisplayDetails(&MsgDisplayDetailObj);
      
        // Print out Persistence Level ie.
        if (PersistenceLevelParams.isValid)
        {
            for (int i = 0; i < 4; i++)
            {
                PersistenceLevelParams.persistenceLevel[i].DisplayDetails(&MsgDisplayDetailObj);
            }
        }
    
        // Print out Global TFI
        if (globalTFI.IsValid())
        {
            globalTFI.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out TLLI
        if (tlli.IsValid())
        {
            tlli.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out TQI
        if (tqi.IsValid())
        {
            tqi.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out Request Ref.
        if (packetReqRef.IsValid())
        {
            packetReqRef.DisplayDetails(&MsgDisplayDetailObj);
        }

        // Print out Channel Coding Command ie.
        chanCoding.DisplayDetails(&MsgDisplayDetailObj);
   
        // Print out TLLI Block Channel Coding.
        MsgDisplayDetailObj.Trace("\tTLLI Block Channel Coding ---> %s\n", 
            tlliCodeNames[tlliBlockCoding]);
    
        // Print out Packet Timing Advance ie.
        timingAdvance.DisplayDetails(&MsgDisplayDetailObj);
    
        // Print out Frequency Parameters ie.
        if (frequencyParameters.IsValid())
        {
            frequencyParameters.DisplayDetails(&MsgDisplayDetailObj);
        }
    
        // Print out Dynamic Allocation.
        if (dynamicAllocation.isValid)
        {
            dynamicAllocation.DisplayDynamicAllocationStruct(&MsgDisplayDetailObj);
        }
        
        // Print out Single Block Allocation.
        if (singleBlockAllocation.isValid)
        {
            singleBlockAllocation.DisplaySingleBlockAllocationStruct(&MsgDisplayDetailObj);
        }
        
        // Print out Fixed Allocation.
        if (fixedAllocation.isValid)
        {
            fixedAllocation.DisplayFixedAllocationStruct(&MsgDisplayDetailObj);
        }
    }
   
    DBG_LEAVE();
}





