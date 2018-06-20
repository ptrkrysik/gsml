// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePTMSI.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEPTMSI_H__
#define __IEPTMSI_H__

#include "IeBase.h"
#include "bitstream.h"


#define MAX_PTMSI           4294967295


// *******************************************************************
// class IePTMSI
//
// Description:
//    PTMSI IE -- GSM03.03
// *******************************************************************

class IePTMSI : public IeBase {
public:

   IePTMSI() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetPTMSI(unsigned int ptmsiIn)
   {
      if (ptmsiIn <= MAX_PTMSI)
      {
         ptmsi = ptmsiIn;
         isValid = TRUE;
         return (RLC_MAC_SUCCESS);
      }
      else
      {
         return (RLC_MAC_PARAMETER_RANGE_ERROR);
      }
   }
   
   unsigned int GetPTMSI()
   {
      return (ptmsi);
   }
   
   void ClearPTMSI() { isValid = FALSE; }
   
private:
   unsigned int ptmsi;
};

#endif