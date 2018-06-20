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
// File        : CdcDiags.h
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description : 
//
// *******************************************************************
#ifndef _CDCDIAGS_H_
#define _CDCDIAGS_H_  /* include once only */


#include "Diagnostic.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "cdc_bsp/i2c.h"

#ifdef __cplusplus
}
#endif

class HPIMemoryTest : public Diagnostic {
public:

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
   HPIMemoryTest(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~HPIMemoryTest();


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
   int InitiateDiagnostic(void);

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
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: HPIMemoryTest constructor
   **
   **    PURPOSE: This is the default constructor for HPIMemoryTest.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      HPIMemoryTest.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   HPIMemoryTest() : Diagnostic(0,0,(ReqSourceType)0,0) {}

};



class I2CLoopbackTest : public Diagnostic {
public:

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
   I2CLoopbackTest(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~I2CLoopbackTest();


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
   int InitiateDiagnostic(void);

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
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:

   int numBytes, verbose, numErrs, haltOnErr;
   unsigned int duration;
   unsigned long totalSent;
   char testName[256];
   unsigned char writeBuf[I2C_TX_LEN];
   unsigned char readBuf[I2C_RX_LEN];
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: I2CLoopbackTest constructor
   **
   **    PURPOSE: This is the default constructor for I2CLoopbackTest.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      I2CLoopbackTest.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   I2CLoopbackTest() : Diagnostic(0,0,(ReqSourceType)0,0) {}
   
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
   int ProcessMessage(DiagnosticMsgType diagMsg);
   
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
   int SendI2CData(void);
   
};


class DSPMemTest : public Diagnostic {
public:

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
   DSPMemTest(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
   ~DSPMemTest();


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
   int InitiateDiagnostic(void);

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
   int TerminateDiagnostic(void);


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
   char *GetDiagnosticHelp(void);

private:

   int testLoc, testType, testTypeTmp, numErrs, verbose;
   unsigned int duration, dsp, testPattern, zerosPattern, onesPattern, seed;
   char testName[256];
   
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
   void TestDspMemBlock(unsigned int startAddr, unsigned int endAddr);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPMemTest::TestDspMemBlockIncAddr
   **
   **    PURPOSE: Test the RAM at the given address with testPattern.
   **
   **    INPUT PARAMETERS: startAddr - starting location to test
   **                      endAddr - ending location to test
   **                      doInverse - test the inverse of incrementing address.
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   void TestDspMemBlockIncAddr(unsigned int startAddr, unsigned int endAddr,
      int doInverse);
   
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
   void TestDspMemBlockRandom(unsigned int startAddr, unsigned int endAddr);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPMemTest constructor
   **
   **    PURPOSE: This is the default constructor for DSPMemTest.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      DSPMemTest.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   DSPMemTest() : Diagnostic(0,0,(ReqSourceType)0,0) {}

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: DSPMemTest::ProcessMessage
   **
   **    PURPOSE: Process messages not handled by Diagnostic base class.  The
   **       only expected message is DSPMemTest.
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
class CFTest : public Diagnostic 
{

public:

   // constructor
   CFTest
   (  
     char *testName,         // IN: name for test
     int testNum,            // IN: test number
     ReqSourceType reqSrc,   // IN: source of the diagnostic request
     char *parms             // IN: pointer to parameter list
   );

   // destructor
   ~CFTest();

   int InitiateDiagnostic(void); // RETURN: status 

   int TerminateDiagnostic(void); // RETURN: status

   char *GetDiagnosticHelp(void); // RETURN: help string


private:

   // default constructor
   /* defined in the private section and cannot be used to create an instance of
      CFTest */  
   CFTest() : Diagnostic(0,0,(ReqSourceType)0,0) {}

}; // class CFTest


class FPGATest : public Diagnostic {
public:

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
FPGATest(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
~FPGATest();

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: FPGATest::InitiateDiagnostic
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
**    METHOD NAME: FPGATest::TerminateDiagnostic
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
**    METHOD NAME: FPGATest::GetDiagnosticHelp
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
   **    METHOD NAME: FPGATest constructor
   **
   **    PURPOSE: This is the default constructor for FPGATest.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      FPGATest.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   FPGATest() : Diagnostic(0,0,(ReqSourceType)0,0) {}

};

class I2CTest : public Diagnostic {
public:

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: I2CTest constructor
**
**    PURPOSE: Initialize data members for FPGI2CTestATest object.  
**
**    INPUT PARAMETERS: testName - ASCII name for test
**                      testNum - test number
**                      reqSrc - source of the diagnostic request
**                      parms - pointer to parameter list in ASCII
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
I2CTest(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

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
~I2CTest();

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
int InitiateDiagnostic(void);

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
int TerminateDiagnostic(void);

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
char *GetDiagnosticHelp(void);

};

#endif