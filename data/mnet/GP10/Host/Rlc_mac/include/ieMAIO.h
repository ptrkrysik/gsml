// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMAIO.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEMAIO_H__
#define __IEMAIO_H__

#include "IeBase.h"
#include "bitstream.h"


// *******************************************************************
// class IeMAIO
//
// Description:
//    MAIO IE -- GSM04.60 12.8
// *******************************************************************

#define MAX_MAIO   63

class IeMAIO : public IeBase {
public:

   IeMAIO() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetMAIO(unsigned char val)
   {
      if (val <= MAX_MAIO)
      {
         maio = val;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned char GetMAIO()
   {
      return(maio);
   }
   
   void ClearMAIO() { isValid = FALSE; }
   
private:
   unsigned char maio;
};

#endif