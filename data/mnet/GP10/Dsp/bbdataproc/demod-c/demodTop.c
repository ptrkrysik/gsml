/***********************************************************************
**
**  GSM Demod w/ TI C6201  implementation
**
** Description:
**
**  This is the main calling routine.  This will process the I/O files
**  and call the low level functions.
**
** $Author: whuang $
** $Date: 1999/07/06 19:35:02 $
** $Source: D:\\stageCell\\Dsp\\bbdataproc\\demod-c\\RCS\\demodTop.c $
** $Revision: 1.1 $
** $Log: demodTop.c $
** Revision 1.1  1999/07/06 19:35:02  whuang
** Initial revision
**
** Revision 1.1  1999-01-07 14:30:24-08  randyt
** Initial revision
**
 * 
 *    Rev 1.0   Oct 27 1998 16:19:34   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   Oct 23 1998 17:56:08   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
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

#include "dsp/dsptypes.h"
#include "gsmtypes.h"
#include "bbdata.h"
#include "bbproto.h"

/* 
*  DEMOD_TOP is used to define the globals, 
*   this should be the only file where it is defined
*/
#define  DEMOD_TOP

/* Include globals and typedefs */
#include "dem_const.h" 
#include "training.h"
#include "dem_globs.h"
#include "dem_protos.h"


       

#if TEST == 1     

/* I/O File global variables */
extern FILE    *out_file;

void moveStealingBits( t_chanSel chanSel, Word softDec[] );
#endif

void demodulate( t_ULComplexSamp   samples[BURST_SIZE_UL_MAX],
                 t_chanSel     chanSel,
                 UChar         tseqNum,
                 t_ULBurstBuf  *demodOut )
{
   
   Word     cir[2][MAX_CIR_LENGTH];       /* Impulse response */
   LWord    energy_L[MAX_CIR_LENGTH];     /* Energy */
   Word2    cirNorm[2][MAX_CIR_LENGTH];   /* Normalized Cir */
   Char     indexBest;              /* Position of the best CIR window */
   Word3    rho[DELAY_MAX];         /* Autocorrelation of the Cir, used by Viterbi */
   /* Demodulated samples before quantizing */
   /*  See if this could be demodIn->samples to save stack space */
   Word3    demodSamps[MAX_NUM_BITS_PER_BURST]; 

   UWord2   mantInv;  /* 1/mantissa of energySum = mantissa * 2^exponent */
   Short   exponent; 
   
   
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
   offsetDerot(  samples, DC_OFFSET_THRESHOLD );
   
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
   **    Output:    Channel impulse response - cir[2][CIR_LENGTH]
   **
   */  
   impulse( samples, g_BConsts_pt->tseqBaseAddr,  cir );
   

   
#if TEST_RHO == 1   
   fprintf( out_file, "### Impulse Response ###\n"); 
   for ( i = 0; i < g_BConsts_pt->cirLength; i++)  
   { 
      fprintf( out_file, "%2.10f\n", WORD_2F( cir[0][i] )); 
      fprintf( out_file, "%2.10f\n", WORD_2F( cir[1][i] )); 
   }
#endif
   
   /*
   **    Compute the energy of the impulse response
   **     and try to find the best energy window of length CIR_SIZE
   **
   **
   **     Input data:
   **                   - The channel impulse response : cir[2][CIR_LENGTH]
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
   **                       - Channel impulse response : cir[2][CIR_LENGTH
   **                       - Energy of the impulse response : energy_L[CIR_LENGTH]
   **                       - Position of the best .. : indexBest
   **                       - cirWindowStart, startinf position
   **
   **       Output data:    - Cleared Cir : cir[2][CIR_LENGTH]
   **                       - First non zero Cir (or begin of the burst) : bob
   **                       - average power in dB: power                                           
   **                       - exponent
   **
   **       Returned data   : mantInv = 1/Mantissa, (1 < mantInv <= 2)          
   **
   **
   */
   mantInv =  normEnergy(  samples,
      &cir[0], 
      &energy_L[0], 
      indexBest,
      &demodOut->toa,
      &demodOut->rcvPower, 
      &exponent);

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
   **               - Channel impulse response : ir[2][CIR_LENGTH]
   **                - 1/Mantissa : mantInv
   **               - Position of the best CIR window : indexBest
   **
   **    Output data:
   **               - Normalized channel impulse response : cirNorm[2][CIR_SIZE]
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
   **       Input data:     - Channel impulse response : cir[2][CIR_LENGTH]
   **                       - Normalized channel impulse resp. : normCir[2][CIR_SIZE]
   **                       - index of the best CIR window : indexBest
   **                       - Exponent of the normalized energy : exponent
   **
   **       Ouput data:     - Autocorrelation of the Cir : rho[DELAY_MAX]
   **
   */
   calcRho( cir, cirNorm, indexBest, exponent, rho );
   
#if TEST_RHO == 1
   
   fprintf( out_file, "### rho ###\n"); 
   for ( i = 0; i < CIR_BEST_LENGTH - 1; i++) {
      fprintf( out_file, "%2.8f\n", WORD3_2F(  rho[i] ));
   }
   
#endif
   
   /*
   **    matchFilter()
   **    -------------
   **     
   **      Compute matched filter for the burst.
   **
   **      Input data:  - Derotated samples : demodIn.samples[2][NUM_DATA]
   **                   - Normalized channel impulse response : NCir[2][CIR_SIZE]
   **                   - Position of the first data to compute : indexBest+TAIL
   **                   - Exponent of the normalized energy : Exponent
   **                   - Number of data to compute : 142
   **
   **      Ouput data:  -Demodulated burst : demodOut.softDec[BITS_PER_BURST]
   **
   */
   matchFilter(  samples, 
      &cirNorm[0], 
      indexBest, 
      exponent, 
      &demodSamps[0] );
   
#if TEST_MATCHO == 1
   
   fprintf( out_file, "### Matcho after Matched() ###\n"); 
   for ( i = 0; i < g_BConsts_pt->numBits; i++) {
      
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
   equalize(&demodSamps[0], rho, g_BConsts_pt->tseqBaseAddr);
   
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
   weight( demodSamps, 
      g_BConsts_pt->tseqBaseAddr, 
      &demodOut->snr);
      
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
   quantize( demodSamps, &demodOut->data[0] );
   
#if TEST_QUANT_SDP == 1
   
   fprintf( out_file, "### Matcho after Quantize ###\n"); 
   for ( i = 0; i < g_BConsts_pt->numDataBits; i++) {
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


