// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieAccessType.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEACCESSTYPE_H__
#define __IEACCESSTYPE_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeAccessType
//
// Description:
//    ACCESS_TYPE IE -- GSM04.60 11.2.16
// *******************************************************************


class IeAccessType : public IeBase {
public:

    typedef enum
    {
        TWO_PHASE,
        PAGE_RESPONSE,
        CELL_UPDATE,
        MM_PROCEDURE
    } ACCESS_TYPE;

    IeAccessType() : IeBase() {}
   
    RlcMacResult DecodeIe(BitStreamIn &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetAccessType(ACCESS_TYPE type)
    {
        accessType = type;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    ACCESS_TYPE GetAccessType()
    {
        return (accessType);
    }
   
    void ClearAccessType() { isValid = FALSE; }
   
private:
    ACCESS_TYPE accessType;
};

#endif