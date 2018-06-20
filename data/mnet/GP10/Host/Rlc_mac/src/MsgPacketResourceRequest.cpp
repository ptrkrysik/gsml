// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketResourceRequest.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketResourceRequest.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

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

RlcMacResult MsgPacketResourceRequest::DecodeMsg(BitStreamIn &dataStream)
{
    DBG_FUNC("MsgPacketResourceRequest::DecodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;
   
    // Message Type decoded earlier
   
    // Decode the ACCESS TYPE.
    //      { 0 | 1 < ACCESS_TYPE : bit (2) > }	
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    if (val == 1)
    {
        result = accessType.DecodeIe(dataStream); 
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        accessType.ClearAccessType();
    }
   
    // Decode the Global TFI or TLLI ie.
    //      { 0 < Global TFI : < Global TFI IE > >	
    //      | 1 < TLLI : < TLLI IE > > }
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    if (val == 0)
    {	
        result = globalTfi.DecodeIe(dataStream); 
        RLC_MAC_RESULT_CHECK(result);
        tlli.ClearTLLI();
    }
    else
    {
        result = tlli.DecodeIe(dataStream); 
        RLC_MAC_RESULT_CHECK(result);   
        globalTfi.ClearGlobalTFI();
    }
    
    // Decode the MS Radio Access Capability ie if present.
    //      { 0 | 1 < MS Radio Access Capability : < MS Radio Access Capability IE > > }	
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    if (val == 1)
    {
        result = msRadioAccessCapability.DecodeIe(dataStream); 
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        msRadioAccessCapability.ClearMSRadioAccessCapability();
    }
   
    // Decode the channel request description ie.
    //      < Channel Request Description : < Channel Request Description IE > >	
    result = chanReqDescription.DecodeIe(dataStream); 
    RLC_MAC_RESULT_CHECK(result);                     
   
    // Decode the Change Mark ie if present.
    //      { 0 | 1 < CHANGE_MARK : bit (2) > }	
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    if (val == 1)
    {
        result = changeMark.DecodeIe(dataStream); 
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        changeMark.ClearChangeMark();
    }
   
    // Decode the C_VALUE.
    //	 < C_VALUE : bit (6) >
    result = dataStream.ExtractBits8(cValue, 6);
    RLC_MAC_RESULT_CHECK(result);
    
    // Decode SIGN_VAR  
    //      { 0 | 1 < SIGN_VAR : bit (6) >}	
    result = dataStream.ExtractBits8(val, 1);
    RLC_MAC_RESULT_CHECK(result);
    if (val == 1)
    {
        result = dataStream.ExtractBits8(SignVar.signVar, 6); 
        RLC_MAC_RESULT_CHECK(result);
        SignVar.isValid = TRUE;
    }
    else
    {
        SignVar.isValid = FALSE;
    }
      
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


void MsgPacketResourceRequest::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketResourceRequest::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayUplinkMsgType());
      
        // Print out ACCESS TYPE.
        accessType.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out the Global TFI or TLLI.
        if (globalTfi.IsValid())
        {
            globalTfi.DisplayDetails(&MsgDisplayDetailObj);
        }
        else
        {
            tlli.DisplayDetails(&MsgDisplayDetailObj);
        }
              
        // Print out the MS Radio Access Capability
        msRadioAccessCapability.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out the Channel Request Description
        chanReqDescription.DisplayDetails(&MsgDisplayDetailObj);
       
        // Print out the Change Mark
        changeMark.DisplayDetails(&MsgDisplayDetailObj);
        
        // Print out the C_VALUE
        MsgDisplayDetailObj.Trace("\tC_VALUE ---> %d\n", cValue);
        
        // Print out the SIGN_VAR
        if (SignVar.isValid)
        {
            MsgDisplayDetailObj.Trace("\tSIGN_VAR ---> %d\n", SignVar.signVar);
        }
        
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

