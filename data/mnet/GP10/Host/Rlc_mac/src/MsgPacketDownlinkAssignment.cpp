// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketDownlinkAssignment.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketDownlinkAssignment.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

// *******************************************************************
// PACKET DOWNLINK ASSIGNMENT
//
// GSM 04.60 11.2.7
//
// < Packet Downlink Assignment message content > ::=
//   < PAGE_MODE : bit (2) >
//   { 0 | 1 <PERSISTENCE_LEVEL : bit (4) > * 4 }
//   {   { 0    < Global TFI : < Global TFI IE > >
//       | 10   < TLLI : bit (32) > }
//       { 0        -- Message escape
//           {   < MAC_MODE : bit (2) >
//               < RLC_MODE : bit (1) >
//               < CONTROL_ACK : bit (1) >
//               < TIMESLOT_ALLOCATION : bit (8) >
//               < Packet Timing Advance : < Packet Timing Advance IE > >
//               { 0 | 1 < P0 : bit (4) >
//                   < BTS_PWR_CTRL_MODE : bit (1) > 
//                   < PR_MODE : bit (1) >}
//               {   { 0 | 1 < Frequency Parameters : < Frequency Parameters IE > > }
//                   { 0 | 1 < DOWNLINK_TFI_ASSIGNMENT : bit (5) > }
//                   { 0 | 1 < Power Control Parameters : < Power Control Parameters IE > > }
//                   { 0 | 1 < TBF Starting Time : < Starting Frame Number Description IE > > }
//                   { 0 | 1 < Measurement Mapping : < Measurement Mapping struct > > }
//                   < padding bits > } //      -- truncation at end of message allowed, bits '0' assumed
//                ! < Non-distribution part error : bit (*) = < no string > > }
//            ! < Message escape : 1 bit (*) = <no string> > }
//        ! < Address information part error : bit (*) = < no string > > }
//    ! < Distribution part error : bit (*) = < no string > > ;
//
// < Measurement Mapping struct > ::=
//   < Measurement Starting Time : < Starting Frame Number Description IE > >
//   < MEASUREMENT_INTERVAL : bit (5) >
//   < MEASUREMENT_BITMAP : bit (8) > ;
//
// *******************************************************************



RlcMacResult MsgPacketDownlinkAssignment::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketDownlinkAssignment::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //      < PAGE_MODE : bit (2) >
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
    
    // Encode Global TFI or TLLI.
    //   {   { 0    < Global TFI : < Global TFI IE > >
    //       | 10   < TLLI : bit (32) > }
    if (globalTFI.IsValid())
    {
        dataStream.InsertBits8(0, 1);
        result = globalTFI.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(2, 2);
        result = tlli.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }

    //       { 0        -- Message escape
    dataStream.InsertBits8(0, 1);
    
    // Encode MAC mode ie.
    //           {   < MAC_MODE : bit (2) >
    result = macMode.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode RLC mode ie.
    //               < RLC_MODE : bit (1) >
    result = rlcMode.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    // Encode Control ACK bit
    //               < CONTROL_ACK : bit (1) >
    dataStream.InsertBits8(controlAck, 1);
    
    // Encode Timeslot Allocation ie.
    //               < TIMESLOT_ALLOCATION : bit (8) >
    result = tsAllocation.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    // Encode Packet Timing Advance ie.
    //               < Packet Timing Advance : < Packet Timing Advance IE > >
    result = timingAdvance.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    // Encode PO, BTS_PWR_CTL_MODE, and PR_MODE bits.
    //               { 0 | 1 < P0 : bit (4) >
    //                   < BTS_PWR_CTRL_MODE : bit (1) > 
    //                   < PR_MODE : bit (1) >}
    if (PwrParams.isValid)
    {
        dataStream.InsertBits8(1, 1);
        result = PwrParams.po.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
        result = PwrParams.pCtlMode.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
        result = PwrParams.prMode.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Frequency Parameters ie.
    //               {   { 0 | 1 < Frequency Parameters : < Frequency Parameters IE > > }
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
    
    // Encode Downlink TFI Assignment ie.
    //                   { 0 | 1 < DOWNLINK_TFI_ASSIGNMENT : bit (5) > }
    if (dlTFIAssignment.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = dlTFIAssignment.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Power Control Parameters ie.
    //                   { 0 | 1 < Power Control Parameters : < Power Control Parameters IE > > }
    if (powerControlParameters.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = powerControlParameters.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode TBF Starting Time ie.
    //                   { 0 | 1 < TBF Starting Time : < Starting Frame Number Description IE > > }
    if (tbfStartingTime.IsValid())
    {
        dataStream.InsertBits8(1, 1);
        result = tbfStartingTime.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    // Encode Measurement Mapping struct.
    //                   { 0 | 1 < Measurement Mapping : < Measurement Mapping struct > > }
    if (measMapping.isValid)
    {
        dataStream.InsertBits8(1, 1);
        result = measMapping.EncodeMeasurementMappingStruct(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0, 1);
    }
    
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return (result);
}



void MsgPacketDownlinkAssignment::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketDownlinkAssignment::DisplayMsgDetail", RLC_MAC);
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
    
        // Print out Global TFI or TLLI.
        if (globalTFI.IsValid())
        {
            globalTFI.DisplayDetails(&MsgDisplayDetailObj);
        }
        else
        {
            tlli.DisplayDetails(&MsgDisplayDetailObj);
        }

        // Print out MAC mode ie.
        macMode.DisplayDetails(&MsgDisplayDetailObj);
   
        // Print out RLC mode ie.
        rlcMode.DisplayDetails(&MsgDisplayDetailObj);
    
        // Print out Control ACK bit
        MsgDisplayDetailObj.Trace("\tCONTROL_ACK ---> %d\n", controlAck);
    
        // Print out Timeslot Allocation ie.
        tsAllocation.DisplayDetails(&MsgDisplayDetailObj);
    
        // Print out Packet Timing Advance ie.
        timingAdvance.DisplayDetails(&MsgDisplayDetailObj);
    
        // Print out PO, BTS_PWR_CTL_MODE, and PR_MODE bits.
        if (PwrParams.isValid)
        {
            PwrParams.po.DisplayDetails(&MsgDisplayDetailObj);
            PwrParams.pCtlMode.DisplayDetails(&MsgDisplayDetailObj);
            PwrParams.prMode.DisplayDetails(&MsgDisplayDetailObj);
        }
    
        // Print out Frequency Parameters ie.
        if (frequencyParameters.IsValid())
        {
            frequencyParameters.DisplayDetails(&MsgDisplayDetailObj);
        }
    
        // Print out Downlink TFI Assignment ie.
        if (dlTFIAssignment.IsValid())
        {
            dlTFIAssignment.DisplayDetails(&MsgDisplayDetailObj);
        }
    
        // Print out Power Control Parameters ie.
        if (powerControlParameters.IsValid())
        {
            powerControlParameters.DisplayDetails(&MsgDisplayDetailObj);
        }
    
        // Print out TBF Starting Time ie.
        if (tbfStartingTime.IsValid())
        {
            tbfStartingTime.DisplayDetails(&MsgDisplayDetailObj);
        }
    
        // Print out Measurement Mapping struct.
        if (measMapping.isValid)
        {
            measMapping.DisplayMeasurementMappingStruct(&MsgDisplayDetailObj);
        }
    }
   
    DBG_LEAVE();
}





