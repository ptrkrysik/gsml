/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/************************************************************************
* File: demodc.c
*
* Description:
*   This files contains only subroutines for the equalization
*   Note: Old revision log is at the end of this file.
*
************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <c6x.h>
#include "dsp/dsptypes.h"
#include "gsmtypes.h"
#include "bbdata.h"
#include "bbproto.h"
#include "dem_globs.h"
#include "dem_aprotos.h"
#include "c6xsupport.h"
/*
* This macro will return the GSM defined value of
* the tseq from what was stored for the demod
*
* demod  GSM
*   1 <-> 1
*  -1 <-> 0
*/
#define SIGNU(_xx) ( (((_xx) + 1)>>1) )

#define INITVAL 0xfffff800 
#define LOOP_TYPE int

#define DC_SMOOTH_MULT      15  /* multiplier for DC offset smoothing */
#define DC_SMOOTH_SHIFT      4  /* downshift for DC offset smoothing  */

      
/*************************************************************************
**   equalize()
**   ----------
**
**     This function will equalize the burst.
**
** 
** Parameter I/O: 
**
**      Input data   - Demodulated burst : demOut.softDec[BURST_MATCH]
**                   - Autocorrelation of the Cir : Rho[delay_l]
**
**      Output data  - Corrected burst : demOut.softDec[BURST_MATCH]
**   
** Returns: none
**
***************************************************************************/
/*
* Force 32-bit alignment of these arrays so they can be accessed as Int's.
* To use this pragma the varaibles must be global.
*/
#pragma DATA_ALIGN(oldMets, 4);
#pragma DATA_ALIGN(newMets, 4);

static Short oldMets[NUM_STATES];
static Short newMets[NUM_STATES];


void equalize (Word matcho[BURST_MATCH],
               Word rho[DELAY_MAX],
               const t_TseqWord tseq[],
               t_chanSel chanSel )
{
   Word *newMetsPtr, *oldMetsPtr;
   Word probs[NUM_STATES];

   Short stateTranTbl[MAX_NUM_ENCRYPT_BITS_PER_SIDE+DELAY_MAX+4][NUM_STATES];  /* state transition table */
   SInt state;
   SInt i,j;
   SInt dataStartIndex;
   SInt sdStartIndex;
   SInt numBitsOut;
   Short rachTseqBits[AB_TRAIN_SIZE+DELAY_MAX];


   oldMetsPtr = &oldMets[0];
   newMetsPtr = &newMets[0];

   
   /*
   * Create the probablities for each state.
   */
   for (i = 0; i < NUM_STATES/2; i++)
   {
      probs[i] = 0;
      for( j = 0; j < DELAY_MAX; j ++ )
      {
         if( i & (1<<j) )
         {
            probs[i] += rho[DELAY_MAX - j - 1];

         } else
         {
            probs[i] -= rho[DELAY_MAX - j - 1];
         }
      }
   }
   /*
   * The other states are just the negative mirror image 
   * of states 0 -> 7
   */
   for (i = NUM_STATES/2; i < NUM_STATES; i++)
   {
      probs[i] = -probs[NUM_STATES - i - 1];
   }

   /**
   **  Do the part of the burst before the tseq first.
   **   This is only for normal and syncro NOT rach.
   **   new 10/5/99...save RACH tseq bits for equalization later
   **/

   if( chanSel == RACH )
   {
    for( i = 0; i < (AB_TRAIN_SIZE+DELAY_MAX); i++ )
    {
      rachTseqBits[i] = matcho[i];
    }
   }
   else
   {
      
      /* 
   
   * Old metrics initialized with initial values 
      * The "unlikely" states get initialized with 2*INITVAL
      * and the "most likely" states get initialized with INITVAL.
      */

      for(i=2; i<16; i++)
      {
         oldMets[i] =  2 * INITVAL;
      } 
      
      /*
      * Computing the first part of the burst
      *   means that the tail bits are in front.
      * The tail bits are 3 0's the other bit can be
      *   a zero or one with equal probabilty.
      * So states 0000 and 0001 are the most likely states.
      *
      * The ordering is from left to right, newest to oldest states
      */     
      oldMets[0] = INITVAL;
      oldMets[1] = INITVAL;
      
      /*
      * Call the viterbi algorithm on this section of the burst.
      *
      * Also, get the number of bits that the viterbi needs to
      * work on, this is the number of encrypted bits plus the 
      * "rho" size.
      *
      *  dataStartIndex: the beginning index of the second part of the burst
      *  sdStartIndex: the Index for the soft decisions when the 
      *     tseq is removed.
      *  numBitsOut:  the number of bits that the viterbi needs to
      *     work on, this is the number of encrypted bits plus the 
      *     "rho" size.
      */
      dataStartIndex = 0; /* at the beginning of the burst */
      sdStartIndex = 0; 
      numBitsOut = g_BConsts_pt->numEncryptBitsPerSide + DELAY_MAX;
      
      viteq( numBitsOut, 
             (short*)newMetsPtr, 
             (short*)oldMetsPtr, 
             &matcho[dataStartIndex], 
             &probs[0], 
             &stateTranTbl[0][0] );
      
      
      /*
      * Perform the backtracking and decisions. numBitsOut-4 bits will be written
      * to u[0..numBitsOut-5]. u[numBitsOut-4..numBitsOut] is not written since
      * this is tseq bits and need not be written. It is assummed that these
      * locations in the output buffer have already been initialized.
      *
      * state is the first 4 tseq bits.
      */
      state = SIGNU( tseq[0] )*1  /* xxxX */   \
            + SIGNU( tseq[1] )*2  /* xxXx */   \
            + SIGNU( tseq[2] )*4  /* xXxx */   \
            + SIGNU( tseq[3] )*8; /* Xxxx */


      for(i= numBitsOut - 1, j=state; i >= DELAY_MAX; i--)
      {      
         SInt x;
         
         /*
         * stateTranTbl[i][j] is actually the LSB of the state number. The observation here 
         * is that this LSB, delayed by 4 units, will appear as the output. 
         */
         matcho[i-DELAY_MAX+sdStartIndex] = stateTranTbl[i][j];
         
         /*
         * Decide which of the two states to backtrack to based on the state LSB 
         * (stateTranTbl[i][j])
         *  j = ( 2*j + stateTranTbl[i][j] ) modulo 16
         */
         x = (stateTranTbl[i][j] > 0);
         j= ((j<<1) + x) & 0x0F;
      } /* end backtracking */

   } /* End normal/sync burst first part processing */
   
     
  /**
   **  Do the second part of the burst.
   **  For AB this is the only part.
   **/

   /*
   * Append the ideal tail bits to the end of the burst.
   * Assume the 4th bit is also zero.
   */
   for( i = 0; i < DELAY_MAX; i++ )
   {
      matcho[g_BConsts_pt->numBits+i] = -(1<<8);
   }

   /* 
   * Old metrics initialized with initial values 
   */
   for(i=0; i<16; i++)
   {
      oldMets[i] = 2 * INITVAL;
   }

   /*
   * Computing the second part of the burst
   * means that the training sequence is in front.
   * Use the last 4 bits of the tseq as the initial
   * state.
   * The ordering is from left to right, newest to oldest states
   */
   state = SIGNU(tseq[g_BConsts_pt->numTseqBits -4] )*1  /* xxxX */   \
         + SIGNU(tseq[g_BConsts_pt->numTseqBits -3] )*2  /* xxXx */   \
         + SIGNU(tseq[g_BConsts_pt->numTseqBits -2] )*4  /* xXxx */   \
         + SIGNU(tseq[g_BConsts_pt->numTseqBits -1] )*8; /* Xxxx */

   oldMets[state] = INITVAL;

   /*
   * Call the viterbi algorithm on this section of the burst.
   *
   * Also, get the number of bits that the viterbi needs to
   * work on, this is the number of encrypted bits plus the 
   * "rho" size.
   *
   *  dataStartIndex: the beginning index of the second part of the burst
   *  sdStartIndex: the Index for the soft decisions when the 
   *     tseq is removed.
   *  numBitsOut:  the number of bits that the viterbi needs to
   *     work on, this is the number of encrypted bits plus the 
   *     "rho" size.
   */
   if (chanSel == RACH )
   {
      dataStartIndex = g_BConsts_pt->numTseqBits;
      sdStartIndex = 0; /* tseq is at the beginning of the burst */
   } else /* Normal Burst or Syncro Burst */
   {
      dataStartIndex = g_BConsts_pt->numEncryptBitsPerSide + g_BConsts_pt->numTseqBits;
      sdStartIndex = g_BConsts_pt->numEncryptBitsPerSide; /* tseq in middle */
   }
   numBitsOut = g_BConsts_pt->numEncryptBitsPerSide + DELAY_MAX;

   viteq( numBitsOut, 
            newMetsPtr, 
            oldMetsPtr, 
            &matcho[dataStartIndex], 
            &probs[0], 
            &stateTranTbl[0][0]);


   /*
   * Perform the backtracking and decisions. numBitsOut-4 bits will be written
   * to u[0..numBitsOut-5]. u[numBitsOut-4..numBitsOut] is not written since
   * this is "0" tail bits and need not be written. It is assummed that these
   * locations in the output buffer have already been initialized.
   *
   * The state is "0" because the tail bits are the final state.
   */
   state = 0;
   for(i= numBitsOut - 1, j=state; i >= DELAY_MAX; i--)
   {      
      SInt x;

      /*
      * stateTranTbl[i][j] is actually the LSB of the state number. The observation here 
      * is that this LSB, delayed by 4 units, will appear as the output. 
      */
      matcho[i-DELAY_MAX+sdStartIndex] = stateTranTbl[i][j];

      /*
      * Decide which of the two states to backtrack to based on the state LSB 
      * (stateTranTbl[i][j])
      *  j = ( 2*j + stateTranTbl[i][j] ) modulo 16
      */
      x = (stateTranTbl[i][j] > 0);
      j= ((j<<1) + x) & 0x0F;
   } /* end backtracking */            

/*  new 10/5/99..equalize the RACH tseq bits  */

   if( chanSel == RACH )
   {
      for(i=2; i<16; i++)
      {
         oldMets[i] =  2 * INITVAL;
      } 
      
      /*
      * Computing the first part of the burst
      */     
      oldMets[0] = INITVAL;
      oldMets[1] = INITVAL;
      
      /*
      * Call the viterbi algorithm on this section of the burst.
      */
      dataStartIndex = 0; /* at the beginning of the burst */
      sdStartIndex = AB_NUM_ENCRYPT_BITS_PER_BURST; 
      numBitsOut = AB_TRAIN_SIZE + DELAY_MAX;
      
      viteq( numBitsOut, 
         newMetsPtr, 
         oldMetsPtr, 
         &rachTseqBits[dataStartIndex], 
         &probs[0], 
         &stateTranTbl[0][0] );
      
      /*
      * Perform the backtracking and decisions. numBitsOut-4 bits will be written
      *
      * state is the first 4 matcho equalized data bits.
      */
      state = ( matcho[0] > 0)*1  /* xxxX */   \
            + ( matcho[1] > 0)*2  /* xxXx */   \
            + ( matcho[2] > 0)*4  /* xXxx */   \
            + ( matcho[3] > 0)*8; /* Xxxx */

      for(i= numBitsOut - 1, j=state; i >= DELAY_MAX; i--)
      {      
         SInt x;
         
         /*
         * stateTranTbl[i][j] is actually the LSB of the state number. The observation here 
         * is that this LSB, delayed by 4 units, will appear as the output.
         * negate sign to reverse effect of equalizer 
         */
         matcho[i-DELAY_MAX+sdStartIndex] = stateTranTbl[i][j];
         
         /*
         * Decide which of the two states to backtrack to based on the state LSB 
         * (stateTranTbl[i][j])
         *  j = ( 2*j + stateTranTbl[i][j] ) modulo 16
         */
         x = (stateTranTbl[i][j] > 0);
         j= ((j<<1) + x) & 0x0F;
	 }
   }
}

/***************************************************************************
**   deEmph()
**   --------
**
**      deemphasize bursts with poor quality.  
**      This will reduce the impact of poor
**      quality bursts on the decoder.
**
**      Input data:     
**                   - Soft Decisions of encrypted bits : softDec[NUM_ENCRYPT_BITS]
**                   - Address of the Training sequence
**                   - Burst Quality Metric (snr)
**
**      Output data:   
**                   - deemphasized burst : softDec[NUM_ENCRYPT_BITS]
**
****************************************************************************/
void  deEmph( t_chanSel  chanSel,
              Word softDec[MAX_NUM_BITS_PER_BURST], 
              const t_TseqWord tseq[], 
              Word snr)
{
   int i;
   Word8  deEmphVal;

   /*
   * Get the deemphasis value from the snr
   * The snr goes from 26 (good burst) to 0 (very bad burst)
   * Typical values for bad bursts are 24->22.
   */                                 
   /*
   * The inputs are 0 or 1,
   * If snr = 26 (perfect) then the mapping should be:
   *   1 -> -8
   *   0 ->  8  
   * And make them smaller if snr is less than 26
   */ 

/* 11/3/99...restore deemphasis of poor quality burst for better decode performance */
 
   if(chanSel == RACH)
    deEmphVal = 0;
   else
   {
    deEmphVal = snr-26;
    deEmphVal = (deEmphVal < -3) ? 3 : -deEmphVal;
   }

   for( i = 0; i < g_BConsts_pt->numEncryptBits; i++ )
   { 
/*  hpines change..scale by right shift for quantize to work ok */

       softDec[i] = softDec[i] * (-1) + 8; /* softDec is now -/+ 8 */

       softDec[i] = (softDec[i]) >> 5; /* softDec is now -/+ 8 */

       if(deEmphVal > 0)
        softDec[i] = (softDec[i])>> deEmphVal ; 

   }

}

/***************************************************************************
**
**  quantize()
**  ----------
**
**   Quantization of the soft-decisions to save memory and to 
**     be compatable with the decoder input.
**
**
**   Input data:    - Soft Decsions
**
**   Output data     - Quantized Soft Decsions
**
***************************************************************************/
void  quantize( Word demodSamps[MAX_NUM_BITS_PER_BURST], Char softDec[] )
{
   int i;

   for ( i = 0; i < g_BConsts_pt->numEncryptBits; i++) 
   {
      softDec[i] = (Char) demodSamps[i];
   }   
}

/***************************************************************************
**
**   computeSNR()
**  ------------
**
**   This will compute the "SNR" of the input data.
**   It compares the training sequence section
**   to the actual training sequence samples. 
**   The result is a number from 0 to 26 where 26 is good 0 is very bad.
**
**   The same amount of data will be computed for all burst types.
**   The NB needs the snr for soft decsion deemphasis for poor bursts.
**   SB & AB need it mainly for diagnostics.
**
**
**   Input data:    - data, input data compute the snr on
**                      This should be a pointer to the first tseq sample
**                  - probs, probablities for the viterbi
**
**  Returned data:  - value proportional to the snr
**
**
***************************************************************************/
Word   computeSNR( Word data[NB_TRAIN_SIZE], t_chanSel chanSel  )
{

   SInt i;
   Word  snr;


   snr = 0;
   if(chanSel == RACH)
   {
    for (i = 0; i < AB_TRAIN_SIZE; i++) 
    {
      snr += ( (data[i] > 0) == (g_BConsts_pt->tseqBaseAddr[i] >0) );
    }
   }
   else
   {
    for (i = 0; i < NB_TRAIN_SIZE; i++) 
    {
      snr += ( (data[i] > 0) == (g_BConsts_pt->tseqBaseAddr[i] >0) );
    }
   }
   return( snr );
}

/*************************************************************************
*
*       
*       DCOffset :
*       -------------
*       DC Offset tracking. Maintains average DC offset of each time slot,
*       and subchannel, and the DC offset of the carrier's idle frames.
*       The latter is used for initializing the offset at activation. 
*
*
*       Input:
*         samples    complex input samples (not derotated)
*         groupNum   group number
*         rfCh       index of RF channel
*         index      index of DC offset in g_DcOffset array:
*                    a. TCH channel (first NUM_TN_PER_RF array locations)
*                    b. SDCCH subchannel (next NUM_SDCCH_SUBCHANS locations)
*                    c. all-carrier value (last array location)
*
*       Output:
*         g_dCOffset[rfCh][timeSlot]   updated DC offset
*
***************************************************************************/
void DCOffset( t_ULComplexSamp samples[BURST_SIZE_UL_MAX],t_chanSel groupNum, Uint8 rfCh,
               Uint8 index )
{  
   Long        offsetI=0;
   Long        offsetQ=0;
   LOOP_TYPE   i;
   const Word  START = (GSM_NUM_SAMPLES_PER_BURST-OFF_NUM_OFFSET_SAMPS)/2;
      
   /* 
   *  Accumulate OFF_NUM_OFFSET_SAMPS, a power of 2
   */  
   for ( i = START; i < START+OFF_NUM_OFFSET_SAMPS; i++ ) 
   {
      offsetI += samples[i].I;
      offsetQ += samples[i].Q;
   }
   /* 
   *  Round and divide by number of samples using shift 
   */  
   offsetI += 1<<(OFF_SHIFT_VAL-1);
   offsetQ += 1<<(OFF_SHIFT_VAL-1);
   offsetI >>= OFF_SHIFT_VAL;
   offsetQ >>= OFF_SHIFT_VAL;
   /* 
   *  Smooth DC offsets in units of 1/256 quantization level, save long  
   */  

   if (groupNum != GSM_GROUP_7){
   g_DcOffset[rfCh][index].I =
      ((DC_SMOOTH_MULT * g_DcOffset[rfCh][index].I) + offsetI*256) >> DC_SMOOTH_SHIFT;
   g_DcOffset[rfCh][index].Q =
      ((DC_SMOOTH_MULT * g_DcOffset[rfCh][index].Q) + offsetQ*256) >> DC_SMOOTH_SHIFT;
   } else {
   g_Dc_7_Offset[rfCh][index].I = 
       ((DC_SMOOTH_MULT * g_Dc_7_Offset[rfCh][index].I) + offsetI*256) >> DC_SMOOTH_SHIFT;
   g_Dc_7_Offset[rfCh][index].Q =
      ((DC_SMOOTH_MULT * g_Dc_7_Offset[rfCh][index].Q) + offsetQ*256) >> DC_SMOOTH_SHIFT;
   }
}

/*************************************************************************
*
*       removeDCOffset() :
*       ------------------
*       DC Offset compensation
*
*
*       Input:
*         samples    complex input samples (not derotated)
*         groupNum   configuration of the timeslot 
*         rfCh       index of RF channel
*         index      index of DC offset in g_DcOffset array:
*                    a. TCH channel (first NUM_TN_PER_RF array locations)
*                    b. SDCCH subchannel (next NUM_SDCCH_SUBCHANS locations)
*                    c. all-carrier value (last array location)
*
*       Output:
*         samples    offset  complex samples
*
***************************************************************************/
void removeDCOffset( t_ULComplexSamp samples[BURST_SIZE_UL_MAX],  t_chanSel groupNum, Uint8 rfCh,
                     Uint8 index ) 
{
   Word        offsetI, offsetQ;
   LOOP_TYPE   i; 

   /* 
   *  Convert DC offsets from units of 1/256 to 1 quantization level  
   */  

   if (groupNum != GSM_GROUP_7){
      offsetI = g_DcOffset[rfCh][index].I >> 8;
      offsetQ = g_DcOffset[rfCh][index].Q >> 8;
   } else {
      offsetI = g_Dc_7_Offset[rfCh][index].I >> 8;
      offsetQ = g_Dc_7_Offset[rfCh][index].Q >> 8;
   }

   for( i = 0;  i < GSM_NUM_SAMPLES_PER_BURST; i++ ) 
   {         
      samples[i].I = SAT_WORD( samples[i].I - offsetI );
      samples[i].Q = SAT_WORD( samples[i].Q - offsetQ );
   }
}

/*************************************************************************
*
*       derotate() :
*       ------------
*       Derotation only
*
*
*       Input:
*         samples    complex input samples
*
*       Output:
*         samples    derotated complex samples
*
***************************************************************************/
void derotate( t_ULComplexSamp samples[BURST_SIZE_UL_MAX])
{
   LOOP_TYPE   i; 
   Word temp;
   
   /*
   *  Note: The saturation checks below would not seem to be necessary.
   *  However, in actual tests the RBER increases when they're left out.
   */
   for( i = 0;  i < (GSM_NUM_SAMPLES_PER_BURST-3); i += (OFF_I_TYPE) 4 ) 
   {         
      /* Derotate 1st sample */
      temp = samples[i].I;
      samples[i].I = samples[i].Q;
      samples[i].Q = SAT_WORD( -temp ); 

      /* Derotate 2nd sample */
      samples[i+1].I = SAT_WORD( -samples[i+1].I );
      samples[i+1].Q = SAT_WORD( -samples[i+1].Q );
      
      /* Derotate 3rd sample */
      temp = samples[i+2].I;
      samples[i+2].I = SAT_WORD( -samples[i+2].Q );
      samples[i+2].Q = SAT_WORD( temp );
      
      /* Derotate 4th sample (no operation needed) */
   }
}

/****************************** Old revision log ************************************
**                
** Revision 1.10  2000-01-25 18:47:01-08  dkaiser
** Check saturation in derotate function
**
** Revision 1.8  1999-11-07 16:54:38-08  dkaiser
** Changed DC offset smoothing from 1/64 to 1/15 factor to be more responsive in case
** offset changes drastically with automatic gain adjustment. Now slews to 90% of its
** steady state value in .35 second.
**
** Revision 1.7  1999-11-04 14:54:35-08  ltravaglione
** <>
**
** Revision 1.7  1999-11-03 18:18:40-08  hpines
** Restored deemphasis of poor snr-quality bursts for better soft decision decoder
** performance, especially at low signal power levels.
**
** Revision 1.6  1999-10-27 13:37:59-07  dkaiser
** DC offset is now computed for each logical channel as well as all-carrier idle frames
**
** Revision 1.5  1999-10-18 18:50:49-07  hpines
** Enhanced RACH detection test...1.  perform equalization on RACH training sequence bits
** 2.  computeSNR over 41 RACH training sequence bits.
**
** Revision 1.4  1999-10-06 16:58:00-07  dkaiser
** added rfCh input param to OffsetDerot function
**
** Revision 1.3  1999-09-23 11:31:43-07  dkaiser
** added DC offset function
** computeSNR function distinguishes access bursts from normal bursts
**
** Revision 1.2  1999-07-14 16:27:36-07  hpines
** Support routines for equlizer demod version now upgraded to Integration status
**
** Revision 1.1  1999-05-04 14:54:28-07  whuang
** Initial revision
**
** Revision 1.7  1998/12/17 16:59:28  bhehn
** Added support for deEmphasis of low snr signals. */
/*
** Revision 1.6  1998/10/27 21:39:52  bhehn
** AB & NB eqaulizer verifed.
** AB ~= 125us, NB ~= 155us.
**
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
***/




