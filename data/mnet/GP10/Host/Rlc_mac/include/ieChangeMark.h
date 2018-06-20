// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieChangeMark.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IECHANGEMARK_H__
#define __IECHANGEMARK_H__

#include "IeBase.h"
#include "bitstream.h"


// *******************************************************************
// class IeChangeMark
//
// Description:
//    Change Mark IE -- GSM04.60 12.8
// *******************************************************************

#define MAX_CHANGE_MARK   3

class IeChangeMark : public IeBase {
public:

   IeChangeMark() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   RlcMacResult DecodeIe(BitStreamIn &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetChangeMark(unsigned char val)
   {
      if (val <= MAX_CHANGE_MARK)
      {
         changeMark = val;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned char GetChangeMark()
   {
      return(changeMark);
   }
   
   void ClearChangeMark() { isValid = FALSE; }
   
private:
   unsigned char changeMark;
};

#endif