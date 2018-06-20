// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieeMLPPPriority.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEEMLPPPRIORITY_H__
#define __IEEMLPPPRIORITY_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeeMLPPPriority
//
// Description:
//    eMLPP Priority IE -- GSM04.60 11.2.10
//                         GSM04.08 10.5.2.24
// *******************************************************************


class IeeMLPPPriority : public IeBase {
public:

    typedef enum
    {
        NO_PRIORITY_APPLIED,
        CALL_PRIORITY_LEVEL_4,
        CALL_PRIORITY_LEVEL_3,
        CALL_PRIORITY_LEVEL_2,
        CALL_PRIORITY_LEVEL_1,
        CALL_PRIORITY_LEVEL_0,
        CALL_PRIORITY_LEVEL_A,
        CALL_PRIORITY_LEVEL_B
    } EMLPP_PRIORITY;
    
    IeeMLPPPriority() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SeteMLPPPriority(EMLPP_PRIORITY priority)
    {
        emlppPriority = priority;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    EMLPP_PRIORITY GeteMLPPPriority()
    {
        return (emlppPriority);
    }
   
    void CleareMLPPPriority() { isValid = FALSE; }
   
private:
    EMLPP_PRIORITY emlppPriority;
};

#endif