/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: oamdata.h
*
* Description:   
*   This file contains references to the global structures and variables
*   used in OAM processing. It includes a file containing the defined
*   constants, enumerations and type definitions that support OAM.
*
*   NOTE: INGLOB and EXGLOB specify internal and external DSP memory
*         allocation, respectively. They are defined in dsptypes.h
*         and included here through oamtypes.h. 
*
*****************************************************************************/
#ifndef _OAMDATA_H_
#define _OAMDATA_H_  /* include once only */

#include "oamtypes.h"                         

EXGLOB t_DynPwrOAMInfo  g_DynPwrOAMInfo;
EXGLOB t_HoOAMInfo      g_HoOAMInfo;
EXGLOB Uint8            g_CurrentTxPwrOffsetForArfcn;
extern Uint16           g_encryptionCapability;   // defined and set in A5 asm

#ifdef ALLOCATE_MEMORY_EXTERNAL /*-------------------------------------------*/ 

/*
* Default map of Static Pwr Level to DAC control values. Half-decibel steps.
*/
Uint8  g_TxMaxPwrRdct2DLPwrTbl[NUM_TX_VS_PWR_LEVEL] =       /* default 1800  */
{ 240, 218, 213, 208, 204,  199, 195, 192, 188, 185, 182, 179, 176, 174, 170, 167,
   164, 161, 158, 155, 152, 149, 146, 142, 139, 135, 131, 127, 124, 120, 115, 111,
   106, 102,  97,  93,  89,  85,  81,  77,  73,  69,  66,  63,  60,  58,  55,  53, 
   50,  48,  45,  43,  40,   38,  36,  34,  31,  29,  27,  24,  22,  20,  17,  15,
   12,  10,   7,   5,   3,   0 };
/*
*  Default map of Tx Static Pwr Level offset vs ARFCN. Half-decibel steps.
*/
Uint8  g_TxPwrOffsetVsArfcnTbl[NUM_TX_VS_ARFCN] =
{ 5, 5, 5, 5, 5, 4, 3, 1, 1, 1, 4, 5, 2, 1, 0 };
/*
*  Fixed maps of measurement ARFCNs for Level vs ARFCN table at 1800 and 1900
*/
Uint16 g_TxOffsetArfcns1800[NUM_TX_VS_ARFCN] =
{ 512, 517, 522, 556, 591, 626, 661, 698, 733, 768, 803, 838, 875, 880, 885 };   
Uint16 g_TxOffsetArfcns1900[NUM_TX_VS_ARFCN] =
{ 512, 517, 522, 549, 577, 605, 633, 661, 689, 717, 745, 773, 800, 805, 810 };

#else  /*--------------------------------------------------------------------*/

extern Uint8   g_TxMaxPwrRdct2DLPwrTbl[NUM_TX_VS_PWR_LEVEL];
extern Uint8   g_TxPwrOffsetVsArfcnTbl[NUM_TX_VS_ARFCN];
extern Uint16  g_TxOffsetArfcns1800[NUM_TX_VS_ARFCN];
extern Uint16  g_TxOffsetArfcns1900[NUM_TX_VS_ARFCN];

#endif /*--------------------------------------------------------------------*/


#endif  /* end of include once only */
