// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieGlobalPowerControlParms.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************

#include "ieGlobalPowerControlParms.h"
#include "logging\vclogging.h"
#include "logging\vcmodules.h"

char pcMeasChanNames[][64] =
{
    "downlink measurements for power control on BCCH",
    "downlink measurements for power control on PDCH"
};

char intMeasChanAvailNames[][64] =
{
    "PSI4 message not broadcast",
    "PSI4 message broadcast"
};


// *******************************************************************
// class IeGlobalPowerControlParms
//
// Description:
//    Global Power Control Parameters IE -- GSM04.60 12.9
//
// < Global Power Control Parameters IE > ::=
//   < ALPHA : bit (4) >
//   < T_AVG_W : bit (5) >
//   < T_AVG_T : bit (5) >
//   < Pb : bit (4) >
//   < PC_MEAS_CHAN : bit (1) >
//   < INT_MEAS_CHANNEL_LIST_AVAIL : bit (1) >
//   < N_AVG_I : bit (4) > ;
// *******************************************************************


RlcMacResult IeGlobalPowerControlParms::EncodeIe(BitStreamOut &dataStream)
{
    DBG_FUNC("IeGlobalPowerControlParms::EncodeIe", RLC_MAC);
    DBG_ENTER();
    
    RlcMacResult result = RLC_MAC_SUCCESS;

    // Pack Global Power Control Pararmeter bits into the output bit stream.
    if (isValid)
    {
        dataStream.InsertBits8(alpha, 4);
        dataStream.InsertBits8(tAvgW, 5);
        dataStream.InsertBits8(tAvgT, 5);
        dataStream.InsertBits8(pb, 4);
        dataStream.InsertBits8((unsigned char)pcMeasChan, 1);
        dataStream.InsertBits8((unsigned char)intMeasChanAvail, 1);
        dataStream.InsertBits8(nAvgI, 4);
    }       
       
    DBG_LEAVE();
    return (result);
}


void IeGlobalPowerControlParms::DisplayDetails(DbgOutput *outObj)
{
    if (isValid)
    {
        outObj->Trace("\tGlobal Power Control Parameters\n");
        outObj->Trace("\t\tAlpha -----> %f\n", 
            (alpha > 10) ? 1.0 : (float) (alpha * 0.1));
        outObj->Trace("\t\tT AVG W ---> %d\n", tAvgW);
        outObj->Trace("\t\tT AVG T ---> %d\n", tAvgT);
        outObj->Trace("\t\tPb --------> -%d dB\n", pb*2);
        outObj->Trace("\t\tPC MEAS CHAN\n");
        outObj->Trace("\t\t\t%s\n", pcMeasChanNames[pcMeasChan]);
        outObj->Trace("\t\tINT_MEAS_CHAN LIST AVAIL\n");
        outObj->Trace("\t\t\t%s\n", intMeasChanAvailNames[intMeasChanAvail]);
        outObj->Trace("\t\tN AVG I ---> %d\n", nAvgI);
    }
}