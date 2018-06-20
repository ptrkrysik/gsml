/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: demtop.c
*
* Description:
*   GSM Demodulator top level. Old logs at bottom of file.            
*
****************************************************************************/
#include <stdio.h> 
#include <stdlib.h>
#include "gsmdata.h"
#include "bbdata.h"
#include "bbproto.h"
#include "c6xsupport.h"
#include "dem_globs.h"
#include "dem_aprotos.h"

#define  LOOP_TYPE int
#define  MAX_NUM_LOPASS_FILTER_TAPS 48 

Int32 snrZeroCount=0;

/* 
 * GSM Channel Filter (window-based design using Kaiser window in Matlab)
 * Taps=39 (pad to 40) Cut=80 Stop=93 Amp=1.00, 0.00 Ripple=0.100, 0.020
 * If changing filter length, may have to adjust MAX_NUM_LOPASS_FILTER_TAPS.
 * Also remember that sum of coef absolute values must be less than 32768.  
 */                         
Int16 g_numLopassTaps = 40;
Int16 g_loPassTaps[40] = {
       0,      30,     -90,      47,      86,    -156,      34,     186,    
    -233,     -26,     349,    -311,    -176,     620,    -379,    -535,
    1183,    -425,   -1903,    4546,   10093,    4546,   -1903,    -425,   
    1183,    -535,    -379,     620,    -176,    -311,     349,     -26,  
    -233,     186,      34,    -156,      86,      47,     -90,      30 };

void firLoPass(short x[], short h[], short y[], int N, int M); 

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

int c_asm_consts[36];

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
  c_asm_consts[8] = ((char *)&t.numEncryptBits)     -   ((char *)&t);
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

void demodulate
(
  t_ULComplexSamp    samples[BURST_SIZE_UL_MAX],
  t_chanSel          chanSel,
  UChar              tseqNum,
  t_ULBurstBuf       *demodOut,
  t_groupNum         groupNum,
  UChar              rfCh,
  UChar              timeSlot,
  UChar              subChan
)
{
   
  t_ULCirSamp cir[MAX_CIR_LENGTH];          /* Impulse response */
  t_ULCirSamp cirTemp[MAX_CIR_LENGTH];      /* Saved Impulse response */
  LWord       energy_L[MAX_CIR_LENGTH];     /* Energy */
  t_ULCirSamp cirNorm[MAX_CIR_LENGTH];      /* Normalized Cir */
  Char        indexBest;           /* Position of the best CIR window */
  Word3       rho[DELAY_MAX];      /* Autocorr of Cir, used by Viterbi */
  Short       filterSamples[BURST_SIZE_UL_MAX+MAX_NUM_LOPASS_FILTER_TAPS];

  /* Demodulated samples before quantizing */
  /*  See if this could be demodIn->samples to save stack space */

  /* 32bit align the demodSamps array! Required by assembly. */
  /* new 4/20/99..added +4 to SampInts dimension to avoid stack overflo */
  Int         demodSampInts[(MAX_NUM_BITS_PER_BURST+1+4)/2];
  Word3       *demodSamps=(Word3 *)demodSampInts;

  UWord2      mantInv;  /* 1/mantissa of energySum = mantissa * 2^exponent */
  Short       exponent; 
   
  int  i,j;  /* used to index arrays */

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
  **     Offset compensation + GSM Channel Filtering + Derotation
  **
  **     Input data     . samples[2][NUM_DATA]
  **
  **     Output data    . samples[2][NUM_DATA]
  **     samples have been overwritten by compensated + derotated samples
  **
  ** NOTE: For RACH, use the all-carrier DC offset computed during idle frames.
  **       For SDCCH, use the DC offset computed for that logical channel.
  **       For TCH, use the DC offset for the time slot containing the TCH.
  **       For SACCH, use DC offset of its associated TCH or SDCCH.
  */

   /*--------------------------------------------------------------------
   *  Remove the DC offset specific to this TCH time slot or SDCCH
   *  subchannel. For RACH, use the DC offset computed over idle frames.
   *--------------------------------------------------------------------*/
   if ( chanSel == RACH )
   {
      removeDCOffset( samples, chanSel, rfCh, NUM_TN_PER_RF + NUM_SDCCH_SUBCHANS );
   }
   else if ( groupNum == GSM_GROUP_5 )  /* SDCCH4 or its SACCH */ 
   {
      removeDCOffset( samples, chanSel, rfCh, NUM_TN_PER_RF + subChan );
   }
   else if (groupNum == GSM_GROUP_7){
     removeDCOffset ( samples, chanSel, rfCh, subChan);
   } else {
      removeDCOffset( samples, chanSel, rfCh, timeSlot );
   }

   /*--------------------------------------------------------------------
   *  Apply GSM channel (lowpass) filter to input data
   *--------------------------------------------------------------------*/
  j = g_numLopassTaps / 2 ;
  for ( i = 0; i < j; i++)
  {
    filterSamples[i] = 0;
    filterSamples[i+j+GSM_NUM_SAMPLES_PER_BURST] = 0;
  }

   /*  Filter real part first */

  for ( i = 0; i < GSM_NUM_SAMPLES_PER_BURST; i++)
  {
    filterSamples[i+j] = samples[i].I;
  }
  firLoPass(filterSamples,g_loPassTaps,&samples[0].I,g_numLopassTaps,GSM_NUM_SAMPLES_PER_BURST);
 
   /*  Filter imaginary part next */

  for ( i = 0; i < GSM_NUM_SAMPLES_PER_BURST; i++)
  {
    filterSamples[i+j] = samples[i].Q;
  }
  firLoPass(filterSamples,g_loPassTaps,&samples[0].Q,g_numLopassTaps,GSM_NUM_SAMPLES_PER_BURST);
 
   /*--------------------------------------------------------------------
   *  Derotate samples
   *--------------------------------------------------------------------*/
  derotate( samples );
          
                    
  /**********************************************************************
  **
  **    Estimate the Channel Impulse Response
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
   
  /**********************************************************************
  **
  **    Compute the energy of the impulse response
  **     and try to find the best energy window of length CIR_SIZE
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
  
  /**********************************************************************
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

   /*--------------------------------------------------------------------
   * Modify received power: (1) divide by 4 to convert from 0.25-dB to
   * 1-dB steps, (2) add 6 dB to compensate for GSM channel filter 
   *--------------------------------------------------------------------*/
  demodOut->rcvPower = (demodOut->rcvPower >> 2) + 6; 
  
#if TEST_POWER == 1   
   fprintf( out_file, "### Power ###\n"); 
   fprintf( out_file, "%2.4f\n", WORD14_2F( demodOut->rcvPower) );
#endif
   
  
  /**********************************************************************
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
  
  
  /**********************************************************************
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

/*  NEW 3/23/99...SINCE CANNOT MAKE THESE MODS IN FLYNN'S DEMSUBS */

  for ( i = 0; i < 4; i++)
  {
     rho[i] = rho[i] >> 5;
  }
  
  /**********************************************************************
  **
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
   **  Compute the quality metric for this burst.
   */
   demodOut->snr = computeSNR( &demodSamps[g_BConsts_pt->tseqStart],chanSel );
   
   /*
    *---The two loops below are the only MIPS consumers in this file----
    */

/*  NEW 3/23/99...SINCE CANNOT MAKE THESE MODS IN FLYNN'S DEMSUBS */

   for ( i = 0; i < g_BConsts_pt->numBits; i++)
   {
      demodSamps[i] = demodSamps[i] >> 5;
   }

  if(demodOut->snr <= 4)
   {
    snrZeroCount++;
    for ( i = 0; i < g_BConsts_pt->numBits; i++)
     {
       demodSamps[i] = demodSamps[i] * -1;   
     }
   }
  
  /**********************************************************************
  **
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
/*  equalize(demodSamps, rho, g_BConsts_pt->tseqBaseAddr);  */
   equalize(&demodSamps[0], rho, g_BConsts_pt->tseqBaseAddr,chanSel);
  
   /*
   **  Compute the post equalized quality metric for this burst.
   */
  if( chanSel == RACH )
   demodOut->snr = computeSNR( &demodSamps[AB_NUM_ENCRYPT_BITS_PER_BURST],chanSel );

#if TEST_EQUALIZE == 1
  fprintf( out_file, "### Matcho after Equalize ###\n"); 
  for ( i = 0; i < g_BConsts_pt->numBits; i++) 
  {
     fprintf( out_file, "%2.4f\n", WORD3_2F( demodSamps[i] ) );
  }
#endif
  
   /**********************************************************************
   **
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
   */
   deEmph( chanSel,
      &demodSamps[0], 
      g_BConsts_pt->tseqBaseAddr, 
      demodOut->snr);
      
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
   quantize( &demodSamps[0], &demodOut->data[0] );
   
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


/************************** Old revision log ****************************************
**
** Revision 1.12  2000-01-25 18:52:01-08  dkaiser
** Works with new demodc.c. Just a comment change.
**
** Revision 1.9  1999-10-27 13:34:41-07  dkaiser
** added new parameters to demodulate function call
** new logic to control which DC offset value is used with each logical channel
**
** Revision 1.8  1999-10-18 18:52:20-07  hpines
** Added enhanced RACH detection test..call computeSNR over 41 RACH training sequence bits.
**
** Revision 1.7  1999-10-06 17:01:11-07  dkaiser
** Added rfCh input param to OffsetDerot function. Later, we need to pass rfCh into Demodulate function, which means changing bbproto.h, pagch.c, sacch.c, sdcch.c and tch.c
**
** Revision 1.6  1999-09-23 11:38:03-07  dkaiser
** removed offsetDerot, moved to demodc.c
** added channel parameter to computeSNR
**
** Revision 1.5  1999-07-14 16:25:49-07  hpines
** Updated equalizer demod version:
** 1.  Fixed dc offset routine by using C-version instead optimized assembly code.
** 2.  uses snr to search for frame sign inversion
**
** Revision 1.4  1999-05-05 14:18:23-07  whuang
** corrected Power meseament(in dB)
**
** Revision 1.3  1999-05-03 11:53:41-07  whuang
** Added Viterbi equalizer function
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
*/


