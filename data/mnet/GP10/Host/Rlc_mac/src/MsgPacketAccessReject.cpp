// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketAccessReject.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "MsgPacketAccessReject.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

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


RlcMacResult RejectStruct::EncodeRejectStruct(BitStreamOut &dataStream)
{
    DBG_FUNC("RejectStruct::EncodeReject", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result = RLC_MAC_SUCCESS;
    
    // Encode the reject struct
    //    < Reject : < Reject struct > >
    //    < Reject struct > ::=
    //      { 0 < TLLI : bit (32) >
    //       | 1    { 0 < Packet Request Reference : < Packet Request Reference IE > >
    //           | 1    < Global TFI : < Global TFI IE > > } }
    if (tlli.IsValid())
    {
        result = dataStream.InsertBits8(0, 1);
        result = tlli.EncodeIe(dataStream);
    }
    else if (packetReqRef.IsValid())
    {
        result = dataStream.InsertBits8(2, 2);
        result = packetReqRef.EncodeIe(dataStream);
    }
    else if (globalTFI.IsValid())
    {
        result = dataStream.InsertBits8(3, 2);
        result = globalTFI.EncodeIe(dataStream);
    }
    else
    {
        DBG_ERROR("RejectStruct::EncodeReject no valid tlli, packet req ref or tfi");    
        result = RLC_MAC_MISSING_IE;
    }
   
    //      { 0 | 1 < WAIT_INDICATION : bit (8) >
    //              < WAIT _INDICATION_SIZE : bit (1) > }
    if (result == RLC_MAC_SUCCESS)
    {
        if (waitInd.IsValid())
        {   
            result = dataStream.InsertBits8(1, 1);
            result = waitInd.EncodeIe(dataStream);
        }
        else
        {
            result = dataStream.InsertBits8(0, 1);
        }
    }
    
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
    return(result);
}


void RejectStruct::DisplayRejectStruct(DbgOutput *outObj)
{
    if (tlli.IsValid())
    {
        tlli.DisplayDetails(outObj);
    }
    else if (packetReqRef.IsValid())
    {
        packetReqRef.DisplayDetails(outObj);
    }
    else if (globalTFI.IsValid())
    {
        globalTFI.DisplayDetails(outObj);
    }

    if (waitInd.IsValid())
    {
        waitInd.DisplayDetails(outObj);
    }
}


RlcMacResult MsgPacketAccessReject::EncodeMsg(BitStreamOut &dataStream)
{
    DBG_FUNC("MsgPacketAccessReject::EncodeMsg", RLC_MAC);
    DBG_ENTER();

    RlcMacResult result;
   
    // Encode the Message Type.
    result = msgType.EncodeMsgType(dataStream);
   
    // Encode the Page Mode ie.
    //      < PAGE_MODE : bit (2) >
    result = pageMode.EncodeIe(dataStream); 
   
    // Encode reject.
    result = reject.EncodeRejectStruct(dataStream);
   
    // If there is a valid additional reject then encode it.
    if (additionalReject.isValid)
    {
        result = dataStream.InsertBits8(1, 1);
        result = additionalReject.EncodeRejectStruct(dataStream);
    }
   
    dataStream.PadToOctetBoundary();  
    DBG_LEAVE();
}



void MsgPacketAccessReject::DisplayMsgDetail()
{
    DBG_FUNC("MsgPacketAccessReject::DisplayMsgDetail", RLC_MAC);
    DBG_ENTER();

    if (viperlog_check_module_filter(RLC_MAC_MSG_DETAIL, LOG_SEVERITY_TRACE))
    {
        // Print out message type.
        MsgDisplayDetailObj.Trace(msgType.DisplayDownlinkMsgType());
      
        // Print out the Page Mode.
        pageMode.DisplayDetails(&MsgDisplayDetailObj);
      
        // Print out the reject struct
        reject.DisplayRejectStruct(&MsgDisplayDetailObj);
      
        // Print out the additional reject struct is it exists
        if (additionalReject.isValid)
        {
            MsgDisplayDetailObj.Trace("\tAdditional Reject Struct\n");
            additionalReject.DisplayRejectStruct(&MsgDisplayDetailObj);
        }
    }
   
    DBG_LEAVE();
}





// *******************************************************************
// class WaitIndication
//
// Description:
//    Wait Indication IE -- GSM04.60 11.2.1
// *******************************************************************

char waitIndSizeNames[][64] = 
{
   "units of seconds",        
   "units of 20 milliseconds"      
};


RlcMacResult IeWaitIndication::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeWaitIndication::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result;

    // Pack Wait Indication bits into the output bit stream.
    if ((result = dataStream.InsertBits8(waitInd, 8)) == 
        RLC_MAC_SUCCESS)
    {
        if ((result = dataStream.InsertBits8((unsigned char)waitIndSize, 1)) !=
            RLC_MAC_SUCCESS)
        {       
            DBG_ERROR("IeWaitIndication::EncodeIe waitIndSize failure %x\n", result);
        }
    }
    else
    {
        DBG_ERROR("IeWaitIndication::EncodeIe waitInd failure %x\n", result);
    }
   
    DBG_LEAVE();
    return (result);
}


void IeWaitIndication::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tWait Indication -------> %#x\n"
                        "\tWait Indication Size --> %s\n",
                        waitInd, waitIndSizeNames[waitIndSize]);
    }
}
