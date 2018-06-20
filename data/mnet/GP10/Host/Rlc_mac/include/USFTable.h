// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : USFTable.h
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#ifndef __USFTABLE_H__
#define __USFTABLE_H__

#include "ULTbf.h"
#include "RlcMacCommon.h"


#define INVALID_USF     0xff


// *******************************************************************
// class USFTable
//
// Description
//    
// *******************************************************************
class USFTable {
public:
    USFTable();
    ~USFTable() {}
    
    unsigned char AllocateUSF(ULTbf *pTbf);
    void ReleaseUSF(unsigned char usf);
    void ShowUSFTable();

private:

    Tbf *pUSFEntry[MAX_USF_PER_TS];

};


#endif
