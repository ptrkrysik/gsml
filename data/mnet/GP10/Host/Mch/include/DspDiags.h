#ifndef DspDiags_H
#define DspDiags_H

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
// File        : DspDiags.h
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description : 
//
// *******************************************************************

#include "Diagnostic.h"

class DSPLoopbackTest : public Diagnostic {
public:

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
   DSPLoopbackTest(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~DSPLoopbackTest();


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
   int InitiateDiagnostic(void);

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
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:

   unsigned int duration;
   unsigned int trx;
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPLoopbackTest constructor
   **
   **    PURPOSE: This is the default constructor for DSPLoopbackTest.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      DSPLoopbackTest.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   DSPLoopbackTest() : Diagnostic(0,0,(ReqSourceType)0,0) {}

};


class DSPMSReport : public Diagnostic {
public:

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
   DSPMSReport(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~DSPMSReport();


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
   int InitiateDiagnostic(void);

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
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:

   unsigned int duration;
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPMSReport constructor
   **
   **    PURPOSE: This is the default constructor for DSPMSReport.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      DSPMSReport.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   DSPMSReport() : Diagnostic(0,0,(ReqSourceType)0,0) {}

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
   int ProcessMessage(DiagnosticMsgType diagMsg);

};



class DSPGPRSMSReport : public Diagnostic {
public:

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
   DSPGPRSMSReport(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~DSPGPRSMSReport();


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
   int InitiateDiagnostic(void);

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
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:

   unsigned int duration;
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPGPRSMSReport constructor
   **
   **    PURPOSE: This is the default constructor for DSPGPRSMSReport.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      DSPGPRSMSReport.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   DSPGPRSMSReport() : Diagnostic(0,0,(ReqSourceType)0,0) {}

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
   int ProcessMessage(DiagnosticMsgType diagMsg);

};




class DSPHOReport : public Diagnostic {
public:

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
   DSPHOReport(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~DSPHOReport();


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
   int InitiateDiagnostic(void);

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
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:

   unsigned int duration;
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPHOReport constructor
   **
   **    PURPOSE: This is the default constructor for DSPHOReport.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      DSPHOReport.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   DSPHOReport() : Diagnostic(0,0,(ReqSourceType)0,0) {}

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
   int ProcessMessage(DiagnosticMsgType diagMsg);

};


class DSPDLReport : public Diagnostic {
public:

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
   DSPDLReport(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~DSPDLReport();


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
   int InitiateDiagnostic(void);

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
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:

   unsigned int duration;
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPDLReport constructor
   **
   **    PURPOSE: This is the default constructor for DSPDLReport.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      DSPDLReport.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   DSPDLReport() : Diagnostic(0,0,(ReqSourceType)0,0) {}

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
   int ProcessMessage(DiagnosticMsgType diagMsg);

};



// This is the maximum number of outstanding message bursts.  This value
// is used to determine when messages have expired and will never be
// responded too.
#define MAX_MSG_BURSTS     20
#define HPI_MSG_SIZE       64

class HPIEchoTest : public Diagnostic {
public:

   enum {UNALLOCATED, RESPONSE_PENDING};

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
   HPIEchoTest(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~HPIEchoTest();


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
   int InitiateDiagnostic(void);

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
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:

   unsigned int duration;
   unsigned int totalErrs[2], totalNoRsp[2], totalMsgs[2], totalUnkownDspErrs;
   int testDsp0, testDsp1, numMsgs, delay, verbose, lockTask;
   
   typedef struct {
      int msgState;
      unsigned char msg[HPI_MSG_SIZE];
   } EchoMsgStruct;
      
   EchoMsgStruct *dspMsgs[2][MAX_MSG_BURSTS];
   int curMsgBurst[2];
      
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: HPIEchoTest constructor
   **
   **    PURPOSE: This is the default constructor for HPIEchoTest.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      DSPDLReport.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   HPIEchoTest() : Diagnostic(0,0,(ReqSourceType)0,0) {}

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
   int ProcessMessage(DiagnosticMsgType diagMsg);

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
   void BuildEchoMsg(int dspNum);

};


class DSPIFReport : public Diagnostic {
public:

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
   DSPIFReport(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~DSPIFReport();


   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPIFReport::InitiateDiagnostic
   **
   **    PURPOSE: 
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): status - 
   **
   **----------------------------------------------------------------------------*/
   int InitiateDiagnostic(void);

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPIFReport::TerminateDiagnostic
   **
   **    PURPOSE: 
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): status - 
   **
   **----------------------------------------------------------------------------*/
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPIFReport constructor
   **
   **    PURPOSE: This is the default constructor for DSPIFReport.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      DSPIFReport.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   DSPIFReport() : Diagnostic(0,0,(ReqSourceType)0,0) {}

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPIFReport::ProcessMessage
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
   int ProcessMessage(DiagnosticMsgType diagMsg);

};


/*******************************************************************************
*/
class DSPBBLoopbackTest : public Diagnostic 
{

public:

   // constructor
   DSPBBLoopbackTest
   (  
     char *testName,         // IN: name for test
     int testNum,            // IN: test number
     ReqSourceType reqSrc,   // IN: source of the diagnostic request
     char *parms             // IN: pointer to parameter list
   );

   // destructor
   ~DSPBBLoopbackTest();

   int InitiateDiagnostic(void); // RETURN: status 

   int TerminateDiagnostic(void); // RETURN: status

   char *GetDiagnosticHelp(void); // RETURN: help string


private:

   int  src;
   int  dst;
   int  index;
   int  shift;
   int  verbose;
   int  DSPBBLBTest_Cmd_Status;
   int  dspbblbtest_verbose;

   // default constructor
   /* defined in the private section and cannot be used to create an instance of
      DSPBBLoopbackTest */  
   DSPBBLoopbackTest() : Diagnostic(0,0,(ReqSourceType)0,0) {}

   int ProcessMessage       // RETUEN: CONTINUE_TEST/STOP_TEST
   (
     DiagnosticMsgType diagMsg  //IN: Message
   );

   void dspBBLoopbackTest(int srcDsp, int dstDsp, int freqIndex, int gainShift, int verbose);
   void dspBBLoopbackTest2(int verbose);
   void dspToneMeasureRsp(unsigned char data[], int verbose);

}; // class DSPBBLoopbackTest


/*******************************************************************************
*/
class DSPExtMemTest : public Diagnostic 
{

public:

   // constructor
   DSPExtMemTest
   (  
     char *testName,         // IN: name for test
     int testNum,            // IN: test number
     ReqSourceType reqSrc,   // IN: source of the diagnostic request
     char *parms             // IN: pointer to parameter list
   );

   // destructor
   ~DSPExtMemTest();

   int InitiateDiagnostic(void); // RETURN: status 

   int TerminateDiagnostic(void); // RETURN: status

   char *GetDiagnosticHelp(void); // RETURN: help string


private:

   const int    MaxTestType = 5;
   int          dspextmemtest_verbose;
   int          dspNum; 
   int          testTypeReq;   /* test type requested */
   int          testType;      /* test type performed */
   int          verbose;    
   char         fileName[100];
   int          dspResponds;   /* TRUE if DSP Responds */  
   unsigned int numErrors;

   // default constructor
   /* defined in the private section and cannot be used to create an instance of
      DSPExtMemTest */  
   DSPExtMemTest() : Diagnostic(0,0,(ReqSourceType)0,0) {}

   int ProcessMessage       // RETUEN: CONTINUE_TEST/STOP_TEST
   (
     DiagnosticMsgType diagMsg  //IN: Message
   );

   void dspExtMemTest(int dspNum, int testType, int verbose);

   void dspExtMemTestRsp(unsigned char data[]);

   void dspExtMemErrorRsp(unsigned char data[]);

}; // class DSPExtMemTest


#endif
