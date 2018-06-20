/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*****************************************************************************
* File: loopback.c
*
* Description:
*   This file contains functions for loopback tests.
*
* Public Functions:
*   mod2DmodLpbk, dlGroupVTableLpbkInit, ulGroupVTableLpbkInit, compLpbkBuf
*
* Private Functions:
*   dlRachProc, lpbkDlGroupV, lpbkULGroupV, 
*
******************************************************************************/
#include <stdlib.h> 
#include <math.h> 
#include "gsmdata.h" 
#include "bbdata.h"
#include "dsprotyp.h"                 
#include <mcbsp.h>      /* mcbsp devlib                                       */
#include <mcbspdrv.h>   /* mcbsp driver                                       */
#include <dma.h>
#include <intr.h>
#include "diagdata.h" 

#define NO_CHANNEL_NOISE 0 
#define YES_CHANNEL_NOISE 1 

#pragma DATA_SECTION(loopBackGain, ".extData");
Int16 loopBackGain = 0x7fff;
#pragma DATA_SECTION(dbSNR, ".extData");
Int16 dbSNR = 0x6;
#pragma DATA_SECTION(sigAttenuation, ".extData");
Int16 sigAttenuation = 0x0;
#pragma DATA_SECTION(fadeAttenuation, ".extData");
Int16 fadeAttenuation = 0x0;
#pragma DATA_SECTION(multiPathDelay, ".extData");
Int16 multiPathDelay = 0x0;
#pragma DATA_SECTION(phaseJitter, ".extData");
Int16 phaseJitter = 0x0;

#pragma DATA_SECTION(chanNoiseSwitch, ".extData");
Int16 chanNoiseSwitch = NO_CHANNEL_NOISE;

#pragma DATA_SECTION(totalSamples, ".extData");
Int32 totalSamples=0;
#pragma DATA_SECTION(totalBursts, ".extData");
Int32 totalBursts=0;
#pragma DATA_SECTION(demodErrs, ".extData");
Int32 demodErrs=0 ;
#pragma DATA_SECTION(massiveDemodHit, ".extData");
Int32 massiveDemodHit=0;
#pragma DATA_SECTION(snrtbl, ".extData");
Int16 snrtbl[32] =
{ 26,25,24,23,20,18,16,14,12,11,10,9,8,7,6,6,5,4,4,3,3,2,2,2,2,
  1,1,1,1,1,1,1};
#pragma DATA_SECTION(cosines, ".extData");
Int16 cosines[32] =
{ 32767,32763,32748,32723,32688,32643,32588,32524,32449,
  32365,32270,32166,32052,31928,31795,31651,31499,31336,31164,30983,
  30792,30592,30382,30163,29935,29698,29451,29197,28932,28660,
  28378,28088};
#pragma DATA_SECTION(sines, ".extData");
Int16 sines[32] =
{ 0,572,1144,1715,2286,2856,3425,3993,4560,5126,5690,6252,
  6812,7371,7927,8481,9032,9580,10126,10668,11207,11743,12275,12803,
  13328,13848,14365,14876,15384,15886,16384,16877};

void channelNoise(t_ULComplexSamp   samples[BURST_SIZE_UL_MAX]);

#define interpHalfSize  9                 /* note: coeffs are scaled 2^15 */
Int16 loopInterpFilter[2*interpHalfSize] =
{ 1, -7, 38, -146, 439, -1115, 2547, -5833, 20459,
20459, -5833, 2547 -1115, 439, -146, 38, -7, 1 };

/*****************************************************************************
*
* Function: mod2DmodLpbk
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void mod2DmodLpbk(void)
{
   t_ULComplexSamp   loopbackBurst[157];
   t_DLComplexSamp   *modSample;
   t_DLComplexSamp   *dmodSample;
   Int16             i;
   Uint32            *sample;
   Int16             temp;
      
   modSample = g_BBInfo[0].dlBBSmpl;
   sample = (Uint32*)modSample;
       
   dlBurstProcIsr();
      
   for(i=0; i<154; i++)
   {
       loopbackBurst[i].I =  ( (Int16)(sample[2*(i+4-0)]<<4 & 0xffff) * loopBackGain) >> 15;
       loopbackBurst[i].Q =  ( (Int16)((sample[2*(i+4-0)]<<8)>>16 & 0xffff) * loopBackGain)>>15 ;        
   }
/*  NEW 3/31/99..add channel noise to test equalizer and decoder */

   if(chanNoiseSwitch == YES_CHANNEL_NOISE)
      channelNoise( loopbackBurst);

   g_BBInfo[0].ulBBSmpl = loopbackBurst;
   ulBurstProcIsr();

   if(chanNoiseSwitch == YES_CHANNEL_NOISE)
   {
      temp = 0;
      for(i=0;i < NB_NUM_ENCRYPT_BITS_PER_BURST;i++)
      {
         temp += ( (g_DLCodedData.data[i] == 0) && (g_ULBurstData.data[i] <= 0 ) );
         temp += ( (g_DLCodedData.data[i] == 1) && (g_ULBurstData.data[i] >= 0 ) );
      }
      if(temp <= 36)
      {
         totalBursts++;
         totalSamples++;
         demodErrs += temp;
      }
      else
         massiveDemodHit++;
   }
}
  
/*****************************************************************************
*
* Function: dlRachProc
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlRachProc(Uint8 rfCh, Uint8 timeSlot)
{   
   t_TNInfo *ts;          
   Uint8 rachData = 0x25;
             
   ts = & g_BBInfo[rfCh].TNInfo[timeSlot];
   
   unpackFromByteBuffer(& rachData,
                        (UChar *)&g_DLCCHData.data[0], 
                        NUM_BITS_CCH_FRAME);        
                               
   g_DLCCHData.bufferValid = TRUE;          
   chanEncoder(RACH, 1, timeSlot, 0);     
   buildGSMBurst(RACH,  g_BBInfo[rfCh].tseqNum);
   GMSKModulate(g_BBInfo[rfCh].dlBBSmpl, RACH);                   
}                               
            
/*****************************************************************************
*
* Function: dlGroupVTableLpbkInit
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void dlGroupVTableLpbkInit(void)
{
   Uint32 *ptr;
   ptr = ulGroupTableV;
   *ptr++ = (Uint32 ) & dlFcchProc;
   *ptr++ = (Uint32 ) & dlSchProc;

   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc3;

   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc3;

   *ptr++ = (Uint32 ) & dlFcchProc;
   *ptr++ = (Uint32 ) & dlSchProc;
                  
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc3;

   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc3;

   *ptr++ = (Uint32 ) & dlFcchProc;
   *ptr++ = (Uint32 ) & dlSchProc;

   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc3;

   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc3;

   *ptr++ = (Uint32 ) & dlFcchProc;
   *ptr++ = (Uint32 ) & dlSchProc;

   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc3;

   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc0;
   *ptr++ = (Uint32 ) & ulSdcchProc3;

   *ptr++ = (Uint32 ) & dlFcchProc;
   *ptr++ = (Uint32 ) & dlSchProc;

   *ptr++ = (Uint32 ) & ulSacchProc0;
   *ptr++ = (Uint32 ) & ulSacchProc0;
   *ptr++ = (Uint32 ) & ulSacchProc0;
   *ptr++ = (Uint32 ) & ulSacchProc3;

   *ptr++ = (Uint32 ) & ulSacchProc0;
   *ptr++ = (Uint32 ) & ulSacchProc0;
   *ptr++ = (Uint32 ) & ulSacchProc0;
   *ptr++ = (Uint32 ) & ulSacchProc3;

   *ptr++ = (Uint32 ) & dlIdleProc;                      
}

/*****************************************************************************
*
* Function: ulGroupVTableLpbkInit
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulGroupVTableLpbkInit(void)
{
   dlGroupTableV[0] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableV[1] = (Uint32 ) & dlSdcchProc3;       
   dlGroupTableV[2] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableV[3] = (Uint32 ) & dlSdcchProc3;

   dlGroupTableV[4] = (Uint32 ) & dlRachProc;
   dlGroupTableV[5] = (Uint32 ) & dlRachProc;

   dlGroupTableV[6] = (Uint32 ) & dlSacchProc0;
   dlGroupTableV[7] = (Uint32 ) & dlSacchProc1;
   dlGroupTableV[8] = (Uint32 ) & dlSacchProc1;
   dlGroupTableV[9] = (Uint32 ) & dlSacchProc3;

   dlGroupTableV[10] = (Uint32 ) & dlSacchProc0;
   dlGroupTableV[11] = (Uint32 ) & dlSacchProc1;
   dlGroupTableV[12] = (Uint32 ) & dlSacchProc1;
   dlGroupTableV[13] = (Uint32 ) & dlSacchProc3;

   dlGroupTableV[14] = (Uint32 ) & dlRachProc;
   dlGroupTableV[15] = (Uint32 ) & dlRachProc;
   dlGroupTableV[16] = (Uint32 ) & dlRachProc;
   dlGroupTableV[17] = (Uint32 ) & dlRachProc;
   dlGroupTableV[18] = (Uint32 ) & dlRachProc;
   dlGroupTableV[19] = (Uint32 ) & dlRachProc;
   dlGroupTableV[20] = (Uint32 ) & dlRachProc;
   dlGroupTableV[21] = (Uint32 ) & dlRachProc;
   dlGroupTableV[22] = (Uint32 ) & dlRachProc;
   dlGroupTableV[23] = (Uint32 ) & dlRachProc;
   dlGroupTableV[24] = (Uint32 ) & dlRachProc;
   dlGroupTableV[25] = (Uint32 ) & dlRachProc;
   dlGroupTableV[26] = (Uint32 ) & dlRachProc;
   dlGroupTableV[27] = (Uint32 ) & dlRachProc;
   dlGroupTableV[28] = (Uint32 ) & dlRachProc;
   dlGroupTableV[29] = (Uint32 ) & dlRachProc;
   dlGroupTableV[30] = (Uint32 ) & dlRachProc;
   dlGroupTableV[31] = (Uint32 ) & dlRachProc;
   dlGroupTableV[32] = (Uint32 ) & dlRachProc;
   dlGroupTableV[33] = (Uint32 ) & dlRachProc;
   dlGroupTableV[34] = (Uint32 ) & dlRachProc;
   dlGroupTableV[35] = (Uint32 ) & dlRachProc;
   dlGroupTableV[36] = (Uint32 ) & dlRachProc;

   dlGroupTableV[37] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableV[38] = (Uint32 ) & dlSdcchProc3;         
   dlGroupTableV[39] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableV[40] = (Uint32 ) & dlSdcchProc3;

   dlGroupTableV[41] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableV[42] = (Uint32 ) & dlSdcchProc3;         
   dlGroupTableV[43] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableV[44] = (Uint32 ) & dlSdcchProc3;

   dlGroupTableV[45] = (Uint32 ) & dlRachProc;
   dlGroupTableV[46] = (Uint32 ) & dlRachProc;

   dlGroupTableV[47] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableV[48] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableV[49] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableV[50] = (Uint32 ) & dlSdcchProc3;
}

/*****************************************************************************
*
* Function: ulGroupVIITableLpbkInit
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void ulGroupVIITableLpbkInit(void)
{
   dlGroupTableVII[0] = (Uint32 ) & dlSacchProc0;
   dlGroupTableVII[1] = (Uint32 ) & dlSacchProc1;       
   dlGroupTableVII[2] = (Uint32 ) & dlSacchProc1;
   dlGroupTableVII[3] = (Uint32 ) & dlSacchProc3;

   dlGroupTableVII[4] = (Uint32 ) & dlSacchProc0;
   dlGroupTableVII[5] = (Uint32 ) & dlSacchProc1;       
   dlGroupTableVII[6] = (Uint32 ) & dlSacchProc1;
   dlGroupTableVII[7] = (Uint32 ) & dlSacchProc3;

   dlGroupTableVII[8] = (Uint32 ) & dlSacchProc0;
   dlGroupTableVII[9] = (Uint32 ) & dlSacchProc1;       
   dlGroupTableVII[10] = (Uint32 ) & dlSacchProc1;
   dlGroupTableVII[11] = (Uint32 ) & dlSacchProc3;

   dlGroupTableVII[12] = (Uint32 ) & dlIdleProc;
   dlGroupTableVII[13] = (Uint32 ) & dlIdleProc;
   dlGroupTableVII[14] = (Uint32 ) & dlIdleProc;


   dlGroupTableVII[15] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableVII[16] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[17] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[18] = (Uint32 ) & dlSdcchProc3;


   dlGroupTableVII[19] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableVII[20] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[21] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[22] = (Uint32 ) & dlSdcchProc3;

   dlGroupTableVII[23] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableVII[24] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[25] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[26] = (Uint32 ) & dlSdcchProc3;


   dlGroupTableVII[27] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableVII[28] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[29] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[30] = (Uint32 ) & dlSdcchProc3;


   dlGroupTableVII[31] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableVII[32] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[33] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[34] = (Uint32 ) & dlSdcchProc3;

   dlGroupTableVII[35] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableVII[36] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[37] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[38] = (Uint32 ) & dlSdcchProc3;

   dlGroupTableVII[39] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableVII[40] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[41] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[42] = (Uint32 ) & dlSdcchProc3;
    
   dlGroupTableVII[43] = (Uint32 ) & dlSdcchProc0;
   dlGroupTableVII[44] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[45] = (Uint32 ) & dlSdcchProc3;
   dlGroupTableVII[46] = (Uint32 ) & dlSdcchProc3;

   dlGroupTableVII[47] = (Uint32 ) & dlSacchProc0;
   dlGroupTableVII[48] = (Uint32 ) & dlSacchProc1;       
   dlGroupTableVII[49] = (Uint32 ) & dlSacchProc1;
   dlGroupTableVII[50] = (Uint32 ) & dlSacchProc3;


}

/*****************************************************************************
*
* Function: lpbkDlGroupV
*
* Description:
*   Loop back downlink Group V frames to uplink, which receives frames as
*   51-frame multiframe
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void lpbkDlGroupV(void)
{   
   dlGroupVTableLpbkInit();
}

/*****************************************************************************
*
* Function: lpbkULGroupV
*
* Description:
*   Send downlink frames as uplink 51-frame multiframe
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void lpbkULGroupV(void)
{
    ulGroupVTableLpbkInit();
}

/*****************************************************************************
*
* Function: compLpbkBuf
*
* Description:
*
* Inputs:
*   inBuf     ptr to I/Q sample pairs directly from AD7002  
*
* Outputs:
*   outBuf    prt to 24-bit packed samples for DAC
*
******************************************************************************/
void compLpbkBuf(Uint32 *inBuf, Uint32 *outBuf)
{  
   Uint16            i, k, m, ioffset, scale;
   Int32             ival, qval;
   Uint32            power;
   t_DLComplexSamp*  s = (t_DLComplexSamp*)inBuf;

   /*---------------------------------------------------------
    * Compute first interpHalfSize output sample pairs 
    *--------------------------------------------------------*/
   for(i=0; i<interpHalfSize; i++)
   {
      /*-------------------------------------------------------
       * Even output sample: Copy input with 4-bit downshift 
       *------------------------------------------------------*/
      outBuf[2*i]  = (inBuf[i] >> 4) & 0x0fff;
      outBuf[2*i] |= (inBuf[i] >> 8) & 0x00fff000;

      /*-------------------------------------------------------
       * Odd output sample: Interpolate and copy with 19-bit
       * shift (15-bit coef scaling plus 4-bit input shift)
       *------------------------------------------------------*/
      ival = qval = 0;
      for ( k=(interpHalfSize - 1 - i),m=0; k<2*interpHalfSize; k++,m++ )
      {
         ival += loopInterpFilter[k] * s[m].I;
         qval += loopInterpFilter[k] * s[m].Q;
      }
      ival >>= 19;
      qval >>= 19;
      outBuf[2*i+1]  = ival & 0x0fff;
      outBuf[2*i+1] |= (qval<<12) & 0x00fff000;
   }

   /*---------------------------------------------------------
    * Compute next 2*(156-2*interpHalfSize) output pairs 
    *--------------------------------------------------------*/
   for(i=interpHalfSize; i<156-interpHalfSize; i++)
   {
      /*-------------------------------------------------------
       * Even output sample: Copy input with 4-bit downshift 
       *------------------------------------------------------*/
      outBuf[2*i]  = (inBuf[i] >> 4) & 0x0fff;
      outBuf[2*i] |= (inBuf[i] >> 8) & 0x00fff000;

      /*-------------------------------------------------------
       * Odd output sample: Interpolate and copy with shift
       *------------------------------------------------------*/
      ival = qval = 0;
      for (k=0; k<interpHalfSize; k++ )
      {
         ival += loopInterpFilter[interpHalfSize+k] * (s[i-k].I + s[i+k+1].I);
         qval += loopInterpFilter[interpHalfSize+k] * (s[i-k].Q + s[i+k+1].Q);
      }
      ival >>= 19;
      qval >>= 19;
      outBuf[2*i+1]  = ival & 0x0fff;
      outBuf[2*i+1] |= (qval<<12) & 0x00fff000;
   }

   /*---------------------------------------------------------
    * Compute last interpHalfSize output sample pairs 
    *--------------------------------------------------------*/
   for(i=0; i<interpHalfSize; i++)
   {
      /*-------------------------------------------------------
       * Even output sample: Copy input with 4-bit downshift 
       *------------------------------------------------------*/
      ioffset = i + 156 - interpHalfSize;
      outBuf[2*ioffset]  = (inBuf[ioffset] >> 4) & 0x0fff;
      outBuf[2*ioffset] |= (inBuf[ioffset] >> 8) & 0x00fff000;

      /*-------------------------------------------------------
       * Odd output sample: Interpolate and copy with shift
       *------------------------------------------------------*/
      ival = qval = 0;
      for ( k=0,m=ioffset-interpHalfSize+1; m<156; k++,m++ )
      {
         ival += loopInterpFilter[k] * s[m].I;
         qval += loopInterpFilter[k] * s[m].Q;
      }
      ival >>= 19;
      qval >>= 19;
      outBuf[2*ioffset+1]  = ival & 0x0fff;
      outBuf[2*ioffset+1] |= (qval<<12) & 0x00fff000;
   }                         
}

/*****************************************************************************
*
* Function: channelNoise
*
* Description:
*
* Inputs:  
*
* Outputs:
*
******************************************************************************/
void  channelNoise( t_ULComplexSamp   samples[BURST_SIZE_UL_MAX] ) 
{
   int j,k,mask,shifter;
   Int16 aterm,bterm,sterm,cterm;

/*  Multi_Path signal delayed by 1,2,3,or 4 samples */
    if(multiPathDelay > 0)
    {
     for ( j = BURST_SIZE_UL_MAX - 1; j >= 2; j--) 
      {
        samples[j].I = (samples[j].I >> 1) +  (samples[j-multiPathDelay].I >> 2) ;
        samples[j].Q = (samples[j].Q >> 1) + (samples[j-multiPathDelay].Q >> 2) ;
      }
    }
   /*  AWGN Simulator..random Number Generator...+ and - numbers */

    if(dbSNR > 0)
    {
     if(dbSNR <= 30)
      shifter = snrtbl[dbSNR];
     else
      shifter = 1;
 
     for ( j = 0; j < BURST_SIZE_UL_MAX; j++) 
     {
      k = rand() >> 6; 
      if((k & 1) == 1)
        k = -1 * k;
      samples[j].I = (samples[j].I >> 1) + (shifter * k);      
      k = rand() >> 6;   
      if((k & 1) == 1)
        k = -1 * k;
      samples[j].Q = (samples[j].Q >> 1) + (shifter * k);      
     }
    }
   /*  Random Phase Error Generator...+ and - numbers */
    if(phaseJitter > 0)
    {
     for ( j = 0; j < BURST_SIZE_UL_MAX; j++) 
     {
  
      mask = rand() & 0x3f;
      k = mask & 0x1f;
      cterm = cosines[k];
      if((mask & 0x20) == 0)
       sterm = sines[k];
      else
       sterm = -1 * sines[k]; 
      aterm = (Int32) ( samples[j].I * cterm) >> 15;
      bterm = (Int32) ( samples[j].Q * sterm) >> 15;
      samples[j].I = (Int32) (aterm - bterm) >> 1;
/*    samples[j].I = (samples[j].I >> 6); */
      aterm = (Int32) ( samples[j].Q * cterm) >> 15;
      bterm = (Int32) ( samples[j].I * sterm) >> 15;
      samples[j].Q = (Int32) (aterm + bterm) >> 1;
/*    samples[j].Q = (samples[j].Q >> 6);   */
     }
    }

/* signal+noise attenuation model...increments of 6db from 15bit fullscale*/
    if(sigAttenuation > 4)
    {
     shifter = (sigAttenuation+2) / 6;
     if((totalBursts & 1) == 1)
        shifter -= fadeAttenuation;

     for ( j = 0; j < BURST_SIZE_UL_MAX; j++) 
     {
      samples[j].I = (samples[j].I >> shifter) ;
      samples[j].Q = (samples[j].Q >> shifter);       
     }
    }
   
}
