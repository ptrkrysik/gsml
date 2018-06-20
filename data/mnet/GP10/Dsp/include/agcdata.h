/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: agcdata.h
*
* Description:   
*   This file contains references to the global structures and variables
*   used in automatic gain control (AGC) and dynamic power control (DPC).
*   It includes a file containing the defined constants, enumerations and
*   type definitions that support AGC and DPC.
*
*   NOTE: INGLOB and EXGLOB specify internal and external DSP memory
*         allocation, respectively. They are defined in dsptypes.h
*         and included here through agctypes.h.  
*
*****************************************************************************/
#ifndef _AGCDATA_H_
#define _AGCDATA_H_  /* include once only */

#include "agctypes.h"

INGLOB t_gainStepsPerDb  g_gainStepsPerDb;

INGLOB Uint8    g_AGC_ON;        /* AGC on/off for all time slots */
INGLOB Uint8    g_DPC_ON;        /* DPC on/off for all time slots */
INGLOB Uint16   g_DSPC;          /* Down Link Static Power DAC value */
INGLOB Uint16   g_USGC;          /* Up Link Static Gain DAC value */
INGLOB Int8     g_TxPwrTemperatureDelta; /* Tx DAC offset due to temperature */

INGLOB Short    g_AgcRxPwrTarget; /* AGC's sample power target in dBr */
INGLOB Int16    g_DpcRxLevThresh; /* UL RXLEV thresh for MS Pwr Control */
INGLOB Uint16   g_TxStaticGainLevel;

/*
*  Uplink signal power measurements are calibrated by one controlled
*  measurement of input power (dBm) at a particular ADC sample power
*  (dBr) and uplink gain (rxgain DAC value). These triplet values are:
*/  
INGLOB Int16    g_MeasAgcDpcDbm;    /* dBm of measured dBm/dBr/rxgain triplet */ 
INGLOB Int16    g_MeasAgcDpcDbr;    /* dBr of measured dBm/dBr/rxgain triplet */
INGLOB Uint16   g_MeasAgcDpcRxgain; /* rxgain of dBm/dBr/rxgain triplet */
 
INGLOB Uint16   g_rxGainMinAct;
INGLOB Uint16   g_rxGainMinIdle;

#endif  /* end of include once only */
 


