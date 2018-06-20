// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieARFCN.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEARFCN_H__
#define __IEARFCN_H__

#include "IeBase.h"
#include "bitstream.h"


// *******************************************************************
// class IeARFCN
//
// Description:
//    ARFCN IE -- GSM04.60 11.2.4
// *******************************************************************

#define MAX_ARFCN   1023

class IeARFCN : public IeBase {
public:

   IeARFCN() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   RlcMacResult DecodeIe(BitStreamIn &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetARFCN(unsigned short freq)
   {
      if (freq <= MAX_ARFCN)
      {
         arfcn = freq;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned short GetARFCN()
   {
      return(arfcn);
   }
   
   void ClearARFCN() { isValid = FALSE; }
   
private:
   unsigned short arfcn;
};

#endif