/***********************************************************************
**
**  GSM Demod w/ TI C6201  implementation
**
** Description:
**
**  This is the main calling routine.  This will process the I/O files
**  and call the low level functions.
**
** $Author: hpines $
** $Date: 1999/07/14 21:38:21 $
** $Source: D:\\stageCell\\Dsp\\bbdataproc\\demod-asm\\RCS\\demtop.c $
** $Revision: 1.1 $
** $Log: demtop.c $
** Revision 1.1  1999/07/14 21:38:21  hpines
** Initial revision
**
** Revision 1.1  1999-02-24 15:16:53-08  whuang
** Initial revision
**
** 
**    Rev 1.0   Oct 27 1998 16:19:34   Administrator
** Checked in from initial workfile by PVCS Version Manager Project Assistant.
** 
**    Rev 1.0   Oct 23 1998 17:56:08   Administrator
** Checked in from initial workfile by PVCS Version Manager Project Assistant.
** Revision 1.6  1998/10/14 18:43:26  bhehn
** AB & NB verified.  Merged with shared header.
** AB ~= 103us, NB ~= 78us
**
** Revision 1.5  1998/10/09 21:20:36  bhehn
** Fast version, Not yet versified.
** AB ~ 125us, NB ~= 110us
**
** Revision 1.4  1998/10/07 15:21:32  bhehn
** One Access Burst verifies.  testfile.fixin is the input testfile.c6x is the output.
**
** Revision 1.3  1998/10/02 17:01:00  bhehn
** Changed calling routine to be more like c6x  implementation.
** BER matches previous version.
**
** Revision 1.2  1998/10/01 16:37:07  bhehn
** Fixed matchfilter output saturation problem
**
** Revision 1.1  1998/10/01 13:57:00  bhehn
** Initial revision
**
** Revision 1.4  1998/09/22 20:00:36  bhehn
** Access Burst support added and verified.
**
** Revision 1.2  1998/09/21 22:04:22  bhehn
** NB demod works, BER close to floating
**
** Revision 1.2  1998/09/18 20:15:05  bhehn
** Outdated (renamed to '\\Fdi\public\rcs\D\rcs\projects\GSM\Art\demod\demodTop.cxx,v').
**
**
************************************************************************/

#include <stdio.h> 
#include <stdlib.h>

#include "dsp\dsptypes.h"
#include "gsmtypes.h"
#include "bbdata.h"
#include "bbproto.h"
#include "c6xsupport.h"

/* 
*  DEMOD_TOP is used to define the globals, 
*   this should be the only file where it is defined
*/
#define  DEMOD_TOP

#define  LOOP_TYPE int

/* Include globals and typedefs */
#include "dem_const.h"
#include "training.h"
#include "dem_globs.h"
#include "dem_aprotos.h"

#if TEST == 1     

/* I/O File global variables */
extern FILE *out_file;

void moveStealingBits( t_chanSel chanSel, Word softDec[] );

#endif

/*
** Log table used by mag2DB in demsubs.sa
**
*/
const Short logTable[4] = {   2,   5,   8,   11 }; /* F16.14 */

/*
** Provide a mechanism for the g_BConsts structure and constants
** #defined in C to be accessed from assembly code.
*/

int c_asm_consts[128];

/*
** This routine must be called once before any routine in
** demsubs.sa is. Changing burst type doesn't alter the field
** offsets in t_burstConsts so that is the only time this routine
** need be called.
**
** The order of objects here must match the list of offset names
** in demsubs.sa. Change one, change the other.
*/

void initAsmConsts()
{
  t_burstConsts t;

  /* offsets to fields within t_burstConsts struct type */

  c_asm_consts[0] = ((char *)&t.tseqBaseAddr)       -   ((char *)&t);
  c_asm_consts[1] = ((char *)&t.cirNumTseqBits)     -   ((char *)&t);
  c_asm_consts[2] = ((char *)&t.cirNumTseqBitsInv)  -   ((char *)&t);
  c_asm_consts[3] = ((char *)&t.impulseTseqOffset)  -   ((char *)&t);
  c_asm_consts[4] = ((char *)&t.cirLength)          -   ((char *)&t);
  c_asm_consts[5] = ((char *)&t.cirStartIndex)      -   ((char *)&t);
  c_asm_consts[6] = ((char *)&t.toaMin)             -   ((char *)&t);
  c_asm_consts[7] = ((char *)&t.numBits)            -   ((char *)&t);
  c_asm_consts[8] = ((char *)&t.numDataBits)        -   ((char *)&t);
  c_asm_consts[9] = ((char *)&t.numTailBits)        -   ((char *)&t);

  /* values of symbols created with #define in C */

  c_asm_consts[10] = OFF_NUM_OFFSET_SAMPS;       
  c_asm_consts[11] = OFF_SHIFT_VAL;
  c_asm_consts[12] = GSM_NUM_SAMPLES_PER_BURST;
  c_asm_consts[13] = MAX_CIR_LENGTH;
  c_asm_consts[14] = CIR_SIZE;
  c_asm_consts[15] = CIR_BEST_LENGTH;
  c_asm_consts[16] = WORD_SIZE;
  c_asm_consts[17] = ACCUM_GUARD_BITS;
  c_asm_consts[18] = ENERGY_THRESHOLD_PERCENT;
  c_asm_consts[19] = E_THRESH_FORMAT;
  c_asm_consts[20] = NB_NUM_ENCRYPT_BITS_PER_SIDE;
  c_asm_consts[21] = NB_TRAIN_SIZE;
  c_asm_consts[22] = AB_TRAIN_SIZE;
  c_asm_consts[23] = TCHF;
  c_asm_consts[24] = RACH;
}

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
void    offsetDerotc( t_ULComplexSamp   samples[BURST_SIZE_UL_MAX], 
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

void demodulate
(
  t_ULComplexSamp   samples[BURST_SIZE_UL_MAX],
  t_chanSel         chanSel,
  UChar             tseqNum,
  t_ULBurstBuf      *demodOut
)
{
   
  t_ULCirSamp cir[MAX_CIR_LENGTH];          /* Impulse response */
  t_ULCirSamp cirTemp[MAX_CIR_LENGTH];      /* Saved Impulse response */
  LWord       energy_L[MAX_CIR_LENGTH];     /* Energy */
  t_ULCirSamp cirNorm[MAX_CIR_LENGTH];      /* Normalized Cir */
  Char        indexBest;           /* Position of the best CIR window */
  Word3       rho[DELAY_MAX];      /* Autocorr of Cir, used by Viterbi */

  /* Demodulated samples before quantizing */
  /*  See if this could be demodIn->samples to save stack space */

  /* 32bit align the demodSamps array! Required by assembly. */
  Int         demodSampInts[(MAX_NUM_BITS_PER_BURST+1)/2];
  Word3       *demodSamps=(Word3 *)demodSampInts;

  UWord2      mantInv;  /* 1/mantissa of energySum = mantissa * 2^exponent */
  Short       exponent; 
   
#if TEST == 1
   int  i;  /* used to index arrays for printing to files */
#endif

  /* Set the burst constants pointer to the proper burst */
  if (chanSel == RACH )
  {
    /* Use the access burst constants */
    g_BConsts_pt =  &g_ABConsts;   
    g_burstType = RACH; /* Needed until equal is implemented */

  } else 
  {
    /* Use the normal burst constants */
    g_BConsts_pt =  &g_NBConsts;
    /* Change the tseq base address */
    g_BConsts_pt->tseqBaseAddr  = &train_nb[tseqNum][0]; 
    g_burstType = TCHF; /* Needed until equal is implemented */
  }

  initAsmConsts();

  /**************************************************************************
  *
  *                     GSM Demod: main program
  *
  **************************************************************************/
  
  /*
  **
  **     Offset computation + Offset compensation + Derotation
  **
  **     Input data     . samples[2][NUM_DATA]
  **
  **     Output data    . samples[2][NUM_DATA]
  **     samples have been overwritten by compensated + derotated samples
  **
  */
  offsetDerotc(  samples, DC_OFFSET_THRESHOLD );

     
  /*
  fprintf( out_file, "* Input samples after Off_Derot*\n"); 
  for ( i = 0; i < CIR_LENGTH ; i++) 
  {
    fprintf( out_file, "%s\n", samples[0][i].dec() );
    fprintf( out_file, "%s\n", samples[1][i].dec() );
  }
  */
  
  /*
  **    Estimate the Channel Impulse Response
  **
  **
  **    Input:     Derotated Samples - demodIn.samples[2][NUM_DATA]
  **               Training Sequence - tseq[TRAINING_SIZE]
  **
  **    Output:    Channel impulse response - cir[CIR_LENGTH]
  **
  */  
  impulse( samples, g_BConsts_pt->tseqBaseAddr,  cir );
  

  
#if TEST_RHO == 1   
  fprintf( out_file, "### Impulse Response ###\n"); 
  for ( i = 0; i < g_BConsts_pt->cirLength; i++)  
  { 
    fprintf( out_file, "%2.10f\n", WORD_2F( cir[i].I )); 
    fprintf( out_file, "%2.10f\n", WORD_2F( cir[i].Q )); 
  }
#endif
   
  /*
  **    Compute the energy of the impulse response
  **     and try to find the best energy window of length CIR_SIZE
  **
  **
  **     Input data:
  **                   - The channel impulse response : cir[CIR_LENGTH]
  **
  **     Output data:
  **                   - Energy of the impulse response : energy_L[CIR_LENGTH]
  **                    - Position of the best CIR window : indexBest
  **
  */
  
  indexBest = Energy(  cir, energy_L );
  
  
#if TEST_RHO == 1

  fprintf( out_file, "### Energy of CIR ###\n"); 
  for ( i = 0; i < g_BConsts_pt->cirLength; i++) 
  {
    fprintf( out_file, "%2.10f\n", LONG_2F(energy_L[i]) );
  }
  
#endif
  
  /*
  **
  **       normEnergy :
  **       ---------
  **               Clear lowest energies of the window found in Energy()
  **               and we compute the sum of the energies of this window
  **               and split it into mantissa * 2^exponent
  **
  **       Input data:     - Derotated samples : samples[2][NUM_DATA]
  **                       - Channel impulse response : cir[CIR_LENGTH
  **                       - Energy of the impulse response : energy_L[CIR_LENGTH]
  **                       - Position of the best .. : indexBest
  **                       - cirWindowStart, startinf position
  **
  **       Output data:    - Cleared Cir : cir[CIR_LENGTH]
  **                       - First non zero Cir (or begin of the burst) : bob
  **                       - average power in dB: power                                           
  **                       - exponent
  **
  **       Returned data   : mantInv = 1/Mantissa, (1 < mantInv <= 2)          
  **
  **
  */
  mantInv =   normEnergy
              (
                samples,
                cir,
                cirTemp,
                energy_L, 
                indexBest,
                &demodOut->toa,
                &demodOut->rcvPower, 
                &exponent
              );

  demodOut->rcvPower = demodOut->rcvPower>>2;
  
#if TEST_POWER == 1
   
   fprintf( out_file, "### Power ###\n"); 
   
   fprintf( out_file, "%2.4f\n", WORD14_2F( demodOut->rcvPower) );
   
#endif
  
  
  
  
  /*
  **
  **    normCir()
  **    ---------
  **
  **    We normalize the value of Cir ( Cir / Mantissa ).
  **
  **
  **    Input data:
  **               - Channel impulse response : ir[CIR_LENGTH]
  **                - 1/Mantissa : mantInv
  **               - Position of the best CIR window : indexBest
  **
  **    Output data:
  **               - Normalized channel impulse response : cirNorm[CIR_SIZE]
  **
  */
  normCir( cir, mantInv, indexBest, cirNorm );
  
  /*
  **
  **       calcRho() :
  **       -------
  **       Compute the autocorrelation of the Cir with normCir.
  **       calcRho will be used for the VITERBI algorithm as the real channel response.
  **
  **       Input data:     - Channel impulse response : cir[CIR_LENGTH]
  **                       - Normalized channel impulse resp. : normCir[CIR_SIZE]
  **                       - index of the best CIR window : indexBest
  **                       - Exponent of the normalized energy : exponent
  **
  **       Ouput data:     - Autocorrelation of the Cir : rho[DELAY_MAX]
  **
  */
  calcRho( cir, cirNorm, indexBest, exponent, rho );
  
#if TEST_RHO == 1
  
  fprintf( out_file, "### rho ###\n"); 
  for ( i = 0; i < CIR_BEST_LENGTH - 1; i++)
  {
    fprintf( out_file, "%2.8f\n", WORD3_2F(  rho[i] ));
  }
  
#endif
  
  /*
  **    matchFilter()
  **    -------------
  **     
  **      Compute matched filter for the burst.
  **
  **      Input data:  - Derotated samples : demodIn.samples[NUM_DATA]
  **                   - Normalized channel impulse response : NCir[CIR_SIZE]
  **                   - Position of the first data to compute : indexBest+TAIL
  **                   - Exponent of the normalized energy : Exponent
  **                   - Number of data to compute : 142
  **
  **      Ouput data:  -Demodulated burst : demodOut.softDec[BITS_PER_BURST]
  **
  */
  matchFilter(  samples, 
     cirNorm, 
     indexBest, 
     exponent, 
     demodSamps );
  
#if TEST_MATCHO == 1
  
  fprintf( out_file, "### Matcho after Matched() ###\n"); 
  for ( i = 0; i < g_BConsts_pt->numBits; i++)
  {
     fprintf( out_file, "%2.4f\n", WORD3_2F( demodSamps[i] ) );
  }
  
#endif
  
  /*
  **   equalize()
  **   ----------
  **
  **     This function will equalize the burst.
  **
  **
  **      Input data:
  **                   - Demodulated burst : demodOut.softDec[BURST_MATCH]
  **                   - Autocorrelation of the Cir : Rho[delay_l]
  **                   - Address of the Training sequence
  **
  **      Output data:
  **                   - Corrected burst : demodOut.softDec[BURST_MATCH]
  **
  */
  equalize(demodSamps, rho, g_BConsts_pt->tseqBaseAddr);
  
#if TEST_EQUALIZE == 1
  
  fprintf( out_file, "### Matcho after Equalize ###\n"); 
  
  for ( i = 0; i < g_BConsts_pt->numBits; i++) 
  {
     fprintf( out_file, "%2.4f\n", WORD3_2F( demodSamps[i] ) );
  }
  
#endif
  
  /*
  **   weight()
  **   --------
  **
  **      Weight the soft-decisions to enhance
  **      good quality bursts and reduce the impact of poor
  **      quality bursts on the decoder.
  **
  **
  **      Input data:     
  **                   -Soft Decisions : demodOut.softDec[BURST_MATCH]
  **                   - Address of the Training sequence
  **
  **      Output data:   
  **                   - Weighted burst : demodOut.softDec[BURST_MATCH]
  **                   - Signal to Noise Ratio : snr
  **
  */
  weight
  (
    demodSamps, 
    g_BConsts_pt->tseqBaseAddr, 
    &demodOut->snr
  );
     
#if TEST == 1
  moveStealingBits( chanSel, &demodSamps[0] ); 
#endif
   
#if TEST_SNR == 1
  
  fprintf( out_file, "### snr ###\n"); 
  fprintf( out_file, "%2.4f\n", (float) demodOut->snr);
  
#endif
  
  /*
  **  quantize()
  **  ----------
  **   Quantization of the soft-decisions to save memory and to 
  **     be compatable with the decoder input.
  **
  **   Input data:
  **               - Soft Decisions,  demodSamps[BURST_MATCH]
  **
  **   Output data - Weighted Soft Decsions : demodOut.softDec[BURST_MATCH]
  **
  */
  quantize( demodSamps, demodOut->data );
  
#if TEST_QUANT_SDP == 1
  
  fprintf( out_file, "### Matcho after Quantize ###\n"); 
  for ( i = 0; i < g_BConsts_pt->numDataBits; i++)
  {
     fprintf( out_file, "%2.4f\n", (float) demodOut->data[i]);
  }
  
#endif
   
}

#if TEST == 1
/*************************************************************************** 
*     moveStealingBits()
*     ----------------
*
*   This will move the stealing bits to the end of the burst for
*   normal bursts only.
* 
*  This is for comparison with the other demods.
***************************************************************************/
void moveStealingBits( t_chanSel chanSel, Word softDec[] )
{
  int i;
  Word hu, hl;

  /* 
  * Move the hu/hl bits to the end 
  * if this is a normal burst
  */
  if( chanSel != RACH )
  {
    hu = softDec[NB_NUM_ENCRYPT_BITS_PER_SIDE-1];
    hl = softDec[NB_NUM_ENCRYPT_BITS_PER_SIDE];
    for( i = 0; i < NB_NUM_ENCRYPT_BITS_PER_SIDE-2; i++ )
    {
      softDec[i+NB_NUM_ENCRYPT_BITS_PER_SIDE-1] = \
        softDec[i+NB_NUM_ENCRYPT_BITS_PER_SIDE+1];
    }
    softDec[NB_NUM_ENCRYPT_BITS_PER_BURST-2] = hl;
    softDec[NB_NUM_ENCRYPT_BITS_PER_BURST-1] = hu;
  }
}
#endif
