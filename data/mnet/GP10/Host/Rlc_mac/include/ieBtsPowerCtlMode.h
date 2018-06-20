// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieBtsPowerCtlMode.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEBTSPOWERCTLMODE_H__
#define __IEBTSPOWERCTLMODE_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeBtsPowerCtlMode
//
// Description:
//    Bts Power Control Mode IE -- GSM04.60 11.2.29
// *******************************************************************


class IeBtsPowerCtlMode : public IeBase {
public:

    typedef enum
    {
        MODE_A,
        MODE_B
    } BTS_PWR_CTL_MODE;

    IeBtsPowerCtlMode() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetBtsPowerCtlMode(BTS_PWR_CTL_MODE mode)
    {
        pCtlMode = mode;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    BTS_PWR_CTL_MODE GetBtsPowerCtlMode()
    {
        return (pCtlMode);
    }
   
    void ClearBtsPowerCtlMode() { isValid = FALSE; }
   
private:
    BTS_PWR_CTL_MODE pCtlMode;
};

#endif