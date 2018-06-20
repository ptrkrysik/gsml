/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
* File: oamtypes.h
**
* Description:   
*   This file contains defined constants, enumerations and type definitions
*   of shared items to support OAM processing.
*
*****************************************************************************/
#ifndef _OAMTYPES_H_
#define _OAMTYPES_H_  /* include once only */

#include "dsp/dsptypes.h"
                           
/*
*  Bit-mapped definitions for the Handover Cause field.  If, during the decision process,
*  the condition for the HO Required message is satisfied at the same time by multiple
*  reasons, then the order of priority from highest to lowest shall be:
*  RXQUAL (Intracell), RXLEV (Intercell).
*/
#define  DL_RXLEV     0x01  
#define  UL_RXLEV     0x02  
#define  DL_RXQUAL    0x04
#define  UL_RXQUAL    0x08
#define  DL_RXQUAL_IH 0x10
#define  UL_RXQUAL_IH 0x20
 
#define NUM_NCELLS 32
/* defaults if serving or neighbour cell's values are undefined */
#define HO_MARGIN_DEF             6
#define RXLEV_MIN_DEF            30  /* RXLEV is 30 for -80 dBm default dBm, 
                                         used to test handover candidacy */   
#define UL_DL_HO_THRESH_DEF      20   /* RXLEV units */
#define NUM_TX_VS_PWR_LEVEL      70
#define NUM_TX_VS_ARFCN          15

typedef struct {
  Uint8 numRxSigSamples;   // renamed 'hreqave' in gsm 05.08
  Int8  LRxPwrULH;         /* cell threshold for handover to commence */
  /*
   *  The following are used in conjunction with co-channel inteference handling.
   */ 
  Uint8 Intave;            // Averaging period of Grp 1 RXLEV samples; measured in SACCH multiframes.
  Uint8 RxLevUL_IH;		   // Thresholds for interference-based handover to commence.
} t_ULSignalStrengthOAM;

typedef struct {
  Uint8 numRxSigSamples;   /* renamed 'hreqave' in gsm 05.08 */
  Uint8 LRxQualULH;        /* cell quality threshold for handover to commence */
} t_ULSignalQualityOAM;

typedef struct {
  Uint8 LRxLevDLH;    /* cell threshold for handover to commence */
  Uint8 hoAveWindow;  /* in units of SACCH periods or approx 0.5 secs */  
  Uint8 RxLevDL_IH;   // Thresholds for interference-based handover to commence.
} t_DLSignalStrengthSCellOAM;  // Serving Cell

typedef struct {
  Uint8 LRxQualDLH;     /* cell quality threshold for handover to commence */
  Uint8 hoAveWindow;    /* in units of SACCH periods or approx 0.5 secs */						
} t_DLSignalQualitySCellOAM;			// Serving Cell		

typedef struct {
  Uint8 pxH;            /* used to measure px out of nx failures on the threshold */
  Uint8 nxH;            /*   before handover commences (P5,N5) */
} t_SignalStrengthOAM;  // Serving Cell

typedef struct {
  Uint8 pxH;            /* used to measure px out of nx failures on the threshold */
  Uint8 nxH;            /*   before handover commences (P6,N6) */
} t_SignalQualityOAM;   // Serving Cell

typedef struct {
  Uint8 pxH;            /* used to measure px out of nx failures on the threshold */
  Uint8 nxH;            /*   before interference-related handover commences (P7,N7) */
} t_IntSignalStrengthOAM;  // Serving Cell

typedef struct {
  Uint8 dynPwrSacchFrmCnt; // !! used for frequency of reporting power control
  t_ULSignalStrengthOAM	      ulsigstren;
  t_ULSignalQualityOAM	      ulsigqual;  
  t_DLSignalStrengthSCellOAM  dlsigstrenSCell;    // Serving Cell
  t_DLSignalQualitySCellOAM   dlsigqualSCell;
  t_SignalStrengthOAM         sigstren;           // variables common to UL and DL
  t_SignalQualityOAM          sigqual;
  t_IntSignalStrengthOAM      intsigstren;
} t_DynPwrOAMInfo;  

typedef struct {
  Uint16 rfCh;   /* ARFCN value */
  Uint8  bsic;
  Int8   hoMargin;   /* prevents repetitive handover (or 'ping-ponging') between adjacent cells */
  Int16  msTxPwrMax; /* max. power for mobile to use in an adjacent cell */
  Int16  rxLevMin;   /* minimum rxlev required for a mobile to handover to another cell */
  Uint8 radioLinkTimeout;  /* units of 4 SACCH frames, or 480ms */
} t_uniqueCellParms, t_serveCellParms;

typedef struct {
  Int16 hoMarginDef;  /* prevents repetitive handover (or 'ping-ponging') between undef. adjacent cells */
  Int16 msTxPwrMaxDef; /* max. power for mobile to use in an undefined, adjacent cell */
  Int16 rxLevMinDef;  /* default minimum to evaluate handover to undefined, adjacent cells */
} t_commonCellParms;

typedef struct {
  t_serveCellParms   serveParms;      
  t_commonCellParms  commonParms; 
  t_uniqueCellParms  uniqueParms[NUM_NCELLS];
} t_HoOAMInfo;

#endif  /* end of include once only */