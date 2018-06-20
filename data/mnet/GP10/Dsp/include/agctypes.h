/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: agctypes.h
*
* Description:   
*   This file contains defined constants, enumerations and type definitions
*   of shared items to support automatic gain control and dynamic power
*   control.
*
*****************************************************************************/
#ifndef _AGCTYPES_H_
#define _AGCTYPES_H_  /* include once only */

#include "dsp/dsptypes.h"
                          
#define AGC_SMOOTH_MULT    15   /* 1-alpha term, multiply by 15 for IIR power smoothing */
#define AGC_SMOOTH_SHIFT    4   /* 1-alpha term, divide   by 16 for IIR power smoothing */

#define RXPWR_TARGET       -20  /* rcvPower target for AGC (dBr) */
#define SAT_RXPWR_TARGET   -35  /* rcvPower target after saturated access burst (dBr) */
#define MEAS_AGCDPC_DBM    -90  /* dBm target for calculating slope */
#define DPC_DBM_THRESH     -80  /* dBm target for the mobiles used by UL power control */

/* ********************************************************************************** */
/* These constants all depend on the frequency band of the radio board */

#define DIVISION_FACTOR     8  /* Shift to avoid cycle costly divide */

/* slope of the amplifier gain 1800 radio characteristic  */
#define GAIN_STEPS_PER_DB_1800        (3456>>DIVISION_FACTOR) 
#define GAIN_STEPS_PER_DB_INV_1800    19  /* x, where (x>>8) = 19/256 = 1/13.50 = 1/slope */

/* slope of the amplifier gain 1900 radio characteristic  */
#define GAIN_STEPS_PER_DB_1900        (3855>>DIVISION_FACTOR) 
#define GAIN_STEPS_PER_DB_INV_1900    17  /* x, where (x>>8) = 17/256 = 1/15.06 = 1/slope */

#define MEAS_AGCDPC_RXGAIN_1800   0x190   /* rx gain at which -90 dBm produces -20 dBr */
#define MEAS_AGCDPC_RXGAIN_1900   0xd0    /* rx gain at which -90 dBm produces -20 dBr */

/* ********************************************************************************** */

#define DL_PWR_CONTROL_MAX     0xff     /* max DL power control DAC value */
#define DL_PWR_CONTROL_MIN     0x00     /* min DL power control DAC value */

#define RXGAIN_MAX             0x80     /* rx gain is inversely proportional to DAC voltage */
#define RXGAIN_MIN_IDLE_1800   0x280    /* min gain = highest DAC control value */
#define RXGAIN_MIN_IDLE_1900   0x180
#define RXGAIN_MIN_ACTIV       0x3ff

#define RXGAIN_INIT            0x280    /* initial rx gain for RACH and MGC */

#define RXGAIN_MAX_ADJUST        20  /* max neg adjust to AGC smooth power per update */ 

#define ONE64THS_DB_2_DB_CONV     6  /* shift for mult or divides for conversions of 1dB <-> 1/64 dB */

#define AGC_TS_OFFSET             2  /* number of timeslots to skip in ISR between current
                                        TS and the TS to receive its uplink gain value */
typedef struct
{
  Uint32 slope;
  Uint16 inverseSlope;
  
} t_gainStepsPerDb;

#endif /* end of include once only */


