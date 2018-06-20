/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
** File: intleavs.c
**
** Description:   
**   This file contains low level interleaving and deinterleaving (and burst 
**   mapping) routines in support of GSM channel coding and decoding.
**   Note: Old revision log is at the end of this file.
**
*****************************************************************************/
#include "gsmdata.h" 
#include "bbdata.h" 
#include "bbproto.h"  
#include "codec.h"

#define STEALING (NB_NUM_ENCRYPT_BITS_PER_SIDE-1)

/* optimized assembly routine prototypes */

void intrleav8(Char *in, Bool steal, Int *bindx, Char *out);
void intrleav4(Char *in, Bool steal, Int *bindx, Char *out);
void dntrleav8(Char *in, Char *out);
void dntrleav4(Char *in, Char *out);


/*****************************************************************************
** Routine Name: blockDiagInterleaver8
**
** Description: 
**   Block interleaving and burst mapping for control channels.   
**
** Method:
**   GSM 05.03 sections 4.1.4 and 4.1.5
**
**   Modulo operations are very expensive and for powers of 2,
**   a bitwise AND operation can be used to implement the mod
**   function.  The following two operations are equivalent:
** 
**       k % 4;
**       k & MODULO_4_MASK; 
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/

void blockDiagInterleaver8(Char *input, Bool stealFlag, Int *burstIndex,
	 Char output[NUM_BURSTS_PER_TCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST])
{
   UShort B, k, j;
   UShort block;
   UShort k49mod57;
   
   /*
   * If the burst index hasn't been initialized (this is the 1st
   * occurence of a frame boundary), initialize it now.
   */
   if (*burstIndex < 0)
   {
      *burstIndex = 0;
   }

#ifdef NOT_ASM_OPTIMIZED

   /*
   * Determine if even/odd bits are used in hi/lo bursts
   * if Block=0 --> even bursts:0..3,   odd  bursts:4..7
   *       else __> odd  bursts:0..3,   even bursts:4..7
   */
   block = (*burstIndex>>2) & 0x1;
   
   /*
   * Perform the interleaving and burst mapping as described in 
   * GSM 05.03 section 4.1.3.
   */
   k49mod57 = 0;
   for(k=0; k<CODEC_FRAME_SIZE; k++) 
   {                                                     /* k=0,1,....455 */
      B = (4*block + k) & MODULO_8_MASK;                 /* B=0,1...7 */

	  /*
	  * The following LOC is equivalent to:
	  *
      * j = 2*((49*k) % 57) + (k % 8) / 4;
	  */
	  j = 2*k49mod57 + ((k & MODULO_8_MASK)>>2); /* j = 0...113 */
      output[B][j] = input[k];    

	  k49mod57 += 49;
	  if (k49mod57 > 56) k49mod57 -= 57;
   }       


   /*
   * Move hi side to make room for stealing flags.  Only move
   * the even bits for each even-bit burst.  Only move
   * the odd bits for each odd-bit burst.
   */
   if (!block)
   {
      output[4][115] = output[4][113];    
      output[5][115] = output[5][113];    
      output[6][115] = output[6][113];    
      output[7][115] = output[7][113];    

      for(j=STEALING-1; j>STEALING/2; j--) 
      { 
	 output[0][2*j+2] = output[0][2*j];    
	 output[1][2*j+2] = output[1][2*j];    
	 output[2][2*j+2] = output[2][2*j];    
	 output[3][2*j+2] = output[3][2*j];    

	 output[4][2*j+1] = output[4][2*j-1];    
	 output[5][2*j+1] = output[5][2*j-1];    
	 output[6][2*j+1] = output[6][2*j-1];    
	 output[7][2*j+1] = output[7][2*j-1];    
      }
   }
   else
   {
      output[0][115] = output[0][113];    
      output[1][115] = output[1][113];    
      output[2][115] = output[2][113];    
      output[3][115] = output[3][113];    

      for(j=STEALING-1; j>STEALING/2; j--) 
      { 
	 output[4][2*j+2] = output[4][2*j];    
	 output[5][2*j+2] = output[5][2*j];    
	 output[6][2*j+2] = output[6][2*j];    
	 output[7][2*j+2] = output[7][2*j];    

	 output[0][2*j+1] = output[0][2*j-1];    
	 output[1][2*j+1] = output[1][2*j-1];    
	 output[2][2*j+1] = output[2][2*j-1];    
	 output[3][2*j+1] = output[3][2*j-1];    
      }
   }
      

   /*
   * Set the stealing flags (hu and hl) to "1" as indicated in GSM 05.03
   * section 3.1.4 for the CCH channels.
   */
   if(block == 0)
      for(B=0; B<4; B++)   
      {
	 output[B][58]   = stealFlag;
	 output[B+4][57] = stealFlag;   
      }    
   else
      for(B=0; B<4; B++) 
      {
	 output[B][57]   = stealFlag;       
	 output[B+4][58] = stealFlag;       
      }  
#else
  /*
  *** Use Optimized assembly routine***
  */
   intrleav8(&input[0], stealFlag, &burstIndex[0],&output[0][0]) ;
#endif  /*  end old interleaver8 */
}

/*****************************************************************************
** Routine Name: blockDiagDeinterleaver8
**
** Description: 
**   Block deinterleaving and burst demapping for control channels.  
**
** Method:
**   GSM 05.03 sections 4.1.4 and 4.1.5
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/

UInt blockDiagDeinterleaver8(
	 Char input[NUM_BURSTS_PER_TCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST],
	 Int *burstIndex, Char *output)
{
   UShort B, k, j;
   UShort block;
   UInt steals;
   UShort k49mod57;
   
   /*
   * If the burst index hasn't been initialized (this is the 1st
   * occurance of a frame boundary), initialize it now.
   * Init to 7 for de-interleaver since 1st increment is before
   * the de-interleaver runs again.
   */
   if (*burstIndex < 0)
   {
      *burstIndex = 7;
   }

#ifdef NOT_ASM_OPTIMIZED

   /*
   * Determine if even/odd bits are used in hi/lo bursts
   * if Block=0 --> even bursts:0..3,   odd  bursts:4..7
   *       else __> odd  bursts:0..3,   even bursts:4..7
   */
   block = !((*burstIndex>>2) & 0x1);
   
   /*
   * Read the appropriate stealing flags, hu & hl, and count the number set out of
   * 8 possible.
   */
   steals = 0;
   if(block == 0)
      for(B=0; B<4; B++)   
      {
	 if (input[B][58] < 0) steals++;
	 if (input[B+4][57] < 0) steals++;
      }    
   else
      for(B=0; B<4; B++) 
      {
	 if (input[B][57] < 0) steals++;
	 if (input[B+4][58] < 0) steals++;
      }  


   /*
   * Move hi side over stealing flags to allow continuous read
   * in the loop below.  Only move the even bits for each
   * even-bit burst.  Only move the odd bits for each odd-bit burst.
   */
   if (!block)
   {
      for(j=(STEALING+2)/2; j<STEALING; j++) 
      { 
	 input[0][2*j] = input[0][2*j+2];    
	 input[1][2*j] = input[1][2*j+2];    
	 input[2][2*j] = input[2][2*j+2];    
	 input[3][2*j] = input[3][2*j+2];    

	 input[4][2*j-1] = input[4][2*j+1];    
	 input[5][2*j-1] = input[5][2*j+1];    
	 input[6][2*j-1] = input[6][2*j+1];    
	 input[7][2*j-1] = input[7][2*j+1];    
      }
      input[4][113] = input[4][115];    
      input[5][113] = input[5][115];    
      input[6][113] = input[6][115];    
      input[7][113] = input[7][115];    

   }
   else
   {
      for(j=(STEALING+2)/2; j<STEALING; j++) 
      { 
	 input[4][2*j] = input[4][2*j+2];    
	 input[5][2*j] = input[5][2*j+2];    
	 input[6][2*j] = input[6][2*j+2];    
	 input[7][2*j] = input[7][2*j+2];    

	 input[0][2*j-1] = input[0][2*j+1];    
	 input[1][2*j-1] = input[1][2*j+1];    
	 input[2][2*j-1] = input[2][2*j+1];    
	 input[3][2*j-1] = input[3][2*j+1];    
      }
      input[0][113] = input[0][115];    
      input[1][113] = input[1][115];    
      input[2][113] = input[2][115];    
      input[3][113] = input[3][115];    

   }

   
   /*
   * Perform the deinterleaving and burst demapping. The stealing flags
   * are not demapped.
   */
   k49mod57 = 0;
   for(k=0; k<CODEC_FRAME_SIZE; k++) 
   {                                       /* k = 0,1,....455 */
      B = (4*block + k) & MODULO_8_MASK;   /* B=0,1...7 */

	  /*
	  * The following LOC is equivalent to:
	  *
	  * j = 2*((49*k) % 57) + (k % 8) / 4;
	  */
	  j = 2*k49mod57 + ((k & MODULO_8_MASK)>>2); /* j = 0...113 */
      output[k] = input[B][j];

	  k49mod57 += 49;
	  if (k49mod57 > 56) k49mod57 -= 57;
   }  
#else

   /*
   *** Use Optimized assembly routine***
   * New 9/7/99..fix buffering of stealing flags so facch interleaved
   * over 2 consecutive frames
   */
   steals = 0;
   for(B=0; B<4; B++)   
   {
    if (input[B][114] < 0) steals++;  /* current frame steal flags */
    if (input[5][112+B] < 0) steals++; /* saved steal flags previous frame */
    input[5][112+B] = input[B][115];
   }    
   dntrleav8(&input[0][0], &output[0]) ;

#endif  /*  end old deinterleaver8 */
   return(steals);
}

/*****************************************************************************
** Routine Name: blockRectInterleaver
**
** Description: 
**   Block interleaving and burst mapping for control channels.   
**
** Method:
**   GSM 05.03 sections 4.1.4 and 4.1.5
**
**   Modulo operations are very expensive and for powers of 2,
**   a bitwise AND operation can be used to implement the mod
**   function.  The following two operations are equivalent:
** 
**       k % 4;
**       k & MODULO_4_MASK; 
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/

void blockRectInterleaver(Char *input, Int *burstIndex,
	 Char output[NUM_BURSTS_PER_CCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST])
{
   UShort j;
   UInt k, B;               
   UShort k49mod57;
   Bool stealFlag;
   Uint8 temp;
 
   /*
   * If the burst index hasn't been initialized (this is the 1st
   * occurance of a frame boundary), initialize it now.
   */
   if (*burstIndex < 0)
   {
      *burstIndex = 0;
   }

#ifdef NOT_ASM_OPTIMIZED

   /*
   * Perform the interleaving and burst mapping as described in 
   * GSM 05.03 section 4.1.4 and 4.1.5.
   */
   k49mod57 = 0;
   for(k=0; k<CODEC_FRAME_SIZE; k++) 
   {                                                     /* k=0,1,....455 */
      B = k & MODULO_4_MASK;                             /* B = 0,1,2,3 */  

	  /*
	  * The following LOC is equivalent to:
	  *
	  * j = 2*((49*k) % 57) + (k % 8) / 4;
	  */
	  j = 2*k49mod57 + ((k & MODULO_8_MASK)>>2); /* j = 0...113 */
      output[B][j] = input[k];    

	  k49mod57 += 49;
	  if (k49mod57 > 56) k49mod57 -= 57;
   }       
    

   /*
   * Move hi side to make room for stealing flags
   */
   for(j=2*STEALING+2-1; j>STEALING+2-1; j--) 
   { 
      output[0][j] = output[0][j-2];    
      output[1][j] = output[1][j-2];    
      output[2][j] = output[2][j-2];    
      output[3][j] = output[3][j-2];    
   }       


   /*
   * Set the stealing flags (hu and hl) to "1" as indicated in GSM 05.03
   * section 4.1.5 for the CCH channels.
   */
   for (B=0; B<4; B++)               
   {
      output[B][57] = 1;
      output[B][58] = 1;
   }
#else
   /*
   *** Use Optimized assembly routine***
   */

   stealFlag = 1;   /* for cch frames */
   intrleav4(&input[0], stealFlag, &burstIndex[0],&output[0][0]) ;

#endif  /*  end old interleaver4 */

   /*
   * if GPRS frame, stuff steal flag locations with GPRS codec flags..msbit in first
   */
/*   
   if (g_DLCCHData.codecFlags != 0)
   {  
*/
     temp = g_DLCCHData.codecFlags;
     for (B=0; B<4; B++)               
     {
	  stealFlag = (temp & 0x80) >> 7;
        output[B][57] = stealFlag & 1;
	  temp = temp << 1;
	  stealFlag = (temp & 0x80) >> 7;
        output[B][58] = stealFlag & 1;
	  temp = temp << 1;
     }
/*
   }
   output[0][57] = g_DLCCHData.codecFlags & 0x01;
   output[0][58] = (g_DLCCHData.codecFlags>>1) & 0x01;

   output[1][57] = (g_DLCCHData.codecFlags>>2) & 0x01;
   output[1][58] = (g_DLCCHData.codecFlags>>3) & 0x01;

   output[2][57] = (g_DLCCHData.codecFlags>>4) & 0x01;
   output[2][58] = (g_DLCCHData.codecFlags>>5) & 0x01;
            
   output[3][57] = (g_DLCCHData.codecFlags>>6) & 0x01;
   output[3][58] = (g_DLCCHData.codecFlags>>7) & 0x01;
*/   

}


/*****************************************************************************
** Routine Name: blockRectDeinterleaver
**
** Description: 
**   Block deinterleaving and burst demapping for control channels.  
**
** Method:
**   GSM 05.03 sections 4.1.4 and 4.1.5
**
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/

UInt blockRectDeinterleaver(Char input[NUM_BURSTS_PER_CCH_BLOCK][NB_NUM_ENCRYPT_BITS_PER_BURST],
			    Int *burstIndex, Char *output)
{
   UShort j;
   Uint16 k, B;    
   UInt stealFlagErrs;
   UShort k49mod57;

   /*
   * If the burst index hasn't been initialized (this is the 1st
   * occurance of a frame boundary), initialize it now.
   * Init to 7 for de-interleaver since 1st increment is before
   * the de-interleaver runs again.
   */
   if (*burstIndex < 0)
   {
      *burstIndex = 7;
   } 
#ifdef NOT_ASM_OPTIMIZED
 
   /*
   * Check the stealing flags for all four bursts. Any they are not a 
   * "+1" (a soft decision >=0) they are in error. Update the estimated 
   * error count to reflect this.
   */
   stealFlagErrs = 0;
   g_ULCCHData.codecFlags =0;     
   for (B=0; B<4; B++)               
   {
      g_ULCCHData.codecFlags <<= 1;   
      if(input[B][57] < 0)
      {
	     stealFlagErrs += 1;
	     g_ULCCHData.codecFlags += 1; 
	  }                           
	  
      g_ULCCHData.codecFlags <<= 1;	  
      if(input[B][58] < 0)
      {
	     stealFlagErrs += 1;
	     g_ULCCHData.codecFlags += 1; 
	  }  
   }


   /*
   * Move hi side over stealing flags to allow continuous read
   * in the loop below.
   */
   for(j=STEALING+2; j<2*STEALING+2; j++) 
   { 
      input[0][j-2] = input[0][j];    
      input[1][j-2] = input[1][j];    
      input[2][j-2] = input[2][j];    
      input[3][j-2] = input[3][j];    
   }       


   /*
   * Perform the deinterleaving and burst demapping. The stealing flags
   * are not demapped.
   */
   k49mod57 = 0;
   for(k=0; k<CODEC_FRAME_SIZE; k++) 
   {                                       /* k = 0,1,....455 */
      B = k & MODULO_4_MASK;               /* B = 0,1,2,3 */

	  /*
	  * The following LOC is equivalent to:
	  *
	  * j = 2*((49*k) % 57) + (k % 8) / 4;
	  */
	  j = 2*k49mod57 + ((k & MODULO_8_MASK)>>2); /* j = 0...113 */
      output[k] = input[B][j];

	  k49mod57 += 49;
	  if (k49mod57 > 56) k49mod57 -= 57;
   }    
#else

   /*
   *** Use Optimized assembly routine***
   */

   stealFlagErrs = 0; 
   g_ULCCHData.codecFlags =0;     
   
   for (B=0; B<4; B++)               
   {               
      g_ULCCHData.codecFlags <<= 1;   
      if(input[B][114] < 0)
      {
	     stealFlagErrs += 1;
	     g_ULCCHData.codecFlags += 1; 
	  }                           
	  
      g_ULCCHData.codecFlags <<= 1;	  
      if(input[B][115] < 0)
      {
	     stealFlagErrs += 1;
	     g_ULCCHData.codecFlags += 1; 
	  }  
   }

   dntrleav4(&input[0][0], &output[0]) ;

#endif  /*  end old deinterleaver4 */

   return(stealFlagErrs);
}


/*****************************************************************************
** Routine Name: burstLoader
**
** Description: 
**   Loads the burst builder input buffer with the appropriate burst.   
**
** Method:
**   
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/
void burstLoader(t_chanSel chanSel, UInt TN, Bool frameBoundary)
{
   UInt i;
   UInt j;

   switch(chanSel)
   {
   case SACCH:
   case SDCCH4:
   case SDCCH8:
   case BCCH:
   case PCH:
   case AGCH:
   case PTCCH:
      /*
      * All these channels use the 4 burst deep interleaving buffer
      * If cchTXBurstIndex is initialized...
      *  cchTXBurstIndex is initialized here !!!!!!!!!!!!!!
      */  
      if(frameBoundary) g_intBuf4Deep.cchTXBurstIndex[TN] = 0;      
      if (g_intBuf4Deep.cchTXBurstIndex[TN] >= 0)
      {
	 /*
	 * Set the tx flag indicating to transmit the buffer.
	 */
	 g_DLCodedData.tx = TRUE;

	 /*
	 * Load the burst builder buffer
	 */
	 for(i=0; i<NB_NUM_ENCRYPT_BITS_PER_BURST; i++)
	 {
	    g_DLCodedData.data[i] =
	       g_intBuf4Deep.data[TN][g_intBuf4Deep.cchTXBurstIndex[TN]][i];
	 }

	 /*
	 * Increment cchBurstIndex
	 */
	 g_intBuf4Deep.cchTXBurstIndex[TN] =
	    (g_intBuf4Deep.cchTXBurstIndex[TN] + 1) & MODULO_4_MASK;
      }
      break;
   
   case PDCH:           
   case PDTCH:
   case PACCH:
      /*
      * All these channels use the 4 burst deep interleaving buffer
      * If cchTXBurstIndex is initialized...
      *  cchTXBurstIndex is initialized here !!!!!!!!!!!!!!
      */  
      if(frameBoundary) g_intBuf8Deep.tchTXBurstIndex[TN] = 0;      
      if (g_intBuf8Deep.tchTXBurstIndex[TN] >= 0)
	  {
	     /*
	     * Set the tx flag indicating to transmit the buffer.
         */
	     g_DLCodedData.tx = TRUE;

	     /*
	     * Load the burst builder buffer
	     */
	     for(i=0; i<NB_NUM_ENCRYPT_BITS_PER_BURST; i++)
		 {
	        g_DLCodedData.data[i] =
	        g_intBuf8Deep.data[TN][g_intBuf8Deep.tchTXBurstIndex[TN]][i];
		 }

	     /*
	     * Increment cchBurstIndex
	     */
	     g_intBuf8Deep.tchTXBurstIndex[TN] = (g_intBuf8Deep.tchTXBurstIndex[TN] + 1) & MODULO_4_MASK;
      }
      break;
      
      
                  
   case TCHF:
   case FACCH:
      /*
      * All these channels use the 8 burst deep interleaving buffer
      * If tchTXBurstIndex is initialized...
      */      
      if (g_intBuf8Deep.tchTXBurstIndex[TN] >= 0)
      {   
         /*
         * Set the tx flag indicating to transmit the buffer.
         */
         if(g_intBuf8Deep.txBurstCount[TN]>0)
		 {  
            g_intBuf8Deep.txBurstCount[TN] -= 1;
	        /*
	        * Load the burst builder buffer
	        */

#ifdef NOT_ASM_OPTIMIZED

	        for(i=0; i<NB_NUM_ENCRYPT_BITS_PER_BURST; i++)
			{
	           g_DLCodedData.data[i] =
	              g_intBuf8Deep.data[TN][g_intBuf8Deep.tchTXBurstIndex[TN]][i];
			}
#else
            /*
	        * Load the burst builder buffer..new 8/25/99...modulo_4_mask indexing for new optimized interleaver
	        */
	        j = g_intBuf8Deep.tchTXBurstIndex[TN] & MODULO_4_MASK;

   	        for(i=0; i<NB_NUM_ENCRYPT_BITS_PER_BURST; i++)
			{
	           g_DLCodedData.data[i] =
	              g_intBuf8Deep.data[TN][j][i];
			}
	 
#endif  /*  end old interleaver4 */

            g_DLCodedData.tx = TRUE;
		 }
	     else
		 {
            g_DLCodedData.tx = FALSE;
		 }	 
	     /*
	     * Increment tchBurstIndex
	     */
	     g_intBuf8Deep.tchTXBurstIndex[TN] =
	        (g_intBuf8Deep.tchTXBurstIndex[TN] + 1) & MODULO_8_MASK;

      }
      break;

   case SCH:
   case RACH:
     /*
      * Nothing to do since the burst builder input buffer has
      * already been loaded.
      */
      break;

   default:
      break;

   }
}

/*****************************************************************************
** Routine Name: deinterleaverLoader
**
** Description: 
**   Loads the deinterleaver buffer with the appropriate burst.   
**
** Method:
**   
** Error Handling: 
**
** Limitations: 
** 
** Parameter I/O: 
**
** Returns: 
**
** Data Store I/O: 
**
*****************************************************************************/
void deinterleaverLoader(t_chanSel chanSel, UInt TN, Bool frameBoundary)
{
   UInt i;
   UInt j;
   Char steal1,steal2;

   switch(chanSel)
   {
   case SACCH:
   case SDCCH4:
   case SDCCH8:
   case BCCH:
   case PCH:
   case AGCH:
      /*
      * All these channels use the 4 burst deep interleaving buffer
      * If cchBurstIndex has been initialized...
      * cchBurstIndex is initialized here !!!!!!!!!!!!
      */
      if(frameBoundary) g_deintBuf4Deep.cchRXBurstIndex[TN] = 2;
      if (g_deintBuf4Deep.cchRXBurstIndex[TN] >= 0)
      {
	 /*
	 * Increment it.
	 */
	 g_deintBuf4Deep.cchRXBurstIndex[TN] =
	    (g_deintBuf4Deep.cchRXBurstIndex[TN] + 1) & MODULO_4_MASK;
	    
#ifdef NOT_ASM_OPTIMIZED
	 /*
	 * Load the de-interleaving buffer
	 */
	 for(i=0; i<NB_NUM_ENCRYPT_BITS_PER_BURST; i++)
	 {
	    g_deintBuf4Deep.data[TN][g_deintBuf4Deep.cchRXBurstIndex[TN]][i] =
	       g_ULBurstData.data[i];
	 }
#else
	 /*
	 * ..new 8/30/99...optimized interleaver...move the steal flags from mid-buffer to end-buffer
	 */
	 steal1 = g_ULBurstData.data[57];
	 steal2 = g_ULBurstData.data[58];
	 for(i=0; i < 57; i++)
	 {
	    g_deintBuf4Deep.data[TN][g_deintBuf4Deep.cchRXBurstIndex[TN]][i] =
	       g_ULBurstData.data[i];
	    g_deintBuf4Deep.data[TN][g_deintBuf4Deep.cchRXBurstIndex[TN]][i+57] =
	       g_ULBurstData.data[i+59];
	 }
	 g_deintBuf4Deep.data[TN][g_deintBuf4Deep.cchRXBurstIndex[TN]][114] = steal1;
	 g_deintBuf4Deep.data[TN][g_deintBuf4Deep.cchRXBurstIndex[TN]][115] = steal2;

#endif  /*  end old deinterleaver4 */
      }
      break;  
      
   case PDCH:
   case PACCH:
   case PDTCH:      
      /*
      * All these channels use the 4 burst deep interleaving buffer
      * If cchBurstIndex has been initialized...
      * cchBurstIndex is initialized here !!!!!!!!!!!!
      */
      if(frameBoundary) g_deintBuf8Deep.tchRXBurstIndex[TN] = 2;
      if (g_deintBuf8Deep.tchRXBurstIndex[TN] >= 0)
      {
	 /*
	 * Increment it.
	 */
	 g_deintBuf8Deep.tchRXBurstIndex[TN] =
	    (g_deintBuf8Deep.tchRXBurstIndex[TN] + 1) & MODULO_4_MASK;
	    
#ifdef NOT_ASM_OPTIMIZED
	 /*
	 * Load the de-interleaving buffer
	 */
	 for(i=0; i<NB_NUM_ENCRYPT_BITS_PER_BURST; i++)
	 {
	    g_deintBuf8Deep.data[TN][g_deintBuf8Deep.tchRXBurstIndex[TN]][i] =
	       g_ULBurstData.data[i];
	 }
#else
	 /*
	 * ..new 8/30/99...optimized deinterleaver...move the steal flags from mid-buffer to end-buffer
	 */
	 steal1 = g_ULBurstData.data[57];
	 steal2 = g_ULBurstData.data[58];
	 for(i=0; i < 57; i++)
	 {
	    g_deintBuf8Deep.data[TN][g_deintBuf8Deep.tchRXBurstIndex[TN]][i] =
	       g_ULBurstData.data[i];
	    g_deintBuf8Deep.data[TN][g_deintBuf8Deep.tchRXBurstIndex[TN]][i+57] =
	       g_ULBurstData.data[i+59];
	 }
	 g_deintBuf8Deep.data[TN][g_deintBuf8Deep.tchRXBurstIndex[TN]][114] = steal1;
	 g_deintBuf8Deep.data[TN][g_deintBuf8Deep.tchRXBurstIndex[TN]][115] = steal2;

#endif  /*  end old deinterleaver4 */
      }
      break;  

   case TCHF:
   case FACCH:
      /*
      * All these channels use the 8 burst deep interleaving buffer
      * If tchRXBurstIndex has been initialized...
       */
      if (g_deintBuf8Deep.tchRXBurstIndex[TN] >= 0)
      {
	 /*
	 * Increment it.
	 */

#ifdef NOT_ASM_OPTIMIZED
	 g_deintBuf8Deep.tchRXBurstIndex[TN] =
	    (g_deintBuf8Deep.tchRXBurstIndex[TN] + 1) & MODULO_8_MASK;

	 /*
	 * Load the de-interleaving buffer
	 */
	 for(i=0; i<NB_NUM_ENCRYPT_BITS_PER_BURST; i++)
	 {
	    g_deintBuf8Deep.data[TN][g_deintBuf8Deep.tchRXBurstIndex[TN]][i] =
	       g_ULBurstData.data[i];
	 }
#else
	 /*
	 * ..new 8/30/99...optimized deinterleaver...modulo_4_mask indexing for new deinterleaver
	 */
	 g_deintBuf8Deep.tchRXBurstIndex[TN] =
	    (g_deintBuf8Deep.tchRXBurstIndex[TN] + 1) & MODULO_8_MASK;

	 /*
	 * Load the de-interleaving buffer
	 */
	 /*
	 * ..new 8/30/99...move the steal flags from mid-buffer to end-buffer
	 */
	 j = g_deintBuf8Deep.tchRXBurstIndex[TN] & MODULO_4_MASK;
	 steal1 = g_ULBurstData.data[57];
	 steal2 = g_ULBurstData.data[58];
	 for(i=0; i < 57; i++)
	 {
	    g_deintBuf8Deep.data[TN][j][i] =
	       g_ULBurstData.data[i];
	    g_deintBuf8Deep.data[TN][j][i+57] =
	       g_ULBurstData.data[i+59];
	 }
	 g_deintBuf8Deep.data[TN][j][114] = steal1;
	 g_deintBuf8Deep.data[TN][j][115] = steal2;

#endif  /*  end old deinterleaver4 */
      }
      break;

   case RACH:
      break;

   default:
      break;

   }
}

/********************* Old revision log *************************************************
**
** Revision 1.6  1999/09/10 00:20:45  whuang
** added optimized interleave/deinterleave code by HP
**
** Revision 1.4  1999-06-09 10:36:42-07  dkaiser
** changed NB_NUM_ENCRYPT_BITS to NB_NUM_ENCRYPT_BITS_PER_BURST
** to match consolidated GSM L1 definitions (eliminating duplicates)
**
** Revision 1.3  1999-05-26 17:13:34-07  hpines
** In blockDiagDeinterleaver8, for tch frames, moved buffering of 4 samples [113] <- [115] from
** prolog of stealing flag buffering loop to epilog, to prevent the samples at [113] from being destroyed. 
**
** Revision 1.2  1999-02-09 11:15:40-08  randyt
** Added Initialize Interleave/deinterleave (4) burst index on each Frame boundry.
** 
 *    Rev 1.1   21 Jan 1999 16:35:50   whuang
 * Added Initialize Interleave/deinterlaeve (4)burst index on each Frame boundary
 * 
 *    Rev 1.0   Oct 27 1998 16:19:28   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   Oct 23 1998 17:56:04   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
** Revision 1.9  1998/10/16 14:35:02  dlanza
** Removed tx member from the interleaving buffers.
**
** Revision 1.8  1998/10/15 20:12:05  dlanza
** Added the TCH path and compiled successfully.
**
** Revision 1.7  1998/10/06 20:43:09  kmcpherson
** RKM - Support added, tested, and validated for SCH downlink, RACH downlink and
** uplink, and downlink and uplink sync processing routines.
**
** Revision 1.6  1998/10/01 18:16:37  dlanza
** Fixed bug in de-interleaver.
**
** Revision 1.5  1998/10/01 15:47:14  dlanza
** Optimized block interleaver and de-interleaver.
**
** Revision 1.4  1998/10/01 01:46:34  dlanza
** Optimaized code.  Recuded cycles by 70%.
**
** Revision 1.3  1998/09/30 17:35:44  kmcpherson
** RKM - Added functionality for CCH channel decoding. Currently does not validate.
**
** Revision 1.2  1998/09/28 19:28:08  kmcpherson
** <>
**
** Revision 1.1  1998/09/24 22:31:24  kmcpherson
** Initial revision
**
** Revision 1.4  1998/09/24 21:04:36  kmcpherson
** <>
**
** Revision 1.3  1998/09/22 13:39:22  kmcpherson
** RKM - CCH Channel Encoder is validated against the simulation test
** vectors.
**
** Revision 1.2  1998/09/19 21:56:17  kmcpherson
** RKM 9/19/98 - CCH Encoder done and ready for test vector validation.  The trival
** case had been validated and seems to work through cnv encoding.  Interleaving not tested in full CODEC test.
**/
