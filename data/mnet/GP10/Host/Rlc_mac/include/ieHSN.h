// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieHSN.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEHSN_H__
#define __IEHSN_H__

#include "IeBase.h"
#include "bitstream.h"


// *******************************************************************
// class IeHSN
//
// Description:
//    HSN IE -- GSM04.60 12.8
// *******************************************************************

#define MAX_HSN   63

class IeHSN : public IeBase {
public:

   IeHSN() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetHSN(unsigned char val)
   {
      if (val <= MAX_HSN)
      {
         hsn = val;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned char GetHSN()
   {
      return(hsn);
   }
   
   void ClearHSN() { isValid = FALSE; }
   
private:
   unsigned char hsn;
};

#endif