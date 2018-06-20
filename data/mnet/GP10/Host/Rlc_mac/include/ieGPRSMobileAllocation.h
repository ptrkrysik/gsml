// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieGPRSMobileAllocation.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEGPRSMOBILEALLOCATION_H__
#define __IEGPRSMOBILEALLOCATION_H__

#include "logging\vclogging.h"
#include "bitstream.h"
#include "ieHSN.h"
#include "ieMABitMap.h"
#include "ieARFCNIndexList.h"
#include "ieRFLNumberList.h"


// *******************************************************************
// class IeGPRSMobileAllocation
//
// Description:
//    GPRS Mobile Allocation IE -- GSM04.60 12.10a
//
// < GPRS Mobile Allocation IE > ::=
//   < HSN : bit (6) >
//   { 0 | 1     < RFL number list : < RFL number list struct > > }
//   { 0 < MA_LENGTH : bit (6) >
//       < MA_BITMAP : bit (val(MA_LENGTH) + 1) >
//   | 1 { 0 | 1    < ARFCN index list : < ARFCN index list struct > > } } ;
//
// < RFL number list struct > ::=
//   < RFL_NUMBER : bit (4) >
//   { 0 | 1 < RFL number list struct > } ;
// < ARFCN index list struct > ::=
//   < ARFCN_INDEX : bit (6) >
//   { 0 | 1 < ARFCN index list struct > } ;
// *******************************************************************

class IeGPRSMobileAllocation : public IeBase {
public:
    
    // IE contents
    IeHSN             hsn;
    IeRFLNumberList   rflNumberList;
    IeMABitMap        maBitMap;
    IeARFCNIndexList  arfcnIndexList;    
    
    IeGPRSMobileAllocation() : IeBase(), hsn(), rflNumberList() {} 
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
    
	void SetGPRSMobileAllocationValid() { isValid = TRUE; }
   	void ClearGPRSMobileAllocation() { isValid = FALSE; }
};

#endif