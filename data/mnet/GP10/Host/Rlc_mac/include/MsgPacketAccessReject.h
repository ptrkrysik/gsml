// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgPacketAccessReject.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGPACKETACCESSREJECT_H__
#define __MSGPACKETACCESSREJECT_H__

#include "MsgBase.h"
#include "bitstream.h"
#include "iePageMode.h"
#include "ieTLLI.h"
#include "iePacketReqRef.h"
#include "ieGlobalTFI.h"


// *******************************************************************
// class IeWaitIndication
//
// Description:
//    Wait Indication IE -- GSM04.60 11.2.1
// *******************************************************************

class IeWaitIndication : public IeBase {
public:

   typedef enum
   {
      UNITS_OF_SECONDS,
      UNITS_OF_20_MS
   } RLC_MAC_WAIT_IND_SIZE;
   
   IeWaitIndication() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetWaitIndication(unsigned char ind, RLC_MAC_WAIT_IND_SIZE size)
   {
      waitInd = ind;
      waitIndSize = size;
      isValid = TRUE;
      return (RLC_MAC_SUCCESS);
   }
   
   void GetWaitIndication(unsigned char *wait, RLC_MAC_WAIT_IND_SIZE *size)
   {
      *wait = waitInd;
      *size = waitIndSize;
   }
   
   void ClearWaitIndication() { isValid = FALSE; }
private:
   unsigned char waitInd;
   RLC_MAC_WAIT_IND_SIZE waitIndSize;
};


// *******************************************************************
// PACKET ACCESS REJECT
//
// GSM 04.60 11.2.1
//
//    < Packet Access Reject message content > ::=
//    	< PAGE_MODE : bit (2) >
//    	< Reject : < Reject struct > >
//    	{ { 1 < Additional Reject: < Reject struct > > } ** 0
//    	 < padding bits > } //		-- truncation at end of message allowed, bits '0' assumed
//    	 ! < Distribution part error : bit (*) = < no string > > ;
//
//    < Reject struct > ::=
//    	{ 0	< TLLI : bit (32) >
//    	 | 1	{ 0	< Packet Request Reference : < Packet Request Reference IE > >
//    		 | 1	< Global TFI : < Global TFI IE > > } }
//    	{ 0 | 1	< WAIT_INDICATION : bit (8) >
//    			< WAIT _INDICATION_SIZE : bit (1) > }
//    	 ! < Ignore : bit (*) = <no string> > ;
//
// *******************************************************************

class RejectStruct {
public:
    IeTLLI              tlli;
    IePacketReqRef      packetReqRef;
    IeGlobalTFI         globalTFI;
    IeWaitIndication    waitInd;
    bool                isValid;
    
    // Default constructor
    RejectStruct() : isValid(FALSE), tlli(), packetReqRef(), globalTFI(), waitInd() {}
    
    RlcMacResult EncodeRejectStruct(BitStreamOut &dataStream);
    void DisplayRejectStruct(DbgOutput *outObj);
};

class MsgPacketAccessReject : public DownlinkMsgBase {
public:

    // Message type is included in MsgBase.
   
    // Distributed message contents
    IePageMode            pageMode;
    RejectStruct          reject;
    RejectStruct          additionalReject; 
       
    // Default constructor
    MsgPacketAccessReject() : DownlinkMsgBase(RlcMacMsgType::PACKET_ACCESS_REJECT),
        pageMode(), reject(), additionalReject(){}
   
    // Default destructor
    ~MsgPacketAccessReject() {}
   
    // Pack IE into an output stream of bits.
    RlcMacResult EncodeMsg(BitStreamOut &dataStream);
   
    // Display IE details.
    void DisplayMsgDetail();
};



#endif