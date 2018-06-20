// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTSC.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IETSC_H__
#define __IETSC_H__

#include "IeBase.h"
#include "bitstream.h"


// *******************************************************************
// class IeTSC
//
// Description:
//    TSC IE -- GSM04.60 12.8
// *******************************************************************

#define MAX_TSC   7

class IeTSC : public IeBase {
public:

   IeTSC() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetTSC(unsigned char val)
   {
      if (val <= MAX_TSC)
      {
         tsc = val;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned char GetTSC()
   {
      return(tsc);
   }
   
   void ClearTSC() { isValid = FALSE; }
   
private:
   unsigned char tsc;
};

#endif