// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : IeBase.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEBASE_H__
#define __IEBASE_H__

#include "RlcMacMsgType.h"
#include "logging\vclogging.h"

// *******************************************************************
// class IeBase
//
// Description
//    
// *******************************************************************
class IeBase {
public:
    RlcMacMsgType     msgType;

    // Constructor
    IeBase() : isValid(FALSE) {}
    // Destructor
    ~IeBase() {}
   
    bool IsValid()
    {
        return (isValid);
    }
   
protected:
    bool isValid;
};


#endif
