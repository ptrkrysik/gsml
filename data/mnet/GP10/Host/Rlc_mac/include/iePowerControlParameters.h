// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePowerControlParameters.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEPOWERCONTROLPARAMETERS_H__
#define __IEPOWERCONTROLPARAMETERS_H__

#include "IeBase.h"
#include "bitstream.h"
#include "RlcMacCommon.h"

// *******************************************************************
// class IePowerControlParameters
//
// Description:
//    Power Control Parameters IE -- GSM04.60 12.13
//
// < Power Control Parameters IE > ::=
//	 < ALPHA : bit (4) >
//	 { 0 | 1 < GAMMA_TN0 : bit (5) > }
//	 { 0 | 1 < GAMMA_TN1 : bit (5) > }
//	 { 0 | 1 < GAMMA_TN2 : bit (5) > }
//	 { 0 | 1 < GAMMA_TN3 : bit (5) > }
//	 { 0 | 1 < GAMMA_TN4 : bit (5) > }
//	 { 0 | 1 < GAMMA_TN5 : bit (5) > }
//	 { 0 | 1 < GAMMA_TN6 : bit (5) > }
//	 { 0 | 1 < GAMMA_TN7 : bit (5) > } ;
//
// *******************************************************************

#define MAX_NUM_GAMMAS		8

typedef struct
{
	bool	isValid;
	unsigned char gamma;
} GammaStruct;
    
class IePowerControlParameters : public IeBase {
public:

    IePowerControlParameters() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetPowerControlParameters(unsigned char alp, GammaStruct *gammas);   
    
    void GetPowerControlParameters(unsigned char *alp, GammaStruct *gammas)
    {
        *alp = alpha;
        for (int i = 0; i < MAX_NUM_GAMMAS; i++)
        {
            gammas[i] = gammaTn[i];
        }
    }
   
    void ClearPowerControlParameters() { isValid = FALSE; }
   
private:
    unsigned char   alpha;
    GammaStruct   	gammaTn[MAX_NUM_GAMMAS];
};

#endif