// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketTimeslotReconfigure.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETTIMESLOTRECONFIGURE_H__
#define __MSGPACKETTIMESLOTRECONFIGURE_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "ieGlobalTFI.h"
#include "ieGlobalPacketTimingAdvance.h"
#include "ieRLCMode.h"
#include "ieTFI.h"
#include "ieTimeslotAllocation.h"
#include "ieFrequencyParameters.h"
#include "ieChannelCodingCommand.h"
#include "DynamicAllocationStruct.h"
#include "FixedAllocationStruct.h"


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

class MsgPacketTimeslotReconfigure : public DownlinkMsgBase {
public:

    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode                      pageMode;
    IeGlobalTFI                     globalTFI;
    IeChannelCodingCommand          chanCoding;
    IeGlobalPacketTimingAdvance     globalTimingAdvance;
    IeRlcMode                       dlRlcMode;
    bool                            controlAck;
    IeTFI                           dlTfi;
    IeTFI                           ulTfi;
    IeTimeslotAllocation            dlTimeslotAllocation;
    IeFrequencyParameters           frequencyParameters;
    DynamicAllocationStruct         dynamicAllocation;
    FixedAllocationStruct           fixedAllocation;

    // Default constructor
    MsgPacketTimeslotReconfigure() : DownlinkMsgBase(RlcMacMsgType::PACKET_TIMESLOT_RECONFIGURE),
        pageMode(), globalTFI(), chanCoding(), globalTimingAdvance(), dlRlcMode(),
        dlTfi(), ulTfi(), dlTimeslotAllocation(), frequencyParameters(), dynamicAllocation(), 
        fixedAllocation() 
    {
    }
   
    // Default destructor
    ~MsgPacketTimeslotReconfigure() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif