#ifndef SysDiags_H
#define SysDiags_H

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
// File        : SysDiags.h
// Author(s)   : Tim Olson
// Create Date : 12/8/99
// Description : 
//
// *******************************************************************

#include "Diagnostic.h"
#include <stdio.h>
#include <stdlib.h>
#include <taskLib.h>
#include <sockLib.h>
#include <inetLib.h>
#include <ioLib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>


#define  DEFPORT            3456
#define  DEFSTART           1024
#define  DEFEND             4096
#define  DEFINC             1024
#define  DEFDURATION        60
#define  TRIALS             15
#define  REPEAT             1000
#define  NSAMP              8000
#define  PERT               3
#define  LATENCYREPS        100
#define  LONGTIME           1e99
#define  CHARSIZE           8
#define  RUNTM              0.25

#define     ABS(x)     (((x) < 0)?(-(x)):(x))
#define     MIN(x,y)   (((x) < (y))?(x):(y))
#define     MAX(x,y)   (((x) > (y))?(x):(y))


typedef struct protocolstruct ProtocolStruct;
struct protocolstruct
{
    struct sockaddr_in      sin1,   /* socket structure #1              */
                            sin2;   /* socket structure #2              */
    int                     nodelay;  /* Flag for TCP nodelay                */
    struct hostent          *addr;    /* Address of host                     */
	int                     sndbufsz,
							rcvbufsz;
};

typedef struct argstruct ArgStruct;
struct argstruct 
{
   /* This is the common information that is needed for all tests              */
   char     *host;            /* Name of receiving host                        */
   int      servicefd,        /* File descriptor of the network socket         */
            commfd;           /* Communication file descriptor                 */
   short    port;             /* Port used for connection                      */
   char     *buff;            /* Transmitted buffer                            */
   char     *buff1;           /* Transmitted buffer                            */
   int      bufflen,          /* Length of transmitted buffer                  */
            tr,               /* Transmit flag                                 */
            nbuff;            /* Number of buffers to transmit                 */
            
    /* Now we work with a union of information for protocol dependent stuff  */
    ProtocolStruct prot;    /* Structure holding necessary info for TCP      */
};


class NetworkTestServer : public Diagnostic {
public:

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer constructor
   **
   **    PURPOSE: Initialize data members for NetworkTestServer object.  
   **
   **    INPUT PARAMETERS: testName - ASCII name for test
   **                      testNum - test number
   **                      reqSrc - source of the diagnostic request
   **                      parms - pointer to parameter list in ASCII
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   NetworkTestServer(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer destructor
   **
   **    PURPOSE: Delete data members for NetworkTestServer object.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   ~NetworkTestServer();


   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::InitiateDiagnostic
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
   **    METHOD NAME: NetworkTestServer::ProcessMessage
   **
   **    PURPOSE: Process messages not handled by Diagnostic base class.
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
   **    METHOD NAME: NetworkTestServer::TerminateDiagnostic
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
   **    METHOD NAME: NetworkTestServer::NetTestServer
   **
   **    PURPOSE: 
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): status - 
   **
   **----------------------------------------------------------------------------*/
   static int NetTestServer(int thisPtr);


   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::GetDiagnosticHelp
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

   int port, streamopt, start, end, inc, verbose, bufsize, duration, verify;
   int taskId;
   ArgStruct   args;

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer constructor
   **
   **    PURPOSE: This is the default constructor for NetworkTestServer.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      NetworkTestServer.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   NetworkTestServer() : Diagnostic(0,0,(ReqSourceType)0,0) {}
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::When
   **
   **    PURPOSE: Return the current time in seconds, using a double precision 
   **       number.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): current time in seconds 
   **
   **----------------------------------------------------------------------------*/
   double When(); 

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::SetupServer
   **
   **    PURPOSE: Setup the socket to act as the server for the test. 
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): status - OK if successful 
   **                              ERROR if setup failed
   **----------------------------------------------------------------------------*/
   int SetupServer(ArgStruct *p);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::SyncServer
   **
   **    PURPOSE: Synchronize with the other end of the connection.
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): none 
   **
   **----------------------------------------------------------------------------*/
   void SyncServer(ArgStruct *p);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::SendDataServer
   **
   **    PURPOSE: Send date to the other end of the connection.
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): none 
   **
   **----------------------------------------------------------------------------*/
   void SendDataServer(ArgStruct *p);

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::RecvDataServer
   **
   **    PURPOSE: Receive data from the other end of the connection. 
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): none 
   **
   **----------------------------------------------------------------------------*/
   void RecvDataServer(ArgStruct *p);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::RecvRepeatServer
   **
   **    PURPOSE: Receive the number of repetitions from the other end of the 
   **       connection. 
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **                      rpt - number of repetitions
   **
   **    RETURN VALUE(S): none 
   **
   **----------------------------------------------------------------------------*/
   void RecvRepeatServer(ArgStruct *p, int *rpt);

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::EstablishServer
   **
   **    PURPOSE: Accept a connection from a client.
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): status - OK if establish successful
   **                              ERROR if establish failed
   **----------------------------------------------------------------------------*/
   int EstablishServer(ArgStruct *p);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::CleanUpServer
   **
   **    PURPOSE: Cleanup sockets used for this test.
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): status - OK if cleanup successful
   **                              ERROR if cleanup failed
   **----------------------------------------------------------------------------*/
   int  CleanUpServer(ArgStruct *p);

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::SendStopMsg
   **
   **    PURPOSE: Send a message to parent task to stop network test.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **----------------------------------------------------------------------------*/
   void  SendStopMsg(void);


   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::ReadFully
   **
   **    PURPOSE: Read data from specified socket.
   **
   **    INPUT PARAMETERS: fd - file descriptor for socket
   **                      obuf - pointer to buffer to be filled
   **                      len - number of bytes to receive
   **
   **    RETURN VALUE(S): for success - number of bytes read
   **                     for failure - ERROR
   **----------------------------------------------------------------------------*/
   int ReadFully(int fd, char *obuf, int len);

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestServer::InvertData
   **
   **    PURPOSE: xor all the data in the received buffer..
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **----------------------------------------------------------------------------*/
   void InvertData();
};















class NetworkTestTransmit : public Diagnostic {
public:

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit constructor
   **
   **    PURPOSE: Initialize data members for NetworkTestTransmit object.  
   **
   **    INPUT PARAMETERS: testName - ASCII name for test
   **                      testNum - test number
   **                      reqSrc - source of the diagnostic request
   **                      parms - pointer to parameter list in ASCII
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   NetworkTestTransmit(char *testName, int testNum, ReqSourceType reqSrc, char *parms);

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit destructor
   **
   **    PURPOSE: Delete data members for NetworkTestTransmit object.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   ~NetworkTestTransmit();


   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::InitiateDiagnostic
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
   **    METHOD NAME: NetworkTestTransmit::ProcessMessage
   **
   **    PURPOSE: Process messages not handled by Diagnostic base class.
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
   **    METHOD NAME: NetworkTestServer::TerminateDiagnostic
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
   **    METHOD NAME: NetworkTestTransmit::NetTestTransmit
   **
   **    PURPOSE: 
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): status - 
   **
   **----------------------------------------------------------------------------*/
   static int NetTestTransmit(int thisPtr);


   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::GetDiagnosticHelp
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

   int port, streamopt, start, end, inc, verbose, bufsize, duration, verify;
   char hostAddr[64];
   int taskId;
   long totalPkts, totalErrors;
   ArgStruct   args;

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit constructor
   **
   **    PURPOSE: This is the default constructor for NetworkTestTransmit.  It is
   **      defined in the private section and cannot be used to create an instance of
   **      NetworkTestServer.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **
   **----------------------------------------------------------------------------*/
   NetworkTestTransmit() : Diagnostic(0,0,(ReqSourceType)0,0) {}
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::When
   **
   **    PURPOSE: Return the current time in seconds, using a double precision 
   **       number.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): current time in seconds 
   **
   **----------------------------------------------------------------------------*/
   double When(); 

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::SetupTransmit
   **
   **    PURPOSE: Setup the socket to act as the server for the test. 
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): status - OK if successful 
   **                              ERROR if setup failed
   **----------------------------------------------------------------------------*/
   int SetupTransmit(ArgStruct *p);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::SyncTransmit
   **
   **    PURPOSE: Synchronize with the other end of the connection.
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): none 
   **
   **----------------------------------------------------------------------------*/
   void SyncTransmit(ArgStruct *p);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::SendDataTransmit
   **
   **    PURPOSE: Send date to the other end of the connection.
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): none 
   **
   **----------------------------------------------------------------------------*/
   void SendDataTransmit(ArgStruct *p);

   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::RecvDataTransmit
   **
   **    PURPOSE: Receive data from the other end of the connection. 
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): none 
   **
   **----------------------------------------------------------------------------*/
   void RecvDataTransmit(ArgStruct *p);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::SendRepeatTransmit
   **
   **    PURPOSE: Send the repeat count to the other end of the connection.
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **                      rpt - repeat count
   **
   **    RETURN VALUE(S): none 
   **
   **----------------------------------------------------------------------------*/
   void SendRepeatTransmit(ArgStruct *p, int rpt);
   
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::EstablishTransmit
   **
   **    PURPOSE: Accept a connection from a client.
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): status - OK if establish successful
   **                              ERROR if establish failed
   **----------------------------------------------------------------------------*/
   int EstablishTransmit(ArgStruct *p);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::CleanUpTransmit
   **
   **    PURPOSE: Cleanup sockets used for this test.
   **
   **    INPUT PARAMETERS: p - pointer to test arguments
   **
   **    RETURN VALUE(S): status - OK if cleanup successful
   **                              ERROR if cleanup failed
   **----------------------------------------------------------------------------*/
   int  CleanUpTransmit(ArgStruct *p);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::SendStopMsg
   **
   **    PURPOSE: Send a message to parent task to stop network test.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): none
   **----------------------------------------------------------------------------*/
   void  SendStopMsg(void);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::ReadFully
   **
   **    PURPOSE: Read data from specified socket.
   **
   **    INPUT PARAMETERS: fd - file descriptor for socket
   **                      obuf - pointer to buffer to be filled
   **                      len - number of bytes to receive
   **
   **    RETURN VALUE(S): for success - number of bytes read
   **                     for failure - ERROR
   **----------------------------------------------------------------------------*/
   int ReadFully(int fd, char *obuf, int len);


   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::FillTestPattern
   **
   **    PURPOSE: Fill up a buffer with the given test pattern.
   **
   **    INPUT PARAMETERS: buf - pointer to buffer to be filled
   **                      bufLen - number of bytes to fill
   **                      patternIndx - selected pattern
   **
   **    RETURN VALUE(S): none
   **----------------------------------------------------------------------------*/
   void FillTestPattern (char *buf, int bufLen, int patternIndx);
   
   /*----------------------------------------------------------------------------**
   **
   **    METHOD NAME: NetworkTestTransmit::VerifyData
   **
   **    PURPOSE: Verify that the transmitted data matches the received data.
   **
   **    INPUT PARAMETERS: none
   **
   **    RETURN VALUE(S): 0 - no errors
   **                     1 - errors detected
   **----------------------------------------------------------------------------*/
   int VerifyData ();
   
};





#endif