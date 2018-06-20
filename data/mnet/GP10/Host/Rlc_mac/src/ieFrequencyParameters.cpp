// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieFrequencyParameters.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieFrequencyParameters.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


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


RlcMacResult IeFrequencyParameters::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeFrequencyParameters::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Frequency List bits into the output bit stream.
    result = tsc.EncodeIe(dataStream);
    RLC_MAC_RESULT_CHECK(result);
    
    if (arfcn.IsValid())
    {
        dataStream.InsertBits8(0, 2);
        result = arfcn.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else if (IndirectEncoding.isValid)
    {
        dataStream.InsertBits8(1, 2);
        result = IndirectEncoding.maio.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
        result = IndirectEncoding.maNumber.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
        if (IndirectEncoding.changeMark1.IsValid())
        {
            dataStream.InsertBits8(1, 1);
            result = IndirectEncoding.changeMark1.EncodeIe(dataStream);
            RLC_MAC_RESULT_CHECK(result);
            if (IndirectEncoding.changeMark2.IsValid())
            {
                dataStream.InsertBits8(1, 1);
                result = IndirectEncoding.changeMark2.EncodeIe(dataStream);
                RLC_MAC_RESULT_CHECK(result);
            }
            else
            {
                dataStream.InsertBits8(0, 1);
            }
        }
        else
        {
            dataStream.InsertBits8(0, 1);
        }
    }
    else if (DirectEncoding1.isValid)
    {
        result = DirectEncoding1.maio.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
        result = DirectEncoding1.gprsMobileAllocation.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else if (DirectEncoding2.isValid)
    {
        result = DirectEncoding2.maio.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
        result = DirectEncoding2.hsn.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
        result = DirectEncoding2.maFreqList.EncodeIe(dataStream);
        RLC_MAC_RESULT_CHECK(result);
    }
    else
    {
        DBG_ERROR("IeFrequencyParameters::EncodeIe failure no valid freq encoding\n");
    }
       
    DBG_LEAVE();
    return (result);
}


void IeFrequencyParameters::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tFrequency Paramters\n");
        tsc.DisplayDetails(outObj);
        if (arfcn.IsValid())
        {
            arfcn.DisplayDetails(outObj);
        }
        else if (IndirectEncoding.isValid)
        {
            IndirectEncoding.maio.DisplayDetails(outObj);
            IndirectEncoding.maNumber.DisplayDetails(outObj);
            if (IndirectEncoding.changeMark1.IsValid())
            {
                IndirectEncoding.changeMark1.DisplayDetails(outObj);
                if (IndirectEncoding.changeMark2.IsValid())
                {
                    IndirectEncoding.changeMark2.DisplayDetails(outObj);
                }
            }
        }
        else if (DirectEncoding1.isValid)
        {
            DirectEncoding1.maio.DisplayDetails(outObj);
            DirectEncoding1.gprsMobileAllocation.DisplayDetails(outObj);
        }
        else if (DirectEncoding2.isValid)
        {
            DirectEncoding2.maio.DisplayDetails(outObj);
            DirectEncoding2.hsn.DisplayDetails(outObj);
            DirectEncoding2.maFreqList.DisplayDetails(outObj);
        }
    }
}