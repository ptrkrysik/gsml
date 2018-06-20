/****************************************************************************
*   (c) Copyright Cisco 2000
*   All Rights Reserved
*****************************************************************************/
/*
**  GSM  Demod Training Sequences
**
** Revision 1.2  1999-05-03 11:53:45-07  whuang
** Added Viterbi equalizer function
**
** Revision 1.3  1998/10/27 21:39:55  bhehn
** AB & NB eqaulizer verifed.
** AB ~= 125us, NB ~= 155us.
**
** Revision 1.2  1998/10/14 18:43:29  bhehn
** AB & NB verified.  Merged with shared headsion 1.1  1998/10/01 13:56:59  bhehn
** Initial revision
**
** Revision 1.2  1998/09/21 22:04:25  bhehn
** NB demod works, BER close to floating
**
*****************************************************************************/
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
  { -1, -1, +1, -1, -1, +1, -1, +1, +1, +1, -1, -1, -1,
    -1, +1, -1, -1, -1, +1, -1, -1, +1, -1, +1, +1, +1 },

  { -1, -1, +1, -1, +1, +1, -1, +1, +1, +1, -1, +1, +1,
    +1, +1, -1, -1, -1, +1, -1, +1, +1, -1, +1, +1, +1 },

  { -1, +1, -1, -1, -1, -1, +1, +1, +1, -1, +1, +1, +1,
    -1, +1, -1, -1, +1, -1, -1, -1, -1, +1, +1, +1, -1 },

  { -1, +1, -1, -1, -1, +1, +1, +1, +1, -1, +1, +1, -1,
    +1, -1, -1, -1, +1, -1, -1, -1, +1, +1, +1, +1, -1 },

  { -1, -1, -1, +1, +1, -1, +1, -1, +1, +1, +1, -1, -1,
    +1, -1, -1, -1, -1, -1, +1, +1, -1, +1, -1, +1, +1 },

  { -1, +1, -1, -1, +1, +1, +1, -1, +1, -1, +1, +1, -1,
    -1, -1, -1, -1, +1, -1, -1, +1, +1, +1, -1, +1, -1 },

  { +1, -1, +1, -1, -1, +1, +1, +1, +1, +1, -1, +1, +1,
    -1, -1, -1, +1, -1, +1, -1, -1, +1, +1, +1, +1, +1 },

  { +1, +1, +1, -1, +1, +1, +1, +1, -1, -1, -1, +1, -1,
    -1, +1, -1, +1, +1, +1, -1, +1, +1, +1, +1, -1, -1 }
}; 

const t_TseqWord train_ab[41] =
{
  -1, +1, -1, -1, +1, -1, +1, +1, -1, +1,
  +1, +1, +1, +1, +1, +1, +1, -1, -1, +1,
  +1, -1, -1, +1, +1, -1, +1, -1, +1, -1,
  +1, -1, -1, -1, +1, +1, +1, +1, -1, -1, -1
};

#else

extern const t_TseqWord train_nb[8][26]; 
extern const t_TseqWord train_ab[41];

#endif     /* ifdef DEMOD_TOP */



#endif
