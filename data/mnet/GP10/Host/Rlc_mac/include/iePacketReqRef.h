// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePacketReqRef.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEPACKETREQREF_H__
#define __IEPACKETREQREF_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IePacketReqRef
//
// Description:
//    Packet Request Ref IE -- GSM04.60 12.11
//                             GSM04.08 10.5.2.38
//
// < Packet Request Reference IE > ::=
//	 < RANDOM_ACCESS_INFORMATION value : bit (11) >
//	 < FRAME_NUMBER : bit (16) > ;
//
// *******************************************************************

class IePacketReqRef : public IeBase {
public:

   IePacketReqRef() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetPacketReqRef(unsigned short randAcc, unsigned char t1,
      unsigned char t2, unsigned char t3)
   {
      randAccInfo = randAcc;
      T1 = t1;
      T2 = t2;
      T3 = t3;
      isValid = TRUE;
      return (RLC_MAC_SUCCESS);
   }
   
   void GetPacketReqRef(unsigned short *randAcc, unsigned char *t1,
      unsigned char *t2, unsigned char *t3)
   {
      *randAcc = randAccInfo;
      *t1 = T1;
      *t2 = T2;
      *t3 = T3;
   }
   
   void ClearPacketReqRef() { isValid = FALSE; }
   
private:
   unsigned short randAccInfo;
   unsigned char T1;
   unsigned char T2;
   unsigned char T3;
};

#endif