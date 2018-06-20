// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

#include <stdio.h>
#include <sysLib.h>
#include "mch/post.h"



void MPC860MemTest(void)
{
   // First validate the MPC860 SDRAM by writing a walking one pattern and a
   // walking zero pattern.  Each memory location is saved prior to testing a
   // location and restored after testing.
   unsigned int save, read, write;
   unsigned int errs = 0;
   
   // Loop through all MPC860 SDRAM.
   for (unsigned int *addr = 0; addr < (unsigned int *)sysMemTop(); addr++)
   {
      // Do not test the memory used to execute this code.
      if (((char *)*addr < pText) && ((char *)*addr > (pText+POT_MEM_SIZE)) && ((char **)*addr != &pText) &&
         ((char **)*addr != &pData) && ((char **)*addr != &pBSS))
      {
         save = *addr;
         write = 0x80000000;
         for(; write; write>>=1)
         {
            *addr = write;
            read = *addr;
            if (read != write)
               errs++;
               
            *addr = ~write;
            read = *addr;
            if (read != ~write)
               errs++;
         }
         *addr = save;
      }
   }
   
   // Update the power on test status that will be checked later by root.
   if (errs) PowerOnTestStatus |= MPC860_SDRAM_TEST;
   
}