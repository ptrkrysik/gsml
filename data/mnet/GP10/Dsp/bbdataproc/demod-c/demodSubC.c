/*
**  GSM Demod w/ A|RT Library implemtation
**
** This files contains all of the subroutines for the demod
**                
** $Author: whuang $
** $Date: 1999/07/09 02:43:12 $
** $Source: D:\\stageCell\\Dsp\\bbdataproc\\demod-c\\RCS\\demodSubC.c $
** $Revision: 1.1 $
** $Log: demodSubC.c $
** Revision 1.1  1999/07/09 02:43:12  whuang
** Initial revision
**
** Revision 1.2  1999-07-08 19:42:07-07  whuang
** <>
**
** Revision 1.1  1999-07-06 12:35:01-07  whuang
** Initial revision
**
** Revision 1.1  1999-01-07 14:30:23-08  randyt
** Initial revision
**
 * 
 *    Rev 1.0   Oct 27 1998 16:19:34   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   Oct 23 1998 17:56:08   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
** Revision 1.5  1998/10/14 18:43:26  bhehn
** AB & NB verified.  Merged with shared header.
** AB ~= 103us, NB ~= 78us
**
** Revision 1.4  1998/10/12 15:38:16  bhehn
** Fixed power computation bug.
**
** Revision 1.3  1998/10/09 21:20:35  bhehn
** Fast version, Not yet versified.
** AB ~ 125us, NB ~= 110us
**
** Revision 1.2  1998/10/09 15:54:00  bhehn
** Made I/O faster and fixed normEnergy() energyAccum probelm for large signals.
**
** Revision 1.1  1998/10/07 15:21:37  bhehn
** Initial revision
**
** Revision 1.4  1998/10/02 17:01:00  bhehn
** Changed calling routine to be more like c6x  implementation.
** BER matches previous version.
**
** Revision 1.3  1998/10/02 15:00:17  bhehn
** Rounding offset & impulse.  Getter much better BER for low level signals.
**
** Revision 1.2  1998/10/01 16:37:07  bhehn
** Fixed matchfilter output saturation problem
**
** Revision 1.1  1998/10/01 13:57:00  bhehn
** Initial revision
**
** Revision 1.6  1998/09/22 20:00:35  bhehn
** Access Burst support added and verified.
**
** Revision 1.7  1998/09/21 22:18:59  bhehn
** <>
**
** Revision 1.6  1998/09/21 22:18:35  bhehn
** <>
**
** Revision 1.5  1998/09/21 22:13:47  bhehn
** Outdated (renamed to '\\Fdi\public\rcs\D\rcs\projects\GSM\Art\demod\demodNBSubC.cxx,v').
**
** Revision 1.4  1998/09/21 22:04:22  bhehn
** NB demod works, BER close to floating
**
** Revision 1.4  1998/09/18 20:14:42  bhehn
** Outdated (renamed to '\\Fdi\public\rcs\D\rcs\projects\GSM\Art\demod\demodSubC.cxx,v').
**
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "dsp/dsptypes.h"
#include "gsmtypes.h"
#include "bbdata.h"
#include "bbproto.h"

#include "dem_const.h"
#include "training.h"
#include "dem_globs.h"
#include "dem_protos.h"
#include "c6xsupport.h"


Word   expX( UVLWord data, Word *exponent );
int smpyC( Short x, Short y);
Word mag2DB( UVLWord mag );
                    
#define LOOP_TYPE    int
                    
/*************************************************************************
*
*       offsetDerot() :
*       -------------
*               Offset computation + Offset compensation + Derotation
*
*
*       Input data      . samples[2][MAX_NUM_DATA]
*             . threshold (positive number)
*
*       Output data     . samples[2][MAX_NUM_DATA]
*
***************************************************************************/
void    offsetDerot( t_ULComplexSamp   samples[BURST_SIZE_UL_MAX], 
                     Word  threshold)

{
   
   VLWord  offsetI_L, offsetQ_L;
   Word    offsetI, offsetQ;
   Word    temp;
   
   
   LOOP_TYPE i; 
   
   
   /* Compute the Offset  */   
   offsetI_L = 0;
   offsetQ_L = 0;
   
   /* 
   **  Do offset compensation on the first NUM_OFFSET_SAMPS bits.
   **   Using OFF_NUM_OFFSET_SAMPS (not 156) to make scaling easy
   */  
   for ( i = 0; i < OFF_NUM_OFFSET_SAMPS; ++i) 
   {
      offsetI_L += samples[i+g_BConsts_pt->numTailBits].I;
      offsetQ_L += samples[i+g_BConsts_pt->numTailBits].Q;
   }
   
   /* Round (add 0.5*OFF_SHIFT_VAL then shift & truncate) */
   offsetI_L += 1<<(OFF_SHIFT_VAL-1);
   offsetQ_L += 1<<(OFF_SHIFT_VAL-1);

   /* Divide by OFF_NUM_OFFSET_SAMPS */
   offsetI = (Word) (offsetI_L >> OFF_SHIFT_VAL); /* Divide by NUM_OFFSET_SAMPS */
   offsetQ = (Word) (offsetQ_L >> OFF_SHIFT_VAL); /* Divide by NUM_OFFSET_SAMPS */
   
   /* Compare offset to threshold and saturate if needed */
   if ( offsetI > threshold) 
   {
      offsetI = threshold;
   } 
   if ( offsetI < (Word) (-1) * threshold) 
   {
      offsetI = (Word) (-1) * threshold ;
   }
   if ( offsetQ > threshold) 
   {
      offsetQ = threshold;
   }
   if ( offsetQ < (Word) (-1) * threshold)
   {
      offsetQ = (Word) (-1) * threshold ;
   }
   
   /* 
   **  DC Offset Compensation & Derotation
   **  Do the derotation over all of the samples of the burst
   **  including ending tail bits and gaurd bits.
   **  Don't need to do the beginning tail bits.
   **  Need to do the guard bits because we are not sure where the
   **  burst will arrive.
   **  Subtract 3 to insure we do not go into the next burst memory area.
   **  For normal burst numTailBits = 3 so samples 3 to 154 will be changed.
   **  For access burst numTailBits = 8 so samples 8 to 155 will be changed.
   **/
   for( i = g_BConsts_pt->numTailBits;  i < (GSM_NUM_SAMPLES_PER_BURST-3); i += (OFF_I_TYPE) 4 ) 
   {
           
      /* Derotate 1st sample */
      temp = samples[i].I;
      samples[i].I = SAT_WORD( samples[i].Q - offsetQ );
      samples[i].Q = SAT_WORD( offsetI - temp );
      
      /* Derotate 2nd sample */
      samples[i+1].I = SAT_WORD( offsetI - samples[i+1].I );
      samples[i+1].Q = SAT_WORD( offsetQ - samples[i+1].Q );
      
      /* Derotate 3rd sample */
      temp = samples[i+2].I;
      samples[i+2].I = SAT_WORD( offsetQ - samples[i+2].Q );
      samples[i+2].Q = SAT_WORD( temp - offsetI );
      
      /* Derotate 4th sample */
      samples[i+3].I = SAT_WORD( samples[i+3].I - offsetI );
      samples[i+3].Q = SAT_WORD( samples[i+3].Q - offsetQ );
   }
   
}


/*************************************************************************
*                                                                               
*       impulse() :                                                               
*       ---------                                                                 
*               Compute the channel impulse response                            
*                                                                               
*                                                                               
*       Input data      . Training Sequence : tseq[TRAIN_SIZE]                    
*                       . Derotated Samples : samples[2][MAX_NUM_DATA]               
*
*       Output data     . Channel impulse response : cir[2][CIR_LENGTH]            
*     
************************************************************************                                                                   
*  An illistration of "impulse()" operation is shown below for the NB:
*  NB impulse starts at an "offset" because it is desirable to correlate the tseq bits
*  against what is probably tseq bits of the samples.  If the samples were
*  shifted because the mobile moved then the offset of 3 will insure that
*  the correlation is still in the tseq bits.  Also, with the diagram below
*  the demod can find toa's from -2 to 2.
*
* Assumes: CIR_LENGTH = 9, CIR_NUM_TSEQ_BITS = 16, offset = 3
*          toaMin = -2
*
*                           offset
*                             | 
*                            ----------------------
*  tseq                     | 3   tseq values    19|   MAC         
*                            ---------------------- 
*                           cirStartIndex
*                             | 
*              0       60 61  v            
*   i = 0       -----------------------------------------------
*  samples[]   |         |   62     tseq        78     |            
*               ------------------------------------------------ 
*
*   i = 2                    ---------------------
*  samples[]                |63   samples[]     79|   MAC        
*             ...            --------------------- 
*   i = 7                    ---------------------
*  samples[]                |70   samples[]     86|   MAC        
*                            --------------------- 
*                            <--   CIR_NUM_TSEQ_BITS    --->
*                            ^
*                            |
*                          cirStartIndex+CIR_LENGTH
*
***************************************************************************/
void impulse(t_ULComplexSamp   samples[BURST_SIZE_UL_MAX],
             const t_TseqWord tseq[MAX_TSEQ_SIZE], 
             Word cir[2][MAX_CIR_LENGTH])
             
{
   VLWord         resultI, resultQ;
   unsigned char  start;
   unsigned char  offset; /* Offset from the tseqPostion to begin processing */
   LOOP_TYPE i;
   LOOP_TYPE j; 
   
   LWord  result; 

            
   offset = g_BConsts_pt->impulseTseqOffset;
   start  = g_BConsts_pt->cirStartIndex;
   
   /* Compute CIR over the window length */
   for ( i = 0; i < g_BConsts_pt->cirLength; ++i) 
   {  
      resultI = 0;
      resultQ = 0;
      
      
      /* For the number of tseq samples */
      for ( j = 0; j < g_BConsts_pt->cirNumTseqBits; ++j) 
      { 
         t_TseqWord temp;
         temp = tseq[j+offset];  
         
         /* Making these intrinsics made things worse */
         resultI += (VLWord) (samples[start + j + i].I * temp);
         resultQ += (VLWord) (samples[start + j + i].Q * temp); 

      }
      
      /* Normalize then convert to a Word */
      resultI = (resultI * g_BConsts_pt->cirNumTseqBitsInv);
      resultQ = (resultQ * g_BConsts_pt->cirNumTseqBitsInv);

      /* Round (add +/- 0.5*WORD_SIZE then shift & truncate) */
      resultI += 1<<(WORD_SIZE-1);
      resultQ += 1<<(WORD_SIZE-1);

      cir[0][i] = (resultI)>>WORD_SIZE;
      cir[1][i] = (resultQ)>>WORD_SIZE;
      
   }
}


/*************************************************************************
*                                                                               
*       Energy() :                                                        
*       --------                                                                  
*               Compute the energy of the impulse response               
*               Find the highest energy window of length CIR_SIZE                   
*                                                                               
*
*       Input data:      The channel impulse response: cir[2][CIR_LENGTH]        
*                                                                               
*       Output data:     Energy of the impulse response: leng[CIR_LENGTH]        
*                        Position of the best: returned by the function    
*                                                                               
***************************************************************************/
Char  Energy( Word cir[2][MAX_CIR_LENGTH], LWord energy_L[MAX_CIR_LENGTH] )
{
   VLWord  result;
   VLWord  result2;
   VLWord  key;
   Char    indexBest;
   LOOP_TYPE    i;
   
   /* 
   ** Energy Computation 
   */
   for ( i = 0; i < g_BConsts_pt->cirLength; ++i) {  
      
      result   = SMPY( cir[0][i] , cir[0][i] );
      result2  = SMPY( cir[1][i] , cir[1][i] ); /* (F16.15 * F16.15) <<1= F32.31 (one sign bits) */
      energy_L[i]  = result + result2; /* F32.31 + F32.31 = F32.31 (one sign bit ) */
   }
   
   /* 
   ** Find the highest energy window
   */
   key = 0;
   result = 0;
   indexBest = 0;
   /* 
   ** Search over CIR_LENGTH - CIR_SIZE so that the
   ** search does not exceed energy_L[]'s array bounds
   */
   for ( i = 0; i < (g_BConsts_pt->cirLength - CIR_SIZE); ++i) { 
      
      /* VLWord needed here */
      result += (energy_L[i + CIR_SIZE] - energy_L[i]);
      
      if ( result >= key ) {
         key   = result;
         indexBest = i+1 ;
      } 
   }   
   
   return( indexBest );
   
} 


/*************************************************************************
*
*       normEnergy :
*       ---------
*               Threshold the lowest energies of the window found in Energy()
*               and we compute the sum of the energies of this window
*               and split it into mantissa * 2^exponent
*
*       Input data:     - Derotated samples : samples[2][MAX_NUM_DATA]
*                       - Channel impulse response : cir[2][CIR_LENGTH
*                       - Energy of the impulse response : energy_L[CIR_LENGTH]
*                       - Position of the best .. : indexBest
*
*       Output data:    - Thresholded Cir : cir[2][CIR_LENGTH]
*                       - First non zero Cir (or begin of the burst) : toa
*                       - average power in dB: power                                           
*                       - exponent
*
*       Returned data   : mantInv = 1/Mantissa, (1 < mantInv <= 2)          
*
*
* "toa" is returned for measurement reporting only.
*  If "toa" is not equal to indexBest, for many bursts,
*  then this indicates that the mobile should adjust it's 
*  timing advance.
*
***************************************************************************/
UWord2  normEnergy(t_ULComplexSamp   samples[BURST_SIZE_UL_MAX],
                   Word    cir[2][MAX_CIR_LENGTH],
                   LWord2  energy_L[MAX_CIR_LENGTH], 
                   Char    indexBest,
                   Word    *toa,
                   Word14  *power, 
                   Short   *exponent)
{
   
   
   /* energyAccum & result is used to calculate the power of the burst */
   UVLWord  energyAccum, result;  
   /* energyThresh is the threshold used to clear a cir position */
   UVLWord  energyThresh;
   
   UInt numSignBits;
   
   /*   
   **   energySum is the sum of the energy in the largest energy window of
   **     the CIR.  energySum is defined as:
   **
   **    sum over largest CIR window( real(CIR)^2 + imag(CIR)^2 ).
   **   This is then broken into mantissa * 2^exponent
   */
   UVLWord   energySum;
   Word      mantissa; 
   UWord2    mantInv;  /* 14 fractional bits to represent 2.0 exactly */
   
   Word      tempCir[2][CIR_SIZE];
   Bool      firstNonZero;
   Bool      allZero;
   Char      cirStartIndex;
   Char      indexTemp;  /* Temporary store of index */  

   ULong     one;
   
   LOOP_TYPE      i;
   
   /* Get the position of the training sequence */
   cirStartIndex = g_BConsts_pt->cirStartIndex + indexBest;
   
   /* 
   *  Compute the average energy of the training sequence 
   */
   energyAccum = 0;
   
   for ( i = 0; i < g_BConsts_pt->cirNumTseqBits; ++i) 
   {
      
      result  = SMPY( samples[cirStartIndex + i].I , samples[cirStartIndex + i].I ); 
      energyAccum    = _lsadd( result, energyAccum );
      result =  SMPY( samples[cirStartIndex + i].Q , samples[cirStartIndex + i].Q );
      energyAccum   = _lsadd( result, energyAccum );
   }
   
   /* 
   * Normalize energyAccum by cirNumTseqBits
   *  energyAccum,F32 * cirNumTseqBitsInv, F16 = F48 -> >> 16 = F32
   * Ideally we do this:
   *  energyAccum = 
   *  ((energyAccum+(1<<(WORD_SIZE-1)))>>WORD_SIZE) * g_BConsts_pt->cirNumTseqBitsInv;
   *
   * However, to maintain accuracy for low level signals (the first shift might zero
   *   energyAccum out) we use NORM to get energyAccum aligned to the 16 lsb's.
   * Also, we must avoid shifts by a signed value because these are not supported
   * under ANSI C.
   * Implemented: (assume WORD_SIZE =16, ACCUM_GUARD_BITS = 8, numSignBits = N)
   *  
   *  ((((((energyAccum << N) >> 24) * 1/cirNumTseqBits ) << 8)>>N
   *  
   * This takes about 15 cycles
   */
   numSignBits = _lnorm( energyAccum );
   /* 
   *  Shift to the 40 MSBs to normalize
   * This will avoid an signed shift in the next step  
   *  Then, Shift to the 16 LSB's
   */
   energyAccum = energyAccum << numSignBits,
   energyAccum = energyAccum >> (WORD_SIZE+ACCUM_GUARD_BITS);
   /* Multiply by 1/cirNumTseqBits */
   energyAccum = (Word) energyAccum * g_BConsts_pt->cirNumTseqBitsInv; 
   /* Shift Back, */
   /* shift by (WORD_SIZE+ACCUM_GUARD_BITS)-numSignBits -WORD_SIZE */
   /* = << ACCUM_GUARD_BITS and >> numSignBits */
   energyAccum = energyAccum << ACCUM_GUARD_BITS;
   energyAccum = energyAccum >> numSignBits; 
     
   /* 
   * TI asm DIVMODU32() can run in 42 cycles. 
   * This divide was clocked at 463 cycles.
   */
   /*energyAccum = energyAccum / g_BConsts_pt->cirNumTseqBits;*/
    
   *power = mag2DB( energyAccum );  
   
   /* Get the Energy Threshold */
   energyThresh = (energyAccum * ENERGY_THRESHOLD_PERCENT ) >> E_THRESH_FORMAT;
   
   
   /* Save cir values and indexBest */
   indexTemp = indexBest;
   /* CIR_SIZE + toa can never be greater than CIR_NUM_TSEQ_BITS */
   for ( i = indexTemp; i < CIR_BEST_LENGTH + indexTemp; ++i) {
      
      tempCir[0][i-indexTemp] = cir[0][i];
      tempCir[1][i-indexTemp] = cir[1][i];
   }
   
   /* 
   ** Threshold the very small values of the CIR.
   ** If they are all small then restore the original CIR. 
   */
   firstNonZero = FALSE;  /* Indicates if the first non-zero cir was found */
   energySum = 0;
   allZero = TRUE; /* Will be set to FALSE is any value is greater than the threshold */
   
   for ( i = indexTemp; i < CIR_BEST_LENGTH + indexTemp; ++i ) 
   {
      if ( energy_L[i] <= energyThresh ) 
      {  /* Zero out the CIR at this position */
      //   cir[0][i] = 0;
      //   cir[1][i] = 0;
      } else 
      {   
         /* Calculate the energy */
         energySum = energySum + energy_L[i];
         
         if ( firstNonZero == FALSE )
         {
            *toa = i;
         }
         firstNonZero = TRUE;  /* first non-zero cir found */
         allZero = FALSE;  /* Not all of the CIR values are zero */
      }
   }
   
   /* If all the cir values are zero then restore the old cir */
   if ( allZero == TRUE ) 
   {
      for ( i = indexTemp; i < CIR_BEST_LENGTH + indexTemp; ++i) {
         
         cir[0][i] = tempCir[0][i-indexTemp];
         cir[1][i] = tempCir[1][i-indexTemp];
         
         /* Calculate the energy */
         energySum = energySum + energy_L[i];
      }
      
      /* Restore the beginning of burst indicator */
      *toa = indexTemp;
   }
   
   /* 
   * Set "toa" to be relative to a ideal toa of zero.
   *   Do this because toa is used in the timing advance 
   *   algorithm uses toa 
   */
   *toa += g_BConsts_pt->toaMin;
   
   /*                                                                                                                                        
   **    Break "energySum" into its components:
   **
   **        energySum = Mantissa * 2^Exponent         
   **                                  
   **   This is done to maximize the fixed point precision in
   **     other routines.                   
   **                                                
   **   The exponent "shifts" the mantissa until the first "one" is just to the 
   **    right of the decimal place. Note the energySum will always be
   **    positive. The picture below shows the result as binary:
   **
   **    energySum = 0.1xxxxxxx * 2^exponent
   ** 
   **  Therefore the mantissa will always be between 0.5 (0.100000) 
   **   and 1-precision (0.111111).  
   **  Also, becuause of this fact 1/mantissa will always be between
   **  2 (when mantissa = 0.5) and 1 (when mantissa = 1-precision).                       
   */
   
   /* Divide energySum into mantissa *2^exponent */
   mantissa = expX( energySum, exponent); 
   
   /* 
   * Compute 1.0/mantissa
   *  "one" can not be represented in fixed point 
   * so approximate it by "1 - word precision" 
   */
   one = (Long) 1<<(2*WORD_SIZE-3); /* 1 in F32.29, signed  */
   
   mantInv =  one / mantissa; /* F32.29 / F16.15 = UF16.14 */
   
   return( mantInv );
}


/*************************************************************************
*       normCir :
*       --------
*               We normalize the value of Cir ( Cir / Mantissa ).
*
*
*       Input data      . Channel impulse response : cir[2][CIR_LENGTH]
*                       . 1/Mantissa : mantInv (1 <mantInt <= 2)
*                       . Position of the best CIR window : *indexBest
*
*       Output data     . Normalized channel impulse response : nCir[2][CIR_SIZE]
*
***************************************************************************/

void    normCir(  Word2     cir[2][MAX_CIR_LENGTH], 
                  UWord2    mantInv, 
                  Char      indexBest_fix, 
                  Word2     cirNorm[2][MAX_CIR_LENGTH] )
{
   
   LOOP_TYPE  i;
   unsigned char  indexBest;

   indexBest = (unsigned char) indexBest_fix;
   
   for ( i = indexBest; i < (CIR_BEST_LENGTH + indexBest ); i++) 
   {
      /*
      * -1 <= cir < 1, 1 < mant_Inv < 2 
      *  So -2 < cirNorm < 2 
      *  cir, SF16,15 and mantInv is UF16,14
      *  SMPY will <<1 to get:   
      *   (SF16,15 * UF16,14)<<1 = SF32,29 << 1 = SF33,30 
      *  and shift that by 16 = SF17,14 
      *  This will not overflow SF16,14 (Word2, two integer bits)
      *  because max(cir) is -1.0 and max(mantInv) is 2.0
      *  -1.0 * 2.0 = -2.0 = 10.0000b
      */
      cirNorm[0][i - indexBest] = SMPY( (SInt) cir[0][i] , mantInv)>>(WORD_SIZE);
      cirNorm[1][i - indexBest] = SMPY( (SInt) cir[1][i] , mantInv)>>(WORD_SIZE);
   }
   
}


/*************************************************************************
**
**       calcRho :
**       ----
**       Compute the autocorrelation of the Cir with cirNorm.
**       calcRho will be used for the VITERBI algorithm as the real channel response.
**
**       Input data      - Channel impulse response : cir[2][CIR_LENGTH]
**                       - Normalized channel impulse response : cirNorm[2][CIR_SIZE]
**                       - Position of the best CIR window : indexBest
**                       - Exponent of the normalized energy : exponent
**
**       Ouput data      - Autocorrelation of the Cir : rho[DELAY_MAX]
**
**  This illistrates the NB case where index = 0 and CIR_SIZE = 5:
**    When i = 1, j = 1:4, j-i = 0:3
**         i = 2, j = 2:4, j-i = 0:2
**         i = 3, j = 3:4, j-i = 0:1
**         i = 4, j = 4,   j-i = 0
**  In this manner the autocorrelation is commputed on the samples except
**  the i = 0 delay which is normalized by the mantissa and exponent.
**
**  NOTE: as of 9/98 rho is not in A|RT.  It will be ported when viterbi
**  is ported to A|RT.
***************************************************************************/
void    calcRho(  Word cir[2][MAX_CIR_LENGTH], 
                Word2 cirNorm[2][MAX_CIR_LENGTH], 
                Char indexBest, 
                Short exponent, 
                Word3 rho[DELAY_MAX] )
{
   
   LWord   result;
   LWord   temp; 
   
   LOOP_TYPE  i, j;
   
   /* 
   ** Correlation of Cir and cirNorm
   **
   **  Note that the maximum of the CIR's aoutcorrelation is:
   **   max( autocorr(cir) ) = energySum = mantissa * 2^exponent 
   **
   **  and that the maximum of the CIR and cirNorms autcorrelation is:
   **   max( corr(cir,cirNorm) ) = energySum/mantissa = 2^exponent 
   **
   **  Because cirNorm = Cir/mantissa
   **
   **  Typically the exponent will be negative, so when Rho is shifted right
   **  by the exponent, it is actually reducing the number of sign bits and
   **  maxmizing the fixed point precision.
   */
   
   for ( i = 1 + indexBest; i < CIR_SIZE + indexBest; i++) {
      result = 0;
      
      for ( j = i; j < CIR_SIZE + indexBest; j++) {
         
         temp =  cir[0][j] * cirNorm[0][j-i] ;
         result = temp + result;
         
         temp =  cir[1][j] * cirNorm[1][j-i] ;
         result = temp + result;
      }
      
      /* Normalize rho to maximize fixed point precision */    
      rho[i-indexBest-1] = SSHIFTR( result, exponent ) >> WORD_SIZE;  
   }
}


/*************************************************************************
**
**       matchFilter :
**       -----------
**       Matched filter the input samples with the normalized CIR 
**
**       Input data     - Input samples : data[2][]
**                      - channel impulse response : cirNorm[2][CIR_SIZE]
**                      - Position of the best CIR window : indexBest
**                      - Exponent of the normalized energy : exponent
**
**       Output data    - Matched filtered samples : Output[nb_samp]
**
** 
** Correlation of Cir and cirNorm
**
**  Note that the maximum of the CIR's aoutcorrelation is:
**   max( autocorr(cir) ) = energySum = mantissa * 2^exponent 
**
**  and that the maximum of the CIR and cirNorms autcorrelation is:
**   max( corr(cir,cirNorm) ) = energySum/mantissa = 2^exponent 
**
**  Because cirNorm = Cir/mantissa
**
**  Typically the exponent will be negative, so when Rho is shifted
**  by the exponent, it is actually reducing the number of sign bits and
**  maxmizing the fixed point precision.
**
***************************************************************************/

void  matchFilter(t_ULComplexSamp   samples[BURST_SIZE_UL_MAX], 
                  Word2  cirNorm[2][MAX_CIR_LENGTH],
                  Char   indexBest_fxp,
                  Short  exponent,
                  Word3   output[])
{
   /* 
   *  "temp" and "result" need to be VLWord only if the input data
   *  has an exponet of 0.  This means the data will 
   *   range from -1 to 1-precision
   */
   VLWord  temp;
   VLWord  result;
   LOOP_TYPE    i, j;
   int    start;   /* DEBUG !! */
   Word3  out_fxp; /* DEBUG !! */
   
   /* 
   * Determine the location of bit 0 in the burst. 
   *   This is first sample after the tail (header) bits plus
   *   any data shift due to MS moving or mulitpath effects.
   */
   start = ((int) indexBest_fxp) + g_BConsts_pt->numTailBits + g_BConsts_pt->toaMin;  /* DEBUG !! */
   
   
   for ( j = 0; j < g_BConsts_pt->numBits; j++) {
      result = 0;
      
      for ( i = 0; i < CIR_SIZE; i++) {
         result  += samples[start + j + i].I * cirNorm[0][i];         
         result  += samples[start + j + i].Q * cirNorm[1][i];
      }
      
      /* Normalize result to maximize fixed point precision */
      result = SSHIFTR( result, exponent );      
      out_fxp = SAT_LONG( result ) >> WORD_SIZE;
      output[j] = out_fxp;
   }
}


/*************************************************************************
**   equalize()
**   ----------
**
**     This function will equalize the burst.
**
**
**      Input data   - Demodulated burst : demOut.softDec[BURST_MATCH]
**                   - Autocorrelation of the Cir : Rho[delay_l]
**
**      Output data  - Corrected burst : demOut.softDec[BURST_MATCH]
**
***************************************************************************/
void equalize (Word matcho[BURST_MATCH],
               Word rho[DELAY_MAX],
               const t_TseqWord tseq[])
{
   LOOP_TYPE i;
   
   /* Just flip sign  */
   
   for( i = 0; i < g_BConsts_pt->numBits; i++ )
   {
      matcho[i] = -1 * matcho[i];
   }
   
}

/***************************************************************************
**   weight()
**   --------
**
**      Weight the soft-decisions to enhance
**      good quality bursts and reduce the impact of poor
**      quality bursts on the decoder.
**      Also, split out the hu/hl bits to the end of the burst.
**
**      Input data:     
**                   -Soft Decisions : demOut.softDec[BURST_MATCH]
**                   - Address of the Training sequence
**
**      Output data:   
**                   - Weighted burst : demOut.softDec[BURST_MATCH]
**                   - Signal to Noise Ratio : snr
**
****************************************************************************/
void  weight( Word softDec[MAX_NUM_BITS_PER_BURST], 
              const t_TseqWord tseq[], 
              Word *snr)
{
   LOOP_TYPE i;
   
   /* Remove the tseq */
   if( g_burstType == TCHF )
   {
      for( i = 0; i < NB_NUM_ENCRYPT_BITS_PER_SIDE; i++)
      {
         softDec[i+NB_NUM_ENCRYPT_BITS_PER_SIDE] = \
            softDec[i+NB_NUM_ENCRYPT_BITS_PER_SIDE+NB_TRAIN_SIZE];
      }
   } else /* AB */
   {
      for( i = 0; i < g_BConsts_pt->numDataBits; i++)
      {
         softDec[i] = softDec[i+AB_TRAIN_SIZE];
      }
   }
   
   /* Just make them perfect SD for now */
   for( i = 0; i < g_BConsts_pt->numDataBits; i++ )
   {
      if( softDec[i] <  0 )
      {
         softDec[i] =  8;
      } else
      {
         softDec[i] = -8;
      }
   }
   
   *snr = 40;  /* Excellent snr */
}


/***************************************************************************
**
**  quantize()
**  ----------
**
**   Quantization of the soft-decisions to save memory and to 
**     be compatable with the decoder input.
**
**   If |softddec| > 4 then quantized_soft_decision = sign(softdec) * 8
**   if |softdec| <= 4 then quantized_soft_decision goes linearly from -8 to +8.
**
**
**   Input data:    - Soft Decsions
**
**   Output data     - Quantized Soft Decsions
**
***************************************************************************/
void  quantize( Word demodSamps[MAX_NUM_BITS_PER_BURST], Char softDec[] )
{
   LOOP_TYPE i;

   for ( i = 0; i < g_BConsts_pt->numDataBits; i++) 
   {
      softDec[i] = (Char) demodSamps[i];
   }   
}



/***************************************************************************
**
**  exp()
**  ----
**
**   This will break a number into:
**      x = mantissa * 2^exponent
**
**   This uses the C6x intrinsic NORM()
**
**
**   Input data:    - x, input data to break apart
**
**   Output data     - exponent, exponent of the "x"
**
**   Return data     - mantissa, mantissa of the "x"
**
***************************************************************************/
Word   expX( UVLWord data, Word *exponent )
{
   Word mantissa;
   UChar  numSignBits;

   /*mantissa = frexp( (double) x / (double)(1<<2*WORD_SIZE-1), &expInt);
     *exponent = (Short) expInt;
     return( (Word) ( mantissa * (1<<(WORD_SIZE-1)) ) );
   */

   numSignBits = LNORM( data ) ;
   *exponent =  -(numSignBits - ACCUM_GUARD_BITS);
   
   /* Normalize the data to 32 lsb's */
   data = data << numSignBits;
   mantissa = data >> (WORD_SIZE+ACCUM_GUARD_BITS);
   return(  mantissa );
}

/*************************************************************************** 
*     mag2DB()
*     -------
*
*   Input Data:   mag, magnitude of a value F48 with 40 actual bits
*
*   Return Data:  power in decibels of format F16.2
*
*  Convert an unsigned long word (40bits) in the format
*    F32  to decibels.
*  The magin F48 because is was calculated by 
*    F16 * F16 = F32.
*  Therefore "31" needs to subtracted from pow2(mag) 
*   in order to align the decimal point.
*
*  In floating point this would be :   
*  If wordsize is 16 then:
*  *power = 10 * log10( (mag >> 16)/(double)((1<<2*16)-1) );
*
* Note that
*   10*log10(x) = (10*0.301)*log2(x)
*  And 3*log2() = 3*(integer + fractional) = 3*integer + 3*fractional
*
* The fractional part is derived from a lookup table with
* the 2 next significant bits after the most significant bit.
*
* The entries are calculated by log2(1.0),log2(1.25),log2(1.5),log2(1.75)
*
* logTable[4] = { 3*-1, 3*-0.6781, 3*-0.415, 3*-0.1926 }; floating 
* Multiply float table by 4 add 1 (to round) and round to nearest integer 
*
***************************************************************************/
const Short logTable[4] = {   2,   5,   8,   11 }; /* F16.14 */

Word14 mag2DB( UVLWord mag )
{
   UShort numSignBits;
   Word14 power;
   Short   intPart;
   Short   fractPart;

   numSignBits = LNORM( mag );

   /*
   * Get the 3 msb's plus the sign for the fractional part.
   *  Shift up to 40 bits to normalize then down.
   */
   mag = mag << numSignBits;
   fractPart = mag >> (2*WORD_SIZE + ACCUM_GUARD_BITS - 4);
 
   /* Get the number of significant integer bits.
   *  = log2(mag) = number of bits to the right of the msbit (not the sign)
   */
   intPart =  (2*WORD_SIZE + ACCUM_GUARD_BITS - 2) - numSignBits;

   power = (intPart-31)*4*3; /* Integer part of power */
   if( fractPart > 3 ) /* Check for zero power */
   {
      fractPart = logTable[fractPart-4];
      power += fractPart;
   }
   
   return( power );
}
