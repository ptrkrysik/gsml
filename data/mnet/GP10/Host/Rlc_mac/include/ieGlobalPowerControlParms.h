// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieGlobalPowerControlParms.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IEGLOBALPOWERCONTROLPARMS_H__
#define __IEGLOBALPOWERCONTROLPARMS_H__

#include "IeBase.h"
#include "bitstream.h"
#include "RlcMacCommon.h"

#define MAX_T_AVG_W         31
#define MAX_T_AVG_T         31
#define MAX_PB              15
#define MAX_N_AVG_I         15

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

class IeGlobalPowerControlParms : public IeBase {
public:
    
    typedef enum
    {
        DL_MEAS_ON_BCCH,
        DL_MEAS_ON_PDCH
    } PC_MEAS_CHAN_TYPE;
    
    typedef enum
    {
        PSI4_MSG_NOT_BROADCAST,
        PSI4_MSG_BROADCAST
    } INT_MEAS_CHAN_LIST_AVAIL_TYPE;
    
    IeGlobalPowerControlParms() : IeBase() {} 
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
    
    RlcMacResult SetAlpha(unsigned char val)
    {
        if (val <= MAX_ALPHA_VALUE)
        {
            alpha = val;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    unsigned char GetAlpha()
    {
        return (alpha);
    }
    
    RlcMacResult SetTAvgW(unsigned char val)
    {
        if (val <= MAX_T_AVG_W)
        {
            tAvgW = val;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    unsigned char GetTAvgW()
    {
        return (tAvgW);
    }
    
    RlcMacResult SetTAvgT(unsigned char val)
    {
        if (val <= MAX_T_AVG_T)
        {
            tAvgT = val;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    unsigned char GetTAvgT()
    {
        return (tAvgT);
    }
    
    RlcMacResult SetPb(unsigned char val)
    {
        if (val <= MAX_PB)
        {
            pb = val;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    unsigned char GetPb()
    {
        return (pb);
    }
    
    RlcMacResult SetPcMeasChan(PC_MEAS_CHAN_TYPE val)
    {
        pcMeasChan = val;
        return (RLC_MAC_SUCCESS);
    }
   
    PC_MEAS_CHAN_TYPE GetPcMeasChan()
    {
        return (pcMeasChan);
    }
    
    
    RlcMacResult SetNAvgI(unsigned char val)
    {
        if (val <= MAX_N_AVG_I)
        {
            nAvgI = val;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    unsigned char GetNAvgI()
    {
        return (nAvgI);
    }
    
    RlcMacResult SetIntMeasChanAvail(INT_MEAS_CHAN_LIST_AVAIL_TYPE val)
    {
        intMeasChanAvail = val;
        return (RLC_MAC_SUCCESS);
    }
   
    INT_MEAS_CHAN_LIST_AVAIL_TYPE GetIntMeasChanAvail()
    {
        return (intMeasChanAvail);
    }
    
    void SetGlobalPowerControlParms() { isValid = TRUE; }
    void ClearGlobalPowerControlParms() { isValid = FALSE; }
    
private:
    unsigned char                   alpha;
    unsigned char                   tAvgW;
    unsigned char                   tAvgT;
    unsigned char                   pb;
    PC_MEAS_CHAN_TYPE               pcMeasChan;
    unsigned char                   nAvgI;
    INT_MEAS_CHAN_LIST_AVAIL_TYPE   intMeasChanAvail;
};

#endif