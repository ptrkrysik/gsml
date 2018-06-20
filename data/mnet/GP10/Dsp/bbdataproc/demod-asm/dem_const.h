/*
**   GSM Demod Constants for A|RT library implentation
**
** $Author: hpines $
** $Date: 1999/07/14 21:45:01 $
** $Source: D:\\stageCell\\Dsp\\bbdataproc\\demod-asm\\RCS\\dem_const.h $
** $Revision: 1.1 $
** $Log: dem_const.h $
** Revision 1.1  1999/07/14 21:45:01  hpines
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
** Revision 1.5  1998/10/14 18:46:30  bhehn
** TEST mode  = 0.
**
** Revision 1.4  1998/10/14 18:43:27  bhehn
** AB & NB verified.  Merged with shared header.
** AB ~= 103us, NB ~= 78us
**
** Revision 1.3  1998/10/12 15:39:47  bhehn
** TEST mode on as default for testing
**
** Revision 1.1  1998/10/07 15:21:35  bhehn
** Initial revision
**
** Revision 1.1  1998/10/01 13:56:58  bhehn
** Initial revision
**
** Revision 1.3  1998/09/22 20:00:37  bhehn
** Access Burst support added and verified.
**
** Revision 1.2  1998/09/21 22:04:23  bhehn
** NB demod works, BER close to floating
**
*/
#ifndef DEM_CONST_H
#define DEM_CONST_H


/*                                                
*   Compilation Flags                               
*     These TEST FLAGS will output data to a file so
*       that it can be compared to others versions.                                                   
*/

#define TEST            0
#define TEST_RHO        TEST
#define TEST_POWER      TEST
#define TEST_MATCHO     TEST



#define TEST_EQUALIZE   TEST
#define TEST_SNR        TEST
#define TEST_QUANT_SDP  TEST
                   
/* These next 2 should normally be 0 */
#define TEST_PATH_SD    0
#define TEST_METRICS    0
/***************************************************************************
*
*                               CONSTANTS
*
****************************************************************************/

/* 
*  Define the number of samples per burst
*    in the input files.  This is the *.dsp file
*/
#define NUM_INPUT_SAMPLES_PER_BURST  160

/*
** Define the number of samples in the burst
** rounded down.
** floor(156.25) = 156
*/
#define GSM_NUM_SAMPLES_PER_BURST  156


/* Threshold value used in offset compensation */
/* Can not be 1.0 because fixed point can't represent it. */
/* #define DC_OFFSET_THRESHOLD    (0.999) in floating point */
#define DC_OFFSET_THRESHOLD    ( (1 << WORD_SIZE-1)-1 )

/*
* Define constants for the bursts
*
* Normal Burst
*  -----------------------------------------------------------
* |TB=3|  DATA = 58   | TSEQ = 26 |  DATA = 58   |TB=3|GP=8.25|
*  -----------------------------------------------------------
*
* Access Burst
*  -----------------------------------------------------------
* |TB=8| TSEQ = 41 | DATA = 36 |TB=3|    GP=68.25             |
*  -----------------------------------------------------------
*
* Sync Burst
*  -----------------------------------------------------------
* |TB=3| DATA = 39|     TSEQ = 64   |  DATA = 39 |TB=3|GP=8.25|
*  -----------------------------------------------------------
*/
#define NB_NUM_SAMPLES      160
#define AB_NUM_SAMPLES      200
#define SB_NUM_SAMPLES      200     
   
#define NB_NUM_DATA         ((NB_NUM_SAMPLES/4)*4)
#define AB_NUM_DATA         ((AB_NUM_SAMPLES/4)*4)
#define SB_NUM_DATA         ((SB_NUM_SAMPLES/4)*4)
#define MAX_NUM_DATA        AB_NUM_DATA


/* Define the number of bits in the burst less tail and guard bits */
#define NB_NUM_BITS_PER_BURST  (NB_NUM_ENCRYPT_BITS_PER_BURST + NB_TRAIN_SIZE)
#define AB_NUM_BITS_PER_BURST  (AB_NUM_ENCRYPT_BITS_PER_BURST + AB_TRAIN_SIZE)
/* Define the maximum number of valid bits in a burst */
/*   This is the normal burst case, the AB has a lot  */
/*   more guard bits.                                 */
#define MAX_NUM_BITS_PER_BURST  (NB_NUM_BITS_PER_BURST)


/*
* CIR_NUM_TSEQ_BITS: Number of training sequence samples to
*  correlate with.
*  Also, define the max number of bits needed for CIR_NUM_TSEQ_BITS
* CIR_NUM_TSEQ_BITS max is 64 (SB) + an offset (<10) so     
*  7 bits are needed.
*/
#define NB_CIR_NUM_TSEQ_BITS         16
#define SB_CIR_NUM_TSEQ_BITS         SB_TRAIN_SIZE
#define AB_CIR_NUM_TSEQ_BITS         AB_TRAIN_SIZE
                                            
/* 
* OFFSET is the offset into the Tseq to where the CIR_NUM_TSEQ_BITS
* will start from.  
* The ideal CIR is shown below for the +/- tseq values starting at the offset:
*
*                 OFFSET
*                    |
* cir value   0 0 0 16 0 0 0 0 0 0
* index       0 1 2  3 4 5 6 7 8 9
*               ^        ^
*               |        |
*             toaMin   toaMax
*                        |<-   ->| - CIR_BEST_LENGTH
*               |<- CIR_LENGTH ->|
*
*/
#define  NB_IMPULSE_TSEQ_OFFSET    (3)
#define  NB_TOA_MIN                (-2)
#define  NB_TOA_MAX                (2)
#define  CIR_BEST_LENGTH           (5)

/*
* The AB does not need a negative TOA.  
* Therefore, offset and TOA_MIN are zero
*/
#define  AB_IMPULSE_TSEQ_OFFSET    (0)
#define  AB_TOA_MIN                (0)
#define  AB_TOA_MAX                (64)



#define BURST_START     3
#define CIR_SIZE        5

/*
* CIR_LENGTH:  Channel Impulse Response Window Length
*  NB is finely syncronized so the window does not have to very large
*  SB the mobile is aquiring the BS timing so this needs to be fairly large
*  AB the BS is aquiring the MS timing so this needs to be fairly large
*/
#define  NB_CIR_LENGTH  (CIR_BEST_LENGTH + NB_TOA_MAX - NB_TOA_MIN)
#define  AB_CIR_LENGTH        64
#define  SB_CIR_LENGTH        50

/* Define the maximum that the CIR length can be */
/* This is the AB case */
#define MAX_CIR_LENGTH    AB_CIR_LENGTH

/*
* Define the position in the data where the CIR calculation begins
*/
#define NB_CIR_START_INDEX   (NB_NUM_TAIL_BITS + NB_NUM_ENCRYPT_BITS_PER_SIDE \
                                  + NB_IMPULSE_TSEQ_OFFSET + NB_TOA_MIN)

#define AB_CIR_START_INDEX   (AB_NUM_TAIL_BITS)
/*#define SB_CIR_WINDOW_START   (TAIL + SB_NUM_DATA_BITS_PER_SIDE) */





#define NUM_CHOICE       (NB_CIR_LENGTH - CIR_SIZE + 1)
#define NB_MATCH        32
#define C_USED_SAMPLE   (MAX_BITS_PER_BURST + (CIR_SIZE-1) + (NB_CHOICE-1) )


/*
**  offsetDerot() related constants
**
** Define the number of samples over which to calc the offset
**   make it a power of two so that no shifting is needed.
**
** NUM_OFFSET_SAMPS = number of samples to compute the offset over.
** It is not 157 (number of samples in a burst) to make
** the scaling easier
*
** Get the log2 to determine how many bits are needed to represent
** the number.  Note log2(128)=7
** However, it takes log2(128)+1 bits to represent 128.
** And ceil(log2(7)) = 3
*/
#define OFF_NUM_OFFSET_SAMPS           (128)
#define OFF_NUM_OFFSET_SAMPS_LOG2      (7)
/* Need the number of bits needed to represent NUM_OFFSET_SAMPS_LOG2 */
#define OFF_NUM_OFFSET_SAMPS_LOG2_BITS (3)

/* The type for the i value in offset comp */
/* #define OFF_I_TYPE  Uint<OFF_NUM_OFFSET_SAMPS_LOG2+1> */
#define OFF_I_TYPE  UChar

/* 
** To scale the accum data we need to divide by 128 or shift by 7 (2^7 =128)
** Number of bits that can represent the shift value, 7 (0b111) can be represented by 3 bits
*/
/*#define OFF_SHIFT_VAL  (Uint<OFF_NUM_OFFSET_SAMPS_LOG2_BITS> (OFF_NUM_OFFSET_SAMPS_LOG2))*/
#define OFF_SHIFT_VAL    (OFF_NUM_OFFSET_SAMPS_LOG2)

/* 
**  ENERGY_THRESHOLD_PERCENT is the percentage of the total energy that 
**    each energy element must be greater then else it will be cleared out 
**
**  It is in the format F8.8
** 
**  Ex: 0.05 ~= (1<<8)*0.05 ~= 13 = 0.05078
**
** Use format F8.8 so that the result will go into the gaurd bits
**  without overflowing
*/
#define ENERGY_THRESHOLD_PERCENT           (13)
/* Define the format for the energy threshold (F8.8) */
#define E_THRESH_FORMAT                    (8)

/* For Equalizer */
#define DELAY_MAX       5
#define STATES_MAX      (1 << DELAY_MAX)
#define BURST_MATCH     (MAX_NUM_BITS_PER_BURST + DELAY_MAX + 2)

/* 
* Compute 1.0/mantissa
*  "one" can not be represented in fixed point 
* so approximate it by "1 - word precision" 
*/
//#define ONE_MINUS_PRECISION (1.0 - 1.0 /  (float) (1 << (WORD_SIZE-1)) )
#define ONE_MINUS_PRECISION (  ( 1<<(WORD_SIZE-1) )-1 )

#endif
