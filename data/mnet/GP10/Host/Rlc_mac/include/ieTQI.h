// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTQI.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IETQI_H__
#define __IETQI_H__

#include "IeBase.h"
#include "bitstream.h"


// *******************************************************************
// class IeTQI
//
// Description:
//    Global TQI IE -- GSM04.60 12.17
//
// *******************************************************************

class IeTQI : public IeBase {
public:

   
   IeTQI() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetTQI(unsigned short tqiIn)
   {
      tqi = tqiIn;
      isValid = TRUE;
      return (RLC_MAC_SUCCESS);
   }
   
   unsigned short GetTQI()
   {
      return (tqi);
   }
   
   void ClearTQI() { isValid = FALSE; }
   
private:
   unsigned short tqi;
};

#endif