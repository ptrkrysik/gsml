/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
**
** File: mod2spb.c
**
** Description:   
**   This file contains the GSM GMSK modulator routine.
**   Note: Old revision log is at the bottom of this file.
**
*****************************************************************************/
#include "gsmdata.h"
#include "bbdata.h"
#include "bbproto.h"
#include "dsprotyp.h"
#include "mod2spb_tab.h"

/*****************************************************************************
** ROUTINE NAME: mod2spb
**
** Description:
**   Look-up-table based GSM GMSK 2 samples per bit modulator.
**
** Method:
**   This function generates a GSM GMSK modulated waveform from a sequence
**   of bits.  Power ramping is also performed.
**
**   148 bits are input: The whole burst including tail bits but excluding
**   guard period bits.
**
**   The ramped samples (corresponding to the guard period and the extreme
**   tail bit on either end) are pre-calculated and simply copied to the
**   output buffer.  The pre-calculation assumes that guard period bits are
**   1's and the outer 3 tail bits on either end are 0's as indicated in
**   GSM 05.02 for the NB.  This is not exactly true for the AB which has
**   0,0,1 as its 1st three tail bits, but close enough to produce acceptable
**   results.
**
**   Output is 148 bits of the burst and 9 (actually 8.25) bits of the guard
**   period.  4 guard period bits are added at the beginning and 5 (actually
**   4.25) are added at the end.  156.25 bits (625 samples) are output.
**
** Error Handling: 
**
** Limitations: 
**   The output buffer pointer MUST point to a buffer that is 32-bit
**   aligned.
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
   
/*
* Declare the assembly modulator 
*/
extern void GMSKModLoop
(
   t_DLComplexSamp *outBurstPtr,
   t_chanSel chanSel,
   UChar *diff,
   Int *constantsPtr
);


Void modRampSample(Uint8 IQSwap, t_DLComplexSamp *outBurstPtr);

void GMSKModulate(t_DLComplexSamp *outBurstPtr, t_chanSel chanSel)
{
   UChar diff[MOD_NUM_BITS_IN + 2];
   Int c_constants[]=      
   {
      MOD_NUM_BITS_IN,                       /* bits differentially encoded */
      AB_BITS_TO_MOD,                        /* size of access (RACH) burst */
      NB_BITS_TO_MOD,                        /* size of normal burst        */
      RAMP_SAMPLES*sizeof(t_DLComplexSamp),  /* offset in output past ramp  */
      RACH,                                  /* used to detect RACH bursts  */
   };

   if(g_DLBurstData.tx == TRUE)
   {
      GMSKModLoop
      (
         outBurstPtr,   /* - pointer to output samples                       */
         chanSel,       /* - type of burst selector                          */
         diff,          /* - differentially encoded bit array, declared here */
         c_constants    /* - pass these thru a pointer so that if they       */
      );                /*   ever change, those changes are reflected       */
                        
      modRampSample(g_BBInfo[0].IQSwap,  outBurstPtr);
   }
   else
   {
      zeroTxBurst((Uint32*)g_BBInfo[0].dlBBSmpl);
   }
}                      /*    in the assembly modulator. DO NOT REORDER      */


Void modRampSample(Uint8 IQSwap, t_DLComplexSamp *outBurstPtr)
{  
   Int16 i, outputIndex;  

   outputIndex = 0;
   if(IQSwap == TRUE)
   {                                                           
      /*
      *  If requested by Host, swap I and Q samples
      */
      for (i=0; i<RAMP_SAMPLES; i++)
      {
         *(Int16 *) & (outBurstPtr[outputIndex].I) = modRampUp[i*2].Q;
         *(Int16 *) & (outBurstPtr[outputIndex++].Q) = (modRampUp[i*2].I) >> 4;         
      }  
   }
   else
   {
      for (i=0; i<RAMP_SAMPLES; i++)
      {
         *(Int16 *) & (outBurstPtr[outputIndex].I) = modRampUp[i*2].I;
         *(Int16 *) & (outBurstPtr[outputIndex++].Q) = (modRampUp[i*2].Q) >> 4;         
      }  
   }      
   outputIndex = 0;       
   for (i=0; i<RAMP_SAMPLES; i++)
   {             
      *(Int32 *) & (outBurstPtr[outputIndex]) >>= 4;
      outputIndex++;
   }
      
   outputIndex = 312-RAMP_SAMPLES;
   if(IQSwap == TRUE)
   {          
      /*
      *  If requested by Host, swap I and Q samples
      */      
      for (i=0; i<RAMP_SAMPLES; i++)
      {      
         *(Int16 *) & (outBurstPtr[outputIndex].I) = modRampDown[i*2].Q;
         *(Int16 *) & (outBurstPtr[outputIndex++].Q) = (modRampDown[i*2].I) >> 4;         
      }     
   }
   else
   {
      for (i=0; i<RAMP_SAMPLES; i++)
      {      
         *(Int16 *) & (outBurstPtr[outputIndex].I) = modRampDown[i*2].I;
         *(Int16 *) & (outBurstPtr[outputIndex++].Q) = (modRampDown[i*2].Q) >> 4;         
      }          
   }
   outputIndex = 312-RAMP_SAMPLES;
   for (i=0; i<RAMP_SAMPLES; i++)
   {               
      *(Int32 *) & (outBurstPtr[outputIndex]) >>= 4;
      outputIndex++;
   }
   *(Int32 *) & (outBurstPtr[outputIndex]) = 0; /* set 313th sample to zero */
}        

#undef TESTMOD
#ifdef TESTMOD

UChar testdata[]=
{
   1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
   1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
   1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
   1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
   1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
   1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
   1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
   1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,1,1,0,
};

t_DLComplexSamp out1[MOD_NUM_BITS_IN+2*RAMP_SAMPLES];
t_DLComplexSamp out2[MOD_NUM_BITS_IN+2*RAMP_SAMPLES];

extern void GMSKModulateC(t_DLComplexSamp *outBurstPtr, t_chanSel chanSel);

void main(void)
{
   int i;

   for (i=0;i<MOD_NUM_BITS_IN+RAMP_SAMPLES;i++)
      g_DLBurstData.data[i] = testdata[i];

   g_DLBurstData.tx = 1;

   for (i=0;i<MOD_NUM_BITS_IN+2*RAMP_SAMPLES;i++)
   {
      out1[i].I = out1[i].Q = 0;
   }

   GMSKModulateC(out1,TCHF);

   for (i=0;i<MOD_NUM_BITS_IN+RAMP_SAMPLES;i++)
      g_DLBurstData.data[i] = testdata[i];

   g_DLBurstData.tx = 1;

   for (i=0;i<MOD_NUM_BITS_IN+2*RAMP_SAMPLES;i++)
   {
      out2[i].I = out2[i].Q = 0;
   }

   GMSKModulate(out2,TCHF);

   for (i=0;i<MOD_NUM_BITS_IN+2*RAMP_SAMPLES;i++)
   {
      if (out1[i].I!=out2[i].I || out1[i].Q!=out2[i].Q)
         goto error;
   }

   while(1);
error:
   for (;;);

}

#endif /* TESTMOD */

/*********************** Old revision log **********************************
**
** Revision 1.5  1999/11/05 02:39:32  whuang
** corrected problem on power ramping up
**
** Revision 1.4  1999-11-04 14:54:48-08  ltravaglione
** <>
**
** Revision 1.4  1999-11-02 18:05:31-08  whuang
** added Ramping for active burst always
**
** Revision 1.3  1999-09-07 17:43:01-07  whuang
** added UL/DL DTX mode handling functions
**
** Revision 1.2  1999-05-10 08:43:48-07  dkaiser
** no longer need to include mod4spb.h, nor extern g_DLBurstBuf (all in bbdata.h)
**
** Revision 1.1  1999-02-09 10:55:13-08  randyt
** Initial revision
**
 * 
 *    Rev 1.0   03 Feb 1999 16:58:48   whuang
 * Checked in by PVCS VM Server.
** Revision 1.4  1999-01-07 14:13:31-08  randyt
** Change back to 2's complement output, McBSP FPGA handles invert the sign bit
** PVCS Version 1.3
**
 * 
 *    Rev 1.3   10 Dec 1998 17:43:46   whuang
 * Change back to 2's complement output, McBSP FPGA handles invert the sign bit
 * 
 *    Rev 1.0   Oct 27 1998 16:19:28   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   Oct 23 1998 17:56:02   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
** Revision 1.13  1998/10/16 15:34:47  dlanza
** <>
**
** Revision 1.12  1998/10/14 22:12:33  bhehn
** Modified for L1 global instantiation
**
** Revision 1.11  1998/10/04 01:13:35  dlanza
** Optimized to 8860 (mod+burst) cycles.
**
** Revision 1.10  1998/10/01 17:19:00  dlanza
** Cut the cycle count from 13,151 to 12,199.
**
** Revision 1.9  1998/09/30 17:27:56  dlanza
** <>
**
** Revision 1.8  1998/09/30 02:08:23  dlanza
** Fixed endian problem for good.  Code is now endian independent.  Mod/burst runs is 13,500 cycles.
**
** Revision 1.7  1998/09/28 23:55:13  dlanza
** <>
**
** Revision 1.6  1998/09/28 23:51:51  dlanza
** Fixed endian problem associated with 32-bit table accesses.
**
** Revision 1.4  1998/09/28 22:08:45  dlanza
** Broken!!!!
**
** Revision 1.3  1998/09/28 14:30:13  dlanza
** Modified loop to improve performance. Helped a little.
**
** Revision 1.2  1998/09/25 19:16:57  dlanza
** Added reference to prototypes header file.
**
** Revision 1.1  1998/09/25 19:00:27  dlanza
** Initial revision
**
**
** Revision: 1.1
** DJL 9/22/98 - Creation.
**/

