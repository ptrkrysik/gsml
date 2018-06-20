// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieNLN.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IENLN_H__
#define __IENLN_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeNLN
//
// Description:
//    NLN IE -- GSM04.60 11.2.10
//              GSM04.08 10.5.2.23
// *******************************************************************


class IeNLN : public IeBase {
public:

   IeNLN() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetNLN(unsigned char val)
   {
      nln = val;
      isValid = TRUE;
      return (RLC_MAC_SUCCESS);
   }
   
   unsigned char GetNLN()
   {
      return (nln);
   }
   
   void ClearNLN() { isValid = FALSE; }
   
private:
   unsigned char nln;
};

#endif