// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : iePowerControlParameters.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "iePowerControlParameters.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"


// *******************************************************************
// class IePowerControlParameters
//
// Description:
//    Power Control Parameters IE -- GSM04.60 12.13
//
// < Power Control Parameters IE > ::=
//   < ALPHA : bit (4) >
//   { 0 | 1 < GAMMA_TN0 : bit (5) > }
//   { 0 | 1 < GAMMA_TN1 : bit (5) > }
//   { 0 | 1 < GAMMA_TN2 : bit (5) > }
//   { 0 | 1 < GAMMA_TN3 : bit (5) > }
//   { 0 | 1 < GAMMA_TN4 : bit (5) > }
//   { 0 | 1 < GAMMA_TN5 : bit (5) > }
//   { 0 | 1 < GAMMA_TN6 : bit (5) > }
//   { 0 | 1 < GAMMA_TN7 : bit (5) > } ;
//
// *******************************************************************

RlcMacResult IePowerControlParameters::SetPowerControlParameters(unsigned char alp, 
    GammaStruct *gammas)
{
    if (alp <= MAX_ALPHA_VALUE)
    {
        alpha = alp;
    }
    else
    {
        return (RLC_MAC_PARAMETER_RANGE_ERROR);
    }
    
    for (int i = 0; i < MAX_NUM_GAMMAS; i++)
    {
        gammaTn[i] = gammas[i];
    }
    isValid = TRUE;
    return (RLC_MAC_SUCCESS);
}




RlcMacResult IePowerControlParameters::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IePowerControlParameters::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Power Control Parameters bits into the output bit stream.
    if (isValid) 
    {
        dataStream.InsertBits8(alpha, 4);
        for (int i = 0; i < MAX_NUM_GAMMAS; i++)
        {
            if (gammaTn[i].isValid)
            {
                dataStream.InsertBits8(1, 1);
                dataStream.InsertBits8(gammaTn[i].gamma, 5); 
            }
            else
            {
                dataStream.InsertBits8(0, 1);
            }
        }
    }
    else
    {
        result = RLC_MAC_INVALID_IE;
        DBG_ERROR("IePowerControlParameters::EncodeIe Power Control Parms not valid\n");
    }
   
    DBG_LEAVE();
    return (result);
}


void IePowerControlParameters::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tPower Control Paramters\n");
        outObj->Trace("\t\tAlpha ---> %f\n", 
            (alpha > 10) ? 1.0 : (float) (alpha * 0.1));
        for (int i = 0; i < MAX_NUM_GAMMAS; i++)
        {
            if (gammaTn[i].isValid)
            {
                outObj->Trace("\t\tGAMMA_TN%d ---> %d dB\n", i, gammaTn[i].gamma<<1);
            }
        }
    }
}