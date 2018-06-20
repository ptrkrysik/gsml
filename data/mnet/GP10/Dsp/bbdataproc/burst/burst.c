/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
**
**  File: burst.c
**
**  Description:   
**    This file contains the GSM GMSK modulator routine.
**
*****************************************************************************/

/*****************************************************************************
** ROUTINE NAME: burst
**
** Description:
**   GSM burst builder.
**
** Method:
**   Up to 116 input bits are combined with known bit patterns for each burst type
**   to build the indicated burst for the modulator.  All bursts are generated
**   148 bits long.
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**    waveform - output array of the modulated waveform
**    bits - input bits array
**    bursttype - input indicating which burst type
**
** Returns: 
**    None
**
** Data Store  I/O: 
**
*****************************************************************************/
#include "gsmdata.h"
#include "bbdata.h"
#include "bbproto.h"
#include "burst.h"

void buildGSMBurst(t_chanSel chanSel, UChar TSEQ)
{
   Int16 i, j;
   Uint8 *modBitBuf;
   /*
   * Check transmission flag and pass along to the modulator.
   * Clear the coded data flag.
   */

   /*
   * Determine burst type and build appropriate burst
   */
   j = 0;
   switch (chanSel)
   {   
        
      case FCCH:
         /*
         * Tail bits
         */
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         
         /*
         * Fixed bits
         */
         for (i=0; i<FB_NUM_FIXED_BITS; i++)
         {
            g_DLBurstData.data[j++] = 0;
         }
         
         /*
         * Tail bits
         */
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         break;
         
      case SCH:
         /*
         * Tail bits
         */
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         
         /*
         * Encrypted bits
         */
         for (i=0; i<SB_NUM_ENCRYPT_BITS_PER_SIDE; i++)
         {
            g_DLBurstData.data[j++] = g_DLCodedData.data[i];
         }
         
         /*
         * Training sequence bits
         */
         for (i=0; i<SB_TRAIN_SIZE; i++)
         {
            g_DLBurstData.data[j++] = trainsb[i];
         }
         
         /*
         * Encrypted bits
         */
         for (i=SB_NUM_ENCRYPT_BITS_PER_SIDE; i<SB_NUM_ENCRYPT_BITS_PER_BURST; i++)
         {
            g_DLBurstData.data[j++] = g_DLCodedData.data[i];
         }
         
         /*
         * Tail bits
         */
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         break;
         
      case RACH:
         /*
         * Extended tail bits
         */
         for (i=0; i<AB_NUM_TAIL_BITS; i++)
         {
            g_DLBurstData.data[j++] = taila[i];
         }
         
         /*
         * Synch sequence bits
         */
         for (i=0; i<AB_TRAIN_SIZE; i++)
         {
            g_DLBurstData.data[j++] = syncseq[i];
         }
         
         /*
         * Encrypted bits
         */   
         for (i=0; i<AB_NUM_ENCRYPT_BITS_PER_BURST; i++)
         {
            g_DLBurstData.data[j++] = g_DLCodedData.data[i];
         }
         
         /*
         * Tail bits
         */
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
             
         break;

      default:   /* Normal burst */
         
         if(g_DLCodedData.tx == FALSE)
         {
            /* trasnmitter if off, no burst is loaded */
            g_DLBurstData.tx = FALSE;
            return;
         }

         
         if(chanSel == IDLE)        
         {
            modBitBuf = (Uint8*)mixed;
         }
         else 
         {
            modBitBuf = g_DLCodedData.data;
         }
       
         /*
         * Tail bits
         */
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         
         /*
         * Encrypted bits
         */
         for (i=0; i<NB_NUM_ENCRYPT_BITS_PER_SIDE; i++)
         {
            g_DLBurstData.data[j++] = modBitBuf[i];
         }
         
         /*
         * Training sequence bits
         */
         for (i=0; i<NB_TRAIN_SIZE; i++)
         {
            g_DLBurstData.data[j++] = trainnb[TSEQ][i];
         }
         
         /*
         * Encrypted bits
         */
         for (i=NB_NUM_ENCRYPT_BITS_PER_SIDE; i<NB_NUM_ENCRYPT_BITS_PER_BURST; i++)
         {
            g_DLBurstData.data[j++] = modBitBuf[i];
         }
         
         /*
         * Tail bits
         */
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         g_DLBurstData.data[j++] = 0;
         
         break;         
      }
         
      g_DLBurstData.tx = TRUE;
}
