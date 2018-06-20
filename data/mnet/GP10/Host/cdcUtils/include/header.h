/************************************************************************/
/* HEADER.H - STANDARD HEADER FILE FOR COFF LOADERS                     */
/************************************************************************/

/*-----------------------------------------------------------------------*/
/* THE PARMS MACRO IS USED TO EXPAND FUNCTION ARGUMENT PROTOTYPES.  THIS */
/* ENABLES THE SHARING OF PROTOTYPE DEFINITIONS BETWEEN ANSI AND N0N-ANSI*/
/* COMPILERS.  FOR ANSI COMPILERS PROTOTYPE PARAMETERS ARE INCLUDED, FOR */
/* NON-ANSI COMPILERS PROTOTYPE PARAMETERS ARE EXCLUDED.		 */
/*	TO USE THE PARMS MACRO, PROTOTYPES SHOULD BE DEFINED IN THE 	 */
/* FOLLOWING MANNER:							 */
/*	extern int foo PARMS((double d, int i, char *s));		 */
/*-----------------------------------------------------------------------*/
#ifdef __STDC__
#define PARMS(x)	x
#else
#define PARMS(x)	()
#endif

/*-----------------------------------------------------------------------*/
/* INCLUDE STANDARD HEADER FILES                                         */
/*-----------------------------------------------------------------------*/
#include <stdio.h>
#include "version.h"
#include "params.h"
#include "coff.h"
#include "cload.h"
#include "proto.h"

/*-----------------------------------------------------------------------*/
/* CONSTANTS, MACROS, VARIABLES, AND STRUCTURES FOR THE LOADER.          */
/*-----------------------------------------------------------------------*/
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

