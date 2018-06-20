/*
**   GSM Demod Prototypes for A|RT library implentation
**
** $Author: whuang $
** $Date: 1999/07/06 19:35:01 $
** $Source: D:\\stageCell\\Dsp\\bbdataproc\\demod-c\\RCS\\dem_protos.h $
** $Revision: 1.1 $
** $Log: dem_protos.h $
** Revision 1.1  1999/07/06 19:35:01  whuang
** Initial revision
**
** Revision 1.1  1999-01-07 14:30:25-08  randyt
** Initial revision
**
 * 
 *    Rev 1.0   Oct 27 1998 16:19:34   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   Oct 23 1998 17:56:08   Administrator
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
** Revision 1.2  1998/10/01 16:37:05  bhehn
** Fixed matchfilter output saturation problem
**
** Revision 1.1  1998/10/01 13:56:58  bhehn
** Initial revision
**
** Revision 1.3  1998/09/22 20:00:38  bhehn
** Access Burst support added and verified.
**
** Revision 1.2  1998/09/21 22:04:24  bhehn
** NB demod works, BER close to floating
**
*/
#ifndef DEM_PROTOS_H
#define DEM_PROTOS_H


/*void offsetDerot(t_ULComplexSamp   samples[BURST_SIZE_UL_MAX], 
                 Word threshold );
*/

void  impulse(t_ULComplexSamp   samples[BURST_SIZE_UL_MAX],
               const t_TseqWord tseq[MAX_TSEQ_SIZE], 
               Word cir[2][MAX_CIR_LENGTH] );

Char  Energy( Word cir[2][MAX_CIR_LENGTH], 
              LWord leng[MAX_CIR_LENGTH] );

UWord2   normEnergy( t_ULComplexSamp   samples[BURST_SIZE_UL_MAX],
                     Word cir[2][MAX_CIR_LENGTH],
                     LWord  energy_L[MAX_CIR_LENGTH], 
                     Char   indexBest,
                     Word   *bob,
                     Word   *power, 
                     Short  *exponent);

void normCir( Word    cir[2][MAX_CIR_LENGTH], 
              UWord2   mantInv,
              Char    index,
              Word2   cirNorm[2][MAX_CIR_LENGTH] );

void calcRho( Word   cir[2][MAX_CIR_LENGTH], 
              Word2  cirNorm[2][MAX_CIR_LENGTH], 
              Char   index,
              Short  exponent, 
              Word3  rho[CIR_BEST_LENGTH-1] );

void matchFilter( t_ULComplexSamp   data[BURST_SIZE_UL_MAX],
                  Word2  cirNorm[2][MAX_CIR_LENGTH],
                  Char   index,
                  Short  exponent, 
                  Word3  output[NB_MATCH]);

void  weight( Word       softDec[], 
              const t_TseqWord tseq[], 
              Word       *snr);

void  quantize( Word  demodSamps[],
                Char  softDec[] );

void  equalize (Word    matcho[BURST_MATCH],
                Word    rho[CIR_BEST_LENGTH-1],
                const t_TseqWord tseq[]);


#endif
