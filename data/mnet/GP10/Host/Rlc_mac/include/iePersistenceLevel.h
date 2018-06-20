// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePersistenceLevel.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEPERSISTENCELEVEL_H__
#define __IEPERSISTENCELEVEL_H__

#include "IeBase.h"
#include "bitstream.h"
#include "RlcMacCommon.h"

// *******************************************************************
// class IePersistenceLevel
//
// Description:
//    Page Mode IE -- GSM04.60 12.14
// *******************************************************************


class IePersistenceLevel : public IeBase {
public:

    IePersistenceLevel() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetPersistenceLevel(unsigned char levels[MAX_NUM_RADIO_PRIORITY])
    {
        for (int i = 0; i < MAX_NUM_RADIO_PRIORITY; i++)
        {
            persistenceLevel[i] = levels[i];
        }
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    void GetPersistenceLevel(unsigned char levels[MAX_NUM_RADIO_PRIORITY])
    {
        for (int i = 0; i < MAX_NUM_RADIO_PRIORITY; i++)
        {
            levels[i] = persistenceLevel[i];
        }
    }
   
    void ClearPersistenceLevel() { isValid = FALSE; }
   
private:
    unsigned char persistenceLevel[MAX_NUM_RADIO_PRIORITY];
};

#endif