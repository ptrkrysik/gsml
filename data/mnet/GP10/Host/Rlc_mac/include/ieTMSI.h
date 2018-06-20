// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieTMSI.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IETMSI_H__
#define __IETMSI_H__

#include "IeBase.h"
#include "bitstream.h"


#define MAX_TMSI           4294967295


// *******************************************************************
// class IeTMSI
//
// Description:
//    TMSI IE -- GSM03.03
// *******************************************************************

class IeTMSI : public IeBase {
public:

   IeTMSI() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetTMSI(unsigned int tmsiIn)
   {
      if (tmsiIn <= MAX_TMSI)
      {
         tmsi = tmsiIn;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned int GetTMSI()
   {
      return (tmsi);
   }
   
   void ClearTMSI() { isValid = FALSE; }
   
private:
   unsigned int tmsi;
};

#endif