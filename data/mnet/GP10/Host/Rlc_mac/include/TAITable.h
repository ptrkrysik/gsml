// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : TAITable.h
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#ifndef __TAITABLE_H__
#define __TAITABLE_H__

#include "RlcMacCommon.h"
#include "Tbf.h"


// *******************************************************************
// class TAITable
//
// Description
//    
// *******************************************************************
class TAITable {
public:
    TAITable();
    ~TAITable() {}
    
    unsigned char AllocateTAI(Tbf *pTbf);
    void ReleaseTAI(unsigned char tai);
    void ShowTAITable();

private:

    Tbf *pTAIEntry[MAX_TAI_PER_TS];

};


#endif
