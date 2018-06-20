// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

/*******************************************************************************
DSP Test Messages 
*/
#include <vxWorks.h>
#include <stdio.h>
#include <semLib.h>
#include <taskLib.h>

#include "dsp/dsptypes.h"
#include "dsp/dsphmsg.h"
#include "DspDiags.h"

#define L1P_WRITE_H2L_4BYTES(x,y) *y = x, *(y+1) = (x >> 8), *(y+2) = (x >> 16), *(y+3) = (x >> 24)

#define TIMEOUT 100

#define SIZE_INDEX         0
#define FUNCTION_INDEX     4
#define TYPE_HI_BYTE_INDEX 5
#define TYPE_LO_BYTE_INDEX 6
#define TRX_INDEX          7

#define MSG_HEADER_SIZE    8

#define GENERATE_TONE_MSG_SIZE  (MSG_HEADER_SIZE + 2)
#define MEASURE_TONE_MSG_SIZE  (MSG_HEADER_SIZE + 0)
#define MEM_TEST_MSG_SIZE  (MSG_HEADER_SIZE + 1) 

typedef enum
{
  LoopbackTest,
  MemoryTest
} t_dspTest;

int         dspDestinationDSP = 0;
int         dspTestInProgress = FALSE;
t_dspTest   dspTest;

void sendDsp(unsigned char *buffer, int len);

/******************************************************************************* 
dspBBLoopbackTest - DSP baseband Loopback Test

Sends messages to DSPs to generate tones and receive the measurement. 
Before sending the message, loads the DSPs with the given file to excute a 
special test program.
After starting the test it blocks till it is informed that a responce was 
received from or it times out.

*/
void DSPBBLoopbackTest::dspBBLoopbackTest
(
  int           srcDsp,      /* DSP that genrates the tone */
  int           dstDsp,      /* DSP that measures the tone */ 
  int           freqIndex,   /* Index of the tone to be generated */
  int           gainShift,   /* Represents the # of right bit shifts  */
  int           verbose      /* verbose=1 => debug messages sent, =0 => not msgs sent  */
)

{
  unsigned char msg[max(GENERATE_TONE_MSG_SIZE,MEASURE_TONE_MSG_SIZE)];
  int   i;

  if (dspTestInProgress == TRUE)
    return;
  else
    dspTestInProgress = TRUE;
  
  dspTest = LoopbackTest;

  L1P_WRITE_H2L_4BYTES(GENERATE_TONE_MSG_SIZE, &msg[SIZE_INDEX]);
  msg[FUNCTION_INDEX]      = DIAGNOSTIC_MSG;
  msg[TYPE_HI_BYTE_INDEX]  = MPH_DSP_GENERATE_TONE_REQ >> 8;
  msg[TYPE_LO_BYTE_INDEX]  = MPH_DSP_GENERATE_TONE_REQ & 0xFF;
  msg[TRX_INDEX]           = src;
  msg[MSG_HEADER_SIZE + 0] = (char)freqIndex;
  msg[MSG_HEADER_SIZE + 1] = (char)gainShift;   
  sendDsp(msg, GENERATE_TONE_MSG_SIZE);

  if ( verbose == TRUE )
  {
      for ( i=0; i<GENERATE_TONE_MSG_SIZE; i++ )
          SendResultData("%x ",msg[i]);

      SendResultData("sendDsp(msg, GENERATE_TONE_MSG_SIZE) - Cmd #1\n");
  }

  dspDestinationDSP = dst;
}

/******************************************************************************* 
dspBBLoopbackTest - DSP baseband Loopback Test2

Sends message to DSPs to receive the measurement. 

*/
void DSPBBLoopbackTest::dspBBLoopbackTest2
(
  int   verbose     /* verbose=1 => debug messages sent, =0 => not msgs sent  */
)

{
  unsigned char msg[max(GENERATE_TONE_MSG_SIZE,MEASURE_TONE_MSG_SIZE)];
  int   i;

  L1P_WRITE_H2L_4BYTES(MEASURE_TONE_MSG_SIZE, &msg[SIZE_INDEX]);
  msg[FUNCTION_INDEX]     = DIAGNOSTIC_MSG;
  msg[TYPE_HI_BYTE_INDEX] = MPH_DSP_MEASURE_TONE_REQ  >> 8;
  msg[TYPE_LO_BYTE_INDEX] = MPH_DSP_MEASURE_TONE_REQ  & 0xFF;
  msg[TRX_INDEX]          = dspDestinationDSP;

  sendDsp(msg, MEASURE_TONE_MSG_SIZE);

  if ( verbose == TRUE )
  {
      for ( i=0; i<MEASURE_TONE_MSG_SIZE; i++ )
          SendResultData("%x ",msg[i]);

      SendResultData("sendDsp(msg, MEASURE_TONE_MSG_SIZE) - Cmd #2\n");
  }
}


/******************************************************************************* 
dspExtMemTest - DSP External Memory Test

Sends a message to DSP to perform external memory test. 
Before sending the message, loads the DSPs with the given file to excute a 
special test program.
After starting the test it blocks till it is informed that a responce was 
received from or it times out.
*/
void DSPExtMemTest::dspExtMemTest
(
  int dspNum,         /* IN: dsp number */
  int testType,       /* IN: test type */
  int verbose         /* verbose=1 => debug messages sent, =0 => not msgs sent  */
)
{
  unsigned char msg[MEM_TEST_MSG_SIZE];
  int           i;

  if (dspTestInProgress == TRUE)
    return;
  else
    dspTestInProgress = TRUE;

  dspTest = MemoryTest;

  L1P_WRITE_H2L_4BYTES(MEM_TEST_MSG_SIZE, &msg[SIZE_INDEX]);
  msg[FUNCTION_INDEX] = DIAGNOSTIC_MSG;
  msg[TYPE_HI_BYTE_INDEX] = MPH_DSP_EXT_MEM_CHECK_REQ >> 8;
  msg[TYPE_LO_BYTE_INDEX] = MPH_DSP_EXT_MEM_CHECK_REQ & 0xFF;
  msg[TRX_INDEX] = dspNum;
  msg[MSG_HEADER_SIZE + 0] = testType;

  sendDsp(msg, MEM_TEST_MSG_SIZE);

  if ( verbose == TRUE )
  {
      for ( i=0; i<MEASURE_TONE_MSG_SIZE; i++ )
          SendResultData("%x ",msg[i]);

      SendResultData("sendDsp(msg, MEM_TEST_MSG_SIZE)\n");
  }
}


#define L1P_READ_H2L_4BYTES(x) (((int)*x << 24) | \
                                ((int)*(x+1) << 16) | \
                                ((int)*(x+2) << 8) |  \
                                ((int)*(x+3))) 


/******************************************************************************* 
read4h2lBytes  - read 4 high to low byes

reads 4bytes that are in high to low order and returns the integer
*/
static int read4h2lBytes(char *x)
{
  int retVal;

  retVal = (((*x)<<24) | (*(x+1)<<16) | (*(x+2)<<8) | (*(x+3)));
  return retVal;
}


/******************************************************************************* 
dspToneMeasureRsp - DSP Tone Measurement Response

Receives result of tone Measurement from DSP
*/
void DSPBBLoopbackTest::dspToneMeasureRsp
(
  unsigned char data[],     /* IN: Response */
  int           verbose    /* verbose=1 => debug messages sent, =0 => not msgs sent  */
)
{
  unsigned char *pData = data;
  int error;
  int i;
  int result[8];

  if (dspTestInProgress != TRUE)
    return;
  
  if (dspTest != LoopbackTest)
    return;

  error = FALSE;
  for (i = 0; i < 8; i++)
  {
    result[i] = L1P_READ_H2L_4BYTES(pData);
    pData    += 4;
  }
  
  SendResultData("DSP Baseband Loopback Measurement:");
  for (i = 0; i < 7; i++)
      SendResultData(" %d,", result[i]);
  SendResultData(" %d\n", result[7]);

  dspTestInProgress = FALSE;
}


/******************************************************************************* 
dspExtMemTestRsp - DSP External Memory Test Response

Receives result of external memory test from DSP
*/
void DSPExtMemTest::dspExtMemTestRsp
(
  unsigned char data[]     /* IN: Response */
)
{
  if (dspTestInProgress != TRUE)
    return;
  
  if (dspTest != MemoryTest)
    return;

  numErrors += L1P_READ_H2L_4BYTES(data);
  dspTestInProgress = FALSE;
}

/******************************************************************************* 
dspExtMemErrorRsp - DSP External Memory Error Response

Receives result of external memory test error reports from DSP
*/
void DSPExtMemTest::dspExtMemErrorRsp
(
  unsigned char data[]     /* IN: Response */
)
{
  int memAddr;
  int dataWrote;
  int dataRead;
 
  if (dspTestInProgress != TRUE)
    return;
  
  if (dspTest != MemoryTest)
    return;

  memAddr = L1P_READ_H2L_4BYTES(&data[4]);
  dataWrote = (data[8] << 8) | (data[9]);
  dataRead = (data[10] << 8) | (data[11]);
  
  SendResultData("DSP External Memory Test: Error at address %08x, " 
                 "wrote %04x, read %04x\n", memAddr, dataWrote, dataRead);
}  
