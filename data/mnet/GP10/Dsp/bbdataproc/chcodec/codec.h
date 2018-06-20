/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
** Filename: codec.h
**
** Description:   
**   This file contains items specific to the GSM Channel CODEC.
** 
*****************************************************************************/ 
#ifndef _CODEC_H_
#define _CODEC_H_  /* include once only */

/****
***Need to uncomment the following # define if reverting optimized .asm routines to original C-code***
***
***#define NOT_ASM_OPTIMIZED
*/

/*
* Channel CODEC specific #defines
*/         
#define CODEC_FRAME_SIZE            456  
#define NUM_CNV_TAIL_BITS           4   

#define NUM_CCH_PARITY_BITS         40 
#define NUM_CCH_CRC_IN_BITS         (NUM_BITS_CCH_FRAME)
#define NUM_CCH_CNV_ENC_IN_BITS     228
#define NUM_CCH_CNV_ENC_OUT_BITS    (2*NUM_CCH_CNV_ENC_IN_BITS)
#define NUM_CCH_CNV_DEC_IN_BITS     (NUM_CCH_CNV_ENC_OUT_BITS)
#define NUM_CCH_CNV_DEC_OUT_BITS    (NUM_CCH_CNV_ENC_IN_BITS)

#define NUM_TCH_PARITY_BITS         3
#define NUM_TCH_CRC_IN_BITS         50 
#define NUM_TCH_CNV_ENC_IN_BITS     189
#define NUM_TCH_CNV_ENC_OUT_BITS    (2*NUM_TCH_CNV_ENC_IN_BITS)
#define NUM_TCH_CNV_DEC_IN_BITS     (NUM_TCH_CNV_ENC_OUT_BITS)
#define NUM_TCH_CNV_DEC_OUT_BITS    (NUM_TCH_CNV_ENC_IN_BITS)

#define TCH_CRC_POLY                0x000D

/*
* EFR Channel CODEC specific #defines
*/
#define EFR_NUM_TCH_PARITY_BITS         8
#define EFR_NUM_TCH_CRC_IN_BITS         65    

#define EFR_NUM_BITS_TCH_CLASSIA        50    /* protected by 3-bit TCH-FS CRC */  
#define EFR_NUM_BITS_TCH_CLASSIB        132   /* protected */
#define EFR_NUM_BITS_TCH_CLASSII        78    /* unprotected */
     
#define EFR_NUM_ENC_REPEAT_BITS         8
 
#define EFR_PRE_CH_CODE_IN_BITS        (NUM_BITS_TCH_FRAME - EFR_NUM_TCH_PARITY_BITS - EFR_NUM_ENC_REPEAT_BITS)

#define EFR_CRC_POLY 0x0171
 
#define NUM_SCH_CRC_IN_BITS         25
#define NUM_SCH_PARITY_BITS         10
#define NUM_SCH_CNV_ENC_IN_BITS     39
#define NUM_SCH_CNV_ENC_OUT_BITS    (2*NUM_SCH_CNV_ENC_IN_BITS)

#define SCH_CRC_POLY                0x0575

#define NUM_RACH_CNV_ENC_IN_BITS    18
#define NUM_RACH_CNV_ENC_OUT_BITS   (2*NUM_RACH_CNV_ENC_IN_BITS)
#define NUM_RACH_CNV_DEC_OUT_BITS   (NUM_RACH_CNV_ENC_IN_BITS)
#define NUM_RACH_CNV_DEC_IN_BITS    (NUM_RACH_CNV_ENC_OUT_BITS)
#define NUM_RACH_CRC_IN_BITS        8
#define NUM_RACH_PARITY_BITS        6

#define RACH_CRC_POLY               0x007b

/*
* GPRS Channel CODEC specific #defines
*/
#define NUM_RACH_GPRS_ADD_BITS   3
#define NUM_PRACH_PUNCTURE_BITS  6
#define NUM_PRACH_CRC_IN_BITS    11
#define GPRS16_CRC_POLY          0x10811

#define NUM_GPRS_CS1_PUNCTURE_BITS   0
#define NUM_GPRS_CS2_PUNCTURE_BITS 132
#define NUM_GPRS_CS3_PUNCTURE_BITS 220

#define NUM_GPRS_CS1_PARITY_BITS 40
#define NUM_GPRS_CS2_PARITY_BITS 16
#define NUM_GPRS_CS3_PARITY_BITS 16
#define NUM_GPRS_CS4_PARITY_BITS 16

#define NUM_GPRS_CS1_CRC_IN_BITS 184                            
#define NUM_GPRS_CS2_CRC_IN_BITS 274                            
#define NUM_GPRS_CS3_CRC_IN_BITS 318                            
#define NUM_GPRS_CS4_CRC_IN_BITS 440

#define NUM_GPRS_CS1_CNV_ENC_IN_BITS 228                           
#define NUM_GPRS_CS2_CNV_ENC_IN_BITS 294                           
#define NUM_GPRS_CS3_CNV_ENC_IN_BITS 338                           
#define NUM_GPRS_CS4_CNV_ENC_IN_BITS 456   
                        
/*
*  uses largest size for buffer allocation
*/
#define MAX_CRC16_TOTAL_BITS  (EFR_NUM_TCH_CRC_IN_BITS + EFR_NUM_TCH_PARITY_BITS)
 
#define MAX_CNV_ENC_IN_BITS         (NUM_CCH_CNV_ENC_IN_BITS)
#define MAX_CNV_ENC_OUT_BITS        (NUM_CCH_CNV_ENC_OUT_BITS)
#define MAX_CNV_DEC_IN_BITS         (NUM_CCH_CNV_DEC_IN_BITS)
#define MAX_CNV_DEC_OUT_BITS        (NUM_CCH_CNV_DEC_OUT_BITS)
#define MAX_PARITY_BITS             (NUM_CCH_PARITY_BITS)

#define BSIC_LENGTH                 (NUM_RACH_PARITY_BITS)

#define NUM_BITS_TCH_CLASSI \
 (NUM_TCH_CNV_ENC_IN_BITS-NUM_TCH_PARITY_BITS-NUM_CNV_TAIL_BITS)
#define NUM_BITS_TCH_CLASSII  (NUM_BITS_TCH_FRAME-NUM_BITS_TCH_CLASSI)
#define TCH_REORDER_MAX_INDEX \
 (NUM_TCH_CNV_ENC_IN_BITS-NUM_CNV_TAIL_BITS-1)
#define TCH_START_OF_TAIL \
 (NUM_TCH_CNV_ENC_IN_BITS-NUM_CNV_TAIL_BITS)

#define MAX_STEALING_FLAGS          8
#define STEALING_FLAG_THRESH        4

/*
* Fire Decoder Processing States
*/
#define NO_ERROR_CORRECT        0      /* no error  */
#define DO_ERROR_CORRECT1       1      /* intermediate error processing */
#define DO_ERROR_CORRECT2       2      /* begin error processing */        

#ifdef ALLOCATE_MEMORY_INTERNAL /*-------------------------------------------*/ 

/*
* Preliminary channel encoder (for EFR TCH CRC calculation) index mapping table
*/
const Uint8 tch_Efr_CRC_calc_LookUpTbl[EFR_NUM_TCH_CRC_IN_BITS] =
{ 
   /*
   * taken from GSM 05.03 (v5.5.0) section 3.1.1.1
   */
    39-1,  40-1,  41-1,  42-1,  43-1,  44-1,  48-1,  87-1,  45-1,   2-1,
     3-1,   8-1,  10-1,  18-1,  19-1,  24-1,  46-1,  47-1, 142-1, 143-1,
   144-1, 145-1, 146-1, 147-1,  92-1,  93-1, 195-1, 196-1,  98-1, 137-1, 
   148-1,  94-1, 197-1, 149-1, 150-1,  95-1, 198-1,   4-1,   5-1,  11-1,
    12-1,  16-1,   9-1,   6-1,   7-1,  13-1,  17-1,  20-1,  96-1, 199-1, 
     1-1,  14-1,  15-1,  21-1,  25-1,  26-1,  28-1, 151-1, 201-1, 190-1, 
   240-1,  88-1, 138-1, 191-1, 241-1
};                           

/*
* GPRS Prach Puncturing Consts
*/
const Uint8 GPRSPrachPunctureBits[NUM_PRACH_PUNCTURE_BITS] = {0,2,5,37,39,41};                  

#else  /*--------------------------------------------------------------------*/

extern const Uint8 tch_Efr_CRC_calc_LookUpTbl[EFR_NUM_TCH_CRC_IN_BITS];
extern const Uint8 GPRSPrachPunctureBits[NUM_PRACH_PUNCTURE_BITS];                  

#endif /*--------------------------------------------------------------------*/

/*
* Define the "non top level" function prototypes
*/
void  cnvEncoder2 (const Char *u, Char *c, UInt numBits);
Short cnvDecoder2 (const Char *c, Char *u, UInt numBits);

void crcEncoder16(Int8 *inputPtr, Int8 *parityPtr, UInt poly,
                  Int8 numParityBits, Uint16 numSigBits, Uint16 bitInvert); 
Bool crcDecoder16(Char *inputPtr, Char *parityPtr, UInt poly,
                  Int8 numParityBits, Uint16 numSigBits, Uint16 bitInvert);

Bool fireDecoderCCHDetect  (Uint8 rfCh, Uint8 timeSlot, t_chanSel chanSel, Char *inputPtr);
Bool fireDecoderCCHCorrect (Char *inputPtr); 

void crcEncoderCCH (Char *inputPtr, Char *parPtr);
Bool crcDecoderCCH (Char *inputPtr, Char *parityPtr);

void crcEncoderTCH (Char *inputPtr, Char *parPtr);
Bool crcDecoderTCH (Char *inputPtr, Char *parityPtr);

Bool crcDecoderEfrTCH(Char *inputPtr, Char *parityPtr);

void crcEncoderRACH (Char *inputPtr, Char *parPtr);
Bool crcDecoderRACH (Char *inputPtr, Char *parityPtr);

void crcEncoderSCH(Char *inputPtr, Char *parPtr);

void blockRectInterleaver(Char *input, Int *burstIndex,
	 Char output[NUM_BURSTS_PER_CCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST]);

UInt blockRectDeinterleaver(Char input[NUM_BURSTS_PER_CCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST],
			    Int *burstIndex, Char *output);

void blockDiagInterleaver8( Char* input, Bool stealFlag, Int *burstIndex,
                Char output[NUM_BURSTS_PER_TCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST]);

UInt blockDiagDeinterleaver8( Char input[NUM_BURSTS_PER_TCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST],
                Int *burstIndex, Char* output);

void deinterleaverLoader(t_chanSel chanSel, UInt TN, Bool frameBoundary);

void burstLoader(t_chanSel chanSel, UInt TN, Bool frameBoundary);

void loadInt4Deep(Char CCHIntOutData[NUM_BURSTS_PER_CCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST], 
                  UInt TN);

void sbjEncoder(Char* inputPtr, t_vocAlgo vocoAlgo);
Bool sbjDecoder(Char* inputPtr, t_vocAlgo vocoAlgo, Uint8 timeSlot);

Bool detectBFI(t_chanSel chanSel, Bool parityError, UInt decoderMetric,
               UInt totalErrors, Bool *perceptualMask);
               
void vitgsm(int n, short old[], short new_s[], 
            short trans[], short m[], char sd[], char so[]);
            
void punctureGPRS(Char *in, Char *out, const codeScheme);            
void restorePunctureGPRS(Char *in, Char *out, const codeScheme);

UInt shiftZeros(UInt regOutSave);
    

            
#endif  /* end of include once only */


               

  