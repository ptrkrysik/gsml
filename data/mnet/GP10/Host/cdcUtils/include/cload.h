/***************************************************************************
* FILENAME: cload.h
* VERSION:  2.4  5/2/96  13:12:48
* SCCS ID:  "@(#)cload.h	2.4  5/2/96"
***************************************************************************/
/*****************************************************************************/
/* CLOAD.H - Header file for Generic COFF Loader     Version 4.00 9/92       */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* THIS MACRO IS USED TO FACILIATE ACCESS TO THE SECTION HEADERS             */
/*---------------------------------------------------------------------------*/
#define SECT_HDR(i) ((SCNHDR *)(sect_hdrs + (i) * SCNHSZ))

/*---------------------------------------------------------------------------*/
/* THIS MACRO IS USED TO FACILITATE BACKWARDS COMPATIBILITY FOR COFF-        */
/* DEPENDENT TOOLS THAT SUPPORT COFF VERSION 2.                              */
/*---------------------------------------------------------------------------*/
#define O_SECT_HDR(i) ((O_SCNHDR *)(o_sect_hdrs + (i)*SCNHSZ_IN(coff_version)))

/*----------------------------------------------------------------------------*/
/* STATIC COPY OF 8 CHARACTER SECTION NAME, GUARANTEED NULL TERMINATION WHEN  */
/* USED AS A STRING.                                                          */
/*----------------------------------------------------------------------------*/
static char stat_nm[SYMNMLEN+1]={'\0','\0','\0','\0','\0','\0','\0','\0','\0'};
#define SNAMECPY(s)	((char *)strncpy(stat_nm, (s), SYMNMLEN))

/*---------------------------------------------------------------------------*/
/* THESE MACROS ARE USED TO FIND CINIT AND BSS SECTIONS                      */
/*---------------------------------------------------------------------------*/
#define IS_BSS(sptr)  ((sptr->s_flags & STYP_BSS) &&            \
                        !strcmp(sptr->s_name, ".bss") )
#define IS_CINIT(sptr)  ((sptr->s_flags & STYP_COPY) &&         \
                        !strcmp(sptr->s_name, CINIT) )

/*---------------------------------------------------------------------------*/
/* VARIABLES SET BY THE APPLICATION.                                         */
/*---------------------------------------------------------------------------*/
extern FILE   *fin;                  /* COFF INPUT FILE                      */
extern int     verbose;              /* PRINT PROGRESS INFO                  */
extern int     need_data;            /* READ IN RAW DATA                     */
extern int     need_symbols;         /* READ IN SYMBOL TABLE                 */
extern int     clear_bss;            /* CLEAR BSS SECTION                    */
extern int     big_e_config;         /* ENDIANNESS CONFIGURATION OF TARGET   */

/*---------------------------------------------------------------------------*/
/* VARIABLES SET BY THE LOADER.                                              */
/*---------------------------------------------------------------------------*/
extern FILHDR  file_hdr;             /* FILE HEADER STRUCTURE                */
extern int     coff_version;         /* VERSION OF COFF USED BY FILE         */
extern AOUTHDR o_filehdr;            /* OPTIONAL (A.OUT) FILE HEADER         */
extern T_ADDR  entry_point;          /* ENTRY POINT OF MODULE                */
extern T_ADDR *reloc_amount;         /* AMOUNT OF RELOCATION PER SECTION     */
extern char   *sect_hdrs;            /* ARRAY OF SECTION HEADERS             */
extern char   *o_sect_hdrs;          /* ARRAY OF OLD COFF SECTION HEADERS    */
extern int     n_sections;           /* NUMBER OF SECTIONS IN THE FILE       */
extern int     big_e_target;         /* OBJECT DATA IS STORED MSB FIRST      */
extern int     byte_swapped;         /* BYTE ORDERING OPPOSITE OF HOST       */
extern int     curr_sect;            /* INDEX OF SECTION CURRENTLY LOADING   */
extern int     load_err;             /* ERROR CODE RETURNED IF LOADER FAILS  */
extern struct strtab *str_head;      /* HEAD OF STRING BUFFER LIST           */

/*--------------------------------------------------------------------------*/
/*			  CLOAD.C PROTOTYPES				    */
/*--------------------------------------------------------------------------*/
extern int	cload PARMS((void));
extern int	cload_headers PARMS((void));
extern int	cload_data PARMS((void));
extern int	cload_sect_data PARMS((struct scnhdr *));
extern int	cload_cinit PARMS((unsigned char *, int *, int *));
extern int	cload_symbols PARMS((void));
extern int      cload_strings PARMS((void));
extern void     str_free PARMS((struct strtab *));
extern long	sym_read PARMS((long, struct syment *, union auxent *));
extern char    *sym_name PARMS((struct syment *));
extern char    *sym_add_name PARMS((struct syment *));
extern int	reloc_add PARMS((long, struct syment *));
extern int 	relocate PARMS((RELOC *, unsigned char *, int));
extern int	reloc_read PARMS((RELOC *));
extern int	reloc_size PARMS((int));
extern int	reloc_offset PARMS((int));
extern int      reloc_stop PARMS((int));
extern long	sym_reloc_amount PARMS((RELOC *));
extern unsigned	long unpack PARMS((unsigned char *, int, int, int));
extern void	repack PARMS((unsigned long, unsigned char *, int,int,int));
extern int	cload_lineno PARMS((long, int, struct lineno *, int));
extern void	swap4byte PARMS((void *));
extern void	swap2byte PARMS((void *));

/*---------------------------------------------------------------------------*/
/* VALUE OF big_e_config IF THERE IS NO TARGET AND SO IT'S A DON'T CARE.     */
/*---------------------------------------------------------------------------*/
#define DONTCARE   -1

/*---------------------------------------------------------------------------*/
/* ERROR CODES                                                               */
/*---------------------------------------------------------------------------*/
#define E_FILE      1                /* ERROR READING COFF FILE              */
#define E_MAGIC     2                /* WRONG MAGIC NUMBER                   */
#define E_RELOC     3                /* FILE IS NOT RELOCATABLE              */
#define E_SYM       4                /* LOAD_SYM RETURNED FALSE              */
#define E_ALLOC     5                /* MYALLOC OR MRALLOC RETURNED FALSE    */
#define E_SETRELOC  6                /* SET_RELOC_AMOUNT RETURNED FALSE      */
#define E_MEMWRITE  7                /* MEM_WRITE RETURNED FALSE             */
#define E_RELOCENT  8                /* RELOC ENTRY RULES VIOLATED           */
#define E_ENDIAN    9                /* OBJ ENDIANESS CONFLICTS WITH TARGET  */
