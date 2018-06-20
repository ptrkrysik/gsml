// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketDownlinkAssignment.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETDOWNLINKASSIGNMENT_H__
#define __MSGPACKETDOWNLINKASSIGNMENT_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "iePersistenceLevel.h"
#include "ieTLLI.h"
#include "iePacketReqRef.h"
#include "ieGlobalTFI.h"
#include "ieMACMode.h"
#include "ieRLCMode.h"
#include "ieTimeslotAllocation.h"
#include "iePacketTimingAdvance.h"
#include "iePO.h"
#include "ieBtsPowerCtlMode.h"
#include "iePRMode.h"
#include "ieFrequencyParameters.h"
#include "ieTFI.h"
#include "iePowerControlParameters.h"
#include "ieStartFrameNum.h"
#include "MeasurementMappingStruct.h"


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

class MsgPacketDownlinkAssignment : public DownlinkMsgBase {
public:

    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode                      pageMode;
    struct PersistenceLevelStruct
    {
        IePersistenceLevel          persistenceLevel[4];
        bool                        isValid;
    } PersistenceLevelParams;
    IeGlobalTFI                     globalTFI;
    IeTLLI                          tlli;
    IeMacMode                       macMode;
    IeRlcMode                       rlcMode;
    bool                            controlAck;
    IeTimeslotAllocation            tsAllocation;
    IePacketTimingAdvance           timingAdvance;
    struct PwrParamsStruct
    {
        IePO                    po;
        IeBtsPowerCtlMode       pCtlMode;
        IePRMode                prMode;
        bool                    isValid;
    } PwrParams;
    IeFrequencyParameters       frequencyParameters;
    IeTFI                       dlTFIAssignment;  
    IePowerControlParameters    powerControlParameters; 
    IeStartFrameNum             tbfStartingTime;
    MeasurementMappingStruct    measMapping;

    // Default constructor
    MsgPacketDownlinkAssignment() : DownlinkMsgBase(RlcMacMsgType::PACKET_DOWNLINK_ASSIGNMENT),
        pageMode(), globalTFI(), tlli(), macMode(), rlcMode(), 
        controlAck(FALSE), tsAllocation(), timingAdvance(), frequencyParameters(), 
        dlTFIAssignment(), powerControlParameters(), tbfStartingTime(), measMapping() 
    {
        PwrParams.isValid = FALSE;
        PersistenceLevelParams.isValid = FALSE;
    }
   
    // Default destructor
    ~MsgPacketDownlinkAssignment() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif