// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTLLI.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IETLLI_H__
#define __IETLLI_H__

#include "IeBase.h"
#include "bitstream.h"


#define MAX_TLLI           4294967295


// *******************************************************************
// class IeTLLI
//
// Description:
//    TLLI IE -- GSM04.60 12.16
// *******************************************************************

class IeTLLI : public IeBase {
public:

   IeTLLI() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   RlcMacResult DecodeIe(BitStreamIn &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetTLLI(unsigned int tlliIn)
   {
      if (tlliIn <= MAX_TLLI)
      {
         tlli = tlliIn;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned int GetTLLI()
   {
      return (tlli);
   }
   
   void ClearTLLI() { isValid = FALSE; }
   
private:
   unsigned int tlli;
};

#endif