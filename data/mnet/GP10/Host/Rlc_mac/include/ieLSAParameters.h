// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieLSAParameters.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IELSAPARAMETERS_H__
#define __IELSAPARAMETERS_H__

#include "IeBase.h"
#include "bitstream.h"

#define MAX_FREQ_OR_CELLS       32
// *******************************************************************
// class IeLSAParameters
//
// Description:
//    LSA Parameters IE -- GSM04.60 12.28
//
//  < LSA Parameters IE > ::=
//			  < NR_OF_FREQ_OR_CELLS : bit (5) >:
//			    < LSA ID information : < LSA ID information struct >> } *(val (NR_OF_FREQ_OR_CELLS)) };
//
//  < LSA ID information struct > ::=
//			  { 1 { 0 < LSA_ID : bit (24) >
// 			          |1 < ShortLSA_ID : bit (10) >} } ** 0 ;
//
// *******************************************************************

class IeLSAParameters : public IeBase {
public:

    IeLSAParameters() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetLSAParameters(unsigned char numIds, unsigned int *ids);   
    RlcMacResult SetLSAParameters(unsigned char numIds, unsigned short *shortIds);
    
    void GetLSAParameters(unsigned char *numIds, unsigned int *ids)
    {
        *numIds = numFreqOrCells;
        ids = LSAIdInfo.lsaIds;
    }
   
    void GetLSAParameters(unsigned char *numIds, unsigned short *shortIds)
    {
        *numIds = numFreqOrCells;
        shortIds = LSAIdInfo.shortLsaIds;
    }
   
    void ClearLSAParameters() { isValid = FALSE; }
   
private:
    unsigned char    numFreqOrCells;
    enum IdType {SHORT, LONG} type;
    union LSAIdStruct
    {
        unsigned int     lsaIds[MAX_FREQ_OR_CELLS];
        unsigned short   shortLsaIds[MAX_FREQ_OR_CELLS];
    } LSAIdInfo;
};

#endif