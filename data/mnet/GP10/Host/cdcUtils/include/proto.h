/*****************************************************************************/
/* PROTO.H - Prototypes for loader required functions.                       */
/*****************************************************************************/

/*--------------------------------------------------------------------------*/
/*  LOADER REQUIRED FUNCTIONS			                            */
/*--------------------------------------------------------------------------*/
extern int   mem_write PARMS((unsigned char *, unsigned int, T_ADDR, unsigned char));
extern int   set_reloc_amount PARMS((void));
extern void  lookup_sym       PARMS((long indx, SYMENT *sym, AUXENT *aux));
extern int   load_syms        PARMS((int need_reloc));
extern void *myalloc          PARMS((unsigned int size));
extern void *mralloc          PARMS((void *p, unsigned int size));
