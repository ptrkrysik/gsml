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
// File        : DspDiags.cpp
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description :  
//
// *******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <taskLib.h>
#include <string.h>
#include "DspDiags.h"
#include "dsp/dsptypes.h"
#include "dsp/dsphmsg.h"
#include "rm/rml1_intf.h"
#include "logging/vclogging.h"
#include "logging/vcmodules.h"
#include "cdcUtils/cdcUtils.h"

// *******************************************************************
// DSPLoopbackTest
// *******************************************************************

#define DSP_NO_LOOPBACK                0
#define DSP_LOCAL_LOOPBACK             1
#define DSP_BASBAND_LOOPBACK           2
#define DSP_IQ_LOOPBACK                3
#define DSP_TCH_LOOPBACK               4
#define DSP_TCH_TO_HOST_LOOPBACK       5

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPLoopbackTest constructor
**
**    PURPOSE: Initialize data members for DSPLoopbackTest object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPLoopbackTest::DSPLoopbackTest(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPLoopbackTest destructor
**
**    PURPOSE: Delete data members for DSPLoopbackTest object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPLoopbackTest::~DSPLoopbackTest()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPLoopbackTest::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPLoopbackTest::InitiateDiagnostic(void)
{
   DBG_FUNC("DSPLoopbackTest::InitiateDiagnostic", DSP_DIAGS);
   DBG_ENTER();
   int loopType;
   char testName[256];
   duration = 0;
   sscanf(parameters, "%s %u %d %u", &testName, &trx, &loopType, &duration);
   
   // Verify parameters
   if (trx > 1)
   {
      SendResultData("Invalid TRX number %u\n", trx);
      DBG_TRACE("Invalid TRX number %u\n", trx);
      DBG_LEAVE();
      return ERROR;
   }
   
   if ((loopType < DSP_LOCAL_LOOPBACK) || (loopType > DSP_TCH_TO_HOST_LOOPBACK))
   {
      SendResultData("Invalid testType number %d\n", loopType);
      DBG_TRACE("Invalid testType number %d\n", loopType);
      DBG_LEAVE();
      return ERROR;
   }
   
   SendResultData ("%s TRX(%d) loopbackType(%d) duration(%d)\n",
      testName, trx, loopType, duration);
   
   DBG_TRACE ("%s TRX(%d) loopbackType(%d) duration(%d)\n",
      testName, trx, loopType, duration);
      
   // If this is a timed test then start the timer.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Start loopback
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_SET_LOOPBACK_MODE_REQ>>8;
   buffer[length++] = MPH_DSP_SET_LOOPBACK_MODE_REQ;
   buffer[length++] = trx;
   buffer[length++] = loopType;
 
   api_SendMsgFromRmToL1(length, buffer);
   
   DBG_LEAVE();
   return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPLoopbackTest::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPLoopbackTest::TerminateDiagnostic(void)
{
	DBG_FUNC("DSPLoopbackTest::TerminateDiagnostic", DSP_DIAGS);
	DBG_ENTER();
   // If we are stopping the test for any reason and it is a timed test
   // then cancel the timer.
   if (duration) 
   {
      SetTimer(0);
   }

   // Stop loopback
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_SET_LOOPBACK_MODE_REQ>>8;
   buffer[length++] = MPH_DSP_SET_LOOPBACK_MODE_REQ;
   buffer[length++] = trx;
   buffer[length++] = 0;
 
   api_SendMsgFromRmToL1(length, buffer);
   
   SendResultData("DSPLoopback Test Complete!");
   DBG_TRACE("DSPLoopback Test Complete!");

   DBG_LEAVE();
   return OK;
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPLoopbackTest::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *DSPLoopbackTest::GetDiagnosticHelp(void)
{
	DBG_FUNC("DSPLoopbackTest::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   DBG_LEAVE();
   return (
      "DSPLoopback\n\n"
      "Enables various data loopbacks with the dsp application\n\n"
      "Parameters - TRX LoopbackType Duration\n"
      "\tTRX - TRX to perform a loopback on (0, 1)\n"
      "\tLoopbackType - The desired loopback mode\n"
      "\t\t1 : Local Loopback.\n"
      "\t\t2 : Baseband Loopback.\n"
      "\t\t3 : IQ Loopback.\n"
      "\t\t4 : TCH Loopback.\n"
      "\t\t5 : TCH to Host Loopback.\n"
      "\tDuration - If non-zero loopback operates for given number of seconds.\n\n"
      "Returns - Nothing\n"
   );
}





// *******************************************************************
// DSPMSReport
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMSReport constructor
**
**    PURPOSE: Initialize data members for DSPMSReport object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPMSReport::DSPMSReport(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMSReport destructor
**
**    PURPOSE: Delete data members for DSPMSReport object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPMSReport::~DSPMSReport()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMSReport::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPMSReport::InitiateDiagnostic(void)
{
	DBG_FUNC("DSPMSReport::InitiateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   unsigned int trx0TsMask, trx1TsMask;
   char testName[256];
   duration = 0;
   sscanf(parameters, "%s %x %x %u", &testName, &trx0TsMask, &trx1TsMask, &duration);
   
   // Verify parameters
   if (trx0TsMask > 0xff)
   {
      SendResultData("Invalid TS Mask for TRX 0 %d\n", trx0TsMask);
      DBG_TRACE("Invalid TS Mask for TRX 0 %d\n", trx0TsMask);
      DBG_LEAVE();
      return ERROR;
   }
   
   if (trx1TsMask > 0xff)
   {
      SendResultData("Invalid TS Mask for TRX 1 %d\n", trx1TsMask);
      DBG_TRACE("Invalid TS Mask for TRX 1 %d\n", trx1TsMask);
      DBG_LEAVE();
      return ERROR;
   }
   
   SendResultData ("%s TRX0TsMask(%x) TRX1TsMask(%x) duration(%d)\n",
      testName, trx0TsMask, trx1TsMask, duration);
   DBG_TRACE ("%s TRX0TsMask(%x) TRX1TsMask(%x) duration(%d)\n",
      testName, trx0TsMask, trx1TsMask, duration);
   
      
   // If this is a timed test then start the timer.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Start MS reporting
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 0;
   buffer[length++] = MS_REPORT;
   buffer[length++] = trx0TsMask;
   
   api_SendMsgFromRmToL1(length, buffer);
   
   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 1;
   buffer[length++] = MS_REPORT;
   buffer[length++] = trx1TsMask;
   
   api_SendMsgFromRmToL1(length, buffer);
   
   DBG_LEAVE();
   return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMSReport::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPMSReport::TerminateDiagnostic(void)
{
	DBG_FUNC("DSPMSReport::TerminateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   // If we are stopping the test for any reason and it is a timed test
   // then cancel the timer.
   if (duration) 
   {
      SetTimer(0);
   }

   // Stop loopback
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 0;
   buffer[length++] = MS_REPORT;
   buffer[length++] = 0;

   api_SendMsgFromRmToL1(length, buffer);

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 1;
   buffer[length++] = MS_REPORT;
   buffer[length++] = 0;

   api_SendMsgFromRmToL1(length, buffer);

   SendResultData("MSReport Test Complete!");
   DBG_TRACE("MSReport Test Complete!");
   
   DBG_LEAVE();
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMSReport::ProcessMessage
**
**    PURPOSE: Process messages not handled by Diagnostic base class.  The
**       only expected message is DIAG_MS_REPORT.
**
**    INPUT PARAMETERS: diagMsg - message sent to diagnostic
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int DSPMSReport::ProcessMessage(DiagnosticMsgType diagMsg)
{
	DBG_FUNC("DSPMSReport::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   
   switch ( diagMsg.func )
   {
      case DIAG_MS_REPORT:
      {
         float 			ber, fer;
         unsigned long	cumErrs, cumBits, cumFrameErrs, cumFrames;
         short 			power, count, ta, cmdPwr;
         short          minToa, maxToa, cumToa, avgToa;
         char           msgString[256];
         			
         unsigned char  TRX      = diagMsg.msgBody[7];
         unsigned char  timeSlot = diagMsg.msgBody[8] & 7;
         unsigned char  subCh    = (diagMsg.msgBody[8] >> 3) & 3;
         
         cumErrs = (unsigned long)(diagMsg.msgBody[10])<<24 |
                   (unsigned long)(diagMsg.msgBody[11])<<16 |
                   (unsigned long)(diagMsg.msgBody[12])<< 8 |
                   (unsigned long)(diagMsg.msgBody[13]);
         cumBits = (unsigned long)(diagMsg.msgBody[14])<<24 |
                   (unsigned long)(diagMsg.msgBody[15])<<16 |
                   (unsigned long)(diagMsg.msgBody[16])<< 8 |
                   (unsigned long)(diagMsg.msgBody[17]);
         cumFrameErrs= (unsigned long)(diagMsg.msgBody[26])<< 8 |
                       (unsigned long)(diagMsg.msgBody[27]);
         cumFrames   = (unsigned long)(diagMsg.msgBody[28])<< 8 |
                       (unsigned long)(diagMsg.msgBody[29]);
         count   = (short) (diagMsg.msgBody[18]);
         power   = (short) ( (unsigned short)(diagMsg.msgBody[19])<<8 |
                             (unsigned short)(diagMsg.msgBody[20]) );
         minToa  = (short) ((unsigned short)(diagMsg.msgBody[21])<<8) >> 8;
         maxToa  = (short) ((unsigned short)(diagMsg.msgBody[22])<<8) >> 8;
         cumToa  = (short) ((unsigned short)(diagMsg.msgBody[23])<<8) >> 8;              
         ta      = (short) (diagMsg.msgBody[24]);
         cmdPwr  = (short) (diagMsg.msgBody[25]);
         ber = (cumBits>0) ? (100.0 * float(cumErrs) / float(cumBits)) : 0.0;
         fer = (cumFrames>0) ? (100.0*float(cumFrameErrs)/float(cumFrames)) : 0.0;
         
         if ( count > 0 )
         {  /*----------- round TOA -------------------*/
            avgToa = (cumToa<0) ? (cumToa-(count>>1))/count
                                : (cumToa+(count>>1))/count;
         }
         else
         {
            avgToa = 99;
         }
         sprintf(msgString, "Trx%1d Ts%1d Sub%1d BER%6.3f%% FER%6.3f%% ",
            TRX, timeSlot, subCh, ber, fer);      
         sprintf(msgString+36, "(%6ld,%3ld) RXLEV%3d CmdPwr %02d TA%2d TOA %d,%d,%d",
            cumBits, cumFrames, power, cmdPwr, ta, minToa, avgToa, maxToa);      
         
         SendResultData("\n%s\n", msgString);
            
         DBG_TRACE("%s", msgString);                            
            
         break;
      }
   }

   DBG_LEAVE();
   return CONTINUE_TEST;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPMSReport::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *DSPMSReport::GetDiagnosticHelp(void)
{
	DBG_FUNC("DSPMSReport::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   DBG_LEAVE();
   return (
      "DSPMSReport\n\n"
      "Continuously reports BER, received power level and other parameters of\n"
      "selected active time slots on TRX0 and TRX1.\n\n"
      "Parameters - TRX0TsMask TRX1TsMask Duration\n"
      "\tTRX0TsMask - Timslot selection for TRX0 (hex) (bit 0 = TS0, bit 1 = TS1 etc.)\n"
      "\tTRX1TsMask - Timslot selection for TRX1 (hex) (bit 0 = TS0, bit 1 = TS1 etc.)\n"
      "\tDuration - If non-zero MS reporting is enabled for given number of seconds.\n\n"
      "Returns - Nothing\n"
   );
}




// *******************************************************************
// DSPGPRSMSReport
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPGPRSMSReport constructor
**
**    PURPOSE: Initialize data members for DSPGPRSMSReport object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPGPRSMSReport::DSPGPRSMSReport(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPGPRSMSReport destructor
**
**    PURPOSE: Delete data members for DSPGPRSMSReport object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPGPRSMSReport::~DSPGPRSMSReport()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPGPRSMSReport::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPGPRSMSReport::InitiateDiagnostic(void)
{
	DBG_FUNC("DSPGPRSMSReport::InitiateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   unsigned long tfiMask0, tfiMask1;
   char testName[256];
   duration = 0;
   sscanf(parameters, "%s %x %x %u", &testName, &tfiMask0, &tfiMask1, &duration);
   
   SendResultData ("%s tfiMask0(%x) tfiMask1(%x) duration(%d)\n",
      testName, tfiMask0, tfiMask1, duration);
   DBG_TRACE ("%s TfiMask(%x) tfiMask1(%x) duration(%d)\n",
      testName, tfiMask0, tfiMask1, duration);
   
      
   // If this is a timed test then start the timer.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Start GPRS MS reporting
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 0;
   buffer[length++] = GPRS_MS_REPORT;
   buffer[length++] = (tfiMask0>>24) & 0xff;
   buffer[length++] = (tfiMask0>>16) & 0xff;
   buffer[length++] = (tfiMask0>> 8) & 0xff;
   buffer[length++] = (tfiMask0    ) & 0xff;
   
   api_SendMsgFromRmToL1(length, buffer);
   
   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 1;
   buffer[length++] = GPRS_MS_REPORT;
   buffer[length++] = (tfiMask1>>24) & 0xff;
   buffer[length++] = (tfiMask1>>16) & 0xff;
   buffer[length++] = (tfiMask1>> 8) & 0xff;
   buffer[length++] = (tfiMask1    ) & 0xff;
   
   api_SendMsgFromRmToL1(length, buffer);
   
   DBG_LEAVE();
   return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPGPRSMSReport::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPGPRSMSReport::TerminateDiagnostic(void)
{
	DBG_FUNC("DSPGPRSMSReport::TerminateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   // If we are stopping the test for any reason and it is a timed test
   // then cancel the timer.
   if (duration) 
   {
      SetTimer(0);
   }

   // Stop reporting
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 0;
   buffer[length++] = GPRS_MS_REPORT;
   buffer[length++] = 0;
   buffer[length++] = 0;
   buffer[length++] = 0;
   buffer[length++] = 0;

   api_SendMsgFromRmToL1(length, buffer);

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 1;
   buffer[length++] = GPRS_MS_REPORT;
   buffer[length++] = 0;
   buffer[length++] = 0;
   buffer[length++] = 0;
   buffer[length++] = 0;

   api_SendMsgFromRmToL1(length, buffer);

   SendResultData("GPRS MSReport Test Complete!");
   DBG_TRACE("GPRS MSReport Test Complete!");
   
   DBG_LEAVE();
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPGPRSMSReport::ProcessMessage
**
**    PURPOSE: Process messages not handled by Diagnostic base class.  The
**       only expected message is DIAG_GPRS_MS_REPORT.
**
**    INPUT PARAMETERS: diagMsg - message sent to diagnostic
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int DSPGPRSMSReport::ProcessMessage(DiagnosticMsgType diagMsg)
{
	DBG_FUNC("DSPGPRSMSReport::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   
   switch ( diagMsg.func )
   {
      case DIAG_GPRS_MS_REPORT:
      {
         //float 			ber, fer;
         unsigned long	cumErrs, cumBits, cumFrameErrs, cumFrames;
         short 			power, count, ta, cmdPwr;
         short          ulcodec, dlcodec;
         short          minToa, maxToa, cumToa, avgToa;
         float          ber, bler;
         unsigned short berAv, blerAv;
         char           msgString[256];
         			
         unsigned char  TRX      = diagMsg.msgBody[7];
         unsigned char  timeSlot = diagMsg.msgBody[8] & 7;
         unsigned char  subCh    = (diagMsg.msgBody[8] >> 3) & 3;

         unsigned char  tfi = diagMsg.msgBody[18];
         
         cumErrs = (unsigned long)(diagMsg.msgBody[10])<<24 |
                   (unsigned long)(diagMsg.msgBody[11])<<16 |
                   (unsigned long)(diagMsg.msgBody[12])<< 8 |
                   (unsigned long)(diagMsg.msgBody[13]);
         cumBits = (unsigned long)(diagMsg.msgBody[14])<<24 |
                   (unsigned long)(diagMsg.msgBody[15])<<16 |
                   (unsigned long)(diagMsg.msgBody[16])<< 8 |
                   (unsigned long)(diagMsg.msgBody[17]);
         cumFrameErrs= (unsigned long)(diagMsg.msgBody[26])<< 8 |
                       (unsigned long)(diagMsg.msgBody[27]);
         cumFrames   = (unsigned long)(diagMsg.msgBody[28])<< 8 |
                       (unsigned long)(diagMsg.msgBody[29]);

         //berAv  =  (float) ((unsigned short)(diagMsg.msgBody[33] << 8) | 
          //                  (unsigned short)(diagMsg.msgBody[34]));
         //blerAv =  (float) ((unsigned short)(diagMsg.msgBody[35] << 8) | 
          //                  (unsigned short)(diagMsg.msgBody[36]));

         count   = (short) (diagMsg.msgBody[30]);
         ulcodec = (short) (diagMsg.msgBody[31]);
         dlcodec = (short) (diagMsg.msgBody[32]);

         power   = (short) ( (unsigned short)(diagMsg.msgBody[19])<<8 |
                             (unsigned short)(diagMsg.msgBody[20]) );
         minToa  = (short) ((unsigned short)(diagMsg.msgBody[21])<<8) >> 8;
         maxToa  = (short) ((unsigned short)(diagMsg.msgBody[22])<<8) >> 8;
         cumToa  = (short) ((unsigned short)(diagMsg.msgBody[23])<<8) >> 8;              
         ta      = (short) (diagMsg.msgBody[24]);
         //cmdPwr  = (short) (diagMsg.msgBody[25]);
         
         ber = (cumBits>0) ? (100.0 * float(cumErrs) / float(cumBits)) : 0.0;
         bler = (cumFrames>0) ? (100.0 * float(cumFrameErrs) / float(cumFrames)) : 0.0;
        
         if ( count > 0 )
         {  /*----------- round TOA -------------------*/
            avgToa = (cumToa<0) ? (cumToa-(count>>1))/count
                                : (cumToa+(count>>1))/count;
         }
         else
         {
            avgToa = 99;
         }

         sprintf(msgString, "Trx%1d Tfi%02d BER%6.3f%% BLR%6.3f%% ",
            TRX, tfi, ber, bler);      
         sprintf(msgString+33, "(%6ld,%3ld) RXLEV%3d TA%2d TOA%2d UR%d",
            cumBits, cumFrames, power, ta, avgToa, ulcodec);      
         
         SendResultData("\n%s\n", msgString);

         //sprintf(msgString, "Continued... AveBER%6.3f%% AveBLER%6.3f%% ",
         //   berAv, blerAv);

         //SendResultData("\n%s\n", msgString);

            
         DBG_TRACE("%s", msgString);                            
            
         break;
      }
   }

   DBG_LEAVE();
   return CONTINUE_TEST;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPGPRSMSReport::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *DSPGPRSMSReport::GetDiagnosticHelp(void)
{
	DBG_FUNC("DSPGPRSMSReport::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   DBG_LEAVE();
   return (
      "DSPGPRSMSReport\n\n"
      "Continuously reports BER, BLER, received power level and other\n"
      "parameters of selected active Temporary Block Flows on TRX0 and TRX1.\n\n"
      "Parameters - TRX0TsMask TRX1TsMask Duration\n"
      "\tTRX0TfiMask - TFI selection for TRX0 (hex) (bit 0 = TFI0, bit 1 = TFI1 etc.)\n"
      "\tTRX1TfiMask - TFI selection for TRX1 (hex) (bit 0 = TFI0, bit 1 = TFI1 etc.)\n"
      "\tDuration - If non-zero MS reporting is enabled for given number of seconds.\n\n"
      "Returns - Nothing\n"
   );
}





// *******************************************************************
// DSPHOReport
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPHOReport constructor
**
**    PURPOSE: Initialize data members for DSPHOReport object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPHOReport::DSPHOReport(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPHOReport destructor
**
**    PURPOSE: Delete data members for DSPHOReport object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPHOReport::~DSPHOReport()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPHOReport::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPHOReport::InitiateDiagnostic(void)
{
	DBG_FUNC("DSPHOReport::InitiateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   unsigned int trx0TsMask, trx1TsMask;
   char testName[256];
   duration = 0;
   sscanf(parameters, "%s %x %x %u", &testName, &trx0TsMask, &trx1TsMask, &duration);
   
   // Verify parameters
   if (trx0TsMask > 0xff)
   {
      SendResultData("Invalid TS Mask for TRX 0 %d\n", trx0TsMask);
      DBG_TRACE("Invalid TS Mask for TRX 0 %d\n", trx0TsMask);
      DBG_LEAVE();
      return ERROR;
   }
   
   if (trx1TsMask > 0xff)
   {
      SendResultData("Invalid TS Mask for TRX 1 %d\n", trx1TsMask);
      DBG_TRACE("Invalid TS Mask for TRX 1 %d\n", trx1TsMask);
      DBG_LEAVE();
      return ERROR;
   }
   
   SendResultData ("%s TRX0TsMask(%x) TRX1TsMask(%x) duration(%d)\n",
      testName, trx0TsMask, trx1TsMask, duration);
   DBG_TRACE ("%s TRX0TsMask(%x) TRX1TsMask(%x) duration(%d)\n",
      testName, trx0TsMask, trx1TsMask, duration);
   
      
   // If this is a timed test then start the timer.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Start MS reporting
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 0;
   buffer[length++] = HO_REPORT;
   buffer[length++] = trx0TsMask;
   
   api_SendMsgFromRmToL1(length, buffer);
   
   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 1;
   buffer[length++] = HO_REPORT;
   buffer[length++] = trx1TsMask;
   
   api_SendMsgFromRmToL1(length, buffer);
   
   DBG_LEAVE();
   return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPHOReport::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPHOReport::TerminateDiagnostic(void)
{
	DBG_FUNC("DSPHOReport::TerminateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   // If we are stopping the test for any reason and it is a timed test
   // then cancel the timer.
   if (duration) 
   {
      SetTimer(0);
   }

   // Stop loopback
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 0;
   buffer[length++] = HO_REPORT;
   buffer[length++] = 0;

   api_SendMsgFromRmToL1(length, buffer);

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 1;
   buffer[length++] = HO_REPORT;
   buffer[length++] = 0;

   api_SendMsgFromRmToL1(length, buffer);

   SendResultData("HOReport Test Complete!");
   DBG_TRACE("HOReport Test Complete!");
   
   DBG_LEAVE();
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPHOReport::ProcessMessage
**
**    PURPOSE: Process messages not handled by Diagnostic base class.  The
**       only expected messages are DIAG_HO_PN_REPORT and DIAG_HO_CAND_REPORT.
**
**    INPUT PARAMETERS: diagMsg - message sent to diagnostic
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int DSPHOReport::ProcessMessage(DiagnosticMsgType diagMsg)
{
	DBG_FUNC("DSPHOReport::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   
   switch ( diagMsg.func )
   {
      case DIAG_HO_PN_REPORT:
      {
         short  ul_rxlev, dl_rxlev, ul_rxqual, dl_rxqual;
         unsigned char TRX = diagMsg.msgBody[7];
         unsigned char timeSlot = diagMsg.msgBody[8] & 7;
         
         ul_rxlev = (short) ((unsigned short)(diagMsg.msgBody[12])<<8) >> 8;  
         ul_rxqual = (short) ((unsigned short)(diagMsg.msgBody[18])<<8) >> 8;  
         dl_rxlev = (short) ((unsigned short)(diagMsg.msgBody[15])<<8) >> 8;  
         dl_rxqual = (short) ((unsigned short)(diagMsg.msgBody[21])<<8) >> 8;
           
         SendResultData("\nTrx%1d Ts%1d Pnow,Pthresh,Avg ULLev:%d,%d,%d ULQual:%d,%d,%d DLLev:%d,%d,%d DLQual:%d,%d,%d ULInt:%d,%d DLInt:%d,%d\n",
            TRX, timeSlot, diagMsg.msgBody[10],diagMsg.msgBody[11],ul_rxlev,
            diagMsg.msgBody[16],diagMsg.msgBody[17],ul_rxqual,diagMsg.msgBody[13],
            diagMsg.msgBody[14],dl_rxlev,diagMsg.msgBody[19],diagMsg.msgBody[20],dl_rxqual,
            diagMsg.msgBody[22],diagMsg.msgBody[23],
            diagMsg.msgBody[24],diagMsg.msgBody[25] ); 
            
         DBG_TRACE("Trx%1d Ts%1d Pnow,Pthresh,Avg ULLev:%d,%d,%d ULQual:%d,%d,%d DLLev:%d,%d,%d DLQual:%d,%d,%d ULInt:%d,%d DLInt:%d,%d",  
            TRX, timeSlot, diagMsg.msgBody[10],diagMsg.msgBody[11],ul_rxlev,
            diagMsg.msgBody[16],diagMsg.msgBody[17],ul_rxqual,diagMsg.msgBody[13],
            diagMsg.msgBody[14],dl_rxlev,diagMsg.msgBody[19],diagMsg.msgBody[20],dl_rxqual,
            diagMsg.msgBody[22],diagMsg.msgBody[23],
            diagMsg.msgBody[24],diagMsg.msgBody[25] ); 
         break;
      }
      case DIAG_HO_CAND_REPORT:
      {
         unsigned char i;
         unsigned char TRX = diagMsg.msgBody[7];
         unsigned char timeSlot = diagMsg.msgBody[8] & 7;
         unsigned char zeroFillStart = 11 + (diagMsg.msgBody[10] * 2);
		 unsigned char zeroFillEnd   = 11 + (6 * 2);
		 
         /* zero fill for nicer format */
		 for (i = zeroFillStart; i < zeroFillEnd; i++) {
           diagMsg.msgBody[i] = 0;
         }

         SendResultData("\nTrx%1d Ts%1d NoCandidate(%o) Idx1(%o) bsic1(%o) Idx2(%o) bsic2(%o) Idx3(%o) bsic3(%o)\n"
                        "                        Idx4(%o) bsic4(%o) Idx5(%o) bsic5(%o) Idx6(%o) bsic6(%o) \n",
           TRX, timeSlot, diagMsg.msgBody[10],
           diagMsg.msgBody[11],diagMsg.msgBody[12],diagMsg.msgBody[13],diagMsg.msgBody[14],diagMsg.msgBody[15],diagMsg.msgBody[16],
           diagMsg.msgBody[17],diagMsg.msgBody[18],diagMsg.msgBody[19],diagMsg.msgBody[20],diagMsg.msgBody[21],diagMsg.msgBody[22]); 
           
         DBG_TRACE("Trx%1d Ts%1d NoCandidate(%o) Idx1(%o) bsic1(%o) Idx2(%o) bsic2(%o) Idx3(%o) bsic3(%o)\n",
           TRX, timeSlot, diagMsg.msgBody[10],
           diagMsg.msgBody[11],diagMsg.msgBody[12],diagMsg.msgBody[13],diagMsg.msgBody[14],diagMsg.msgBody[15],diagMsg.msgBody[16]);
         if ( diagMsg.msgBody[17] > 0 )
         {
            DBG_TRACE("Trx%1d Ts%1d MoreCandidates: Idx4(%o) bsic4(%o) Idx5(%o) bsic5(%o) Idx6(%o) bsic6(%o)  ",
            TRX, timeSlot,
            diagMsg.msgBody[17],diagMsg.msgBody[18],diagMsg.msgBody[19],diagMsg.msgBody[20],diagMsg.msgBody[21],diagMsg.msgBody[22]); 
         }
         break;
      }
   }
   
   DBG_LEAVE();
   return CONTINUE_TEST;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPHOReport::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *DSPHOReport::GetDiagnosticHelp(void)
{
   DBG_FUNC("DSPHOReport::GetDiagnosticHelp", DSP_DIAGS);
   DBG_ENTER();
   DBG_LEAVE();
   return (
      "DSPHOReport\n\n"
      "Continuously reports results of tests in the DSP to determine whether handover\n" 
      "is required, and BSIC and DL power of candidate neighbor cells for handover of\n"
      "selected active time slots on TRX0 and TRX1.\n\n"
      "Parameters - TRX0TsMask TRX1TsMask Duration\n"
      "\tTRX0TsMask - Timslot selection for TRX0 (hex) (bit 0 = TS0, bit 1 = TS1 etc.)\n"
      "\tTRX1TsMask - Timslot selection for TRX1 (hex) (bit 0 = TS0, bit 1 = TS1 etc.)\n"
      "\tDuration - If non-zero MS reporting is enabled for given number of seconds.\n\n"
      "Returns - Nothing\n"
   );
}




// *******************************************************************
// DSPDLReport
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPDLReport constructor
**
**    PURPOSE: Initialize data members for DSPDLReport object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPDLReport::DSPDLReport(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPDLReport destructor
**
**    PURPOSE: Delete data members for DSPDLReport object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPDLReport::~DSPDLReport()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPDLReport::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPDLReport::InitiateDiagnostic(void)
{
	DBG_FUNC("DSPDLReport::InitiateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   unsigned int trx0TsMask, trx1TsMask;
   char testName[256];
   duration = 0;
   sscanf(parameters, "%s %x %x %u", &testName, &trx0TsMask, &trx1TsMask, &duration);
   
   // Verify parameters
   if (trx0TsMask > 0xff)
   {
      SendResultData("Invalid TS Mask for TRX 0 %d\n", trx0TsMask);
      DBG_TRACE("Invalid TS Mask for TRX 0 %d\n", trx0TsMask);
      DBG_LEAVE();
      return ERROR;
   }
   
   if (trx1TsMask > 0xff)
   {
      SendResultData("Invalid TS Mask for TRX 1 %d\n", trx1TsMask);
      DBG_TRACE("Invalid TS Mask for TRX 1 %d\n", trx1TsMask);
      DBG_LEAVE();
      return ERROR;
   }
   
   SendResultData ("%s TRX0TsMask(%x) TRX1TsMask(%x) duration(%d)\n",
      testName, trx0TsMask, trx1TsMask, duration);
   DBG_TRACE ("%s TRX0TsMask(%x) TRX1TsMask(%x) duration(%d)\n",
      testName, trx0TsMask, trx1TsMask, duration);
   
      
   // If this is a timed test then start the timer.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Start MS reporting
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 0;
   buffer[length++] = DL_MEAS_REPORT;
   buffer[length++] = trx0TsMask;
   
   api_SendMsgFromRmToL1(length, buffer);
   
   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 1;
   buffer[length++] = DL_MEAS_REPORT;
   buffer[length++] = trx1TsMask;
   
   api_SendMsgFromRmToL1(length, buffer);
   
   DBG_LEAVE();
   return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPDLReport::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int DSPDLReport::TerminateDiagnostic(void)
{
	DBG_FUNC("DSPDLReport::TerminateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   // If we are stopping the test for any reason and it is a timed test
   // then cancel the timer.
   if (duration) 
   {
      SetTimer(0);
   }

   // Stop loopback
   unsigned short    length;	   
   unsigned char     buffer[40];

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 0;
   buffer[length++] = DL_MEAS_REPORT;
   buffer[length++] = 0;

   api_SendMsgFromRmToL1(length, buffer);

   length = 0;

   buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
   buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
   buffer[length++] = 1;
   buffer[length++] = DL_MEAS_REPORT;
   buffer[length++] = 0;

   api_SendMsgFromRmToL1(length, buffer);

   SendResultData("DLReport Test Complete!");
   DBG_TRACE("DLReport Test Complete!");
   
   DBG_LEAVE();
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPDLReport::ProcessMessage
**
**    PURPOSE: Process messages not handled by Diagnostic base class.  The
**       only expected message is DIAG_DL_REPORT.
**
**    INPUT PARAMETERS: diagMsg - message sent to diagnostic
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int DSPDLReport::ProcessMessage(DiagnosticMsgType diagMsg)
{
	DBG_FUNC("DSPDLReport::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();

   static unsigned char msgCount = 0; 
   msgCount++;

   switch ( diagMsg.func )
   {
      case DIAG_DL_REPORT:
      {
         /*
         *  Supress printouts from every 0.5 secs to every 3 secs (i.e. msgCount > 2 for 1 sec)
         */
         if (msgCount > 6) {
           unsigned char i=0;
           unsigned char TRX = diagMsg.msgBody[7];
           unsigned char timeSlot = diagMsg.msgBody[8] & 7;
         
           /*
           *  Clean up report by zeroing non-valid fields
           */
           if (diagMsg.msgBody[14] == 0) 
              i=15;
              
           else if (diagMsg.msgBody[14] == 1)
              i=20;
      
           else if (diagMsg.msgBody[14] == 2)
              i=25;
       
           else if (diagMsg.msgBody[14] == 3)
              i=30;
      
           else if (diagMsg.msgBody[14] == 4)
              i=35;
        
           else if (diagMsg.msgBody[14] == 5)
              i=40;
      
           else
              i=45;              
           

           while (i < 45) {
             diagMsg.msgBody[i] = 0;
             i++;
           } 


           /*
           *  Serving Cell information
           */
           SendResultData("\nTrx%1d Ts%1d: DL RxLevFull(%d) Sub(%d)"
               " RxQualFull(%d) Sub(%d) NO-NCELL(%d)\n",
               TRX, timeSlot,  diagMsg.msgBody[10], diagMsg.msgBody[11], 
               diagMsg.msgBody[12], diagMsg.msgBody[13], diagMsg.msgBody[14]); 

           /*
           *  Neighbor information
           */
           SendResultData("\nTrx%1d Ts%1d:\n"
                     "BcchFrqNCell-1(%d) BsicNCell-1(%o) IndxNCell-1(%d) RxLevNCell-1(%d)\n"
                     "BcchFrqNCell-2(%d) BsicNCell-2(%o) IndxNCell-2(%d) RxLevNCell-2(%d)\n"
                     "BcchFrqNCell-3(%d) BsicNCell-3(%o) IndxNCell-3(%d) RxLevNCell-3(%d)\n"
                     "BcchFrqNCell-4(%d) BsicNCell-4(%o) IndxNCell-4(%d) RxLevNCell-4(%d)\n"
                     "BcchFrqNCell-5(%d) BsicNCell-5(%o) IndxNCell-5(%d) RxLevNCell-5(%d)\n"
                     "BcchFrqNCell-6(%d) BsicNCell-6(%o) IndxNCell-6(%d) RxLevNCell-6(%d)\n",
               TRX, timeSlot,  (diagMsg.msgBody[15]<<8)+diagMsg.msgBody[16], 
               diagMsg.msgBody[17], diagMsg.msgBody[18], diagMsg.msgBody[19],
                               (diagMsg.msgBody[20]<<8)+diagMsg.msgBody[21], 
               diagMsg.msgBody[22], diagMsg.msgBody[23], diagMsg.msgBody[24],
                               (diagMsg.msgBody[25]<<8)+diagMsg.msgBody[26], 
               diagMsg.msgBody[27], diagMsg.msgBody[28], diagMsg.msgBody[29],
                               (diagMsg.msgBody[30]<<8)+diagMsg.msgBody[31], 
               diagMsg.msgBody[32], diagMsg.msgBody[33], diagMsg.msgBody[34],
                               (diagMsg.msgBody[35]<<8)+diagMsg.msgBody[36], 
               diagMsg.msgBody[37], diagMsg.msgBody[38], diagMsg.msgBody[39],
                               (diagMsg.msgBody[40]<<8)+diagMsg.msgBody[41], 
               diagMsg.msgBody[42], diagMsg.msgBody[43], diagMsg.msgBody[44]); 
               

           //---------------------------------- DEBUG TRACES
           /*
           *  Serving Cell information
           */
           DBG_TRACE("Trx%1d Ts%1d: DL RxLevFull(%d) Sub(%d) RxQualFull(%d) Sub(%d) NO-NCELL(%d)",
               TRX, timeSlot,  diagMsg.msgBody[10], diagMsg.msgBody[11], 
               diagMsg.msgBody[12], diagMsg.msgBody[13], diagMsg.msgBody[14]); 

           /*
           *  Neighbor information      
           */
           DBG_TRACE("\nTrx%1d Ts%1d:\n"
                     "BcchFrqNCell-1(%d) BsicNCell-1(%o) IndxNCell-1(%d) RxLevNCell-1(%d)\n"
                     "BcchFrqNCell-2(%d) BsicNCell-2(%o) IndxNCell-2(%d) RxLevNCell-2(%d)\n"
                     "BcchFrqNCell-3(%d) BsicNCell-3(%o) IndxNCell-3(%d) RxLevNCell-3(%d)\n"
                     "BcchFrqNCell-4(%d) BsicNCell-4(%o) IndxNCell-4(%d) RxLevNCell-4(%d)\n"
                     "BcchFrqNCell-5(%d) BsicNCell-5(%o) IndxNCell-5(%d) RxLevNCell-5(%d)\n"
                     "BcchFrqNCell-6(%d) BsicNCell-6(%o) IndxNCell-6(%d) RxLevNCell-6(%d)\n",
               TRX, timeSlot,  (diagMsg.msgBody[15]<<8)+diagMsg.msgBody[16], 
               diagMsg.msgBody[17], diagMsg.msgBody[18], diagMsg.msgBody[19],
                               (diagMsg.msgBody[20]<<8)+diagMsg.msgBody[21], 
               diagMsg.msgBody[22], diagMsg.msgBody[23], diagMsg.msgBody[24],
                               (diagMsg.msgBody[25]<<8)+diagMsg.msgBody[26], 
               diagMsg.msgBody[27], diagMsg.msgBody[28], diagMsg.msgBody[29],
                               (diagMsg.msgBody[30]<<8)+diagMsg.msgBody[31], 
               diagMsg.msgBody[32], diagMsg.msgBody[33], diagMsg.msgBody[34],
                               (diagMsg.msgBody[35]<<8)+diagMsg.msgBody[36], 
               diagMsg.msgBody[37], diagMsg.msgBody[38], diagMsg.msgBody[39],
                               (diagMsg.msgBody[40]<<8)+diagMsg.msgBody[41], 
               diagMsg.msgBody[42], diagMsg.msgBody[43], diagMsg.msgBody[44]); 

           /*
           *  Reset counter
           */
           msgCount = 0;
         }

         break;
      }
   }

   
   DBG_LEAVE();
   return CONTINUE_TEST;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPDLReport::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *DSPDLReport::GetDiagnosticHelp(void)
{
	DBG_FUNC("DSPDLReport::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   DBG_LEAVE();
   return (
      "DSPDLReport\n\n"
      "Continuously reports the downlink power level and quality as measured by the MS of\n"
      "selected active time slots on TRX0 and TRX1.\n\n"
      "Parameters - TRX0TsMask TRX1TsMask Duration\n"
      "\tTRX0TsMask - Timslot selection for TRX0 (hex) (bit 0 = TS0, bit 1 = TS1 etc.)\n"
      "\tTRX1TsMask - Timslot selection for TRX1 (hex) (bit 0 = TS0, bit 1 = TS1 etc.)\n"
      "\tDuration - If non-zero MS reporting is enabled for given number of seconds.\n\n"
      "Returns - Nothing\n"
   );
}





// *******************************************************************
// HPIEchoTest
// *******************************************************************

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIEchoTest constructor
**
**    PURPOSE: Initialize data members for HPIEchoTest object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
HPIEchoTest::HPIEchoTest(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIEchoTest destructor
**
**    PURPOSE: Delete data members for HPIEchoTest object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
HPIEchoTest::~HPIEchoTest()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIEchoTest::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int HPIEchoTest::InitiateDiagnostic(void)
{
	DBG_FUNC("HPIEchoTest::InitiateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   char testName[256];
   duration = verbose = lockTask = 0;
   sscanf(parameters, "%s %d %d %d %d %u %d %d", &testName, &testDsp0, &testDsp1, 
      &numMsgs, &delay, &duration, &lockTask, &verbose);
      
   // Verify parameters
   if (numMsgs > 10)
   {
      SendResultData("Invalid number of messages in burst %d\n", numMsgs);
      DBG_TRACE("Invalid number of messages in burst %d\n", numMsgs);
      DBG_LEAVE();
      return ERROR;
   }
      
   SendResultData ("%s testDsp0(%d) testDsp1(%d) numMsgs(%d) delay(%d) duration(%d) lockTask(%d) verbose(%d)\n",
      testName, testDsp0, testDsp1, numMsgs, delay, duration, lockTask, verbose);
   DBG_TRACE ("%s testDsp0(%d) testDsp1(%d) numMsgs(%d) delay(%d) duration(%d) lockTask(%d) verbose(%d)\n",
      testName, testDsp0, testDsp1, numMsgs, delay, duration, lockTask, verbose);   
      
   // Allocate some memory to hold messages while they are pending dsp
   // responses.
   for (int i = 0; i < MAX_MSG_BURSTS; i++)
   {
      if (testDsp0)
      {
         dspMsgs[0][i] = new EchoMsgStruct[numMsgs]; 
         for (int j = 0; j < numMsgs; j++)
            dspMsgs[0][i][j].msgState = UNALLOCATED;  
      }
      
      if (testDsp1)
      {
         dspMsgs[1][i] = new EchoMsgStruct[numMsgs];   
         for (int j = 0; j < numMsgs; j++)
            dspMsgs[1][i][j].msgState = UNALLOCATED;  
      }
   }
   
   // Set the current burst block to the first block.
   curMsgBurst[0] = curMsgBurst[1] = 0;
      
   totalErrs[0] = totalNoRsp[0] = totalMsgs[0] = 0;
   totalErrs[1] = totalNoRsp[1] = totalMsgs[1] = 0;
   totalUnkownDspErrs = 0;
   
   // If this is a timed test then start the timer.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Send a message to start sending data.
   DiagnosticMsgType diagMsg;

   // Initialize the diagnostic message.
   diagMsg.func = HPI_ECHO_TEST_CONTINUE;

   msgQSend(diagQId, 
            (char *)&diagMsg, 
            sizeof(DiagnosticMsgType), 
            WAIT_FOREVER, 
            MSG_PRI_NORMAL);
   
   DBG_LEAVE();
   return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIEchoTest::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int HPIEchoTest::TerminateDiagnostic(void)
{
	DBG_FUNC("HPIEchoTest::TerminateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

   // If we are stopping the test for any reason and it is a timed test
   // then cancel the timer.
   if (duration) 
   {
      SetTimer(0);
   }

   SendResultData("HPIEchoTest Complete!\n");
   DBG_TRACE("HPIEchoTest Complete!\n");
   if (testDsp0)
   {
      SendResultData("HPIEchoTest: DSP 0 total msgs (%d) error msgs(%d) no response msgs(%d)\n", 
         totalMsgs[0], totalErrs[0], totalNoRsp[0]);
      DBG_TRACE("HPIEchoTest: DSP 0 total msgs (%d) error msgs(%d) no response msgs(%d)\n", 
         totalMsgs[0], totalErrs[0], totalNoRsp[0]);
   }
   
   if (testDsp1)
   {
      SendResultData("HPIEchoTest: DSP 1 total msgs (%d) error msgs(%d) no response msgs(%d)\n", 
         totalMsgs[1], totalErrs[1], totalNoRsp[1]);
      DBG_TRACE("HPIEchoTest: DSP 0 total msgs (%d) error msgs(%d) no response msgs(%d)\n", 
         totalMsgs[1], totalErrs[1], totalNoRsp[1]);
   }
   
   SendResultData("HPIEchoTest: Errors where DSP could not be determied(%d)\n", 
      totalUnkownDspErrs);
   DBG_TRACE("HPIEchoTest: Errors where DSP could not be determied(%d)\n", 
      totalUnkownDspErrs);
   
   // Free the memory used for saving messages.
   for (int i = 0; i < MAX_MSG_BURSTS; i++)
   {
      if (testDsp0)
      {
          delete [] dspMsgs[0][i]; 
      }
      
      if (testDsp1)
      {
          delete [] dspMsgs[1][i]; 
      }
   }
   
   DBG_LEAVE();
   return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIEchoTest::BuildEchoMsg
**
**    PURPOSE: Build up a burst of echo messages for a specified dsp.  Each
**       message contains a response message in the payload.  The burst of
**       messages will be saved for verification when the response is returned.
**       The burst of messages is kept in a circular buffer.  If the messages
**       in the next buffer have not received a response then an error is
**       flagged.
**
**    INPUT PARAMETERS: dspNum - dsp to test (0,1)
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
void HPIEchoTest::BuildEchoMsg(int dspNum)
{
	DBG_FUNC("HPIEchoTest::BuildEchoMsg", DSP_DIAGS);
	DBG_ENTER();
   
   // Move to the next burst of messages.
   curMsgBurst[dspNum] = (curMsgBurst[dspNum] + 1) % MAX_MSG_BURSTS;
   
   for (int i = 0; i < numMsgs; i++)
   {
      // First make sure that this message has received a response.
      // If there is no response then this is considered to be the
      // timeout so flag an error.
      if (dspMsgs[dspNum][curMsgBurst[dspNum]][i].msgState != UNALLOCATED)
      {
         totalNoRsp[dspNum]++;
         if (verbose)
         {
            SendResultData("HPIEchoTest: ERROR: No response received!\n");
            DBG_TRACE("HPIEchoTest: ERROR: No response received!\n");
         }
         dspMsgs[dspNum][curMsgBurst[dspNum]][i].msgState = UNALLOCATED;
      }
      
      int curByte = 0;
      
      // The echo message is constructed as follows:
      //
      //        0            4              8            c
      // +-------------------------------------------------------+
      // | Msg Discrm. | Msg Type(H) | Msg Type(L) |   dspNum    | 0x0
      // +-------------------------------------------------------+
      // |    Length of response message                         | 0x4
      // +-------------------------------------------------------+
      // | Msg Discrm. | Msg Type(H) | Msg Type(L) |   dspNum    | 0x8
      // | (response)  | (response)  | (response)  | (response)  |
      // +-------------------------------------------------------+
      // |  Burst of messages index                              | 0xc
      // +-------------------------------------------------------+
      // |  Message in burst index                               | 0x10
      // +-------------------------------------------------------+
      // |  Rand number                                          | 0x14
      // +-------------------------------------------------------+
      // |                         .                             |  .
      // |                         .                             |  .
      // |                         .                             |  .
      // +-------------------------------------------------------+
      // |  Rand number                                          | 0x38
      // +-------------------------------------------------------+
      
      // Build Echo message header.
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         DIAGNOSTIC_MSG;
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         MPH_DSP_DIAG_ECHO_MSG_REQ>>8;
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         MPH_DSP_DIAG_ECHO_MSG_REQ;
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         (unsigned char)dspNum;
      
      // Now build the payload.  The first two words of the payload
      // are the header for the response.
      int len = MAX_PARAMS;
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         (unsigned char) len;
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         (unsigned char) (len>>8);
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         (unsigned char) (len>>16);
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         (unsigned char) (len>>24);
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         DIAGNOSTIC_MSG;
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         MPH_DSP_DIAG_ECHO_MSG_RSP>>8;
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
         MPH_DSP_DIAG_ECHO_MSG_RSP;
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = dspNum;
      
      // Next put the message burst index and the message index in the
      // payload.  This will be used to verify the response.
      int *pInt;
      pInt = (int *)&dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte]; 
      *pInt =  curMsgBurst[dspNum];
      curByte += sizeof(int);
      
      pInt = (int *)&dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte]; 
      *pInt = i;
      curByte += sizeof(int);
      
      // Finally fill the rest of the message with random data.
      for (int j = 0; j < MAX_PARAMS-16; j+=4)
      {
         int randNum =  rand();
         dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
            (unsigned char) randNum;
         dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
            (unsigned char) (randNum>>8);
         dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
            (unsigned char) (randNum>>16);
         dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg[curByte++] = 
            (unsigned char) (randNum>>24);
      }  
      
      dspMsgs[dspNum][curMsgBurst[dspNum]][i].msgState = RESPONSE_PENDING;
      
      totalMsgs[dspNum]++;
      api_SendMsgFromRmToL1(curByte, dspMsgs[dspNum][curMsgBurst[dspNum]][i].msg);
       
   }
   
   DBG_LEAVE();
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIEchoTest::ProcessMessage
**
**    PURPOSE: Process messages not handled by Diagnostic base class.  The
**       only expected messages are HPI_ECHO_TEST_CONTINUE and 
**       DIAG_DSP_ECHO_MSG.
**
**    INPUT PARAMETERS: diagMsg - message sent to diagnostic
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int HPIEchoTest::ProcessMessage(DiagnosticMsgType diagMsg)
{
	DBG_FUNC("HPIEchoTest::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   
   switch ( diagMsg.func )
   {
      case HPI_ECHO_TEST_CONTINUE:
      {
         // Build and send a burst of echo messages.
         if (testDsp0)
         {
            if (lockTask)
            {
               taskLock();
            }
            
            BuildEchoMsg(0);
            
            if (lockTask)
            {
               taskUnlock();
            }
         }
         
         if (testDsp1)
         {
            if (lockTask)
            {
               taskLock();
            }
            
            BuildEchoMsg(1);
            
            if (lockTask)
            {
               taskUnlock();
            }
         }
         
         // Delay here before sending any more messages.
         taskDelay(delay);
         
         // Send a message to start sending data.
         DiagnosticMsgType contMsg;

         // Initialize the diagnostic message.
         contMsg.func = HPI_ECHO_TEST_CONTINUE;

         msgQSend(diagQId, 
                  (char *)&contMsg, 
                  sizeof(DiagnosticMsgType), 
                  NO_WAIT, 
                  MSG_PRI_NORMAL);
   
         break;
      }
      case DIAG_DSP_ECHO_MSG:
      {
         int len, burstIndx, msgIndx, dsp;
         unsigned char *pLen = (unsigned char *)&len;
         
         // Grab the length, dsp number, msg burst index and msg index.
         pLen[0] = diagMsg.msgBody[3];
         pLen[1] = diagMsg.msgBody[2];
         pLen[2] = diagMsg.msgBody[1];
         pLen[3] = diagMsg.msgBody[0];
         
         dsp = (int)diagMsg.msgBody[7];
         
         burstIndx = *((int *)(diagMsg.msgBody + 8));
         msgIndx = *((int *)(diagMsg.msgBody + 0xc));
         
         // First make sure the length makes sense.
         if (len != MAX_PARAMS)
         {
            if (verbose)
            {
               SendResultData("HPIEchoTest: ERROR: Invalid length in msg!\n");
               SendResultData("Sent DSP %d len(%d) received len(%d)\n", 
                  dsp, MAX_PARAMS, len);
            }
            totalUnkownDspErrs++;
         }
         else
         {
            // Verify dsp number, burst index and message index.
            if ((burstIndx >= 0) && (burstIndx < MAX_MSG_BURSTS) && 
               (msgIndx >= 0) && (msgIndx < numMsgs) &&
               (dsp >= 0) && (dsp < 2))
            {
               // Verify all the data received.  Must skip the first for bytes
               // in the saved message since the saved message has the original
               // message header in it too.
               for (int i = 0; i < MAX_PARAMS-8; i++)
               {
                  if (diagMsg.msgBody[i] != 
                     dspMsgs[dsp][burstIndx][msgIndx].msg[i+4])
                  {
                     if (verbose)
                     {
                        SendResultData("HPIEchoTest: ERROR: Invalid data in msg!\n");
                        SendResultData("Sent DSP %d the following message:\n", dsp);
                        for (int j = 0; j < HPI_MSG_SIZE-8; j+=4)
                        {
                           if ((j % 20) || (j == 0))
                           {
                              SendResultData("%010#x ", 
                                 *((unsigned int *)(dspMsgs[dsp][burstIndx][msgIndx].msg + j + 4)));    
                           }
                           else
                           {
                              SendResultData("%010#x\n",
                                 *((unsigned int *)(dspMsgs[dsp][burstIndx][msgIndx].msg + j + 4)));    
                           }
                        }
                        
                        SendResultData("\nReceived from DSP %d the following message:\n", dsp);
                        for (j = 0; j < HPI_MSG_SIZE-8; j+=4)
                        {
                           if ((j % 20) || (j == 0))
                           {
                              SendResultData("%010#x ", *((unsigned int *)(diagMsg.msgBody + j)));    
                           }
                           else
                           {
                              SendResultData("%010#x\n", *((unsigned int *)(diagMsg.msgBody + j)));    
                           }
                        }
                     }
                     totalErrs[dsp]++;
                     break;
                  }
               }
               
               // If the indexes were in range then clear this message.
               dspMsgs[dsp][burstIndx][msgIndx].msgState = UNALLOCATED;
            }
            else
            {
               if (verbose)
               {
                  SendResultData("HPIEchoTest: ERROR: Invalid params in msg!\n");
                  SendResultData("Received dsp(%d) burstIndx(%d) msgIndx(%d)\n",
                     dsp, burstIndx, msgIndx);
               }
               totalUnkownDspErrs++;
            }
         }
            
         break;
      }
   }
   
   DBG_LEAVE();
   return CONTINUE_TEST;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: HPIEchoTest::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *HPIEchoTest::GetDiagnosticHelp(void)
{
	DBG_FUNC("HPIEchoTest::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   DBG_LEAVE();
   return (
      "HPIEchoTest\n\n"
      "This is an interactive test of the host port interface.  Echo messages\n"
      "will be sent in bursts to either or both dsp 0 and 1.  Up to 10\n"
      "messages per burst may be sent with a specified delay between each\n"
      "burst.\n\n"
      "Parameters - testDsp0 testDsp1 numMsgs delay duration taskLock verbose\n"
      "\ttestDsp0 - if non-zero DSP 0 will be tested\n"
      "\ttestDsp1 - if non-zero DSP 1 will be tested\n"
      "\tnumMsgs - number of msgs to send in a single burst (max 10)\n"
      "\tdelay - number of ticks to wait between sending bursts of msgs\n"
      "\tduration - If non-zero test will run for specified time otherwise\n"
      "\t           test runs until stopped (time in seconds)\n"
      "\ttaskLock - If non-zero task switching will be locked when messages are\n"
      "\t           sent to insure multiple messages sent to the dsps.\n"
      "\tverbose - If non-zero error messages will be displayed as they occur\n\n"
      "Returns - total msgs sent, total number of msgs with errors, total no response msgs\n"
   );
}


// *******************************************************************
// DSPBBLoopbackTest
// *******************************************************************

//------------------------------------------------------------------------------
DSPBBLoopbackTest::DSPBBLoopbackTest(char *testName, int testNum, 
                                     ReqSourceType reqSrc, char *parms)
                  : Diagnostic(testName, testNum, reqSrc, parms)
{
}

//------------------------------------------------------------------------------
DSPBBLoopbackTest::~DSPBBLoopbackTest()
{
}

//------------------------------------------------------------------------------
int DSPBBLoopbackTest::InitiateDiagnostic(void)
{
  char testName[100];

  verbose                = -1;
  dspbblbtest_verbose    = FALSE;
  DSPBBLBTest_Cmd_Status = OK;

  sscanf(parameters, "%s %d %d %d %d %d", &testName, &src, &dst, &index, &shift, &verbose);

  if ((src < 0) || (src > 1))
  {
    SendResultData("Invalid dsp number for tone generation [%d]\n", src);
    return ERROR;
  }

  if ((dst < 0) || (dst > 1))
  {
    SendResultData("Invalid dsp number for measurement [%d]\n", dst);
    return ERROR;
  }

  if ((index < 0) || (index > 7))
  {
    SendResultData("Invalid frequency index [%d]\n", index);
    return ERROR;
  }

  if ((shift < 0) || (shift > 7))
  {
    SendResultData("Invalid gain shift [%d]\n", shift);
    return ERROR;
  }

  if ( (verbose > TRUE) || (verbose < FALSE) )
  {
    SendResultData("verbose value is out of range [%d]\n", verbose);
    return ERROR;
  }
  else
      dspbblbtest_verbose = verbose;

  if ( dspbblbtest_verbose == TRUE )
      SendResultData("%s src(%d) dst(%d) index(%d) shift(%d) verbose(%d)\n", 
                      testName, src, dst, index, shift, verbose);

  DSPBBLBTest_Cmd_Status = DSP_BASBAND_LOOPBACK;
  dspBBLoopbackTest(src, dst, index, shift, verbose);
  SetTimer(8);

  return OK;
}

//------------------------------------------------------------------------------
int DSPBBLoopbackTest::TerminateDiagnostic(void)
{
  DBG_FUNC("DSP Baseband Loopback Test::TerminateDiagnostic", DSP_DIAGS);
  DBG_ENTER();  

  extern int dspTestInProgress;
 
  if (dspTestInProgress == TRUE || DSPBBLBTest_Cmd_Status == DSP_BASBAND_LOOPBACK )
  {
    SendResultData("DSP Baseband Loopback Test: FAILED - DSP Response problem.\n");
    dspTestInProgress = FALSE;
    DSPBBLBTest_Cmd_Status = ERROR;
  }
  else if (dspTestInProgress == FALSE || DSPBBLBTest_Cmd_Status != DSP_BASBAND_LOOPBACK ) 
    SendResultData("DSP Baseband Loopback Test: COMPLETE\n");

  return OK;
}

//------------------------------------------------------------------------------
int DSPBBLoopbackTest::ProcessMessage       // RETURN: CONTINUE_TEST/STOP_TEST
(
  DiagnosticMsgType diagMsg    //IN: Message
)
{
  DBG_FUNC("DSP Baseband Loopback Test::ProcessMessage", DSP_DIAGS);
  DBG_ENTER();
  DBG_LEAVE();

  if (diagMsg.func == DSP_TONE_MEASURE_RSP)
  {
      if ( dspbblbtest_verbose == TRUE )
          SendResultData("dsp_Tone_Measure_Rsp Received\n");

      dspToneMeasureRsp((unsigned char*)&diagMsg.msgBody[8], dspbblbtest_verbose);
      return STOP_TEST;
  }
  else if (diagMsg.func == DSP_TONE_GENERATE_RSP)
  {
      if ( dspbblbtest_verbose == TRUE )
              SendResultData("dsp_Tone_Generate_Rsp Received\n");

      dspBBLoopbackTest2(dspbblbtest_verbose);
      DSPBBLBTest_Cmd_Status = OK;
  }

  return CONTINUE_TEST;
}

//------------------------------------------------------------------------------
char* DSPBBLoopbackTest::GetDiagnosticHelp(void)
{
  DBG_FUNC("DSP Baseband Loopback Test::GetDiagnosticHelp", DSP_DIAGS);
  DBG_ENTER();
  DBG_LEAVE();

  return
  (    
     "DSPBBLoopbackTest - DSP Baseband Loopback Test\n\n"
     "----------------------------------------------------------------------\n"
     "                     FOR CISCO SYSTEMS USE ONLY\n"
     "----------------------------------------------------------------------\n\n"
     "CAUTION: This test is INTRUSIVE. It does not allow normal ViperCell operation.\n"
     "This can not be run from normal operating configuration. This test\n"
     "requires baseband test fixture board. The Vipercell must be booted\n" 
     "with the DSPTest startup script. However, it can be run before\n"
     "or after the DSPExtMemTest.\n"
     "----------------------------------------------------------------------\n\n"
     "This test requires baseband test fixture board.\n"
     "The test makes a DSP generate tone at one out of eight possible\n"
     "frequencies and makes same or different DSP mesure the signal strength\n"
     "at eight possible frequencies.\n"
     "The result of the test is eight values mesured by the DSP.\n\n"
     "Parameters:\n\n"
     "src      - [0:1] dsp generating tone\n"
     "dst      - [0:1] dsp measuring signal strength\n"
     "index    - [0:7] frequency index\n"
     "shift    - [0:7] gain shift, 0 = no right bit shifts\n\n"
     "verbose  - [0] = Only final result is displayed.\n"
     "           [1] = All error messages are displayed. In addition if all\n"
     "                 tests are requested, results of individul tests are displayed.\n\n"
     "----------------------------------------------------------------------\n"
     "                     FOR CISCO SYSTEMS USE ONLY\n"
     "----------------------------------------------------------------------\n"
  );
}


// *******************************************************************
// DSPExtMemTest
// *******************************************************************

//------------------------------------------------------------------------------
DSPExtMemTest::DSPExtMemTest(char *testName, int testNum, 
                             ReqSourceType reqSrc, char *parms)
              : Diagnostic(testName, testNum, reqSrc, parms)
{
}

//------------------------------------------------------------------------------
DSPExtMemTest::~DSPExtMemTest()
{
}

//------------------------------------------------------------------------------
int DSPExtMemTest::InitiateDiagnostic(void)
{
  char testName[100];
 
  numErrors             = 0;
  dspResponds           = FALSE;
  verbose               = -1;
  dspextmemtest_verbose = FALSE;

  sscanf(parameters, "%s %d %d %d", &testName, &dspNum, &testTypeReq, &verbose);
  if ((dspNum < 0) || (dspNum > 1))
  {
    SendResultData("Invalid DSP Number [%d]\n", dspNum);
    return ERROR;
  }

  if ((testTypeReq < 0) || (testTypeReq > MaxTestType))
  {
    SendResultData("Invalid Test Type Requested [%d]\n", testTypeReq);
    return ERROR;
  }

  if ( (verbose > TRUE) || (verbose < FALSE) )
  {
    SendResultData("verbose value is out of range [%d]\n", verbose);
    return ERROR;
  }
  else
      dspextmemtest_verbose = verbose;

  if ( dspextmemtest_verbose == TRUE )
      SendResultData("%s dspNum(%d) testTypeReq(%d) verbose(%d) \n", 
                      testName, dspNum, testTypeReq, verbose);
  
  verbose = verbose ? TRUE : FALSE;
  testType = testTypeReq ? testTypeReq : 1; /* if requested test is 0, start with the first */

  dspExtMemTest(dspNum, testType - 1, dspextmemtest_verbose);
  SetTimer(10);

  return OK;
}

//------------------------------------------------------------------------------
int DSPExtMemTest::TerminateDiagnostic(void)
{
  extern int dspTestInProgress;

  if (dspResponds == FALSE)
  {
    SendResultData("DSP External Memory Test: FAIL; DSP does not respond\n");
  }
  dspTestInProgress = FALSE;

  return OK;
}

//------------------------------------------------------------------------------
int DSPExtMemTest::ProcessMessage       // RETUEN: CONTINUE_TEST/STOP_TEST
(
  DiagnosticMsgType diagMsg    //IN: Message
)
{
  if (diagMsg.func == DSP_EXTMEM_TEST_RSP)
  {
    dspResponds = TRUE;
    dspExtMemTestRsp((unsigned char*)&diagMsg.msgBody[8]);

    if ((testTypeReq != 0) || (verbose == TRUE))
    {    /* print the result */
      if (numErrors == 0)
        SendResultData("DSP %d External Memory Test: PASS\n", dspNum);
      else
        SendResultData("DSP %d External Memory Test: FAIL; %d Errors\n", dspNum, numErrors);
    }

    if (testTypeReq != 0)
      return STOP_TEST;
    else
    {
      if (testType == MaxTestType)
      {   /* all tests are over */
        if (verbose != TRUE)
        {    /* print result summary */
          if (numErrors == 0)
            SendResultData("DSP %d External Memory Test: PASS\n", dspNum);
          else
            SendResultData("DSP %d External Memory Test: FAIL; %d Errors\n", dspNum, numErrors);
        }
        return STOP_TEST;
      }
      else  /* goto the next test */
        dspExtMemTest(dspNum, ++testType - 1, dspextmemtest_verbose);        
    }  
  }
  else if (diagMsg.func == DSP_EXTMEM_ERROR_RSP)
  {
    dspResponds = TRUE;
    if (verbose == TRUE)
      dspExtMemErrorRsp((unsigned char*)&diagMsg.msgBody[8]);
  }
  
  return CONTINUE_TEST;
}

//------------------------------------------------------------------------------
char* DSPExtMemTest::GetDiagnosticHelp(void)
{
  DBG_FUNC("DSP External Memory Test::GetDiagnosticHelp", DSP_DIAGS);
  DBG_ENTER();
  DBG_LEAVE();

  return
  (
     "DSPExtMemTest - DSP External Memory Test\n\n"
     "----------------------------------------------------------------------\n"
     "                     FOR CISCO SYSTEMS USE ONLY\n"
     "----------------------------------------------------------------------\n\n"
     "----------------------------------------------------------------------\n"
     "CAUTION: This test is INTRUSIVE. It does not allow normal ViperCell operation.\n\n"              
     "This can not be run from normal operating configuration. The Vipercell\n"
     "must be booted with the DSPTest startup script. However,\n" 
     "it can be run before or after the DSPBBLoopbackTest.\n"
     "----------------------------------------------------------------------\n\n"
     "This test requires baseband test fixture board.\n\n"
     "Parameters:\n\n"
     "dspNum      - [0:1]  dsp to test\n\n"
     "testTypeReq - Test Type Requested [0-5]\n"
     "              0 - All\n"
     "              1 - Walk Zeros\n"
     "              2 - Walk Ones\n"
     "              3 - Increament\n"
     "              4 - Increament and complement\n"
     "              5 - Random Pattern\n\n"
     "verbose     - [0] = Only final result is displayed.\n"
     "              [1] = All error messages are displayed. In addition if all\n"
     "              tests are requested, results of individul tests are displayed.\n\n"
     "----------------------------------------------------------------------\n"
     "                     FOR CISCO SYSTEMS USE ONLY\n"
     "----------------------------------------------------------------------\n"
  );
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPIFReport constructor
**
**    PURPOSE: Initialize data members for DSPIFReport object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPIFReport::DSPIFReport(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPIFReport destructor
**
**    PURPOSE: Delete data members for DSPIFReport object.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
DSPIFReport::~DSPIFReport()
{
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPIFReport::InitiateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - OK if able to Initiate or ERROR if not
**
**----------------------------------------------------------------------------*/
int DSPIFReport::InitiateDiagnostic(void)
{
	DBG_FUNC("DSPIFReport::InitiateDiagnostic", DSP_DIAGS);
	DBG_ENTER();
 
    unsigned int trx0TsMask, trx1TsMask;
    char testName[256];
    sscanf(parameters, "%s %x %x %u", &testName, &trx0TsMask, &trx1TsMask);
   
    // Verify parameters
    if (trx0TsMask > 0xff)
    {
       SendResultData("Invalid TS Mask for TRX 0 %d\n", trx0TsMask);
       DBG_TRACE("Invalid TS Mask for TRX 0 %d\n", trx0TsMask);
       DBG_LEAVE();
       return ERROR;
    }
   
    if (trx1TsMask > 0xff)
    {
       SendResultData("Invalid TS Mask for TRX 1 %d\n", trx1TsMask);
       DBG_TRACE("Invalid TS Mask for TRX 1 %d\n", trx1TsMask);
       DBG_LEAVE();
       return ERROR;
    }
   
    // Start MS reporting
    unsigned short    length;	   
    unsigned char     buffer[40];

    length = 0;

    buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
    buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
    buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
    buffer[length++] = 0;
    buffer[length++] = IF_REPORT;
    buffer[length++] = trx0TsMask;
   
    api_SendMsgFromRmToL1(length, buffer);
   
    length = 0;

    buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
    buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
    buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
    buffer[length++] = 1;
    buffer[length++] = IF_REPORT;
    buffer[length++] = trx1TsMask;
   
    api_SendMsgFromRmToL1(length, buffer);

    SendResultData("Start DSPIFReport\n");
    DBG_TRACE("Start DSPIFReport\n");

    DBG_LEAVE();
    return OK;
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPIFReport::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - OK if able to Terminate or ERROR if not
**
**----------------------------------------------------------------------------*/
int DSPIFReport::TerminateDiagnostic(void)
{
	DBG_FUNC("DSPIFReport::TerminateDiagnostic", DSP_DIAGS);
	DBG_ENTER();

    // Stop loopback
    unsigned short    length;	   
    unsigned char     buffer[40];

    length = 0;

    buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
    buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
    buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
    buffer[length++] = 0;
    buffer[length++] = IF_REPORT;
    buffer[length++] = 0;

    api_SendMsgFromRmToL1(length, buffer);

    length = 0;

    buffer[length++] = DIAGNOSTIC_MSG;     /* function type */
    buffer[length++] = MPH_DSP_DIAG_REPORT_REQ>>8;
    buffer[length++] = MPH_DSP_DIAG_REPORT_REQ;
    buffer[length++] = 1;
    buffer[length++] = IF_REPORT;
    buffer[length++] = 0;

    api_SendMsgFromRmToL1(length, buffer);

    SendResultData("DSPIFReport Complete!\n");
    DBG_TRACE("DSPIFReport Complete!\n");
  
    DBG_LEAVE();
    return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPIFReport::ProcessMessage
**
**    PURPOSE: Process messages not handled by Diagnostic base class.  The
**       only expected messages are HPI_ECHO_TEST_CONTINUE and 
**       DIAG_DSP_ECHO_MSG.
**
**    INPUT PARAMETERS: diagMsg - message sent to diagnostic
**
**    RETURN VALUE(S): CONTINUE_TEST - if the controlling message loop should continue
**                     STOP_TEST - if the controlling message loop should end
**
**----------------------------------------------------------------------------*/
int DSPIFReport::ProcessMessage(DiagnosticMsgType diagMsg)
{
    unsigned char    *trx_num;
    unsigned char     ts[8];
    unsigned char    *msgPtr = (unsigned char *)&diagMsg.msgBody[0];
    int               index; 

	DBG_FUNC("DSPIFReport::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();

    switch ( diagMsg.func )    
    {
        case DIAG_IF_REPORT:

            /*for( index=0; index<diagMsg.msgLen; index++ )    Used to debug incoming message
                SendResultData("%x ",*(msgPtr+index));*/

            /* Get the data out of the Message */
            trx_num = (msgPtr + 7);

            for ( index=0; index<8; index++ )
                ts[index] = *( msgPtr + 10 + index );            

            SendResultData("trx(%x): ts0 (%.2d) ts1 (%.2d) ts2 (%.2d) ts3 (%.2d) ts4 (%.2d) ts5 (%.2d) ts6 (%.2d) ts7 (%.2d)\n", 
                            *trx_num, ts[0], ts[1], ts[2], ts[3], ts[4], ts[5], ts[6], ts[7] );

            DBG_TRACE("trx(%x): ts0 (%.2d) ts1 (%.2d) ts2 (%.2d) ts3 (%.2d) ts4 (%.2d) ts5 (%.2d) ts6 (%.2d) ts7 (%.2d)\n", 
                       *trx_num, ts[0], ts[1], ts[2], ts[3], ts[4], ts[5], ts[6], ts[7] );

            if ( *trx_num == 1 )
            {
                SendResultData("\n");
                DBG_TRACE("\n");
            }

            break;
       
        default:
            break;

   }
   
   DBG_LEAVE();
   return CONTINUE_TEST;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: DSPIFReport::GetDiagnosticHelp
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): help string
**
**----------------------------------------------------------------------------*/
char *DSPIFReport::GetDiagnosticHelp(void)
{
	DBG_FUNC("DSPIFReport::GetDiagnosticHelp", DSP_DIAGS);
	DBG_ENTER();
   DBG_LEAVE();
   return (
      "IFReport: Provides interference reports from DSP's \n\n"
      "Parameters:  trx0TsMask, trx1TsMask\n\n"
      "    trx0TsMask: Time slot Mask for DSP 0  [ 0 : 0xFF ]\n"
      "    trx1TsMask: Time slot Mask for DSP 1  [ 0 : 0xFF ]\n\n"
      "Returns - IF Report which provides TRX # and Time Slot Data.\n"
   );
}
