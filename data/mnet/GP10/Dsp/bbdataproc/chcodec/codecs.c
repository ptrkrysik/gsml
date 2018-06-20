/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
** Filename: codecs.c
**
** Description:   
**   This file contains low level encoding and decoding routines in support 
**   of GSM channel coding and decoding.
**
**   Note: Old revision notes are stored at the bottom of this file.
**
*****************************************************************************/
#include "gsmdata.h" 
#include "bbdata.h" 
#include "bbproto.h"  
#include "codec.h"
#include "dsprotyp.h"  

/*
* Align these metric arrays on 32-bit boundaries so we can accessed as Int.
* These are used by the cnvEncoder2 function below. Declare here because
* DATA_ALIGN is not valid on static arrays defined inside the function.
*/ 
#pragma DATA_ALIGN(oldMets, 4);
#pragma DATA_ALIGN(newMets, 4);
static Short oldMets[16];
static Short newMets[16];
   
/*
* Temporary variables for debug?
*/
#pragma DATA_SECTION(perfectCchFrames, ".extData");
Int32 perfectCchFrames = 0;
#pragma DATA_SECTION(correctedCchFrames, ".extData");
Int32 correctedCchFrames = 0;
#pragma DATA_SECTION(uncorrectedCchFrames, ".extData");
Int32 uncorrectedCchFrames = 0;
#pragma DATA_SECTION(preUncorrectedCchFrames, ".extData");
Int32 preUncorrectedCchFrames = 0;
#pragma DATA_SECTION(badCchBits, ".extData");
Int32 badCchBits = 0;
#pragma DATA_SECTION(correctedCchBits, ".extData");
Int32 correctedCchBits = 0;

/*****************************************************************************
** ROUTINE NAME: codecInit
*****************************************************************************/
void codecInit(void)
{
   g_ULCCHFireData.fireStatus = NO_ERROR_CORRECT;
}

/*****************************************************************************
** ROUTINE NAME: cnvEncoder2
**
** Description: 
**   Rate 1/2 convolutional encoder routine used in GSM. Uses the following
**   generator polynimials.
**
**      g0 = 1 + D3 + D4
**      g1 = 1 + D + D3 + D4
**
**   The order of the bits within the arrays is consistent the GSM 
**   specification format, in that array index k stores u(k)/c(k).         
**
** Method:
**   Since the modulo operation is very expensive and is a power of 2,
**   a bitwise AND operation will be used to implement the mod 2
**   function.  The following two lines of code are equivalent (for
**   MODULO_2_MASK = 0x0001):
** 
**       c[7] = (u[3] + u[2] + u[0]) % 2;
**       c[7] = (u[3] + u[2] + u[0]) & MODULO_2_MASK;   
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**    u - input bit array
**    c - output but array
**    numBits - number of input bits to encode
**
** Returns: 
**    None
**
** Data Store  I/O: 
**
*****************************************************************************/
void cnvEncoder2(const Char *u,Char *c, UInt numBits)
{
   UInt i;

   c[0] = u[0] & MODULO_2_MASK; 
   c[1] = u[0] & MODULO_2_MASK;
   c[2] = u[1] & MODULO_2_MASK; 
   c[3] = (u[1] + u[0]) & MODULO_2_MASK;
   c[4] = u[2] & MODULO_2_MASK; 
   c[5] = (u[2] + u[1]) & MODULO_2_MASK;
   c[6] = (u[3] + u[0]) & MODULO_2_MASK; 
   c[7] = (u[3] + u[2] + u[0]) & MODULO_2_MASK;

   for(i=4; i < numBits; i++) 
   {
	  c[2*i]    = (u[i] +          u[i-3] + u[i-4]) & MODULO_2_MASK;
	  c[2*i+1]  = (u[i] + u[i-1] + u[i-3] + u[i-4]) & MODULO_2_MASK;
   }
}

/*****************************************************************************
** ROUTINE NAME: cnvDecoder2
**
** Description: 
**   Rate 1/2 convolutional decoder routine used in GSM. Uses the following
**   generator polynimials.
**
**      g0 = 1 + D3 + D4
**      g1 = 1 + D + D3 + D4   
**
**   K (constraint length) = 5                                           
**   2^(K-1) = 16 (number of states required) 
**
**   The order of the bits within the arrays is consistent the GSM 
**   specification format, in that array index k stores u(k)/c(k).   
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   c - recieved coded bit array
**   u - output decoded bit array
**   numBits - Number of bits to decode
**
** Returns: 
**
** Data Store  I/O: 
**
*****************************************************************************/
Short cnvDecoder2(const Char *c, Char *u, UInt numBitsOut)
{
   Int oldMet;
   Short *newMetsPtr, *oldMetsPtr, *tmpPtr;
   Short branchMet;             /* branch metric */
   Short val1, val2, val3, val4;
   Short i,j,l;
                   
   Int16 m[8];

#ifdef NOT_ASM_OPTIMIZED
#define INITVAL 0xEC78
   Char stateTranTbl[228][16];  /* state transition table, non-opt */
   /*
   * Need to replace the above line of C-Code with the following commented out line of code when..
   * ..porting the non-optimized C-code to another DSP platform...the current on-chip data ram space
   * ..isn't big enough to support the larger size array
   * 
   *     Char stateTranTbl[NUM_GPRS_CS3_CNV_ENC_IN_BITS][16];  /* state transition table, non-opt */
   */
#else
#define INITVAL 0x0000
   Short stateTranTbl[NUM_GPRS_CS3_CNV_ENC_IN_BITS];     /* state transition table, opt version */
#endif

   oldMetsPtr = &oldMets[0];
   newMetsPtr = &newMets[0];
   
   /* 
   * Old metrics initialized with initial values 
   * hpines...5/28/99..all states initially = 0...undo "start at 1 mod"
   */
/* *(Int *)&oldMets[0] = INITVAL;  */
   for(i=0; i<8; i++)                  /*** what is the intent here? start at 1 okay? ***/
   {
      *(Int *)&oldMets[2*i] = (INITVAL<<16) | INITVAL;
   };
   
   /*---------------------- Start of Non-Optimized Version --------------------------*/
#ifdef NOT_ASM_OPTIMIZED

   /*
   * Perform outer loop numBitsOut times, 2 input bits are read for each iteration.
   * The index j indexes the input signal c.
   */
   for(j=0; j<numBitsOut; j++)
   {              
      /*
      * Perform inner loop num_states/2 times, one iteration for each butterfly in 
      * the trellis.
      *   i indexes the bufferfly and states
      *   l indexes the expected encoder output for each butterfly
      *   j indexes the recieved bits c
      */
      for(i=0,l=0; i<8; i++,l+=2)
      {
         /*
         * Compute the branch metric branchMet. This is a measure of similarity of the
         * recieved bits to the expected bits. Note that for any butterfly, the output encoded 
         * data is as follows. These arrays represent the coded data output from state transitions
         * on the rate 1/2 encoder trellis diagram. The first bit pair represents 
         * the data for the first butterfly, and so on, for all 8 butterflies of 
         * the rate 1/2 trellis. The 1st set is used with the demod if the demod 
         * delivers sign inverted soft decisions to the channel decoder.  Other-
         * wise, the 2nd set is used.
         *
         *      expData = {1,1, -1,-1, 1,1, -1,-1, 1,-1, -1,1, 1,-1, -1,1};
         * -OR- expData = {-1,-1, 1,1, -1,-1, 1,1, -1,1, 1,-1, -1,1, 1,-1};
         *
         *   -branchMet[i] = c[2*j]*(-expData[l]) + c[2*j+1]*(-expData[l+1]);
         */
         branchMet = c[2*j] + ((l&0x8) ?  -c[2*j+1] : c[2*j+1]);
         branchMet = (l&0x2) ? -branchMet : branchMet;

         /*
         * Eliminate the smallest path, retain the survior path. stateTranTbl[k][i]
         * stores the LSB of the state number. This information determines
         * from which of the two previous states (in the butterfly) you transitioned 
         * to the current state. This information is used in the backtracking.
         */
         oldMet = *(Int *)&oldMetsPtr[l];
         
         /*
         * Compute the cumulative path metrics up to the current state
         */
#ifdef _LITTLE_ENDIAN
         val1 = (Short)(oldMet) + branchMet;                
         val2 = (Short)(oldMet>>16) - branchMet;
         val3 = (Short)(oldMet) - branchMet;
         val4 = (Short)(oldMet>>16) + branchMet;
#else
         val1 = (Short)(oldMet>>16) + branchMet;                
         val2 = (Short)(oldMet) - branchMet;
         val3 = (Short)(oldMet>>16) - branchMet;
         val4 = (Short)(oldMet) + branchMet;
#endif
         newMetsPtr[i]   = (stateTranTbl[j][i]=(val2>val1)) ? val2 : val1;
         newMetsPtr[i+8] = (stateTranTbl[j][i+8]=(val4>val3)) ? val4 : val3;
      }
      
      /*
      * Metrics update
      */
      tmpPtr = oldMetsPtr;
      oldMetsPtr = newMetsPtr;
      newMetsPtr = tmpPtr;
   } 

   /*
   * Write the 4 known tail bits to the end of the buffer
   */
   for(i=numBitsOut-4; i<numBitsOut; i++)
   {   
      u[i] = 0;
   }

   /*
   * Perform the backtracking and decisions. numBitsOut-4 bits will be written
   * to u[0..numBitsOut-5]. u[numBitsOut-4..numBitsOut] is not written since
   * this is "0" tail bits and need not be written. It is assummed that these
   * locations in the output buffer have already been initialized.
   */
   for(i=numBitsOut-1,j=0; i>=4; i--)
   {                               
      /*
      * stateTranTbl[i][j] is actually the LSB of the state number. The observation here 
      * is that this LSB, delayed by 4 units, will appear as the output. 
      */
      u[i-4] = stateTranTbl[i][j];

      /*
      * Decide which of the two states to backtrack to based on the state LSB 
      * (stateTranTbl[i][j])
      */
      j= ((j<<1) + stateTranTbl[i][j]) & 0x0F;
   }
    oldMets[0] -= INITVAL;   /* restore accum metric baseline to 0 */

   /*---------------- Start of Assembly Optimized Version ---------------------*/
#else    
                       
	vitgsm((int)numBitsOut,
		    (short*)(&oldMets[0]),
		    (short*)(&newMets[0]), 
		             &stateTranTbl[0],
		             &m[0],
		    (char*) (&c[0]), 
		             &u[0]);
	
/*
*  7/5/00...swap oldMets/newMets pointers if odd # input samples
*/
	if(numBitsOut & 1)	
	 oldMets[0] = newMets[0] - INITVAL;   /* restore accum metric baseline to 0 */
	else
	 oldMets[0] -= INITVAL;   /* restore accum metric baseline to 0 */

#endif

   /*--------------------------------------------------------------------------*/

   return(oldMets[0]);                  
}


/*****************************************************************************
** ROUTINE NAME: crcEncoder16
**
** Description: 
**   Performs the Binary Cyclic Encoding (Block Encoding) used in GSM. 
**   The input data and parity bit array used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the CCH block coding as described in GSM 05.03.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**
** Data Store  I/O: 
**
*****************************************************************************/
void crcEncoder16(Int8 *inputPtr, Int8 *parityPtr, UInt poly,
                  Int8 numParityBits, Uint16 numSigBits, Uint16 bitInvert) 
{
   UInt i;
   UInt regIn;
   UInt regOut;
   UInt result;
   UInt bit;
   UInt shiftedBit;
   Uint16 numBits;
   Char output;
   Char input[NUM_GPRS_CS4_CNV_ENC_IN_BITS];

   numBits = numSigBits + numParityBits;
   /*
   * Multiply the input data polynomial by D^numParityBits. This basically
   * amounts to appending numParityBits "0s" to the tail of the vector. 
   */
   for(i=0; i<numSigBits; i++)
   {
      input[i] = inputPtr[i];
   }

   for(i=numSigBits; i<numBits; i++)
   {
      input[i] = 0;
   }

#ifdef NOT_ASM_OPTIMIZED
 
   /*
   * Init the register outputs to zero
   */
   regOut = 0x0000;

   /*
   * Or in the first data bit
   */
   bit = input[0] << numParityBits;
   regOut |= bit;

   /*
   * Loop over all bits in the input vector ( D^N d(D), N = numParityBits )
   */
   for(i=0; i<numSigBits + numParityBits; i++)
   {
      output = regOut & 0x0001;

      result = poly * output;

      regIn = result ^ regOut;

      regOut = regIn >> 1;

      bit = input[i+1] << numParityBits;

      regOut |= bit;
   }

#else
   /*
   * Use Optimized Assembly routine to do the encoding... 
   * compute shifted input bit mask
   */
   shiftedBit = 1 << numParityBits;

   /*
   * Loop over all bits in the input vector ( D^N d(D), N = numParityBits )
   */

   regOut = crcShiftReg(&input[0],numBits,shiftedBit,poly);

#endif

   /*
   * Return inverted parity bits, modulo 2 added with bsic.
   * Note that the bsic is stored in the reverse order relative to 
   * the parity bits.
   */
   regOut ^= bitInvert;

   for(i=0; i<numParityBits; i++)
   {
      bit = (regOut >> i) & 0x0001;
      parityPtr[i] = (Char)(bit);
   }
}

/*****************************************************************************
** ROUTINE NAME: crcDecoder16
**
** Description: 
**   Performs the Binary Cyclic Encoding (Block Encoding) used in GSM. 
**   The input data and parity bit array used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the CCH block coding as described in GSM 05.03.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**
** Data Store  I/O: 
**
*****************************************************************************/
Bool crcDecoder16(Char *inputPtr, Char *parityPtr, UInt poly,
                  Int8 numParityBits, Uint16 numSigBits, Uint16 bitInvert)
{
   Uint8 i;
   Int8  genParityBits[16];
   Bool  parityError = 0;
     
   /*
   * Perform the crc encoding to calculate the parity based on the recieved
   * bits. The calculated parity is stored in calParity array.
   */
   crcEncoder16(inputPtr, genParityBits, poly, numParityBits, numSigBits, bitInvert);
   
   /*
   * Compare recieved parity to calculated parity
   */
   for(i=0; i<numParityBits; i++)
   {
      if(genParityBits[i] != parityPtr[i])
      {
         parityError = TRUE;
         break;
      }
   }
   
   return(parityError);
}

/*****************************************************************************
** ROUTINE NAME: crcEncoderCCH
**
** Description: 
**   Performs the CCH Binary Cyclic Encoding (Block Encoding) used in GSM. 
**   The input data and parity bit array used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the CCH block coding as described in GSM 05.03.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**
** Data Store  I/O: 
**
*****************************************************************************/
void crcEncoderCCH(Char *inputPtr, Char *parityPtr) 
{
   UInt i;
   UInt regIn[2];
   UInt regOut[2];
   UInt result[2];
   const UInt poly[2] = {0x00824001, 0x00000120};
   UInt bit;
   UShort output;
   Char input[NUM_CCH_CRC_IN_BITS+NUM_CCH_PARITY_BITS];
   Uint16 numBits;
   UInt shiftedBit;

   numBits = NUM_CCH_CRC_IN_BITS+NUM_CCH_PARITY_BITS;


   /*
   * Multiply the input data polynomial by D^numParityBits. This basically
   * amounts to appending numParityBits "0s" to the tail of the vector. 
   */
   for(i=0; i<NUM_CCH_CRC_IN_BITS; i++)
   {
      input[i] = inputPtr[i];
   }

   for(i=NUM_CCH_CRC_IN_BITS; i<NUM_CCH_CRC_IN_BITS + NUM_CCH_PARITY_BITS; i++)
   {
	  input[i] = 0;
   }
 
#ifdef NOT_ASM_OPTIMIZED
 
   /*
   * Init the register outputs to zero
   */
   regOut[0] = 0x00000000;
   regOut[1] = 0x00000000;

   /*
   * Or in the first data bit
   */
   bit = input[0] << (NUM_CCH_PARITY_BITS-32);
   regOut[1] |= bit;

   /*
   * Loop over all bits in the input vector ( D^N d(D), N = numParityBits )
   */
   for(i=0; i<NUM_CCH_CRC_IN_BITS + NUM_CCH_PARITY_BITS; i++)
   {
      output = regOut[0] & 0x00000001;

	  result[0] = poly[0] * output;
	  result[1] = poly[1] * output;

	  regIn[0] = result[0] ^ regOut[0];
	  regIn[1] = result[1] ^ regOut[1];

	  regOut[0] = (regIn[0] >> 1);
	  bit = (regIn[1]) << 31;
	  regOut[0] |= bit;

	  regOut[1] = (regIn[1] >> 1);
	  bit = (input[i+1]) << (NUM_CCH_PARITY_BITS-32);
	  regOut[1] |= bit;
   }

#else

   /*
   * Use Optimized Assembly routine to do the encoding... 
   * compute shifted input bit mask
   */
   shiftedBit = 1 << (NUM_CCH_PARITY_BITS-32);

   /*
   * Loop over all bits in the input vector ( D^N d(D), N = numParityBits )
   */

   crcLongShiftReg(&input[0],shiftedBit,numBits,poly[0],poly[1],&regOut[0]);

#endif

   /*
   * Return inverted parity bits
   */
   regOut[0] ^= 0xffffffff;
   regOut[1] ^= 0xffffffff;

   for(i=0; i<32; i++)
   {
	  bit = (regOut[0]>>i) & 0x00000001;
	  parityPtr[i] = (Char)bit;
   }

   for(i=0; i<NUM_CCH_PARITY_BITS-32; i++)
   {
	  bit = (regOut[1]>>i) & 0x00000001;
	  parityPtr[32+i] = (Char)bit;
   }

}

/*****************************************************************************
** ROUTINE NAME: ulCCHFireDecoder
**
** Description: 
**  Looks for correctable FireCoded data errors using method described in patent #CPOL 89101
**
*****************************************************************************/
void ulCCHFireDecoder(void) 
{
   UInt i;
   Int8     rfChFire;
   Uint8    subChan;	
   Uint8    timeSlotFire,chType;	
   Bool error;
	          
   error = fireDecoderCCHCorrect((Char*)(&g_ULCCHFireData.data));
   /*
   * pack frame and finish bookkeeping if fire decoder processing all done and all
   * errors are corrected
   */     
   if(! error )
   {
      rfChFire = g_ULCCHFireData.rfCh;  
      timeSlotFire = g_ULCCHFireData.timeslot;  
      subChan = g_ULCCHFireData.subChan;  
      chType = g_ULCCHFireData.chType;  

	  /*
	  * move corrected frame to CCCH decoder buffer
	  */
      for(i=0; i<NUM_BITS_CCH_FRAME; i++)
	  {
         g_ULCCHData.data[i] = g_ULCCHFireData.data[i];
	  }
	  /*
	  *  set decoder status and flags
	  */
	  g_ULCCHData.ulcchword0.bitval.fireStatus = 0;
      g_ULCCHData.ulcchword0.bitval.bufferValid = TRUE;
      ulSyncMsgProcess(chType, subChan, timeSlotFire, rfChFire); 
   }            
}
          
/*****************************************************************************
** ROUTINE NAME:  fireDecoderCCHDetect
**
** Description: 
**  Looks for correctable FireCoded data errors using method described in patent #CPOL 89101
**
** Method:
**  Reference method described in patent #CPOL 89101
**
** Parameter I/O: 
**   inputPtr - indexes the input bit array
**   g_ULCCHFireData - Data structure used for fire decoder processing
** Returns: 
**   Error - Flag indicating detection of a correctable error
**
** Data Store  I/O: 
**
*****************************************************************************/
Bool fireDecoderCCHDetect(Uint8 rfCh, Uint8 timeSlot, t_chanSel chanSel, Char *inputPtr) 
{
   t_TNInfo       *ts; 
   UInt k,kk,kkk;
   UInt i,j,index;
   UInt regIn,regIn1;
   UInt regOut,regOut1,regOutSave;
   UInt result,result1,result2;
   const UInt poly1 = 0x24001;  
   const UInt poly = 0x800001; 
   const Int mult1 = 0x7fffd;  
   const Int mult2 = 0x7fffc;
   Int32 lambda1, lambda2;  
   UInt bit,bit1;
   Long resulta, resultb;
   UShort output,output1;
   Bool Error,noCorrectFrame;
   Char input[NUM_CCH_CRC_IN_BITS+NUM_CCH_PARITY_BITS+2];

   noCorrectFrame = 0;

   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];

   g_ULCCHFireData.rfCh = rfCh;  
   g_ULCCHFireData.timeslot = timeSlot;  
   g_ULCCHFireData.chType = chanSel;  

   switch(ts->groupNum)
   {
   case GSM_GROUP_5:
	   if(chanSel == SDCCH4)
          g_ULCCHFireData.subChan = ts->u.group5.ulSdcchSubCh;
	   else
          g_ULCCHFireData.subChan = ts->u.group5.ulSacchSubCh;
	   break;

   default:
       g_ULCCHFireData.subChan = 0; 
	   break;
   }

#define RANDOM_TEST 0
#if RANDOM_TEST == 2
	      k = (rand() & 0x7f);
	      kk = (rand() & 0x1f);
	      kkk = (rand() & 0x7) + 5;
            result2 = k + kk;
           for(i=0; i < kkk; i++)
            {
             inputPtr[i+k+kk] ^= 1;  
	       badCchBits++;
            }          
#endif   
 
   /*
   * Multiply the input data polynomial by D^numParityBits. This basically
   * amounts to appending numParityBits "0s" to the tail of the vector. 
   */
   for(i=0; i<NUM_CCH_CRC_IN_BITS; i++)
   {
      input[i] = inputPtr[i];
   }

/* invert parity bits to compensate for encoder */

   for(i=0; i<NUM_CCH_PARITY_BITS; i++)
   {
	  input[i+NUM_CCH_CRC_IN_BITS] = inputPtr[i+NUM_CCH_CRC_IN_BITS] ^ 1 ;
   }
 
/* add 1 shortened 0 bit */

   input[NUM_CCH_CRC_IN_BITS+NUM_CCH_PARITY_BITS] = 0 ;

  /*
   * Init the register outputs to zero
   */
   regOut = 0x00000000;
   regOut1 = 0x00000000;

   /*
   * Or in the first data bit
   */

   bit = input[0] << 23;
   regOut |= bit;
   bit1 = input[0] << 17;
   regOut1 |= bit1;

   /*
   * Loop over all bits in the input vector ( D^N d(D), N = numParityBits )
   */
   for(i=0; i<(NUM_CCH_CRC_IN_BITS + NUM_CCH_PARITY_BITS ); i++)
   {

      output = regOut & 0x0001;
      output1 = regOut1 & 0x0001;
      result = poly * output;
      result1 = poly1 * output1;
      regIn = result ^ regOut;
      regIn1 = result1 ^ regOut1;
      regOut = regIn >> 1;
      regOut1 = regIn1 >> 1;
      bit = input[i+1] << 23;
      bit1 = input[i+1] << 17;
      regOut |= bit;
      regOut1 |= bit1;
  }
/* break here, i.e. no errors detected, if both syndrome registers = 0 */
   if((regOut == 0) && (regOut1 == 0))
   {
    noCorrectFrame = TRUE;
    perfectCchFrames++;
	Error = 0;
   } 
   else
 /* break here if error pattern > 12 bits long */
   {
	Error = TRUE;
    lambda1 = 0;
    regOutSave = regOut;
    for(j=0; j < 23; j++)
    {
    if((regOutSave & 0xfff) != 0x800)
     {
      output = regOutSave & 0x0001;
      result = poly * output;
      regIn = result ^ regOutSave;
      regOutSave = regIn >> 1;
     }
    else
     {
      lambda1 = j;
      break;
     }
    }
    if((regOutSave & 0xfff) == 0x800)
     lambda1 = 23;

 /*  no can do error correction */
    if(lambda1 == 0)
    {
     noCorrectFrame = TRUE;
     preUncorrectedCchFrames++;
	  sendDiagMsg(0x63, rfCh, timeSlot, 0, &rfCh); /* 5th arg to satisy prototype */
    }

   }
/* correctable error detected...buffer data for later correction */ 
   if(noCorrectFrame != TRUE)
   { 
     g_ULCCHFireData.fireStatus = DO_ERROR_CORRECT2;
     g_ULCCHFireData.errorPatternReg = regOut;
     g_ULCCHFireData.errorLocationReg = regOut1;
     for(i=0; i<NUM_BITS_CCH_FRAME; i++)
      {
        g_ULCCHFireData.data[i] = inputPtr[i];
      }
   } 
  return(Error);
}

/*****************************************************************************
** ROUTINE NAME:  fireDecoderCCHCorrect
**
** Description: 
**  Attempts to correct FireCoded data errors using method described in patent #CPOL 89101
**  Processing performed over two consecutive frames of inactive radio channel timeslots  
** Parameter I/O: 
**   inputPtr - indexes the input bit array
**   g_ULCCHFireData - Data structure used for fire decoder processing
** Returns:
**   Error - flag to indicate whether or not error was correctable
**   corrected bits in input bit array 
*****************************************************************************/
Bool fireDecoderCCHCorrect(Char *inputPtr) 
{
   UInt k,kk,kkk;
   UInt i,j,index;
   UInt regIn,regIn1;
   UInt regOut,regOut1,regOutSave;
   UInt result,result1,result2;
   const UInt poly1 = 0x24001;  
   const UInt poly = 0x800001; 
   const Int mult1 = 0x7fffd;  
   const Int mult2 = 0x7fffc;
   Int32 lambda1, lambda2;  
   UInt bit,bit1;
   Long resulta, resultb;
   UShort output,output1;
   Bool Error,noCorrectFrame;
   Char input[NUM_CCH_CRC_IN_BITS+NUM_CCH_PARITY_BITS+2];
   UInt fireDecoderStatus;

   Int8     rfChFire;
   Uint8    timeSlotFire;	
   t_TNInfo       *ts;          
   t_SACCHInfo    *sacchInfo;
	          

   /*
   *   Return value set as TRUE initially
   */
   Error = TRUE;

/* shift in shortened bits */
/* new 2/28/00...break into two calls */

   fireDecoderStatus = g_ULCCHFireData.fireStatus ;
   switch(fireDecoderStatus)
   {
      case DO_ERROR_CORRECT2:
      {
#ifdef NOT_ASM_OPTIMIZED
        regOut1 = g_ULCCHFireData.errorLocationReg;
        for(i=0; i < (65424 - 1); i++) 
        {
         output1 = regOut1 & 0x0001;
         result1 = poly1 * output1;
         regIn1 = result1 ^ regOut1;
         regOut1 = regIn1 >> 1;
        }
#else
        regOutSave = g_ULCCHFireData.errorLocationReg;
        regOut1 = shiftZeros(regOutSave);
#endif

        g_ULCCHFireData.errorLocationReg = regOut1;
        fireDecoderStatus = DO_ERROR_CORRECT1;
        break;
      }

      case DO_ERROR_CORRECT1:
      {
#ifdef NOT_ASM_OPTIMIZED
        regOut1 = g_ULCCHFireData.errorLocationReg;
        for(i=0; i < (65423 - 1); i++) 
        {
         output1 = regOut1 & 0x0001;
         result1 = poly1 * output1;
         regIn1 = result1 ^ regOut1;
         regOut1 = regIn1 >> 1;
        }
#else
       regOutSave = g_ULCCHFireData.errorLocationReg;
       regOut1 = shiftZeros(regOutSave);
#endif

      fireDecoderStatus = NO_ERROR_CORRECT; /* restore fire decoder status */
      regOut = g_ULCCHFireData.errorPatternReg;
      for(i=0; i < 22; i++) 
       {
        output = regOut & 0x0001;
        result = poly * output;
        regIn = result ^ regOut;
        regOut = regIn >> 1;
       }
       for(j=0; j < 23; j++)
       {
        if((regOut & 0xfff) != 0x800)
        {
         output = regOut & 0x0001;
         result = poly * output;
         regIn = result ^ regOut;
         regOut = regIn >> 1;
        }
        else
        {
         lambda1 = j;
         regOut = regOut >> 6;
         break;
        }
       }

       lambda2 = 0;
       for(i=0; i < 239; i++)
       {
        if((regOut1 & 0x1fffe) != regOut)
        {
         output1 = regOut1 & 0x0001;
         result1 = poly1 * output1;
         regIn1 = result1 ^ regOut1;
         regOut1 = regIn1 >> 1;
        }
       else
        {
         lambda2 = i;
         break;
        }
      }
      if(lambda2 == 0)
      {
 /*  no can do error correction */
         uncorrectedCchFrames++;
/* new 7/6/00..correct accum fer diag for fire decoder failure to correct */ 
         rfChFire = g_ULCCHFireData.rfCh;  
         timeSlotFire = g_ULCCHFireData.timeslot;  
         ts = & g_BBInfo[rfChFire].TNInfo[timeSlotFire];
         sacchInfo = & ts->u.group1.sacch; 
         sacchInfo->frameErrorAccum += 1;
      }
/*  do error correction */
      else
      {    
         Error = FALSE; 
         correctedCchFrames++;
         if(lambda2 < 197)
		 {
            index = lambda2 - 0xd;
            bit = 0x20;  
            for(i=0; i < 12; i++)
			{
               if((regOut1 & bit) != 0)
			   {
                  inputPtr[i+index] ^= 1;
                  correctedCchBits++;
			   }
               bit = bit << 1;  
			}
		 }
      }
        
	  sendDiagMsg(0x64, g_ULCCHFireData.rfCh, g_ULCCHFireData.timeslot, 1, & Error);

     } /* end case */

   } /* end switch */
   g_ULCCHFireData.fireStatus  = fireDecoderStatus;
  return(Error);
}

/*****************************************************************************
** ROUTINE NAME: crcDecoderCCH
**
** Description: 
**   Performs the Binary Cyclic Decoding (Block Decoding) used in GSM. 
**   The input data and parity bit arrays used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the block decoding as described in GSM 05.03. 
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit+parity array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**   parityError - TRUE indicates that a parity error has been detected
**
** Data Store  I/O: 
**
*****************************************************************************/
Bool crcDecoderCCH(Char *inputPtr, Char *parityPtr)
{
   UInt i;
   Bool parityError;

   parityError = 0;  
   crcEncoderCCH(inputPtr, parityPtr); 

   /*
   * Compare recieved parity to calculated parity
   */
   for(i=0; i<NUM_CCH_PARITY_BITS; i++)
   {
      if(inputPtr[i+NUM_CCH_CRC_IN_BITS] != parityPtr[i])
      {
         parityError = TRUE;
      }
   }
   return(parityError);
}                      

#define COMBINED_CRC_16          
#ifndef COMBINED_CRC_16

The following code is not used !!!!!!!!!!!!!!!!!!!!!!!!

/*****************************************************************************
** ROUTINE NAME: crcEncoderEfrTCH
**
** Description:
**   Performs the TCH Binary Cyclic Encoding (Block Encoding) used in GSM. 
**   The input data and parity bit arrays used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the block coding as described in GSM 05.03.
** 
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**
** Data Store  I/O: 
**
*****************************************************************************/
void crcEncoderEfrTCH(Char *inputPtr, Char *parityPtr) 
{
   UInt i;
   UShort regIn;
   UShort regOut;   
   UShort result;
   const UShort poly = 0x0171;     /* D^0 .. D^N */
   UShort bit;
   Char output;
   Char input[EFR_PRE_CH_CODE_IN_BITS + EFR_NUM_TCH_PARITY_BITS];
  
   /*  
              __________                _________          ____________
      s(k)   |          |  b(k), p(k)  |         |  w(k)  |            |  
      -----> |   CRC    |------------->| Mapping |------->| Reordering |---->
             |__________|              |_________|        |____________|
     244 bits             252 bits                260 bits                  

   *  where:
   *
   *   s(k) = inputPtr[i]
   *   b(k) = input[i]
   *   p(k) = parityPtr[i]
   *   w(k) = input[i]       ... reused; to be sent downstream to RTP module in host 
 
   *  An 8-bit CRC is used for error detection.  These 8 parity bits 
   *  (bits w253-w260) are generated by the cyclic polynomial: 
   *  g(D) = D^8 + D^4 + D^3 + D^2 + 1 from the 65 most important bits 
   *  (50 bits of class 1a and another 15 bits of class 1b).  These bits 
   *  are taken from in the order defined by GSM 05.03 v5.5.0, section 3.1.1.1.
   *  This order shall be implemented as a table in software.   
   */

   /* bit mapping of 65 bits used for CRC calculation */  
   for(i=0; i<EFR_NUM_TCH_CRC_IN_BITS; i++)
   {
      input[i] = inputPtr[tch_Efr_CRC_calc_LookUpTbl[i]];
   }      
 
   /*
   * Multiply the input data polynomial by D^numParityBits. This basically
   * amounts to appending numParityBits "0s" to the tail of the vector. 
   */
   for(i=EFR_NUM_TCH_CRC_IN_BITS; i<EFR_NUM_TCH_CRC_IN_BITS + EFR_NUM_TCH_PARITY_BITS; i++)
   {
      input[i] = 0;
   }
 
   /*
   * Init the register outputs to zero
   */
   regOut = 0x0000;

   /*
   * Or in the first data bit
   */
   bit = input[0] << EFR_NUM_TCH_PARITY_BITS;
   regOut |= bit;

   /*
   * Loop over all bits in the input vector ( D^N d(D), N = numParityBits )
   */
   for(i=0; i<EFR_NUM_TCH_CRC_IN_BITS + EFR_NUM_TCH_PARITY_BITS; i++)
   {
      output = regOut & 0x0001;

      result = poly * output;

      regIn = result ^ regOut;

      regOut = regIn >> 1;

      bit = input[i+1] << EFR_NUM_TCH_PARITY_BITS;

      regOut |= bit;
   }

   /*
   * Do not inverted parity bits, since the Remainder is '0"
   */
   for(i=0; i<EFR_NUM_TCH_PARITY_BITS; i++)
   {
      bit = (regOut>>i) & 0x0001;
      parityPtr[i] = (Char)bit;
   }      

   return;
}

/*****************************************************************************
** ROUTINE NAME: crcDecoderEfrTCH
**
** Description: 
**   Performs the TCH Binary Cyclic Decoding (Block Decoding) used in GSM. 
**   The input data and parity bit arrays used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the block decoding as described in GSM 05.03.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit+parity array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**   parityError - TRUE indicates that a parity error has been detected
**
** Data Store  I/O: 
**
*****************************************************************************/
Bool crcDecoderEfrTCH(Char *inputPtr, Char *parityPtr)
{
   UInt  i;
   Uint8 genParityBits[EFR_NUM_TCH_PARITY_BITS];
   Uint8 tempBuf[16];
   Bool  parityError = 0;
   
   /*
   * Perform the crc encoding to calculate the parity based on the recieved
   * bits. The calculated parity is stored in calParity array.
   */
   crcEncoderEfrTCH(inputPtr, genParityBits);
   
   /*
   * Compare recieved parity to calculated parity
   */
   for(i=0; i<EFR_NUM_TCH_PARITY_BITS; i++)
   {
      if(genParityBits[i] != parityPtr[i])
      {
         parityError = TRUE;
         break;
      }
   }
   return(parityError);
}

/*****************************************************************************
** ROUTINE NAME: crcEncoderSCH
**
** Description:
**   Performs the SCH Binary Cyclic Encoding (Block Encoding) used in GSM. 
**   The input data and parity bit arrays used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the block coding as described in GSM 05.03.
** 
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**
** Data Store  I/O: 
**
*****************************************************************************/
void crcEncoderSCH(Char *inputPtr, Char *parityPtr) 
{
   UInt i;
   UShort regIn;
   UShort regOut;
   UShort result;
   const UShort poly = 0x0575;     /* D^0 .. D^N */
   UShort bit;
   Char output;
   Char input[NUM_SCH_CRC_IN_BITS + NUM_SCH_PARITY_BITS];

   /*
   * Multiply the input data polynomial by D^numParityBits. This basically
   * amounts to appending numParityBits "0s" to the tail of the vector. 
   */
   for(i=0; i<NUM_SCH_CRC_IN_BITS; i++)
   {
      input[i] = inputPtr[i];
   }

   for(i=NUM_SCH_CRC_IN_BITS; i<NUM_SCH_CRC_IN_BITS + NUM_SCH_PARITY_BITS; i++)
   {
      input[i] = 0;
   }
 
   /*
   * Init the register outputs to zero
   */
   regOut = 0x0000;

   /*
   * Or in the first data bit
   */
   bit = input[0] << NUM_SCH_PARITY_BITS;
   regOut |= bit;

   /*
   * Loop over all bits in the input vector ( D^N d(D), N = numParityBits )
   */
   for(i=0; i<NUM_SCH_CRC_IN_BITS + NUM_SCH_PARITY_BITS; i++)
   {
      output = regOut & 0x0001;

      result = poly * output;

      regIn = result ^ regOut;

      regOut = regIn >> 1;

      bit = input[i+1] << NUM_SCH_PARITY_BITS;

      regOut |= bit;
   }
   /*
   * Return inverted parity bits
   */
   regOut ^= 0xffff;

   for(i=0; i<NUM_SCH_PARITY_BITS; i++)
   {
      bit = (regOut>>i) & 0x0001;
      parityPtr[i] = (Char)bit;
   }
}

/*****************************************************************************
** ROUTINE NAME: crcEncoderRACH
**
** Description:
**   Performs the RACH Binary Cyclic Encoding (Block Encoding) used in GSM. 
**   The input data and parity bit arrays used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the block coding as described in GSM 05.03.
** 
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**
** Data Store  I/O: 
**
*****************************************************************************/
void crcEncoderRACH(Char *inputPtr, Char *parityPtr) 
{
   UInt i;
   UShort regIn;
   UShort regOut;
   UShort result;
   const UShort poly = 0x007b;     /* D^0 .. D^N */
   UShort bit;
   Char output;
   Char input[NUM_RACH_CRC_IN_BITS + NUM_RACH_PARITY_BITS];

   /*
   * Multiply the input data polynomial by D^numParityBits. This basically
   * amounts to appending numParityBits "0s" to the tail of the vector. 
   */
   for(i=0; i<NUM_RACH_CRC_IN_BITS; i++)
   {
      input[i] = inputPtr[i];
   }

   for(i=NUM_RACH_CRC_IN_BITS; i<NUM_RACH_CRC_IN_BITS + NUM_RACH_PARITY_BITS; i++)
   {
      input[i] = 0;
   }
 
   /*
   * Init the register outputs to zero
   */
   regOut = 0x0000;

   /*
   * Or in the first data bit
   */
   bit = input[0] << NUM_RACH_PARITY_BITS;
   regOut |= bit;

   /*
   * Loop over all bits in the input vector ( D^N d(D), N = numParityBits )
   */
   for(i=0; i<NUM_RACH_CRC_IN_BITS + NUM_RACH_PARITY_BITS; i++)
   {
      output = regOut & 0x0001;

      result = poly * output;

      regIn = result ^ regOut;

      regOut = regIn >> 1;

      bit = input[i+1] << NUM_RACH_PARITY_BITS;

      regOut |= bit;
   }

   /*
   * Return inverted parity bits, modulo 2 added with bsic.
   * Note that the bsic is stored in the reverse order relative to 
   * the parity bits.
   */
   regOut ^= 0xffff;

   for(i=0; i<NUM_RACH_PARITY_BITS; i++)
   {
      bit = (regOut >> i) & 0x0001;
      parityPtr[i] = (Char)(bit);
   }
}

/*****************************************************************************
** ROUTINE NAME: crcDecoderRACH
**
** Description: 
**   Performs the RACH Binary Cyclic Decoding (Block Decoding) used in GSM. 
**   The input data and parity bit arrays used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the block decoding as described in GSM 05.03.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit+parity array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**   parityError - TRUE indicates that a parity error has been detected
**
** Data Store  I/O: 
**
*****************************************************************************/
Bool crcDecoderRACH(Char *inputPtr, Char *parityPtr)
{
   UInt i;
   Bool parityError;

   parityError = 0;

   /*
   * Perform the crc encoding to calculate the parity based on the recieved
   * bits. The calculated parity is stored in calParity array.
   */
   crcEncoderRACH(inputPtr, parityPtr);

   /*
   * Compare recieved parity to calculated parity
   */
   for(i=0; i<NUM_RACH_PARITY_BITS; i++)
   {
      if(inputPtr[i+NUM_RACH_CRC_IN_BITS] != parityPtr[i])
      {
         parityError = TRUE;
      }
   }
   return(parityError);
}
 
/*****************************************************************************
** ROUTINE NAME: crcEncoderTCH
**
** Description:
**   Performs the TCH Binary Cyclic Encoding (Block Encoding) used in GSM. 
**   The input data and parity bit arrays used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the block coding as described in GSM 05.03.
** 
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**
** Data Store  I/O: 
**
*****************************************************************************/
void crcEncoderTCH(Char *inputPtr, Char *parityPtr) 
{
   UInt i;
   UShort regIn;
   UShort regOut;
   UShort result;
   const UShort poly = 0x000d;     /* D^0 .. D^N */
   UShort bit;
   Char output;
   Char input[NUM_TCH_CRC_IN_BITS + NUM_TCH_PARITY_BITS];

   /*
   * Multiply the input data polynomial by D^numParityBits. This basically
   * amounts to appending numParityBits "0s" to the tail of the vector. 
   */
   for(i=0; i<NUM_TCH_CRC_IN_BITS; i++)
   {
      input[i] = inputPtr[i];
   }

   for(i=NUM_TCH_CRC_IN_BITS; i<NUM_TCH_CRC_IN_BITS + NUM_TCH_PARITY_BITS; i++)
   {
      input[i] = 0;
   }
 
   /*
   * Init the register outputs to zero
   */
   regOut = 0x0000;

   /*
   * Or in the first data bit
   */
   bit = input[0] << NUM_TCH_PARITY_BITS;
   regOut |= bit;

   /*
   * Loop over all bits in the input vector ( D^N d(D), N = numParityBits )
   */
   for(i=0; i<NUM_TCH_CRC_IN_BITS + NUM_TCH_PARITY_BITS; i++)
   {
      output = regOut & 0x0001;

      result = poly * output;

      regIn = result ^ regOut;

      regOut = regIn >> 1;

      bit = input[i+1] << NUM_TCH_PARITY_BITS;

      regOut |= bit;
   }

   /*
   * Return inverted parity bits
   */
   regOut ^= 0xffff;

   for(i=0; i<NUM_TCH_PARITY_BITS; i++)
   {
      bit = (regOut>>i) & 0x0001;
      parityPtr[i] = (Char)bit;
   }
}

/*****************************************************************************
** ROUTINE NAME: crcDecoderTCH
**
** Description: 
**   Performs the TCH Binary Cyclic Decoding (Block Decoding) used in GSM. 
**   The input data and parity bit arrays used by this routine are assummed 
**   in the following format with respect to bit order within the array:
**
**      polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**      data/parity bits   u(0)   u(1)   ......  u(N-3)  u(N-2)  u(N-1)
**      array index        0      1      ......  N-3     N-2     N-1
**      
** Method:
**   Performs the block decoding as described in GSM 05.03.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**   inputPtr - indexes the input bit+parity array
**   parityPtr - indexes the array to store the calculated parity bits
**
** Returns: 
**   parityPtr - indexes the parity bits that are calculated by the routine
**   parityError - TRUE indicates that a parity error has been detected
**
** Data Store  I/O: 
**
*****************************************************************************/
Bool crcDecoderTCH(Char *inputPtr, Char *parityPtr)
{
   UInt i;
   Bool parityError;

   parityError = 0;

   /*
   * Perform the crc encoding to calculate the parity based on the recieved
   * bits. The calculated parity is stored in calParity array.
   */
   crcEncoderTCH(inputPtr, parityPtr);

   /*
   * Compare recieved parity to calculated parity
   */
   for(i=0; i<NUM_TCH_PARITY_BITS; i++)
   {
      if(inputPtr[i+NUM_TCH_CRC_IN_BITS] != parityPtr[i])
      {
         parityError = TRUE;
      }
   }
   return(parityError);
}
 
                         
#endif /* end of COMBINED_CRC_16 code; not yet used!!! */


/************************************* Old Rev Notes ***********************************
**
** Revision 1.5  2000-01-12 10:17:03-08  whuang
** Added CRC encoder/Decoder, subjective encoder/decoder and VOIP format for EFR.
** No uplink frame is sent out, if CRC error is found for EFR also. Created a common
** CRC encoder/decoder for SCH, TCH, RA
**
** Revision 1.4  1999-05-28 16:25:50-07  hpines
** 1.  undo last checked-in change relative to oldMets[] = INITVAL;  
** set all states = INITVAL 2.  INITVAL now = 0 (instead 0x9000), since soft-decision
** inputs only 5 bits max therefore no worry about 
** 3.  return OLDMETS[0] instead NEWMETS[0], but may need to modify later.
**
** Revision 1.3  1999-05-05 18:53:12-07  dkaiser
** combined previous codecs.c with chdecode.c, result is codecs.c, deleted chdecode.c
**
** Revision 1.2  1999-02-09 11:13:12-08  randyt
** Used assembly optimized Viterbi decoder
 * 
 *    Rev 1.1   03 Feb 1999 17:09:24   whuang
 * Used assembly optimized Viterbi decoder
 * 
 *    Rev 1.0   Oct 27 1998 16:19:28   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   Oct 23 1998 17:56:02   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
** Revision 1.10  1998/10/16 14:13:47  dlanza
** Fixed possible alignment problem.
**
** Revision 1.9  1998/10/15 20:12:02  dlanza
** Added the TCH path and compiled successfully.
**
** Revision 1.8  1998/10/06 20:43:08  kmcpherson
** RKM - Support added, tested, and validated for SCH downlink, RACH downlink and
** uplink, and downlink and uplink sync processing routines.
**
** Revision 1.7  1998/10/03 23:23:45  dlanza
** Optimized c code...executes in 24,010 cycles.
**
** Revision 1.6  1998/10/02 19:49:40  dlanza
** 27,429 cycles.
**
** Revision 1.5  1998/10/02 16:31:02  dlanza
** <>
**
** Revision 1.4  1998/10/02 15:16:51  dlanza
** <>
**
** Revision 1.3  1998/09/30 21:51:54  kmcpherson
** RKM - CCH Encoder/Decoder functionally validated.
**
** Revision 1.2  1998/09/28 19:28:09  kmcpherson
** <>
**
** Revision 1.1  1998/09/24 22:31:23  kmcpherson
** Initial revision
**
** Revision 1.4  1998/09/24 21:04:34  kmcpherson
** <>
**
** Revision 1.3  1998/09/22 13:39:23  kmcpherson
** RKM - CCH Channel Encoder is validated against the simulation test
** vectors.
**
** Revision 1.2  1998/09/19 21:56:16  kmcpherson
** RKM 9/19/98 - CCH Encoder done and ready for test vector validation.  The trival
** case had been validated and seems to work through cnv encoding.  Interleaving not tested in full CODEC test.
*************************************************************************************/                         
                         
 