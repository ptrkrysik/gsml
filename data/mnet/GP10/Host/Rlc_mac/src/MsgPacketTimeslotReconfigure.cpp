// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketTimeslotReconfigure.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketTimeslotReconfigure.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// PACKET TIMESLOT RECONFIGURE
//
// GSM 04.60 11.2.31
//
// < Packet Timeslot Reconfigure message content > ::=
//   < PAGE_MODE : bit (2) >
//   {  0 < GLOBAL_TFI : < Global TFI IE > >
//      { 0     -- Message escape
//          {   < CHANNEL_CODING_COMMAND : bit (2) >
//              < Global Packet Timing Advance : < Global Packet Timing Advance IE > >
//              < DOWNLINK_RLC_MODE : bit (1) >
//              < CONTROL_ACK : bit (1) >
//              { 0 | 1 < DOWNLINK_TFI_ASSIGNMENT : bit (5) > }
//              { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
//              < DOWNLINK_TIMESLOT_ALLOCATION : bit (8) >
//              { 0 | 1 < Frequency Parameters : < Frequency Parameters IE > > }
//              { 0 < Dynamic Allocation : < Dynamic Allocation struct > >
//              | 1 < Fixed allocation : < Fixed Allocation struct > >}
//              < padding bits >
//               ! < Non-distribution part error : bit (*) = < no string > > }
//           ! < Message escape : 1 bit (*) = <no string> > }
//       ! < Address information part error : bit (*) = < no string > > }
//   ! < Distribution part error : bit (*) = < no string > > ;
//
// <Dynamic Allocation struct > ::=
//   < Extended Dynamic Allocation : bit (1) >
//   { 0 | 1 < P0 : bit (4) > 
//           < PR_MODE : bit (1) > }
//   < USF_GRANULARITY : bit (1) >
//   { 0 | 1 < RLC_DATA_BLOCKS_GRANTED : bit (8) > }
//   { 0 | 1 < TBF Starting Time : < Starting Frame Number Description IE > > }
//   { 0                        -- Timeslot Allocation
//       { 0 | 1 < USF_TN0 : bit (3) > }
//       { 0 | 1 < USF_TN1 : bit (3) > }
//       { 0 | 1 < USF_TN2 : bit (3) > }
//       { 0 | 1 < USF_TN3 : bit (3) > }
//       { 0 | 1 < USF_TN4 : bit (3) > }
//       { 0 | 1 < USF_TN5 : bit (3) > }
//       { 0 | 1 < USF_TN6 : bit (3) > }
//       { 0 | 1 < USF_TN7 : bit (3) > }
//    | 1                       -- Timeslot Allocation with Power Control Parameters
//       < ALPHA : bit (4) >
//       { 0 |   < USF_TN0 : bit (3) >
//               < GAMMA_TN0 : bit (5) > }
//       { 0 | 1 < USF_TN1 : bit (3) >
//               < GAMMA_TN1 : bit (5) > }
//       { 0 | 1 < USF_TN2 : bit (3) >
//               < GAMMA_TN2 : bit (5) > }
//       { 0 | 1 < USF_TN3 : bit (3) >
//               < GAMMA_TN3 : bit (5) > }
//       { 0 | 1 < USF_TN4 : bit (3) >
//               < GAMMA_TN4 : bit (5) > }
//       { 0 | 1 < USF_TN5 : bit (3) >
//               < GAMMA_TN5 : bit (5) > }
//       { 0 | 1 < USF_TN6 : bit (3) >
//               < GAMMA_TN6 : bit (5) > }
//       { 0 | 1 < USF_TN7 : bit (3) >
//               < GAMMA_TN7 : bit (5) > } } ;
//
// <Fixed Allocation struct > ::=
//   { 0    < UPLINK_TIMESLOT_ALLOCATION : bit (8) >
//    | 1   < Power Control Parameters : < Power Control Parameters IE > > }
//   < FINAL_ALLOCATION : bit (1) >
//   < DOWNLINK_CONTROL_TIMESLOT: bit (3) >
//   { 0 | 1 < P0 : bit (4) >
//           < BTS_PWR_CTRL_MODE : bit (1) > 
//           < PR_MODE : bit (1) > }
//   { 0 | 1    < Measurement Mapping : < Measurement Mapping struct > > }
//   < TBF Starting Time : < Starting Frame Number Description IE > >
//   { 0 { 0            -- with length of Allocation Bitmap
//           < BLOCKS_OR_BLOCK_PERIODS : bit (1) >
//           < ALLOCATION_BITMAP_LENGTH : bit (7) >
//           < ALLOCATION_BITMAP : bit (val(ALLOCATION_BITMAP_LENGTH)) >
//        | 1           -- without length of Allocation Bitmap (fills remainder of the message)
//           < ALLOCATION_BITMAP : bit ** > }
//        ! < Message escape : 1 bit (*) = <no string> > } ;
//
// < Measurement Mapping struct > ::=
//   < Measurement Starting Time : < Starting Frame Number Description IE >
//   < MEASUREMENT_INTERVAL : bit (5) >
//   < MEASUREMENT_BITMAP : bit (8) > ;
//
// *******************************************************************



RlcMacResult MsgPacketTimeslotReconfigure::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketTimeslotReconfigure::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //   < PAGE_MODE : bit (2) >
    result = pageMode.EncodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
      
    // Encode Global TFI.
    //   {  0 < GLOBAL_TFI : < Global TFI IE > >
    dataStream.InsertBits8(0, 1);
    result = globalTFI.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);

    //      { 0     -- Message escape
    dataStream.InsertBits8(0, 1);
    
    // Encode Channel Coding Command ie.
    //          {   < CHANNEL_CODING_COMMAND : bit (2) >
    result = chanCoding.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    
    // Encode Global Packet Timing Advance ie.
    //              < Global Packet Timing Advance : < Global Packet Timing Advance IE > >
    result = globalTimingAdvance.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    // Encode Downlink RLC Mode ie.
    //              < DOWNLINK_RLC_MODE : bit (1) >
    result = dlRlcMode.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    // Encode Control ACK bit
    //              < CONTROL_ACK : bit (1) >
    dataStream.InsertBits8(controlAck, 1);
    
    // Encode Downlink TFI if valid.
    //              { 0 | 1 < DOWNLINK_TFI_ASSIGNMENT : bit (5) > }
    if (dlTfi.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = dlTfi.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Uplink TFI if valid.
    //              { 0 | 1 < UPLINK_TFI_ASSIGNMENT : bit (5) > }
    if (ulTfi.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = ulTfi.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Downlink Timeslot Allocation ie.
    //              < DOWNLINK_TIMESLOT_ALLOCATION : bit (8) >
    result = dlTimeslotAllocation.EncodeIe(dataStream);
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
    
    // Encode Dynamic Allocation, or Fixed Allocation.
    //              { 0 < Dynamic Allocation : < Dynamic Allocation struct > >
    //              | 1 < Fixed allocation : < Fixed Allocation struct > >}
    if (dynamicAllocation.isValid)
    {
        dataStream.InsertBits8(0, 1);
        result = dynamicAllocation.EncodeDynamicAllocationStruct(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else if (fixedAllocation.isValid)
    {
        dataStream.InsertBits8(1, 1);
        result = fixedAllocation.EncodeFixedAllocationStruct(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("MsgPacketTimeslotReconfigure::EncodeMsgPacketTimeslotReconfigure Dynamic, "
            "or Fixed Allocation must be valid\n");
    }
    
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return (result);
}



void MsgPacketTimeslotReconfigure::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketTimeslotReconfigure::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayDownlinkMsgType());
      
        // Print out the Page Mode.
        pageMode.DisplayDetails(&MsgDisplayDetailObj);
      
        // Print out Global TFI
        if (globalTFI.IsValid())
        {
            globalTFI.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out Channel Coding Command ie.
        chanCoding.DisplayDetails(&MsgDisplayDetailObj);
   
        // Print out Global Packet Timing Advance ie.
        globalTimingAdvance.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out Donwlink RLC mode.
        dlRlcMode.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out Control ACK bit.
        MsgDisplayDetailObj.Trace("\tCONTROL_ACK ---> %d\n", controlAck);
    
        // Print out Downlink TFI
        if (dlTfi.IsValid())
        {
            MsgDisplayDetailObj.Trace("\tDownlink TFI\n");
            dlTfi.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out Uplink TFI
        if (ulTfi.IsValid())
        {
            MsgDisplayDetailObj.Trace("\tUplink TFI\n");
            ulTfi.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out Downlink Timeslot Allocation
        if (dlTimeslotAllocation.IsValid())
        {
            dlTimeslotAllocation.DisplayDetails(&MsgDisplayDetailObj);
        }
        
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
        
        // Print out Fixed Allocation.
        if (fixedAllocation.isValid)
        {
            fixedAllocation.DisplayFixedAllocationStruct(&MsgDisplayDetailObj);
        }
    }
   
    DBG_LEAVE();
}





