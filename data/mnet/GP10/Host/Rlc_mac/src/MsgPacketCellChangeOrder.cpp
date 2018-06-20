// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketCellChangeOrder.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketCellChangeOrder.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

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
//   < CELL_BAR_ACCESS_2 : bit (1)MsgPacketCellChangeOrder >
//   < EXC_ACC : bit >
//   < SAME_RA_AS_SERVING_CELL : bit (1) >
//   { 0 | 1 < GPRS_RXLEV_ACCESS_MIN : bit (6) >
//           < GPRS_MS_TXPWR_MAX_CCH : bit (5) > }
//   { 0 | 1 < GPRS_TEMPORARY_OFFSET : bit (3) >
//           < GPRS_PENALTY_TIME : bit (5) > }
//   { 0 | 1    < GPRS_RESELECT_OFFSET : bit (5) > }
//   { 0 | 1    < HCS params : < HCS struct > > }
//   { 0 | 1 < SI13_PBCCH_LOCATION : < SI13_PBCCH_LOCATION struct > > } ;
//
// < SI13_PBCCH_LOCATION struct > ::=
//   { 0    < SI13_LOCATION : bit (1) >
//    | 1   < PBCCH_LOCATION : bit (2) >
//       < PSI1_REPEAT_PERIOD : bit (4) > } ;
//
// < HCS struct > ::=
//   < PRIORITY_CLASS : bit (3) >
//   < HCS_THR : bit (5) > ;
// *******************************************************************




RlcMacResult MsgPacketCellChangeOrder::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketCellChangeOrder::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode the Page Mode ie.
    //      < PAGE_MODE : bit (2) >
    result = pageMode.EncodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode Global TFI or TLLI.
    //  {   { 0 < Global TFI : < Global TFI IE > >
    //       | 10   < TLLI : bit (32) > }
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
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("MsgPacketCellChangeOrder::EncodeMsg Global TFI, "
            "or TLLI must be valid\n");
    }   
    // Encode message escape
    //      { 0     -- Message escape
    dataStream.InsertBits8(0, 1);
   
    // Encode IMMEDIATE_REL
    //          {   < IMMEDIATE_REL : bit >
    dataStream.InsertBits8(immediateRel, 1);
    
    // Encode ARFCN.
    //              < ARFCN : bit (10) >
    result = arfcn.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
   
    // Encode BSIC.
    //              < BSIC : bit (6) >
    result = bsic.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    // Encode NC Measurement Parameters struct
    //              < NC Measurement Parameters : < NC Measurement Parameters struct > >
    result = ncMeasParam.EncodeNCMeasParamStruct(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    // Encode LSA Parameters ie
    //              { 0 | 1 < LSA Parameters : < LSA Parameters IE >> }
    if (lsaParameters.IsValid())
    {
        dataStream.InsertBits8(1,1);
        result = lsaParameters.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        dataStream.InsertBits8(0,1);
    }
    
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return (result);
}


char immediateRelNames[][64] = 
{
    "Not required",        
    "Required"      
};


void MsgPacketCellChangeOrder::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketCellChangeOrder::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayDownlinkMsgType());
      
        // Print out the Page Mode.
        pageMode.DisplayDetails(&MsgDisplayDetailObj);
      
        // Print out the Global TFI or TLLI.
        if (globalTFI.IsValid())
        {
            globalTFI.DisplayDetails(&MsgDisplayDetailObj);
        }
        else if (tlli.IsValid())
        {
            tlli.DisplayDetails(&MsgDisplayDetailObj);
        }
        
        // Print out the IMMEDIATE_REL
        MsgDisplayDetailObj.Trace("\tIMMEDIATE REL ---> %s\n", immediateRelNames[immediateRel]);
        
        // Print out the ARFCN
        arfcn.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out the BSIC
        bsic.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out the NC Measurement Parameters struct
        ncMeasParam.DisplayNCMeasParamStruct(&MsgDisplayDetailObj);      
    }
   
    DBG_LEAVE();
}

