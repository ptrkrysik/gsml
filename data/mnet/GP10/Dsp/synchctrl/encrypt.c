/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: encrypt.c
*
* Description:
*   This file contains functions that encrypt and decrypt bursts on TCH,
*   SDCCH and SACCH logical channels. The theory of operation is follows:
*
*   The encryptState structure in gsmtypes.h contains current and new
*   algorithm parameters which indicates A5/1, A5/2 or no encryption.
*   It also contains an encryptMode which is set to Normal mode except  
*   around the time of a Layer 3 Encryption Command message from the host.
*   When this message is received, the DSP saves the New Algorithm and 
*   switches to Pending Change mode. At this point, the DSP is still using
*   the Current Algorithm. The DSP waits until the current FACCH or SDCCH
*   frame is sent -- hopefully with a Layer 2 Ciphering Mode Command sent
*   previously by the host -- and then switches from Pending Change mode
*   to Change mode. In this mode, the DSP uses the Current Algorithm on the
*   downlink and the New Algorithm on the uplink. Upon decoding a valid
*   Layer 2 frame, the DSP switches from Change to Normal mode and copies
*   the New Algorithm into the Current Algorithm parameter. As before in
*   Normal mode, the DSP uses the Current Algorithm -- which is now the
*   same as the newly commanded algorithm -- on the uplink and downlink.
*
*   State changes vs. burst phase for SDCCH and TCH:
*   DL:   3     0     1     2     3     0     1     2     3     0
*   UL:     3     0     1     2     3     0     1     2     3     0
*   SDCCH: a    b                       c                             ...d
*   TCH:   a    b                                               c     ...d
*
*   a = Cipher Mode & Encr Cmd received from Host, set mode=ENCR_PEND_CHANGE
*   b = Cipher Mode cmd starts to go on DL, set mode=ENCR_CHANGE
*   c = Cipher Mode cmd is finished going out on DL
*   d = Some time later, receive a valid Layer 2 frame, set mode=ENCR_NORMAL
*
*   Event   Where It Is Handled   At What Burst Phase
*     a        dcchMsgProc               N/A
*     b        encrypt                    0  
*     c        encrypt                    0  
*     d        ulSyncMsgProcess          N/A    
*
* Public Functions:
*   decrypt, encrypt
*
* Private Functions:
*   computeEncryption
*
******************************************************************************/
#include "stdlib.h"
#include "gsmdata.h" 
#include "dsp/dsphmsg.h"
#include "bbdata.h"
#include "dsprotyp.h"
#include "diagdata.h" 

void decryptBits(Uint8  *data, Uint8 *encryptBits, Uint16 numbits); 

#define A5_ASM_OPTIMIZED
#ifndef A5_ASM_OPTIMIZED

/*****************************************************************************
* C Version of A5/1, A5/2 Helper Functions
* These functions plus computeEncryption will be replaced by assembly
*****************************************************************************/

#ifdef _INLINE
#define __INLINE inline
#else
#define __INLINE
#endif

typedef unsigned long uint32;
uint32 r1, r2, r3, r4;

/*-------------------------------------------------------------
 * Majority(x,y,z) = (x&y)^(x&z)^(y&z) = (x&(y^z))^(y&z)
 *-----------------------------------------------------------*/
#define M3(x,y,z)  ((x) & ((y)^(z))) ^ ((y) & (z))

/*---------------------------------------------------------------------------
 * Single step shift functions for A5/1
 *--------------------------------------------------------------------------*/
void step_r1A51(void)
{
   uint32 feedback;
   feedback = ((r1>>18) ^ (r1>>17) ^ (r1>>16) ^ (r1>>13)) & 1;
   r1 <<= 1;
   r1 ^= feedback;
}

void step_r2A51(void)
{
   uint32 feedback;
   feedback = ((r2>>22) ^ (r2>>21) ^ (r2>>20) ^ (r2>>7)) & 1;
   r2 <<= 1;
   r2 ^= feedback;
}

void step_r3A51(void)
{
   uint32 feedback;
   feedback = ((r3>>21) ^ (r3>>20)) & 1;
   r3 <<= 1;
   r3 ^= feedback;
}

/*---------------------------------------------------------------------------
 * Single step shift functions. The R1, R2 and R3 step functions are called
 * an average of 86 + (6/8)*388 = 377 times per burst. The R4 step function
 * is called 86 + 338 = 474 times per burst.
 *--------------------------------------------------------------------------*/
__INLINE void step_r1(void)
{
   uint32 feedback;
   feedback = (r1 ^ (r1>>1) ^ (r1>>2) ^ (r1>>5)) & 1;
   r1 >>= 1;
   r1 ^= feedback << 18;
}

__INLINE void step_r2(void)
{
   uint32 feedback;
   feedback = (r2 ^ (r2>>1) ^ (r2>>2) ^ (r2>>15)) & 1;
   r2 >>= 1;
   r2 ^= feedback << 22;
}

__INLINE void step_r3(void)
{
   uint32 feedback;
   feedback = (r3 ^ (r3>>1)) & 1;
   r3 >>= 1;
   r3 ^= feedback << 21;
}

__INLINE void step_r4(void)
{
   uint32 feedback;
   feedback = (r4 ^ (r4>>5)) & 1;
   r4 >>= 1;
   r4 ^= feedback << 16;
}


/*----------------------------------------------------------------
 *  Shifts of the 3 registers are determined by 1 bit per register.
 *----------------------------------------------------------------*/
void clock_controlA51(void)
{
   
  int input_bits, x9, y11, z11;

  x9  = ((r1 & 0x0100) >> 6);
  y11 = ((r2 & 0x0400) >> 9);
  z11 = ((r3 & 0x0400) >> 10);

  input_bits = ((x9 | y11 | z11) & 0x07);

  switch (input_bits)
   {
   case 3:
   case 4:
      step_r2A51();
      step_r3A51();
      break;
   case 2:
   case 5:
      step_r1A51();
      step_r3A51();
      break;
   case 1:
   case 6:
      step_r1A51();
      step_r2A51();
      break;
   default:
      step_r1A51();
      step_r2A51();
      step_r3A51();
   }

   return;
}


/*---------------------------------------------------------------------------
 * Determine which registers to step under R4 clock control, then call the
 * appropriate step functions. This function is called 288 times per burst.
 *--------------------------------------------------------------------------*/
__INLINE void clock_control(void)
{
   switch (r4 & 0x2240)
   {
   case 0x0040:
   case 0x2200:
      step_r2();
      step_r3();
      break;
   case 0x0200:
   case 0x2040:
      step_r1();
      step_r3();
      break;
   case 0x0240:
   case 0x2000:
      step_r1();
      step_r2();
      break;
   default:
      step_r1();
      step_r2();
      step_r3();
   }
   step_r4();

   return;
}


/*-----------------------------------------------------------
 * No R4 register for A5/1; A5/1 stepping is opposite to A5/2.
 *----------------------------------------------------------*/
void step_registersA51(int clocking)
{
   if ( clocking )
   {
      clock_controlA51();
   }
   else
   {
      step_r1A51();
      step_r2A51();
      step_r3A51();
   }
   return;
}

/*---------------------------------------------------------------------------
 * Step appropriate registers. For each burst, this functions is called
 * 86 times with clocking=FALSE plus 388 times with clocking=TRUE.
 *--------------------------------------------------------------------------*/
__INLINE void step_registers(int clocking)
{
   if ( clocking )
   {
      clock_control();
   }
   else
   {
      step_r1();
      step_r2();
      step_r3();
      step_r4();
   }
   return;
}


/*---------------------------------------------------------------------------
 * Load key and count into all four (three for A5/1) registers. One or the other  
 * function is called three times per burst using lengths of 32, 32 and 22.
 *--------------------------------------------------------------------------*/
void load_inA51(uint32 load_val, int length)
{
   int i;

   for ( i=0; i<length; i++ )
   {
      /*--------------------------------------------
       * step without clocking, then add key bit
       *-------------------------------------------*/
      step_registersA51(0);
      if ( load_val & 1 )
      {
         r1 ^= 1;
         r2 ^= 1;
         r3 ^= 1;
      }
      load_val >>= 1;
   }
   return;
}


void load_in(uint32 load_val, int length)
{
   int i;

   for ( i=0; i<length; i++ )
   {
      /*--------------------------------------------
       * step without clocking, then add key bit
       *-------------------------------------------*/
      step_registers(0);
      if ( load_val & 1 )
      {
         r1 ^= 1 << 18;
         r2 ^= 1 << 22;
         r3 ^= 1 << 21;
         r4 ^= 1 << 16;
      }
      load_val >>= 1;
   }
   return;
}


/*---------------------------------------------------------------------------
 * Initialize registers with key and count.
 *--------------------------------------------------------------------------*/
void init_registersA51(uint32 kclo, uint32 kchi, uint32 count)
{
   int i;

   r1 = r2 = r3 = 0;
  
   /*---------------------------------------
    * Load Kc
    *--------------------------------------*/
   load_inA51(kclo, 32);
   load_inA51(kchi, 32);
  
   /*---------------------------------------
    * Load COUNT
    *--------------------------------------*/
   load_inA51(count, 22);
  
   /*---------------------------------------
    * No need to Fix bits, like in A5/2 
    *--------------------------------------*/

   /*---------------------------------------
    * Run up
    *--------------------------------------*/
   for ( i=0; i<100; i++ )
      step_registersA51(1);
 
   return;
}


void init_registers(uint32 kclo, uint32 kchi, uint32 count)
{
   int i;

   r1 = r2 = r3 = r4 = 0;

   /*---------------------------------------
    * Load Kc
    *--------------------------------------*/
   load_in(kclo, 32);
   load_in(kchi, 32);

   /*---------------------------------------
    * Load COUNT
    *--------------------------------------*/
   load_in(count, 22);

   /*---------------------------------------
    * Fix bits
    *--------------------------------------*/
   r1 |= 0x8;
   r2 |= 0x10;
   r3 |= 0x20;
   r4 |= 0x40;

   /*---------------------------------------
    * Run up
    *--------------------------------------*/
   for ( i=0; i<100; i++ )
      step_registers(1);

   return;
}


/*-----------------------------------------------------------
 * output for A51 BLOCK1/BLOCK2 construction
 *-----------------------------------------------------------*/
unsigned char a5v1(void)
{
   unsigned char output;

   /*---------------------------------------
    * Compute the output bit
    *--------------------------------------*/
   output = ((r1>>18) & 1) ^ ((r2>>22) & 1) ^ ((r3>>21) & 1);

   /*---------------------------------------
    * Step the appropriate registers
    *--------------------------------------*/
   step_registersA51(1);

   return output;
}


/*---------------------------------------------------------------------------
 * Compute one encryption bit to be XOR'd with one air bit. This function is
 * called 288 times per burst.
 *--------------------------------------------------------------------------*/
__INLINE unsigned char a5v2(void)
{
   unsigned char output;

   /*---------------------------------------
    * Compute the output bit
    *--------------------------------------*/
   output = ( (r1 & 1) ^ (r2 & 1) ^ (r3 & 1) );
   output ^= M3( (r1>>3) & 1, ((r1>>4) & 1) ^ 1, (r1>>6) & 1 );
   output ^= M3( (r2>>4) & 1, (r2>>6) & 1, ((r2>>9) & 1) ^ 1 );
   output ^= M3( ((r3>>5) & 1) ^ 1, (r3>>8) & 1, (r3>>12) & 1 );

   /*---------------------------------------
    * Step the appropriate registers
    *--------------------------------------*/
   step_registers(1);

   return output;
}
/************** end of C Version A5/1, A5/2 Helper Functions **********************/
#endif


void computeA52Asm(Char* Inputs, Char* Outputs);
void computeA51Asm(Char* Inputs, Char* Outputs);

void computeA5(Uint32* key, Uint32 count, UChar* block1, UChar* block2,
               t_encryptAlgorithm encryptAlg )
{                
   int i,j;
   Char     encryptInputs[86];
   Char     encryptOutputs[100+2*NB_NUM_ENCRYPT_BITS_PER_BURST];

   /*----------------------------------------------
   * Read the cipher key (32+32 bits) into encryption input buffer
   *---------------------------------------------*/

   for ( i=0; i<32; i++ )
   {
      encryptInputs[i] = (Char)(key[1] & 1);
      encryptInputs[32+i] = (Char)(key[0] & 1);
      key[0] = key[0] >> 1;
      key[1] = key[1] >> 1;
   }
   /*----------------------------------------------
   * Read the frame COUNT into encryption input buffer
   *---------------------------------------------*/
   for ( i=0; i<22; i++ )
   {
      encryptInputs[64+i] = (Char)(count & 1);
      count = count >> 1;
   }
/* fast assembly routine to compute ul,dl encryption bits */

   if(encryptAlg == ENCR_A52)
   {
      computeA52Asm(&encryptInputs[0], &encryptOutputs[0]);
      j = 100;  /* offset 100 dummy output bits */
   }
   else
   {
      computeA51Asm(&encryptInputs[0], &encryptOutputs[0]);
      j = 0;
   }

/* unpack the encryption bits to downlink, uplink buffers */
/* for now,first 100 outputs are dummy */

   for ( i=0; i<NB_NUM_ENCRYPT_BITS_PER_BURST-2; i++ )
   {
      block1[i] = encryptOutputs[j+i]; 
   }
   for ( i=0; i<NB_NUM_ENCRYPT_BITS_PER_BURST-2; i++ )
   {
      block2[i]= encryptOutputs[j+i+NB_NUM_ENCRYPT_BITS_PER_BURST-2];
   }
   return;
}

/*****************************************************************************
*
* Function: decrypt
*
* Description:
*   Decrypts one uplink burst using previously computed encryption stream.
*   Input consists of soft decisions, so decryption amounts to sign changes. 
*   This function should be called for each burst just before channel decoding.
*
* Input from call list:
*   rfCh                      RF channel number
*   timeSlot                  GSM time slot
*
* Input from global memory:
*   g_ULBurstData.data[]      ptr to array of soft decisions
*   g_BBInfo[rf].TNInfo[tn]   timeslot information structure
*    .ulEncryptStream[]         ptr to encryption stream
*
* Output to global memory:
*   g_ULBurstData.data[]      ptr to decrypted soft decisions
*
******************************************************************************/
void decrypt(Uint8 rfCh, Uint8 timeSlot)
{
   register UChar*    encryptStream; 
   register Char*    data;
   t_encryptState*   encryptState; 
   UChar*             ulEncryptStream;
   t_TNInfo          *ts;
   Uint8             subCh;         
    
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   
   /*--------------------------------------------------------------
    * Get pointers to encryption state and buffer for storing
    * uplink encryption stream
    *-------------------------------------------------------------*/
   if ( ts->groupNum == GSM_GROUP_5 )
   {
      subCh = ts->u.group5.ulSdcchSubCh;
      encryptState = & ts->u.group5.sdcch[subCh].encryptState;
      ulEncryptStream = &sdcchUlEncryptStream[subCh];  /* use new onchip buffer */
/*      ulEncryptStream = encryptState->ulEncryptStream; */
   }
   else if (ts->groupNum == GSM_GROUP_7){
	  subCh = ts->u.group7.ulSdcchSubCh;
      encryptState = & ts->u.group7.sdcch[subCh].encryptState;
      ulEncryptStream = &sdcch8UlEncryptStream[subCh];  /* use new onchip buffer */

   }else
   {
      encryptState = & ts->u.group1.tch.encryptState;
      ulEncryptStream = &tchUlEncryptStream[rfCh][timeSlot];  /* use new onchip buffer */
/*     ulEncryptStream = encryptState->ulEncryptStream; */
   }
  
   /*--------------------------------------------------------------
    * If an encryption algorithm is in use on UL, apply it to data
    *-------------------------------------------------------------*/
  if ( (encryptState->encryptMode != ENCR_CHANGE &&
         encryptState->algorithm != ENCR_NONE)  ||
        (encryptState->encryptMode == ENCR_CHANGE &&
         encryptState->newAlgorithm != ENCR_NONE) )
   {
     /*--------------------------------------------------------------
      * Decrypt first half of burst (not including stealing flag)
      *-------------------------------------------------------------*/
      encryptStream = ulEncryptStream;

#ifndef A5_ASM_OPTIMIZED
      for ( data = g_ULBurstData.data ;
            data < g_ULBurstData.data + NB_NUM_ENCRYPT_BITS_PER_SIDE - 1;
            data++ )
      {
         if ( *encryptStream++ )  *data = -*data; 
      }
#else
      decryptBits((Uint8 *) & g_ULBurstData.data[0],(Uint8 *) & encryptStream[0],
                                NB_NUM_ENCRYPT_BITS_PER_SIDE - 1);
#endif
     /*--------------------------------------------------------------
      * Decrypt second half of burst (not including stealing flag)
      *-------------------------------------------------------------*/

#ifndef A5_ASM_OPTIMIZED
      for ( data += 2;
            data < g_ULBurstData.data + NB_NUM_ENCRYPT_BITS_PER_BURST;
            data++ )
      {
         if ( *encryptStream++ )  *data = -*data; 
      }
#else
      decryptBits((Uint8 *) & g_ULBurstData.data[NB_NUM_ENCRYPT_BITS_PER_SIDE+1],
          (Uint8 *) & encryptStream[NB_NUM_ENCRYPT_BITS_PER_SIDE-1], NB_NUM_ENCRYPT_BITS_PER_SIDE - 1);
#endif
   }
}

/*****************************************************************************
*
* Function: encrypt
*
* Description:
*   Computes encryption stream for one burst. Applies encryption to downlink
*   immediately and saves uplink encryption stream for use by decrypt().
*   This function should be called for each burst after channel encoding.
*
* Input from call list:
*   rfCh                      RF channel number
*   timeSlot                  GSM time slot
*   burstPhase                phase within TCH or SDCCH block (0,1,2,3)
*
* Input from global memory:
*   g_DLCodedData.data[]      ptr to array of downlink bits (0/1)
*   g_BBInfo[rf].TNInfo[tn]   timeslot information structure
*    .ulEncryptStream[]         ptr to uplink encryption stream
*
* Output to global memory:
*   g_DLCodedData.data[]      ptr to encrypted downlink bits
*   g_BBInfo[rf].TNInfo[tn]   timeslot information structure
*    .ulEncryptStream[]         computed uplink encryption stream
*   
*
******************************************************************************/
void encrypt(Uint8 rfCh, Uint8 timeSlot, Uint8 burstPhase)
{
   register UChar*   encryptStream; 
   register UChar*   data;
   t_encryptState*   encryptState; 
   UChar             dlEncryptStream[NB_NUM_ENCRYPT_BITS_PER_BURST-2];
   UChar*            ulEncryptStream;
   Uint32            key[ENCRYPT_KEY_LEN];
   Uint32            count;
   t_TNInfo          *ts;
   Uint8             subCh;
   Uint8             i;  
  
	ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   
   /*--------------------------------------------------------------
    * Get pointers to encryption state and buffer for storing
    * uplink encryption stream
    *-------------------------------------------------------------*/
   if ( ts->groupNum == GSM_GROUP_5   || ts->groupNum == GSM_GROUP_7)
   {
	   if (ts->groupNum == GSM_GROUP_5){
		subCh = ts->u.group5.ulSdcchSubCh;
		encryptState = & ts->u.group5.sdcch[subCh].encryptState;
                ulEncryptStream = &sdcchUlEncryptStream[subCh];  /* use new onchip buffer */
	   } else {
		subCh = ts->u.group7.ulSdcchSubCh;
		encryptState = & ts->u.group7.sdcch[subCh].encryptState;
                ulEncryptStream = &sdcch8UlEncryptStream[subCh];  /* use new onchip buffer */
	   }
 
/*      ulEncryptStream = encryptState->ulEncryptStream; */
      
      /*-----------------------------------------------------------
      * Run state machine for SDCCH. See file header for details.
      *-----------------------------------------------------------*/
      if ( burstPhase==0 )
      {
         if ( encryptState->encryptMode == ENCR_PEND_CHANGE )
         {
            encryptState->encryptMode = ENCR_CHANGE;
         }
      }   
   }
   else
   {
      encryptState = & ts->u.group1.tch.encryptState;
      ulEncryptStream = &tchUlEncryptStream[rfCh][timeSlot];  /* use new onchip buffer */
/*      ulEncryptStream = encryptState->ulEncryptStream; */

      /*-----------------------------------------------------------
      * Run state machine for TCH. See file header for details.
      *-----------------------------------------------------------*/
      if ( burstPhase==0 )
      {
         if ( encryptState->encryptMode == ENCR_PEND_CHANGE )
         {
            encryptState->encryptMode = ENCR_CHANGE;
         }
      }   
   }
   
   /*--------------------------------------------------------------
    * If encryption is on or has just been commanded, compute
    * encryption streams for UL and DL. Save UL stream for use
    * later in decryption and decide whether to use DL stream.
    * Note: Do not support direct switch between A5/1 and A5/2.
    *-------------------------------------------------------------*/
   for ( i=0; i<ENCRYPT_KEY_LEN; i++ )
   {
      key[i] = encryptState->encryptKey[i]; 
   }
   count = ((Uint32)sysFrameNum.t1<<11) |
           ((Uint32)sysFrameNum.t3<<5)  |
           (Uint32)sysFrameNum.t2;
              
   if ( encryptState->algorithm == ENCR_A51 ||
        (encryptState->encryptMode == ENCR_CHANGE &&
         encryptState->newAlgorithm == ENCR_A51)  )
   {
      computeA5(key, count, dlEncryptStream, ulEncryptStream, ENCR_A51);
   } 
   if ( encryptState->algorithm == ENCR_A52 ||
        (encryptState->encryptMode == ENCR_CHANGE &&
         encryptState->newAlgorithm == ENCR_A52)  )
   {   
      computeA5(key, count, dlEncryptStream, ulEncryptStream, ENCR_A52);    
   } 
   
   /*--------------------------------------------------------------
    * If encryption is on, apply current algorithm to downlink
    *-------------------------------------------------------------*/
   if ( encryptState->algorithm != ENCR_NONE )
   {
     /*--------------------------------------------------------------
      * Encrypt first half of burst (not including stealing flag)
      *-------------------------------------------------------------*/
 
      encryptStream = dlEncryptStream;

      for ( data = g_DLCodedData.data ;
            data < g_DLCodedData.data + NB_NUM_ENCRYPT_BITS_PER_SIDE - 1;
            data++ )
      {
         *data ^= *encryptStream++;
      }
     /*--------------------------------------------------------------
      * Encrypt second half of burst (not including stealing flag)
      *-------------------------------------------------------------*/
      for ( data += 2;
            data < g_DLCodedData.data + NB_NUM_ENCRYPT_BITS_PER_BURST;
            data++ )
      {
         *data ^= *encryptStream++;
      }
   }
}
   
 

