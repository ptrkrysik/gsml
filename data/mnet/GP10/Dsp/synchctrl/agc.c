/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: agc.c
*
* Description:
*   This file contains processing needed for uplink automatic 
*   gain control and uplink dynamic power control algorithms. 
*
* Public Functions:
*
* Private Functions:
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h"
#include "dsp/dsphmsg.h"
#include "bbdata.h"
#include "dsprotyp.h"
#include "l1config.h"
#include "diagdata.h" 
#include "agcdata.h"

/*****************************************************************************
*
* Function: ul_agc()
*
* Description:
*   Automatic Gain Control of the received power from the RF dual radio amplifier.
*
*
* Input: 
*   *sigstren       ptr to t_ULSignalStrength structure
*
* Output:
*  .rxgain          inside t_ULSignalStrength
*  .rxPwrNormSmooth   "
*
*        this value gets updated periodically (originally 120ms or 1/4 SACCH
*            multiframe) by smoothingFilter().  The updated value gets transferred
*            to the FPGA (and consequently the RF board) via the burstisr.c's 
*            ulBurstProcIsr( ) function every 4.615ms per timeslot. 
*                                            
*
******************************************************************************/
Int16 ul_agc(t_ULSignalStrength *sigstren)
{
   Int16  dBDelta;  
   Int16  agcDelta;
   Int16  rxgainTest; 
  
   dBDelta = (Int16) (sigstren->rxPwrNormSmooth >> ONE64THS_DB_2_DB_CONV);
   agcDelta = dBDelta * g_gainStepsPerDb.slope;                     
   
   /* limit to an unsigned 10-bit value */   
   rxgainTest = sigstren->rxgain + agcDelta;

   if ( rxgainTest > g_rxGainMinAct )
   {  
     agcDelta = g_rxGainMinAct - sigstren->rxgain;
     sigstren->rxgain = g_rxGainMinAct;                
   }
   else if ( rxgainTest < RXGAIN_MAX )
   {
     agcDelta = RXGAIN_MAX - sigstren->rxgain;
     sigstren->rxgain = RXGAIN_MAX;
   } 
   else
   {
     sigstren->rxgain += agcDelta;
   }  

   /* update, multiply by inverseSlope/256 (where = 1/GAIN_STEPS_PER_DB) dB    */
   /*  and multiply by 64 to 1/64th dB units, instead of step units            */       
   /* rxPwrNormSmooth = rxPwrNormSmooth - (64 * agcDelta * (inverseSlope/256)) */
   /*                 = rxPwrNormSmooth - (agcDelta * inverseSlope) >> 2       */
   sigstren->rxPwrNormSmooth -= ((agcDelta * g_gainStepsPerDb.inverseSlope) >> 2); 

   return (sigstren->rxPwrNormSmooth);
}



/*****************************************************************************
*
* Function: rcvPwrSmoothingFilter()
*
* Description:
*   IIR Smoothing Filter: smooth the received power from the RF dual radio amplifier.
*
* Input: 
*   *sigstren    ptr to t_ULSignalStrength structure
*   rcvPower	 accumulated received power
*
* Output:
*   .rcvPowerNormSmooth             inside t_ULSignalStrength
*
*        this value gets updated periodically via a closed-loop smoothing filter 
*        that is IIR with a difference equation of y[n] = (1-alpha)*y[n-1] + (alpha)*x[n].
*
*
                       alpha
             x[n]  o----->-----o----->-----o----->-----o  y[n]
                               |           |
                               |           |  -1
                               |           | z
                               |           |
                               o-----<-----o

                                 (1-alpha)
*                                            
*
******************************************************************************/
Int16 rcvPwrSmoothingFilter(t_ULSignalStrength *sigstren, Short rcvPower)
{
  Int16 testPower, powerAdjust;
  Int32 accum;
 									
  /* notes: assumes rcvPower (dspSacchInfo->MSPowerAccum) in 1 dB steps.
  *         assume normalized, smoothed rcvPower is in 1/64 dB steps.
  */  

  /*-----------------------------------------------------------------------
  * Compute dB relative to desired operating point of g_AgcRxPwrTarget dB
  * Shift the entire range by an offset equal to g_AgcRxPwrTarget dB in 
  * order to get sign of values used for gain adjustment up or down.  
  * The magnitude of the amplifier gain remains unaffected by this.  
  *----------------------------------------------------------------------*/
  testPower = rcvPower - g_AgcRxPwrTarget;

  /*------------------------------------------------------------------------
  * IIR Smoothing Filter:
  * Update smoothed power in units of 1/64 dB. Remember testPower units
  * are 1 dB per step. Convert units to fractions of a dB
  * for extra resolution.  
  * Never update by less than -RXGAIN_MAX_ADJUST dB. 
  *-----------------------------------------------------------------------*/       
  powerAdjust = testPower << ONE64THS_DB_2_DB_CONV;

  accum = sigstren->rxPwrNormSmooth * AGC_SMOOTH_MULT;

  /* one-sided limiting */
  accum += (powerAdjust > -(RXGAIN_MAX_ADJUST << ONE64THS_DB_2_DB_CONV)) ?
            powerAdjust : -(RXGAIN_MAX_ADJUST << ONE64THS_DB_2_DB_CONV);   
  
  accum >>= AGC_SMOOTH_SHIFT;          

  sigstren->rxPwrNormSmooth = (Int16) accum;

  return (sigstren->rxPwrNormSmooth);
}


