// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieMSRadioAccessCapability.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieMSRadioAccessCapability.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


char raTypeNames[][64] =
{
    "GSM P",
    "GSM E",
    "GSM R",
    "GSM 1800",
    "GSM 1900"
};

char a5BitsNames[][64] =
{
    "encryption algorithm NOT available",
    "encryption algorithm available"
};                        
                        
char esIndNames[][64] =
{
    "controlled early Classmark Sending - option is not implemented",
    "controlled early Classmark Sending - option isimplemented"
};

char vgcsNames[][64] =
{
    "no VGCS capability or no notifications wanted",
    "VGCS capability and notifications wanted"
};

char vbsNames[][64] =
{
    "no VBS capability or no notifications wanted",
    "VBS capability and notifications wanted"
};

char psNames[][64] =
{
    "PS capability not present",
    "PS capability present"
};

char gprsExtendedDynamicAllocNames[][80] =
{
    "Extended Dynamic Allocation Capability for GPRS is not implemented",
    "Extended Dynamic Allocation Capability for GPRS is implemented"
};

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


RlcMacResult IeMSRadioAccessCapability::DecodeIe(BitStreamIn &dataStream)
{
    DBG_FUNC("IeMSRadioAccessCapability::DecodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;
    unsigned char val;

    // Extract the MS Radio Access Capablility from the input bit stream.
    for (int i = 0; i < MAX_ACCESS_TECHNOLOGIES; i++)
    {
        // First instance of the MS RA capability value part struct is mandatory.
        // Subsequent instances are optional.
        if (i != 0)
        {
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            if (val == 0)
            {
                isValid = TRUE;
                return (result);
            }
        }
        
        //      < Access Technology Type: bit (4) >
        result = dataStream.ExtractBits8(val, 4);
        RLC_MAC_RESULT_CHECK(result);
        MSRACapabilityValuePart[i].accessTechnologyType = (ACCESS_TECHNOLOGY_TYPE)val;
        
        //      < Length : bit (7) > -- length in bits of Content and spare bits
        result = dataStream.ExtractBits8(MSRACapabilityValuePart[i].length, 7);
        RLC_MAC_RESULT_CHECK(result);
        
        //      < RF Power Capability : bit (3) >
        result = dataStream.ExtractBits8(MSRACapabilityValuePart[i].rfPowerCapability, 3);
        RLC_MAC_RESULT_CHECK(result);
        
        //      { 0 | 1 <A5 bits : <A5 bits> > } -- zero means that the same values apply for parameters as in the immediately preceeding Access capabilities field within this IE
        //      -- The presence of the A5 bits is mandatory in the 1st Access capabilities struct within this IE.
        result = dataStream.ExtractBits8(val, 1);
        RLC_MAC_RESULT_CHECK(result);
        if (val == 1)
        {
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            MSRACapabilityValuePart[i].a51Bit = (A5_BITS)val;
            
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            MSRACapabilityValuePart[i].a52Bit = (A5_BITS)val;
            
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            MSRACapabilityValuePart[i].a53Bit = (A5_BITS)val;
            
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            MSRACapabilityValuePart[i].a54Bit = (A5_BITS)val;
            
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            MSRACapabilityValuePart[i].a55Bit = (A5_BITS)val;
            
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            MSRACapabilityValuePart[i].a56Bit = (A5_BITS)val;
            
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            MSRACapabilityValuePart[i].a57Bit = (A5_BITS)val;
        }
        else
        {
            if (i != 0)
            {
                MSRACapabilityValuePart[i].a51Bit = MSRACapabilityValuePart[i-1].a51Bit;
                MSRACapabilityValuePart[i].a52Bit = MSRACapabilityValuePart[i-1].a52Bit;
                MSRACapabilityValuePart[i].a53Bit = MSRACapabilityValuePart[i-1].a53Bit;
                MSRACapabilityValuePart[i].a54Bit = MSRACapabilityValuePart[i-1].a54Bit;
                MSRACapabilityValuePart[i].a55Bit = MSRACapabilityValuePart[i-1].a55Bit;
                MSRACapabilityValuePart[i].a56Bit = MSRACapabilityValuePart[i-1].a56Bit;
                MSRACapabilityValuePart[i].a57Bit = MSRACapabilityValuePart[i-1].a57Bit;
            }
            else
            {
                DBG_ERROR("IeMSRadioAccessCapability::DecodeIe A5 bits not valid\n");
            }    
        }
        
        //      < ES IND : bit >
        result = dataStream.ExtractBits8(val, 1);
        RLC_MAC_RESULT_CHECK(result);
        MSRACapabilityValuePart[i].esInd = (ES_IND)val;
        
        //      < PS : bit >
        result = dataStream.ExtractBits8(val, 1);
        RLC_MAC_RESULT_CHECK(result);
        MSRACapabilityValuePart[i].ps = (PS)val;
        
        //      < VGCS : bit >
        result = dataStream.ExtractBits8(val, 1);
        RLC_MAC_RESULT_CHECK(result);
        MSRACapabilityValuePart[i].vgcs = (VGCS)val;
        
        //      < VBS : bit >
        result = dataStream.ExtractBits8(val, 1);
        RLC_MAC_RESULT_CHECK(result);
        MSRACapabilityValuePart[i].vbs = (VBS)val;
        
        //      { 0 | 1 < Multislot capability : Multislot capability struct > } ;  -- zero means that the same values apply for multislot parameters as in the immediately preceeding Access capabilities field within this IE.
        //      -- The presence of the Multislot capability struct is mandatory in the 1st Access capabilities struct within this IE.
        result = dataStream.ExtractBits8(val, 1);
        if (val == 1)
        {
            //      { 0 | 1 < HSCSD multislot class : bit (5) > }
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            if (val == 1)
            {
                result = dataStream.ExtractBits8(
                    MSRACapabilityValuePart[i].HSCSDMultislotClass.hscsdMultislotClass, 5);
                RLC_MAC_RESULT_CHECK(result);
                MSRACapabilityValuePart[i].HSCSDMultislotClass.isValid = TRUE;
            }
            else
            {
                MSRACapabilityValuePart[i].HSCSDMultislotClass.isValid = FALSE;
            }
            
            //      { 0 | 1 < GPRS multislot class : bit (5) > < GPRS Extended Dynamic Allocation Capability : bit > }
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            if (val == 1)
            {
                result = dataStream.ExtractBits8(
                    MSRACapabilityValuePart[i].GPRSCapability.gprsMultislotClass, 5);
                RLC_MAC_RESULT_CHECK(result);
                result = dataStream.ExtractBits8(val, 1);
                RLC_MAC_RESULT_CHECK(result);
                MSRACapabilityValuePart[i].GPRSCapability.gprsExtendedDynamicAllocationCapability = 
                    (GPRS_EXTENDED_DYNAMIC_ALLOC_CAPABILITY)val;
                MSRACapabilityValuePart[i].GPRSCapability.isValid = TRUE;
            }
            else
            {
                MSRACapabilityValuePart[i].GPRSCapability.isValid = FALSE;
            }
            
            //      { 0 | 1 < SMS_VALUE : bit (4)  > < SM_VALUE : bit (4)  > } ;
            result = dataStream.ExtractBits8(val, 1);
            RLC_MAC_RESULT_CHECK(result);
            if (val == 1)
            {
                result = dataStream.ExtractBits8(
                    MSRACapabilityValuePart[i].SwitchMeasure.smsValue, 4);
                RLC_MAC_RESULT_CHECK(result);
                result = dataStream.ExtractBits8(
                    MSRACapabilityValuePart[i].SwitchMeasure.smValue, 4);
                RLC_MAC_RESULT_CHECK(result);
                MSRACapabilityValuePart[i].SwitchMeasure.isValid = TRUE;
            }
            else
            {
                MSRACapabilityValuePart[i].SwitchMeasure.isValid = FALSE;
            }
        }
        else
        {
            if (i != 0)
            {
                MSRACapabilityValuePart[i].HSCSDMultislotClass.hscsdMultislotClass =
                    MSRACapabilityValuePart[i-1].HSCSDMultislotClass.hscsdMultislotClass;
                MSRACapabilityValuePart[i].HSCSDMultislotClass.isValid =
                    MSRACapabilityValuePart[i-1].HSCSDMultislotClass.isValid;
                    
                MSRACapabilityValuePart[i].GPRSCapability.gprsMultislotClass =
                    MSRACapabilityValuePart[i-1].GPRSCapability.gprsMultislotClass;
                MSRACapabilityValuePart[i].GPRSCapability.gprsExtendedDynamicAllocationCapability =
                    MSRACapabilityValuePart[i-1].GPRSCapability.gprsExtendedDynamicAllocationCapability;
                MSRACapabilityValuePart[i].GPRSCapability.isValid =
                    MSRACapabilityValuePart[i-1].GPRSCapability.isValid;
                    
                MSRACapabilityValuePart[i].SwitchMeasure.smsValue =
                    MSRACapabilityValuePart[i-1].SwitchMeasure.smsValue;
                MSRACapabilityValuePart[i].SwitchMeasure.smValue =
                    MSRACapabilityValuePart[i-1].SwitchMeasure.smValue;
                MSRACapabilityValuePart[i].SwitchMeasure.isValid =
                    MSRACapabilityValuePart[i-1].SwitchMeasure.isValid;
            }
            else
            {
                DBG_ERROR("IeMSRadioAccessCapability::DecodeIe Multislot capability not valid\n");
            }    
        }
        
        MSRACapabilityValuePart[i].isValid = TRUE;
    }
    
    isValid = TRUE;       
    DBG_LEAVE();
    return (result);
}


IeMSRadioAccessCapability &IeMSRadioAccessCapability::operator=(
    const T_CNI_RIL3_IE_MS_RADIO_ACCESS_CAPABILITY &raCap)
{
    if (raCap.ie_present == TRUE)
    {
        SetMSRadioAccessCapabilityValid();
        for (int i = 0; i < raCap.number_of_ra_capabilities; i++)
        {
            MSRACapabilityValuePart[i].isValid = TRUE;
            MSRACapabilityValuePart[i].accessTechnologyType = 
                (ACCESS_TECHNOLOGY_TYPE)raCap.ms_ra_capability[i].access_technology_type;
            MSRACapabilityValuePart[i].rfPowerCapability = 
                raCap.ms_ra_capability[i].rf_power_capability;
            MSRACapabilityValuePart[i].a51Bit = 
                (raCap.ms_ra_capability[i].a5_1_algorithm) ? A5_X_AVAILABLE : A5_X_NOT_AVAILABLE;
            MSRACapabilityValuePart[i].a52Bit = 
                (raCap.ms_ra_capability[i].a5_2_algorithm) ? A5_X_AVAILABLE : A5_X_NOT_AVAILABLE;
            MSRACapabilityValuePart[i].a53Bit = 
                (raCap.ms_ra_capability[i].a5_3_algorithm) ? A5_X_AVAILABLE : A5_X_NOT_AVAILABLE;
            MSRACapabilityValuePart[i].a54Bit = 
                (raCap.ms_ra_capability[i].a5_4_algorithm) ? A5_X_AVAILABLE : A5_X_NOT_AVAILABLE;
            MSRACapabilityValuePart[i].a55Bit = 
                (raCap.ms_ra_capability[i].a5_5_algorithm) ? A5_X_AVAILABLE : A5_X_NOT_AVAILABLE;
            MSRACapabilityValuePart[i].a56Bit = 
                (raCap.ms_ra_capability[i].a5_6_algorithm) ? A5_X_AVAILABLE : A5_X_NOT_AVAILABLE;
            MSRACapabilityValuePart[i].a57Bit = 
                (raCap.ms_ra_capability[i].a5_7_algorithm) ? A5_X_AVAILABLE : A5_X_NOT_AVAILABLE;
            MSRACapabilityValuePart[i].esInd = 
                (raCap.ms_ra_capability[i].es_ind) ? EARLY_CLASSMARK_SENDING_IMPLEMENTED :
                    EARLY_CLASSMARK_SENDING_NOT_IMPLEMENTED;
            MSRACapabilityValuePart[i].ps = 
                (raCap.ms_ra_capability[i].ps) ? PS_PRESENT : PS_NOT_PRESENT;
            MSRACapabilityValuePart[i].vgcs = 
                (raCap.ms_ra_capability[i].vgcs) ? VGCS_CAPABILITY : NO_VGCS_CAPABILITY;
            MSRACapabilityValuePart[i].vbs = 
                (raCap.ms_ra_capability[i].vbs) ? VBS_CAPABILITY : NO_VBS_CAPABILITY;
            if (raCap.ms_ra_capability[i].multislot_capability.present)
            {
                if (raCap.ms_ra_capability[i].multislot_capability.hscsd_param_present)
                {
                    MSRACapabilityValuePart[i].HSCSDMultislotClass.isValid = TRUE;
                    MSRACapabilityValuePart[i].HSCSDMultislotClass.hscsdMultislotClass = 
                        raCap.ms_ra_capability[i].multislot_capability.hscsd_multi_slot_class;  
                }
                if (raCap.ms_ra_capability[i].multislot_capability.gprs_params_present)
                {
                    MSRACapabilityValuePart[i].GPRSCapability.isValid = TRUE;
                    MSRACapabilityValuePart[i].GPRSCapability.gprsMultislotClass = 
                        raCap.ms_ra_capability[i].multislot_capability.gprs_multi_slot_class;
                    MSRACapabilityValuePart[i].GPRSCapability.gprsExtendedDynamicAllocationCapability = 
                        (raCap.ms_ra_capability[i].multislot_capability.gprs_dynamic_allocation_capability) ? 
                            EXTENDED_DYNAMIC_ALLOC_IMPLEMENTED : EXTENDED_DYNAMIC_ALLOC_NOT_IMPLEMENTED;
                }
                if (raCap.ms_ra_capability[i].multislot_capability.sms_params_present)
                {
                    MSRACapabilityValuePart[i].SwitchMeasure.isValid = TRUE;
                    MSRACapabilityValuePart[i].SwitchMeasure.smsValue = 
                        raCap.ms_ra_capability[i].multislot_capability.switch_measure_switch_value;
                    MSRACapabilityValuePart[i].SwitchMeasure.smValue = 
                        raCap.ms_ra_capability[i].multislot_capability.switch_measure_value;
                }
            }
        } 
    }
    return (*this);
}


void IeMSRadioAccessCapability::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tMS Radio Access Capability\n");
        for (int i = 0; i < MAX_ACCESS_TECHNOLOGIES; i++)
        {
            if (MSRACapabilityValuePart[i].isValid)
            {
                outObj->Trace("\tAccess Technology #%d\n", i);
                outObj->Trace("\t\tAccess Technology Type ---> %s\n", 
                    raTypeNames[MSRACapabilityValuePart[i].accessTechnologyType]);
                
                outObj->Trace("\t\tLength ---> %d\n", MSRACapabilityValuePart[i].length);
            
                outObj->Trace("\t\tRF Power Capability ---> %d\n", 
                    MSRACapabilityValuePart[i].rfPowerCapability);
                
                outObj->Trace("\t\tA5/1 ---> %s\n", 
                    a5BitsNames[MSRACapabilityValuePart[i].a51Bit]);
                outObj->Trace("\t\tA5/2 ---> %s\n", 
                    a5BitsNames[MSRACapabilityValuePart[i].a52Bit]);
                outObj->Trace("\t\tA5/3 ---> %s\n", 
                    a5BitsNames[MSRACapabilityValuePart[i].a53Bit]);
                outObj->Trace("\t\tA5/4 ---> %s\n", 
                    a5BitsNames[MSRACapabilityValuePart[i].a54Bit]);
                outObj->Trace("\t\tA5/5 ---> %s\n", 
                    a5BitsNames[MSRACapabilityValuePart[i].a55Bit]);
                outObj->Trace("\t\tA5/6 ---> %s\n", 
                    a5BitsNames[MSRACapabilityValuePart[i].a56Bit]);
                outObj->Trace("\t\tA5/7 ---> %s\n", 
                    a5BitsNames[MSRACapabilityValuePart[i].a57Bit]);
                
                outObj->Trace("\t\tES IND ---> %s\n", 
                    esIndNames[MSRACapabilityValuePart[i].esInd]);
                
                outObj->Trace("\t\tPS ---> %s\n", 
                    psNames[MSRACapabilityValuePart[i].ps]);
                
                outObj->Trace("\t\tVGCS ---> %s\n", 
                    vgcsNames[MSRACapabilityValuePart[i].vgcs]);
                
                outObj->Trace("\t\tVBS ---> %s\n", 
                    vbsNames[MSRACapabilityValuePart[i].vbs]);
                
                if (MSRACapabilityValuePart[i].HSCSDMultislotClass.isValid)
                {
                    outObj->Trace("\t\tHSCSD multislot class ---> %d\n", 
                        MSRACapabilityValuePart[i].HSCSDMultislotClass.hscsdMultislotClass);
                }
            
                if (MSRACapabilityValuePart[i].GPRSCapability.isValid)
                {
                    outObj->Trace("\t\tGPRS Multislot class ---> %d\n", 
                        MSRACapabilityValuePart[i].GPRSCapability.gprsMultislotClass);
                    outObj->Trace("\t\tGPRS Extended Dynamic Allocation Capability ---> \n\t\t\t%s\n", 
                        gprsExtendedDynamicAllocNames[MSRACapabilityValuePart[i].GPRSCapability.gprsExtendedDynamicAllocationCapability]);
                }
            
                if (MSRACapabilityValuePart[i].SwitchMeasure.isValid)
                {
                    outObj->Trace("\t\tSMS_VALUE ---> %d/4 timeslot\n", 
                        MSRACapabilityValuePart[i].SwitchMeasure.smsValue+1);
                    outObj->Trace("\t\tSM_VALUE ---> %d/4 timeslot\n", 
                        MSRACapabilityValuePart[i].SwitchMeasure.smValue+1);
                }
            }
        }
    }       
}