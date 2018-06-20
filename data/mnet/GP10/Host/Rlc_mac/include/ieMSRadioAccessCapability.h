// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMSRadioAccessCapability.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEMSRADIOACCESSCAPABILITY_H__
#define __IEMSRADIOACCESSCAPABILITY_H__

#include "IeBase.h"
#include "bitstream.h"

#include "ril3/ril3_gmm_msg.h"

// *******************************************************************
// class IeMSRadioAccessCapability
//
// Description:
//    Power Control Parameters IE -- GSM04.60 11.2.16, GSM04.08 10.5.5.12a
//
// < MS Radio Access Capability IE > ::=
//
// <MS RA capability  value part struct >::=  --recursive structure allows any number of Access technologies
//      < Access Technology Type: bit (4) >
//      < Access capabilities : <Access capabilities struct> >
//      { 0 | 1 <MS RA capability  value part struct> } ;
//
// < Access capabilities struct > ::=
//      < Length : bit (7) > -- length in bits of Content and spare bits
//      <Access capabilities : <Content>> 
//      <spare bits>** ; -- expands to the indicated length
//            -- may be used for future enhancements
//
// < Content > ::=
//      < RF Power Capability : bit (3) >
//      { 0 | 1 <A5 bits : <A5 bits> > } -- zero means that the same values apply for parameters as in the immediately preceeding Access capabilities field within this IE
//      -- The presence of the A5 bits is mandatory in the 1st Access capabilities struct within this IE.
//      < ES IND : bit >
//      < PS : bit >
//      < VGCS : bit >
//      < VBS : bit >
//      { 0 | 1 < Multislot capability : Multislot capability struct > } ;  -- zero means that the same values apply for multislot parameters as in the immediately preceeding Access capabilities field within this IE.
//      -- The presence of the Multislot capability struct is mandatory in the 1st Access capabilities struct within this IE.
//
// < Multislot capability struct > ::=
//      { 0 | 1 < HSCSD multislot class : bit (5) > }
//      { 0 | 1 < GPRS multislot class : bit (5) > < GPRS Extended Dynamic Allocation Capability : bit > }
//      { 0 | 1 < SMS_VALUE : bit (4)  > < SM_VALUE : bit (4)  > } ;
//
// <A5 bits> ::= < A5/1 : bit> <A5/2 : bit> <A5/3 : bit> <A5/4 : bit> <A5/5 : bit> <A5/6 : bit> <A5/7 : bit>; -- bits for circuit mode ciphering algorithms
//
// *******************************************************************

// (GSM P, GSM E, GSM R), or GSM 1800, or GSM 1900
#define MAX_ACCESS_TECHNOLOGIES 3

class IeMSRadioAccessCapability : public IeBase {
public:
    
    typedef enum
    {
        GSM_P,
        GSM_E,
        GSM_R,
        GSM_1800,
        GSM_1900
    } ACCESS_TECHNOLOGY_TYPE;
    
    typedef enum
    {
        A5_X_NOT_AVAILABLE,
        A5_X_AVAILABLE,
    } A5_BITS;
    
    typedef enum
    {
        EARLY_CLASSMARK_SENDING_NOT_IMPLEMENTED,
        EARLY_CLASSMARK_SENDING_IMPLEMENTED
    } ES_IND;
    
    typedef enum
    {
        PS_NOT_PRESENT,
        PS_PRESENT
    } PS;
    
    typedef enum
    {
        NO_VGCS_CAPABILITY,
        VGCS_CAPABILITY
    } VGCS;
    
    typedef enum
    {
        NO_VBS_CAPABILITY,
        VBS_CAPABILITY
    } VBS;
    
    typedef enum
    {
        EXTENDED_DYNAMIC_ALLOC_NOT_IMPLEMENTED,
        EXTENDED_DYNAMIC_ALLOC_IMPLEMENTED
    } GPRS_EXTENDED_DYNAMIC_ALLOC_CAPABILITY;
    
    // IE contents
    struct MSRACapabilityValuePartStruct
    {
        bool                        isValid;
        ACCESS_TECHNOLOGY_TYPE      accessTechnologyType;
        unsigned char               length;
        unsigned char               rfPowerCapability;
        A5_BITS                     a51Bit;
        A5_BITS                     a52Bit;
        A5_BITS                     a53Bit;
        A5_BITS                     a54Bit;
        A5_BITS                     a55Bit;
        A5_BITS                     a56Bit;
        A5_BITS                     a57Bit;
        ES_IND                      esInd;
        PS                          ps;
        VGCS                        vgcs;
        VBS                         vbs;
        struct
        {
            bool                    isValid;
            unsigned char           hscsdMultislotClass;
        } HSCSDMultislotClass;
        struct
        {
            bool                    isValid;
            unsigned char           gprsMultislotClass;
            GPRS_EXTENDED_DYNAMIC_ALLOC_CAPABILITY   gprsExtendedDynamicAllocationCapability;
        } GPRSCapability;
        struct
        {
            bool                    isValid;
            unsigned char           smsValue;
            unsigned char           smValue;
        } SwitchMeasure;
    } MSRACapabilityValuePart[MAX_ACCESS_TECHNOLOGIES];
    
    IeMSRadioAccessCapability() : IeBase() 
    {
        for (int i=0; i < MAX_ACCESS_TECHNOLOGIES; i++)
        {
            MSRACapabilityValuePart[i].isValid = FALSE;
        }
    } 
   
    RlcMacResult DecodeIe(BitStreamIn &dataStream);
    void DisplayDetails(DbgOutput *outObj);
    
	void SetMSRadioAccessCapabilityValid() { isValid = TRUE; }
   	void ClearMSRadioAccessCapability() { isValid = FALSE; }
    IeMSRadioAccessCapability &operator=(const T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY &raCap);
};

#endif