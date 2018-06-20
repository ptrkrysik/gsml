/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: dem_globs.h
*
* Description:   
*   GSM Demod Globals & types. Old revision log at end of file.
*
*   NOTE: INGLOB and EXGLOB specify internal and external DSP memory
*         allocation, respectively. They are defined in dsptypes.h
*         and included here through bbtypes.h. 
*
*****************************************************************************/
#ifndef _DEM_GLOBS_H
#define _DEM_GLOBS_H  /* include once only */

#include "dem_const.h"

/* Define the wordlength of the data */
#define WORD_SIZE          (16)
#define ACCUM_GUARD_BITS   (8)

/* Define some common types */  

typedef signed int   SInt;      /* signed integer */
typedef Short        Word;      /*  word */
typedef UShort       UWord;     /*  unsigned word */
typedef SInt         LWord;     /* long word */
typedef UInt         ULWord;    /* unsigned long word */
typedef Long         VLWord;    /* very long word */
typedef ULong        UVLWord;   /* unsigned very long word */

/* 
* Define the data types for variables with the decimal point
*   at different places.
*
* The number appended to these indicates how many integer bits in
*   the word.
* For instance, a "Word2" has two integer bits and can be converted
* to a "Word" (one integer bit) with a right shift on 2-1 = 1
* A "Wordm3" can be converted to a "Word" by a right shift of -3-1 = -4
* or a left shift of 4.
* Note that these types are just reminders of where the decimal point is 
* located, they are all actually of type "Short"
* 
* x = 1 data bit, S = sign bit, X = xxxx  (4 bits)
*/
/*typedef Word   Word;  /*  1 interger bits, Fix<16,16-1>    S.xxxXXX */
typedef Word    Word2;  /*  2 interger bits, Fix<16,16-2>   Sx.xxXXX */
typedef Word    Word3;  /*  3 interger bits, Fix<16,16-3>     Sxx.xXXX */
typedef Word    Word8;  /*  8 interger bits, Fix<16,16-8>   SxxxX.XX */
typedef Word    Word14; /*  14 interger bits, Fix<16,16-14>  XXXxx.xx*/

typedef Word    Wordm3; /*  3 interger bits, Fix<16,16+2>  SSS.SSSSxxxXXX*/

typedef UWord   UWord2; /*  2 interger bits,  UFix<16,16-2> 0xx.xxXXX */
typedef LWord   LWord2; /*  2 interger bits,  Fix<32,32-2>   Sx.xxXXXXXXX */
typedef LWord   LWord17; /*  17 interger bits,  Fix<32,32-17>, 15 fract bits */

/*
* Define macros  that convert data formats to floating point.
* These are used to output data to a file which will then be 
* compared with other demod implementations.
*/
#define WORD_2F(x)   ( ((float ) (x)) / (float)  (1<<WORD_SIZE-1) )
#define WORD2_2F(x)  ( ((float ) (x)) / (float)  (1<<WORD_SIZE-2) )
#define WORD3_2F(x)  ( ((float ) (x)) / (float)  (1<<WORD_SIZE-3) )
#define WORD4_2F(x)  ( ((float ) (x)) / (float)  (1<<WORD_SIZE-4) )
#define WORD8_2F(x)  ( ((float ) (x)) / (float)  (1<<WORD_SIZE-8) )
#define WORD14_2F(x) ( ((float ) (x)) / (float)  (1<<WORD_SIZE-14) )
#define LONG_2F(x)   ( ((double) (x)) / (double) ((1<<2*WORD_SIZE-1)-1) )

/* 
* Define the data type for the equalizer input data 
*  5 bits left of the decimal point are needed,
*  1 for the sign, 4 for the integer part.
*  4 integer bits are needed because the data
*  of this type was multiply/accumulated  
*  over CIR_SIZE (5) and it was complex (*2).
*  Therefore, this data has a scale of 2*5 = 10.
*  2^4 = 16 can handle the scale of 10.
*/
/* typedef Fix<WORD_SIZE,WORD_SIZE-5>  t_EqualData; */   
typedef Word  t_EqualData;

/* typedef  Uint<7> CIR_NUM_TSEQ_BITS_TYPE;   */
typedef  UChar      CIR_NUM_TSEQ_BITS_TYPE;

/* Define the max number of bits needed for CIR_LENGTH */
/* For AB CIR_LENGTH = 64 = 0x1000000 = 7 bits         */
typedef  UChar  CIR_LENGTH_TYPE;

/* 
* Define the training sequence word length, 
* tseq = -1 or 1 = 2 bits 
*/
typedef Char  t_TseqWord; 


/*
** Define a structure that contains all the unique constants for each burst type.
**
** NOTE: 
** NB: CIR_NUM_TSEQ_BITS = 16, SB: CIR_NUM_TSEQ_BITS = 64, AB: CIR_NUM_TSEQ_BITS = 41)
** "cirNumTseqBitsInv" max is 1/16 = 0.0625 = 0b0.0001 so maximize precision.
** ex: Fix<8,7> would be S.XXXXXX, Fix<8,10> would be S.SSSXXXXXXX
** where X is a significant digit and S is a sign bit.
** In this example you could not have Fix<8,11> because of the sign bit.
**
** cirWindowStart defines the position that the CIR calculation begins.
*/
typedef struct  {
   const t_TseqWord              *tseqBaseAddr; /* Training sequences base addrress */
   const UChar                   numTseqBits;		/* Number of tseq bits in the burst */
   const CIR_NUM_TSEQ_BITS_TYPE  cirNumTseqBits;		/* Number of tseq samples to correlate over */
   const Wordm3                  cirNumTseqBitsInv;	/* 1.0/CIR_NUM_TSEQ_BITS */
   const UChar                   impulseTseqOffset;  /* Position of the first tseq bit for CIR calc. */
   const UChar                   cirLength;     /* Length of the CIR */
   const UChar                   cirStartIndex; /* index of first samples to calc CIR over */
   const Char                    toaMin;        /* Minimum posible time of arrival, relative */
   const UChar                   numBits;       /* Number of bits in the burst less tail/guard bits*/
   const UChar                   numEncryptBits;   /* Number of data bits in the burst */
                                                 /*  NB = 58*2, AB = 36 */
   const UChar                   numEncryptBitsPerSide;   /* Number of data bits on each side of the tseq */
   const UChar                   tseqStart;   /* starting position of the training sequence */
   const UChar                   numTailBits;   /* Number of tail bits at the beginning of the burst */

} t_burstConsts;

/*
** Define structures that have the constants for each burst type
*/ 
#ifdef ALLOCATE_MEMORY_INTERNAL /*-----------------------------------------------*/

const t_TseqWord train_nb[8][26] = 
{
  { -1, -1, +1, -1, -1, +1, -1, +1, +1, +1, -1, -1, -1,
    -1, +1, -1, -1, -1, +1, -1, -1, +1, -1, +1, +1, +1 },

  { -1, -1, +1, -1, +1, +1, -1, +1, +1, +1, -1, +1, +1,
    +1, +1, -1, -1, -1, +1, -1, +1, +1, -1, +1, +1, +1 },

  { -1, +1, -1, -1, -1, -1, +1, +1, +1, -1, +1, +1, +1,
    -1, +1, -1, -1, +1, -1, -1, -1, -1, +1, +1, +1, -1 },

  { -1, +1, -1, -1, -1, +1, +1, +1, +1, -1, +1, +1, -1,
    +1, -1, -1, -1, +1, -1, -1, -1, +1, +1, +1, +1, -1 },

  { -1, -1, -1, +1, +1, -1, +1, -1, +1, +1, +1, -1, -1,
    +1, -1, -1, -1, -1, -1, +1, +1, -1, +1, -1, +1, +1 },

  { -1, +1, -1, -1, +1, +1, +1, -1, +1, -1, +1, +1, -1,
    -1, -1, -1, -1, +1, -1, -1, +1, +1, +1, -1, +1, -1 },

  { +1, -1, +1, -1, -1, +1, +1, +1, +1, +1, -1, +1, +1,
    -1, -1, -1, +1, -1, +1, -1, -1, +1, +1, +1, +1, +1 },

  { +1, +1, +1, -1, +1, +1, +1, +1, -1, -1, -1, +1, -1,
    -1, +1, -1, +1, +1, +1, -1, +1, +1, +1, +1, -1, -1 }
}; 

const t_TseqWord train_ab[41] =
{
  -1, +1, -1, -1, +1, -1, +1, +1, -1, +1,
  +1, +1, +1, +1, +1, +1, +1, -1, -1, +1,
  +1, -1, -1, +1, +1, -1, +1, -1, +1, -1,
  +1, -1, -1, -1, +1, +1, +1, +1, -1, -1, -1
};

t_burstConsts g_ABConsts = { &train_ab[0],
                             AB_TRAIN_SIZE,
                             AB_CIR_NUM_TSEQ_BITS,
                             (1<<WORD_SIZE)/AB_CIR_NUM_TSEQ_BITS,
                             AB_IMPULSE_TSEQ_OFFSET,
                             AB_CIR_LENGTH,
                             AB_CIR_START_INDEX,
                             AB_TOA_MIN,
                             AB_NUM_BITS_PER_BURST,
                             AB_NUM_ENCRYPT_BITS_PER_BURST,
                             AB_NUM_ENCRYPT_BITS_PER_BURST,
                             AB_TRAIN_START,
                             AB_NUM_TAIL_BITS
                           };   
                                      
t_burstConsts g_NBConsts  = { &train_nb[0][0],
                              NB_TRAIN_SIZE,
                              NB_CIR_NUM_TSEQ_BITS,
                              (1<<WORD_SIZE)/NB_CIR_NUM_TSEQ_BITS,
                              NB_IMPULSE_TSEQ_OFFSET,
                              NB_CIR_LENGTH,
                              NB_CIR_START_INDEX,
                              NB_TOA_MIN,
                              NB_NUM_BITS_PER_BURST,
                              NB_NUM_ENCRYPT_BITS_PER_BURST,
                              NB_NUM_ENCRYPT_BITS_PER_SIDE,
                              NB_TRAIN_START,
                              NB_NUM_TAIL_BITS
                            };
                            
#else  /*----------------------------------------------------------------------*/

extern const t_TseqWord train_nb[8][26]; 
extern const t_TseqWord train_ab[41];

extern t_burstConsts    g_ABConsts;    /* Access burst constants */
extern t_burstConsts    g_NBConsts;    /* Normal burst constants */
 
#endif /*----------------------------------------------------------------------*/

INGLOB t_burstConsts    *g_BConsts_pt; /* Pointer to the burst constants */
INGLOB t_chanSel        g_burstType;   /* Burst type */

#endif  /* end of include once only */


/**************** Old revision log ******************
**
** Revision 1.3  1999/07/14 23:32:44  hpines
** header files to support Equalizer version of demod routines
**
** Revision 1.2  1999-05-03 11:53:45-07  whuang
** Added Viterbi equalizer function
**
** Revision 1.4  1998/12/17 16:59:29  bhehn
** Added support for deEmphasis of low snr signals.
**
** Revision 1.3  1998/10/27 21:39:54  bhehn
** AB & NB eqaulizer verifed.
** AB ~= 125us, NB ~= 155us.
**
** Revision 1.2  1998/10/14 18:43:28  bhehn
** AB & NB verified.  Merged with shared header.
** AB ~= 103us, NB ~= 78us
**
** Revision 1.1  1998/10/07 15:21:36  bhehn
** Initial revision
**
** Revision 1.2  1998/10/02 17:01:01  bhehn
** Changed calling routine to be more like c6x  implementation.
** BER matches previous version.
**
** Revision 1.1  1998/10/01 13:56:58  bhehn
** Initial revision
**
** Revision 1.3  1998/09/22 20:00:37  bhehn
** Access Burst support added and verified.
**
** Revision 1.2  1998/09/21 22:04:24  bhehn
** NB demod works, BER close to floating
*/

