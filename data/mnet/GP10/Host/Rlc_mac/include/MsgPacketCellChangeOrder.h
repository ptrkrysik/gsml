// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketCellChangeOrder.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETCELLCHANGEORDER_H__
#define __MSGPACKETCELLCHANGEORDER_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "ieTLLI.h"
#include "ieGlobalTFI.h"
#include "NCMeasurementStruct.h" 
#include "ieLSAParameters.h"
#include "ieARFCN.h"
#include "ieBSIC.h"




// *******************************************************************
// PACKET CELL CHANGE ORDER
//
// GSM 04.60 11.2.4
//
// < Packet Cell Change Order message content > ::=
//  < PAGE_MODE : bit (2) >
//  {   { 0 < Global TFI : < Global TFI IE > >
//       | 10   < TLLI : bit (32) > }
//      { 0     -- Message escape
//          {   < IMMEDIATE_REL : bit >
//              < ARFCN : bit (10) >
//              < BSIC : bit (6) >
//              < NC Measurement Parameters : < NC Measurement Parameters struct > >
//              { 0 | 1 < LSA Parameters : < LSA Parameters IE >> }
//              < padding bits > 
//               ! < Non-distribution part error : bit (*) = < no string > > }
//           ! < Message escape : 1 bit (*) = <no string> > }
//       ! < Address information part error : bit (*) = < no string > > }
//   ! < Distribution part error : bit (*) = < no string > > ;
//
// < NC Measurement Parameters struct > ::=
//   < NETWORK_CONTROL_ORDER : bit (2) >
//   { 0 | 1 < NC_NON_DRX_PERIOD : bit (3) >
//           < NC_REPORTING_PERIOD_I : bit (3) >
//           < NC_REPORTING_PERIOD_T : bit (3) > }
//   { 0 | 1 < NC_FREQUENCY_LIST : NC Frequency list struct > } ;
//
// < NC Frequency list struct > ::=
//   { 0 | 1     < NR_OF_REMOVED_FREQ : bit (5) >
//           { < REMOVED_FREQ_INDEX : bit (6) > } * (1 + val(NR_OF_REMOVED_FREQ)) }
//   { 1 < List of added Frequency : < Add Frequency list struct > >} ** 0;
//
// < Add Frequency list struct > ::=
//   < START_FREQUENCY : bit (10) >
//   < BSIC : bit (6) >
//   { 0 | 1 < Cell selection params : < Cell Selection struct > > }
//           < NR_OF_FREQUENCIES : bit (5) >
//           < FREQ_DIFF_LENGTH : bit (3) >
//           { < FREQUENCY_DIFF : bit (val(FREQ_DIFF_LENGTH)) >
//           < BSIC : bit (6) >
//           { 0 | 1    < Cell selection params : 
//                       < Cell Selection struct > > } } * (val(NR_OF_FREQUENCIES));
//
// < Cell Selection struct > ::=
//   < CELL_BAR_ACCESS_2 : bit (1) >
//   < EXC_ACC : bit >
//   < SAME_RA_AS_SERVING_CELL : bit (1) >
//   { 0 | 1 < GPRS_RXLEV_ACCESS_MIN : bit (6) >
//           < GPRS_MS_TXPWR_MAX_CCH : bit (5) > }
//   { 0 | 1 < GPRS_TEMPORARY_OFFSET : bit (3) >
//           < GPRS_PENALTY_TIME : bit (5) > }
//	 { 0 | 1	< GPRS_RESELECT_OFFSET : bit (5) > }
//	 { 0 | 1	< HCS params : < HCS struct > > }
//	 { 0 | 1 < SI13_PBCCH_LOCATION : < SI13_PBCCH_LOCATION struct > > } ;
//
// < SI13_PBCCH_LOCATION struct > ::=
//	 { 0	< SI13_LOCATION : bit (1) >
//	  | 1	< PBCCH_LOCATION : bit (2) >
//		 < PSI1_REPEAT_PERIOD : bit (4) > } ;
//
// < HCS struct > ::=
//	 < PRIORITY_CLASS : bit (3) >
//	 < HCS_THR : bit (5) > ;
// *******************************************************************


class MsgPacketCellChangeOrder : public DownlinkMsgBase {
public:

    typedef enum
    {
        NOT_REQUIRED,
        REQUIRED
    } IMMEDIATE_REL;

    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode          pageMode;
    IeGlobalTFI         globalTFI;
    IeTLLI              tlli;
    IMMEDIATE_REL       immediateRel;
    IeARFCN             arfcn;
    IeBSIC              bsic;
    NCMeasParamStruct   ncMeasParam;
    IeLSAParameters     lsaParameters;
       
    // Default constructor
    MsgPacketCellChangeOrder() : DownlinkMsgBase(RlcMacMsgType::PACKET_CELL_CHANGE_ORDER),
        pageMode(), globalTFI(), tlli(), immediateRel(NOT_REQUIRED), arfcn(), bsic(),
        ncMeasParam(), lsaParameters() {}
   
    // Default destructor
    ~MsgPacketCellChangeOrder() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};


#endif