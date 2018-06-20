#include "MsgPacketAccessReject.h"
#include "MsgPacketDownlinkAssignment.h"
#include "MsgPacketPagingRequest.h"
#include "MsgPacketPDCHRelease.h"
#include "MsgPacketPollingRequest.h"
#include "MsgPacketPowerCtlTimingAdvance.h"
#include "MsgPacketTBFRelease.h"
#include "MsgPacketDownlinkDummyControlBlock.h"
#include "MsgPacketUplinkAssignment.h"
#include "MsgPacketTimeslotReconfigure.h"
#include "MsgPacketUplinkAckNack.h"
#include "MsgPacketCellChangeFailure.h"
#include "MsgPacketControlAcknowledgement.h"
#include "MsgPacketDownlinkAckNack.h"
#include "MsgPacketUplinkDummyControlBlock.h"
#include "MsgPacketResourceRequest.h"
#include "MsgPacketMobileTBFStatus.h"

// *******************************************************************
// PACKET ACCESS REJECT
//
// GSM 04.60 11.2.1
//
//    < Packet Access Reject message content > ::=
//      < PAGE_MODE : bit (2) >
//      < Reject : < Reject struct > >
//      { { 1 < Additional Reject: < Reject struct > > } ** 0
//       < padding bits > } //      -- truncation at end of message allowed, bits '0' assumed
//       ! < Distribution part error : bit (*) = < no string > > ;
//
//    < Reject struct > ::=
//      { 0 < TLLI : bit (32) >
//       | 1    { 0 < Packet Request Reference : < Packet Request Reference IE > >
//           | 1    < Global TFI : < Global TFI IE > > } }
//      { 0 | 1 < WAIT_INDICATION : bit (8) >
//              < WAIT _INDICATION_SIZE : bit (1) > }
//       ! < Ignore : bit (*) = <no string> > ;
//
// *******************************************************************
void TestPacketAccessRejectMsg()
{
    BitStreamOut outStream;

    MsgPacketAccessReject  packetAccessRejectMsg;
   
    packetAccessRejectMsg.pageMode.SetPageMode(PageMode::EXTENDED_PAGING);
    packetAccessRejectMsg.reject.tlli.SetTLLI(0x1234);
    packetAccessRejectMsg.reject.waitInd.SetWaitIndication(200, WaitIndication::UNITS_OF_SECONDS);
   
    packetAccessRejectMsg.EncodeMsg(outStream);
    packetAccessRejectMsg.DisplayMsgShort(&outStream);
    packetAccessRejectMsg.DisplayMsgDetail();
   
    packetAccessRejectMsg.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    packetAccessRejectMsg.reject.tlli.ClearTLLI();
    packetAccessRejectMsg.reject.packetReqRef.SetPacketReqRef(0x12,1,2,3);
    packetAccessRejectMsg.reject.waitInd.SetWaitIndication(200, WaitIndication::UNITS_OF_20_MS);
   
    outStream.ClearBitStream();
    packetAccessRejectMsg.EncodeMsg(outStream);
    packetAccessRejectMsg.DisplayMsgShort(&outStream);
    packetAccessRejectMsg.DisplayMsgDetail();
   
    packetAccessRejectMsg.pageMode.SetPageMode(PageMode::PAGING_REORGANIZATION);
    packetAccessRejectMsg.reject.packetReqRef.ClearPacketReqRef();
    packetAccessRejectMsg.reject.waitInd.ClearWaitIndication();
    packetAccessRejectMsg.reject.globalTFI.SetGlobalTFI(18, GlobalTFI::UPLINK_TFI);
   
    outStream.ClearBitStream();
    packetAccessRejectMsg.EncodeMsg(outStream);
    packetAccessRejectMsg.DisplayMsgShort(&outStream);
    packetAccessRejectMsg.DisplayMsgDetail();
   
    packetAccessRejectMsg.pageMode.SetPageMode(PageMode::PAGING_SAME_AS_BEFORE);
    packetAccessRejectMsg.additionalReject.globalTFI.SetGlobalTFI(18, GlobalTFI::DOWNLINK_TFI);
    packetAccessRejectMsg.additionalReject.isValid = TRUE;
   
    outStream.ClearBitStream();
    packetAccessRejectMsg.EncodeMsg(outStream);
    packetAccessRejectMsg.DisplayMsgShort(&outStream);
    packetAccessRejectMsg.DisplayMsgDetail();
}

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
void TestPacketDownlinkAssignmentMsg()
{
    BitStreamOut outStream;

    MsgPacketDownlinkAssignment  packetDlAssignMsg;
   
    packetDlAssignMsg.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    unsigned char persLev[4] = {1,2,3,4};
    packetDlAssignMsg.persistenceLevel.SetPersistenceLevel(persLev);
    packetDlAssignMsg.tlli.SetTLLI(0x1234);
    packetDlAssignMsg.macMode.SetMacMode(MacMode::DYNAMIC_ALLOCATION);
    packetDlAssignMsg.rlcMode.SetRlcMode(RlcMode::RLC_ACKNOWLEDGED);
    packetDlAssignMsg.controlAck = FALSE;
    packetDlAssignMsg.tsAllocation.SetTimeslotAllocation(0x4);
    packetDlAssignMsg.timingAdvance.SetPacketTimingAdvance(1);
    packetDlAssignMsg.PwrParams.po.SetPO(9);
    packetDlAssignMsg.PwrParams.pCtlMode.SetBtsPowerCtlMode(BtsPowerCtlMode::MODE_A);
    packetDlAssignMsg.PwrParams.prMode.SetPRMode(PRMode::PR_MODE_B_ALL_MS);
    packetDlAssignMsg.PwrParams.isValid = TRUE;
    packetDlAssignMsg.frequencyParameters.tsc.SetTSC(5);
    packetDlAssignMsg.frequencyParameters.arfcn.SetARFCN(585);
    packetDlAssignMsg.frequencyParameters.SetFrequencyParameters();
    packetDlAssignMsg.dlTFIAssignment.SetTFI(18);
    GammaStruct gammas[8] = {{1,1},{1,2},{0,3},{1,4},{1,5},{1,6},{1,7},{1,8}};
    packetDlAssignMsg.powerControlParameters.SetPowerControlParameters(3, gammas); 
    packetDlAssignMsg.tbfStartingTime.SetStartFrameNum(1,2,3);
    packetDlAssignMsg.measMapping.measStartTime.SetStartFrameNum(4,5,6);
    packetDlAssignMsg.measMapping.measInterval = 5;
    packetDlAssignMsg.measMapping.measBitMap = 10;
    packetDlAssignMsg.measMapping.isValid = TRUE;
   
    packetDlAssignMsg.EncodeMsg(outStream);
    packetDlAssignMsg.DisplayMsgShort(&outStream);
    packetDlAssignMsg.DisplayMsgDetail();
}



// *******************************************************************
// PACKET PAGING REQUEST
//
// GSM 04.60 11.2.10
//
// < Packet Paging Request message content > ::=
//   < PAGE_MODE : bit (2) >
//   { 0 | 1 < PERSISTENCE_LEVEL : bit (4) >* 4}
//   { 0 | 1 < NLN : bit (2) > }
//   {  { 1 < Repeated Page info : < Repeated Page info struct > > } ** 0
//      < padding bits > } //       -- truncation at end of message allowed, bits '0' assumed
//   ! < Distribution part error : bit (*) = < no string > > ;
//
// < Repeated Page info struct > ::=
//   { 0                                        -- Page request for TBF establishment
//       { 0 < PTMSI : bit (32) >
//       | 1 < Length of Mobile Identity contents : bit (4) >
//          < Mobile Identity : octet (val (Length of Mobile Identity contents)) > }
//   | 1                                            -- Page request for RR conn. establishment
//       { 0 < TMSI : bit (32) >
//       | 1 < Length of Mobile Identity contents : bit (4) >
//           < Mobile Identity : octet (val (Length of Mobile Identity contents)) > }
//       < CHANNEL_NEEDED : bit (2) >
//       { 0 | 1 < eMLPP_PRIORITY : bit (3) > } }
//   ! < Ignore : bit (*) = <no string> > ;
//
// *******************************************************************
void TestPacketPagingRequestMsg()
{
    BitStreamOut outStream;

    MsgPacketPagingRequest  packetPageReqMsg;
   
    packetPageReqMsg.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    unsigned char persLev[4] = {1,2,3,4};
    packetPageReqMsg.persistenceLevel.SetPersistenceLevel(persLev);
    packetPageReqMsg.nln.SetNLN(2);
    packetPageReqMsg.RepeatedPageInfo[0].TBFPage.ptmsi.SetPTMSI(0x1234abcd);
    packetPageReqMsg.RepeatedPageInfo[0].TBFPage.isValid = TRUE;
    packetPageReqMsg.RepeatedPageInfo[0].isValid = TRUE;
    unsigned char imsi[15] = {0,1,2,3,4,5,6,7,8,9,0,1,2,3,4};
    packetPageReqMsg.RepeatedPageInfo[1].TBFPage.mobileId.SetMobileIdentity(
        MobileIdentity::IMSI, 15, imsi);
    packetPageReqMsg.RepeatedPageInfo[1].TBFPage.isValid = TRUE;
    packetPageReqMsg.RepeatedPageInfo[1].isValid = TRUE;
    packetPageReqMsg.RepeatedPageInfo[3].RRConnPage.tmsi.SetTMSI(0xa5a5a5a5);
    packetPageReqMsg.RepeatedPageInfo[3].RRConnPage.chanNeeded.SetChannelNeeded(
        ChannelNeeded::TCH_F_FULL_RATE);
    packetPageReqMsg.RepeatedPageInfo[3].RRConnPage.isValid = TRUE;
    packetPageReqMsg.RepeatedPageInfo[3].isValid = TRUE;
    
    packetPageReqMsg.RepeatedPageInfo[2].RRConnPage.mobileId.SetMobileIdentity(
        MobileIdentity::TMSI_PTMSI, 0xf0f0f0f0);
    packetPageReqMsg.RepeatedPageInfo[2].RRConnPage.chanNeeded.SetChannelNeeded(
        ChannelNeeded::ANY_CHANNEL);
    packetPageReqMsg.RepeatedPageInfo[2].RRConnPage.emlpPriority.SeteMLPPPriority(
        eMLPPPriority::CALL_PRIORITY_LEVEL_A);
    packetPageReqMsg.RepeatedPageInfo[2].RRConnPage.isValid = TRUE;
    packetPageReqMsg.RepeatedPageInfo[2].isValid = TRUE;
    
    packetPageReqMsg.EncodeMsg(outStream);
    packetPageReqMsg.DisplayMsgShort(&outStream);
    packetPageReqMsg.DisplayMsgDetail();
}


// *******************************************************************
// PACKET PDCH RELEASE
//
// GSM 04.60 11.2.11
//
// < Packet PDCH Release message content > ::=
//   < PAGE_MODE : bit (2) >
//   { 0 | 1 < TIMESLOTS_AVAILABLE : bit (8) > }
//   < padding bits >
//    ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************
void TestPacketPDCHReleaseMsg()
{
    BitStreamOut outStream;

    MsgPacketPDCHRelease  packetPDCHRelMsg;
   
    packetPDCHRelMsg.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    packetPDCHRelMsg.tsAllocation.SetTimeslotAllocation(0x2);
   
    packetPDCHRelMsg.EncodeMsg(outStream);
    packetPDCHRelMsg.DisplayMsgShort(&outStream);
    packetPDCHRelMsg.DisplayMsgDetail();
}


// *******************************************************************
// PACKET POLLING REQUEST
//
// GSM 04.60 11.2.12
//
// < Packet Polling Request message content > ::=
//   < PAGE_MODE : bit (2) >
//   {  { 0 < Global TFI : < Global TFI IE > >
//      | 10    < TLLI : bit (32) >
//      | 110   < TQI : bit (16) > }
//      < TYPE_OF_ACK : bit (1) >
//      {   < padding bits >
//          ! < Non-distribution part error : bit (*) = < no string > > }
//       ! < Address information part error : bit (*) = < no string > > }
//    ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************
void TestPacketPollingRequestMsg()
{
    BitStreamOut outStream;

    MsgPacketPollingRequest  packetPollReqMsg;
   
    packetPollReqMsg.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    packetPollReqMsg.tqi.SetTQI(0xfefe);
    packetPollReqMsg.typeOfAck = MsgPacketPollingRequest::ACK_MSG_RLCMAC_CONTROL_BLOCK;
   
    packetPollReqMsg.EncodeMsg(outStream);
    packetPollReqMsg.DisplayMsgShort(&outStream);
    packetPollReqMsg.DisplayMsgDetail();
}


// *******************************************************************
// PACKET POWER CONTROL / TIMING ADVANCE
//
// GSM 04.60 11.2.13
//
// < Packet Power Control/Timing Advance message content > ::=
//   < PAGE_MODE : bit (2) >
//   {   { 0    < Global TFI : < Global TFI IE > >
//       | 110  < TQI : bit (16) >
//       | 111  < Packet Request Reference : < Packet Request Reference IE > > }
//       { 0    -- Message escape
//          {   { 0 | 1 < Global Power Control Parameters : < Global Power Control Parameters IE >> }
//              { 0 < Global Packet Timing Advance : < Global Packet Timing Advance IE > >
//                  < Power Control Parameters : < Power Control Parameters IE > >
//               | 1 { 0 < Global Packet Timing Advance : < Global Packet Timing Advance IE > >
//                   | 1 < Power Control Parameters : < Power Control parameters IE > > } }
//              < padding bits >
//               ! < Non-distribution part error : bit (*) = < no string > > }
//           ! < Message escape : 1 bit (*) = <no string> > }
//       ! < Address information part error : bit (*) = < no string > > }
//   ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************
void TestPacketPowerCtlTimingAdvanceMsg()
{
    BitStreamOut outStream;

    MsgPacketPowerCtlTimingAdvance  packetPwrTAMsg;
   
    packetPwrTAMsg.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    packetPwrTAMsg.tqi.SetTQI(0xabcd);
    packetPwrTAMsg.globalPowerControlParams.SetAlpha(12);
    packetPwrTAMsg.globalPowerControlParams.SetTAvgW(28);
    packetPwrTAMsg.globalPowerControlParams.SetTAvgT(18);
    packetPwrTAMsg.globalPowerControlParams.SetPb(6);
    packetPwrTAMsg.globalPowerControlParams.SetPcMeasChan(
        GlobalPowerControlParms::DL_MEAS_ON_BCCH);
    packetPwrTAMsg.globalPowerControlParams.SetNAvgI(7);
    packetPwrTAMsg.globalPowerControlParams.SetIntMeasChanAvail(
        GlobalPowerControlParms::PSI4_MSG_BROADCAST);
    packetPwrTAMsg.globalPowerControlParams.SetGlobalPowerControlParms();
    packetPwrTAMsg.globalTA.SetGlobalPacketTimingAdvanceUplink(3,1);
    packetPwrTAMsg.globalTA.SetGlobalPacketTimingAdvanceDownlink(5,2);
   
    packetPwrTAMsg.EncodeMsg(outStream);
    packetPwrTAMsg.DisplayMsgShort(&outStream);
    packetPwrTAMsg.DisplayMsgDetail();
}


// *******************************************************************
// PACKET TBF RELEASE
//
// GSM 04.60 11.2.26
//
// < Packet TBF Release message content > ::=
//   < PAGE_MODE : bit (2) >
//   {  0 < GLOBAL_TFI : Global TFI IE >
//      {   < UPLINK_RELEASE : bit (1) >
//          < DOWNLINK_RELEASE : bit (1) >
//          < TBF_RELEASE_CAUSE : bit (4)  = { 0000 | 0010 } >
//          < padding bits >
//          ! < Non-distribution part error : bit (*) = < no string > > }
//      ! < Address information part error : bit (*) = < no string > > }
//   ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************
void TestPakcetTBFReleaseMsg()
{
    BitStreamOut outStream;

    MsgPacketTBFRelease  packetTBFRelMsg;
   
    packetTBFRelMsg.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    packetTBFRelMsg.globalTFI.SetGlobalTFI(16, GlobalTFI::UPLINK_TFI);
    packetTBFRelMsg.uplinkRel = MsgPacketTBFRelease::TBF_NOT_RELEASED;
    packetTBFRelMsg.downlinkRel = MsgPacketTBFRelease::TBF_RELEASED;
    packetTBFRelMsg.relCause = MsgPacketTBFRelease::NORMAL_RELEASE;
   
    packetTBFRelMsg.EncodeMsg(outStream);
    packetTBFRelMsg.DisplayMsgShort(&outStream);
    packetTBFRelMsg.DisplayMsgDetail();
}


// *******************************************************************
// PACKET DOWNLINK DUMMY CONTROL BLOCK
//
// GSM 04.60 11.2.7
//
// < Packet Downlink Dummy Control Block message content > ::=
//   < PAGE_MODE : bit (2) >
//   { 0 | 1 <PERSISTENCE_LEVEL : bit (4) > * 4 }
//   < padding bits >
//    ! < Distribution part error : bit (*) = < no string > > ;
//
// *******************************************************************
void TestPacketDownlinkDummyControlBlockMsg()
{
    BitStreamOut outStream;

    MsgPacketDownlinkDummyControlBlock  packetDlDummyCtlBlk;
   
    packetDlDummyCtlBlk.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    unsigned char persLev[4] = {1,2,3,4};
    packetDlDummyCtlBlk.persistenceLevel.SetPersistenceLevel(persLev);
   
    packetDlDummyCtlBlk.EncodeMsg(outStream);
    packetDlDummyCtlBlk.DisplayMsgShort(&outStream);
    packetDlDummyCtlBlk.DisplayMsgDetail();
}


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
void TestPacketUplinkAssignmentMsg()
{
    BitStreamOut outStream;

    MsgPacketUplinkAssignment  packetULAssignment;
   
    packetULAssignment.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    unsigned char persLev[4] = {1,2,3,4};
    packetULAssignment.persistenceLevel.SetPersistenceLevel(persLev);
    packetULAssignment.globalTFI.SetGlobalTFI(11, GlobalTFI::UPLINK_TFI);
    packetULAssignment.chanCoding.SetChannelCodingCommand(
        ChannelCodingCommand::CS_1);
    packetULAssignment.tlliBlockCoding = 
        MsgPacketUplinkAssignment::USE_CS1_FOR_DATA_BLK_WITH_TLLI;
    packetULAssignment.timingAdvance.SetPacketTimingAdvance(0);
    packetULAssignment.dynamicAllocation.allocType = 
        DynamicAllocationStruct::EXTENDED_DYNAMIC_ALLOCATION;
    packetULAssignment.dynamicAllocation.usfGranularity = 
        DynamicAllocationStruct::ONE_RLC_MAC_BLOCK;
    packetULAssignment.dynamicAllocation.uplinkTFI.SetTFI(22);
    packetULAssignment.dynamicAllocation.rlcBlocksGranted.SetRLCBlocksGranted(100);
    packetULAssignment.dynamicAllocation.timeslotAllocation.isValid = TRUE;
    TimeslotAllocationEntry entries[8] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,7}};
    packetULAssignment.dynamicAllocation.timeslotAllocation.tsAllocEntry = entries;
    packetULAssignment.dynamicAllocation.isValid = TRUE;
    
    packetULAssignment.EncodeMsg(outStream);
    packetULAssignment.DisplayMsgShort(&outStream);
    packetULAssignment.DisplayMsgDetail();
}


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
void TestPacketTimeslotReconfigureMsg()
{
    BitStreamOut outStream;

    MsgPacketTimeslotReconfigure  packetTSRecon;
   
    packetTSRecon.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    packetTSRecon.globalTFI.SetGlobalTFI(11, GlobalTFI::UPLINK_TFI);
    packetTSRecon.chanCoding.SetChannelCodingCommand(
        ChannelCodingCommand::CS_4);
    packetTSRecon.globalTimingAdvance.SetGlobalPacketTimingAdvanceUplink(0,2);
    packetTSRecon.globalTimingAdvance.SetGlobalPacketTimingAdvanceDownlink(1,2);
    packetTSRecon.dlRlcMode.SetRlcMode(RlcMode::RLC_UNACKNOWLEDGED);
    packetTSRecon.controlAck = TRUE;
    packetTSRecon.dlTfi.SetTFI(31);
    packetTSRecon.dlTimeslotAllocation.SetTimeslotAllocation(3);
    packetTSRecon.dynamicAllocation.allocType = 
        DynamicAllocationStruct::EXTENDED_DYNAMIC_ALLOCATION;
    packetTSRecon.dynamicAllocation.usfGranularity = 
        DynamicAllocationStruct::ONE_RLC_MAC_BLOCK;
    packetTSRecon.dynamicAllocation.uplinkTFI.SetTFI(22);
    packetTSRecon.dynamicAllocation.rlcBlocksGranted.SetRLCBlocksGranted(100);
    packetTSRecon.dynamicAllocation.timeslotAllocation.isValid = TRUE;
    TimeslotAllocationEntry entries[8] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,7}};
    packetTSRecon.dynamicAllocation.timeslotAllocation.tsAllocEntry = entries;
    packetTSRecon.dynamicAllocation.isValid = TRUE;
    
    packetTSRecon.EncodeMsg(outStream);
    packetTSRecon.DisplayMsgShort(&outStream);
    packetTSRecon.DisplayMsgDetail();
}


// *******************************************************************
// PACKET UPLINK ACK NACK
//
// GSM 04.60 11.2.28
//
// < Packet Uplink Ack/Nack message content > ::=
//   < PAGE MODE : bit (2) >
//   {  00 < UPLINK_TFI : bit (5) >
//       { 0        -- Message escape
//           {   < CHANNEL_CODING_COMMAND : bit (2) >
//               < Ack/Nack Description : < Ack/Nack Description IE > >
//               { 0 | 1    < CONTENTION_RESOLUTION_TLLI : bit (32) > }
//               { 0 | 1    < Packet Timing Advance : < Packet Timing Advance IE > > }
//               { 0 | 1    < Power Control Parameters : < Power Control Parameters IE > > }
//               { 0 | 1    < Extension Bits : Extension Bits IE > }                -- sub-clause 12.26
//               { 0 | 1    < Fixed Allocation Parameters : < Fixed Allocation struct > > }
//               < padding bits >
//               ! < Non-distribution part error : bit (*) = < no string > > }
//           ! < Message escape : 1 bit (*) = <no string> > }
//       ! < Address information part error : bit (*) = < no string > > }
//   ! < Distribution part error : bit (*) = < no string > > ;
//
// < Fixed Allocation struct > ::=
//   < FINAL_ALLOCATION : bit (1) >
//   { 0    -- Repeat Allocation
//       < TS_OVERRIDE : bit (8) >
//   | 1        -- Allocation with Allocation bitmap
//       < TBF Starting Time : < Starting Frame Number Description IE > >
//       { 0 | 1    <TIMESLOT_ALLOCATION : bit (8) > }
//       { 0 { 0        -- with length of Allocation Bitmap
//               < BLOCKS_OR_BLOCK_PERIODS : bit (1) >
//               < ALLOCATION_BITMAP_LENGTH : bit (7) >
//               < ALLOCATION_BITMAP : bit (val(ALLOCATION_BITMAP_LENGTH)) >
//           | 1        -- without length of Allocation Bitmap (fills remainder of the message)
//               < ALLOCATION_BITMAP : bit ** > } } 
//            ! < Message escape : 1 bit (*) = <no string> >;
//
// *******************************************************************
void TestPacketUplinkAckNackMsg()
{
    BitStreamOut outStream;

    MsgPacketUplinkAckNack  packetUplinkAckNack;
   
    packetUplinkAckNack.pageMode.SetPageMode(PageMode::NORMAL_PAGING);
    packetUplinkAckNack.uplinkTFI.SetTFI(0xd);
    packetUplinkAckNack.chanCoding.SetChannelCodingCommand(
        ChannelCodingCommand::CS_2);
    unsigned char rrb[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    packetUplinkAckNack.ackNackDescription.SetAckNackDescription(AckNackDescription::TBF_COMPLETE, 
        (unsigned char)15, rrb);
    packetUplinkAckNack.timingAdvance.SetPacketTimingAdvance(2);
    
    packetUplinkAckNack.EncodeMsg(outStream);
    packetUplinkAckNack.DisplayMsgShort(&outStream);
    packetUplinkAckNack.DisplayMsgDetail();
}



// *******************************************************************
// PACKET CELL CHANGE FAILURE
//
// GSM 04.60 11.2.26
//
// < Packet Cell Change Failure message content > ::=
//	 < TLLI : bit (32) >
//	 < ARFCN : bit (10) >
//	 < BSIC : bit (6) >
//	 < CAUSE : bit (4) >
//	 < padding bits > ;
//
// *******************************************************************
void TestPacketCellChangeFailureMsg()
{
    BitStreamOut outStream;
    MsgPacketCellChangeFailure packetCellChangeFailure;
    
    // Add msg type
    outStream.InsertBits8(RlcMacMsgType::PACKET_CELL_CHANGE_FAILURE, 6);    
    // Add a tlli
    outStream.InsertBits32(0xabcd, 32);    
    // Add ARFCN
    outStream.InsertBits16(0x1f, 10);    
    // Add BSIC
    outStream.InsertBits8(4, 6);    
    // Add cause
    outStream.InsertBits8(0, 4);    
    outStream.PadToOctetBoundary();  
    
    BitStreamIn inStream(outStream.GetBitStream(), outStream.GetStreamLen());
    
    packetCellChangeFailure.DecodeMsg(inStream);
    packetCellChangeFailure.DisplayMsgShort(&outStream);
    packetCellChangeFailure.DisplayMsgDetail();
}


// *******************************************************************
// PACKET CONTROL ACKNOWLEDGEMENT
//
// GSM 04.60 11.2.2
//
// < Packet Control Acknowledgement message content > ::= -- RLC/MAC control block format   
//      < TLLI : bit (32) > 
//      < CTRL_ACK : bit (2) >  
//      < padding bits > ;
//
// < Packet Control Acknowledgement 11 bit message > ::=  -- 11-bit access burst format 
//      < MESSAGE_TYPE : bit (9) == 1111 1100 1 >   
//      < CTRL_ACK : bit (2) > ;
//
// < Packet Control Acknowledgement 8 bit message > ::=   -- 8-bit access burst format  
//      < MESSAGE_TYPE : bit (6) == 0111 11 >   
//      < CTRL_ACK : bit (2) > ;
//
// *******************************************************************
void TestPacketControlAcknowledgementMsg()
{
    BitStreamOut outStream;
    MsgPacketControlAcknowledgement packetControlAcknowledgement;
    
    // Add msg type
    outStream.InsertBits8(RlcMacMsgType::PACKET_CONTROL_ACK, 6);    
    // Add a tlli
    outStream.InsertBits32(0x1234abcd, 32);    
    // Add control ack
    outStream.InsertBits8(1, 2);    
    outStream.PadToOctetBoundary();  
    
    BitStreamIn inStream(outStream.GetBitStream(), outStream.GetStreamLen());
    
    packetControlAcknowledgement.DecodeMsg(inStream);
    packetControlAcknowledgement.DisplayMsgShort(&outStream);
    packetControlAcknowledgement.DisplayMsgDetail();
}


// *******************************************************************
// PACKET DOWNLINK ACK NACK
//
// GSM 04.60 11.2.6
//
// < Packet Downlink Ack/Nack message content > ::=	
//      < DOWNLINK_TFI : bit (5) >	
//      < Ack/Nack Description : < Ack/Nack Description IE > >	
//      { 0 | 1 < Channel Request Description : < Channel Request Description IE > > }	
//      < Channel Quality Report : < Channel Quality Report struct > >	
//      < padding bits > ;
//
//      < Channel Quality Report struct > ::=	
//          < C_VALUE : bit (6) >	
//          < RXQUAL : bit (3) >	
//          < SIGN_VAR : bit (6) >	
//          { 0 | 1 < I_LEVEL_TN0 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN1 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN2 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN3 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN4 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN5 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN6 : bit (4) > }	
//          { 0 | 1 < I_LEVEL_TN7 : bit (4) > } ;
//
// *******************************************************************
void TestPacketDownlinkAckNackMsg()
{
    BitStreamOut outStream;
    MsgPacketDownlinkAckNack packetDownlinkAckNack;
    
    // Add msg type
    outStream.InsertBits8(RlcMacMsgType::PACKET_DOWNLINK_ACK_NACK, 6);    
    // Add a downlink tfi
    outStream.InsertBits8(0xa, 5);    
    // Add Ack/Nack Description
    outStream.InsertBits8(AckNackDescription::TBF_INCOMPLETE, 1);
    outStream.InsertBits8(0x35, 7);
    unsigned char recBlkBmap[RRB_SIZE];
    for (int i=0; i<RRB_SIZE; i++)
    {
        recBlkBmap[i] = i;
    }
    outStream.InsertBytesN(recBlkBmap, RRB_SIZE);
    
    // Add in Channel Request Description
    outStream.InsertBits8(1, 1);
    outStream.InsertBits8(5, 4);
    outStream.InsertBits8(0, 2);
    outStream.InsertBits8(RlcMode::RLC_ACKNOWLEDGED, 1);
    outStream.InsertBits8(ChannelRequestDescription::NOT_SACK_OR_ACK, 1);
    outStream.InsertBits16(0x9876, 16);
    
    // Add in Channel Quality Report
    outStream.InsertBits8(1, 6);
    outStream.InsertBits8(2, 3);
    outStream.InsertBits8(3, 6);
    for (i=0; i < MAX_TIMESLOTS-1; i++)
    {
        outStream.InsertBits8(0, 1);
    }
    outStream.InsertBits8(1, 1); 
    outStream.InsertBits8(3, 4);
                                                                      
    outStream.PadToOctetBoundary();  
    
    BitStreamIn inStream(outStream.GetBitStream(), outStream.GetStreamLen());
    
    packetDownlinkAckNack.DecodeMsg(inStream);
    packetDownlinkAckNack.DisplayMsgShort(&outStream);
    packetDownlinkAckNack.DisplayMsgDetail();
}

// *******************************************************************
// PACKET UPLINK DUMMY CONTROL BLOCK
//
// GSM 04.60 11.2.8b
//
// < Packet Uplink Dummy Control Block message content > ::=	
//      < TLLI : bit (32) >	
//      < padding bits > ;
//
// *******************************************************************
void TestPacketUplinkDummyControlBlockMsg()
{
    BitStreamOut outStream;
    MsgPacketUplinkDummyControlBlock packetUplinkDummyControlBlock;
    
    // Add msg type
    outStream.InsertBits8(RlcMacMsgType::PACKET_UPLINK_DUMMY_CONTROL_BLOCK, 6);    
    // Add a downlink tlli
    outStream.InsertBits32(0x1234abcd, 32);    
                                                                      
    outStream.PadToOctetBoundary();  
    
    BitStreamIn inStream(outStream.GetBitStream(), outStream.GetStreamLen());
    
    packetUplinkDummyControlBlock.DecodeMsg(inStream);
    packetUplinkDummyControlBlock.DisplayMsgShort(&outStream);
    packetUplinkDummyControlBlock.DisplayMsgDetail();
}


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
void TestPacketResourceRequestMsg()
{
    BitStreamOut outStream;
    MsgPacketResourceRequest packetResourceRequest;
    
    // Add msg type
    outStream.InsertBits8(RlcMacMsgType::PACKET_RESOURCE_REQUEST, 6);   
    // Add ACCESS_TYPE
    outStream.InsertBits8(1,1);
    outStream.InsertBits8(AccessType::TWO_PHASE,2);
     
    // Add a downlink global tfi
    outStream.InsertBits8(0,1);
    outStream.InsertBits8(1,1);
    outStream.InsertBits8(0x5, 5);    
    // Add MS Radio Access Capability
    outStream.InsertBits8(1,1);
    //   Access Technology Type
    outStream.InsertBits8(MSRadioAccessCapability::GSM_1900, 4);
    //   Length of Content Struct
    outStream.InsertBits8(37,7);
    
    //   RF Power Capability
    outStream.InsertBits8(2, 3);
    //   A5 bits
    outStream.InsertBits8(1,1);
    outStream.InsertBits8(0x60,7);
    //   ES IND
    outStream.InsertBits8(MSRadioAccessCapability::EARLY_CLASSMARK_SENDING_NOT_IMPLEMENTED,1);
    //   PS
    outStream.InsertBits8(MSRadioAccessCapability::PS_PRESENT,1);
    //   VGCS
    outStream.InsertBits8(MSRadioAccessCapability::VGCS_CAPABILITY,1);
    //   VBS
    outStream.InsertBits8(MSRadioAccessCapability::VBS_CAPABILITY,1);
    //   Multislot class
    outStream.InsertBits8(1,1);
    //       HSCSD multislot class
    outStream.InsertBits8(1,1);
    outStream.InsertBits8(18,5);
    //       GPRS multislot class and GPRS Extended Dynamic Allocation Capability
    outStream.InsertBits8(1,1);
    outStream.InsertBits8(5,5);
    outStream.InsertBits8(1,1);
    //       SMS_VALUE and SM_VALUE
    outStream.InsertBits8(1,1);
    outStream.InsertBits8(0xf,4);
    outStream.InsertBits8(0,4);
    
    // Only one MS RA capability  value part struct
    outStream.InsertBits8(0,1);
    
    // Add in Channel Request Description
    outStream.InsertBits8(5, 4);
    outStream.InsertBits8(0, 2);
    outStream.InsertBits8(RlcMode::RLC_ACKNOWLEDGED, 1);
    outStream.InsertBits8(ChannelRequestDescription::NOT_SACK_OR_ACK, 1);
    outStream.InsertBits16(0x9876, 16);
    
    // Add in Change Mark
    outStream.InsertBits8(1, 1);
    outStream.InsertBits8(1, 2);
    // Add in C_VALUE
    outStream.InsertBits8(2, 6);
    // Add in SIGN_VAR
    outStream.InsertBits8(1, 1);
    outStream.InsertBits8(0x12, 6);
    // Add in I_LEVEL_TNX values
    outStream.InsertBits8(1, 1); 
    outStream.InsertBits8(1, 4);
    for (int i=1; i < MAX_TIMESLOTS; i++)
    {
        outStream.InsertBits8(0, 1);
    }
                                                                      
    outStream.PadToOctetBoundary();  
    
    BitStreamIn inStream(outStream.GetBitStream(), outStream.GetStreamLen());
    
    packetResourceRequest.DecodeMsg(inStream);
    packetResourceRequest.DisplayMsgShort(&outStream);
    packetResourceRequest.DisplayMsgDetail();
}

// *******************************************************************
// PACKET MOBILE TBF STATUS
//
// GSM 04.60 11.2.9c
//
// < Packet Mobile TBF Status message content > ::=	
//      < GLOBAL TFI : < Global TFI IE > >	
//      < TBF_CAUSE : bit (3) >	
//      { 0 | 1	< STATUS_MESSAGE_TYPE : bit (6) > }	
//      < padding bits > ;
//
// *******************************************************************
void TestPacketMobileTBFStatusMsg()
{
    BitStreamOut outStream;
    MsgPacketMobileTBFStatus packetMobileTBFStatus;
    
    // Add msg type
    outStream.InsertBits8(RlcMacMsgType::PACKET_MOBILE_TBF_STATUS, 6);    
    // Add a uplink global tfi
    outStream.InsertBits8(0,1);
    outStream.InsertBits8(0x12, 5); 
    // Add TBF_CAUSE   
    outStream.InsertBits8(MsgPacketMobileTBFStatus::NON_DISTRIBUTION_TYPE,3);
    // Add STATUS_MESSAGE_TYPE 
    outStream.InsertBits8(1,1);
    outStream.InsertBits8(RlcMacMsgType::PACKET_TIMESLOT_RECONFIGURE, 6);    
                                                                      
    outStream.PadToOctetBoundary();  
    
    BitStreamIn inStream(outStream.GetBitStream(), outStream.GetStreamLen());
    
    packetMobileTBFStatus.DecodeMsg(inStream);
    packetMobileTBFStatus.DisplayMsgShort(&outStream);
    packetMobileTBFStatus.DisplayMsgDetail();
}


void RlcMacMsgTest()
{
    TestPacketAccessRejectMsg() ;
    TestPacketDownlinkAssignmentMsg();
    TestPacketPagingRequestMsg();
}



void RlcMacIeTest()
{

}