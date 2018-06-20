// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : pontest.cpp
// Author(s)   : Tim Olson
// Create Date : 7/26/99
// Description :  
//
// *******************************************************************
#include <stdio.h>
#include <loadLib.h>
#include <unldLib.h>
#include "mch/post.h"
#include <string.h>
#include <vxLib.h>
#include "drv/sio/ppc860Sio.h"
#include "taskLib.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "cdcUtils/CPU_Mon_IF.h"
#include "l1proxy/l1proxyhpi.h"
#include "cdc_bsp/nvRam.h"
#ifdef __cplusplus
}
#endif



#define NUM_DSPS                       2
#define DSP_INT_DATA_RAM_START         0x80000000
#define DSP_INT_DATA_RAM_END           0x80010000
#define DSP_INT_PROG_RAM_START         0x01400000
#define DSP_INT_PROG_RAM_END           0x01410000
#define DSP_EXT_RAM_START              0x00000000
#define DSP_EXT_RAM_END                0x00100000

#define ponStarting                    0
#define ponPASSED                      1
#define ponIntDataRam                  2
#define ponIntProgRam                  3
#define ponExtRam                      4
#define ponFAILED                      5

unsigned int DspInternalDataMem[] = {
   DSP_0_INTERNAL_DATA_RAM_TEST,
   DSP_1_INTERNAL_DATA_RAM_TEST};
unsigned int DspInternalProgMem[] = {
   DSP_0_INTERNAL_PROG_RAM_TEST,
   DSP_1_INTERNAL_PROG_RAM_TEST};
unsigned int DspExternalMem[] = {
   DSP_0_EXTERNAL_RAM_TEST,
   DSP_1_EXTERNAL_RAM_TEST};

MODULE_ID potModule;

int     ponAccessTest(char *adrs);
void    ponTestNotify(int Type, int DSPNum);
void    ponFPGATest(void);
void    ponI2CTest(void);

// The following functions may be called to test whether a given power on
// test passed or failed.
bool postAllTestsOkay()
{
   return ((bool)(!((postGetTestResult(ALL_POWER_ON_TESTS) & ALL_POWER_ON_TESTS) == ALL_POWER_ON_TESTS)));
}

bool postMpcSramOkay()
{
   return ((bool)((postGetTestResult(MPC860_SDRAM_TEST)) ? 0 : 1));
}

bool postDsp0IntDataRamOkay()
{
   return ((bool)((postGetTestResult(DSP_0_INTERNAL_DATA_RAM_TEST)) ? 0 : 1));
}

bool postDsp0IntProgRamOkay()
{
   return ((bool)((postGetTestResult(DSP_0_INTERNAL_PROG_RAM_TEST)) ? 0 : 1));
}

bool postDsp0ExtRamOkay()
{
   return ((bool)((postGetTestResult(DSP_0_EXTERNAL_RAM_TEST)) ? 0 : 1));
}

bool postDsp1IntDataRamOkay()
{
   return ((bool)((postGetTestResult(DSP_1_INTERNAL_DATA_RAM_TEST)) ? 0 : 1));
}

bool postDsp1IntProgRamOkay()
{
   return ((bool)((postGetTestResult(DSP_1_INTERNAL_PROG_RAM_TEST)) ? 0 : 1));
}

bool postDsp1ExtRamOkay()
{
   return ((bool)((postGetTestResult(DSP_1_EXTERNAL_RAM_TEST)) ? 0 : 1));
}

bool postFPGA0Okay()
{
   return ((bool)((postGetTestResult(FPGA_0_TEST)) ? 0 : 1));
}

bool postFPGA1Okay()
{
   return ((bool)((postGetTestResult(FPGA_1_TEST)) ? 0 : 1));
}

bool postI2COkay()
{
   return ((bool)((postGetTestResult(I2C_TEST)) ? 0 : 1));
}

int testDspMemBlock(int port, unsigned int startAddr, unsigned int endAddr)
{
   unsigned int read, write;
   int errs = 0;
   for (unsigned int dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {
      write = 0x80000000;
      for(; write; write>>=1)
      {
         // Write walking ones test pattern.
         HPIA(port) = dspAddr;
         *(unsigned int *)HPID(port) = write;
         
         // Read back from memory location.
         HPIA(port) = dspAddr;
         read = *(unsigned int *)HPID(port);
         
         if (read != write)
            errs++;
            
         // Write walking zeros test pattern.
         HPIA(port) = dspAddr;
         *(unsigned int *)HPID(port) = ~write;
         
         // Read back from memory location.
         HPIA(port) = dspAddr;
         read = *(unsigned int *)HPID(port);
         
         if (read != ~write)
            errs++;
      }
   }
   
   // Now do an address test.
   for (dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {
      HPIA(port) = dspAddr;
      *(unsigned int *)HPID(port) = dspAddr;
   }
   
   for (dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {
      HPIA(port) = dspAddr;
      read = *(unsigned int *)HPID(port);
      if (read != dspAddr)
         errs++;
   }
   
   // Now do the complement address test.
   for (dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {
      HPIA(port) = dspAddr;
      *(unsigned int *)HPID(port) = ~dspAddr;
   }
   
   for (dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {
      HPIA(port) = dspAddr;
      read = *(unsigned int *)HPID(port);
      if (read != ~dspAddr)
         errs++;
   }
   
   
   return errs;
}


void TestDspMem(void)
{
   int   AccessResult;

   // Next, test the DSP memory for all dsp's.
   for (int port = 0; port < NUM_DSPS; port++)
   {
      ponTestNotify(ponStarting, port);

      if (AccessResult = ponAccessTest((char *)HPIC(port)) == OK )
      {
         if (AccessResult = ponAccessTest((char *)HPIA(port)) == OK )
         {
            AccessResult = ponAccessTest((char *)HPID(port));
         }
      }

      if ( AccessResult == OK )
      {
         printf("HPI Access Passed for DSP #%d\n", port);

         // Initialize EMIF registers to allow access to extenal RAM.
         HPIC(port) = 0x00000000;
         HPIA(port) = 0x1800000;
         *(unsigned int *)HPID(port) = 0x00003060;
         HPIA(port) = 0x1800008;
         *(unsigned int *)HPID(port) = 0x00000040;
         HPIA(port) = 0x1800004;
         *(unsigned int *)HPID(port) = 0x20E30322;
         HPIA(port) = 0x1800010;
         *(unsigned int *)HPID(port) = 0x00000040;
         HPIA(port) = 0x1800014;
         *(unsigned int *)HPID(port) = 0x00000040;
         HPIA(port) = 0x1800018;
         *(unsigned int *)HPID(port) = 0x07116000;
         HPIA(port) = 0x180001c;
         *(unsigned int *)HPID(port) = 0x00000410;
         
         // Test internal data RAM.  If errors occur then set a bit in the
         // power on test status register.

         ponTestNotify(ponIntDataRam, port);
         if (testDspMemBlock(port, DSP_INT_DATA_RAM_START, DSP_INT_DATA_RAM_END))
         {
            postSetTestResult(DspInternalDataMem[port], POST_FAIL);
            ponTestNotify(ponFAILED, port);
         }
         else
         {
            postSetTestResult(DspInternalDataMem[port], POST_PASS);
            ponTestNotify(ponPASSED, port);
         }
         
         // Test internal program RAM.  If errors occur then set a bit in the
         // power on test status register.

         ponTestNotify(ponIntProgRam, port);
         if (testDspMemBlock(port, DSP_INT_PROG_RAM_START, DSP_INT_PROG_RAM_END))
         {
            postSetTestResult(DspInternalProgMem[port], POST_FAIL);
            ponTestNotify(ponFAILED, port);
         }
         else
         {
            postSetTestResult(DspInternalProgMem[port], POST_PASS);
            ponTestNotify(ponPASSED, port);
         }
      
         // Test extenal RAM.  If errors occur then set a bit in the
         // power on test status register.

         ponTestNotify(ponExtRam, port);
         if (testDspMemBlock(port, DSP_EXT_RAM_START, DSP_EXT_RAM_END))
         {
            postSetTestResult(DspExternalMem[port], POST_FAIL);
            ponTestNotify(ponFAILED, port);
         }   
         else
         {
            postSetTestResult(DspExternalMem[port], POST_PASS);
            ponTestNotify(ponPASSED, port);
         }
      }
      else /* Access Test Failed, Write to serial port & set memory storage */
      {
         printf("HPI Access Failed for DSP #%d\n", port);
         postSetTestResult(DspInternalDataMem[port], POST_FAIL);
         postSetTestResult(DspInternalProgMem[port], POST_FAIL);
         postSetTestResult(DspExternalMem[port], POST_FAIL);
      }
   }      
}



void ExecutePost(void)
{
   TestDspMem();

   /* Run the FPGA Test  */
   ponFPGATest();         


   /* Run the I2C Test  */
   ponI2CTest();   
}

/*******************************************************************************
*
* ponAccessTest
*
* It will probe the address for a bus error. 
*
* NOMANUAL
*********************************************************************************/

int   ponAccessTest(char *adrs)
{
   char testW = 1;
   char testR;
   int   AccessResult = OK;
   
   if (vxMemProbe (adrs, VX_WRITE, 1, &testW) == OK)
   {
       if (vxMemProbe (adrs, VX_READ, 1, &testR) != OK)
          AccessResult = ERROR;
       else
       { 
          if ( testW != testR )
             AccessResult = ERROR;
       }
   }
   else
       AccessResult = ERROR;

   return AccessResult;
}

/*******************************************************************************
*
* ponTestNotify
*
* It will send an ASCII message out Standard I/O. All messages are
* intended for DSP memory testing information.
*
* NOMANUAL
*********************************************************************************/

void ponTestNotify(int Type, int DSPNum)
{
   switch( Type )
   {
       case ponStarting:
         printf("\nStarting Power On Memory Test - DSP #%d\n", DSPNum);
         break;

       case ponPASSED:
         printf(" - Passed\n");
         break;

       case ponFAILED:
         printf(" - Failed\n");
         break;

      case ponIntDataRam:
         printf("\nTesting Internal Data RAM");
         break;

      case ponIntProgRam:
         printf("\nTesting Internal Program RAM");
         break;

      case ponExtRam:
         printf("\nTesting External Data RAM");
         break;

      default:
         printf("\nponTestNotify Selection Error");
         break;
   }
}

/*******************************************************************************
*
* ponFPGATest
*
* Writes and reads to FPGA locations for production testing.
*
* NOMANUAL
*********************************************************************************/

void ponFPGATest(void)
{
	unsigned int	value;
	unsigned int    mask	  = 0x0000FFFF;
	unsigned int    addr	  = 0x1030000;
	unsigned int	testValue = 0x00000080;
	unsigned int	fpga_num;
	unsigned int	index;
    int             result    = OK;

	for	( fpga_num=0; fpga_num<2; fpga_num++)
	{
		printf("Starting FPGA #%d Test...\n", fpga_num);
		for ( index=0; index<0x10000; index++ )
		{			
			/* Write each location with an incrementing value up
			   to 16 bit unsigned limit. Make sure each register 
			   is offset from each other to test internals.	*/

			hpi2dspD(fpga_num,0x1000000,((index)   % 0x10000));
			hpi2dspD(fpga_num,0x1030000,((index+1) % 0x100));

			// Test location 0x1000000
			// Read back corresponding values, only take lower 16 bits back
			value=dsp2hpiD(fpga_num,0x1000000) & 0x0000ffff;
			if (value!=((index) % 0x10000)) 
			{
				printf("FPGA #%d Test - FAILED at Location 0x1000000\n", fpga_num);
				result = ERROR;
                break;
			}			

			// Test location 0x1030000
			// Read back corresponding values
			value=dsp2hpiD(fpga_num,0x1030000) & 0x000000ff;
			if (value!=((index+1) % 0x100)) 
			{
				printf("FPGA #%d Test - FAILED at Location 0x1030000\n", fpga_num);
				result = ERROR;
                break;
			}		
		}

        if ( result == OK )  
        {
            if ( fpga_num == 0 )
                postSetTestResult(FPGA_0_TEST, POST_PASS);
            else if ( fpga_num == 1 )
                postSetTestResult(FPGA_1_TEST, POST_PASS);

            printf("FPGA #%d Test - PASSED\n", fpga_num);
        }
        else if ( result == ERROR )
        {
            if ( fpga_num == 0 )
                postSetTestResult(FPGA_0_TEST, POST_FAIL);
            else if ( fpga_num == 1 )
                postSetTestResult(FPGA_0_TEST, POST_FAIL);
        }
		    
	}
}

/*****************************************************************************
 *
 *  Module Name: ponI2CTest
 *  
 *  Purpose:     Initializes the DS1780 and sets it to its operating mode.
 *
 *****************************************************************************/

void   ponI2CTest(void)
{
   STATUS   result;

   printf("Starting I2C Test...");

   if (( result = DS1780_Init() ) == OK )
   {
       postSetTestResult(I2C_TEST, POST_PASS);
       printf("PASSED\n");
   }
   else
   {
       postSetTestResult(I2C_TEST, POST_FAIL);
       printf("FAILED\n");
   }
}