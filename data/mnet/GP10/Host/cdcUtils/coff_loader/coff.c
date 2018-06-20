/****************************************************************************/
/* MAIN.C  - Skeleton Driver for COFF Loader       Version 6.00 4/96        */
/****************************************************************************/
/*                                                                          */
/* General:  This module is a skeleton driver for a standalone COFF         */
/*           loader.  This file is NOT part of the generic loader, but      */
/*           provides a sample interface to it that can provide a basis     */
/*           for a specific loader.  YOU MUST CUSTOMIZE THIS PROGRAM        */
/*           FOR YOUR APPLICATION.  In particular, you must write the       */
/*           function 'mem_write()' which is the low-level function to      */
/*           load target memory.                                            */
/*                                                                          */
/*           Refer to the documentation provided with the loader for        */
/*           details on how to interface with it.                           */
/*                                                                          */
/* Usage:    cload <-options> filename                                      */
/* Options:     -b        clear .bss section                                */
/*              -q        quiet mode (no banner)                            */
/*              -r xxxx   relocate by xxxx                                  */
/*              -v        verbose (debug)                                   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* Functions defined in this module:                                        */
/*                                                                          */
/*    main()             - Main driver for loader.                          */
/*    mem_write()        - Load a buffer of raw data to the target.         */
/*    set_reloc_amount() - Determine relocation amount for each section.    */
/*    lookup_sym()       - Stub for symbol lookup routine.                  */
/*    myalloc()          - Application version of 'malloc()'.               */
/*    mralloc()          - Application version of 'realloc()'.              */
/*    load_msg()         - Called by loader to write debug messages.        */
/*                                                                          */
/****************************************************************************/
#include "header.h"
#include <string.h>
#include "l1proxy/l1proxy.h"

#if MSDOS
#define READ_BIN "rb"
#else
#define READ_BIN "r"
#endif

void myfreeall(void);
   
/*--------------------------------------------------------------------------*/
/* LOADER DEFINITIONS                                                       */
/*--------------------------------------------------------------------------*/
FILE *fopen();
char *strrchr(), *malloc(), *realloc();

unsigned long reloc = 0;                 /* RELOCATION AMOUNT               */
int           quiet = 0;                 /* NO BANNER                       */
int           memDebug = 0;              /* DEBUG MEMORY                    */

#ifdef unix

/****************************************************************************/
/*                                                                          */
/* MAIN() - Main driver for loader.  Read in options and filename, open     */
/*          COFF file, and call 'cload()'.                                  */
/*                                                                          */
/****************************************************************************/
main(argc, argv)
   int argc;
   char *argv[];
{
   char fname[256];
   char *ctmp;
   int  i;
   int  files = 0;

   need_symbols = 0;                  /* TELL LOADER NOT TO READ SYM TABLE  */

   /*-----------------------------------------------------------------------*/
   /* PROCESS COMMAND LINE ARGUMENTS                                        */
   /*-----------------------------------------------------------------------*/
   for (i = 1; i < argc; ++i)
   {
      char *argp = argv[i];

      if (*argp == '-')                      /* OPTIONS                     */
         while (*++argp) switch(*argp)
         {
            case 'B': case 'b':  clear_bss = 1;                     break;
            case 'Q': case 'q':  quiet     = 1;                     break;
            case 'V': case 'v':  ++verbose;                         break;
            case 'R': case 'r':  sscanf(argv[++i],"%lx",&reloc);    break;
            default :            continue;
         }
      else                                   /* FILENAMES                   */
      {
         strcpy(fname, argp);
         files = 1;
      }
   }

   /*-----------------------------------------------------------------------*/
   /* SAY HELLO TO USER AND PROMPT FOR FILENAME IF NOT GIVEN.               */
   /*-----------------------------------------------------------------------*/
   if (!quiet) printf("COFF Loader   v6.00\n");

   if (!files)
   {
      char *p;
      printf("   COFF file [.out]: ");
      fgets(fname, 64, stdin);
      if (p = strrchr(fname, '\n')) *p = '\0';
   }

   /*-----------------------------------------------------------------------*/
   /* OPEN THE COFF FILE.  IF THE HOST DISTIGUISHES FILE TYPES, BE SURE     */
   /* TO OPEN IT AS A BINARY FILE.                                          */
   /*-----------------------------------------------------------------------*/
   ctmp = strrchr(fname,'.');
   if ((!ctmp) || (ctmp < strrchr(fname,'\\'))) strcat(fname,".out");

   if (!(fin = fopen(fname, READ_BIN)))
     { printf("\n>> can't open file <%s>\n", fname);  exit(-1); }

   /*-----------------------------------------------------------------------*/
   /* LOAD THE FILE.  CLOSE IT WHEN FINISHED.                               */
   /*-----------------------------------------------------------------------*/
   if (!cload()) { printf("\n>> error loading file\n"); exit(-1); }
   fclose(fin);
}
#endif


/****************************************************************************/
/*                                                                          */
/* ldCoff() - Main driver for loader.                                       */
/*                                                                          */
/****************************************************************************/
int ldCoff(char *file, char *base_prefix)
{
   char fname[256];
   char *ctmp;
   int  i;
   int  files = 0;
   int  result = ERROR;

   need_symbols = 0;                  /* TELL LOADER NOT TO READ SYM TABLE  */
   clear_bss = 1;
   quiet = 1;
   verbose = 0;
   memDebug = 0;
   strcpy(fname, base_prefix);
   strcat(fname, "/");
   strcat(fname, file);
   files = 1;

   /*-----------------------------------------------------------------------*/
   /* SAY HELLO TO USER AND PROMPT FOR FILENAME IF NOT GIVEN.               */
   /*-----------------------------------------------------------------------*/
   if (!quiet) printf("COFF Loader   v6.00\n");

   if (!files)
   {
      char *p;
      printf("   COFF file [.out]: ");
      fgets(fname, 64, stdin);
      if (p = strrchr(fname, '\n')) *p = '\0';
   }

   /*-----------------------------------------------------------------------*/
   /* OPEN THE COFF FILE.  IF THE HOST DISTIGUISHES FILE TYPES, BE SURE     */
   /* TO OPEN IT AS A BINARY FILE.                                          */
   /*-----------------------------------------------------------------------*/
   ctmp = strrchr(fname,'.');
   if ((!ctmp) || (ctmp < strrchr(fname,'\\'))) strcat(fname,".out");

   if (!(fin = fopen(fname, READ_BIN)))
   { 
       printf("\n>> can't open file <%s>\n", fname);  
       return(ERROR); 
   }

   /*-----------------------------------------------------------------------*/
   /* LOAD THE FILE.  CLOSE IT WHEN FINISHED.                               */
   /*-----------------------------------------------------------------------*/
   if (!cload()) 
   { 
       printf("\n>> error loading file\n"); 
       myfreeall(); 
       return(ERROR); 
   }
   fclose(fin);
   myfreeall();
   return(OK);
}


/****************************************************************************/
/*                                                                          */
/* MEM_WRITE() - Load a buffer of raw data to the target.                   */
/*                                                                          */
/*   THIS FUNCTION MUST BE CUSTOMIZED FOR YOUR APPLICATION !                */
/*                                                                          */
/*   (As supplied, this function simply dumps the data out to the screen.)  */
/*                                                                          */
/****************************************************************************/
int mem_write(buffer, nbytes, addr, page)
   unsigned char *buffer;                   /* POINTER TO DATA BUFFER       */
   unsigned int   nbytes;                   /* NUMBER OF 8-BIT BYTES        */
   T_ADDR         addr;                     /* TARGET DESTINATION ADDRESS   */
   unsigned char  page;                     /* TARGET DESTINATION PAGE      */
{
   if (verbose)
   {
      int i;
      printf("\n[%0*lx]:%d:", 2 * sizeof(T_ADDR), addr, nbytes);
      for (i = 0; i < nbytes; ++i) printf("%02x ", buffer[i]);
      printf("\n");
   }

#ifndef unix
   {
       unsigned char *p = buffer;
       int verifyCount;
       int isVerified = FALSE;
       unsigned int verifyWord, n;
       
       for (n = 0; n < nbytes; n += 4)
       {
           swap4byte(&p[n]);
       }
       
       /* Write the data to both dsp 0 and 1 */
       hpi2dsp(1, buffer, addr, nbytes);
       hpi2dsp(0, buffer, addr, nbytes);
       
       /* Now verify that the data was written correctly.  Each word will */
       /* be verified one by one to assure that hpi auto increment did    */
       /* not introduce any errors.                                       */
       for (n = 0; n < nbytes; n += sizeof(unsigned int))
       {
          /* First try DSP 0 */
          isVerified = FALSE;
          verifyCount = 3;
          do 
          {
             dsp2hpi(0, addr+n, (unsigned char *)&verifyWord, sizeof(unsigned int));
             
             if (*(unsigned int *)(buffer + n) != verifyWord)
             {
                hpi2dsp(0, buffer+n, addr+n, sizeof(unsigned int));
                verifyCount--; 
                printf ("WARNING: Retrying writing DSP 0 addr %#x wrote(%#x)"
                   " read back(%#x)\n", addr+n, *(unsigned int *)(buffer + n),
                   verifyWord);     
             } 
             else
             {
                isVerified = TRUE;
             }
             
          } while ((isVerified == FALSE) && verifyCount);
            
          if (!verifyCount)
          {
             printf ("ERROR: After 3 retries unable to write to DSP 0 addr %#x\n", 
                addr+n);
          }
             
          /* Now try DSP 1 */
          isVerified = FALSE;
          verifyCount = 3;
          do 
          {
             dsp2hpi(1, addr+n, (unsigned char *)&verifyWord, sizeof(unsigned int));
             
             if (*(unsigned int *)(buffer + n) != verifyWord)
             {
                hpi2dsp(1, buffer+n, addr+n, sizeof(unsigned int));
                verifyCount--; 
                printf ("WARNING: Retrying writing DSP 1 addr %#x wrote(%#x)"
                   " read back(%#x)\n", addr+n, *(unsigned int *)(buffer + n),
                   verifyWord);     
             } 
             else
             {
                isVerified = TRUE;
             }
             
          } while ((isVerified == FALSE) && verifyCount);
          
          if (!verifyCount)
          {
             printf ("ERROR: After 3 retries unable to write to DSP 1 addr %x\n",
                 addr+n);
          }
       }
   }
#endif

   /*-----------------------------------------------------------------------*/
   /* INSERT CUSTOM CODE HERE TO LOAD TARGET MEMORY.                        */
   /*-----------------------------------------------------------------------*/

   return 1;
}


/****************************************************************************/
/*                                                                          */
/* SET_RELOC_AMOUNT() - Determine relocation amount for each section.       */
/*                                                                          */
/****************************************************************************/
int set_reloc_amount()
{
  int i;

  for (i = 0; i<  n_sections; ++i) reloc_amount[i] = reloc;
  return 1;
}


/****************************************************************************/
/*                                                                          */
/* LOOKUP_SYM() - Stub for symbol lookup routine.                           */
/*                                                                          */
/****************************************************************************/
void lookup_sym(indx, sym, aux)
   long indx;
   SYMENT *sym;
   AUXENT *aux;
{}


/****************************************************************************/
/*                                                                          */
/* LOAD_SYMS() - Stub for symbol load routine.  This routine is only called */
/*               if the global flag 'need_symbols' is TRUE.                 */
/*                                                                          */
/****************************************************************************/
int load_syms(need_reloc)
   int need_reloc;
{
   return 1;
}



/****************************************************************************/
/*                                                                          */
/* MYALLOC() - Application version of 'malloc()'.                           */
/* MRALLOC() - Application version of 'realloc()'.                          */
/*                                                                          */
/****************************************************************************/
void *myalloc(size)
   unsigned int size;
{
   char *p = malloc(size);
   if (memDebug) printf("malloc-->%08x %08x\n", p, size);
   if (p) return p;

   printf("\n>> out of memory\n");
   exit(-1);
}

void *mralloc(p, size)
   void *p;
   unsigned int size;
{
   if (memDebug) printf("realloc-->%08x %08x", p, size);
   p = realloc(p, size);
   if (memDebug) printf(" %08x\n", p);
   if (p) return p;

   printf("\n>> out of memory\n");
   exit(-1);
}
void myfree(void *p)
{
   if (memDebug) printf("free-->%08x\n", p);
   free(p);
}

void *reloc_tab;
void myfreeall(void)
{
   if (sect_hdrs) 
   {
       myfree(sect_hdrs);
       sect_hdrs = 0;
   }

   if (o_sect_hdrs) 
   {
       myfree(o_sect_hdrs);
       o_sect_hdrs = 0;
   }

   if (reloc_amount) 
   {
       myfree(reloc_amount);
       reloc_amount = 0;
   }

   if (reloc_tab)
   {
       myfree(reloc_tab);
       reloc_tab = 0;
   }

   if (str_head) 
   {
       str_free(str_head);
       str_head = 0;
   }
}


/****************************************************************************/
/*                                                                          */
/* LOAD_MSG() - Called by loader to write debug messages.                   */
/*                                                                          */
/****************************************************************************/
void load_msg(a,b,c,d,e,f,g)
char *a;
long  b,c,d,e,f,g;
{
   printf(a,b,c,d,e,f,g);
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: LoadDSPCode
**
**    PURPOSE: Send the DSP message to the preregistered message que. 
**
**    INPUT PARAMETERS: fileName - path & name of the file to load into DSP's
**
**    RETURN VALUE(S):  OK = File Loaded,  ERROR = No File Loaded.
**
**----------------------------------------------------------------------------*/

STATUS  LoadDSPCode(char *fileName, char *base_prefix)
{
    STATUS   result = ERROR;

    if (fileName != NULL)
    {         
        dspReset();
        SetUpForDSPTestBootupMsg();

        result = ldCoff(fileName, base_prefix);  
        if ( result == OK )
        {
            printf("Waiting for DSP Bootup message...\n");
            WaitForDSPTestBootupMsg();
            printf("Received DSP Bootup message\n");
        }
        else
            printf("Bootup ERROR: ldCoff FAILED \n");
    }

    return(result);
}