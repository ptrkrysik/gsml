/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/****************************************************************************
** File: dem_aprotos.h
**
** Description:
**   GSM Demod Prototypes for A|RT library implentation
**   Note: Old revision log is at the end of this file.
**
*****************************************************************************/
#ifndef DEM_PROTOS_H
#define DEM_PROTOS_H  /* include once only */

#include "gsmtypes.h"

typedef t_ULComplexSamp t_ULCirSamp;

void offsetDerot
(
  t_ULComplexSamp   samples[BURST_SIZE_UL_MAX], 
  Word              threshold
);

void impulse
(
  t_ULComplexSamp   samples[BURST_SIZE_UL_MAX],
  const t_TseqWord  tseq[MAX_TSEQ_SIZE], 
  t_ULCirSamp       cir[MAX_CIR_LENGTH]
);

Char Energy
(
  t_ULCirSamp       cir[MAX_CIR_LENGTH], 
  LWord             leng[MAX_CIR_LENGTH]
);

void normCir
(
  t_ULCirSamp       cir[MAX_CIR_LENGTH], 
  UWord2            mantInv,
  Char              index,
  t_ULCirSamp       cirNorm[MAX_CIR_LENGTH]
);

void calcRho
(
  t_ULCirSamp       cir[MAX_CIR_LENGTH], 
  t_ULCirSamp       cirNorm[MAX_CIR_LENGTH], 
  Char              index,
  Short             exponent, 
  Word3             rho[CIR_BEST_LENGTH-1]
);

void matchFilter
(
  t_ULComplexSamp   data[BURST_SIZE_UL_MAX],
  t_ULCirSamp       cirNorm[MAX_CIR_LENGTH],
  Char              index,
  Short             exponent, 
  Word3             output[NB_MATCH]
);

void  deEmph( t_chanSel        chanSel,
              Word             softDec[], 
              const t_TseqWord tseq[], 
              Word             snr);

Word   computeSNR( Word data[NB_TRAIN_SIZE],t_chanSel chanSel);

void quantize
(
  Word              demodSamps[],
  Char              softDec[]
);

void  equalize (Word    matcho[BURST_MATCH],
                Word    rho[CIR_BEST_LENGTH-1],
                const t_TseqWord tseq[],
                t_chanSel chanSel);

extern void viteq( SInt numBits, 
              Short newMetsPtr[], 
              Short oldMetsPtr[], 
              Short data[], 
              Short probs[], 
              Short *stateTranTbl );

extern UVLWord energyAccumed
(
  t_ULComplexSamp   samples[BURST_SIZE_UL_MAX],
  Char              indexBest
);

extern UVLWord thresholdCir
(
  UVLWord           energyAccum,
  Char              indexBest,
  t_ULCirSamp       cir[MAX_CIR_LENGTH],
  t_ULCirSamp       tempCir[CIR_SIZE],
  LWord2            energy_L[MAX_CIR_LENGTH],
  Word              *toa
);

UWord2 normEnergy
(
  t_ULComplexSamp   samples[BURST_SIZE_UL_MAX],
  t_ULCirSamp       cir[MAX_CIR_LENGTH],
  t_ULCirSamp       cirTemp[MAX_CIR_LENGTH],
  LWord             energy_L[MAX_CIR_LENGTH], 
  Char              indexBest,
  Word              *bob,
  Word              *power, 
  Short             *exponent
);

Word   expX
(
  UVLWord data,
  Word    *exponent
);

Word14 mag2DB
(
  UVLWord mag
);

/* demodc.c */
void removeDCOffset (t_ULComplexSamp samples[], t_chanSel chanSel, Uint8 rfCh, Uint8 index); 
void derotate (t_ULComplexSamp samples[]);


#endif  /* end of include once only */


/********************* Old revision log *****************************
**
** Revision 1.6  1999/09/23 18:38:37  dkaiser
** added channel type to computSNR (Howard)
**
** Revision 1.5  1999-07-14 16:32:46-07  hpines
** header files to support Equalizer version of demod routines
**
** Revision 1.3  1999-05-04 14:29:28-07  whuang
** <>
**
** Revision 1.1  1999-02-24 15:16:50-08  whuang
** Initial revision
**
** Revision 1.1  1999-01-07 14:30:25-08  randyt
** Initial revision
**
 * 
 *    Rev 1.0   Oct 27 1998 16:19:34   Administrator
 * Checked in from initial workfile by PVCS Version Manrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
** Revision 1.2  1998/10/14 18:43:28  bhehn
** AB & NB verified.  Merged with shared header.
** AB ~= 103us, NB ~= 78us
**
** Revision 1.1  1998/10/07 15:21:37  bhehn
** Initial revision
**
** Revision 1.3  1998/10/02 17:01:01  bhehn
** Changed calling routine to be more like c6x  implementation.
** BER matches previous version.
**
** Revision 1.2  1998/10/01 16:37:0558  bhehn
** Initial revision
**
** Revision 1.3  1998/09/22 20:00:38  bhehn
** Access Burst support added and verified.
**
** Revision 1.2  1998/09/21 22:04:24  bhehn
** NB demod works, BER close to floating
**/

