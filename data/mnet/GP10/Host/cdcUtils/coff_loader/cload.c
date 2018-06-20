/***************************************************************************
* FILENAME: cload.c
* VERSION:  2.6  5/2/96  13:11:03
* SCCS ID:  "@(#)cload.c	2.6  5/2/96"
***************************************************************************/
/******************************************************************************/
/* CLOAD.C  - Generic COFF Loader                      Version 6.00 4/96      */
/******************************************************************************/
/*                                                                            */
/*  This module contains functions to read in and load a COFF object file.    */
/*  The following routines are defined here:                                  */
/*                                                                            */
/*    cload()           - Main driver for COFF loader.                        */
/*    cload_headers()   - Read in the various headers of the COFF file.       */
/*    cload_data()      - Read in the raw data and load it into memory.       */
/*    cload_sect_data() - Read, relocate, and write out one section.          */
/*    cload_cinit()     - Process one buffer of C initialization records.     */
/*    cload_symbols()   - Read in the symbol table.                           */
/*    cload_strings()   - Read in the string table.                           */
/*    str_free()        - Free a string table.                                */
/*    sym_read()        - Read and relocate a symbol and its aux entry.       */
/*    sym_name()        - Return a pointer to the name of a symbol.           */
/*    sym_add_name()    - Add a symbol name to the string table.              */
/*    reloc_add()       - Add a symbol to the relocation symbol table.        */
/*    relocate()        - Perform a single relocation.                        */
/*    reloc_read()      - Read in and swap one relocation entry.              */
/*    reloc_size()      - Return the field size of a relocation type.         */
/*    reloc_offset()    - Return the field offset of a relocation type.       */
/*    reloc_stop()      - Return the number of bits to read for a reloc type. */
/*    sym_reloc_amount()- Return relocation amount for a relocation entry.    */
/*    unpack()          - Extract a relocation field from object bytes.       */
/*    repack()          - Encode a relocated field into object bytes.         */
/*    cload_lineno()    - Read in & swap line number entries.                 */
/*    swap4byte()       - Swap the order of bytes in a long.                  */
/*    swap2byte()       - Swap the order of bytes in a short.                 */
/*                                                                            */
/*  The loader calls the following external functions to perform application  */
/*  specific tasks:                                                           */
/*                                                                            */
/*   set_reloc_amount() - Define relocation amounts for each section.         */
/*   mem_write()        - Load a buffer of data into memory.                  */
/*   lookup_sym()       - Look up a symbol in an external symbol table.       */
/*   load_syms()        - Build the symbol table for the application.         */
/*   myalloc()          - Application version of malloc().                    */
/*   mralloc()          - Application version of realloc().                   */
/*   load_msg()         - Write diagnostic messages during loading.           */
/*                                                                            */
/******************************************************************************/
#include "header.h"


/*----------------------------------------------------------------------------*/
/* CONSTANTS, MACROS, VARIABLES, AND STRUCTURES FOR THE LOADER.               */
/*----------------------------------------------------------------------------*/
#define TRUE 1
#define FALSE 0

#define WORDSZ sizeof(T_DATA)           /* SIZE OF DATA UNITS IN OBJ FILE     */

/*----------------------------------------------------------------------------*/
/* APPLICATION VARIABLES                                                      */
/*----------------------------------------------------------------------------*/
FILE   *fin;                            /* INPUT FILE                         */
int     verbose      = FALSE;           /* PRINT PROGRESS INFO                */
int     need_data    = TRUE;            /* APPLICATION NEEDS RAW DATA         */
int     need_symbols = FALSE;           /* APPLICATION NEEDS SYMBOL TABLE     */
int     clear_bss    = FALSE;           /* CLEAR BSS SECTION                  */

/*----------------------------------------------------------------------------*/
/* LOADER VARIABLES                                                           */
/*----------------------------------------------------------------------------*/
FILHDR  file_hdr;                       /* FILE HEADER STRUCTURE              */
int     coff_version;                   /* COFF VERSION USED IN FILE          */
AOUTHDR o_filehdr;                      /* OPTIONAL (A.OUT) FILE HEADER       */
T_ADDR  entry_point;                    /* ENTRY POINT OF MODULE              */
T_ADDR *reloc_amount = NULL;            /* AMOUNT OF RELOCATION PER SECTION   */
char   *sect_hdrs = NULL;               /* ARRAY OF SECTION HEADERS           */
char   *o_sect_hdrs = NULL;             /* ARRAY OF OLD COFF SECTION HEADERS  */
int     n_sections;                     /* NUMBER OF SECTIONS IN THE FILE     */
int     big_e_target;                   /* TARGET DATA IN BIG-ENDIAN FORMAT   */
int     byte_swapped;                   /* BYTE ORDERING OPPOSITE OF HOST     */
int     curr_sect;                      /* SECTION NUMBER CURRENTLY LOADING   */
int     load_err;                       /* ERROR CODE RETURNED IF LOADER FAILS*/
struct strtab *str_head = NULL;         /* LIST OF STRING TABLE BUFFERS       */

static T_SIZE  init_size = 0;           /* CURRENT SIZE OF C INITIALIZATION   */
static int     need_reloc;              /* RELOCATION REQUIRED                */

#ifdef notnow
int big_e_config = TRG_DEF_BIG_E;       /* ENDIANNESS CONFIGURATION           */
#endif
int big_e_config = FALSE;               /* ENDIANNESS CONFIGURATION           */

/*----------------------------------------------------------------------------*/
/* THIS STRUCTURE IS USED TO STORE THE RELOCATION AMOUNTS FOR SYMBOLS.        */
/* EACH RELOCATABLE SYMBOL HAS A CORRESPONDING ENTRY IN THIS TABLE.           */
/* THE TABLE IS SORTED BY SYMBOL INDEX; LOOKUP USES A BINARY SEARCH.          */
/*----------------------------------------------------------------------------*/
typedef struct
{
   long rt_index;                       /* INDEX OF SYMBOL IN SYMBOL TABLE    */
   long rt_disp;                        /* AMOUNT OF RELOCATION               */
} RELOC_TAB;

/*----------------------------------------------------------------------------*/
/* THE RELOCATION SYMBOL TABLE IS ALLOCATED DYNAMICALLY, AND REALLOCATED      */
/* AS MORE SPACE IS NEEDED.                                                   */
/*----------------------------------------------------------------------------*/
#define RELOC_TAB_START 128             /* STARTING SIZE OF TABLE             */
#define RELOC_GROW_SIZE 128             /* REALLOC AMOUNT FOR TABLE           */

#ifdef notnow
static RELOC_TAB *reloc_tab = NULL;     /* RELOCATION SYMBOL TABLE            */
#endif
RELOC_TAB *reloc_tab = NULL;     /* RELOCATION SYMBOL TABLE            */

static int        reloc_tab_size;       /* CURRENT ALLOCATED AMOUNT           */
static int        reloc_sym_index;      /* CURRENT SIZE OF TABLE              */

/*----------------------------------------------------------------------------*/
/* RUN-TIME RELOCATION (AS OPPOSED TO LOAD-TIME) RELOCATION IS DETERMINED     */
/* AS FOLLOWS:  IF THE SECTION'S RUNTIME ADDRESS == LOADTIME ADDRESS, USE     */
/* LOADTIME RELOCATION.  OTHERWISE, ASSUME LOADTIME RELOC ONLY (RUNTIME RELOC */
/* == 0).                                                                     */
/*----------------------------------------------------------------------------*/
#define RUN_RELOC_AMOUNT(i) ((SECT_HDR(i)->s_paddr == SECT_HDR(i)->s_vaddr) ?  \
				    reloc_amount[i] : 0)

/*----------------------------------------------------------------------------*/
/* DEFINE A STRUCTURE FOR STRING TABLE BUFFERS.  THESE BUFFERS ARE STORED     */
/* AS A LINKED LIST OF MEMORY PACKETS, EACH CONTAINING UP TO 64K OF THE       */
/* STRING TABLE.                                                              */
/*----------------------------------------------------------------------------*/
typedef struct strtab
{
   unsigned long  size;                 /* SIZE OF THIS PACKET                */
   unsigned long  offset;               /* STARTING OFFSET OF THIS PACKET     */
   unsigned long  index;                /* AMOUNT CURRENTLY FILLED            */
   struct strtab *next;                 /* NEXT BUFFER                        */
   char           buf[1];               /* STRING DATA (EXPAND AS ALLOCATED)  */
} STRTAB;

#define MAX_STRING_ALLOC (unsigned int)(0xffff-sizeof(STRTAB)+1)
                                       /* MAX STRING BUFFER: 64K (SMALL HOSTS)*/
#define MIN_STRING_ALLOC 0x0400        /* MIN STRING BUFFER: 1K               */

unsigned long unpack();
int cload_strings();
int reloc_offset(int);
void str_free(STRTAB *);


/******************************************************************************/
/*                                                                            */
/* CLOAD() - Main driver for COFF loader.                                     */
/*                                                                            */
/******************************************************************************/
int cload()
{
   int result;

   load_err = 0;
   result   = cload_headers() && cload_symbols() && cload_data();

   if (reloc_tab) myfree(reloc_tab);
   reloc_tab = NULL;

   return result;
}


/******************************************************************************/
/*                                                                            */
/* CLOAD_HEADERS() - Read in the various headers of the COFF file.            */
/*                                                                            */
/******************************************************************************/
int cload_headers()
{
   int i;

   byte_swapped = FALSE;
   need_reloc   = FALSE;

   if (fseek(fin, 0L, 0) != 0 || !fread(&file_hdr, FILHSZ, 1, fin)) 
      { load_err = E_FILE; return FALSE; }

   /*-------------------------------------------------------------------------*/
   /* MAKE SURE THIS IS REALLY A COFF FILE. CHECK FOR SWAPPED FILES.          */
   /* DETERMINE BYTE ORDERING OF OBJECT DATA.                                 */
   /*-------------------------------------------------------------------------*/
   if (!ISCOFF(file_hdr.f_magic))
   {
       swap2byte(&file_hdr.f_magic);

       if (!ISCOFF(file_hdr.f_magic)) { load_err = E_MAGIC; return FALSE; }

       byte_swapped = TRUE;

       swap2byte(&file_hdr.f_nscns);  swap4byte(&file_hdr.f_timdat);
       swap4byte(&file_hdr.f_symptr); swap4byte(&file_hdr.f_nsyms);
       swap2byte(&file_hdr.f_opthdr); swap2byte(&file_hdr.f_flags);
#if COFF_VERSION_1 || COFF_VERSION_2
       swap2byte(&file_hdr.f_target_id); 
#endif
   }

   /*-------------------------------------------------------------------------*/
   /* DETERMINE THE ENDIANNESS OF THE COFF FILE, AND ENSURE THE ENDIANNESS OF */
   /* THE FILE IS THE SAME AS THE TARGET, IF THERE IS A TARGET.               */
   /*-------------------------------------------------------------------------*/
   big_e_target = ((file_hdr.f_flags & F_BIG) != 0);
   if (big_e_config != DONTCARE && big_e_target != big_e_config) 
      { load_err = E_ENDIAN; return FALSE; }

   /*-------------------------------------------------------------------------*/
   /* DETERMINE VERSION OF COFF BEING USED, CHECK TARGET ID IF NEEDED.        */
   /*-------------------------------------------------------------------------*/
   if (ISCOFF_1(file_hdr.f_magic) || ISCOFF_2(file_hdr.f_magic))
   {
       if (!ISMAGIC(file_hdr.f_target_id)) { load_err = E_MAGIC; return FALSE; }
       coff_version = file_hdr.f_magic;
   } 
   else coff_version = COFF_MAGIC_0;

   /*-------------------------------------------------------------------------*/
   /* READ IN OPTIONAL HEADER, IF THERE IS ONE, AND SWAP IF NEEDED.           */
   /*-------------------------------------------------------------------------*/
   if (file_hdr.f_opthdr == AOUTSZ)
   {
      fseek(fin, (long)FILHSZ_IN(coff_version), 0); 
      if (fread(&o_filehdr, file_hdr.f_opthdr, 1, fin) != 1) 
	 { load_err = E_FILE; return FALSE; }

      if (byte_swapped)
      {
	  swap2byte(&o_filehdr.magic);      swap2byte(&o_filehdr.vstamp);
	  swap4byte(&o_filehdr.tsize);      swap4byte(&o_filehdr.dsize);
	  swap4byte(&o_filehdr.bsize);      swap4byte(&o_filehdr.entrypt);
	  swap4byte(&o_filehdr.text_start); swap4byte(&o_filehdr.data_start);
      }
      entry_point = o_filehdr.entrypt;
   }

   /*-------------------------------------------------------------------------*/
   /* Read in string table so that we can see long section names, if needed.  */
   /* This used tobe read right before the symbol table was read, but now the */
   /* section headers use "flexname" method to specify section names and so   */
   /* might need access to a string table entry.                              */
   /*-------------------------------------------------------------------------*/
   if (!cload_strings()) return FALSE;

   /*-------------------------------------------------------------------------*/
   /* READ IN SECTION HEADERS.                                                */
   /*-------------------------------------------------------------------------*/
   if (sect_hdrs)   myfree(sect_hdrs);
   if (o_sect_hdrs) myfree(o_sect_hdrs);

   if (!(sect_hdrs = myalloc((n_sections = file_hdr.f_nscns) * SCNHSZ)))
      { load_err = E_ALLOC;  return FALSE; }

   fseek(fin, (long)FILHSZ_IN(coff_version) + file_hdr.f_opthdr, 0); 

   /*-------------------------------------------------------------------------*/
   /* Depending on which version of COFF we are reading, set up the section   */
   /* headers or s copy that we can translate into the new version.           */
   /*-------------------------------------------------------------------------*/
   if (ISCOFF_2(coff_version))
   {
      if (fread(sect_hdrs,SCNHSZ_IN(coff_version),n_sections,fin) != n_sections)
         { load_err = E_FILE;   return FALSE; }
   }
   else
   {
      if (!(o_sect_hdrs = myalloc(n_sections * SCNHSZ_IN(coff_version))))
         { load_err = E_ALLOC;  return FALSE; }

      if (fread(o_sect_hdrs,SCNHSZ_IN(coff_version),n_sections,fin) != n_sections)
         { load_err = E_FILE;   return FALSE; }
   }

   if (reloc_amount) myfree(reloc_amount);

   if (!(reloc_amount = myalloc(n_sections * sizeof(T_ADDR))))
      { load_err = E_ALLOC;  return FALSE; }

   /*-------------------------------------------------------------------------*/
   /* SWAP SECTION HEADERS IF REQUIRED.                                       */
   /*-------------------------------------------------------------------------*/
   for (i = 0; i < n_sections; i++)
   {
       SCNHDR   *sptr = SECT_HDR(i);
       O_SCNHDR *tptr = O_SECT_HDR(i);

       if (byte_swapped)
       {
          /*-----------------------------------------------------------------*/
	  /* Swap sections according to native COFF version.                 */
	  /*-----------------------------------------------------------------*/
	  if (ISCOFF_2(coff_version))
	  {
             if (sptr->s_zeroes == 0L) swap4byte(&sptr->s_offset);
             swap4byte(&sptr->s_paddr);  swap4byte(&sptr->s_vaddr);
             swap4byte(&sptr->s_size);   swap4byte(&sptr->s_scnptr);
             swap4byte(&sptr->s_relptr); swap4byte(&sptr->s_lnnoptr);
             swap4byte(&sptr->s_nreloc); swap4byte(&sptr->s_nlnno);
             swap4byte(&sptr->s_flags);  swap2byte(&sptr->s_page);
	  }
	  else
	  {
             swap4byte(&tptr->os_paddr);  swap4byte(&tptr->os_vaddr);
             swap4byte(&tptr->os_size);   swap4byte(&tptr->os_scnptr);
             swap4byte(&tptr->os_relptr); swap4byte(&tptr->os_lnnoptr);
             swap2byte(&tptr->os_nreloc); swap2byte(&tptr->os_nlnno);
             swap2byte(&tptr->os_flags);
	  }
       }

       /*---------------------------------------------------------------------*/
       /* Old COFF version section headers are now ready to be transferred.   */
       /*---------------------------------------------------------------------*/
       if (!ISCOFF_2(coff_version))
       {
          strncpy(sptr->s_name, tptr->os_name, SYMNMLEN);
          sptr->s_paddr   = tptr->os_paddr;
          sptr->s_vaddr   = tptr->os_vaddr;
          sptr->s_size    = tptr->os_size;
          sptr->s_scnptr  = tptr->os_scnptr;
          sptr->s_relptr  = tptr->os_relptr;
          sptr->s_lnnoptr = tptr->os_lnnoptr;
          sptr->s_nreloc  = tptr->os_nreloc;
          sptr->s_nlnno   = tptr->os_nlnno;
          sptr->s_flags   = tptr->os_flags;
          sptr->s_page    = tptr->os_page;
       }

       reloc_amount[i] = 0;
    
      /*---------------------------------------------------------------------*/
      /* Fix up section name if it is a pointer into the string table.       */
      /*---------------------------------------------------------------------*/
      if (sptr->s_zeroes == 0L)
      {
         STRTAB *packet = str_head;
         while (sptr->s_offset < (long)packet->offset) packet = packet->next;
         sptr->s_nptr = packet->buf + (sptr->s_offset - packet->offset);
      }
   }

   /*-------------------------------------------------------------------------*/
   /* CALL AN EXTERNAL ROUTINE TO DETERMINE THE RELOCATION AMOUNTS FOR        */
   /* EACH SECTION.                                                           */
   /*-------------------------------------------------------------------------*/
   if (!set_reloc_amount()) { load_err = E_SETRELOC; return FALSE; }
   for (i = 0; i < n_sections; i++) need_reloc |= (reloc_amount[i] != 0);
   if (!need_data) need_reloc = FALSE;

   if (need_reloc && (file_hdr.f_flags & F_RELFLG)) 
      { load_err = E_RELOC; return FALSE; }

   if (verbose) load_msg("   %d sections, %s format, %s data\n   %s\n",
		          n_sections, 
			  byte_swapped ? "swapped" : "unswapped",
			  big_e_target ? "big-E" : "little-E",
			  need_reloc   ? "Relocatable file" : "No reloc");

   return TRUE;
}


/******************************************************************************/
/*                                                                            */
/* CLOAD_DATA() - Read in the raw data and load it into memory.               */
/*                                                                            */
/******************************************************************************/
int cload_data()
{
   int ok = TRUE;

   if (!need_data) return TRUE;

   /*-------------------------------------------------------------------------*/
   /* LOOP THROUGH THE SECTIONS AND LOAD THEM ONE AT A TIME.                  */
   /*-------------------------------------------------------------------------*/
   for (curr_sect = 0; curr_sect < n_sections && ok; curr_sect++)
   {
      SCNHDR *sptr = SECT_HDR(curr_sect);                        
      char   *sname = (sptr->s_zeroes == 0L) ? 
				sptr->s_nptr : SNAMECPY(sptr->s_name);

      /*----------------------------------------------------------------------*/
      /* IF THIS IS THE TEXT SECTION, RELOCATE THE ENTRY POINT.               */
      /*----------------------------------------------------------------------*/
      if ((sptr->s_flags & STYP_TEXT) && !strcmp(sname, ".text"))
	 entry_point += RUN_RELOC_AMOUNT(curr_sect);

      /*----------------------------------------------------------------------*/
      /* IGNORE EMPTY SECTIONS OR SECTIONS WHOSE FLAGS INDICATE THE           */
      /* SECTION IS NOT TO BE LOADED.  IF THE CLEAR_BSS FLAG IS SET, BSS      */
      /* IS "LOADED" EVEN THOUGH IT HAS NO DATA, AND DEFER THE CINIT          */
      /* SECTION UNTIL LATER TO ENSURE BSS IS LOADED FIRST.                   */
      /*----------------------------------------------------------------------*/
      if ((sptr->s_scnptr || (clear_bss && IS_BSS(sptr)))             &&
	  (sptr->s_size)                                              &&
          !(sptr->s_flags & STYP_DSECT)                               &&
          !(sptr->s_flags & STYP_COPY)                                &&
	  !(sptr->s_flags & STYP_NOLOAD))
      {
	  if (verbose) load_msg("   Loading <%s>,\taddr=0x%lx, size=0x%03lx", 
		        sname, sptr->s_vaddr + reloc_amount[curr_sect], 
			sptr->s_size);

	  ok &= cload_sect_data(sptr);
          if (verbose) load_msg("\n");
      }

      /*----------------------------------------------------------------------*/
      /* IF CLEAR_BSS IS NOT SET, GO AHEAD AND DO CINIT IN ORDER.             */
      /*----------------------------------------------------------------------*/
      if (!clear_bss && IS_CINIT(sptr))
      {
	 if (verbose) load_msg("   Autoinitialization from <%s>", sname);
	 ok &= cload_sect_data(sptr);
	 if (verbose) load_msg("\n");
      }
   }

  /*-------------------------------------------------------------------------*/
  /* IF WE DEFERRED CINIT, LOAD IT/THEM NOW.                                 */
  /*-------------------------------------------------------------------------*/
  if (clear_bss)
     for (curr_sect = 0; curr_sect < n_sections && ok; curr_sect++)
     {
	SCNHDR *sptr = SECT_HDR(curr_sect);
	char   *sname = (sptr->s_zeroes == 0L) ? 
				  sptr->s_nptr : SNAMECPY(sptr->s_name);
	
	if (IS_CINIT(sptr))
	{
	   if (verbose) load_msg("   Autoinitialization from <%s>", sname);
	   ok &= cload_sect_data(sptr);
	   if (verbose) load_msg("\n");
	}
     }

   return ok;
}


/******************************************************************************/
/*                                                                            */
/* CLOAD_SECT_DATA() - Read, relocate, and write out the data for one section.*/
/*                                                                            */
/******************************************************************************/
int cload_sect_data(sptr)
   SCNHDR *sptr;
{
   T_ADDR        addr    = sptr->s_vaddr; /* CURRENT ADDRESS IN SECTION       */
   unsigned long nbytes;                  /* BYTE COUNT WITHIN SECTION        */
   int           packet_size;             /* SIZE OF CURRENT DATA BUFFER      */
   int           excess  = 0;             /* BYTES LEFT FROM PREVIOUS BUFFER  */
   unsigned int  n_reloc = 0;             /* COUNTER FOR RELOCATION ENTRIES   */
   RELOC         reloc;                   /* RELOCATION ENTRY                 */
   unsigned char packet[LOADBUFSIZE];     /* LOAD BUFFER                      */
   int           relsz = RELSZ_IN(coff_version); 

   /*-------------------------------------------------------------------------*/
   /* READ THE FIRST RELOCATION ENTRY, IF THERE ARE ANY.                      */
   /* IF THIS IS A BSS SECTION, CLEAR THE LOAD BUFFER.                        */
   /*-------------------------------------------------------------------------*/
   if (need_reloc && sptr->s_nreloc &&
       (fseek(fin, sptr->s_relptr, 0) != 0 || !reloc_read(&reloc)))
      { load_err = E_FILE; return FALSE; }

   if (IS_BSS(sptr))
#ifdef OTIS
      if (mem_fill((TRG_MVAL)0, LOCTOBYTE(sptr->s_size), addr, sptr->s_page))
         return TRUE;
      else { load_err = E_MEMWRITE; return FALSE; }
#else
      for (nbytes = 0; nbytes < LOADBUFSIZE; ++nbytes) packet[nbytes] = 0;
#endif

   /*-------------------------------------------------------------------------*/
   /* COPY ALL THE DATA IN THE SECTION.                                       */
   /*-------------------------------------------------------------------------*/
   for (nbytes = 0; nbytes < (unsigned long)LOCTOBYTE(sptr->s_size); 
	nbytes += packet_size)
   {
      int j;

      /*----------------------------------------------------------------------*/
      /* READ IN A BUFFER OF DATA.  IF THE PREVIOUS RELOCATION SPANNED        */
      /* ACCROSS THE END OF THE LAST BUFFER, COPY THE LEFTOVER BYTES INTO     */
      /* THE BEGINNING OF THE NEW BUFFER.                                     */
      /*----------------------------------------------------------------------*/
      for (j = 0; j < excess; ++j) packet[j] = packet[packet_size + j];

      packet_size = (int)MIN(LOCTOBYTE(sptr->s_size) - nbytes, LOADBUFSIZE);

      if ( !IS_BSS(sptr) &&
	  (fseek(fin, sptr->s_scnptr + (long)(nbytes + excess), 0) != 0 ||
	   fread(packet + excess, packet_size - excess, 1, fin) != 1))
         { load_err = E_FILE; return FALSE; }
      excess = 0;

      /*----------------------------------------------------------------------*/
      /* READ AND PROCESS ALL THE RELOCATION ENTRIES THAT AFFECT DATA         */
      /* CURRENTLY IN THE BUFFER.                                             */
      /*----------------------------------------------------------------------*/
      if (need_reloc)
	 while (n_reloc < sptr->s_nreloc && 
		(T_ADDR)reloc.r_vaddr < addr + BYTETOLOC(packet_size))
	 {
	    int i = (int)LOCTOBYTE(reloc.r_vaddr - addr);/*BYTE INDEX OF FIELD*/

	    /*----------------------------------------------------------------*/
	    /* IF THIS RELOCATION SPANS PAST THE END OF THE BUFFER,           */
	    /* SET 'EXCESS' TO THE NUMBER OF REMAINING BYTES AND FLUSH THE    */
	    /* BUFFER.  AT THE NEXT FILL, THESE BYTES WILL BE COPIED FROM     */
	    /* THE END OF THE BUFFER TO THE BEGINNING AND THEN RELOCATED.     */
	    /*----------------------------------------------------------------*/
	    if (i + MAX((int)WORDSZ, reloc_size(reloc.r_type)) > packet_size)
	    { 
	       i          -= i % LOADWORDSIZE;   /* DON'T BREAK WITHIN A WORD */
	       excess      = packet_size - i; 
	       packet_size = i;
	       break;
	    }

	    /*----------------------------------------------------------------*/
	    /* PERFORM THE RELOCATION AND READ IN THE NEXT RELOCATION ENTRY.  */
	    /*----------------------------------------------------------------*/
	    if (!relocate(&reloc, packet + i, curr_sect)) return FALSE;

	    if (n_reloc++ < sptr->s_nreloc                                    &&
	       (fseek(fin, sptr->s_relptr + ((long)n_reloc * relsz), 0) != 0  ||
		 !reloc_read(&reloc)))
               { load_err = E_FILE; return FALSE; }
	 }

      /*----------------------------------------------------------------------*/
      /* WRITE OUT THE RELOCATED DATA TO THE TARGET DEVICE.  IF THIS IS A     */
      /* CINIT SECTION, CALL A SPECIAL FUNCTION TO HANDLE IT.                 */
      /*----------------------------------------------------------------------*/
      if (!(IS_CINIT(sptr) ?
	      cload_cinit(packet, &packet_size, &excess) :
              mem_write(packet, packet_size, addr + reloc_amount[curr_sect], 
						            sptr->s_page)))
         { load_err = E_MEMWRITE; return FALSE; }

      /*----------------------------------------------------------------------*/
      /* KEEP TRACK OF THE ADDRESS WITHIN THE SECTION.                        */
      /*----------------------------------------------------------------------*/
      addr += BYTETOLOC(packet_size);

      if (verbose) load_msg(".");
   }
   return TRUE;
}


/******************************************************************************/
/*                                                                            */
/* CLOAD_CINIT() - Process one buffer of C initialization records.            */
/*                                                                            */
/******************************************************************************/
int cload_cinit(packet, packet_size, excess)
   unsigned char *packet;                /* LOAD BUFFER                       */
   int           *packet_size;           /* POINTER TO BUFFER SIZE            */
   int           *excess;                /* RETURNED NUMBER OF UNUSED BYTES   */
{
   int           i;                      /* BYTE COUNTER                      */
   int           init_packet_size;       /* SIZE OF CURRENT INITIALIZATION    */
   static T_ADDR init_addr;              /* ADDRESS OF CURRENT INITIALIZATION */
   int           bss_sect;               /* BSS SECTION ASSOC WITH CINIT SEC  */
 
   /*-------------------------------------------------------------------------*/
   /* FIND THE BSS SECTION ASSOCIATED WITH THE THE CINIT SECTION CURRENTLY    */
   /* BEING LOADED.                                                           */
   /*-------------------------------------------------------------------------*/
   for (bss_sect = 0; bss_sect < n_sections; ++bss_sect)
      if (IS_BSS(SECT_HDR(bss_sect))) break;

   /*-------------------------------------------------------------------------*/
   /*  PROCESS ALL THE INITIALIZATION RECORDS IN THE BUFFER.                  */
   /*-------------------------------------------------------------------------*/
   for (i = 0; i < *packet_size; i += init_packet_size)
   {
      /*----------------------------------------------------------------------*/
      /* IF STARTING A NEW INITIALIZATION, READ THE SIZE AND ADDRESS FROM     */
      /* THE TABLE.                                                           */
      /*----------------------------------------------------------------------*/
      if (init_size == 0)
      {
	 T_SIZE temp;
	 int    align;

	 /*-------------------------------------------------------------------*/
	 /* POSITION THE BYTE INDEX ON THE NEXT INIT RECORD.                  */
	 /*-------------------------------------------------------------------*/
         if (align = (i % INIT_ALIGN)) i += (INIT_ALIGN - align);

	 /*-------------------------------------------------------------------*/
	 /* IF THE SIZE AND ADDRESS ARE NOT FULLY CONTAINED IN THIS BUFFER,   */
	 /* STOP HERE.  SET THE 'EXCESS' COUNTER TO THE NUMBER OF UNPROCESSED */
	 /* BYTES - THESE WILL BE COPIED TO THE HEAD OF THE NEXT BUFFER.      */
	 /*-------------------------------------------------------------------*/
	 if ((int)(i + sizeof(T_SIZE)) > *packet_size)
	    { *excess += *packet_size - i;  *packet_size = i;  break; }

	 /*-------------------------------------------------------------------*/
	 /* IF THE NEXT SIZE FIELD IS ZERO, BREAK.                            */
	 /*-------------------------------------------------------------------*/
	 temp = unpack(packet + i, sizeof(T_SIZE)*8, sizeof(T_SIZE)*8, 0);
	 if (temp == 0) break;

	 /*-------------------------------------------------------------------*/
	 /* READ THE ADDRESS FIELD, IF IT'S ALL HERE.                         */
	 /*-------------------------------------------------------------------*/
	 if ((int)(i + sizeof(T_SIZE) + sizeof(T_IADDR)) > *packet_size)
	    { *excess += *packet_size - i;  *packet_size = i;  break; }

	 i         += sizeof(T_SIZE);
	 init_size  = temp;
	 init_addr  = unpack(packet+i, sizeof(T_IADDR)*8, sizeof(T_IADDR)*8, 0);
	 i         += sizeof(T_IADDR);
      }

      /*----------------------------------------------------------------------*/
      /* WRITE OUT THE CURRENT PACKET, UP TO THE END OF THE BUFFER.           */
      /*----------------------------------------------------------------------*/
      if (init_packet_size =
	     MIN(*packet_size - i, (int)(init_size * INIT_WSIZE)))
      {
	 if (verbose)
	     load_msg("\n\tLoading <.cinit>,\taddr=0x%lx, size=0x%04lx",
		      init_addr, init_size);

	 if (!mem_write(packet + i, init_packet_size, init_addr, 
				                    SECT_HDR(bss_sect)->s_page))
            return FALSE;

	 init_addr += BYTETOLOC(init_packet_size);
	 init_size -= init_packet_size / INIT_WSIZE;
      }
   }
   return TRUE;
}


/******************************************************************************/
/*                                                                            */
/* CLOAD_SYMBOLS() - Read in the symbol table.                                */
/*                                                                            */
/******************************************************************************/
int cload_symbols()
{
   SYMENT sym;
   AUXENT aux;
   long first, next;

   if (file_hdr.f_nsyms == 0 || (!need_symbols && !need_reloc)) return TRUE;

   /*------------------------------------------------------------------------*/
   /* ALLOCATE THE RELOCATION SYMBOL TABLE.                                  */
   /*------------------------------------------------------------------------*/
   if (need_reloc)
   {
      reloc_sym_index = 0;
      reloc_tab_size  = MIN(RELOC_TAB_START, (int)file_hdr.f_nsyms);

      if (!(reloc_tab = myalloc(reloc_tab_size * sizeof(RELOC_TAB))))
	  { load_err = E_ALLOC; return FALSE; }
   }

   /*------------------------------------------------------------------------*/
   /* IF THE APPLICATION NEEDS THE SYMBOL TABLE, LET IT READ IT IN.          */
   /* PASS NEED_RELOC TO THE APPLICATION SO THAT IT CAN CALL RELOC_ADD().    */
   /*------------------------------------------------------------------------*/
   if (need_symbols) 
   {
      if (load_syms(need_reloc)) return TRUE;
      else { load_err = E_SYM; return FALSE; }
   }

   /*------------------------------------------------------------------------*/
   /*  READ THE SYMBOL TABLE AND BUILD THE RELOCATION SYMBOL TABLE           */
   /*  FOR SYMBOLS THAT CAN BE USED IN RELCOATION, STORE THEM IN A           */
   /*  SPECIAL SYMBOL TABLE THAT CAN BE SEARCHED QUICKLY DURING              */
   /*  RELOCATION.                                                           */
   /*------------------------------------------------------------------------*/
   for (first = 0; first < file_hdr.f_nsyms; first = next)
   {
       if (!(next = sym_read(first, &sym, &aux))) 
          { load_err = E_FILE; return FALSE; }

       if (sym.n_sclass == C_EXT     || sym.n_sclass == C_EXTREF  ||
           sym.n_sclass == C_STAT    || sym.n_sclass == C_LABEL   ||
           sym.n_sclass == C_SYSTEM  || sym.n_sclass == C_BLOCK   || 
	   sym.n_sclass == C_FCN     || sym.n_sclass == C_STATLAB || 
	   sym.n_sclass == C_EXTLAB) 
	  if (!reloc_add(first, &sym)) return FALSE;
   }
   return TRUE;
}


/******************************************************************************/
/*                                                                            */
/* CLOAD_STRINGS() - Read in the string table.                                */
/*                                                                            */
/******************************************************************************/
int cload_strings()
{
   unsigned long str_size;              /* SIZE OF STRING TABLE              */
   unsigned long bufsize;               /* SIZE OF CURRENT BUFFER            */
   unsigned int ntoread;                /* AMOUNT TO READ FROM FILE          */
   int excess;                          /* AMOUNT LEFT OVER FROM LAST BUFFER */
   STRTAB *packet;                      /* STRING TABLE BUFFER PACKET        */

   /*------------------------------------------------------------------------*/
   /* FREE ANY PREVIOUS STRING BUFFERS                                       */
   /*------------------------------------------------------------------------*/
   str_free(str_head); str_head = NULL;

   /*------------------------------------------------------------------------*/
   /* SEEK TO THE END OF THE SYMBOL TABLE AND READ IN THE SIZE OF THE STRING */
   /* TABLE.                                                                 */
   /*------------------------------------------------------------------------*/
   if ((file_hdr.f_nsyms == 0) ||
   	   fseek(fin, file_hdr.f_symptr + (file_hdr.f_nsyms * SYMESZ), 0) != 0 ||
       fread(&str_size, sizeof(long), 1, fin) != 1)
       return TRUE;

   if (byte_swapped) swap4byte(&str_size);

   /*------------------------------------------------------------------------*/
   /* THE STRING TABLE IS READ IN AS A LINKED LIST OF BUFFERS.  TO           */
   /* PREVENT NAMES FROM BEING SPLIT ACROSS MULTIPLE BUFFERS, ANY PARTIAL    */
   /* NAME AT THE END OF A BUFFER IS COPIED INTO THE BEGINNING OF THE        */
   /* NEXT BUFFER.  THE VARIABLE 'EXCESS' KEEPS TRACK OF HOW MUCH NEEDS      */
   /* TO BE COPIED FROM THE PREVIOUS BUFFER.                                 */
   /*------------------------------------------------------------------------*/
   str_size -= 4;                       /* SUBTRACT OFF 4 BYTES ALREADY READ */
   excess    = 0;                       /* INITIALIZE LAST-BUFFER OVERFLOW   */

   if (verbose) load_msg("   %ld symbols.  String table size=%ld\n",
		     file_hdr.f_nsyms, str_size ? str_size : 0L);

   /*------------------------------------------------------------------------*/
   /* READ STRING BUFFERS UNTIL THE WHOLE STRING TABLE IS READ.              */
   /*------------------------------------------------------------------------*/
   while (str_size)
   {
      /*---------------------------------------------------------------------*/
      /* ALLOCATE A NEW BUFFER.  ON 16-BIT MACHINES, RESTRICT THE            */
      /* BUFFER SIZE TO THE MAXIMUM THAT CAN BE ALLOCATED AT ONCE.           */
      /*---------------------------------------------------------------------*/
      bufsize = str_size + excess;

      if (sizeof(int) < 4  && bufsize > MAX_STRING_ALLOC)
	 bufsize = MAX_STRING_ALLOC;

      if (!(packet = myalloc(sizeof(STRTAB) + (unsigned int)bufsize - 1)))
	 { load_err = E_ALLOC; return FALSE; }

      /*---------------------------------------------------------------------*/
      /* COPY ANY PARTIAL STRING FROM THE LAST BUFFER INTO THIS ONE.         */
      /* THEN FILL THE REST OF THE BUFFER BY READING FROM THE FILE.          */
      /*---------------------------------------------------------------------*/
      if (excess)
	 strncpy(packet->buf, str_head->buf + str_head->size, excess);

      ntoread = (unsigned int)(bufsize - excess);
      if (fread(packet->buf + excess, ntoread, 1, fin) != 1) 
	 { load_err = E_FILE; return FALSE; }
      str_size -= ntoread;

      /*---------------------------------------------------------------------*/
      /* IF THE BUFFER ENDS IN A PARTIAL STRING (DOESN'T END IN NULL),       */
      /* KEEP TRACK OF HOW MANY CHARACTERS ARE IN THE PARTIAL STRING         */
      /* SO THEY CAN BE COPIED INTO THE NEXT BUFFER.                         */
      /*---------------------------------------------------------------------*/
      for (excess = 0; packet->buf[bufsize - 1]; --bufsize, ++excess) ;

      /*---------------------------------------------------------------------*/
      /* LINK THE NEW BUFFER INTO THE HEAD OF THE LIST.                      */
      /*---------------------------------------------------------------------*/
      packet->size   = 
      packet->index  = bufsize;
      packet->next   = str_head;
      packet->offset = str_head ? (str_head->offset + str_head->size) : 4;
      str_head       = packet;
   }
   return TRUE;
}


/******************************************************************************/
/*                                                                            */
/* STR_FREE() - Free the list of string table buffers.                        */
/*                                                                            */
/******************************************************************************/
void str_free(str_head)
   STRTAB *str_head;
{
   STRTAB *this, *next;

   for (this = str_head; this; this = next)
   {
      next = this->next;
      myfree(this);
   }
}



/******************************************************************************/
/*                                                                            */
/* SYM_READ() - Read and relocate a symbol and its aux entry.  Return the     */
/*              index of the next symbol.                                     */
/*                                                                            */
/******************************************************************************/
long sym_read(index, sym, aux)
  long index;
  SYMENT *sym;
  AUXENT *aux;
{
    /*------------------------------------------------------------------------*/
    /* READ IN A SYMBOL AND ITS AUX ENTRY.                                    */
    /*------------------------------------------------------------------------*/
    if (fseek(fin, file_hdr.f_symptr + (index * SYMESZ), 0) != 0 ||
        fread(sym, SYMESZ, 1, fin) != 1                                ||
        (sym->n_numaux && fread(aux, SYMESZ, 1, fin) != 1)) 
      { load_err = E_FILE; return FALSE; }

    if (byte_swapped)
    {
        /*--------------------------------------------------------------------*/
	/* SWAP THE SYMBOL TABLE ENTRY.                                       */
        /*--------------------------------------------------------------------*/
        if (sym->n_zeroes == 0) swap4byte(&sym->n_offset);
        swap4byte(&sym->n_value);
        swap2byte(&sym->n_scnum);
        swap2byte(&sym->n_type);

        /*--------------------------------------------------------------------*/
	/* SWAP THE AUX ENTRY, BASED ON THE STORAGE CLASS.                    */
        /*--------------------------------------------------------------------*/
        if (sym->n_numaux) switch(sym->n_sclass)
        {
          case C_FILE    : break;

          case C_STRTAG  :
          case C_UNTAG   :
          case C_ENTAG   : swap4byte(&aux->x_tag.x_fsize);
                           swap4byte(&aux->x_tag.x_endndx);
                           break;

          case C_FCN     : if (!strcmp(sym->n_name, ".bf"))
			   {
		               swap2byte(&aux->x_block.x_lcnt);
			       swap4byte(&aux->x_block.x_regmask); 
			       swap4byte(&aux->x_block.x_framesize);
			   }
			        
          case C_BLOCK   : swap2byte(&aux->x_block.x_lnno);
			   swap4byte(&aux->x_block.x_endndx);
                           break;

          case C_EOS     : swap4byte(&aux->x_eos.x_fsize);
                           swap4byte(&aux->x_eos.x_tagndx);
                           break;

          default        : /*-------------------------------------------------*/
                           /* HANDLE FUNCTION DEFINITION SYMBOL               */
                           /*-------------------------------------------------*/
                           if (((sym->n_type >> 4) & 3) == DT_FCN)
                           {
                               swap4byte(&aux->x_func.x_tagndx);
                               swap4byte(&aux->x_func.x_fsize);
                               swap4byte(&aux->x_func.x_lnnoptr);
                               swap4byte(&aux->x_func.x_endndx);
                           }

                           /*-------------------------------------------------*/
                           /* HANDLE ARRAYS.                                  */
                           /*-------------------------------------------------*/
                           else if (((sym->n_type >> 4) & 3) == DT_ARY)
                           {
                               swap4byte(&aux->x_array.x_tagndx);
                               swap4byte(&aux->x_array.x_fsize);
                               swap2byte(&aux->x_array.x_dimen[0]);
                               swap2byte(&aux->x_array.x_dimen[1]);
                               swap2byte(&aux->x_array.x_dimen[2]);
                               swap2byte(&aux->x_array.x_dimen[3]);
                           }

                           /*-------------------------------------------------*/
                           /* HANDLE SECTION DEFINITIONS                      */
                           /*-------------------------------------------------*/
                           else if (sym->n_type == 0)
                           {
                               swap4byte(&aux->x_scn.x_scnlen);
                               swap2byte(&aux->x_scn.x_nreloc);
                               swap2byte(&aux->x_scn.x_nlinno);
                           }

                           /*-------------------------------------------------*/
                           /* HANDLE MISC SYMBOL RECORD                       */
                           /*-------------------------------------------------*/
                           else
                           {
                               swap4byte(&aux->x_sym.x_fsize);
                               swap4byte(&aux->x_sym.x_tagndx);
                           }
        }
    }

    /*------------------------------------------------------------------------*/
    /* RELOCATE THE SYMBOL, BASED ON ITS STORAGE CLASS.                       */
    /*------------------------------------------------------------------------*/
    switch(sym->n_sclass)
    {
       case C_SYSTEM  :
       case C_EXT     :
       case C_EXTREF  :
       case C_STAT    :
       case C_LABEL   :
       case C_BLOCK   :
       case C_FCN     : 
       case C_STATLAB :
       case C_EXTLAB  :
	  /*------------------------------------------------------------------*/
	  /* IF THE SYMBOL IS UNDEFINED, CALL AN APPLICATION ROUTINE TO LOOK  */
	  /* IT UP IN AN EXTERNAL SYMBOL TABLE.  IF THE SYMBOL IS DEFINED,    */
	  /* RELOCATE IT ACCORDING TO THE SECTION IT IS DEFINED IN.           */
	  /*------------------------------------------------------------------*/
          if (sym->n_scnum == 0) 
	     lookup_sym((short)index, sym, aux);
          else if (sym->n_scnum > 0) 
	  {
	     if (sym->n_sclass == C_STATLAB || sym->n_sclass == C_EXTLAB)
	          sym->n_value += reloc_amount[sym->n_scnum - 1];
	     else sym->n_value += RUN_RELOC_AMOUNT(sym->n_scnum - 1);
          }
    }

    return (index + sym->n_numaux + 1);
}


/******************************************************************************/
/*                                                                            */
/* SYM_NAME() - Return a pointer to the name of a symbol in either the symbol */
/*              entry or the string table.                                    */
/*                                                                            */
/******************************************************************************/
char *sym_name(symptr)
   SYMENT *symptr;
{
    static char temp[9];

    if (symptr->n_zeroes == 0)
    {
       STRTAB *packet = str_head;
       while (symptr->n_offset < (long)packet->offset) packet = packet->next;

       return packet->buf + (symptr->n_offset - packet->offset);
    }
    
    strncpy(temp, symptr->n_name, 8);
    temp[8] = 0;
    return temp;
}


/******************************************************************************/
/*                                                                            */
/* SYM_ADD_NAME() - Given a symbol table entry, return a pointer to the       */
/*                  symbol's name in the string table.  Add the name to the   */
/*                  table if it's not already there.                          */
/*                                                                            */
/******************************************************************************/
char *sym_add_name(symptr)
    SYMENT *symptr;
{
    char *dest;
    char *result;
    int i;

    /*------------------------------------------------------------------------*/
    /* IF THE SYMBOL'S NAME WAS IN THE COFF STRING TABLE, LOOK THROUGH THE    */
    /* LIST OF STRING TABLE BUFFERS UNTIL FINDING THE ONE THE NAME IS IN,     */
    /* AND SIMPLY POINT INTO THE BUFFER.                                      */
    /*------------------------------------------------------------------------*/
    if (symptr->n_zeroes == 0)
       return sym_name(symptr);

    /*------------------------------------------------------------------------*/
    /* OTHERWISE ADD THE STRING TO THE STRING TABLE.                          */
    /* ALLOCATE AND LINK IN A NEW PACKET IF NECESSARY.  NEW PACKETS ARE       */
    /* LINKED TO THE HEAD OF THE LIST TO EASE ADDING NEW SYMBOLS.             */
    /*------------------------------------------------------------------------*/
    if (!str_head || str_head->index + SYMNMLEN + 1 > str_head->size)
    {
       STRTAB *packet;

       if (!(packet = myalloc(sizeof(STRTAB) + MIN_STRING_ALLOC - 1)))
          { load_err = E_ALLOC; return NULL; }

       packet->size   = MIN_STRING_ALLOC;
       packet->index  = 0;
       packet->next   = str_head;
       packet->offset = str_head ? (str_head->offset + str_head->size) : 4;
       str_head       = packet;
    }

    /*------------------------------------------------------------------------*/
    /* COPY THE NAME INTO THE STRING TABLE.                                   */
    /*------------------------------------------------------------------------*/
    result = dest = str_head->buf + str_head->index; 
    for (i = 0; *dest++ = symptr->n_name[i++]; )
       if (i == SYMNMLEN) { *dest++ = '\0'; ++i; break; }

    symptr->n_zeroes = 0;
    symptr->n_offset = str_head->offset + str_head->index;
    str_head->index += i;
    return result;
}


/******************************************************************************/
/*                                                                            */
/* RELOC_ADD() - Add an entry to the relocation symbol table.  This table     */
/*               stores relocation information for each relocatable symbol.   */
/*                                                                            */
/******************************************************************************/
int reloc_add(index, sym)
   long   index;
   SYMENT *sym;
{
   long disp;                            /* RELOCATION AMOUNT FOR THIS SYMBOL */

   if (!need_reloc) return TRUE;

   /*-------------------------------------------------------------------------*/
   /* USE THE SYMBOL VALUE TO CALCULATE THE RELOCATION AMOUNT:                */
   /* 1) IF THE SYMBOL WAS UNDEFINED (DEFINED IN SECTION 0), USE THE          */
   /*    SYMBOL'S VALUE.                                                      */
   /* 2) IF THE SYMBOL HAS A POSITIVE SECTION NUMBER, USE THE RELOCATION      */
   /*    AMOUNT FOR THE SECTION IN WHICH THE SYMBOL IS DEFINED.               */
   /* 3) OTHERWISE, THE SYMBOL IS ABSOLUTE, SO THE RELOCATION AMOUNT IS 0.    */
   /*-------------------------------------------------------------------------*/
   if (sym->n_scnum == 0)
      disp = sym->n_value;
   else if (sym->n_scnum > 0)
   {
      if (sym->n_sclass == C_STATLAB || sym->n_sclass == C_EXTLAB)
	   disp = reloc_amount[sym->n_scnum - 1];
      else disp = RUN_RELOC_AMOUNT(sym->n_scnum - 1);
   }
   else disp = 0;

   /*-------------------------------------------------------------------------*/
   /* IF THERE IS A NON-ZERO RELOCATION AMOUNT, ADD THE SYMBOL TO THE TABLE.  */
   /*-------------------------------------------------------------------------*/
   if (disp == 0) return TRUE;

   if (reloc_sym_index >= reloc_tab_size)
   {
      reloc_tab_size += RELOC_GROW_SIZE;
      reloc_tab = mralloc((char *)reloc_tab, reloc_tab_size*sizeof(RELOC_TAB));

      if (!reloc_tab) { load_err = E_ALLOC; return FALSE; }
   }
   reloc_tab[reloc_sym_index  ].rt_index = index;
   reloc_tab[reloc_sym_index++].rt_disp  = disp;

   return TRUE;
}


/******************************************************************************/
/*                                                                            */
/* RELOCATE() - Perform a single relocation by patching the raw data.         */
/*                                                                            */
/******************************************************************************/
int relocate(rp, data, s)
   RELOC         *rp;                   /* RELOCATION ENTRY                   */
   unsigned char *data;                 /* DATA BUFFER                        */
   int            s;                    /* INDEX OF CURRENT SECTION           */
{
   int fieldsz = reloc_size(rp->r_type);     /* SIZE OF ACTUAL PATCH VALUE    */
   int offset  = reloc_offset(rp->r_type);   /* OFFSET OF ACTUAL PATCH VALUE  */
   int wordsz  = MAX(fieldsz, reloc_stop(rp->r_type)); /* SIZE CONTAINING FLD */
   long objval;                              /* FIELD TO BE PATCHED           */
   long reloc_amt;                           /* AMOUNT OF RELOCATION          */

   int pp_shift_cnt = 0;
   int pp_local     = FALSE;
   int pp_neg       = FALSE;

   if (rp->r_type == R_ABS) return TRUE;          /* NOTHING TO DO   */

   /*-------------------------------------------------------------------------*/
   /* DETERMINE THE RELOCATION AMOUNT FROM THE RELOCATION SYMBOL TABLE.       */
   /*-------------------------------------------------------------------------*/
   reloc_amt = (rp->r_symndx == -1) ? RUN_RELOC_AMOUNT(s) 
				    : sym_reloc_amount(rp);

   /*-------------------------------------------------------------------------*/
   /* EXTRACT THE RELOCATABLE FIELD FROM THE OBJECT DATA.                     */
   /*-------------------------------------------------------------------------*/
   objval = unpack(data, fieldsz, wordsz, offset + BIT_OFFSET(rp->r_vaddr));

   /*-------------------------------------------------------------------------*/
   /* MODIFY THE FIELD BASED ON THE RELOCATION TYPE.                          */
   /*-------------------------------------------------------------------------*/
   switch (rp->r_type)
   {
      /*----------------------------------------------------------------------*/
      /* NORMAL RELOCATIONS: ADD IN THE RELOCATION AMOUNT.                    */
      /*----------------------------------------------------------------------*/
      case R_RELBYTE:
      case R_RELWORD:
      case R_REL24:
      case R_RELLONG:
      case R_DIR32:
      case R_PARTLS16:
	 objval += reloc_amt;
	 break;

      /*--------------------------------------------------------------------*/
      /* ADD IN THE RELOCATION AMOUNT, BUT MAKE SURE WE'RE STILL IN THE     */
      /* 370'S REGISTER FILE.                                               */
      /*--------------------------------------------------------------------*/
      case R_RRNREG:
      case R_RRRELREG:
	 if (rp->r_type == R_RRNREG)
	    objval = ((char)objval + reloc_amt);
	 else objval += reloc_amt;
 
         if (objval & ((-1L >> 8*fieldsz) << 8*fieldsz))
         {
	    /* ERROR */
         }
 
         break;
 
      /*--------------------------------------------------------------------*/
      /* PP UNSCALED 15-BIT OFFSET RELOCATION.                              */
      /*--------------------------------------------------------------------*/
      case R_PP15    :
      case R_PPL15   :
      case R_PPN15   :
      case R_PPLN15  :
	 {
	    int  bit;
	    char *sname = (SECT_HDR(s)->s_zeroes == 0L) ?
			  SECT_HDR(s)->s_nptr : SNAMECPY(SECT_HDR(s)->s_name);

	    pp_local = (rp->r_type == R_PPL15) || (rp->r_type == R_PPLN15);

	    /*--------------------------------------------------------------*/
	    /* IF NEGATIVE RELOC TYPE, THEN TREAT CONST OFFSET AS A NEGATIVE*/
	    /*--------------------------------------------------------------*/
	    if (rp->r_type == R_PPN15 || rp->r_type == R_PPLN15)
	    {
		objval      = -objval;
                rp->r_type -= 010;           /* CHANGE TYPE TO NON NEG.     */
            }

	    objval += reloc_amt;

	    /*--------------------------------------------------------------*/
            /* IF THE ADDRESS STILL FALLS WITHIN AN APPROPRIATE RANGE       */
	    /*--------------------------------------------------------------*/
	    if ((objval >= 0x00000000 && objval <= 0x00007fff) || 
	        (objval >= 0x01000000 && objval <= 0x010007ff) )
	        break;

	    /*--------------------------------------------------------------*/
            /* IF THE ADDRESS FALLS OUTSIDE AN APPROPRIATE RANGE, BUT CAN   */
	    /* BE SCALED BY SIZE TO GET BACK INTO RANGE, THEN READ THE UPPER*/
	    /* BIT OF THE SIZE FIELD.  IF IT IS A 1, THEN WE CAN SCALE THIS */
	    /* OFFSET BY 4, IF IT IS 0, THEN WE CAN SCALE THIS OFFSET BY 2. */ 
	    /*--------------------------------------------------------------*/
	    bit = unpack(data, 1, 64, pp_local ? 30 : 8);
 
	    /*--------------------------------------------------------------*/
	    /* DETERMINE IF THE OFFSET IS ALIGNED FOR SCALING.  IF SO,      */
	    /* THEN PACK THE SCALED OFFSET INTO INSTRUCTION, CHANGE THE     */
	    /* RELOC TYPE TO SCALED, AND TURN ON SCALE BIT IN INSTRUCT.     */
	    /*--------------------------------------------------------------*/
	    if (!(objval & ((2<<bit)-1)) && 
		 (objval >>= (bit+1)) >= 0 && objval <= 0x7fff)
	    {
		rp->r_type = pp_local ? (bit ? R_PPL15W : R_PPL15H) : 
					(bit ? R_PP15W : R_PP15H);
		repack(1, data, 1, 64, pp_local ? 28 : 6);
	        break;
	    }
	    
	    /*--------------------------------------------------------------*/
	    /* ERROR, THE OFFSET WILL NOT FIT SCALED OR UNSCALED.           */
	    /*--------------------------------------------------------------*/
	    if (verbose)
	        load_msg( "PP 15-bit offset overflow at %08lx in section %s",
			   rp->r_vaddr, sname);
	    load_err = E_RELOCENT;
	    return FALSE;
         }

      /*--------------------------------------------------------------------*/
      /* PP SCALED 15-BIT OFFSET RELOCATION. FOR R_PP15W THE RELOC_AMT IS   */
      /* DIVIDED BY 4.  FOR R_PP15H THE RELOC_AMT IS DIVIDED BY 2.          */ 
      /*--------------------------------------------------------------------*/
      case R_PP15W   :
      case R_PPL15W  :
      case R_PPN15W  :
      case R_PPLN15W :	pp_shift_cnt++;   /* FALL THROUGH */

      case R_PP15H   :
      case R_PPL15H  :
      case R_PPN15H  :
      case R_PPLN15H :  pp_shift_cnt++;   /* FALL THROUGH */
	 {
	    long obj_addr_x;
	    char *sname = (SECT_HDR(s)->s_zeroes == 0) ? 
			  SECT_HDR(s)->s_nptr : SNAMECPY(SECT_HDR(s)->s_name);

	    /*--------------------------------------------------------------*/
	    /* NOTE THAT THIS IS DEPENDENT ON THE NUMBERING OF THESE RELOC  */
	    /* VALUES.                                                      */
	    /*--------------------------------------------------------------*/
	    pp_local = (rp->r_type & 4);

	    /*--------------------------------------------------------------*/
	    /* IF NEGATIVE RELOC TYPE, THEN TREAT CONST OFFSET AS NEGATIVE  */
	    /*--------------------------------------------------------------*/
	    if (rp->r_type >= R_PPN15) 
	    {
		objval      = -objval;
                rp->r_type -= 010;           /* CHANGE TYPE TO NON NEG.     */
            }

	    obj_addr_x = (objval << pp_shift_cnt) + reloc_amt;

	    /*--------------------------------------------------------------*/
            /* LINK TIME ADDRESS VIOLATES THE SCALING FACTOR WE ARE USING   */
	    /* FOR THIS OPERAND. UNSCALE THE VALUE IF IT WILL FIT IN 15 BITS*/
	    /* BY CHANGING RELOC TYPE TO UNSCALED, AND CHANGING SCALE BIT   */
	    /* IN THE INSTRUCTION.                                          */
	    /*--------------------------------------------------------------*/
	    if (pp_shift_cnt && (reloc_amt & ((1<<pp_shift_cnt)-1)))
	    {
		objval     = obj_addr_x;
		rp->r_type = (pp_local ? R_PPL15 : R_PP15);
	        repack(0, data, 1, 64, pp_local ? 28 : 6);
	    }
	    else objval = obj_addr_x >> pp_shift_cnt;

	    /*--------------------------------------------------------------*/
            /* IF THE ADDRESS STILL FALLS WITHIN AN APPROPRIATE RANGE       */
	    /*--------------------------------------------------------------*/
	    if ((objval     >= 0x00000000  && objval     <= 0x00007fff) || 
	        (obj_addr_x >= 0x01000000  && obj_addr_x <= 0x010007ff) )
	        break;

	    /*--------------------------------------------------------------*/
	    /* ERROR, THE OFFSET WILL NOT FIT SCALED OR UNSCALED.           */
	    /*--------------------------------------------------------------*/
	    if (verbose)
	        load_msg( "PP 15-bit offset overflow at %08lx in section %s",
			   rp->r_vaddr, sname);
	    load_err = E_RELOCENT;
	    return FALSE;
         }

      /*--------------------------------------------------------------------*/
      /* PP 16-bit byte offset relocation. For R_PP16B the lower 15-bits    */
      /* are handled just like R_PP15, and the upper bit is placed in the   */
      /* scale indicator bit of the field.                                  */
      /*--------------------------------------------------------------------*/
      case R_PP16B   :
      case R_PPL16B  :
      case R_PPN16B  :
      case R_PPLN16B :
	 {
	    char *sname = (SECT_HDR(s)->s_zeroes == 0) ? 
		          SECT_HDR(s)->s_nptr : SNAMECPY(SECT_HDR(s)->s_name);

	    pp_local = (rp->r_type == R_PPL16B) || (rp->r_type == R_PPLN16B);

            /*--------------------------------------------------------------*/
	    /* READ THE SCALE BIT (16th BIT) AND CREATE 16 BIT CONSTANT OFF */
            /*--------------------------------------------------------------*/
	    objval |= (unpack(data, 1, 64, pp_local ? 28 : 6) << 15);

	    /*--------------------------------------------------------------*/
	    /* IF NEGATIVE RELOC TYPE, THEN TREAT CONST OFFSET AS NEGATIVE  */
	    /*--------------------------------------------------------------*/
	    if (rp->r_type == R_PPN16B || rp->r_type == R_PPLN16B)
	    {
	       objval      = - objval;
               rp->r_type -= 010; /* CHANGE THE TYPE TO A NON NEG TYPE.     */
            }

	    objval += reloc_amt;

	    /*--------------------------------------------------------------*/
            /* IF THE ADDRESS STILL FALLS WITHIN AN APPROPRIATE RANGE       */
	    /*--------------------------------------------------------------*/
	    if ((objval >= 0x00000000 && objval <= 0x0000ffff) || 
		(objval >= 0x01000000 && objval <= 0x010007ff) )
	    {
	       /*-----------------------------------------------------------*/
	       /* RELOCATE THE 16TH BIT OF THE ADDRESS.                     */
	       /*-----------------------------------------------------------*/
	       repack(((objval&0x8000) >> 15), data, 1, 64, pp_local ? 28 : 6);
	       break;
            }

	    /*--------------------------------------------------------------*/
            /* ADDRESS IS OUT OF RANGE.                                     */
	    /*--------------------------------------------------------------*/
	    if (verbose)
	        load_msg( "PP 15-bit offset overflow at %08lx in section %s",
			   rp->r_vaddr, sname);
	    load_err = E_RELOCENT;
	    return FALSE;
         }

      /*--------------------------------------------------------------------*/
      /* PP BASE ADDRESS RELOCATION.  BIT 0 IS 0 IF IN DATA RAM, 1 IF IN    */
      /* PARAMETER RAM.  THIS CODE ASSUMES THAT WE DO NOT RELOCATE FROM     */
      /* PRAM TO DRAM OR FROM DRAM TO PRAM AT LOAD TIME.                    */
      /*--------------------------------------------------------------------*/
      case R_PPLBASE: pp_local = TRUE;
      case R_PPBASE:
         {
	   /*---------------------------------------------------------------*/
	   /* IF WAS DRAM AND RELOC_AMT IS GREAT ENOUGH TO MOVE INTO PRAM,  */
	   /* CHANGE TO PRAM                                                */
	   /*---------------------------------------------------------------*/
	   if (!objval && reloc_amt > (long)(0x01000000 - 0xC000)) objval = 1;

	   /*---------------------------------------------------------------*/
	   /* IF WAS PRAM AND RELOC_AMT IS NEGATIVE AND CAN MOVE INTO DRAM, */
	   /* CHANGE TO DRAM                                                */
	   /*---------------------------------------------------------------*/
	   else if (objval && (-reloc_amt) > (long)(0x01000000 - 0xC000))
	       objval = 0;

	   break;
         }

      /*----------------------------------------------------------------------*/
      /* 34010 ONE'S COMPLEMENT RELOCATION.  SUBTRACT INSTEAD OF ADD.         */
      /*----------------------------------------------------------------------*/
      case R_OCRLONG:
	 objval -= reloc_amt;
	 break;

      /*----------------------------------------------------------------------*/
      /* 34020 WORD-SWAPPED RELOCATION.  SWAP BEFORE RELOCATING.              */
      /*----------------------------------------------------------------------*/
      case R_GSPOPR32:
      case R_OCBD32:
          objval  = ((objval >> 16) & 0xFFFF) | (objval << 16); 
          objval += (rp->r_type == R_GSPOPR32) ? reloc_amt : -reloc_amt; 
          objval  = ((objval >> 16) & 0xFFFF) | (objval << 16);
          break; 

      /*----------------------------------------------------------------------*/
      /* PC-RELATIVE RELOCATIONS.  IN THIS CASE THE RELOCATION AMOUNT         */
      /* IS ADJUSTED BY THE PC DIFFERENCE.   IF THIS IS AN INTERNAL           */
      /* RELOCATION TO THE CURRENT SECTION, NO ADJUSTMENT IS NEEDED.          */
      /*----------------------------------------------------------------------*/
      case R_PCRBYTE:
      case R_PCRWORD:
      case R_GSPPCR16:
      case R_GSPPCA16:
      case R_PCRLONG:
      case R_PCR24:
	 {
            int           shift = 8 * (4 - fieldsz);
	    unsigned long pcdif = RUN_RELOC_AMOUNT(s);

	    /*----------------------------------------------------------------*/
	    /* HANDLE SPECIAL CASES OF JUMPING FROM ABSOLUTE SECTIONS (SPECIAL*/
	    /* RELOC TYPE) OR TO ABSOLUTE DESTINATION (SYMNDX == -1).  IN     */
	    /* EITHER CASE, SET THE APPROPRIATE RELOCATION AMOUNT TO 0.       */
	    /*----------------------------------------------------------------*/
	    if( rp->r_symndx == -1 )      reloc_amt = 0;
	    if( rp->r_type == R_GSPPCA16) pcdif = 0;

	    reloc_amt -= pcdif;

            if (rp->r_type == R_GSPPCR16 || rp->r_type == R_GSPPCA16)
	       reloc_amt >>= 4;                              /* BITS TO WORDS */

	    objval  = (long)(objval << shift) >> shift;      /* SIGN EXTEND   */
	    objval += reloc_amt;
            break;
	 }

      /*----------------------------------------------------------------------*/
      /* 320C30 PAGE-ADDRESSING RELOCATION.  CALCULATE THE ADDRESS FROM       */
      /* THE 8-BIT PAGE VALUE IN THE FIELD, THE 16-BIT OFFSET IN THE RELOC    */
      /* ENTRY, AND THE RELOCATION AMOUNT.  THEN, STORE THE 8-BIT PAGE        */
      /* VALUE OF THE RESULT BACK IN THE FIELD.                               */
      /*----------------------------------------------------------------------*/
      case R_PARTMS8:
	  objval = (long)((objval << 16) + rp->r_disp + reloc_amt) >> 16;
	  break;

      /*----------------------------------------------------------------------*/
      /* DSP(320) PAGE-ADDRESSING.  CALCULATE ADDRESS FROM THE 16-BIT         */
      /* VALUE IN THE RELOCATION FIELD PLUS THE RELOCATION AMOUNT.  OR THE    */
      /* TOP 9 BITS OF THIS RESULT INTO THE RELOCATION FIELD.                 */
      /*----------------------------------------------------------------------*/
      case R_PARTMS9:
	  objval = (objval & 0xFE00) | 
		   (((long)(rp->r_disp + reloc_amt) >> 7) & 0x1FF);
          break;

      /*----------------------------------------------------------------------*/
      /* DSP(320) PAGE-ADDRESSING.  CALCULATE ADDRESS AS ABOVE, AND OR THE    */
      /* 7-BIT DISPLACEMENT INTO THE FIELD.                                   */
      /*----------------------------------------------------------------------*/
      case R_PARTLS7:
	  objval = (objval & 0x80) | ((rp->r_disp + reloc_amt) & 0x7F);
	  break;

      /*--------------------------------------------------------------------*/
      /* RR(370) MSB RELOCATION.  CALCULATE ADDRESS FROM THE 16-BIT VALUE   */
      /* IN THE RELOCATION ENTRY PLUS THE RELOCATION AMOUNT.  PATCH THE     */
      /* MSB OF THE RESULT INTO THE RELOCATION FIELD.                       */
      /*--------------------------------------------------------------------*/
      case R_HIWORD:
	  objval += (rp->r_disp += (unsigned short)reloc_amt) >> 8;
	  break;

      /*--------------------------------------------------------------------*/
      /* C8+ High byte of 24-bit address.  Calculate address from 24-bit    */
      /* value in the relocation entry plus the relocation amount.  Patch   */
      /* the MSB of the result into the relocation field.                   */
      /*--------------------------------------------------------------------*/
      case R_C8PHIBYTE:
          objval = (((objval << 16) + rp->r_disp + reloc_amt) >> 16);
          break;

      /*--------------------------------------------------------------------*/
      /* C8+ Middle byte of 24-bit address.  Calculate address from 24-bit  */
      /* value in the relocation entry plus the relocation amount.  Patch   */
      /* the middle byte of the result into the relocation field.           */
      /*--------------------------------------------------------------------*/
      case R_C8PMIDBYTE:
          objval = (((objval << 16) + rp->r_disp + reloc_amt) >> 8);
          break;

      /*--------------------------------------------------------------------*/
      /* C8+ Vector Address.  Calculate address from 24-bit value in the    */
      /* relocation entry plus the relocation amount.  MSB must be 0xFF     */
      /* since interrupt and trap handlers must be programmed in the top-   */
      /* most segment of memory.  Patch bottom 16-bits of the result into   */
      /* the relocation field.                                              */
      /*--------------------------------------------------------------------*/
      case R_C8PVECADR:
          objval += reloc_amt;
          if ((objval & 0xFF0000) != 0xFF0000)
          {
	     /* ERROR */
          }
          objval &= 0xFFFF;
          break;

      /*----------------------------------------------------------------------*/
      /* C8+ 24-bit Address.  The byte ordering for 24-bit addresses on the   */
      /* C8+ is reversed (low, middle, high).  Needs to be unordered, add     */
      /* in reloc_amt, then re-ordered.                                       */
      /*----------------------------------------------------------------------*/
      case R_C8PADR24:
          objval = ((objval>>16) | (objval&0xff00) | ((objval&0xff)<<16));
          objval += reloc_amt;
          objval = ((objval>>16) | (objval&0xff00) | ((objval&0xff)<<16));
          break;

      /*----------------------------------------------------------------------*/
      /* DSP(320) 13-BIT CONSTANT.  RELOCATE ONLY THE LOWER 13 BITS OF THE    */
      /* FIELD.                                                               */
      /*----------------------------------------------------------------------*/
      case R_REL13:
          objval = (objval & 0xE000) | ((objval + reloc_amt) & 0x1FFF);
          break;

      /*----------------------------------------------------------------------*/
      /* PRISM (370/16) code label relocation.  Convert word address to byte  */
      /* address, add in relocation, convert back to word address.            */
      /*----------------------------------------------------------------------*/
      case R_LABCOD:
	  objval = ((objval << 1) + reloc_amt) >> 1;
	  break;
   }

   /*-------------------------------------------------------------------------*/
   /* PACK THE RELOCATED FIELD BACK INTO THE OBJECT DATA.                     */
   /*-------------------------------------------------------------------------*/
   repack(objval, data, fieldsz, wordsz, offset + BIT_OFFSET(rp->r_vaddr));
   return TRUE;
} 


/******************************************************************************/
/*                                                                            */
/* RELOC_READ() - Read a single relocation entry.                             */
/*                                                                            */
/******************************************************************************/
int reloc_read(rptr)
   RELOC *rptr;
{
#if COFF_VERSION_1 || COFF_VERSION_2
   /*------------------------------------------------------------------------*/
   /* THE FOLLOWING UNION IS USED TO READ IN VERSION 0 OR 1 RELOC ENTRIES    */
   /*------------------------------------------------------------------------*/
   /* THE FORMAT OF RELOCATION ENTRIES CHANGED BETWEEN COFF VERSIONS 0 AND 1.*/
   /* VERSION 0 HAS A 16 BIT SYMBOL INDEX, WHILE VERSION 1 HAS A 32 BIT INDX.*/
   /*------------------------------------------------------------------------*/
   union { RELOC new; RELOC_OLD old; } input_entry;

   if (fread(&input_entry, RELSZ_IN(coff_version), 1, fin) != 1)
      { load_err = E_FILE; return FALSE; }

   /*------------------------------------------------------------------------*/
   /* IF LOADING A VERSION 0 FILE, TRANSLATE ENTRY TO VERSION 1 FORMAT.      */
   /* (THIS COULD BE SIMPLER - ALL THE SWAPS EXCEPT FOR THE SYMBOL INDEX     */
   /* COULD BE DONE AFTER THE TRANSLATION - BUT THIS SEEMS TO BE CLEARER)    */
   /*------------------------------------------------------------------------*/
   if (ISCOFF_0(coff_version))
   {
      if (byte_swapped)
      {
         swap4byte(&input_entry.old.r_vaddr);
	 swap2byte(&input_entry.old.r_symndx);
         swap2byte(&input_entry.old.r_disp);
	 swap2byte(&input_entry.old.r_type);
      }
      rptr->r_vaddr  = input_entry.old.r_vaddr;
      rptr->r_symndx = input_entry.old.r_symndx;
      rptr->r_disp   = input_entry.old.r_disp;
      rptr->r_type   = input_entry.old.r_type;
   }
   else 
   {
      if (byte_swapped)
      {
         swap4byte(&rptr->r_vaddr); swap4byte(&rptr->r_symndx);
         swap2byte(&rptr->r_disp);  swap2byte(&rptr->r_type);
      }
      *rptr = input_entry.new;
   }

#else
   /*-------------------------------------------------------------------------*/
   /* READ IN AND BYTE SWAP AN VERSION 0 RELOC ENTRY                          */
   /*-------------------------------------------------------------------------*/
   if (fread(rptr, RELSZ, 1, fin) != 1) { load_err = E_FILE; return FALSE; }

   if (byte_swapped)
   {
      swap4byte(&rptr->r_vaddr);
      swap2byte(&rptr->r_symndx);
      swap2byte(&rptr->r_disp);
      swap2byte(&rptr->r_type);  
   }
#endif

   return TRUE;
}


/*************************************************************************/
/*                                                                       */
/*   RELOC_SIZE()-                                                       */  
/*      Return the field size of a relocation type.                      */
/*                                                                       */
/*************************************************************************/
    
int reloc_size(type)
   int type;
{
   switch (type)
   {
       case R_PPBASE:
       case R_PPLBASE:	    return 1;

       case R_HIWORD:
       case R_C8PHIBYTE:
       case R_C8PMIDBYTE:
       case R_RELBYTE:
       case R_PCRBYTE:
       case R_RRNREG:
       case R_RRRELREG:
       case R_PARTLS7:      return 8;

       case R_PP15:
       case R_PP15W:
       case R_PP15H:
       case R_PP16B:
       case R_PPN15:
       case R_PPN15W:
       case R_PPN15H:
       case R_PPN16B:
       case R_PPL15:
       case R_PPL15W:
       case R_PPL15H:
       case R_PPL16B:
       case R_PPLN15:
       case R_PPLN15W:
       case R_PPLN15H:
       case R_PPLN16B:      return 15;

       case R_LABCOD:
       case R_RELWORD:
       case R_PCRWORD:
       case R_GSPPCR16:
       case R_GSPPCA16:
       case R_PARTLS16:   
       case R_PARTMS8:
       case R_PARTMS9:
       case R_REL13:        
       case R_C8PVECADR:    return 16;

       case R_REL24:
       case R_PCR24:
       case R_PCR24W:
      case R_C8PADR24:      return 24;

       case R_MPPCR:
       case R_GSPOPR32:
       case R_RELLONG:
       case R_PCRLONG:
       case R_OCBD32:
       case R_OCRLONG:     
       case R_DIR32:        return 32;           

       default:             return 0;
   }
}


/*************************************************************************/
/*                                                                       */
/*   RELOC_OFFSET()-                                                     */  
/*      Return the offset of a relocation type field.  The value of      */
/*      offset should be the bit offset of the LSB of the field in       */
/*      little-endian mode.                                              */
/*                                                                       */
/*************************************************************************/
    
int reloc_offset(type)
   int type;
{
   switch (type)
   {
       case R_PP15    :
       case R_PP15W   : 
       case R_PP15H   :
       case R_PP16B   :
       case R_PPN15   :
       case R_PPN15W  : 
       case R_PPN15H  :
       case R_PPN16B  :
       case R_PPLBASE :     return 22;

       case R_PPL15   :
       case R_PPL15W  :
       case R_PPL15H  :
       case R_PPL16B  :
       case R_PPLN15  :
       case R_PPLN15W :
       case R_PPLN15H :
       case R_PPLN16B :
       case R_PPBASE  :     return 0;

       default        :     return 0;
   }
}


/*************************************************************************/
/*                                                                       */
/*   RELOC_STOP() -                                                      */  
/*      Return the number of bits to read for a relocation type.         */
/*                                                                       */
/*************************************************************************/
    
int reloc_stop(type)
   int type;
{
   switch (type)
   {
       case R_PPBASE  : 
       case R_PPLBASE :

       case R_PP15    :
       case R_PP15W   :
       case R_PP15H   :
       case R_PP16B   :

       case R_PPL15   :
       case R_PPL15W  :
       case R_PPL15H  :
       case R_PPL16B  :

       case R_PPN15   :
       case R_PPN15W  :
       case R_PPN15H  :
       case R_PPN16B  :

       case R_PPLN15  :
       case R_PPLN15W :
       case R_PPLN15H :
       case R_PPLN16B :   return 64;

       default       :    return WORDSZ * 8;
   }
}


/******************************************************************************/
/*                                                                            */
/* SYM_RELOC_AMOUNT() - Determine the amount of relocation for a particular   */
/*                      relocation entry.  Search the relocation symbol table */
/*                      for the referenced symbol, and return the amount from */
/*                      the table.                                            */
/*                                                                            */
/******************************************************************************/
long sym_reloc_amount(rp)
   RELOC *rp;
{
   long index = rp->r_symndx;
   
   int i = 0,
       j = reloc_sym_index - 1;

   /*-------------------------------------------------------------------------*/
   /* THIS IS A SIMPLE BINARY SEARCH (THE RELOC TABLE IS ALWAYS SORTED).      */
   /*-------------------------------------------------------------------------*/
   while (i <= j)
   {
      int m = (i + j) / 2;
      if      (reloc_tab[m].rt_index < index) i = m + 1;
      else if (reloc_tab[m].rt_index > index) j = m - 1;
      else return reloc_tab[m].rt_disp;                              /* FOUND */
   }

   /*-------------------------------------------------------------------------*/
   /* IF NOT FOUND, SYMBOL WAS NOT RELOCATED.                                 */
   /*-------------------------------------------------------------------------*/
   return 0;
}


/******************************************************************************/
/*                                                                            */
/*  UNPACK() - Extract a relocation field from object bytes and convert into  */
/*             a long so it can be relocated.                                 */
/*                                                                            */
/******************************************************************************/
unsigned long unpack(data, fieldsz, wordsz, bit_offset)
   unsigned char *data;
   int fieldsz;
   int wordsz;
   int bit_offset;
{
   register int  i;
   unsigned long objval;
   int           start;                       /* MS byte with reloc data      */
   int           stop;                        /* LS byte with reloc data      */
   int           r  = bit_offset & 7;         /* num unused LS bits in stop   */
   int           l  = 8 - r;                  /* num used   MS bits in stop   */
   int           tr = ((bit_offset+fieldsz-1) & 7)+1; /* # LS bits in strt*/
   int           tl = 8 - tr;	              /* num unused MS bits in start  */

   start = (big_e_target ? (wordsz-fieldsz-bit_offset) : 
			   (bit_offset+fieldsz-1)) >>3;
   stop  = (big_e_target ? (wordsz-bit_offset-1) : bit_offset) >>3;

   if (start == stop) return (data[stop] >> r) & ((0x1 << fieldsz) - 0x1);

   objval = (unsigned)((data[start] << tl) & 0xFF) >> tl;
   
   if (big_e_target) 
	 for (i=start+1; i<stop; ++i) objval = (objval << 8) | data[i];
   else  for (i=start-1; i>stop; --i) objval = (objval << 8) | data[i];
   
   return (objval << l) | (data[stop] >> r);
}



/******************************************************************************/
/*                                                                            */
/* REPACK() - Encode a binary relocated field back into the object data.      */
/*                                                                            */
/******************************************************************************/
void repack(objval, data, fieldsz, wordsz, bit_offset)
   unsigned long objval;
   unsigned char *data;
   int fieldsz;
   int wordsz;
   int bit_offset;
{
   register int  i;
   int           start;                      /* MS byte with reloc data      */
   int           stop;                       /* LS byte with reloc data      */
   int           r     = bit_offset & 7;     /* num unused LS bits in stop   */
   int           l     = 8 - r;              /* num used   MS bits in stop   */
   int           tr    = ((bit_offset+fieldsz-1) & 7)+1; /* # LS bits in strt*/
   int           tl    = 8 - tr;	     /* num unused MS bits in start  */
   unsigned long mask  = (1ul << fieldsz) - 1ul;

   if (fieldsz < sizeof(objval)) objval &= mask;
   
   start = (big_e_target ? (wordsz-fieldsz-bit_offset) : 
			   (bit_offset+fieldsz-1)) >>3;
   stop  = (big_e_target ? (wordsz-bit_offset-1) : bit_offset) >>3;

   if (start == stop)
   {
       data[stop] &= ~(mask << r);
       data[stop] |= (objval << r); 
       return;
   }

   data[start] &= ~((1<<tr)-1);
   data[stop]  &=  ((1<< r)-1);
   data[stop]  |= (objval << r); 
   objval     >>= l;

   if (big_e_target) 
        for (i = stop - 1; i > start; objval >>= 8) data[i--] = objval;
   else for (i = stop + 1; i < start; objval >>= 8) data[i++] = objval;
   
   data[start] |= objval; 
}



/******************************************************************************/
/*                                                                            */
/* CLOAD_LINENO() - Read in, swap, and relocate line number entries.  This    */
/*                  function is not called directly by the loader, but by the */
/*                  application when it needs to read in line number entries. */
/*                                                                            */
/******************************************************************************/
int cload_lineno(filptr, count, lptr, scnum) 
   long    filptr;                     /* WHERE TO READ FROM                  */
   int     count;                      /* HOW MANY TO READ                    */
   LINENO *lptr;                       /* WHERE TO PUT THEM                   */
   int     scnum;                      /* SECTION NUMBER OF THESE ENTRIES     */
{
    int i;

    /*------------------------------------------------------------------------*/
    /* READ IN THE REQUESTED NUMBER OF LINE NUMBER ENTRIES AS A BLOCK.        */
    /*------------------------------------------------------------------------*/
    if (fseek(fin, filptr, 0) != 0) { load_err = E_FILE; return FALSE; }
    for (i = 0; i < count; i++)
        if (fread(lptr + i, 1, LINESZ, fin) != LINESZ) 
                                    { load_err = E_FILE; return FALSE; }

    /*------------------------------------------------------------------------*/
    /* SWAP AND RELOCATE EACH ENTRY, AS NECESSARY.                            */
    /*------------------------------------------------------------------------*/
    if (byte_swapped || RUN_RELOC_AMOUNT(scnum - 1))
       for (i = 0; i < count; i++, lptr++)
       {
	  if (byte_swapped)
	  {
	      swap2byte(&lptr->l_lnno);
	      swap4byte(&lptr->l_addr.l_paddr);
	  }

	  if (lptr->l_lnno != 0) 
	     lptr->l_addr.l_paddr += RUN_RELOC_AMOUNT(scnum - 1);
       }
    
    return TRUE;
}


/******************************************************************************/
/*                                                                            */
/*  SWAP4BYTE() - Swap the order of bytes in a long.                          */
/*                                                                            */
/******************************************************************************/
void swap4byte(addr)
   void *addr;
{
   unsigned long *value = addr;
   unsigned long temp1, temp2, temp3, temp4;

   temp1 = (*value)       & 0xFF;
   temp2 = (*value >> 8)  & 0xFF;
   temp3 = (*value >> 16) & 0xFF;
   temp4 = (*value >> 24) & 0xFF;

   *value = (temp1 << 24) | (temp2 << 16) | (temp3 << 8) | temp4;
}


/******************************************************************************/
/*                                                                            */
/*  SWAP2BYTE() - Swap the order of bytes in a short.                         */
/*                                                                            */
/******************************************************************************/
void swap2byte(addr)
   void *addr;
{
   unsigned short *value = addr;
   unsigned short temp1,temp2;

   temp1 = temp2 = *value;
   *value = ((temp2 & 0xFF) << 8) | ((temp1 >> 8) & 0xFF);
}
