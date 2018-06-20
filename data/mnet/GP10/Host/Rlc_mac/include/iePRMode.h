// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePRMode.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEPRMODE_H__
#define __IEPRMODE_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IePRMode
//
// Description:
//    PR Mode IE -- GSM04.60 11.2.29
// *******************************************************************


class IePRMode : public IeBase {
public:

    typedef enum
    {
        PR_MODE_A_ONE_MS,
        PR_MODE_B_ALL_MS
    } PR_MODE;

    IePRMode() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetPRMode(PR_MODE mode)
    {
        prMode = mode;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    PR_MODE GetPRMode()
    {
        return (prMode);
    }
   
    void ClearPRMode() { isValid = FALSE; }
   
private:
    PR_MODE prMode;
};

#endif