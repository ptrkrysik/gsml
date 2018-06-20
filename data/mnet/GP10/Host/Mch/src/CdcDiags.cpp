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
// File        : CdcDiags.cpp
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description :  
//
// *******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <taskLib.h>
#include <ioLib.h>
#include <stdLib.h>
#include "CdcDiags.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "l1proxy/l1proxyhpi.h"
#include "cdcUtils/CPU_Mon_IF.h"
#include "cdc_bsp/nvRam.h"
#include "cdcUtils/auxPort.h"
#ifdef __cplusplus
}
#endif


// *******************************************************************
// HPIMemoryTest
// *******************************************************************

    
#define DSPBUFFERMAX    0x204           /* max HPI buffer size              */
#define DSPUPBUFFER     0x80000000      /* HPI upstream buffer              */
#define DSPDNBUFFER     0x80000204      /* HPI downstream buffer            */
    
#define WALKING_ZEROS         0x01
#define WALKING_ONES          0x02
#define INC_MEM_LOC           0x04
#define INVERSE_INC_MEM_LOC   0x08
#define RANDOM_DATA           0x10
#define ALL_MEM_TESTS   (WALKING_ZEROS | WALKING_ONES | INC_MEM_LOC | \
   INVERSE_INC_MEM_LOC | RANDOM_DATA)

#define NUM_DSPS         2               /* Number of DSPs */
static const unsigned int hpiBase[NUM_DSPS] = {PORT0_BASE_ADDR, PORT1_BASE_ADDR};
#define HPIC_OFFSET 0x0
#define HPIA_OFFSET 0x8
#define HPID_OFFSET 0xC
#define HPIS_OFFSET 0x4

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIMemoryTest constructor
**
**    PURPOSE: Initialize data members for HPIMemoryTest object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
HPIMemoryTest::HPIMemoryTest(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIMemoryTest destructor
**
**    PURPOSE: Delete data members for HPIMemoryTest object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
HPIMemoryTest::~HPIMemoryTest()
{
}

void HpiWrite
(
unsigned int port,
void *buffer,
unsigned int addr,
int nBytes
)
{
unsigned int *bufferWord = (unsigned int *) buffer;
int n;
volatile unsigned int* hpisAddr = (unsigned int*)(hpiBase[port] + HPIS_OFFSET);

*(volatile unsigned int*)(hpiBase[port] + HPIA_OFFSET) = addr;
for (n = 0; n < nBytes; n += 4)
    {
    *hpisAddr = *bufferWord++;
    }
}

void HpiRead
(
unsigned int port,
unsigned int addr,
void *buffer,
int nBytes
)
{
unsigned int *bufferWord = (unsigned int *) buffer;
int n;
volatile unsigned int* hpisAddr = (unsigned int *)(hpiBase[port] + HPIS_OFFSET);

*(volatile unsigned int*)(hpiBase[port] + HPIA_OFFSET) = addr;    
for (n = 0; n < nBytes; n += 4)
    {
    *bufferWord++ =  *hpisAddr;
    }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIMemoryTest::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int HPIMemoryTest::InitiateDiagnostic(void)
{
   int testType, verbose;
   char testName[256];
   int numErrs = 0;
   unsigned int dsp;
   
   // Extract paramters from command line.
   //   
   // Command Line : test number, dsp, test type, verbose
   sscanf(parameters, "%s %u %d %d", &testName, &dsp, &testType, &verbose);
   
   // Verify that the paramaters are reasonable
   if (dsp > 1)
   {
      SendResultData("Invalid DSP number %u\n", dsp);
      return ERROR;
   }
   
   if (testType < WALKING_ZEROS || testType > ALL_MEM_TESTS)
   {
      SendResultData("Invalid testType %d\n", testType);
      return ERROR;
   }
   
   SendResultData ("%s dsp(%d) testType(%d) verbose(%d)\n",
      testName, dsp, testType, verbose);
  
   char writeBuf[DSPBUFFERMAX];
   char readBuf[DSPBUFFERMAX];
   unsigned int zeroPattern = 0x80000000;
   unsigned int onePattern = 0x80000000;
   
   while (testType)
   {
      // Build the test pattern for walking zeros.
      if (testType & WALKING_ZEROS)
      {
         unsigned int *writePtr = (unsigned int *)writeBuf;
         for (int i = 0; i < DSPBUFFERMAX/(sizeof(unsigned int)); i++)
         {
            *writePtr = ~zeroPattern;
            writePtr++;
         }
         
         zeroPattern >>= 1;
         if (!zeroPattern)
         {
            testType &= ~WALKING_ZEROS;
         }
      }
      // Build the test pattern for walking ones.
      else if (testType & WALKING_ONES)
      {
         unsigned int *writePtr = (unsigned int *)writeBuf;
         for (int i = 0; i < DSPBUFFERMAX/(sizeof(unsigned int)); i++)
         {
            *writePtr = onePattern;
            writePtr++;
         }
         
         onePattern >>= 1;
         if (!onePattern)
         {
            testType &= ~WALKING_ONES;
         }
      }
      // Build the test pattern for incrementing memory location.
      else if (testType & INC_MEM_LOC)
      {
         unsigned int *writePtr = (unsigned int *)writeBuf;
         
         for (unsigned int i = 0; i < DSPBUFFERMAX/(sizeof(unsigned int)); i++)
         {
            *writePtr = i;
            writePtr++;
         }
         testType &= ~INC_MEM_LOC;
      }
      
      // Test uplink buffer then downlink buffer.      
      HpiWrite(dsp, writeBuf, DSPUPBUFFER, DSPBUFFERMAX); 
      HpiRead(dsp, DSPUPBUFFER, readBuf, DSPBUFFERMAX); 
      
      for (int i = 0; i < DSPBUFFERMAX; i++)
      {
         if (writeBuf[i] != readBuf[i])
         {
            numErrs++;
            if (verbose)
            { 
               SendResultData ("0x%x wrote 0x%x read 0x%x\n", DSPUPBUFFER+i, 
                  writeBuf[i], readBuf[i]);
            }
         }
      }
      
      HpiWrite(dsp, writeBuf, DSPDNBUFFER, DSPBUFFERMAX); 
      HpiRead(dsp, DSPDNBUFFER, readBuf, DSPBUFFERMAX); 
      
      for (i = 0; i < DSPBUFFERMAX; i++)
      {
         if (writeBuf[i] != readBuf[i])
         {
            numErrs++;
            if (verbose)
            { 
               SendResultData ("0x%x wrote 0x%x read 0x%x\n", DSPDNBUFFER+i, 
                  writeBuf[i], readBuf[i]);
            }
         }
      }
   }
   
   if (numErrs)
   {
      SendResultData("HPIMemAccess failed with %d errors.\n", numErrs);
   }
   else
   {
      SendResultData("HPIMemAccess passed with zero errors!\n");
   }
   
   InternalStopTest();
   return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIMemoryTest::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int HPIMemoryTest::TerminateDiagnostic(void)
{
   return OK;
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIMemoryTest::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *HPIMemoryTest::GetDiagnosticHelp(void)
{
   return (
      "HPIMemoryAccess\n\n"
      "The transmit and receive memory defined for HPI transfers will be tested\n"
      "utilizing a parameter selected memory test.\n\n"
      "WARNING!!! DO NOT RUN WHILE DSP'S ARE RUNNING!!!!\n\n"
      "Parameters - DSPNum TestType Verbose\n"
      "\tDSPNum - Dsp to test (0, 1)\n"
      "\tTestType - Memory test algorithm\n"
      "\t\t1 = walking 0's\n"
      "\t\t2 = walking 1's\n"
      "\t\t4 = incrementing memory location\n"
      "\t\t7 = all tests\n"
      "\tVerbose - If non-zero show individual error occurences.\n\n"
      "Returns - Number of failed memory locations\n"
   );
}



// *******************************************************************
// I2CLoopbackTest
// *******************************************************************

    
/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CLoopbackTest constructor
**
**    PURPOSE: Initialize data members for I2CLoopbackTest object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
I2CLoopbackTest::I2CLoopbackTest(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CLoopbackTest destructor
**
**    PURPOSE: Delete data members for I2CLoopbackTest object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
I2CLoopbackTest::~I2CLoopbackTest()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CLoopbackTest::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int I2CLoopbackTest::InitiateDiagnostic(void)
{
   
   // Extract parameters from command line.
   //   
   // Command Line : test number, numBytes, duration, verbose
   numBytes = duration = verbose = haltOnErr = 0;
   sscanf(parameters, "%s %d %u %d %d", &testName, &numBytes, &duration, &verbose,
      &haltOnErr);
   
   // Validate parameters.
   if ((numBytes == 0) && (duration == 0))
   {
      SendResultData("I2CLoopbackTest: numBytes or duration must be nonzero!\n");
      return ERROR;
   }
   
   SendResultData ("%s numBytes(%d) duration(%d) verbose(%d) haltOnErr(%d)\n",
      testName, numBytes, duration, verbose, haltOnErr);
      
   // Initialize a counter for the total bytes sent and total errors.
   totalSent = 0;
   numErrs = 0;
   
   // If duration is non-zero then start the diag timer.  numBytes is ignored
   // if duration is non-zero.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Send a message to start sending data.
   DiagnosticMsgType diagMsg;

   // Initialize the diagnostic message.
   diagMsg.func = I2C_LOOPBACK_CONTINUE;

   msgQSend(diagQId, 
            (char *)&diagMsg, 
            sizeof(DiagnosticMsgType), 
            WAIT_FOREVER, 
            MSG_PRI_NORMAL);
   
   
   return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CLoopbackTest::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int I2CLoopbackTest::TerminateDiagnostic(void)
{
   SendResultData("I2CLoopback: Sent Bytes(%d) Error Bytes(%d).\n", 
      totalSent, numErrs);
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CLoopbackTest::ProcessMessage
**
**    PURPOSE: Process messages not handled by Diagnostic base class.  The
**       only expected message is I2C_LOOPBACK_CONTINUE.  When this message
**       is received another buffer of data is sent to the I2C.
**
**    INPUT PARAMETERS: diagMsg - message sent to diagnostic
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int I2CLoopbackTest::ProcessMessage(DiagnosticMsgType diagMsg)
{
   int testDone = CONTINUE_TEST;
   switch ( diagMsg.func )
   {
      case I2C_LOOPBACK_CONTINUE:
      {
         testDone = SendI2CData();
         break;
      }
   }
   
   return testDone;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CLoopbackTest::SendI2CData
**
**    PURPOSE: Send some data to the I2C for loopback.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int I2CLoopbackTest::SendI2CData(void)
{
   int numSent = 0;
   int retVal = CONTINUE_TEST;
   
   // Enable I2C facility and enable receive buffer.
   I2Coperation(ON);
   RxBD(ON);
   
   // If this is a timed run then just send a full buffer of data.
   if (duration)
   {
      numBytes = I2C_TX_LEN-1;
   }
   
   while (numBytes && (numSent < I2C_TX_LEN-1))
   {
      writeBuf[numSent] = (unsigned char)rand();
      numBytes--;
      numSent++;
   }
   
   // Write out test data.
   int status;
   if ((status = I2Cwrite2(writeBuf, MPC860_I2C_ADDR>>1, numSent)) == ERROR)
   {
      SendResultData("I2Cwrite2 failed %d\n", status);
      return CONTINUE_TEST;
   }
   taskDelay(1);
   readRxBD(readBuf, numSent+1);
   
   for (int i = 0; i < numSent; i++)
   {
      if (readBuf[i+1] != writeBuf[i])
      {
         if (verbose)
         {
            SendResultData("I2CLoopback:ERROR Offset(%d) sent(%x) received(%x)\n",
               i, writeBuf[i], readBuf[i+1]);
         }
         numErrs++;
         if (haltOnErr)
         {
            retVal = STOP_TEST;
         }
      }
   }
   
   // Keep track of how many bytes were sent.
   totalSent += numSent;
   
   I2Coperation(OFF);
   
   // If this test is not running for some duration then check to see
   // if all the bytes have been sent.
   if (!duration)
   {
      if (numBytes <= 0)
      {
         retVal = STOP_TEST;
      }
   }
   
   if (retVal == CONTINUE_TEST)
   {
      // Send a message to start sending data.
      DiagnosticMsgType diagMsg;

      // Initialize the diagnostic message.
      diagMsg.func = I2C_LOOPBACK_CONTINUE;

      msgQSend(diagQId, 
               (char *)&diagMsg, 
               sizeof(DiagnosticMsgType), 
               WAIT_FOREVER, 
               MSG_PRI_NORMAL);
   }
   else
   {
      // If we are stopping the test for any reason and it is a timed test
      // then cancel the timer.
      if (duration) 
      {
         SetTimer(0);
      }
   }
   
   return retVal;
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CLoopbackTest::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *I2CLoopbackTest::GetDiagnosticHelp(void)
{
   return (
      "I2CLoopback Test\n\n"
      "The MP860 is allowed to send data addressed to itself.  This creates\n"
      "a loopback scenario.  For this test the MP860 will send random data from\n"
      "its transmit buffers to its receive buffers.  The received data will be\n"
      "verified and errors will be flagged.\n\n"
      "Parameters - NumBytes Duration Verbose HaltOnErr\n"
      "\tNumBytes - Number of bytes to send in loopback\n"
      "\tDuration - Optional test duration in seconds.  If zero the NumBytes\n"
      "\t\tparameter is used.  If non-zero the test will run for the\n"
      "\t\tspecified number of seconds and the NumBytes parameter is ignored.\n"
      "\tVerbose - If non-zero show individual error occurences.\n"
      "\tHaltOnErr - If non-zero the test will stop when an error occurs.\n\n"
      "Returns - Number of bytes transmitted, number of erroneous bytes received\n"
   );
}


// *******************************************************************
// DSPMemTest
// *******************************************************************
#define DSP_INT_DATA_RAM_START         0x80000000
#define DSP_INT_DATA_RAM_END           0x80010000
#define DSP_INT_PROG_RAM_START         0x01400000
#define DSP_INT_PROG_RAM_END           0x01410000
#define DSP_EXT_RAM_START              0x00000000
#define DSP_EXT_RAM_END                0x00100000

#define TEST_INTERNAL_PROGRAM_MEM      0x1
#define TEST_INTERNAL_DATA_MEM         0x2
#define TEST_EXTERNAL_MEM              0x4

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMemTest constructor
**
**    PURPOSE: Initialize data members for DSPMemTest object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPMemTest::DSPMemTest(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMemTest destructor
**
**    PURPOSE: Delete data members for DSPMemTest object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPMemTest::~DSPMemTest()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMemTest::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPMemTest::InitiateDiagnostic(void)
{
   duration = verbose = 0;
   // Extract paramters from command line.
   //   
   // Command Line : test number, dsp, test type, duration, verbose
   sscanf(parameters, "%s %u %x %x %u %d", &testName, &dsp, &testLoc, &testType, &duration, &verbose);
   
   // Verify that the paramaters are reasonable
   if (dsp > 1)
   {
      SendResultData("Invalid DSP number %u\n", dsp);
      return ERROR;
   }
   
   if (testType < WALKING_ZEROS || testType > ALL_MEM_TESTS)
   {
      SendResultData("Invalid testType %d\n", testType);
      return ERROR;
   }
   
   SendResultData ("%s dsp(%d) testLocation (%#x) testType(%#x) duration(%d) verbose(%d)\n",
      testName, dsp, testLoc, testType, duration, verbose);

   // Initialize the random number seed to 1.  Each pass will increment the
   // seed by one.
   seed = 1;
      
   // Initialize a counter for the total errors.
   numErrs = 0;
   
   // Save a temp copy of the test type.
   testTypeTmp = testType;
   
   // Setup the starting test pattern
   onesPattern = 0x80000000;
   zerosPattern = 0x7fffffff;
   if (testType & WALKING_ZEROS)
   {            
      testPattern = zerosPattern;
   }
   else if (testType & WALKING_ONES)
   {
      testPattern = onesPattern;
   }
   
   // Reset DSPs
   auxPortOutSet(~(DSPA_RST_NOT|DSPB_RST_NOT), DSPA_RST_NOT|DSPB_RST_NOT);   
   taskDelay((int)(0.350 * sysClkRateGet()));
   auxPortOutSet((DSPA_RST_NOT|DSPB_RST_NOT), DSPA_RST_NOT|DSPB_RST_NOT);
   
   // In case the dsp EMIF registers have not been configured set them up
   // to allow access to external memory.
   HPIC(dsp)  = 0x00000000;
   HPIA(dsp) = 0x1800000;
   *(unsigned int *)HPID(dsp) = 0x00003060;
   HPIA(dsp) = 0x1800008;
   *(unsigned int *)HPID(dsp) = 0x00000040;
   HPIA(dsp) = 0x1800004;
   *(unsigned int *)HPID(dsp) = 0x20E30322;
   HPIA(dsp) = 0x1800010;
   *(unsigned int *)HPID(dsp) = 0x00000040;
   HPIA(dsp) = 0x1800014;
   *(unsigned int *)HPID(dsp) = 0x00000040;
   HPIA(dsp) = 0x1800018;
   *(unsigned int *)HPID(dsp) = 0x07116000;
   HPIA(dsp) = 0x180001c;
   *(unsigned int *)HPID(dsp) = 0x00000410;
   
   
   // If duration is non-zero then start the diag timer.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Send a message to start sending data.
   DiagnosticMsgType diagMsg;

   // Initialize the diagnostic message.
   diagMsg.func = DSP_MEM_TEST_CONTINUE;

   msgQSend(diagQId, 
            (char *)&diagMsg, 
            sizeof(DiagnosticMsgType), 
            WAIT_FOREVER, 
            MSG_PRI_NORMAL);
            
   return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMemTest::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPMemTest::TerminateDiagnostic(void)
{
   // If we are stopping the test for any reason and it is a timed test
   // then cancel the timer.
   if (duration) 
   {
      SetTimer(0);
   }
   
   SendResultData("DSPMemTest: Num Errors(%d).\n", numErrs); 

   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMemTest::ProcessMessage
**
**    PURPOSE: Process messages not handled by Diagnostic base class.  The
**       only expected message is DSP_MEM_TEST_CONTINUE.
**
**    INPUT PARAMETERS: diagMsg - message sent to diagnostic
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int DSPMemTest::ProcessMessage(DiagnosticMsgType diagMsg)
{
   switch ( diagMsg.func )
   {
      case DSP_MEM_TEST_CONTINUE:
      {
         if ((testTypeTmp & WALKING_ZEROS) || (testTypeTmp & WALKING_ONES))
         {
            // Test internal data RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_INTERNAL_DATA_MEM)
               TestDspMemBlock(DSP_INT_DATA_RAM_START, DSP_INT_DATA_RAM_END);
            
            // Test internal program RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_INTERNAL_PROGRAM_MEM)
               TestDspMemBlock(DSP_INT_PROG_RAM_START, DSP_INT_PROG_RAM_END);
            
            // Test extenal RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_EXTERNAL_MEM)
               TestDspMemBlock(DSP_EXT_RAM_START, DSP_EXT_RAM_END);
            
            if (testTypeTmp & WALKING_ZEROS)
            {            
               zerosPattern = ~((~zerosPattern) >> 1);
               testPattern = zerosPattern;
               if (!(~zerosPattern))
               {
                  testTypeTmp &= ~WALKING_ZEROS;
                  zerosPattern = 0x7fffffff;
               }
            }
            // Build the test pattern for walking ones.
            else if (testTypeTmp & WALKING_ONES)
            {
               onesPattern >>= 1;
               testPattern = onesPattern;
               if (!onesPattern)
               {
                  testTypeTmp &= ~WALKING_ONES;
                  onesPattern = 0x80000000;
               }
            }           
         }
         else if (testTypeTmp & INC_MEM_LOC)
         {
            // Test internal data RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_INTERNAL_DATA_MEM)
               TestDspMemBlockIncAddr(DSP_INT_DATA_RAM_START, DSP_INT_DATA_RAM_END, FALSE);
            
            // Test internal program RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_INTERNAL_PROGRAM_MEM)
               TestDspMemBlockIncAddr(DSP_INT_PROG_RAM_START, DSP_INT_PROG_RAM_END, FALSE);
            
            // Test extenal RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_EXTERNAL_MEM)
               TestDspMemBlockIncAddr(DSP_EXT_RAM_START, DSP_EXT_RAM_END, FALSE);
            
            testTypeTmp &= ~INC_MEM_LOC;
         }
         else if (testTypeTmp & INVERSE_INC_MEM_LOC)
         {
            // Test internal data RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_INTERNAL_DATA_MEM)
               TestDspMemBlockIncAddr(DSP_INT_DATA_RAM_START, DSP_INT_DATA_RAM_END, TRUE);
            
            // Test internal program RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_INTERNAL_PROGRAM_MEM)
               TestDspMemBlockIncAddr(DSP_INT_PROG_RAM_START, DSP_INT_PROG_RAM_END, TRUE);
            
            // Test extenal RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_EXTERNAL_MEM)
               TestDspMemBlockIncAddr(DSP_EXT_RAM_START, DSP_EXT_RAM_END, TRUE);
         
            testTypeTmp &= ~INVERSE_INC_MEM_LOC;
         }
         else if (testTypeTmp & RANDOM_DATA)
         {
            // Test internal data RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_INTERNAL_DATA_MEM)
               TestDspMemBlockRandom(DSP_INT_DATA_RAM_START, DSP_INT_DATA_RAM_END);
            
            // Test internal program RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_INTERNAL_PROGRAM_MEM)
               TestDspMemBlockRandom(DSP_INT_PROG_RAM_START, DSP_INT_PROG_RAM_END);
            
            // Test extenal RAM.  If errors occur then set a bit in the
            // power on test status register.
            if (testLoc & TEST_EXTERNAL_MEM)
               TestDspMemBlockRandom(DSP_EXT_RAM_START, DSP_EXT_RAM_END);
         
            testTypeTmp &= ~RANDOM_DATA;
         }
         break;
      }
   }
   
   if (!testTypeTmp)
   {
      if (duration)
      {
         testTypeTmp = testType;
      }
      else
      {
         return STOP_TEST;
      }
   }
   
   // Send a message to start sending data.
   DiagnosticMsgType contMsg;

   // Initialize the diagnostic message.
   contMsg.func = DSP_MEM_TEST_CONTINUE;

   msgQSend(diagQId, 
            (char *)&contMsg, 
            sizeof(DiagnosticMsgType), 
            WAIT_FOREVER, 
            MSG_PRI_NORMAL);
   
   return CONTINUE_TEST;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMemTest::TestDspMemBlock
**
**    PURPOSE: Test the RAM at the given address with testPattern.
**
**    INPUT PARAMETERS: startAddr - starting location to test
**                      endAddr - ending location to test
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DSPMemTest::TestDspMemBlock(unsigned int startAddr, unsigned int endAddr)
{
   unsigned int read;
   for (unsigned int dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {
      // Write test pattern.
      HPIA(dsp) = dspAddr;
      *(unsigned int *)HPID(dsp) = testPattern;
      
      // Read back from memory location.
      HPIA(dsp) = dspAddr;
      read = *(unsigned int *)HPID(dsp);
      
      if (read != testPattern)
      {
         numErrs++;
         if (verbose)
         {
            SendResultData("DSPMemTest: Error address(%x) wrote(%x) read(%x)\n",
               dspAddr, testPattern, read);
         }
      }
   }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMemTest::TestDspMemBlockIncAddr
**
**    PURPOSE: Test the RAM at the given address with testPattern.
**
**    INPUT PARAMETERS: startAddr - starting location to test
**                      endAddr - ending location to test
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DSPMemTest::TestDspMemBlockIncAddr(unsigned int startAddr, unsigned int endAddr,
   int doInverse)
{
   unsigned int read;
   HPIA(dsp) = startAddr;
   for (unsigned int dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {
      // Write test pattern.
      if (doInverse)
      {
         *(unsigned int *)HPIS(dsp) = ~dspAddr;
      }
      else
      {
         *(unsigned int *)HPIS(dsp) = dspAddr;
      }
   }
  
   for (dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {  
      unsigned int expectedResult = dspAddr;
        
      // Read back from memory location.
      HPIA(dsp) = dspAddr;
      read = *(unsigned int *)HPID(dsp);
      
      if (doInverse)
      {
         expectedResult = ~expectedResult;
      }
      
      if (read != expectedResult)
      {
         numErrs++;
         if (verbose)
         {
            SendResultData("DSPMemTest: Error address(%x) wrote(%x) read(%x)\n",
               dspAddr, expectedResult, read);
         }
      }
   }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMemTest::TestDspMemBlockRandom
**
**    PURPOSE: Test the RAM at the given address range with random data.
**
**    INPUT PARAMETERS: startAddr - starting location to test
**                      endAddr - ending location to test
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void DSPMemTest::TestDspMemBlockRandom(unsigned int startAddr, unsigned int endAddr)
{
   unsigned int read, write;
   HPIA(dsp) = startAddr;
   srand(seed);
   for (unsigned int dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {
      // Generate a 32-bit random number.  rand() only returns 16 bits.
      write = (rand() << 16);
      write |= rand();
      // Write test pattern.
      *(unsigned int *)HPIS(dsp) = write;
   }
  
  
   srand(seed);
   seed++;
   for (dspAddr = startAddr; dspAddr < endAddr; dspAddr+=sizeof(unsigned int))
   {  
      unsigned int expectedResult;
        
      // Regenerate the random number sequence to determine the expected result.
      expectedResult = (rand() << 16);
      expectedResult |= rand();
        
      // Read back from memory location.
      HPIA(dsp) = dspAddr;
      read = *(unsigned int *)HPID(dsp);
      
      if (read != expectedResult)
      {
         numErrs++;
         if (verbose)
         {
            SendResultData("DSPMemTest: Error address(%x) wrote(%x) read(%x)\n",
               dspAddr, expectedResult, read);
         }
      }
   }
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMemTest::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *DSPMemTest::GetDiagnosticHelp(void)
{
   return (
      "DSPMemTest\n\n"
      "Internal program and data RAM and external RAM will be tested \n"
      "utilizing a parameter selected memory test.\n\n"
      "WARNING!!! DO NOT RUN WHILE DSP'S ARE RUNNING!!!!\n\n"
      "Parameters - DSPNum TestLocation TestType Duration Verbose\n"
      "\tDSPNum - Dsp to test (0, 1)\n"
      "\tTestLocation - Which part of DSP memory to test\n"
      "\t\t0x01 = Internal PROGRAM memory\n"
      "\t\t0x02 = Internal DATA memory\n"
      "\t\t0x04 = External memory\n"
      "\t\t0x07 = all locations\n"
      "\tTestType - Memory test algorithm\n"
      "\t\t0x01 = walking 0's\n"
      "\t\t0x02 = walking 1's\n"
      "\t\t0x04 = incrementing address\n"
      "\t\t0x08 = inverse incrementing address\n"
      "\t\t0x10 = random data test\n"
      "\t\t0x1f = all tests\n"
      "\tDuration - If non-zero the test will execute for the specified number\n"
      "\t\tof seconds.  If zero the specified test(s) will execute once.\n"
      "\tVerbose - If non-zero show individual error occurences.\n\n"
      "Returns - Number of failed memory locations\n"
   );
}


// *******************************************************************
// CFTest
// *******************************************************************

//------------------------------------------------------------------------------
CFTest::CFTest(char *testName, int testNum, 
               ReqSourceType reqSrc, char *parms)
       : Diagnostic(testName, testNum, reqSrc, parms)
{
}

//------------------------------------------------------------------------------
CFTest::~CFTest()
{
}

//------------------------------------------------------------------------------
int CFTest::InitiateDiagnostic(void)
{
  int numRepeat;      /* number of times to repeat the test */
  char testName[100];

  sscanf(parameters, "%s %d", &testName, &numRepeat);
  if (numRepeat < 0)
  {
    SendResultData("Invalid repeat count for the test (numRepeat)\n");
    return ERROR;
  }
  SendResultData("%s numRepeat(%d)\n", testName, numRepeat);

#define FILE_NAME "/ata/cfTestFile"

/*******************************************************************************
cfTest : CompactFlash Test

Test the compact flash interface ann the media by writing a file with random 
characters and reading back to verify the write.
Performs the test given number of times.
*/

  const int FileSize = 1024 * 1024;
  const int BuffSize = 1024;
  STATUS retStat = OK;
  int fd;
  char buff[BuffSize];
  int i,j,n;
  char* errorStr = "";
 
  for (n = 0; (n < numRepeat) && (retStat != ERROR); n++)
  {  
    fd = creat(FILE_NAME, O_RDWR);
    if (fd != ERROR)
    {
      srand(1);     /* set the seed */
      /* write */
      for (i = 0; i < (FileSize/BuffSize) && (retStat != ERROR); i++)
      {
        for (j = 0; j < BuffSize; j++)
        {
          buff[j] = rand();
        }
        if (write(fd, buff, BuffSize) == ERROR) 
        {
          retStat = ERROR;
          errorStr = "Error Writing";
          break;
        }
      }
      ioctl(fd, FIOSYNC, 0);   /* flush the output, invalidate cache */

      lseek(fd, 0, SEEK_SET);  /* read from the start of the file */
      srand(1);     /* set the seed to generate same sequence of random numbers */
      /* read */
      for (i = 0; (i < (FileSize/BuffSize)) && (retStat != ERROR); i++)
      {
        if (read(fd, buff, BuffSize) == ERROR)
        {          
          retStat = ERROR;
          errorStr = "Error Reading";
          break;
        }
        for (j = 0; j < BuffSize; j++)
        {
          if (buff[j] != (char)rand())
          {
            retStat = ERROR;
            errorStr = "Incorrect Data Read";
            break;
          }
        }
      }
      close(fd);
      remove(FILE_NAME);
    }
    else
    {    
      retStat = ERROR;
      errorStr = "Cannot Create File";
      break;
    }
  }
  SendResultData("CompactFlash Test: %s %s\n", (retStat == ERROR) ? "FAIL;" : "PASS", errorStr);

  InternalStopTest();
  return retStat;

}

//------------------------------------------------------------------------------
int CFTest::TerminateDiagnostic(void)
{
  return OK;
}

//------------------------------------------------------------------------------
char* CFTest::GetDiagnosticHelp(void)
{
  return
  ("
   CFTest - CompactFlash Test

   Tests the CompactFlash interface and the media by writing a file with 
   random characters and reading back the data.

   Parameters:
   numRepeat - number of times to repeat the test.
  ");  
}


// *******************************************************************
// FPGA Test
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: FPGATest constructor
**
**    PURPOSE: Initialize data members for FPGATest object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
FPGATest::FPGATest(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: FPGATest destructor
**
**    PURPOSE: Delete data members for FPGATest object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
FPGATest::~FPGATest()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: FPGATest::InitiateDiagnostic
**
**    PURPOSE: This test will test out four FPGA registers with an incrementing 
**             value test. Essentially we want to write from 0 to 0xffff to the 
**             FPGA registers.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int FPGATest::InitiateDiagnostic(void)
{
	unsigned int	value;
	unsigned int    mask	  = 0x0000FFFF;
	unsigned int    addr	  = 0x1030000;
	unsigned int	testValue = 0x00000080;
	unsigned int	fpga_num;
	unsigned int	index;
	int				verbose = 0;
    char            testName[256];

	sscanf(parameters, "%s %d", &testName, &verbose);

    if ( verbose != 0 & verbose != 1 )
    {
        SendResultData("invalid value for verbose: %d\n", verbose);
        return ERROR;
    }

	for	( fpga_num=0; fpga_num<2; fpga_num++)
	{
		SendResultData("Starting FPGA #%d Test...\n", fpga_num);
		for ( index=0; index<0x10000; index++ )
		{			
			/* Write each location with an incrementing value up
			   to 16 bit unsigned limit. Make sure each register 
			   is offset from each other to test internals.	*/

			hpi2dspD(fpga_num,0x1000000,((index)   % 0x10000));
			hpi2dspD(fpga_num,0x1040000,((index+1) % 0x10000));
			hpi2dspD(fpga_num,0x1050000,((index+2) % 0x10000));
			hpi2dspD(fpga_num,0x1060000,((index+3) % 0x10000));

			if ( verbose & ((index == 0x1 ) || (index == 0x3000) || (index == 0x9000)))
				SendResultData("Finished writing memory - index=%d.\n", index);

			// Test location 0x1000000
			// Read back corresponding values, only take lower 16 bits back
			value=dsp2hpiD(fpga_num,0x1000000) & 0x0000ffff;
			if (value!=((index) % 0x10000)) 
			{
				SendResultData("FPGA #%d Test - FAILED at Location 0x1000000\n", fpga_num);
				return ERROR;
			}
			else if ( verbose & ((index == 0x1 ) || (index == 0x3000) || (index == 0x9000)))
				SendResultData("Finished reading memory at addr 0x1000000 - value = %d.\n", value);			

			// Test location 0x1040000
			// Read back corresponding values
			value=dsp2hpiD(fpga_num,0x1040000) & 0x0000ffff;
			if (value!=((index+1) % 0x10000)) 
			{
				SendResultData("FPGA #%d Test - FAILED at Location 0x1040000\n", fpga_num);
				return ERROR;
			}
			else if ( verbose & ((index == 0x1 ) || (index == 0x3000) || (index == 0x9000)))
				SendResultData("Finished reading memory at addr 0x1040000 - value = %d.\n", value);			

			// Test location 0x1050000
			// Read back corresponding values
			value=dsp2hpiD(fpga_num,0x1050000) & 0x0000ffff;
			if (value!=((index+2) % 0x10000)) 
			{
				SendResultData("FPGA #%d Test - FAILED at Location 0x1050000\n", fpga_num);
				return ERROR;
			}
			else if ( verbose & ((index == 0x1 ) || (index == 0x3000) || (index == 0x9000)))
				SendResultData("Finished reading memory at addr 0x1050000 - value = %d.\n", value);			

			// Test location 0x1060000
			// Read back corresponding values
			value=dsp2hpiD(fpga_num,0x1060000) & 0x0000ffff;
			if (value!=((index+3) % 0x10000)) 
			{
				SendResultData("FPGA #%d Test - FAILED at Location 0x1060000\n", fpga_num);
				return ERROR;
			}
			else if ( verbose & ((index == 0x1 ) || (index == 0x3000) || (index == 0x9000)))
				SendResultData("Finished reading memory at addr 0x1060000 - value = %d.\n\n", value);			
		}
		SendResultData("FPGA #%d Test - PASSED\n", fpga_num);
	}
    InternalStopTest();

    return(OK);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: FPGATest::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int FPGATest::TerminateDiagnostic(void)
{
   return OK;
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: FPGATest::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *FPGATest::GetDiagnosticHelp(void)
{
   return (
      "**************************************\n"
      "* CAUTION: This is an INTRUSIVE Test *\n"
      "**************************************\n\n"
      "It can only be used after running the dspTest script at boot up.\n\n"
      "FPGA Test - Checks FPGA 0 then FPGA 1.\n"
      "            It executes writes and reads to the FPGA.\n\n"
      "Parameters: \n\n"
      "      Verbose: 1 = Status Messages Sent, 0 = No Status Mesgs\n\n"
      
      "Returns - Test Results: Pass or Fail for each FPGA.\n"
   );
}



// *******************************************************************
// I2C Test
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CTest constructor
**
**    PURPOSE: Initialize data members for I2C Test object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
I2CTest::I2CTest(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CTest destructor
**
**    PURPOSE: Delete data members for I2CTest object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
I2CTest::~I2CTest()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CTest::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int I2CTest::InitiateDiagnostic(void)
{
   STATUS      result;

   SendResultData("Starting I2C Test...");

   if (( result = DS1780_Init() ) == OK )
      SendResultData("PASSED\n");
   else
      SendResultData("FAILED\n");

   InternalStopTest();

   return(OK);
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CTest::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int I2CTest::TerminateDiagnostic(void)
{
   return OK;
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CTest::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *I2CTest::GetDiagnosticHelp(void)
{
   return (
      "I2C Test\n\n"
      "Initializes the DS1780 and sets it to its operating mode.\n"
      "Returns - Status on the Standard Output\n");
}
