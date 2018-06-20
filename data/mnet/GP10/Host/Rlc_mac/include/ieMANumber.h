// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMANumber.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEMANUMBER_H__
#define __IEMANUMBER_H__

#include "IeBase.h"
#include "bitstream.h"


// *******************************************************************
// class IeMANumber
//
// Description:
//    MANumber IE -- GSM04.60 12.8
// *******************************************************************

#define MAX_MA_NUMBER   15

class IeMANumber : public IeBase {
public:

   IeMANumber() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetMANumber(unsigned char val)
   {
      if (val <= MAX_MA_NUMBER)
      {
         maNum = val;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned char GetMANumber()
   {
      return(maNum);
   }
   
   void ClearMANumber() { isValid = FALSE; }
   
private:
   unsigned char maNum;
};

#endif