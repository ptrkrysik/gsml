/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
** Filename: topcodec.c
**
** Description:   
**   This file contains the top level routines for GSM channel encoding and
**   decoding. Old revision notes at end of this file.
** 
*****************************************************************************/
#include "gsmdata.h" 
#include "bbdata.h" 
#include "bbproto.h"  
#include "codec.h"
#include "stdlib.h"
#include "l1config.h"
 
#include "dsprotyp.h"

#define BER_TEST 0
#define RANDOM_TEST 0
#define FIRE_TEST 0
#define PRACH_TEST 0

#pragma DATA_SECTION(badTchFrameCounter, ".extData");
Int32 badTchFrameCounter = 0;
#pragma DATA_SECTION(badTchMetricCounter, ".extData");
Int32 badTchMetricCounter = 0;
#pragma DATA_SECTION(badCchFrameCounter, ".extData");
Int32 badCchFrameCounter = 0;
#pragma DATA_SECTION(badfacchFrameCounter, ".extData");
Int32 badfacchFrameCounter = 0;
#pragma DATA_SECTION(badRachFrameCounter, ".extData");
Int32 badRachFrameCounter = 0;
#pragma DATA_SECTION(badPdchFrameCounter, ".extData");
Int32 badPdchFrameCounter = 0;
#pragma DATA_SECTION(badPrachFrameCounter, ".extData");
Int32 badPrachFrameCounter = 0;
#pragma DATA_SECTION(tchFrameCounter, ".extData");
Int32 tchFrameCounter = 0;
#pragma DATA_SECTION(cchFrameCounter, ".extData");
Int32 cchFrameCounter = 0;
#pragma DATA_SECTION(facchFrameCounter, ".extData");
Int32 facchFrameCounter = 0;
#pragma DATA_SECTION(rachFrameCounter, ".extData");
Int32 rachFrameCounter = 0;
#pragma DATA_SECTION(pdchFrameCounter, ".extData");
Int32 pdchFrameCounter = 0;
#pragma DATA_SECTION(prachFrameCounter, ".extData");
Int32 prachFrameCounter = 0;
#pragma DATA_SECTION(totalTchSamples, ".extData");
Int32 totalTchSamples = 0;
#pragma DATA_SECTION(totalCchSamples, ".extData");
Int32 totalCchSamples = 0;
#pragma DATA_SECTION(totalfacchSamples, ".extData");
Int32 totalfacchSamples = 0;
#pragma DATA_SECTION(totalRachSamples, ".extData");
Int32 totalRachSamples = 0;
#pragma DATA_SECTION(totalPdchSamples, ".extData");
Int32 totalPdchSamples = 0;
#pragma DATA_SECTION(totalPrachSamples, ".extData");
Int32 totalPrachSamples = 0;
#pragma DATA_SECTION(totalTchChanErrs, ".extData");
Int32 totalTchChanErrs = 0;
#pragma DATA_SECTION(totalCchChanErrs, ".extData");
Int32 totalCchChanErrs = 0;
#pragma DATA_SECTION(totalfacchChanErrs, ".extData");
Int32 totalfacchChanErrs = 0;
#pragma DATA_SECTION(totalRachChanErrs, ".extData");
Int32 totalRachChanErrs = 0;
#pragma DATA_SECTION(totalPdchChanErrs, ".extData");
Int32 totalPdchChanErrs = 0;
#pragma DATA_SECTION(totalPrachChanErrs, ".extData");
Int32 totalPrachChanErrs = 0;
#pragma DATA_SECTION(massiveTchFrameErr, ".extData");
Int32 massiveTchFrameErr = 0;
#pragma DATA_SECTION(massiveCchFrameErr, ".extData");
Int32 massiveCchFrameErr = 0;

Uint8 USFCodingTableCS23[8] =
{0x00, 0x29, 0x1A, 0x33, 0x34, 0x1D, 0x2E, 0x07};

Uint16 USFCodingTableCS4[8] =
{0x0000, 0x0D0B, 0x06EC, 0x0BE7, 0x0BB0, 0x06BB, 0x0D5C, 0x0057};
/* {0x0000, 0x0D0B, 0x06EC, 0x0B77, 0x0BB0, 0x06BB, 0x0B5C, 0x0057}; */
 

Uint16 computeTotalErrors(Char *in, Char *out,  Uint16 numbits);
void copyByteSamples(Uint8  *in, Uint8 *out, Uint16 numbits); 
   
/*****************************************************************************
** Routine Name: chanEncoder
**
** Description: 
**   Performs the channel encoding for the specified channel as described in
**   GSM 05.03.      
**
** Method:
**   Algorithms coded as specified in GSM 05.03.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**    chanSel - specifies the channel type to encode
**    frameBoundary - specifies if we are on a channel subframe frame boundary
**    TN - GSM timeslot number (0..7)
**
** Returns: 
**    None
**
** Data Store I/O: 
**     Unpacked BSIC array - g_codecBsic[0..5], Used for RACH encoding.
**        This array is initialized from g_BBInfo[trx].bsic in chanCodecInit.
**
**     For CCH, SCH, and RACH channels, the following output buffer is used:
**
**        g_DLCCHData.dlCCHword0.bitval.bufferValid
**        g_ULCCHData.data[i], for 0 <= i <= 7
**
**            The input data and array used by this routine 
**            is assummed in the following format with respect to bit order 
**            within the array. See GSM 05.03. For RACH, N = 8. 
**            For CCH, N = 184.  For SCH, N = 25.
**
**            polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**            data/parity bits   d(0)   d(1)   ......  d(N-3)  d(N-2)  d(N-1)
**            array index        0      1      ......  N-3     N-2     N-1
**
*****************************************************************************/

void chanEncoder(t_chanSel chanSel, Bool frameBoundary, Uint8 TN, Bool tx)
{
   Char parityBits[MAX_PARITY_BITS];
   Char cnvEncInData[NUM_GPRS_CS4_CNV_ENC_IN_BITS];
   Char cnvEncOutData[2*NUM_GPRS_CS3_CNV_ENC_IN_BITS];
   UInt i, j;                                      
   Uint8 trx;
   Uint8 temp;
   UInt ii, jj;                                      
#if BER_TEST == 1
   UChar k;
#endif

   t_TNInfo    *ts;   
      
   trx = 0;
				
   ts = & g_BBInfo[trx].TNInfo[TN];

   /*
   * for loopback test
   g_DLTCHData.dltchword0.bitval.bufferValid = TRUE;   
   */

   if(frameBoundary)
   {
      if(((chanSel != TCHF) && g_DLCCHData.bufferValid) || 
         ((chanSel == TCHF) && g_DLTCHData.dltchword0.bitval.bufferValid))
      {
#if FIRE_TEST == 1
       if((chanSel == TCHF) && (TN == 2) && ((tchFrameCounter % 1) == 0))
        chanSel = FACCH;    
#endif
#if PRACH_TEST == 1
       if(chanSel == RACH) 
        chanSel = PRACH;    
#endif
         switch(chanSel)
         {
            case TCHF:
               /*
               * Fullrate speech traffic channel
               */
               if(tx) g_intBuf8Deep.txBurstCount[TN] = 8;
               
               
#if RANDOM_TEST == 1

	       for(i=0; i < NUM_BITS_TCH_FRAME; i++)
	       {
	         k = (rand() & 1);
	         g_DLTCHData.data[i] = k;
	       }
#endif
               
               /*
               * Perform subjective encoding of speech parameters based
               * on speech algorithm in use.
               */
               sbjEncoder(g_DLTCHData.data,
                          g_BBInfo[trx].TNInfo[TN].u.group1.tch.vocAlgo);

               /*
               * Invoke CRC Encoder. Put parity bits directly into Convolutional
               * input buffer
               */
               //crcEncoderTCH(g_DLTCHData.data, & cnvEncInData[(NUM_BITS_TCH_CLASSI/2)]);
               crcEncoder16(g_DLTCHData.data,
                            & cnvEncInData[(NUM_BITS_TCH_CLASSI/2)],
                            TCH_CRC_POLY,
                            NUM_TCH_PARITY_BITS,
                            NUM_TCH_CRC_IN_BITS,                            
                            0xFFFF); 
               

               /*
               * Copy input data into separate buffer in preparation
               * of appending the parity bits.  Data is reordered per GSM 05.03
               * paragraph 3.1.2.1.
               */
               for(i=0; i<(NUM_BITS_TCH_CLASSI/2); i++)
               {
                  cnvEncInData[i] = g_DLTCHData.data[2*i];
                  cnvEncInData[TCH_REORDER_MAX_INDEX-i] = g_DLTCHData.data[2*i+1];
               }

               /*
               * Add tail bits
               */
               for(i=0; i<NUM_CNV_TAIL_BITS; i++)
               {
                  cnvEncInData[TCH_START_OF_TAIL+i] = 0;
               }

               /*
               * Execute the cnv encoder
               */
               cnvEncoder2(cnvEncInData,cnvEncOutData,NUM_TCH_CNV_ENC_IN_BITS);

               /*
               * Add the Class II bits to the encoded bits.
               */
               for(i=0; i<NUM_BITS_TCH_CLASSII; i++)
               {
                  cnvEncOutData[NUM_TCH_CNV_ENC_OUT_BITS+i] =
                                  g_DLTCHData.data[NUM_BITS_TCH_CLASSI+i];
               }

               /*
               * Execute the TCH Interleaver with the stealing flag FALSE.
               * Clear the Valid flag, indicating buffer has been processed.
               */
               blockDiagInterleaver8(cnvEncOutData, FALSE,
                                     &g_intBuf8Deep.tchTXBurstIndex[TN],
                                     g_intBuf8Deep.data[TN]);
               g_DLTCHData.dltchword0.bitval.bufferValid = FALSE;
               break;

            case FACCH:
               /*
               *  FACCH Message
               */
               if(tx)  g_intBuf8Deep.txBurstCount[TN] = 8;
                          
               /*
               * Invoke the CRC Encoder. Put parity bits directly into Convolutional
               * input buffer
               */
               crcEncoderCCH(g_DLCCHData.data, & cnvEncInData[NUM_CCH_CRC_IN_BITS]);

               /*
               * Copy input data into seperate buffer in preparation
               * of appending the parity bits
               */
               for(i=0; i<NUM_CCH_CRC_IN_BITS; i++)
               {
                  cnvEncInData[i] = g_DLCCHData.data[i];
               }
               
               /*
               * Add tail bits
               */
               for(i=0; i<NUM_CNV_TAIL_BITS; i++)
               {
                  cnvEncInData[NUM_CCH_CRC_IN_BITS+NUM_CCH_PARITY_BITS+i] = 0;
               }

               /*
               * Execute the cnv encoder
               */
               cnvEncoder2(cnvEncInData, cnvEncOutData, NUM_CCH_CNV_ENC_IN_BITS);

               /*
               * Execute the TCH Interleaver with the stealing flag TRUE.
               * Clear the Valid flag for both the CCH ad TCH buffers since CCH
               * was used and TCH was discarded (FACCH stole TCH).
               */
               blockDiagInterleaver8(cnvEncOutData, TRUE,
                                     &g_intBuf8Deep.tchTXBurstIndex[TN],
                                     g_intBuf8Deep.data[TN]);
               g_DLTCHData.dltchword0.bitval.bufferValid = FALSE;
               g_DLCCHData.bufferValid = FALSE;
               break;  
                    
            case PDCH:
            case PDTCH:                    
            case PACCH:  
                        
            switch(g_DLCCHData.chCodec)
            {
             case GPRS_CS_1:

#if RANDOM_TEST == 1
	       for(i=0; i < NUM_BITS_CCH_FRAME; i++)
	       {
	         k = (rand() & 1);
	         g_DLCCHData.data[i] = k; 
	       }
#endif

                /*
               * These channels all use the common CCH channel encoding
               * algorithms
               */     

               /*
               * Invoke the CRC Encoder. Put parity bits directly into Convolutional
               * input buffer
               */
               crcEncoderCCH(&g_DLCCHData.data[0], & cnvEncInData[NUM_CCH_CRC_IN_BITS]);

               /*
               * Copy input data into seperate buffer in preparation
               * of appending the parity bits
               */
               for(i=0; i<NUM_CCH_CRC_IN_BITS; i++)
               {
                  cnvEncInData[i] = g_DLCCHData.data[i];
               }
               
               /*
               * Add tail bits
               */
               for(i=0; i<NUM_CNV_TAIL_BITS; i++)
               {
                  cnvEncInData[NUM_CCH_CRC_IN_BITS+NUM_CCH_PARITY_BITS+i] = 0;
               }


               /*
               * Execute the cnv encoder
               */
               cnvEncoder2(cnvEncInData, cnvEncOutData, NUM_CCH_CNV_ENC_IN_BITS);

              break;     /* end GPRS-CS1 */

 /*  do GPRS-CS2 */

             case GPRS_CS_2:

#if RANDOM_TEST == 1
	       for(i=6; i < NUM_GPRS_CS2_CRC_IN_BITS; i++)
	       {
	         k = (rand() & 1);
	         g_DLCCHData.data[i] = k;
/*	         g_DLCCHData.data[i] = i & 1; */
	       }
#endif

/* USF begins &g_DLCCHData.data[3] */

               crcEncoder16(& g_DLCCHData.data[3],
                            parityBits,
                            GPRS16_CRC_POLY,
                            NUM_GPRS_CS2_PARITY_BITS,
                            NUM_BITS_GPRS_CS2,                            
                            0xFFFF); 

               /*
               *  unpack 6-bit PreCoded USF 
               */    
               unpackFromByteBuffer((Uint8 *)&USFCodingTableCS23[ts->u.group13.nextUsf], 
                                   (Uint8 *)&g_DLCCHData.data[0], 
                                   6);   
                                 
               /*
               * Copy input data into seperate buffer in preparation
               * of appending the parity bits
               */
               for(i=0; i<NUM_GPRS_CS2_CRC_IN_BITS; i++)
               {
                  cnvEncInData[i] = g_DLCCHData.data[i];
               }

               /*
               * Append the parity bits to the data bits
               */
               for(i=NUM_GPRS_CS2_CRC_IN_BITS;
                  i<NUM_GPRS_CS2_CRC_IN_BITS+NUM_GPRS_CS2_PARITY_BITS; i++)
               {
                  cnvEncInData[i] = parityBits[i-NUM_GPRS_CS2_CRC_IN_BITS];
               }

               /*
               * Add tail bits
               */
               for(i=0; i<NUM_CNV_TAIL_BITS; i++)
               {
                  cnvEncInData[NUM_GPRS_CS2_CRC_IN_BITS+NUM_GPRS_CS2_PARITY_BITS+i] = 0;
               }

               /*
               * Execute the cnv encoder
               */
               cnvEncoder2(cnvEncInData, cnvEncOutData, NUM_GPRS_CS2_CNV_ENC_IN_BITS);

/*
*GPRS...puncture the convolutionally encoded bits
*/ 
#ifdef NOT_ASM_OPTIMIZED
		 j = 0;
             ii = 15;
     		 jj = 39;
	       for(i=0; i < (2*NUM_GPRS_CS2_CNV_ENC_IN_BITS); i++)
	       {
               if(i != ii)
               {
	          cnvEncOutData[j] = cnvEncOutData[i];
                j++;
               }
               else
               {
		    ii += 4;
                if(ii == jj)
                 {
                   jj += 48;
                   ii += 4;
                 }
		   }
             }
#else
 		   punctureGPRS(&cnvEncOutData[0],&cnvEncOutData[0],GPRS_CS_2);
#endif

               break;     /* end GPRS-CS2 */

 /*  do GPRS-CS3 */

             case GPRS_CS_3:

#if RANDOM_TEST == 1
	       for(i=6; i < NUM_GPRS_CS3_CRC_IN_BITS; i++)
	       {
	         k = (rand() & 1);
	         g_DLCCHData.data[i] = k;
/*	         g_DLCCHData.data[i] = i & 1; */
	       }
#endif

/* USF begins &g_DLCCHData.data[3] */

               crcEncoder16(& g_DLCCHData.data[3],
                            parityBits,
                            GPRS16_CRC_POLY,
                            NUM_GPRS_CS3_PARITY_BITS,
                            NUM_BITS_GPRS_CS3,                            
                            0xFFFF); 

               /*
               *  unpack 6-bit PreCoded USF 
               */    
               unpackFromByteBuffer((Uint8 *)&USFCodingTableCS23[ts->u.group13.nextUsf], 
                                   (Uint8 *)&g_DLCCHData.data[0], 
                                   6);                                    
               
               /*
               * Copy input data into seperate buffer in preparation
               * of appending the parity bits
               */

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_GPRS_CS3_CRC_IN_BITS; i++)
               {
                  cnvEncInData[i] = g_DLCCHData.data[i];
               }
#else
               copyByteSamples( (Uint8 *) & g_DLCCHData.data[0], (Uint8 *) & cnvEncInData[0],
                                NUM_GPRS_CS3_CRC_IN_BITS);
#endif
                     
               /*
               * Append the parity bits to the data bits
               */
               for(i=NUM_GPRS_CS3_CRC_IN_BITS;
                  i<NUM_GPRS_CS3_CRC_IN_BITS+NUM_GPRS_CS3_PARITY_BITS; i++)
               {
                  cnvEncInData[i] = parityBits[i-NUM_GPRS_CS3_CRC_IN_BITS];
               }

               /*
               * Add tail bits
               */
               for(i=0; i<NUM_CNV_TAIL_BITS; i++)
               {
                  cnvEncInData[NUM_GPRS_CS3_CRC_IN_BITS+NUM_GPRS_CS3_PARITY_BITS+i] = 0;
               }

               /*
               * Execute the cnv encoder
               */
               cnvEncoder2(cnvEncInData, cnvEncOutData, NUM_GPRS_CS3_CNV_ENC_IN_BITS);

/*
*GPRS...puncture the convolutionally encoded bits
*/ 
#ifdef NOT_ASM_OPTIMIZED
		 j = 0;
             ii = 15;
     		 jj = 17;
	       for(i=0; i < (2*NUM_GPRS_CS3_CNV_ENC_IN_BITS); i++)
	       {
               if((i != ii) && (i != jj))
               {
	          cnvEncOutData[j] = cnvEncOutData[i];
                j++;
               }
               else
               {
                if(i == ii)
		     ii += 6;
                else
                 jj += 6;
		   }
             }
#else
 		   punctureGPRS(&cnvEncOutData[0],&cnvEncOutData[0],GPRS_CS_3);
#endif
               break;     /* end GPRS-CS3 */

 /*  do GPRS-CS4 */

             case GPRS_CS_4:

#if RANDOM_TEST == 1
	       for(i=12; i < NUM_GPRS_CS4_CRC_IN_BITS; i++)
	       {
	         k = (rand() & 1);
	         g_DLCCHData.data[i] = k;
/*	         g_DLCCHData.data[i] = i & 1; */
	       }
#endif

/* USF begins &g_DLCCHData.data[9] */

               crcEncoder16(& g_DLCCHData.data[9],
                            parityBits,
                            GPRS16_CRC_POLY,
                            NUM_GPRS_CS4_PARITY_BITS,
                            NUM_BITS_GPRS_CS4,                            
/*                            NUM_GPRS_CS4_CRC_IN_BITS,        */                    
                            0xFFFF); 

               /*
               *  unpack 12-bit Precoded USF 
               */    
               unpackFromByteBuffer((Uint8 *)&USFCodingTableCS4[ts->u.group13.nextUsf], 
                                   (Uint8 *)&g_DLCCHData.data[0], 
                                   12);                                    
               
               
               /*
               * Copy input data into seperate buffer in preparation
               * of appending the parity bits
               */
               for(i=0; i<NUM_GPRS_CS4_CRC_IN_BITS; i++)
               {
                  cnvEncOutData[i] = g_DLCCHData.data[i];
               }


               /*
               * Append the parity bits to the data bits
               */
               for(i=NUM_GPRS_CS4_CRC_IN_BITS;
                  i<NUM_GPRS_CS4_CRC_IN_BITS+NUM_GPRS_CS4_PARITY_BITS; i++)
               {
                  cnvEncOutData[i] = parityBits[i-NUM_GPRS_CS4_CRC_IN_BITS];
               }

               /*
               * No do cnv encoder for CS-4
               */

               break;     /* end GPRS-CS4 */

              } /* end GPRS chCodec switch */

              if(g_DLCCHData.chCodec != GPRS_CS_1)
              {
                 Uint8 ParityWord[2];
                 
                  packToByteBuffer(parityBits, 
                                   ParityWord,
                                   16 );
                               
                 sendDiagMsg(41, trx, TN, 2, ParityWord);
              }

              g_DLCCHData.codecFlags = g_GPRSCodecFlags[g_DLCCHData.chCodec];    
 
               /*
               * Execute the CCH Interleaver
               */
               blockRectInterleaver(cnvEncOutData,
                                    &g_intBuf8Deep.tchTXBurstIndex[TN],
                                    g_intBuf8Deep.data[TN]);
               g_DLCCHData.bufferValid = FALSE;               
               
              break;  /* end PDCH Case */
               
            case SACCH:
            case SDCCH4:
            case SDCCH8:
            case BCCH: 
            case PCH:
            case AGCH:
            case PTCCH:            
               /*
               * These channels all use the common CCH channel encoding
               * algorithms
               */     


#if RANDOM_TEST == 1
	       for(i=0; i < NUM_BITS_CCH_FRAME; i++)
	       {
	        k = (rand() & 1);
	        g_DLCCHData.data[i] = k;
	       }
#endif

               /*
               * Invoke the CRC Encoder. Put parity bits directly into Convolutional
               * input buffer
               */
               crcEncoderCCH(&g_DLCCHData.data[0], & cnvEncInData[NUM_CCH_CRC_IN_BITS]);

               /*
               * Copy input data into seperate buffer in preparation
               * of appending the parity bits
               */
               for(i=0; i<NUM_CCH_CRC_IN_BITS; i++)
               {
                  cnvEncInData[i] = g_DLCCHData.data[i];
               }
               
               /*
               * Add tail bits
               */
               for(i=0; i<NUM_CNV_TAIL_BITS; i++)
               {
                  cnvEncInData[NUM_CCH_CRC_IN_BITS+NUM_CCH_PARITY_BITS+i] = 0;
               }

               /*
               * Execute the cnv encoder
               */
               cnvEncoder2(cnvEncInData, cnvEncOutData, NUM_CCH_CNV_ENC_IN_BITS);
		   
     
               g_DLCCHData.codecFlags = 0xFF;     
              /*
               * Execute the CCH Interleaver
               */
               blockRectInterleaver(cnvEncOutData,
                                    &g_intBuf4Deep.cchTXBurstIndex[TN],
                                    g_intBuf4Deep.data[TN]);
               g_DLCCHData.bufferValid = FALSE;
               break;
               
            case SCH:
               /*
               * Invoke the SCH CRC Encoder
               */
               //crcEncoderSCH(&g_DLCCHData.data[0], parityBits);
               crcEncoder16(g_DLCCHData.data,
                            parityBits,
                            SCH_CRC_POLY,
                            NUM_SCH_PARITY_BITS,
                            NUM_SCH_CRC_IN_BITS,                            
                            0xFFFF); 
               
               /*
               * Copy input data into seperate buffer in preparation
               * of appending the parity bits
               */
               for(i=0; i<NUM_SCH_CRC_IN_BITS; i++)
               {
                  cnvEncInData[i] = g_DLCCHData.data[i];
               }

               /*
               * Append the parity bits to the data bits
               */
               for(i=NUM_SCH_CRC_IN_BITS; i<NUM_SCH_CRC_IN_BITS+NUM_SCH_PARITY_BITS; i++)
               {
                  cnvEncInData[i] = parityBits[i-NUM_SCH_CRC_IN_BITS];
               }

               /*
               * Add tail bits
               */
               for(i=0; i<NUM_CNV_TAIL_BITS; i++)
               {
                  cnvEncInData[NUM_SCH_CRC_IN_BITS+NUM_SCH_PARITY_BITS+i] = 0;
               }

               /*
               * Execute the cnv encoder & set the TX bit field in the buffer
               */
               cnvEncoder2(cnvEncInData, (Char *)g_DLCodedData.data,
                           NUM_SCH_CNV_ENC_IN_BITS);
               g_DLCodedData.tx = TRUE;
               g_DLCCHData.bufferValid = FALSE;
               break;

            case RACH:
               /*
               * Perform the RACH channel coding in support of RF loopback with
               * access bursts
               */
#if RANDOM_TEST == 1
	       for(i=0; i < NUM_BITS_RACH_FRAME; i++)
	       {
	         k = (rand() & 1);
	         g_DLCCHData.data[i] = k;
	       }
#endif
               crcEncoder16(g_DLCCHData.data,
                            parityBits,
                            RACH_CRC_POLY,
                            NUM_RACH_PARITY_BITS,
                            NUM_RACH_CRC_IN_BITS,                            
                            0xFFFF); 
                              

               /*
               * Copy input data into seperate buffer in preparation
               * of appending the parity bits
               */
               for(i=0; i<NUM_RACH_CRC_IN_BITS; i++)
               {
                  cnvEncInData[i] = g_DLCCHData.data[i];
               }

               /*
               * Perform the modulo 2 addition of the bsic, and append the parity 
               * bits to the data bits
               */
               for(i=NUM_RACH_CRC_IN_BITS; i<NUM_RACH_CRC_IN_BITS+NUM_RACH_PARITY_BITS; i++)
               {
                  cnvEncInData[i] = (parityBits[i-NUM_RACH_CRC_IN_BITS] +
                                     g_codecBsic[i-NUM_RACH_CRC_IN_BITS]) & MODULO_2_MASK;
               }

               /*
               * Add tail bits
               */
               for(i=0; i<NUM_CNV_TAIL_BITS; i++)
               {
                  cnvEncInData[NUM_RACH_CRC_IN_BITS+NUM_RACH_PARITY_BITS+i] = 0;
               }

               /*
               * Execute the cnv encoder & set the TX bit field in the buffer
               */
               cnvEncoder2(cnvEncInData, (Char *)g_DLCodedData.data,
                           NUM_RACH_CNV_ENC_IN_BITS);
               g_DLCodedData.tx = TRUE;
               g_DLCCHData.bufferValid = FALSE;
               break;

            case PRACH:
               /*
               * Perform the RACH channel coding in support of RF loopback with
               * access bursts
               */

#if RANDOM_TEST == 1
	       for(i=0; i < NUM_BITS_PRACH_FRAME; i++)
	       {
	         k = (rand() & 1);
	         g_DLCCHData.data[i] = k;
	       }
#endif

               crcEncoder16(g_DLCCHData.data,
                            parityBits,
                            RACH_CRC_POLY,
                            NUM_RACH_PARITY_BITS,
                            NUM_PRACH_CRC_IN_BITS,                            
                            0xFFFF); 

               /*
               * Copy input data into seperate buffer in preparation
               * of appending the parity bits
               */
               for(i=0; i<NUM_PRACH_CRC_IN_BITS; i++)
               {
                  cnvEncInData[i] = g_DLCCHData.data[i];
               }
               /*
               * Perform the modulo 2 addition of the bsic, and append the parity 
               * bits to the data bits
               */
               for(i=NUM_PRACH_CRC_IN_BITS; i<NUM_PRACH_CRC_IN_BITS+NUM_RACH_PARITY_BITS; i++)
               {
                  cnvEncInData[i] = (parityBits[i-NUM_PRACH_CRC_IN_BITS] +
                                     g_codecBsic[i-NUM_PRACH_CRC_IN_BITS]) & MODULO_2_MASK;
               }

               /*
               * Add tail bits
               */
               for(i=0; i<NUM_CNV_TAIL_BITS; i++)
               {
                  cnvEncInData[NUM_PRACH_CRC_IN_BITS+NUM_RACH_PARITY_BITS+i] = 0;
               }

               /*
               * Execute the cnv encoder & set the TX bit field in the buffer
               */
               cnvEncoder2(cnvEncInData, (Char *)g_DLCodedData.data,
                           NUM_RACH_CNV_ENC_IN_BITS+NUM_RACH_GPRS_ADD_BITS);
/*
*GPRS...puncture the convolutionally encoded bits..pattern = 0,2,5,..,37,39,41
*/ 
		 j = 0;
             ii = 0;
	       for(i=0; i < 2*(NUM_RACH_CNV_ENC_IN_BITS+NUM_RACH_GPRS_ADD_BITS); i++)
	       {
               if(i != GPRSPrachPunctureBits[ii])
               {
	          g_DLCodedData.data[j] = g_DLCodedData.data[i];
                j++;
               }
               else
		    ii++;
             }
               g_DLCodedData.tx = TRUE;
               g_DLCCHData.bufferValid = FALSE;
               break;

            case IDLE:
               /*
               * Channel CODEC not involved here. A dummy burst will be transmitted
               */
               break;

            default:
               /*
               * Other channel types are not yet supported, return error
               * message
               */
               break;
         }
      }
      else
      {
         /*
         * Buffer is not ready to be processed.  Load a "zero" or fill 
         * frame into the interleaver buffer
         */
         for(i=0; i<NUM_BURSTS_PER_CCH_BLOCK; i++)
         {
            for(j=0; j<NB_NUM_ENCRYPT_WORDS; j++)
            {
               g_intBuf4Deep.data[TN][i][j] = 0x0000;
            }
         }
      }
   }

   /*
   * Based on chanSel and TN Output the appropriate burst from the interleaver 
   * buffer to the burst builder input buffer
   */
   burstLoader(chanSel, TN, frameBoundary);

}
                     
/*****************************************************************************
** Routine Name: chanDecoder
**
** Description: 
**   Performs the channel decoding for the specified channel.
**
** Method:
**   Algorithms are inverses of those encoding algorithms as specified 
**   in GSM 05.03.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**    chanSel - specifies the channel type to encode
**    frameBoundary - specifies if we are on a channel subframe frame boundary
**    TN - GSM timeslot number (0..7)
**
** Returns: 
**    achDetect - This boolean variable indicates if a FACCH or RACH has 
**      been detected by the Channel Decoder.
**
** Data Store I/O:
**     TDMA frame number - sysFrameNum.FN, Read only and placed
**        into the uplink buffers to indicate the frame on which the frame 
**        was recieved.
**
**     Unpacked BSIC array - g_codecBsic[0..5], Used for RACH decoding.
**        This array is initialized from g_BBInfo[trx].bsic in chanCodecInit.
**
**     For RACH channels, the following output buffer is updated:
**
**        g_ULRACHData.ulRACHword0.bitval.bufferValid
**        g_ULRACHData.ulRACHword0.bitval.rachDet
**        g_ULRACHData.ulRACHword0.bitval.spare
**        g_ULRACHData.ulRACHword0.bitval.bfi
**        g_ULRACHData.ulRACHword0.bitval.crc
**        g_ULRACHData.nErrs
**        g_ULRACHData.metric
**        g_ULRACHData.frameNumber
**        g_ULRACHData.data[i], for 0 <= i <= 7
**
**            The output data and array used by this routine 
**            is assummed in the following format with respect to bit order 
**            within the array. See GSM 05.03. For RACH, N = 8.
**
**            polynomial power   D^N-1  D^N-2  ......  D^2     D       1
**            data/parity bits   d(0)   d(1)   ......  d(N-3)  d(N-2)  d(N-1)
**            array index        0      1      ......  N-3     N-2     N-1
**
**     For CCH channels, the following output buffer is updated:
**
**        g_ULCCHData.ulCCHword0.bitval.bufferValid
**        g_ULCCHData.ulCCHword0.bitval.spare
**        g_ULCCHData.ulCCHword0.bitval.fireStatus
**        g_ULCCHData.nErrs
**        g_ULCCHData.metric
**        g_ULCCHData.frameNumber
**        g_ULCCHData.data[i], for 0 <= i <= 7
**
**            The output data array is in the same format as desribed above
**            for the RACH case.  For CCH, N = 184.
**
*****************************************************************************/
                              
Uint8 g_useFireDecoder = 0;

Bool chanDecoder(t_chanSel chanSel, Bool frameBoundary, UInt TN)
{
   Char parityBits[MAX_PARITY_BITS];
   Char cnvDecInData[2*NUM_GPRS_CS3_CNV_ENC_IN_BITS];
   Char cnvDecOutData[NUM_GPRS_CS3_CNV_ENC_IN_BITS];
   Char cnvEncOutTestData[2*NUM_GPRS_CS3_CNV_ENC_IN_BITS];
   UInt totalErrors;
   UInt metric;
   Bool parityError;
   Uint16 i,j;
   Bool achDetect;
   Uint8 trx;
   Uint8 cs1Errors;
   Uint8 codecFlags;
   Uint16 tempUsfCode;
   UInt ii,jj;
   Bool mask;
   Uint8 tempBuf[48];   
   trx = 0;     
   achDetect = FALSE;
   
     
	
   /*
   * Based on chanSel and TN load the appropriate deinterleaver 
   * buffer with the demodulated burst data
   */
   deinterleaverLoader(chanSel, TN, frameBoundary);

   if(frameBoundary)
   {
#if PRACH_TEST == 1
    if(chanSel == RACH) 
      chanSel = PRACH;    
#endif
      switch(chanSel)
      {
         case TCHF:
            /*
            * This could be TCHF of FACCH channel, depending on the stealing
            * flags recovered from the received frame.
            */

            /*
            * Execute the deinterleaving.  Returns # of stealing flags that
            * were TRUE out of 8 max.  Any value other than 0 or 8 indicate
            * bit errors.
            */
            totalErrors = blockDiagDeinterleaver8(g_deintBuf8Deep.data[TN],
                                      &g_deintBuf8Deep.tchRXBurstIndex[TN],
                                      cnvDecInData);

            /*
            * Threshold stealing flags and decide if TCH of FACCH is received.
            */
            if (totalErrors > STEALING_FLAG_THRESH)
            {
               /*
               * Frame is FACCH
               */
               totalErrors = MAX_STEALING_FLAGS - totalErrors;

#if BER_TEST == 1
	         facchFrameCounter++;
#endif
#if FIRE_TEST == 1
             cnvDecInData[80] = -cnvDecInData[80];
             cnvDecInData[81] = -cnvDecInData[81];
             cnvDecInData[82] = -cnvDecInData[82];
             cnvDecInData[83] = -cnvDecInData[83];
             cnvDecInData[84] = -cnvDecInData[84];
             cnvDecInData[85] = -cnvDecInData[85];
             cnvDecInData[86] = -cnvDecInData[86];
             cnvDecInData[87] = -cnvDecInData[87];
#endif

               /*
               * Execute the cnv decoder
               */
               metric = cnvDecoder2(cnvDecInData, cnvDecOutData,
                                    NUM_CCH_CNV_DEC_OUT_BITS);

               /*
               * Perform the reconvolution and update the number of
               * estimated bit errors
               */
               cnvEncoder2(cnvDecOutData, cnvEncOutTestData,
                           NUM_CCH_CNV_ENC_IN_BITS);
               
/*  Call Optimized Assembly Function to Compute Errors */

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_CCH_CNV_ENC_OUT_BITS; i++)
               {
                  if((cnvDecInData[i]<0) != cnvEncOutTestData[i])
                  {
                     totalErrors += 1;
                  }
               }
#else
		   totalErrors += computeTotalErrors(&cnvEncOutTestData[0],
  					 &cnvDecInData[0],NUM_CCH_CNV_ENC_OUT_BITS);
#endif

#if BER_TEST == 1
	       if(facchFrameCounter > 22)
		{
		  totalfacchSamples += NUM_CCH_CNV_ENC_OUT_BITS;
		  totalfacchChanErrs += totalErrors;
		}
#endif        
               /*
               * Invoke the CRC Decoder
               */
            if(g_useFireDecoder && g_ULCCHFireData.fireStatus == NO_ERROR_CORRECT)
            {
				/*
				*  using fire decoder error detect
				*/
		    parityError = fireDecoderCCHDetect(trx, TN, chanSel, cnvDecOutData);

                if(g_ULCCHFireData.fireStatus == DO_ERROR_CORRECT2)
				{
				   /*
				   *  if a correctable error found, this buffer is not valid yet
				   */
/* new 7/6/00..hpines..always assume valid for diag purposes and reverse later */

				   g_ULCCHData.ulcchword0.bitval.bufferValid = TRUE;
				}
				else
				{
                   /*
				   *  otherwise, this buffer is valid
				   */
                           g_ULCCHData.ulcchword0.bitval.bufferValid = TRUE;
				}
			}
		    else
            {
                parityError = crcDecoderCCH(cnvDecOutData, parityBits);
				g_ULCCHData.ulcchword0.bitval.bufferValid = TRUE;
			}  

               /*
               * Update the TCH data buffer appropriately since the TCH
               * frame has been stolen.
               */
               g_ULTCHData.ultchword0.bitval.bufferValid = FALSE;
               g_ULTCHData.ultchword0.bitval.bfi = TRUE;

               /*
               * Update the output data buffer
               */
               g_ULCCHData.ulcchword0.bitval.bfi = parityError; /*is this needed?*/
#if BER_TEST == 1
	        if((facchFrameCounter > 22) && (!g_useFireDecoder))
	           badfacchFrameCounter += g_ULCCHData.ulcchword0.bitval.bfi;
#endif
               g_ULCCHData.ulcchword0.bitval.fireStatus = parityError;
               g_ULCCHData.ulcchword0.bitval.spare = 0;
               g_ULCCHData.ulcchword0.bitval.spare2 = 0;
               g_ULCCHData.nErrs = totalErrors;
               g_ULCCHData.metric = metric;
               g_ULCCHData.T1 = sysFrameNum.t1;
               g_ULCCHData.T2 = sysFrameNum.t2;
               g_ULCCHData.T3 = sysFrameNum.t3;
               achDetect = TRUE;
#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_BITS_CCH_FRAME; i++)
               {
                  g_ULCCHData.data[i] = cnvDecOutData[i];
               }
#else
               copyByteSamples( (Uint8 *) & cnvDecOutData[0], (Uint8 *) & g_ULCCHData.data[0],
                                NUM_BITS_CCH_FRAME);
#endif
            }
            else
            {
#if BER_TEST == 1
	         tchFrameCounter++;
#endif
               /*
               * Frame is TCH - Execute the cnv decoder
               */
               metric = cnvDecoder2(cnvDecInData, cnvDecOutData,
                                    NUM_TCH_CNV_DEC_OUT_BITS);

               /*
               * Perform the reconvolution and update the number
               * of estimated bit errors
               */
               cnvEncoder2(cnvDecOutData, cnvEncOutTestData,
                           NUM_TCH_CNV_ENC_IN_BITS);
               
 /*  Call Optimized Assembly Function to Compute Errors */

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_TCH_CNV_ENC_OUT_BITS; i++)
               {
                  if((cnvDecInData[i]<0) != cnvEncOutTestData[i])
                  {
                     totalErrors += 1;
                  }
               }
#else
		   totalErrors += computeTotalErrors(&cnvEncOutTestData[0],
  					 &cnvDecInData[0],NUM_TCH_CNV_ENC_OUT_BITS);
#endif
               
               /*
               * Move the ClassIa bits into the result buffer.
               * Data is reordered per GSM 05.03 paragraph 3.1.2.1.
               */
               for(i=0; i<(NUM_TCH_CRC_IN_BITS/2); i++)
               {
                  g_ULTCHData.data[2*i]   = cnvDecOutData[i];
                  g_ULTCHData.data[2*i+1] =
                                  cnvDecOutData[TCH_REORDER_MAX_INDEX-i];
               }

               /*
               * Add the parity bits to the ClassIa bits for CRC check.
               */
               for(i=0; i<NUM_TCH_PARITY_BITS; i++)
               {
                  g_ULTCHData.data[i+NUM_TCH_CRC_IN_BITS] =
                                cnvDecOutData[i+(NUM_BITS_TCH_CLASSI/2)];
               }

               /*
               * Invoke the CRC Decoder
               */
               //parityError = crcDecoderTCH(g_ULTCHData.data, parityBits);
               
               parityError = crcDecoder16(& g_ULTCHData.data[0],     /* input data bits */
                            & g_ULTCHData.data[NUM_TCH_CRC_IN_BITS], /* input parity bits */
                            TCH_CRC_POLY,
                            NUM_TCH_PARITY_BITS,
                            NUM_TCH_CRC_IN_BITS,                            
                            0xFFFF); 
                              
               
               /*
               * Add the ClassIb bits to the result buffer.  Note that the
               * previously written parity bits are now over-written.
               */
               for(i=(NUM_TCH_CRC_IN_BITS/2); i<(NUM_BITS_TCH_CLASSI/2); i++)
               {
                  g_ULTCHData.data[2*i]   = cnvDecOutData[i];
                  g_ULTCHData.data[2*i+1] =
                                  cnvDecOutData[TCH_REORDER_MAX_INDEX-i];
               }

               /*
               * Add the ClassII bits to the decoded bits.
               */
               for(i=0; i<NUM_BITS_TCH_CLASSII; i++)
               {
                  g_ULTCHData.data[NUM_BITS_TCH_CLASSI+i] =
                     (cnvDecInData[NUM_TCH_CNV_DEC_IN_BITS+i] < 0) ? 1 : 0;
               }

               /*
               * Perform subjective decoding of speech parameters based
               * on speech algorithm in use.
               * Also, Check Parity Error within speech frame(GSM EFR)
               */          
               if( parityError == 0)
               {
                  parityError = sbjDecoder(g_ULTCHData.data,
                              g_BBInfo[trx].TNInfo[TN].u.group1.tch.vocAlgo, TN);
               }
               /*
               * Perform Bad Frame detection and decide perceptual masking
               */
               g_ULTCHData.ultchword0.bitval.bfi =
                     detectBFI(chanSel, parityError, metric, totalErrors, &mask);
               g_ULTCHData.ultchword0.bitval.mask = mask;

#if BER_TEST == 1
	        if(tchFrameCounter > 22)
	         {
	      	  totalTchSamples += NUM_TCH_CNV_ENC_OUT_BITS;
	      	  totalTchChanErrs += totalErrors;
                    if(totalErrors > 36)massiveTchFrameErr++;
            	 badTchFrameCounter += g_ULTCHData.ultchword0.bitval.bfi;
                   if(metric < 0x600) badTchMetricCounter++;
               }
#endif
               /*
               * Update the output data buffer
               */
               g_ULTCHData.ultchword0.bitval.bufferValid = TRUE;
               g_ULTCHData.ultchword0.bitval.parityStatus = parityError;
               g_ULTCHData.ultchword0.bitval.sid = 0;
               g_ULTCHData.ultchword0.bitval.spare = 0;
               g_ULTCHData.nErrs = totalErrors;
               g_ULTCHData.metric = metric; 
            }
            break;
         
         case SACCH:
         case SDCCH4:
         case SDCCH8:
         case BCCH:
         case PCH:
         case AGCH:    
            /*
            * These channels all use the common CCH channel decoding
            * algorithms
            */
               
#if BER_TEST == 1
	      cchFrameCounter++;
#endif
            /*
            * Execute the deinterleaving
            */
            totalErrors = blockRectDeinterleaver(g_deintBuf4Deep.data[TN],
                                     &g_deintBuf4Deep.cchRXBurstIndex[TN],
                                     cnvDecInData);
            
            /*
            * fix 7/1/99...correct total errors update for steal flags
            */
            totalErrors = MAX_STEALING_FLAGS - totalErrors;

#if FIRE_TEST == 2
            if(chanSel == SDCCH4)
            {
             cnvDecInData[80] = -cnvDecInData[80];
             cnvDecInData[81] = -cnvDecInData[81];
             cnvDecInData[82] = -cnvDecInData[82];
             cnvDecInData[83] = -cnvDecInData[83];
             cnvDecInData[84] = -cnvDecInData[84];
             cnvDecInData[85] = -cnvDecInData[85];
             cnvDecInData[86] = -cnvDecInData[86];
             cnvDecInData[87] = -cnvDecInData[87];
            }
#endif

            /*
            * Execute the cnv decoder
            */
            metric = cnvDecoder2(cnvDecInData, cnvDecOutData,
                                 NUM_CCH_CNV_DEC_OUT_BITS);

            /*
            * Perform the reconvolution and update the number
            * of estimated bit errors
            */
            cnvEncoder2(cnvDecOutData, cnvEncOutTestData,
                        NUM_CCH_CNV_ENC_IN_BITS);

/*  Call Optimized Assembly Function to Compute Errors */

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_CCH_CNV_ENC_OUT_BITS; i++)
               {
                  if((cnvDecInData[i]<0) != cnvEncOutTestData[i])
                  {
                     totalErrors += 1;
                  }
               }
#else
	      totalErrors += computeTotalErrors(&cnvEncOutTestData[0],
  					 &cnvDecInData[0],NUM_CCH_CNV_ENC_OUT_BITS);
#endif

            /*
            * Invoke the CRC Decoder
            */
            if(g_useFireDecoder && g_ULCCHFireData.fireStatus == NO_ERROR_CORRECT)
            {
				/*
				*  using fire decoder error detect
				*/
		    parityError = fireDecoderCCHDetect(trx, TN, chanSel, cnvDecOutData);

                if(g_ULCCHFireData.fireStatus == DO_ERROR_CORRECT2)
				{
				   /*
				   *  if a correctable error found, this buffer is not valid yet
				   */
/* new 7/6/00..hpines..always assume valid for diag purposes and reverse later */
				   g_ULCCHData.ulcchword0.bitval.bufferValid = TRUE;
				}
				else
				{
                   /*
				   *  otherwise, this buffer is valid
				   */
                   g_ULCCHData.ulcchword0.bitval.bufferValid = TRUE;
				}
			}
		    else
            {
                parityError = crcDecoderCCH(cnvDecOutData, parityBits);
				g_ULCCHData.ulcchword0.bitval.bufferValid = TRUE;
			}  

#if BER_TEST == 1
	       if(cchFrameCounter > 22)
		{
		  totalCchSamples += NUM_CCH_CNV_ENC_OUT_BITS;
		  totalCchChanErrs += totalErrors;
              if(!g_useFireDecoder)badCchFrameCounter += parityError;
              if(totalErrors > 36)massiveCchFrameErr++;
		}
#endif    
            /*
            * Update the output data buffer
            */
            g_ULCCHData.ulcchword0.bitval.bfi = parityError; /*is this needed?*/
            g_ULCCHData.ulcchword0.bitval.fireStatus = parityError;
            g_ULCCHData.ulcchword0.bitval.spare = 0;
            g_ULCCHData.ulcchword0.bitval.spare2 = 0;
            g_ULCCHData.nErrs = totalErrors;
            g_ULCCHData.metric = metric;
            g_ULCCHData.T1 = sysFrameNum.t1;
            g_ULCCHData.T2 = sysFrameNum.t2;
            g_ULCCHData.T3 = sysFrameNum.t3;

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_BITS_CCH_FRAME; i++)
               {
                  g_ULCCHData.data[i] = cnvDecOutData[i];
               }
#else
            copyByteSamples( (Uint8 *) & cnvDecOutData[0], (Uint8 *) & g_ULCCHData.data[0],
                                NUM_BITS_CCH_FRAME);
#endif
            break;
                    
         case PDCH:                    
         case PDTCH:            
#if BER_TEST == 1
	         pdchFrameCounter++;
#endif
            /*
            * These channels all use the common CCH channel decoding
            * algorithms
            */
               
            /*
            * Execute the deinterleaving
            */
            totalErrors = blockRectDeinterleaver(g_deintBuf8Deep.data[TN],
                                     &g_deintBuf8Deep.tchRXBurstIndex[TN],
                                     cnvDecInData);
#undef NOT_USED                                     

            /*
            *  Try to find out what kind of CODEC used
            */
            for(j=GPRS_CS_1; j<GPRS_CODEC_TYPE; j++)
            {                                                                              
               totalErrors = 0;
               codecFlags = g_ULCCHData.codecFlags ^ g_GPRSCodecFlags[j];
               /*
               *  it is a GPRS_CS_1 code
               */         
               for(i=0; i<8; i++)
               {
                  totalErrors += (codecFlags>>i) & 0x01;
               }
               if(totalErrors < 4)
               {
                 g_ULCCHData.chCodec = j;
                 break;
               }
            }

#ifdef  NOT_USED            
            /*
            *  Codec is either CS-1 or assigned other GPRS Codec
            */
//          codecFlags = g_ULCCHData.codecFlags ^ g_GPRSCodecFlags[GPRS_CS_1];
            /*
            *  it is a GPRS_CS_1 code ?
            */  
//          cs1Errors = 0;
                   
//          for(i=0; i<8; i++)
//          {
//            cs1Errors += (codecFlags>>i) & 0x01;
//          }          
            
//          codecFlags = g_ULCCHData.codecFlags ^ g_GPRSCodecFlags[g_ULCCHData.chCodec];
            /*
            *  or Assigned Codec
            */    
//          totalErrors = 0;
                 
//          for(i=0; i<8; i++)
//          {
//            totalErrors += (codecFlags>>i) & 0x01;
//          }          
                        

//          if(totalErrors >= cs1Errors)
//          {
//             g_ULCCHData.chCodec = GPRS_CS_1;
//              totalErrors = cs1Errors;           											           
//										
//												}
#endif  

            switch(g_ULCCHData.chCodec)
            {
             case GPRS_CS_1:

            /*
            * Execute the cnv decoder
            */
            metric = cnvDecoder2(cnvDecInData, cnvDecOutData,
                                 NUM_CCH_CNV_DEC_OUT_BITS);

            /*
            * Perform the reconvolution and update the number
            * of estimated bit errors
            */ 

            cnvEncoder2(cnvDecOutData, cnvEncOutTestData,
                        NUM_CCH_CNV_ENC_IN_BITS);

/*  Call Optimized Assembly Function to Compute Errors */

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_CCH_CNV_ENC_OUT_BITS; i++)
               {
                  if((cnvDecInData[i]<0) != cnvEncOutTestData[i])
                  {
                     totalErrors += 1;
                  }
               }
#else
	      totalErrors += computeTotalErrors(&cnvEncOutTestData[0],
  					 &cnvDecInData[0],NUM_CCH_CNV_ENC_OUT_BITS);
#endif
            /*
            * Invoke the CRC Decoder
            */

            parityError = crcDecoderCCH(cnvDecOutData, parityBits);

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_BITS_CCH_FRAME; i++)
               {
                  g_ULCCHData.data[i] = cnvDecOutData[i];
               }
#else
            copyByteSamples( (Uint8 *) & cnvDecOutData[0], (Uint8 *) & g_ULCCHData.data[0],
                               NUM_BITS_CCH_FRAME);
#endif

            break;   /* end GPRS-CS1 */

         case GPRS_CS_2:

/*
*GPRS...replace punctured bits with 0
*/ 
#ifdef NOT_ASM_OPTIMIZED
		 j = 0;
             ii = 15;
     		 jj = 39;
	       for(i=0; i < (2*NUM_GPRS_CS2_CNV_ENC_IN_BITS);i++)
	       {
               if(i != ii)
               {
	          cnvEncOutTestData[i] = cnvDecInData[j];
                j++;
               }
               else
               {
	          cnvEncOutTestData[i] = 0;
		    ii += 4;
                if(ii == jj)
                 {
                   jj += 48;
                   ii += 4;
                 }
		   }
             }
#else
 	      restorePunctureGPRS(&cnvDecInData[0],&cnvEncOutTestData[0],GPRS_CS_2);
#endif
            /*
            * Execute the cnv decoder
            */
            metric = cnvDecoder2(cnvEncOutTestData, cnvDecOutData,
                                 NUM_GPRS_CS2_CNV_ENC_IN_BITS );

            /*
            * Perform the reconvolution and update the number
            * of estimated bit errors
            */

            cnvEncoder2(cnvDecOutData, cnvEncOutTestData,
                        NUM_GPRS_CS2_CNV_ENC_IN_BITS );

/*
*GPRS...puncture the convolutionally encoded bits
*/ 
#ifdef NOT_ASM_OPTIMIZED
		      j = 0;
        ii = 15;
     		 jj = 39;
	       for(i=0; i < (2*NUM_GPRS_CS2_CNV_ENC_IN_BITS); i++)
	       {
           if(i != ii)
           {
	             cnvEncOutTestData[j] = cnvEncOutTestData[i];
              j++;
           }
           else
           {
		            ii += 4;
              if(ii == jj)
               {
                  jj += 48;
                  ii += 4;
               }
           }
        }


#else
        punctureGPRS(&cnvEncOutTestData[0],&cnvEncOutTestData[0],GPRS_CS_2);
#endif

/*  Call Optimized Assembly Function to Compute Errors */

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_CCH_CNV_ENC_OUT_BITS; i++)
               {
                  if((cnvDecInData[i]<0) != cnvEncOutTestData[i])
                  {
                     totalErrors += 1;
                  }
               }
#else
	         totalErrors += computeTotalErrors(&cnvEncOutTestData[0],
  					 &cnvDecInData[0],NUM_CCH_CNV_ENC_OUT_BITS);
#endif
           /*
           *  decode precoded first 3 bits of a RLC Packet for CS2
           */
              tempUsfCode = 0;
              packToByteBuffer(cnvDecOutData, (Uint8 *) & tempUsfCode, 6);  
                               
              for(i=0; i<8; i++)
              {  
                 if(tempUsfCode == USFCodingTableCS23[i])
                 {
                    unpackFromByteBuffer( (Uint8 *) & i, & cnvDecOutData[3], 3);
                    break;
                 }
              }

               parityError = crcDecoder16(& cnvDecOutData[3],  
                            & cnvDecOutData[NUM_GPRS_CS2_CRC_IN_BITS], 
                            GPRS16_CRC_POLY,
                            NUM_GPRS_CS2_PARITY_BITS,
                            NUM_BITS_GPRS_CS2,                            
                            0xFFFF); 

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_GPRS_CS2_CRC_IN_BITS; i++)
               {
                  g_ULCCHData.data[i] = cnvDecOutData[i];
               }
#else
               copyByteSamples( (Uint8 *) & cnvDecOutData[3], (Uint8 *) & g_ULCCHData.data[0],
                            NUM_BITS_GPRS_CS2);                            
#endif

            break;   /* end GPRS-CS2 */

         case GPRS_CS_3:
/*
*GPRS...replace punctured bits with 0
*/ 
#ifdef NOT_ASM_OPTIMIZED
		 j = 0;
             ii = 15;
     		 jj = 17;
	       for(i=0; i < (2*NUM_GPRS_CS3_CNV_ENC_IN_BITS);i++)
	       {
               if((i != ii) && (i != jj))
               {
	          cnvEncOutTestData[i] = cnvDecInData[j];
                j++;
               }
               else
               {
	          cnvEncOutTestData[i] = 0;
                if(i == ii)
		     ii += 6;
                else
                 jj += 6;
		   }
             }
#else
 	      restorePunctureGPRS(&cnvDecInData[0],&cnvEncOutTestData[0],GPRS_CS_3);
#endif
            /*
            * Execute the cnv decoder
            */
            metric = cnvDecoder2(cnvEncOutTestData, cnvDecOutData,
                                 NUM_GPRS_CS3_CNV_ENC_IN_BITS );

            /*
            * Perform the reconvolution and update the number
            * of estimated bit errors
            */

            cnvEncoder2(cnvDecOutData, cnvEncOutTestData,
                        NUM_GPRS_CS3_CNV_ENC_IN_BITS );


/*
*GPRS...puncture the convolutionally encoded bits
*/ 

#ifdef NOT_ASM_OPTIMIZED
		 j = 0;
             ii = 15;
     		 jj = 17;
	       for(i=0; i < (2*NUM_GPRS_CS3_CNV_ENC_IN_BITS); i++)
	       {
               if((i != ii) && (i != jj))
               {
	          cnvEncOutTestData[j] = cnvEncOutTestData[i];
                j++;
               }
               else
               {
                if(i == ii)
		     ii += 6;
                else
                 jj += 6;
		   }
             }
#else
            punctureGPRS(&cnvEncOutTestData[0],&cnvEncOutTestData[0],GPRS_CS_3);
#endif

/*  Call Optimized Assembly Function to Compute Errors */

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_CCH_CNV_ENC_OUT_BITS; i++)
               {
                  if((cnvDecInData[i]<0) != cnvEncOutTestData[i])
                  {
                     totalErrors += 1;
                  }
               }
#else
	      totalErrors += computeTotalErrors(&cnvEncOutTestData[0],
  					 &cnvDecInData[0],NUM_CCH_CNV_ENC_OUT_BITS);
#endif


           /*
           *  decode precoded first 3 bits of a RLC Packet for CS3
           */
              tempUsfCode = 0;
              packToByteBuffer(cnvDecOutData, (Uint8 *) & tempUsfCode, 6);  
                               
              for(i=0; i<8; i++)
              {  
                 if(tempUsfCode == USFCodingTableCS23[i])
                 {
                    unpackFromByteBuffer( (Uint8 *) & i, & cnvDecOutData[3], 3);
                    break;
                 }
              }

               parityError = crcDecoder16(& cnvDecOutData[3],  
                            & cnvDecOutData[NUM_GPRS_CS3_CRC_IN_BITS], 
                            GPRS16_CRC_POLY,
                            NUM_GPRS_CS3_PARITY_BITS,
                            NUM_BITS_GPRS_CS3,                            
                            0xFFFF); 

#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_GPRS_CS3_CRC_IN_BITS; i++)
               {
                  g_ULCCHData.data[i] = cnvDecOutData[i];
               }
#else
               copyByteSamples( (Uint8 *) & cnvDecOutData[3], (Uint8 *) & g_ULCCHData.data[0],
                            NUM_BITS_GPRS_CS3);                            
#endif

            break;   /* end GPRS-CS3 */

         case GPRS_CS_4:
            /*
            * No cnv decoder for CS-4, so extract one's/zeros from softdecisions
            */
            for(i=0; i<NUM_CCH_CNV_ENC_OUT_BITS; i++)
            {
               if(cnvDecInData[i] < 0) 
			cnvDecInData[i] = 1;
               else	
  			cnvDecInData[i] = 0;
            }

           /*
           *  decode precoded first 3 bits of a RLC Packet for CS4
           */
              tempUsfCode = 0;
              packToByteBuffer(cnvDecInData, (Uint8 *) & tempUsfCode,  12);  
                               
              for(i=0; i<8; i++)
              {  
                 if(tempUsfCode == USFCodingTableCS4[i])
                 {
                    unpackFromByteBuffer( (Uint8 *) & i, & cnvDecInData[9], 3);
                    break;
                 }
              }

               parityError = crcDecoder16(& cnvDecInData[9],  
                            & cnvDecInData[NUM_GPRS_CS4_CRC_IN_BITS], 
                            GPRS16_CRC_POLY,
                            NUM_GPRS_CS4_PARITY_BITS,
                            NUM_BITS_GPRS_CS4,                            
/*                            NUM_GPRS_CS4_CRC_IN_BITS,       */                     
                            0xFFFF); 
#ifdef NOT_ASM_OPTIMIZED
               for(i=0; i<NUM_GPRS_CS4_CRC_IN_BITS; i++)
               {
                  g_ULCCHData.data[i] = cnvDecInData[i];
               }
#else
               copyByteSamples((Uint8 *) & cnvDecInData[9],  (Uint8 *) & g_ULCCHData.data[0],
                            NUM_BITS_GPRS_CS4);                            
#endif

             break;     /* end GPRS-CS4 */

            } /* end GPRS chCodec switch */
/*
												if(g_ULCCHData.codecFlags == g_GPRSCodecFlags[g_ULCCHData.chCodec] && g_ULCCHData.chCodec != 0 && parityError==0)
												{
												  tempBuf[0] = g_ULCCHData.codecFlags;
              tempBuf[1] = g_GPRSCodecFlags[g_ULCCHData.chCodec];
              tempBuf[2] = g_ULCCHData.chCodec;
              tempBuf[3] = parityError;

          	   sendDiagMsg(55, 0, 4, 4, tempBuf);   

            }
*/

#if BER_TEST == 1
	      if(pdchFrameCounter > 5)
		{
		  totalPdchSamples += NUM_CCH_CNV_ENC_OUT_BITS;
		  totalPdchChanErrs += totalErrors;
	        badPdchFrameCounter += parityError;
		}
#endif 
           /*
            * Update the output data buffer
            */
            g_ULCCHData.ulcchword0.bitval.bufferValid = TRUE;
            g_ULCCHData.ulcchword0.bitval.bfi = parityError; /*is this needed?*/
            g_ULCCHData.ulcchword0.bitval.fireStatus = parityError;
            g_ULCCHData.ulcchword0.bitval.spare = 0;
            g_ULCCHData.ulcchword0.bitval.spare2 = 0;
            g_ULCCHData.nErrs = totalErrors;
            g_ULCCHData.metric = metric;
            g_ULCCHData.T1 = sysFrameNum.t1;
            g_ULCCHData.T2 = sysFrameNum.t2;
            g_ULCCHData.T3 = sysFrameNum.t3;
            break;

         case RACH:
#if BER_TEST == 1
	         rachFrameCounter++;
#endif
            if((g_ULBurstData.toa > 8) || 
               (g_ULBurstData.snr < g_RachSnrThresh))
            {         
            	/*
            	* RCV power is too low, abort channel decoding
                  * new 10/6/99...added snr threshold 
            	*/
            	g_ULRACHData.ulRACHword0.bitval.bufferValid = FALSE;
            	break;
            }
            /*
            * Read the demod buffer and execute the cnv decoder
            */
            metric = cnvDecoder2(g_ULBurstData.data, cnvDecOutData, NUM_RACH_CNV_DEC_OUT_BITS);

            /*
            * Perform the reconvolution and update the number of estimated bit errors
            */
            cnvEncoder2(cnvDecOutData, cnvEncOutTestData, NUM_RACH_CNV_ENC_IN_BITS);

            totalErrors = 0;
            for(i=0; i<NUM_RACH_CNV_ENC_OUT_BITS; i++)
            {
               if((g_ULBurstData.data[i]<0) != cnvEncOutTestData[i])
               {
                  totalErrors += 1;
               }
            }

            /*
            * Modulo 2 add the bsic to the recieved "parity" bits to recover the original
            * crc parity bits
            */
            for(i=0; i<NUM_RACH_PARITY_BITS; i++)
            {
               cnvDecOutData[NUM_BITS_RACH_FRAME+i] =
                               (cnvDecOutData[NUM_BITS_RACH_FRAME+i] +
                                g_codecBsic[i]) & MODULO_2_MASK;
            } 
            
            /*
            * Invoke the CRC Decoder
            */ 
            if(totalErrors < RACH_BIT_ERROR_MAX)
            {
               //parityError = crcDecoderRACH(cnvDecOutData, parityBits);
               
               parityError = crcDecoder16(cnvDecOutData,            /* input data bits */
                            & cnvDecOutData[NUM_RACH_CRC_IN_BITS],        /* input parity bits */
                            RACH_CRC_POLY,
                            NUM_RACH_PARITY_BITS,
                            NUM_RACH_CRC_IN_BITS,                            
                            0xFFFF); 
                              
        
            }
            else
               parityError = TRUE;

#if BER_TEST == 1
		{
		  totalRachSamples += 2*NUM_RACH_CNV_ENC_IN_BITS;
		  totalRachChanErrs += totalErrors;
	        badRachFrameCounter += parityError;
		}
#endif 
            /*
            * Perform thresholding to determine if a RACH was recieved
            */
            achDetect = detectBFI(chanSel, parityError, metric, totalErrors, &mask);

            /*
            * Update the output data buffer
            */
            g_ULRACHData.ulRACHword0.bitval.bufferValid = TRUE;
            g_ULRACHData.ulRACHword0.bitval.rachDet = !achDetect;
            g_ULRACHData.ulRACHword0.bitval.bfi = achDetect;
            g_ULRACHData.ulRACHword0.bitval.spare = 0;
            g_ULRACHData.nErrs = totalErrors;
            g_ULRACHData.metric = metric;
            g_ULRACHData.T1 = sysFrameNum.t1;
            g_ULRACHData.T2 = sysFrameNum.t2;
            g_ULRACHData.T3 = sysFrameNum.t3;
            for(i=0; i<NUM_BITS_RACH_FRAME; i++)
            {
               g_ULRACHData.data[i] = cnvDecOutData[i];
            }
            break;

         case PRACH:
         case PTCCH:
            if((g_ULBurstData.toa > 8) || 
               (g_ULBurstData.snr < g_RachSnrThresh))
            {         
            	/*
            	* RCV power is too low, abort channel decoding
                  * new 10/6/99...added snr threshold 
            	*/
            	g_ULRACHData.ulRACHword0.bitval.bufferValid = FALSE;
            	break;
            }

#if BER_TEST == 1
	         prachFrameCounter++;
#endif

/*
*GPRS...replace punctured bits with 0
*/ 
		 j = 0;
             ii = 0;
	       for(i=0; i < 2*(NUM_RACH_CNV_ENC_IN_BITS+NUM_RACH_GPRS_ADD_BITS); i++)
	       {
               if(i != GPRSPrachPunctureBits[ii])
               {
 	          cnvEncOutTestData[i] = g_ULBurstData.data[j];
               j++;
               }
               else
               {
	          cnvEncOutTestData[i] = 0;
		    ii++;
		   }
             }

            /*
            * Read the demod buffer and execute the cnv decoder
            */
            metric = cnvDecoder2(cnvEncOutTestData, cnvDecOutData, 
                       NUM_RACH_CNV_DEC_OUT_BITS+NUM_RACH_GPRS_ADD_BITS);

            /*
            * Perform the reconvolution and update the number of estimated bit errors
            */
            cnvEncoder2(cnvDecOutData, cnvEncOutTestData,
                       NUM_RACH_CNV_ENC_IN_BITS+NUM_RACH_GPRS_ADD_BITS);

/*
*GPRS...puncture the convolutionally encoded bits
*/ 
		 j = 0;
             ii = 0;
	       for(i=0; i < 2*(NUM_RACH_CNV_ENC_IN_BITS+NUM_RACH_GPRS_ADD_BITS); i++)
	       {
               if(i != GPRSPrachPunctureBits[ii])
               {
	          cnvEncOutTestData[j] = cnvEncOutTestData[i];
                j++;
               }
               else
		    ii++;
             }

            totalErrors = 0;
            for(i=0; i<NUM_RACH_CNV_ENC_OUT_BITS; i++)
            {
               if((g_ULBurstData.data[i]<0) != cnvEncOutTestData[i])
               {
                  totalErrors += 1;
               }
            }
            /*
            * Modulo 2 add the bsic to the recieved "parity" bits to recover the original
            * crc parity bits
            */
            for(i=0; i<NUM_RACH_PARITY_BITS; i++)
            {
               cnvDecOutData[NUM_BITS_PRACH_FRAME+i] =
                               (cnvDecOutData[NUM_BITS_PRACH_FRAME+i] +
                                g_codecBsic[i]) & MODULO_2_MASK;
            } 
            
            /*
            * Invoke the CRC Decoder
            */ 
            if(totalErrors < RACH_BIT_ERROR_MAX)
            {
               //parityError = crcDecoderRACH(cnvDecOutData, parityBits);
               
               parityError = crcDecoder16(cnvDecOutData,            /* input data bits */
                            & cnvDecOutData[NUM_PRACH_CRC_IN_BITS],        /* input parity bits */
                            RACH_CRC_POLY,
                            NUM_RACH_PARITY_BITS,
                            NUM_PRACH_CRC_IN_BITS,                            
                            0xFFFF); 
                              
        
            }
            else
               parityError = TRUE;

#if BER_TEST == 1
/*	       if(rachFrameCounter > 22) */
		{
		  totalPrachSamples += 2*(NUM_RACH_CNV_ENC_IN_BITS+NUM_RACH_GPRS_ADD_BITS);
		  totalPrachChanErrs += totalErrors;
	        badPrachFrameCounter += parityError;
		}
#endif 
            /*
            * Perform thresholding to determine if a RACH was recieved
            */
            achDetect = detectBFI(RACH, parityError, metric, totalErrors, &mask);

          
                    

            /*
            * Update the output data buffer
            */
            g_ULRACHData.ulRACHword0.bitval.bufferValid = TRUE;
            g_ULRACHData.ulRACHword0.bitval.rachDet = !achDetect;
            g_ULRACHData.ulRACHword0.bitval.bfi = achDetect;
            g_ULRACHData.ulRACHword0.bitval.spare = 0;
            g_ULRACHData.nErrs = totalErrors;
            g_ULRACHData.metric = metric;
            g_ULRACHData.T1 = sysFrameNum.t1;
            g_ULRACHData.T2 = sysFrameNum.t2;
            g_ULRACHData.T3 = sysFrameNum.t3;
            for(i=0; i<NUM_BITS_PRACH_FRAME; i++)
            {
               g_ULRACHData.data[i] = cnvDecOutData[i];
            }
            break;
         default:
            /*
            * Other channel types are not yet supported, return error
            * message
            */
            break;
      }
   }

   return (achDetect);
}

/*****************************************************************************
** Routine Name: chanCodecInit
**
** Description: 
**   Initializes the channel CODEC.
**
** Method:
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**    None
**
** Returns: 
**    None
**
** Data Store I/O: 
**     Packed BSIC - g_BBInfo[trx].bsic, Used for RACH encoding and decoding.
**        This value is used to initialize the unpacked bsic array 
**        g_codecBsic[0..5] which will be used by the Channel CODEC for RACH
**        channels.  The format of the bsic is as follows, see GSM 05.03 
**        for more details:
**
**                MSB                           LSB  
**                 0  0    b0  b1  b2    b3  b4  b5 
**                        PLMN(3 bits)   BS(3 bits)   
**
*****************************************************************************/

void chanCodecInit(void)
{
   UInt i, j, k;
   Bool bsicBit;
   Uint8 trx;
   /*
   * Init the 4 deep interleaver and deinterleaver buffers
   */
   for(k=0; k<NUM_TN_PER_RF; k++)
   {
      for(i=0; i<NUM_BURSTS_PER_CCH_BLOCK; i++)
      {
         for(j=0; j<NB_NUM_ENCRYPT_BITS_PER_BURST; j++)
         {
            g_intBuf4Deep.data[k][i][j] = 0;
            g_deintBuf4Deep.data[k][i][j] = 0;
         }
      }
   }

   /*
   * Init the 8 deep interleaver and deinterleaver buffers
   */
   for(k=0; k<NUM_TN_PER_RF; k++)
   {
      for(i=0; i<NUM_BURSTS_PER_TCH_BLOCK; i++)
      {
         for(j=0; j<NB_NUM_ENCRYPT_BITS_PER_BURST; j++)
         {
            g_intBuf8Deep.data[k][i][j] = 0;
            g_deintBuf8Deep.data[k][i][j] = 0;
         }
      }
   }

   /*
   * Init the interleaving and deinterleaving burst indexes
   */
   for(i=0; i<NUM_TN_PER_RF; i++)
   {
      g_intBuf4Deep.cchTXBurstIndex[i]   = -1;
      g_deintBuf4Deep.cchRXBurstIndex[i] = -1;
      g_intBuf8Deep.tchTXBurstIndex[i]   = -1;
      g_deintBuf8Deep.tchRXBurstIndex[i] = -1;
   }

   /*
   * Load the g_codecBsic variable based on the packed g_bsic variable
   */    
   for(trx=0; trx<NUM_RFS_PER_DSP; trx++)
   for(i=0; i<BSIC_LENGTH; i++)
   {
      bsicBit = (g_BBInfo[trx].bsic >> i) & 0x0001;
      g_codecBsic[BSIC_LENGTH-1-i] = bsicBit;
   }
/*
* Init GPRS codec bits per coding class and associated flags 
*/
    g_GPRSCodecFlags[GPRS_CS_1] = FRAME_FLAGS_CS1;
    g_GPRSCodecFlags[GPRS_CS_2] = FRAME_FLAGS_CS2;
    g_GPRSCodecFlags[GPRS_CS_3] = FRAME_FLAGS_CS3;
    g_GPRSCodecFlags[GPRS_CS_4] = FRAME_FLAGS_CS4;
    g_GPRSCodecBits[GPRS_CS_1] = NUM_BITS_GPRS_CS1; 
    g_GPRSCodecBits[GPRS_CS_2] = NUM_BITS_GPRS_CS2; 
    g_GPRSCodecBits[GPRS_CS_3] = NUM_BITS_GPRS_CS3; 
    g_GPRSCodecBits[GPRS_CS_4] = NUM_BITS_GPRS_CS4; 
}

/********* Old revision notes **************************************
**
** Revision 1.14  2000-01-12 10:17:03-08  whuang
** Added CRC encoder/Decoder, subjective encoder/decoder and VOIP format for EFR. No uplink frame is sent out, if CRC error is found for EFR also. Created a common CRC encoder/decoder for SCH, TCH, RA
**
** Revision 1.13  1999-11-04 14:54:27-08  ltravaglione
** <>
**
** Revision 1.11  1999-10-28 13:29:00-07  whuang
** easy RACH detection S/N threshold setting from 39 to 35
**
** Revision 1.10  1999-10-19 11:49:55-07  whuang
** Replaced Power constrain with TOA in RACH detection
**
** Revision 1.9  1999-10-18 18:53:38-07  hpines
** Added enhanced RACH detection test...test post equalized RACH training sequence bits versus their reference values.
**
** Revision 1.8  1999-09-07 17:42:58-07  whuang
** added UL/DL DTX mode handling functions
**
** Revision 1.7  1999-07-19 08:49:43-07  dkaiser
** deleted some long time non-functional commented-out lines
**
** Revision 1.6  1999-07-07 16:33:16-07  hpines
** Fixed "total_errors" update bugs:
** 1.  removed effects of steal flags in CCH messages
** 2.  compare "CNV_ENC_OUT_BITS"  instead of "CNV_ENC_IN_BITS" in 4 separate places where total errors is updated.
**
** Revision 1.5  1999-04-26 20:28:31-07  whuang
** Added Controls for RACH detection
**
** Revision 1.4  1999-03-11 15:55:59-08  whuang
** <>
**
** Revision 1.3  1999-03-10 17:43:30-08  whuang
** Corrected using "RFA" for TRX number
**/

