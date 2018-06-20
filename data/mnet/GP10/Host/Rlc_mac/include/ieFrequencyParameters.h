// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieFrequencyParameters.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEFREQUENCYPARAMETERS_H__
#define __IEFREQUENCYPARAMETERS_H__

#include "logging\vclogging.h"
#include "bitstream.h"
#include "ieTSC.h"
#include "ieARFCN.h"
#include "ieMAIO.h"
#include "ieMANumber.h"
#include "ieChangeMark.h"
#include "ieGPRSMobileAllocation.h"
#include "ieMAFreqList.h"


// *******************************************************************
// class IeFrequencyParameters
//
// Description:
//    Frequency Parameters IE -- GSM04.60 12.8
//
// < Frequency Parameters IE > ::=
//   < TSC : bit (3) >
//   { 00 < ARFCN : bit (10) >
//   | 01 < Indirect encoding : < Indirect encoding struct > >
//   | 10 < Direct encoding 1 : < Direct encoding 1 struct > >
//   | 11 < Direct encoding 2 : < Direct encoding 2 struct > > } ;
//
// < Indirect encoding struct > ::=
//   < MAIO : bit (6) >
//   < MA_NUMBER : bit (4) >
//   { 0 | 1 < CHANGE_MARK_1 : bit (2) >
//          { 0 | 1 < CHANGE_MARK_2 : bit (2) > } } ;
//
// < Direct encoding 1 struct > ::=
//   < MAIO : bit (6) >
//   < GPRS Mobile Allocation : < GPRS Mobile Allocation IE > > ;
//
// < Direct encoding 2 struct > ::=
//   < MAIO : bit (6) >
//   < HSN : bit (6) >
//   < Length of MA Frequency List contents : bit (4) >
//   < MA Frequency List contents : octet (val(Length of MA Frequency List contents) + 3) > ;
// *******************************************************************

class IeFrequencyParameters : public IeBase {
public:
    
    // IE contents
    IeTSC             tsc;
    IeARFCN           arfcn;
    struct IndirectEncodingStruct
    {
        bool                    isValid;
        IeMAIO                  maio;
        IeMANumber              maNumber;
        IeChangeMark            changeMark1;
        IeChangeMark            changeMark2;
    } IndirectEncoding;
    struct DirectEncoding1Struct
    {
        bool                    isValid;
        IeMAIO                  maio;
        IeGPRSMobileAllocation  gprsMobileAllocation;
    } DirectEncoding1;
    struct DirectEncoding2Struct
    {
        bool                    isValid;
        IeMAIO                  maio;
        IeHSN                   hsn;
        IeMAFreqList            maFreqList;
    } DirectEncoding2;
    
        
    IeFrequencyParameters() : IeBase(), tsc() 
    {
        IndirectEncoding.isValid = FALSE;
        DirectEncoding1.isValid = FALSE;
        DirectEncoding2.isValid = FALSE;
    } 
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
    
    void SetFrequencyParameters() { isValid = TRUE; }
    void ClearFrequencyParameters() { isValid = FALSE; }
};

#endif