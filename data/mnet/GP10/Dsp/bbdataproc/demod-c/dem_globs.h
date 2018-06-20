/*
**   GSM  Demod Globals & types
**
** $Author: whuang $
** $Date: 1999/07/06 19:35:00 $
** $Source: D:\\stageCell\\Dsp\\bbdataproc\\demod-c\\RCS\\dem_globs.h $
** $Revision: 1.1 $
** $Log: dem_globs.h $
** Revision 1.1  1999/07/06 19:35:00  whuang
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
**
*/
#ifndef DEM_GLOBS_H
#define DEM_GLOBS_H

/* 
* Set up a #define that determines whether the 
* variables should be defined here or just
* externed here
*/
#ifdef DEMOD_TOP
#define DEMGLOBS /**/
#else
#define DEMGLOBS extern
#endif



/* Define the wordlength of the data */
#define WORD_SIZE          (16)
#define ACCUM_GUARD_BITS   (8)


/* Define some common types */  

typedef signed int     SInt;      /* signed integer */


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
typedef Word    Word3;  /*  3 interger bits, Fix<16,16-3>  Sxx.xXXX*/
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
   const t_TseqWord        *tseqBaseAddr; /* Training sequences base addrress */
   const CIR_NUM_TSEQ_BITS_TYPE  cirNumTseqBits;     /* Number of tseq samples to correlate over */
   const Wordm3                  cirNumTseqBitsInv;	 /* 1.0/CIR_NUM_TSEQ_BITS */
   const UChar                   impulseTseqOffset;  /* Position of the first tseq bit for CIR calc. */
   const UChar                   cirLength;     /* Length of the CIR */
   const UChar                   cirStartIndex; /* index of first samples to calc CIR over */
   const Char                    toaMin;        /* Minimum posible time of arrival, relative */
   const UChar                   numBits;       /* Number of bits in the burst less tail/guard bits*/
  /*const UChar                   numDataBits; */  /* Number of data bits in the burst */
                                                /*   NB = 58*2, AB = 36 */
   UChar                         numDataBits; /* const made weight() fail */
   const UChar                   numTailBits;   /* Number of tail bits at the beginning of the burst */


} t_burstConsts; 

/*
** Define structures that have the constants for each burst type
*/ 
#ifdef DEMOD_TOP
t_burstConsts g_ABConsts = { &train_ab[0],
                             AB_CIR_NUM_TSEQ_BITS,
                             (1<<WORD_SIZE)/AB_CIR_NUM_TSEQ_BITS,
                             AB_IMPULSE_TSEQ_OFFSET,
                             AB_CIR_LENGTH,
                             AB_CIR_START_INDEX,
                             AB_TOA_MIN,
                             AB_NUM_BITS_PER_BURST,
                             AB_NUM_ENCRYPT_BITS_PER_BURST,
                             AB_NUM_TAIL_BITS
};   
                                      
t_burstConsts g_NBConsts  = { &train_nb[0][0],
                              NB_CIR_NUM_TSEQ_BITS,
                              (1<<WORD_SIZE)/NB_CIR_NUM_TSEQ_BITS,
                              NB_IMPULSE_TSEQ_OFFSET,
                              NB_CIR_LENGTH,
                              NB_CIR_START_INDEX,
                              NB_TOA_MIN,
                              NB_NUM_BITS_PER_BURST,
                              NB_NUM_ENCRYPT_BITS_PER_BURST,
                              NB_NUM_TAIL_BITS
};
#else
extern t_burstConsts   g_ABConsts; /* Access burst constants */
extern t_burstConsts   g_NBConsts; /* Normal burst constants */
#endif

DEMGLOBS t_burstConsts *g_BConsts_pt;  /* Pointer to the burst constants */

 
/* Define a varaible the is the burst type */
DEMGLOBS  t_chanSel g_burstType; 





#endif
