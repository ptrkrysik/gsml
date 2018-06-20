// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTFI.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IETFI_H__
#define __IETFI_H__

#include "IeBase.h"
#include "bitstream.h"
#include "RlcMacCommon.h"


// *******************************************************************
// class IeTFI
//
// Description:
//    Global TFI IE -- GSM04.60 12.15
//
// *******************************************************************

class IeTFI : public IeBase {
public:

   
   IeTFI() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   RlcMacResult DecodeIe(BitStreamIn &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetTFI(unsigned char tfiIn)
   {
      if (tfiIn <= MAX_TFI)
      {
         tfi = tfiIn;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned char GetTFI()
   {
      return (tfi);
   }
   
   void ClearTFI() { isValid = FALSE; }
   
private:
   unsigned char tfi;
};

#endif