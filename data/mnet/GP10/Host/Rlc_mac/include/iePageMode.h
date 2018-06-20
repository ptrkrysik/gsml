// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePageMode.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEPAGEMODE_H__
#define __IEPAGEMODE_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IePageMode
//
// Description:
//    Page Mode IE -- GSM04.60 12.20
// *******************************************************************


class IePageMode : public IeBase {
public:

   typedef enum
   {
      NORMAL_PAGING           = 0,
      EXTENDED_PAGING         = 1,
      PAGING_REORGANIZATION   = 2,
      PAGING_SAME_AS_BEFORE   = 3
   } RLC_MAC_PAGE_MODE;

   IePageMode() : IeBase() {}
   
   RlcMacResult EncodeIe(BitStreamOut &dataStream);
   void DisplayDetails(DbgOutput *outObj);
   
   RlcMacResult SetPageMode(RLC_MAC_PAGE_MODE mode)
   {
      pageMode = mode;
      isValid = TRUE;
      return (RLC_MAC_SUCCESS);
   }
   
   RLC_MAC_PAGE_MODE GetPageMode()
   {
      return (pageMode);
   }
   
   void ClearPageMode() { isValid = FALSE; }
   
private:
   RLC_MAC_PAGE_MODE pageMode;
};

#endif