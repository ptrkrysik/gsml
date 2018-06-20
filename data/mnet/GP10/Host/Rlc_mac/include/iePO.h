// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePO.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEPO_H__
#define __IEPO_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IePO
//
// Description:
//    PO IE -- GSM04.60 11.2.29
// *******************************************************************


class IePO : public IeBase {
public:

   IePO() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetPO(unsigned char val)
   {
      po = val;
      isValid = TRUE;
      return (RLC_MAC_SUCCESS);
   }
   
   unsigned char GetPO()
   {
      return (po);
   }
   
   void ClearPO() { isValid = FALSE; }
   
private:
   unsigned char po;
};

#endif