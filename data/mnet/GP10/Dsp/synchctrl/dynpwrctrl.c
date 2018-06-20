/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: dynpwrctrl.c
*
* Description:
*   This file contains functions that support dynamic uplink power control 
*
* Public Functions:
*
* Private Functions:
*
******************************************************************************/ 
#include "stdlib.h"
#include "gsmdata.h"
#include "oamdata.h"
#include "dsp/dsphmsg.h"
#include "dsprotyp.h"
#include "bbdata.h"
#include "l1config.h"
#include "agcdata.h"

/*****************************************************************************
* Function: uplink_dynamic_power_control
*
* Description:
*   This function is derived from the GSM spec 05.08 to perform 
*   dynamic power control for the uplink sacch processing. 
*
******************************************************************************/ 
void uplink_dynamic_power_control(t_TNInfo *ts, Uint8 subChan)
{
   t_ULSignalStrength   *sigStren;
   Int16	               *pav;
   Int16                delta; 
   Int8                 pwrAdj;
   Int8                 previousMSPwr;
   Int8                 newMSPwr;
   Uint8                channelNumber;
   t_SACCHInfo          *sacchInfo;
   Uint8                msgBuf[12];

   /*
   *  Point to the appropriate SACCH info structure
   */
   if ( ts->groupNum == GSM_GROUP_1 )
   {
      sacchInfo = & ts->u.group1.sacch; 								  
   }
   else if ( ts->groupNum == GSM_GROUP_5 )
   {
      sacchInfo = & ts->u.group5.sacch[subChan]; 								  
   }
   else if ( ts->groupNum == GSM_GROUP_7 )
   {
      sacchInfo = & ts->u.group7.sacch[subChan]; 								  
   }
   else   
   {
      return;   /* should not get here */ 
   }
   
   /*
   *  Check for minimum number of SACCH intervals since last MS Power Cmd
   */
   if ( sacchInfo->dynPwrRprtCntr >= g_DynPwrOAMInfo.ulsigstren.numRxSigSamples )
   {
      sigStren = & sacchInfo->ulsigstren;
      
      /* compute difference (dB) between current avg RXLEV and MIB target */   
      delta = - (sigStren->rxLevAve - g_DpcRxLevThresh); 

      /* divide delta by 2 b/c of 2dB steps for pwr control*/
      pwrAdj = - ((delta+1) >> 1);   /* add 1 so that there is no negative bias */ 


      previousMSPwr = sacchInfo->powerLevel; 
      sacchInfo->powerLevel += pwrAdj;

      /* range check to 4 bit values */
      if (sacchInfo->powerLevel > MS_TXPWR_MIN_DEF)   /* saturation */
      {
         sacchInfo->powerLevel = MS_TXPWR_MIN_DEF;
      }
      else if (sacchInfo->powerLevel < g_HoOAMInfo.serveParms.msTxPwrMax)
      {
         sacchInfo->powerLevel = g_HoOAMInfo.serveParms.msTxPwrMax;
      }	   

      /*
      *  If MS commanded power changes, send diag msg and reset SACCH 
      *  interval counter. Power cannot change again until counter expires.
      */
      delta = sacchInfo->powerLevel - previousMSPwr;
      if ( delta != 0 )
      {  
         msgBuf[0] = sacchInfo->powerLevel;
         msgBuf[1] = sigStren->rxLevAve>>8;	           
         msgBuf[2] = sigStren->rxLevAve & 0xff;		         
         msgBuf[3] = ts->u.group1.sacch.timingAdvance;
         msgBuf[4] = sigStren->rxPwrNormSmooth>>8;
         msgBuf[5] = sigStren->rxPwrNormSmooth & 0xff;
         msgBuf[6] = (sigStren->rxgain >> 8) & 0xff;
         msgBuf[7] = sigStren->rxgain & 0xff;        
         sendDiagMsg(POWER_CONTROL_REPORT,0,ts->tsNumber,8,msgBuf);
         
         sacchInfo->dynPwrRprtCntr = 0; 
      } 
   }
   return;
}


/* *********************************************************
 *	Routines: 
 * 		void averageS(), void averageQ() 
 *                   
 *	Description:  Averaging functions used by gsm 05.08 processes. 
 *               Recomputes sum of power or BER each time.
 *
 *  These routines currently make no provision for weighted averages.
 *  The maximum buffer size available is for 32 samples.     
 * 
 *  INPUTS:
 *
 *  arg1: pointer to the buffer that contains numbers to be averaged.
 *        Structure should contain the following 
 *         (not necessarily this order):
 *        ------------------------
 *       |     values[bufSiz]     |
 *        ------------------------
 *       |     count              |
 *        ------------------------
 *       |     sum of values      |
 *        ------------------------ 
 *       |     average            | 
 *        ------------------------
 *       where the count is used for the circular averaging buffer and
 *       all values sized depending on the defined structure.
 *
 *  arg2: buffer size of the values to be averaged.
 *  arg3: newest sample
 *  
 *  
 * OUTPUT(S): averageS - value sent back to calling function
 *             averageQ - value (placed in the buffer). 

 /* Example from sacch.c : 
    average(& ts->u.group1.sacch.ulsigstren, 
            numRxSigSamples, & ts->u.group1.sacch.MSPowerAccum); 
*
*/																                                                          
/* ************************************************************ */
void averageS (t_ULSignalStrength *s, t_DLSignalStrengthSCell *sD,
               Uint8 avgSiz, Int16 *newSmpl, Bool isUplink)
{
   Int8  index;
   Int32 sum;
   Int8  i;
   Int8  *rxLev;
   Int8  *rxLevAve;
   Uint8 *count;
   
   /*
   * Set local pointers to appropriate storage locations (UL or DL)
   */
   
   if ( isUplink )
   {
      rxLev = s->rxLev;
      rxLevAve = & s->rxLevAve;
      count = & s->count;
   }
   else
   {
      rxLev = sD->rxLev;
      rxLevAve = & sD->rxLevAve;
      count = & sD->count;
   }
   
   /*
   * Sum the most recent avgSiz power values. Buffer is circular,
   * and new sample belongs at location s->count or sD->count.
   */
  
   sum = *newSmpl;
   index = *count;
   for ( i=0; i<avgSiz-1; i++ )
   {
      if ( --index < 0 )
      {
      index = NUM_RX_SIGNAL_SAMPLES_MAX - 1;
      } 
      sum += rxLev[index];
   }
   
   /*
   * Compute the average RXLEV and store away the new sample
   */
   *rxLevAve = ( sum + (avgSiz>>1) ) / avgSiz;
   rxLev[*count] = *newSmpl;
   
   /*
   * Increment count to the oldest remaining value
   */
   if ( ++(*count) >= NUM_RX_SIGNAL_SAMPLES_MAX )
   {
      *count = 0;
   }	

   return;
}

/*****************************************************************************
* Function: averageQ
*
* Description: See above
*
******************************************************************************/ 
void averageQ (t_ULSignalQuality *q, t_DLSignalQualitySCell *qD, Uint8 avgSiz,
               Uint16 *newSmpl, Bool convertNewSmpl)  
{      
   Int8     index;
   Int32    sum;
   Uint8    newRxqual;
   Uint16   medianBer;
   Int8     i;

   /*
   * Convert newest BER value to RXQUAL, if needed. From the most recent 
   * avgSiz RXQUAL values, loop up BER and compute the sum.   
   * Buffer is circular, new sample belongs at location q->count or qD->count.
   */

    switch (convertNewSmpl) {

      case TRUE:

        newRxqual = ber2rxQualConv(*newSmpl);
        medianBer = g_rxQual2ber_LookUpTbl[ newRxqual ]; 
        sum = medianBer; 

        index = q->count;

        for ( i=0; i<avgSiz-1; i++ )
        {
          if ( --index < 0 )
          {
            index = NUM_RX_SIGNAL_SAMPLES_MAX - 1;
          }
          sum += g_rxQual2ber_LookUpTbl[ q->rxQual[index] ];
        }
        /*
        * Compute the average power and store away the new sample
        */
        q->berAve = ( sum + (avgSiz>>1) ) / avgSiz;
        q->rxQualAve = ber2rxQualConv(q->berAve);
        q->rxQual[q->count] = newRxqual;
   
        /*
        * Increment count to the oldest remaining value
        */
        if ( ++(q->count) >= NUM_RX_SIGNAL_SAMPLES_MAX )
        {
          q->count = 0;
        }

        break;


      case FALSE:
        /* added for DL RXQUAL averaging where we never get BER from DL meas. report */

        newRxqual = *newSmpl;  
        sum = g_rxQual2ber_LookUpTbl[ newRxqual ]; 

        index = qD->count;

        for ( i=0; i<avgSiz-1; i++ )
        {
          if ( --index < 0 )
          {
            index = NUM_RX_SIGNAL_SAMPLES_MAX - 1;
          }
          sum += g_rxQual2ber_LookUpTbl[ qD->rxQual[index] ];
        }
        /*
        * Compute the average power and store away the new sample
        */
        qD->berAve = ( sum + (avgSiz>>1) ) / avgSiz;
        qD->rxQualAve = ber2rxQualConv(qD->berAve);
        qD->rxQual[qD->count] = newRxqual;
   
        /*
        * Increment count to the oldest remaining value
        */
        if ( ++(qD->count) >= NUM_RX_SIGNAL_SAMPLES_MAX )
        {
          qD->count = 0;
        }

        break;
    }

   return;
}

/*****************************************************************************
*
* Function: ber2rxQualConv()
*
* Description:
*   Conversion utility for translating bit error rate (ber) to rxQual [0..7]
*   using the limits taken from gsm05.08 section 8.2.4.
*
* Input: 
*   ber                       in units of 0.01% (resolution = 1/10000)
*                              (e.g. 14 = 0.14%)
*
* Output:
*   rxQual                    from sacchInfo.sigqual or dlsigqualSCell
*      or
*   rxQualAve                 averaged over OAM defined period (hreqave)
*
*
******************************************************************************/
Uint8 ber2rxQualConv(Uint16 berval)
{
  Uint8 rxqualval;
  
  if ( berval < 20 ) {
    rxqualval = 0;
  }
  
  else if ( berval < 40 ) {
    rxqualval = 1;
  }

  else if ( berval < 80 ) {
    rxqualval = 2;
  }

  else if ( berval < 160 ) {
    rxqualval = 3;
  }

  else if ( berval < 320 ) {
    rxqualval = 4;
  }

  else if ( berval < 640 ) {
    rxqualval = 5;
  }

  else if ( berval < 1280 ) {
    rxqualval = 6;
  }

  else {   /* berval >= 12.80 % or 1280 */
    rxqualval = 7;
  }

  return (rxqualval);
}  
