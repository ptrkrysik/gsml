/***************************************************************************
* FILENAME: coff.h
* VERSION:  2.5  8/5/96  11:36:08
* SCCS ID:  "@(#)coff.h	2.5  8/5/96"
***************************************************************************/
/**************************************************************************/
/*  COFF.H - Definition of COFF structures and definitions.               */
/*                                                                        */
/*  This file defines all standard COFF definitions, used by any program  */
/*  which reads or writes COFF files.                                     */
/*                                                                        */
/*  HISTORY                                                               */
/*       --/--/83     - Original (lost in the mists of time)              */
/*       --/--/91     - Tektronics relocation entry kludge                */
/*       01/07/94 RET - Reworked file header and relocation entries, add  */
/*                      COFF version support.   (Removed Tek kludge)      */
/*       12/24/95 TMS - Reworked section header structures for COFF 2     */
/*                                                                        */
/**************************************************************************/

/*------------------------------------------------------------------------*/
/*  COFF VERSION FLAGS                                                    */
/*------------------------------------------------------------------------*/
#if !defined(COFF_VERSION_0) && \
    !defined(COFF_VERSION_1) && \
    !defined(COFF_VERSION_2)
#define COFF_VERSION_0     0
#define COFF_VERSION_1     0
#define COFF_VERSION_2     1
#endif

#ifndef COFF_VERSION_0 
#define COFF_VERSION_0     0
#endif

#ifndef COFF_VERSION_1
#define COFF_VERSION_1     0
#endif

#ifndef COFF_VERSION_2
#define COFF_VERSION_2     0
#endif

/*------------------------------------------------------------------------*/
/*  COFF MAGIC NUMBERS                                                    */
/*------------------------------------------------------------------------*/
#define COFF_MAGIC_0 0300         /* ORIGINAL VERSION OF COFF             */
#define COFF_MAGIC_1 0301
#define COFF_MAGIC_2 0302

/*------------------------------------------------------------------------*/
/*  COFF TARGET ID's (FORMERLY MAGIC NUMBERS)                             */
/*  NOTE!!! DEFINE THE MACRO "MAGIC" TO BE ONE OF THESE MACROS.           */
/*------------------------------------------------------------------------*/
#define MAGIC_340     0220
#define MAGIC_370     0221
#define MAGIC_DSP     0222
#define MAGIC_C30     0223
#define MAGIC_380     0224
#define MAGIC_MVP     0225
#define MAGIC_C16     0226
#define MAGIC_ARM     0227
#define MAGIC_LEAD    0230
#define MAGIC_C60     0231
#define MAGIC_C8P     0232
#define MAGIC_LEADSDK 0233
#define MAGIC_LEAD3   0234
#define MAGIC_ANKOOR  0235

#define MAGIC_MIN   0220          /* MINIMUM VALID MAGIC NUMBER           */
#define MAGIC_MAX   0235          /* MAXIMUM VALID MAGIC NUMBER           */

/*------------------------------------------------------------------------*/
/*  Macros to recognize magic numbers                                     */
/*------------------------------------------------------------------------*/
#define ISCOFF(x)       (ISCOFF_0(x)||ISCOFF_1(x)||ISCOFF_2(x)||ISMAGIC(x))
#define ISMAGIC(x)      (((unsigned short)(x)) == MAGIC)
#define BADMAGIC(x)     (((unsigned short)(x) & 0x8080) && !ISMAGIC(x))

#if COFF_VERSION_2
#define ISCOFF_0(x)     ((unsigned short)(x) == COFF_MAGIC_0)
#define ISCOFF_1(x)     ((unsigned short)(x) == COFF_MAGIC_1)
#define ISCOFF_2(x)	((unsigned short)(x) == COFF_MAGIC_2)
#elif COFF_VERSION_1
#define ISCOFF_0(x)     ((unsigned short)(x) == COFF_MAGIC_0)
#define ISCOFF_1(x)     ((unsigned short)(x) == COFF_MAGIC_1)
#define ISCOFF_2(x)	FALSE
#else
#define ISCOFF_0(x)     FALSE
#define ISCOFF_1(x)     FALSE
#define ISCOFF_2(x)	FALSE
#endif

#define ISMAGIC_ANY(x)  (((unsigned short)(x)) >= MAGIC_MIN &&   \
                         ((unsigned short)(x)) <= MAGIC_MAX)
#define ISCOFF_ANY(x)   (ISCOFF_0(x) || ISCOFF_1(x) || \
			 ISCOFF_2(x) || ISMAGIC_ANY(x))


#include "coffdefs.h"

/*------------------------------------------------------------------------*/
/*  COFF FILE HEADER                                                      */
/*------------------------------------------------------------------------*/
struct filehdr
{
   unsigned short  f_magic;        /* magic number */
   unsigned short  f_nscns;        /* number of sections */
   long            f_timdat;       /* time & date stamp */
   long            f_symptr;       /* file pointer to symtab */
   long            f_nsyms;        /* number of symtab entries */
   unsigned short  f_opthdr;       /* sizeof(optional hdr) */
   unsigned short  f_flags;        /* flags */
   unsigned short  f_target_id;    /* target architecture id */
};

#define FILHDR  struct filehdr
#define FILHSZ             (COFF_VERSION_0 ? 20 : 22)
#define FILHSZ_IN(version) (version >= COFF_MAGIC_1 ? 22 : 20)

/*------------------------------------------------------------------------*/
/*  File header flags                                                     */
/*------------------------------------------------------------------------*/
#define  F_RELFLG   0x01       /* relocation info stripped from file      */
#define  F_EXEC     0x02       /* file is executable (no unresolved refs) */
#define  F_LNNO     0x04       /* line nunbers stripped from file         */
#define  F_LSYMS    0x08       /* local symbols stripped from file        */

/*------------------------------------------------------------------------*/
/*  Target device identification flags (bits 4-7 in file header flags)    */
/*------------------------------------------------------------------------*/
#define  F_VERS0    0x0        /* 0th generation CPU                      */
#define  F_VERS1    0x10       /* 1st generation CPU                      */
#define  F_VERS2    0x20       /* 2nd generation CPU                      */
#define  F_VERS3    0x40       /* 3rd generation CPU                      */
#define  F_VERS4    0x80       /* 4th generation CPU                      */
#define  F_VERSION  (F_VERS1 | F_VERS2 | F_VERS3 | F_VERS4)

/*------------------------------------------------------------------------*/
/*  Target device raw data byte ordering flags (bits 8-9)                 */
/*------------------------------------------------------------------------*/
#define  F_LITTLE   0x100      /* object code is LSB first                */
#define  F_BIG      0x200      /* object code is MSB first                */
#define  F_BYTE_ORDER (F_LITTLE | F_BIG)

#define  F_SYMMERGE 0x1000     /* Tags, etc were merged - no duplicates   */


/*------------------------------------------------------------------------*/
/*  OPTIONAL FILE HEADER                                                  */
/*------------------------------------------------------------------------*/
typedef struct aouthdr
{
   short   magic;          /* optional file header magic number    */
   short   vstamp;         /* version stamp                        */
   long    tsize;          /* text size in bytes, padded to FW bdry*/
   long    dsize;          /* initialized data "  "                */
   long    bsize;          /* uninitialized data "   "             */
   long    entrypt;        /* entry pt.                            */
   long    text_start;     /* base of text used for this file      */
   long    data_start;     /* base of data used for this file      */
} AOUTHDR;

#define AOUTSZ     sizeof(AOUTHDR)
#define AOUT1MAGIC 0410


/*------------------------------------------------------------------------*/
/*  COMMON ARCHIVE FILE STRUCTURES                                        */
/*                                                                        */
/*       ARCHIVE File Organization:                                       */
/*       +---------------------------------------------+                  */
/*       |          ARCHIVE_MAGIC_STRING               |                  */
/*       +---------------------------------------------+                  */
/*       |          ARCHIVE_FILE_MEMBER_1              |                  */
/*       +- - - - - - - - - - - - - - - - - - - - - - -+                  */
/*       |   Archive File Header "ar_hdr"              |                  */
/*       |   Contents (Ext symbol direct, text file)   |                  */
/*       +---------------------------------------------+                  */
/*       |          ARCHIVE_FILE_MEMBER_2              |                  */
/*       +- - - - - - - - - - - - - - - - - - - - - - -+                  */
/*       |   Archive File Header "ar_hdr"              |                  */
/*       |   Contents (.o or text file)                |                  */
/*       +---------------------------------------------+                  */
/*       |       .               .               .     |                  */
/*       |       .               .               .     |                  */
/*       |       .               .               .     |                  */
/*       +---------------------------------------------+                  */
/*       |          ARCHIVE_FILE_MEMBER_n              |                  */
/*       +- - - - - - - - - - - - - - - - - - - - - - -+                  */
/*       |   Archive File Header "ar_hdr"              |                  */
/*       |   Contents (.o or text file)                |                  */
/*       +---------------------------------------------+                  */
/*                                                                        */
/*------------------------------------------------------------------------*/

#define ARMAG   "!<arch>\n"
#define SARMAG  8
#define ARFMAG  "`\n"
#define ARFMAG_SIZE   2

struct ar_hdr           /* archive file member header - printable ascii */
{
   char    ar_name[16];    /* file member name - `/' terminated */
   char    ar_date[12];    /* file member date - decimal */
   char    ar_uid[6];      /* file member user id - decimal */
   char    ar_gid[6];      /* file member group id - decimal */
   char    ar_mode[8];     /* file member mode - octal */
   char    ar_size[10];    /* file member size - decimal */
   char    ar_fmag[2];     /* ARFMAG - string to end header */
};

#define ARHDR     struct ar_hdr
#define ARHSZ     sizeof(ARHDR)
#define AR_HDR_SZ sizeof(ARHDR)
#define AR_FNAME_SIZE 16

/*------------------------------------------------------------------------*/
/*  SECTION HEADER                                                        */
/*------------------------------------------------------------------------*/
#define  SYMNMLEN   8      /*  Number of characters in a symbol name      */

/*------------------------------------------------------------------------*/
/* THE OLD COFF VERSION TYPE DEFINITION FOR SECTION HEADERS TO PROVIDE    */
/* BACKWARDS COMPATIBILITY FOR COFF-DEPENDENT TOOLS THAT SUPPORT COFF 2.  */
/*------------------------------------------------------------------------*/
struct o_scnhdr 
{
	char            os_name[8];      /* section name                  */
	long            os_paddr;        /* physical address              */
	long            os_vaddr;        /* virtual address               */
	long            os_size;         /* section size                  */
	long            os_scnptr;       /* file ptr to raw data for scn  */
	long            os_relptr;       /* file ptr to relocation        */
	long            os_lnnoptr;      /* file ptr to line numbers      */
	unsigned short  os_nreloc;       /* number of relocation entries  */
	unsigned short  os_nlnno;        /* number of line number entries */
	unsigned short  os_flags;        /* flags                         */
	char            os_reserved;     /* reserved byte                 */
	unsigned char   os_page;         /* memory page id                */
};

/*------------------------------------------------------------------------*/
/* THE NEW COFF VERSION TYPE DEFINITION FOR SECTION HEADERS.  THIS        */
/* REVISION ALLOWS FOR UNRESTRICTED SECTION NAME LENGTH.                  */
/*------------------------------------------------------------------------*/
struct scnhdr
{
   union
   {
      char            _s_name[SYMNMLEN];   /* old COFF version name fld   */
      struct
      {
         long    _s_zeroes;                /* new == 0                    */
         long    _s_offset;                /* offset into string table    */
      } _s_s;
      char      *_s_nptr[2];               /* allows for overlaying       */
   } _s;

   long            s_paddr;        /* physical address                    */
   long            s_vaddr;        /* virtual address                     */
   long            s_size;         /* section size                        */
   long            s_scnptr;       /* file ptr to raw data for section    */
   long            s_relptr;       /* file ptr to relocation              */
   long            s_lnnoptr;      /* file ptr to line numbers            */
   unsigned long   s_nreloc;       /* number of relocation entries        */
   unsigned long   s_nlnno;        /* number of line number entries       */
   unsigned long   s_flags;        /* flags                               */
   short           s_reserved;     /* reserved 2 bytes                    */
   unsigned short  s_page;         /* memory page id                      */
};

#define s_name          _s._s_name
#define s_nptr          _s._s_nptr[1]
#define s_zeroes        _s._s_s._s_zeroes
#define s_offset        _s._s_s._s_offset

#define O_SCNHDR	struct o_scnhdr
#define SCNHDR		struct scnhdr
#define O_SCNHSZ	sizeof(O_SCNHDR)
#define SCNHSZ		sizeof(SCNHDR)
#define SCNHSZ_IN(version) (version == COFF_MAGIC_2 ? SCNHSZ : O_SCNHSZ)

/*------------------------------------------------------------------------*/
/* Define constants for names of "special" sections                       */
/*------------------------------------------------------------------------*/
#define _TEXT    ".text"
#define _DATA    ".data"
#define _BSS     ".bss"
#define _REG	 ".reg"
#define _CINIT   ".cinit"
#define _PINIT   ".pinit"

/*------------------------------------------------------------------------*/
/* The low 8 bits of s_flags is used as a section "type"                  */
/*------------------------------------------------------------------------*/
#define STYP_REG    0x00  /* "regular" : allocated, relocated, loaded     */
#define STYP_DSECT  0x01  /* "dummy"   : !allocated, relocated, !loaded   */
#define STYP_NOLOAD 0x02  /* "noload"  : allocated, relocated, !loaded    */
#define STYP_GROUP  0x04  /* not used                                     */
#define STYP_PAD    0x08  /* not used                                     */
#define STYP_COPY   0x10  /* "copy"    : used for C init tables - 
                                           not allocated, relocated,
                                           loaded;  reloc & lineno
                                           entries processed normally     */
#define STYP_TEXT   0x20  /* section contains text only                   */
#define STYP_DATA   0x40  /* section contains data only                   */
#define STYP_BSS    0x80  /* section contains bss only                    */

/*------------------------------------------------------------------------*/
/* Bits 8-11 specify an alignment.  The alignment is (2 ** x).            */
/*------------------------------------------------------------------------*/
#define ALIGN_MASK         0xF00  /* mask for alignment factor            */
#define ALIGN_SIZE(s_flag) (1 << (((unsigned)s_flag & ALIGN_MASK) >> 8))

#define STYP_BLOCK  0x1000        /* use alignment as blocking factor */
#define STYP_PASS   0x2000        /* Pass section through unchanged   */
#define STYP_CLINK  0x4000	  /* Conditionally link section       */



/*------------------------------------------------------------------------*/
/*  RELOCATION ENTRIES                                                    */
/*  WE SUPPORT TWO TYPES OF RELOCATION ENTRIES:                           */
/*     1) OLD STYLE, WITH 16 BIT SYMBOL TABLE INDEX.                      */
/*     2) NEW STYLE, WITH 32 BIT SYMBOL TABLE INDEX.                      */
/*  FOR ANY GIVEN INPUT FILE, THE FILE HEADER FLAG "F_RELOC_12" INDICATES */
/*  THE TYPE OF RELOCATION ENTRY IN THE FILE.                             */
/*  THE TARGET-SPECIFIC FLAG RELOC_ENTRY_12 DETERMINES WHETHER THE NEW    */
/*  STYLE IS SUPPORTED ON A GIVEN TARGET.                                 */
/*------------------------------------------------------------------------*/
typedef struct reloc_old
{
   long            r_vaddr;       /* (virtual) address of reference       */
   short           r_symndx;      /* index into symbol table              */
   unsigned short  r_disp;        /* additional bits for addr calc        */
   unsigned short  r_type;        /* relocation type                      */
} RELOC_OLD;

typedef struct reloc
{
   long            r_vaddr;       /* (virtual) address of reference       */
#if COFF_VERSION_0 
   short           r_symndx;      /* 16-bit index into symbol table       */
#else
   long            r_symndx;      /* 32-bit index into symbol table       */
#endif
   unsigned short  r_disp;        /* additional bits for addr calc        */
   unsigned short  r_type;        /* relocation type                      */
} RELOC;

#define RELSZ              (COFF_VERSION_0 ? 10 : 12)
#define RELSZ_IN(version)  ((version >= COFF_MAGIC_1) ? 12 : 10)

/*------------------------------------------------------------------------*/
/*   define all relocation types                                          */
/*------------------------------------------------------------------------*/
#define R_ABS           0         /* absolute address - no relocation     */
#define R_DIR16         01        /* UNUSED                               */
#define R_REL16         02        /* UNUSED                               */
#define R_DIR24         04        /* UNUSED                               */
#define R_REL24         05        /* 24 bits, direct                      */
#define R_DIR32         06        /* UNUSED                               */
#define R_RRRELREG     016        /* RR: 8 bit relocatable register       */
#define R_RELBYTE      017        /* 8 bits, direct                       */
#define R_RELWORD      020        /* 16 bits, direct                      */
#define R_RELLONG      021        /* 32 bits, direct                      */
#define R_PCRBYTE      022        /* 8 bits, PC-relative                  */
#define R_PCRWORD      023        /* 16 bits, PC-relative                 */
#define R_PCRLONG      024        /* 32 bits, PC-relative                 */
#define R_PCR24        025        /* 24 bits, PC-relative                 */
#define R_PCR23H       026        /* 23 bits, PC-rel in halfwords(x>>1)   */
#define R_PCR24W       027        /* 24 bits, PC-rel in words (x >> 2)    */
#define R_OCRLONG      030        /* GSP: 32 bits, one's compl direct     */
#define R_GSPPCR16     031        /* GSP: 16 bits, PC relative (in words) */
#define R_GSPOPR32     032        /* GSP: 32 bits, direct big-endian      */
#define R_GSPPCA16     033        /* GSP: same as GSPPCR16, but PC const  */
#define R_OCBD32       034        /* GSP: 32 bits, 1's compl,big-endian   */
#define R_RRNREG       035        /* RR: 8 bit reloc. reg. w/ neg off     */
#define R_PARTLS16     040        /* Brahma: 16 bit offset of 24 bit addr */
#define R_PARTMS8      041        /* Brahma: 8 bit page of 24 bit addr    */
#define R_PARTLS7      050        /* DSP: 7 bit offset of 16 bit addr     */
#define R_PARTMS9      051        /* DSP: 9 bit page of 16 bit addr       */
#define R_REL13        052        /* DSP: 13 bits, direct                 */
#define R_REL23        053        /* DSP,C54X: 23 bits, direct (ext addr) */
#define R_RELXPC       054        /* DSP,C54X: 16 bits, relative to XPC   */
#define R_HIEXT        055        /* C54X: Hi word of extended prog addr  */
#define R_HIWORD       061	  /* RR: 8 bit reloc. hi byte of word     */
#define R_LABCOD       062        /* C16 16-bit code address relocation   */

#define R_PPBASE       064        /* PP: Global Base address type         */
#define R_PPLBASE      065        /* PP: Local Base address type          */
#define R_PP15	       070        /* PP: Global 15 bit offset             */
#define R_PP15W        071        /* PP: Global 15 bit offset / 4         */
#define R_PP15H	       072        /* PP: Global 15 bit offset / 2         */
#define R_PP16B        073        /* PP: Global 16 bit offset for bytes   */
#define R_PPL15	       074        /* PP: Local 15 bit offset              */
#define R_PPL15W       075        /* PP: Local 15 bit offset divided by 4 */
#define R_PPL15H       076        /* PP: Local 15 bit offset divided by 2 */
#define R_PPL16B       077        /* PP: Local 16 bit offset for bytes    */
#define R_PPN15	       0100       /* PP: Global 15 bit neg offset         */
#define R_PPN15W       0101       /* PP: Global 15 bit neg offset / 4     */
#define R_PPN15H       0102       /* PP: Global 15 bit neg offset / 2     */
#define R_PPN16B       0103       /* PP: Global 16 bit neg byte offset    */
#define R_PPLN15       0104       /* PP: Local 15 bit neg offset          */
#define R_PPLN15W      0105       /* PP: Local 15 bit neg offset / 4      */
#define R_PPLN15H      0106       /* PP: Local 15 bit neg offset / 2      */
#define R_PPLN16B      0107       /* PP: Local 16 bit neg byte offset     */

#define R_MPPCR	       0117       /* MP: 32-bit PC-relative / 4           */

#define R_C60BASE      0120	  /* C60: Data Page Pointer Based Offset  */
#define R_C60DIR15     0121       /* C60: LD/ST long Displacement         */
#define R_C60PCR21     0122       /* C60: 21-bit Packet PC Relative       */
#define R_C60LO16      0124       /* C60: MVK Low Half Register           */
#define R_C60HI16      0125       /* C60: MVKH/MVKLH High Half Register   */

#define R_C8PHIBYTE    0130	  /* C8+: High byte of 24-bit address.    */
#define R_C8PMIDBYTE   0131	  /* C8+: Middle byte of 24-bit address.  */
#define R_C8PVECADR    0132	  /* C8+: Vector address (0xFFnnnn)       */
#define R_C8PADR24     0133	  /* C8+: 24-bit address (rev byte order) */

#define R_PARTLS6      0135       /* ANKOOR: 6 bit offset of 22 bit addr  */
#define R_PARTMID10    0136       /* ANKOOR: Middle 10 bits of 22 bit addr*/ 
#define R_REL22        0137       /* ANKOOR: 22 bits, direct              */
#define R_PARTMS6      0140       /* ANKOOR: Upper 6 bits of 22 bit addr  */
#define R_PARTMS16     0141       /* ANKOOR: Upper 16 bits of 22 bit addr */
#define R_ANKPCR16     0142       /* ANKOOR: PC relative 16 bit           */
#define R_ANKPCR8      0143       /* ANKOOR: PC relatvie 8 bit            */


/*------------------------------------------------------------------------*/
/*  LINE NUMBER ENTRIES                                                   */
/*------------------------------------------------------------------------*/
struct lineno
{
   union
   {
      long         l_symndx; /* sym index of fcn name iff l_lnno == 0     */
      long         l_paddr;  /* (physical) address of line number         */
   }  l_addr;
   unsigned short  l_lnno;   /* line number                               */
};

#define LINENO  struct lineno
#define LINESZ  6       /* sizeof(LINENO)                                 */


/*------------------------------------------------------------------------*/
/*  SYMBOL TABLE ENTRIES                                                  */
/*------------------------------------------------------------------------*/

#define  FILNMLEN   14     /*  Number of characters in a file name        */
#define  DIMNUM     4      /*  Number of array dimensions in aux entry    */

typedef unsigned short STYPE;

struct syment
{
    union
    {
       char            _n_name[SYMNMLEN];   /* old COFF version           */
       struct
       {
          long    _n_zeroes;                /* new == 0                   */
          long    _n_offset;                /* offset into string table   */
       } _n_n;
       char      *_n_nptr[2];               /* allows for overlaying      */
    } _n;

    long                    n_value;        /* value of symbol            */
    short                   n_scnum;        /* section number             */
    STYPE                   n_type;         /* type and derived type      */
    char                    n_sclass;       /* storage class              */
    char                    n_numaux;       /* number of aux. entries     */
};

#define n_name          _n._n_name
#define n_nptr          _n._n_nptr[1]
#define n_zeroes        _n._n_n._n_zeroes
#define n_offset        _n._n_n._n_offset

/*------------------------------------------------------------------------*/
/* Relocatable symbols have a section number of the                       */
/* section in which they are defined.  Otherwise, section                 */
/* numbers have the following meanings:                                   */
/*------------------------------------------------------------------------*/
#define  N_UNDEF  0                     /* undefined symbol               */
#define  N_ABS    -1                    /* value of symbol is absolute    */
#define  N_DEBUG  -2                    /* special debugging symbol       */


/*------------------------------------------------------------------------*/
/*  AUXILIARY SYMBOL ENTRY                                                */
/*------------------------------------------------------------------------*/
#define SPACE(len, name)    char  name[len]

union auxent
{
        struct
	{
	    SPACE(4, _0_3);
            long    x_fsize;        /* size of struct in bits.            */
	    SPACE(4, _8_11);
            long    x_endndx;       /* ptr to next sym beyond .eos        */
	    SPACE(2, _16_17);
	} x_tag;

        struct
	{
	    long    x_tagndx;       /* ptr to beginning of struct         */
	    long    x_fsize;        /* size of struct in bits.            */
	    SPACE(10, _8_17);
        } x_eos;

	struct 
	{
	    long    x_tagndx;       /* ptr to tag for function            */
	    long    x_fsize;        /* size of function in bits           */
	    long    x_lnnoptr;      /* file ptr to fcn line #             */
	    long    x_endndx;       /* ptr to next sym beyond .ef         */
	    SPACE(2, _16_17);
        } x_func;

        struct
	{
            long    x_regmask;      /* Mask of regs use by func           */
	    unsigned short  x_lnno; /* line number of block begin         */
	    unsigned short  x_lcnt; /* # line number entries in func      */
	    long    x_framesize;    /* size of func local vars            */
	    long    x_endndx;       /* ptr to next sym beyond .eb         */
	    SPACE(2, _16_17);
	} x_block;

        struct
	{
	    long    x_tagndx;       /* ptr to tag for array type          */
	    long    x_fsize;        /* Size of array in bits.             */
            unsigned short  x_dimen[DIMNUM];
	    SPACE(2, _16_17);
	} x_array;

	struct
	{
            long    x_tagndx;       /* str, un, or enum tag indx          */
	    long    x_fsize;        /* Size of symbol                     */
	    SPACE(10, _10_17);
	} x_sym;

        struct
        {
	    char    x_fname[FILNMLEN];
        } x_file;

        struct
        {
            long            x_scnlen;  /* section length                  */
            unsigned short  x_nreloc;  /* number of reloc entries         */
            unsigned short  x_nlinno;  /* number of line numbers          */
	    SPACE(8, _10_17);
        } x_scn;
};

#define SYMENT  struct syment
#define SYMESZ  18      /* sizeof(SYMENT) */

#define AUXENT  union auxent
#define AUXESZ  18      /* sizeof(AUXENT) */

/*------------------------------------------------------------------------*/
/*  NAMES OF "SPECIAL" SYMBOLS                                            */
/*------------------------------------------------------------------------*/
#define  _BF            ".bf"
#define  _EF            ".ef"
#define _STEXT          ".text"
#define _ETEXT          "etext"
#define _SDATA          ".data"
#define _EDATA          "edata"
#define _SBSS           ".bss"
#define _END            "end"
#define _CINITPTR       "cinit"
#define _PINITPTR       "pinit"
#define _ASM_SRC_NAME   "$ASM$"    /* SPECIAL SYMBOL FOR ASSY SRC DEBUG */

/*------------------------------------------------------------------------*/
/*  ENTRY POINT SYMBOLS                                                   */
/*------------------------------------------------------------------------*/
#define _START          "_start"
#define _MAIN           "_main"

