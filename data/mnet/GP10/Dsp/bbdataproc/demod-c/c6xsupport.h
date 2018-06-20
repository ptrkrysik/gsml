/*
**   TMS320C6x related macros and intrinsics prototypes
**
** $Author: whuang $
** $Date: 1999/07/06 19:34:59 $
** $Source: D:\\stageCell\\Dsp\\bbdataproc\\demod-c\\RCS\\c6xsupport.h $
** $Revision: 1.1 $
** $Log: c6xsupport.h $
** Revision 1.1  1999/07/06 19:34:59  whuang
** Initial revision
**
** Revision 1.1  1999-01-07 14:30:22-08  randyt
** Initial revision
**
 * 
 *    Rev 1.0   Oct 27 1998 16:19:34   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   Oct 23 1998 17:56:06   Administrator
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
** Revision 1.4  1998/10/12 15:40:26  bhehn
** Casting SAT_WORD to long to make work.
**
** Revision 1.3  1998/10/09 21:20:35  bhehn
** Fast version, Not yet versified.
** AB ~ 125us, NB ~= 110us
**
** Revision 1.2  1998/10/09 15:54:00  bhehn
** Made I/O faster and fixed normEnergy() energyAccum probelm for large signals.
**
** Revision 1.1  1998/10/07 15:21:36  bhehn
** Initial revision
**
** Revision 1.1  1998/10/01 13:56:59  bhehn
** Initial revision
**
**
*/

/*
* Define a signed shift macro
*  ANSI C does not support this but A|RT & TI do.
*/
/* signed left shift */
#define SSHIFTL( _x, _sft)  ( (_sft) > 0 ? (_x) << (_sft) : (_x) >> -(_sft) )
/* signed right shift */
#define SSHIFTR( _x, _sft)  ( (_sft) > 0 ? (_x) >> (_sft) : (_x) << -(_sft) )


/* 
* Used to set the arithmetic saturation mode
*  Examples:
*    c = a + b; // Unsaturated 
*    c = SAT( a+b ); // Saturated 
*
* SAT typically needs to be used only if the input data
* energy has an "exponent" of 0.  This means that the
* input samples will range from -1 to 1-precision
*/
/*#define SAT(x)   oqc( x, fxpOqc::saturated() )     
********************************************************************/
#define  SAT_LONG(z)  ( _sat(z) )

#define  SAT_WORD(_arg)  ( SAT_LONG( ((long) _arg) << 16 ) >> 16 )


/************************************************************8
**   Intrinsic Macros
**
**  these may need to be ported to subroutines to make completely
**  compatable with the C6x intrinsics operation.
*************************************************************/
/*
* define a multiply that takes two Words (16 bits)
* multipies them, shifts by one and saturates
*
* TI 6X equivelent #define   SMPY(x,y)   (_smpy( x, y )) 
*/
#define SMPY(x,y)     ( _smpy(x, y ) )
/*
* define a C equiv for the C6c EXTU() intrinsic
*/
#define EXTU(src2,cst1,cst2)     ( ( (src2)<<(cst1) )>>(cst2) )
#define EXT(src2,cst1,cst2)     ( (int)( (src2)<<(cst1) )>>(cst2) )


/* 
* NORM returns the number bits to the left of the sign bit
* for a "int" (32 bits)  
* For positive values this is 2*WORD_SIZE - (log2 + 2)
*/
#define  NORM(z)    ( _norm(z) )
/* 
* LNORM returns the number bits to the left of the sign bit
* for a "long" (40 bits)  
* For positive values this is 2*WORD_SIZE+ACCUM_GUARD_BITS - (log2 + 2)
*/
#define  LNORM(z)    ( _lnorm( z ) )
  
/* 
* log2(x) = log10(x)*3.3219, log10(x) = log2(x)*0.301 
* log2 returns the number of bits to the left of the msbit 
*/
#define LOG2(z)     ( 2*WORD_SIZE - NORM(z) )


