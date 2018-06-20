/*
**  GSM  Demod Training Sequences
**
** $Author: whuang $
** $Date: 1999/07/06 19:35:02 $
** $Source: D:\\stageCell\\Dsp\\bbdataproc\\demod-c\\RCS\\training.h $
** $Revision: 1.1 $
** $Log: training.h $
** Revision 1.1  1999/07/06 19:35:02  whuang
** Initial revision
**
** Revision 1.1  1999-01-07 14:30:26-08  randyt
** Initial revision
**
 * 
 *    Rev 1.0   Oct 27 1998 16:19:34   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   Oct 23 1998 17:56:08   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
** Revision 1.2  1998/10/14 18:43:29  bhehn
** AB & NB verified.  Merged with shared header.
** AB ~= 103us, NB ~= 78us
**
** Revision 1.1  1998/10/07 15:21:38  bhehn
** Initial revision
**
** Revision 1.1  1998/10/01 13:56:59  bhehn
** Initial revision
**
** Revision 1.2  1998/09/21 22:04:25  bhehn
** NB demod works, BER close to floating
**
*/
#ifndef TRAINING_H
#define TRAINING_H

/* 
* Define the training sequence word length, 
* tseq = -1 or 1 = 2 bits 
*/
typedef Char  t_TseqWord;

/* 
* Set up a #define that determines whether the 
* variables should be defined here or just
* externed here
*/
#ifdef DEMOD_TOP




const t_TseqWord train_nb[8][26] = 
{
  { + 1, + 1, -1, + 1, + 1, -1, + 1, -1, -1, -1, + 1, + 1, + 1,
   + 1, -1, + 1, + 1, + 1, -1, + 1, + 1, -1, + 1, -1, -1, -1 },
  { + 1, + 1, -1, + 1, -1, -1, + 1, -1, -1, -1, + 1, -1, -1,
  -1, -1, + 1, + 1, + 1, -1, + 1, -1, -1, + 1, -1, -1, -1 },
  { + 1, -1, + 1, + 1, + 1, + 1, -1, -1, -1, + 1, -1, -1, -1,
   + 1, -1, + 1, + 1, -1, + 1, + 1, + 1, + 1, -1, -1, -1, + 1 },
  { + 1, -1, + 1, + 1, + 1, -1, -1, -1, -1, + 1, -1, -1, + 1,
  -1, + 1, + 1, + 1, -1, + 1, + 1, + 1, -1, -1, -1, -1, + 1 },
  { + 1, + 1, + 1, -1, -1, + 1, -1, + 1, -1, -1, -1, + 1, + 1,
  -1, + 1, + 1, + 1, + 1, + 1, -1, -1, + 1, -1, + 1, -1, -1 },
  { + 1, -1, + 1, + 1, -1, -1, -1, + 1, -1, + 1, -1, -1, + 1,
   + 1, + 1, + 1, + 1, -1, + 1, + 1, -1, -1, -1, + 1, -1, + 1 },
  { -1, + 1, -1, + 1, + 1, -1, -1, -1, -1, -1, + 1, -1, -1,
   + 1, + 1, + 1, -1, + 1, -1, + 1, + 1, -1, -1, -1, -1, -1 },
  { -1, -1, -1, + 1, -1, -1, -1, -1, + 1, + 1, + 1, -1, + 1,
   + 1, -1, + 1, -1, -1, -1, + 1, -1, -1, -1, -1, + 1, + 1 }
}; 


const t_TseqWord train_sb[64] = 
{
  -1, + 1, -1, -1, -1, + 1, + 1, -1, + 1, -1, -1, + 1, + 1, + 1, -1, + 1,
   + 1, + 1, + 1, + 1, + 1, -1, + 1, + 1, + 1, + 1, + 1, + 1, -1, -1, -1, -1,
   + 1, + 1, -1, + 1, -1, -1, + 1, -1, + 1, -1, + 1, + 1, + 1, -1, + 1, -1,
   + 1, -1, -1, -1, + 1, -1, -1, + 1, + 1, + 1, + 1, -1, -1, + 1, -1, -1
};

const t_TseqWord train_ab[41] =
{
  +1, -1, +1, +1, -1, +1, -1, -1, +1, -1,
  -1, -1, -1, -1, -1, -1, -1, +1, +1, -1,
  -1, +1, +1, -1, -1, +1, -1, +1, -1, +1,
  -1, +1, +1, +1, -1, -1, -1, -1, +1, +1,+1
};

#else

extern const t_TseqWord train_nb[8][26]; 
extern const t_TseqWord train_sb[64];
extern const t_TseqWord train_ab[41];

#endif     /* ifdef DEMOD_TOP */



#endif
