// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketDownlinkAckNack.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketDownlinkAckNack.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

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

RlcMacResult MsgPacketDownlinkAckNack::DecodeMsg(BitStreamIn &dataStream)
{
    DBG_FUNC("MsgPacketDownlinkAckNack::DecodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;
   
    // Message Type decoded earlier.
   
    // Decode the Downlink TFI ie.
    //	 < DOWNLINK_TFI : bit (5) >
    result = dlTfi.DecodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
    
    // Decode the Ack Nack Descrition ie.
    //	 < Ack/Nack Description : < Ack/Nack Description IE > >
    result = ackNackDescription.DecodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);
   
    // Decode the channel request description ie.
    //	 { 0 | 1 < Channel Request Description : < Channel Request Description IE > > }
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    if (val)
    {
        result = chanReqDescription.DecodeIe(dataStream); 
        RLC_MAC_RESULT_CHECK(result);                     
    }
   
    // Decode the C_VALUE.
    //	 < C_VALUE : bit (6) >
    result = dataStream.ExtractBits8(cValue, 6);
    RLC_MAC_RESULT_CHECK(result);
   
    // Decode the RXQUAL.
    //	 < RXQUAL : bit (3) >
    result = dataStream.ExtractBits8(rxqual, 3);
    RLC_MAC_RESULT_CHECK(result);
   
    // Decode the SIGN_VAR.
    //	 < SIGN_VAR : bit (6) >
    result = dataStream.ExtractBits8(signVar, 6);
    RLC_MAC_RESULT_CHECK(result);
   
    // Decode the I_LEVEL for each ts.
    //	 { 0 | 1 < I_LEVEL_TNx : bit (4) > }
    for (int i=0; i < MAX_TIMESLOTS; i++)
    {
        result = dataStream.ExtractBits8(val, 1);
        RLC_MAC_RESULT_CHECK(result);
        if (val)
        {
            result = dataStream.ExtractBits8(ILevel[i].iLevel, 4);
            RLC_MAC_RESULT_CHECK(result);
            ILevel[i].isValid = TRUE;
            
        }
        else
        {
            ILevel[i].isValid = FALSE;
        }
    }
   
    DBG_LEAVE();
    return (result);
}


void MsgPacketDownlinkAckNack::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketDownlinkAckNack::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayUplinkMsgType());
      
        // Print out the TFI.
        dlTfi.DisplayDetails(&MsgDisplayDetailObj);
              
        // Print out the Ack Nack Descrition
        ackNackDescription.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out the Channel Request Description
        chanReqDescription.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out the C_VALUE
        MsgDisplayDetailObj.Trace("\tC_VALUE ---> %d\n", cValue);
        
        // Print out the RXQUAL
        MsgDisplayDetailObj.Trace("\tRXQUAL ---> %d\n", rxqual);
     
        // Print out the SIGN_VAR
        MsgDisplayDetailObj.Trace("\tSIGN_VAR ---> %d\n", signVar);
        
        // Print out the I_LEVEL for each ts.
        for (int i=0; i < MAX_TIMESLOTS; i++)
        {
            if (ILevel[i].isValid)
            {
                MsgDisplayDetailObj.Trace("\tI_LEVEL_TN%x ---> %d\n", i, ILevel[i].iLevel);
            }
        }
    }
   
    DBG_LEAVE();
}

