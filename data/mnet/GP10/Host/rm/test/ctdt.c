/* ctors and dtors arrays -- to be used by runtime system */
/*   to call static constructors and destructors          */
/*                                                        */
/* NOTE: Use a C compiler to compile this file. If you    */
/*       are using GNU C++, be sure to use compile this   */
/*       file using GNU C and the -traditional flag.      */

typedef void (*VOIDFUNCPTR) ();	/* ptr to function returning void */


extern VOIDFUNCPTR _ctors[];
VOIDFUNCPTR _ctors[] =
    {
    0
    };


extern VOIDFUNCPTR _dtors[];
VOIDFUNCPTR _dtors[] =
    {
    0
    };
