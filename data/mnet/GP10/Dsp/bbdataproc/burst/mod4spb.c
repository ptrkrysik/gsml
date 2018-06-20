/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
**
** File mod4bpb.c
**
** Description:   
**   This file contains the GSM GMSK modulator routine.
**
*****************************************************************************/
#include "gsmdata.h"
#include "bbdata.h"
#include "bbproto.h"
#include "mod4spb_tab.h"  

/*****************************************************************************
** ROUTINE NAME: mod4spb
**
** Description:
**   Look-up-table based GSM GMSK 4 samples per bit modulator.
**
** Method:
**   This function generates a GSM GMSK modulated waveform from a sequence
**   of bits.  Power ramping is also performed.
**
**   148 bits are input: The whole burst including tail bits but excluding
**   guard period bits.
**
**   The ramped samples (corresponding to the guard period and the extreme
**   tail bit on either end) are pre-calculated and simply copied to the
**   output buffer.  The pre-calculation assumes that guard period bits are
**   1's and the outer 3 tail bits on either end are 0's as indicated in
**   GSM 05.02 for the NB.  This is not exactly true for the AB which has
**   0,0,1 as its 1st three tail bits, but close enough to produce acceptable
**   results.
**
**   Output is 148 bits of the burst and 9 (actually 8.25) bits of the guard
**   period.  4 guard period bits are added at the beginning and 5 (actually
**   4.25) are added at the end.  156.25 bits (625 samples) are output.
**
** Error Handling: 
**
** Limitations: 
**   The output buffer pointer MUST point to a buffer that is 32-bit
**   aligned.
** 
** Parameter I/O: 
**    waveform - output array of the modulated waveform
**    bits - input bits array
**    bursttype - input indicating which burst type
**
** Returns: 
**    None
**
** Data Store  I/O: 
**
*****************************************************************************/
void GMSKModulate(t_DLComplexSamp *outBurstPtr, t_chanSel chanSel)
{
   UChar diff[MOD_NUM_BITS_IN + 2];
   t_DLComplexSamp integerPhase;
   Short temp;
   Int bitsToModulate;
   Int index;
   Int outputIndex;
   Int i, k;
   Int32 tempIndex;
                        
   /*
   * Check if transmission is enabled. If not, output all 0's.
   */
   /*if (g_DLBurstData.tx == FALSE)
    if ( g_burstProc.dlTN == 3 || g_burstProc.dlTN == 5  || g_burstProc.dlTN == 7)
   { 
      for (i=0; i<BURST_SIZE_DL; i++)
      {
         *(Int *)&outBurstPtr[i] = 0;
      }
   }   
   else*/  
   {
      /*
      * Transmission is enabled, clear flag then perform modulation.
      */
      g_DLBurstData.tx = FALSE;
      
      /*
      * Differentially encode input bit stream.  The input bit stream is
      * effectively pre-pended with two 1's and post-pended with one 1. 
      * The differentially encoded array is 150 bits long, and modulated
      * samples corresponding to only the center 146 bits are generated. 
      * The samples corresponding to bits before and after these 146 bits
      * are known a-priori and have been pre-computed.
      */
      diff[0] = 1;
      diff[1] = g_DLBurstData.data[0];
      diff[MOD_NUM_BITS_IN + 1] = g_DLBurstData.data[MOD_NUM_BITS_IN - 1];
   
      for (i=1; i<(MOD_NUM_BITS_IN+1); i++)
      {
         diff[i+1] = (g_DLBurstData.data[i] == g_DLBurstData.data[i-1]);
      }

      /*********************************************************************
      ** Use look-up table to convert bits into a waveform
      ** Accumulate the integer phase and modify the waveform accordingly
      *********************************************************************/
   
      /*
      * Initialize the integer phase to 0
      */
      integerPhase.I = 1;
      integerPhase.Q = 0;
   
      /*
      * Number of bits to modulate depends on the burst type
      */
      if (chanSel == RACH)
      {
         bitsToModulate = AB_BITS_TO_MOD;
      }
      else
      {
         bitsToModulate = NB_BITS_TO_MOD;
      }
   
      /*
      * The LUT is indexed based on the 5 most recent differentially encoded
      * bits. Initialize the index.  Newest bit is the least significant.
      */
      index = diff[4] + diff[3]*2 + diff[2]*4 + diff[1]*8 + diff[0]*16;
      
#ifdef DYNAMIC_RAMPING                 
      /*
      * Copy the ramp-up waveform to the output buffer
      */    
      outputIndex = 0;
      
      for (i=0; i<RAMP_SAMPLES; i++)
      {
         *(Int16 *) & (outBurstPtr[outputIndex].I) = modRampUp[i*2].I;
         *(Int16 *) & (outBurstPtr[outputIndex++].Q) = (modRampUp[i*2].Q) >> 4;         
       }  
       
      for (i=0; i<RAMP_SAMPLES; i++)
      {             
         *(Int32 *) & (outBurstPtr[i]) = *(Int32 *) & (outBurstPtr[i]) >>4;
      }
                     
      /*
      * Loop over the center 146 bits computing the output samples associated
      * with each. i points to the bit corresponding to the output samples
      * currently being generated.  Four pre-pended guard bits and the leading
      * tail bit are pre-calculated.  The first output sample corresponds to
      * the 6th bit period, therefore the output index is init'ed to skip the
      * first 5 bit periods.
      */  
#else      
      outputIndex = (5 * SAMPLES_PER_BIT);      
#endif

      for (i=2; i<(bitsToModulate+2); i++)
      {
         /*
         * For each phase of the output, index the LUT to determine the
         * output sample. The integrated phase is used to determine any
         * additional N*90 degree offset.
         */             
         
         tempIndex =  outputIndex;
                 
         for(k=0; k<SAMPLES_PER_BIT; k++)
         {
            *(Int16 *) & (outBurstPtr[outputIndex].I) = 
               (integerPhase.I * modLUT4x[index][k*2].I -
                integerPhase.Q * modLUT4x[index][k*2].Q);
               
           *(Int16 *) & (outBurstPtr[outputIndex++].Q) =
               (integerPhase.I * modLUT4x[index][k*2].Q +
                integerPhase.Q * modLUT4x[index][k*2].I) >> 4;
         }
               
         for(k=0; k<SAMPLES_PER_BIT; k++)
         {                                      
       
            *(Int32 *) & (outBurstPtr[tempIndex]) = 
                *(Int32 *) & (outBurstPtr[tempIndex++]) >> 4;
         }                
                
         /*
         * Accumulate the integer phase modulo 2*pi (-2,-1,0, or 1).
         * The oldest bit is accumulated since it will not be within the
         * [-2:+2] window in the next iteration.
         * If diff=1,rotate 90 deg, else -90 deg.
         */
         temp = integerPhase.I;
         integerPhase.I = ((diff[i-2]) ? -integerPhase.Q :  integerPhase.Q);
         integerPhase.Q = ((diff[i-2]) ?  temp : -temp);
      
         /*
         * Update the LUT index, taking into account the newest bit influencing
         * the output and disreguarding the oldest.
         */
         index = ((index<<1) & 0x1E) + diff[i+3];
 
      }
#ifdef DYNAMIC_RAMPING   

      /*
      * Copy the ramp-down waveform to the output buffer.  index points to
      * the beginning of where the ramped samples are copied.
      */
      if (chanSel == RACH)
      {
         /*
         * Access Burst: init index and zero fill the extended guard period.
         */
         index = (BURST_SIZE_DL/2 - 1 - RAMP_SAMPLES)/ - \
                 (NB_BITS_TO_MOD - AB_BITS_TO_MOD)*SAMPLES_PER_BIT;
                  
         for (i=(BURST_SIZE_DL/2 - 1 - RAMP_SAMPLES ) - \
                (NB_BITS_TO_MOD - AB_BITS_TO_MOD)*SAMPLES_PER_BIT + \
                RAMP_SAMPLES;
              i<BURST_SIZE_DL/2; i++)
         {
            *(Int *)&outBurstPtr[i] = 0;
         }
      }
      else
      {
         /*
         * Not an Access Burst: init index
         */
         index = BURST_SIZE_DL/2-1-RAMP_SAMPLES;
      }
        
      tempIndex =  outputIndex;   
      for (i=0; i<RAMP_SAMPLES; i++)
      {
         *(Int16 *) & (outBurstPtr[outputIndex].I) = modRampDown[i*2].I;
         *(Int16 *) & (outBurstPtr[outputIndex++].Q) = (modRampDown[i*2].Q)>>4;         
       }     
    
      
      for (i=0; i<RAMP_SAMPLES; i++)
      {               
         *(Int32 *) & (outBurstPtr[tempIndex]) = *(Int32 *) & (outBurstPtr[tempIndex++]) >>4;
      }
   
      /*  
      * The final sample is always zero
      */
      *(Int32 *) & outBurstPtr[outputIndex++] = 0;      
#endif  

   } 
}
