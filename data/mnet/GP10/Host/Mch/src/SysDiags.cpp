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
// File        : SysDiags.cpp
// Author(s)   : Tim Olson
// Create Date : 6/29/99
// Description :  
//
// *******************************************************************



#include "SysDiags.h"

#define NUM_PATTERN_BYTES     5
#define MAX_PATTERNS          3
char TestPatterns[][NUM_PATTERN_BYTES] = 
{
   {0x00, 0x11, 0x22, 0x33, 0x44},
   {0x55, 0x66, 0x77, 0x88, 0x99},
   {0xaa, 0xbb, 0xcc, 0xdd, 0xee}
};



// *******************************************************************
// NetworkTestServer
// *******************************************************************

    
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
NetworkTestServer::NetworkTestServer(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

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
NetworkTestServer::~NetworkTestServer()
{
}


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
int NetworkTestServer::InitiateDiagnostic(void)
{
   char testName[256];
   
   // Setup default parameter values.
   start=DEFSTART;    
   end=DEFEND; 
   port=DEFPORT;
   inc=DEFINC;
   verbose=0;
   bufsize = 0;
   duration=DEFDURATION;
   verify=0;
   
   
   // Extract paramters from command line.
   //   
   // Command Line : port, verbose
   sscanf(parameters, "%s %d %d %d %d %d %d %d %d",
      &testName, &verbose, &duration, &verify, &start, &end, &inc, &bufsize, &port);
   
   // Verify that the paramaters are reasonable
   if (start > end)
   {
      SendResultData("Start MUST be LESS than end %d %d\n", start, end);
      return ERROR;
   }
   
   SendResultData ("%s verbose(%d) duration(%d) verify(%d) start(%d) end(%d) inc(%d) bufsize(%d) port(%d)\n",
      testName, verbose, duration, verify, start, end, inc, bufsize, port);
   
   // If this is a timed test then start the timer.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Spawn a task to do the sending and receiving of data.  
   // The function called is a static function
   // of diagnostic.  Since the object needs an instance to access data
   // members and non-static functions, the this pointer is passed as an argument.
   taskId = taskSpawn("NetTstSrv", DIAGNOSTIC_TEST_TASK_PRI, 0,
             DIAGNOSTIC_TEST_STACK_SIZE, 
             (FUNCPTR) NetworkTestServer::NetTestServer,
             (int)this, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

   if ( taskId == ERROR )
   {
      SendResultData("NetworkTestServer:Unable to create Task %d", taskId);
      return ERROR;
   }
     
   return OK;
}


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
int NetworkTestServer::ProcessMessage(DiagnosticMsgType diagMsg)
{
   switch ( diagMsg.func )
   {
   }
   return CONTINUE_TEST;
}


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
int NetworkTestServer::TerminateDiagnostic(void)
{
   SendResultData("NetworkTestServer Complete!\n");
     
   taskDelete(taskId);
   CleanUpServer(&args);
     
   if (args.buff)      
      free(args.buff);
      
   return OK;
}


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
int NetworkTestServer::NetTestServer(int thisPtr)
{   
   // Grab a pointer to the network test object.
   NetworkTestServer *pNetTest = (NetworkTestServer *)thisPtr;
   
   int         i, j, n,        /* Loop indices                              */
               nrepeat,        /* Number of time to do the transmission     */
               len,            /* Number of bytes to be transmitted         */
               pert;           /* Perturbation value                        */
   

   pNetTest->args.prot.sndbufsz = pNetTest->bufsize;
   pNetTest->args.prot.rcvbufsz = pNetTest->bufsize;

   pNetTest->args.nbuff = TRIALS;
   pNetTest->args.port = pNetTest->port;

   pNetTest->SetupServer(&pNetTest->args);
   pNetTest->EstablishServer(&pNetTest->args);

   while (1)
   {
      pNetTest->args.bufflen = 1;
      pNetTest->args.buff = (char *)malloc(pNetTest->args.bufflen);
      pNetTest->SyncServer(&pNetTest->args);
      for (i = 0; i < LATENCYREPS; i++)
      {
          pNetTest->RecvDataServer(&pNetTest->args);
          pNetTest->SendDataServer(&pNetTest->args);
      }
      
      free(pNetTest->args.buff);
      pNetTest->args.buff = 0;
      
      if (pNetTest->inc == 0)
      {
   	   /* Set a starting value for the message size increment. */
   	   pNetTest->inc = (pNetTest->start > 1) ? pNetTest->start / 2 : 1;
      }
      
      /* Main loop of benchmark */
      for (n = 0, len = pNetTest->start; 
           n < NSAMP - 3 && len <= pNetTest->end; 
           len = len + pNetTest->inc)
      {        
          /* This is a perturbation loop to test nearby values */
          for (pert = (pNetTest->inc > PERT+1)? -PERT: 0;
               pert <= PERT; 
               n++, pert += (pNetTest->inc > PERT+1)? PERT: PERT+1)
          {

              /* Calculate howmany times to repeat the experiment. */
              pNetTest->RecvRepeatServer(&pNetTest->args, &nrepeat);

              /* Allocate the buffer */
              pNetTest->args.bufflen = ((len + pert) <= 0)? 1: (len + pert);
              if((pNetTest->args.buff=(char *)malloc(pNetTest->args.bufflen))==(char *)NULL)
              {
                  pNetTest->SendResultData("Couldn't allocate memory\n");
                  break;
              }
              if((pNetTest->args.buff1=(char *)malloc(pNetTest->args.bufflen))==(char *)NULL)
              {
                  pNetTest->SendResultData("Couldn't allocate memory\n");
                  break;
              }

              /* Finally, we get to receive */
              for (i = 0; i < 3; i++)
              {
                  pNetTest->SyncServer(&pNetTest->args);
                  for (j = 0; j < nrepeat; j++)
                  {
                      pNetTest->RecvDataServer(&pNetTest->args);
                      if (pNetTest->verify)
                          pNetTest->InvertData();
                      pNetTest->SendDataServer(&pNetTest->args);
                  }
              }
                              
              free(pNetTest->args.buff);
              pNetTest->args.buff = 0;

          } /* End of perturbation loop */
      } /* End of main loop  */
   } /* Loop forever */     
}


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
double NetworkTestServer::When()
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double) tp.tv_sec + (double) tp.tv_nsec * 1e-9);
}


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
int NetworkTestServer::SetupServer(ArgStruct *p)
{

   int one = 1;
   int sockfd;
   struct sockaddr_in *lsin1, *lsin2;      /* ptr to sockaddr_in in ArgStruct */
   struct hostent *addr;

   lsin1 = &(p->prot.sin1);
   lsin2 = &(p->prot.sin2);

   bzero((char *) lsin1, sizeof(*lsin1));
   bzero((char *) lsin2, sizeof(*lsin2));

   if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      SendResultData("server: can't open stream socket");
      return(ERROR);
   }

   /* Attempt to set TCP_NODELAY */
   if(setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one)) < 0)
   {
      SendResultData("setsockopt: nodelay\n");
      return(ERROR);
   }

   /* If requested, set the send and receive buffer sizes */
   if(p->prot.sndbufsz > 0)
   {
      SendResultData("Send and Receive Buffers set to %d bytes\n", p->prot.sndbufsz);
      if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&(p->prot.sndbufsz), 
                                       sizeof(p->prot.sndbufsz)) < 0)
      {
          SendResultData("setsockopt: sndbuf\n");
          return(ERROR);
      }
      if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&(p->prot.rcvbufsz), 
                                       sizeof(p->prot.rcvbufsz)) < 0)
      {
          SendResultData("setsockopt: rcvbuf\n");
          return(ERROR);
      }
   }

   bzero((char *) lsin1, sizeof(*lsin1));
   lsin1->sin_family      = AF_INET;
   lsin1->sin_addr.s_addr = htonl(INADDR_ANY);
   lsin1->sin_port        = htons(p->port);
   
   if (bind(sockfd, (struct sockaddr *) lsin1, sizeof(*lsin1)) < 0)
   {
      SendResultData("server: can't bind local address! E#:%d", errno);
      return(ERROR);
   }

   p->servicefd = sockfd;

   return(OK);
 
}   

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
void NetworkTestServer::SyncServer(ArgStruct *p)
{
   char s[] = "SyncMe";
   char response[7];

   if (send(p->commfd, s, strlen(s), 0) < 0 ||
	   ReadFully(p->commfd, response, strlen(s)) < 0)
   {
	   SendResultData("Server: error writing or reading synchronization string");
	   SendStopMsg();
   }
   if (strncmp(s, response, strlen(s)))
   {
	   SendResultData("Server: Synchronization string incorrect!\n");
	   SendStopMsg();
   }
}


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
void NetworkTestServer::SendDataServer(ArgStruct *p)
{
   int bytesWritten, bytesLeft;
   char *q;

   bytesLeft = p->bufflen;
   bytesWritten = 0;
   q = p->buff;
   while (bytesLeft > 0 &&
	  (bytesWritten = send(p->commfd, q, bytesLeft, 0)) > 0)
   {
	   bytesLeft -= bytesWritten;
	   q += bytesWritten;
   }
   if (bytesWritten == -1)
   {
	   SendResultData("Server: send: error encountered, errno=%d\n", errno);
	   SendStopMsg();
   }

}


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
void NetworkTestServer::RecvDataServer(ArgStruct *p)
{
   int bytesLeft;
   int bytesRead;
   char *q;

   bytesLeft = p->bufflen;
   bytesRead = 0;
   q = p->buff;
   while (bytesLeft > 0 &&
	   (bytesRead = recv(p->commfd, q, bytesLeft, 0)) > 0)
   {
	   bytesLeft -= bytesRead;
	   q += bytesRead;
   }
   if (bytesLeft > 0 && bytesRead == 0)
   {
	   SendResultData("Server: \"end of file\" encountered on reading from socket\n");
   }
   else if (bytesRead == -1)
   {
	   SendResultData("Server: read: error encountered, errno=%d\n", errno);
	   SendStopMsg();
   }
}


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
void NetworkTestServer::RecvRepeatServer(ArgStruct *p, int *rpt)
{
   unsigned long lrpt, nrpt;
	int bytesRead;

	bytesRead = ReadFully(p->commfd, (char *)&nrpt, sizeof(unsigned long));
	if (bytesRead < 0)
	{
		SendResultData("Server: read failed in RecvRepeat: errno=%d\n", errno);
		SendStopMsg();
   }
	else if (bytesRead != sizeof(unsigned long))
   {
		SendResultData("Server: partial read in RecvRepeat of %d bytes\n",
	      bytesRead);
		SendStopMsg();
   }
	lrpt = ntohl(nrpt);

	*rpt = lrpt;
}


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
int NetworkTestServer::EstablishServer(ArgStruct *p)
{
   int clen;

   clen = sizeof(p->prot.sin2);
   /* SERVER */
   listen(p->servicefd, 5);
   p->commfd = accept(p->servicefd, (struct sockaddr *)&(p->prot.sin2), &clen);

   if(p->commfd < 0)
   {
      SendResultData("Server: Accept Failed! E#:%d\n",errno);
      SendStopMsg();
      return(ERROR);
      
   }
   return(OK);
}

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
int  NetworkTestServer::CleanUpServer(ArgStruct *p)
{
   char *quit="QUIT";
   send(p->commfd,quit, 5, 0);
   recv(p->commfd,quit,5, 0);
   close(p->commfd);
   close(p->servicefd);
   return(OK);
}


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
char *NetworkTestServer::GetDiagnosticHelp(void)
{
   return (
      "NetworkTestServer\n\n"
      "Test the integrity and throughput of the network connection.  Must use this"
      "test in conjunction with NetworkTestTransmit or external client.\n\n"
      "Parameters - verbose duration verify start end inc bufsize port\n"
      "\tverbose - If non-zero show individual error occurences.\n"
      "\tduration - Time in seconds to run test (DEFAULT 60)\n"
      "\tverify - Invert data for verfication by the tranmitter (DEFAULT 0)\n"
	   "\tstart - lower bound start value (DEFAULT 1024)\n"
	   "\tend - upper bound stop value (DEFAULT 4096)\n"
	   "\tinc - specify increment step size (DEFAULT 1024)\n"
      "\tbufsize - size of receive and send buffers (DEFAULT OS default)\n"
      "\tport - port number to listen to (DEFAULT 3456)\n"      
      "Returns - Number of error packets\n"
   );
}



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
void  NetworkTestServer::SendStopMsg(void)
{
   // Send a message to stop the test.
   DiagnosticMsgType diagMsg;

   // Initialize the diagnostic message.
   diagMsg.func = DIAG_TEST_INTERNAL_STOP_TEST;

   msgQSend(diagQId, 
            (char *)&diagMsg, 
            sizeof(DiagnosticMsgType), 
            WAIT_FOREVER, 
            MSG_PRI_NORMAL);
}


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
int NetworkTestServer::ReadFully(int fd, char *obuf, int len)
{
	int bytesLeft = len;
	char *buf = obuf;
	int bytesRead = 0;

	while (bytesLeft > 0 &&
	 (bytesRead = recv(fd, buf, bytesLeft, 0)) > 0)
	{
      bytesLeft -= bytesRead;
      buf += bytesRead;
	}
	if (bytesRead <= 0)
		return bytesRead;
	return len;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: NetworkTestServer::InvertData
**
**    PURPOSE: Invert all the data in the received buffer..
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**----------------------------------------------------------------------------*/
void NetworkTestServer::InvertData()
{
	for (int i = 0; i < args.bufflen; i++)
		args.buff[i] = ~args.buff[i];
}





// *******************************************************************
// NetworkTestTransmit
// *******************************************************************

    
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
NetworkTestTransmit::NetworkTestTransmit(char *testName, int testNum, ReqSourceType reqSrc,
   char *parms) : Diagnostic(testName, testNum, reqSrc, parms)
{
}

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
NetworkTestTransmit::~NetworkTestTransmit()
{
}


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
int NetworkTestTransmit::InitiateDiagnostic(void)
{
   char testName[256];
   
   // Setup default parameter values.
   start=DEFSTART;    
   end=DEFEND; 
   port=DEFPORT;
   inc=DEFINC;
   verbose=0;
   bufsize = 0;
   duration=DEFDURATION;
   verify=0;
   
   
   // Extract paramters from command line.
   //   
   // Command Line : port, verbose
   sscanf(parameters, "%s %s %d %d %d %d %d %d %d %d",
      &testName, &hostAddr, &verbose, &duration, &verify, &start, &end, &inc, &bufsize, &port);
   
   // Verify that the paramaters are reasonable
   if (start > end)
   {
      SendResultData("Start MUST be LESS than end %d %d\n", start, end);
      return ERROR;
   }
   
   SendResultData ("%s hostAddr(%s) verbose(%d) duration(%d) verify(%d) start(%d) end(%d) inc(%d) bufsize(%d) port(%d)\n",
      testName, hostAddr, verbose, duration, verify, start, end, inc, bufsize, port);
   
   // If this is a timed test then start the timer.
   if (duration)
   {
      SetTimer(duration);
   }
   
   // Spawn a task to do the sending and receiving of data.  
   // The function called is a static function
   // of diagnostic.  Since the object needs an instance to access data
   // members and non-static functions, the this pointer is passed as an argument.
   taskId = taskSpawn("NetTstTrn", DIAGNOSTIC_TEST_TASK_PRI, 0,
             DIAGNOSTIC_TEST_STACK_SIZE, 
             (FUNCPTR) NetworkTestTransmit::NetTestTransmit,
             (int)this, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

   if ( taskId == ERROR )
   {
      SendResultData("NetworkTestTransmit:Unable to create Task %d", taskId);
      return ERROR;
   }
     
   return OK;
}


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
int NetworkTestTransmit::ProcessMessage(DiagnosticMsgType diagMsg)
{
   switch ( diagMsg.func )
   {
   }
   return CONTINUE_TEST;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: NetworkTestTransmit::TerminateDiagnostic
**
**    PURPOSE: 
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): status - 
**
**----------------------------------------------------------------------------*/
int NetworkTestTransmit::TerminateDiagnostic(void)
{
   SendResultData("NetworkTestTransmit Complete!\n");
     
   taskDelete(taskId);
   CleanUpTransmit(&args);
       
   if (args.buff)      
      free(args.buff);
      
   if (args.buff1)      
      free(args.buff1);
      
   SendResultData("Total Packets Transmitted (%d) Total Errors (%d)\n",
      totalPkts, totalErrors);
   return OK;
}


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
int NetworkTestTransmit::NetTestTransmit(int thisPtr)
{   
   // Grab a pointer to the network test object.
   NetworkTestTransmit *pNetTest = (NetworkTestTransmit *)thisPtr;
   
   int         i, j, n,        /* Loop indices                              */
               nrepeat,        /* Number of time to do the transmission     */
               len,            /* Number of bytes to be transmitted         */
               pert,           /* Perturbation value                        */
               currPattern;
   
   double      t, t0,          /* Time variables                            */
               tlast,          /* Time for the last transmission            */
               tzero=0,
               latency,        /* Network message latency                   */
               bps,
               bits;
   

   currPattern = 0;
   
   pNetTest->args.prot.sndbufsz = pNetTest->bufsize;
   pNetTest->args.prot.rcvbufsz = pNetTest->bufsize;

   pNetTest->args.nbuff = TRIALS;
   pNetTest->args.port = pNetTest->port;
   
   pNetTest->args.host = pNetTest->hostAddr;

   pNetTest->SetupTransmit(&pNetTest->args);
   pNetTest->EstablishTransmit(&pNetTest->args);
   
   pNetTest->totalPkts = pNetTest->totalErrors = 0;

   while (1)
   {
      pNetTest->args.bufflen = 1;
      pNetTest->args.buff = (char *)malloc(pNetTest->args.bufflen);
      pNetTest->args.buff1 = (char *)malloc(pNetTest->args.bufflen);
      pNetTest->SyncTransmit(&pNetTest->args);
      
      t0 = pNetTest->When();
      t0 = pNetTest->When();
      t0 = pNetTest->When();
      t0 = pNetTest->When();
      for (i = 0; i < LATENCYREPS; i++)
      {
         pNetTest->SendDataTransmit(&pNetTest->args);
         pNetTest->RecvDataTransmit(&pNetTest->args); 
      }
      
      latency = (pNetTest->When() - t0)/(2 * LATENCYREPS);
      free(pNetTest->args.buff);
      pNetTest->args.buff = 0;
      free(pNetTest->args.buff1);
      pNetTest->args.buff1 = 0;
      
      if (pNetTest->verbose)
      {
         pNetTest->SendResultData("Latency: %lf\n", latency);
         pNetTest->SendResultData("Now starting main loop\n");
      }
      tlast = latency;
      if (pNetTest->inc == 0)
      {
   	   /* Set a starting value for the message size increment. */
   	   pNetTest->inc = (pNetTest->start > 1) ? pNetTest->start / 2 : 1;
      }
      
      /* Main loop of benchmark */
      for (n = 0, len = pNetTest->start; 
           n < NSAMP - 3 && len <= pNetTest->end; 
           len = len + pNetTest->inc)
      {        
         /* This is a perturbation loop to test nearby values */
         for (pert = (pNetTest->inc > PERT+1)? -PERT: 0;
              pert <= PERT; 
              n++, pert += (pNetTest->inc > PERT+1)? PERT: PERT+1)
         {

            /* Calculate howmany times to repeat the experiment. */
            nrepeat = (int)MAX((RUNTM / (((double)(((len + pert) <= 0)? 1: (len + pert)) /
				   (double)pNetTest->args.bufflen) * tlast)),
					TRIALS);
            pNetTest->SendRepeatTransmit(&pNetTest->args, nrepeat);

            /* Allocate the buffer */
            pNetTest->args.bufflen = ((len + pert) <= 0)? 1: (len + pert);
            if((pNetTest->args.buff=(char *)malloc(pNetTest->args.bufflen))==(char *)NULL)
            {
                pNetTest->SendResultData("Couldn't allocate memory\n");
                break;
            }
            
            if (pNetTest->verify)
               pNetTest->FillTestPattern(pNetTest->args.buff, pNetTest->args.bufflen, 
                  (currPattern >= MAX_PATTERNS) ? currPattern=0 : currPattern++);
            
            if((pNetTest->args.buff1=(char *)malloc(pNetTest->args.bufflen))==(char *)NULL)
            {
                pNetTest->SendResultData("Couldn't allocate memory\n");
                break;
            }

            /* Finally, we get to transmit and time */
            if (pNetTest->verbose)
					pNetTest->SendResultData("%3d: %9d bytes %4d times --> ",
               n,pNetTest->args.bufflen,nrepeat);
				/*
				This is the transmitter: send the block TRIALS times, and
				expect the receiver to return each block.
				*/
            tlast = LONGTIME;
            for (i = 0; i < 3; i++)
            {
                pNetTest->SyncTransmit(&pNetTest->args);
                t0 = pNetTest->When();
                for (j = 0; j < nrepeat; j++)
                {
                    pNetTest->SendDataTransmit(&pNetTest->args);
                    pNetTest->RecvDataTransmit(&pNetTest->args);
                    pNetTest->totalPkts++;
                    if (pNetTest->verify)
                    {
                        if (pNetTest->VerifyData())
                           pNetTest->totalErrors++;  
                    }
                }
                t = (pNetTest->When() - t0)/(2 * nrepeat);

                tlast = MIN(tlast, t);
            }

				bits = pNetTest->args.bufflen * CHARSIZE;
				bps = bits / (tlast * 1024 * 1024);
            
				if (pNetTest->verbose)
					pNetTest->SendResultData(" %6.2lf Mbps in %lf sec\n", 
                  bps,tlast);
                            
            free(pNetTest->args.buff);
            pNetTest->args.buff = 0;
            free(pNetTest->args.buff1);
            pNetTest->args.buff1 = 0;

          } /* End of perturbation loop */
      } /* End of main loop  */
   } /* Loop forever */     
}


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
double NetworkTestTransmit::When()
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double) tp.tv_sec + (double) tp.tv_nsec * 1e-9);
}


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
int NetworkTestTransmit::SetupTransmit(ArgStruct *p)
{

   int one = 1;
   int sockfd;
   struct sockaddr_in *lsin1, *lsin2;      /* ptr to sockaddr_in in ArgStruct */
   struct hostent *addr;
   char * host;

   host = p->host;
   
   lsin1 = &(p->prot.sin1);
   lsin2 = &(p->prot.sin2);

   bzero((char *) lsin1, sizeof(*lsin1));
   bzero((char *) lsin2, sizeof(*lsin2));

   if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      SendResultData("server: can't open stream socket");
      return(ERROR);
   }

   /* Attempt to set TCP_NODELAY */
   if(setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one)) < 0)
   {
      SendResultData("setsockopt: nodelay\n");
      return(ERROR);
   }

   /* If requested, set the send and receive buffer sizes */
   if(p->prot.sndbufsz > 0)
   {
      SendResultData("Send and Receive Buffers set to %d bytes\n", p->prot.sndbufsz);
      if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&(p->prot.sndbufsz), 
                                       sizeof(p->prot.sndbufsz)) < 0)
      {
          SendResultData("setsockopt: sndbuf\n");
          return(ERROR);
      }
      if(setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&(p->prot.rcvbufsz), 
                                       sizeof(p->prot.rcvbufsz)) < 0)
      {
          SendResultData("setsockopt: rcvbuf\n");
          return(ERROR);
      }
   }

	if (atoi(host) > 0) 
	{                   /* Numerical IP address */
		lsin1->sin_family = AF_INET;
		lsin1->sin_addr.s_addr = inet_addr(host);
	} 

	lsin1->sin_port = htons(p->port);
            
   p->commfd = sockfd;

   return(OK);
 
}   

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
void NetworkTestTransmit::SyncTransmit(ArgStruct *p)
{
   char s[] = "SyncMe";
   char response[7];

   if (send(p->commfd, s, strlen(s), 0) < 0 ||
	   ReadFully(p->commfd, response, strlen(s)) < 0)
   {
	   SendResultData("Transmit: error writing or reading synchronization string");
	   SendStopMsg();
   }
   if (strncmp(s, response, strlen(s)))
   {
	   SendResultData("Transmit: Synchronization string incorrect!\n");
	   SendStopMsg();
   }
}


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
void NetworkTestTransmit::SendDataTransmit(ArgStruct *p)
{
   int bytesWritten, bytesLeft;
   char *q;

   bytesLeft = p->bufflen;
   bytesWritten = 0;
   q = p->buff;
   while (bytesLeft > 0 &&
	  (bytesWritten = send(p->commfd, q, bytesLeft, 0)) > 0)
   {
	   bytesLeft -= bytesWritten;
	   q += bytesWritten;
   }
   if (bytesWritten == -1)
   {
	   SendResultData("Transmit: send: error encountered, errno=%d\n", errno);
	   SendStopMsg();
   }
}


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
void NetworkTestTransmit::RecvDataTransmit(ArgStruct *p)
{
   int bytesLeft;
   int bytesRead;
   char *q;

   bytesLeft = p->bufflen;
   bytesRead = 0;
   q = p->buff1;
   while (bytesLeft > 0 &&
	   (bytesRead = recv(p->commfd, q, bytesLeft, 0)) > 0)
   {
	   bytesLeft -= bytesRead;
	   q += bytesRead;
   }
   if (bytesLeft > 0 && bytesRead == 0)
   {
	   SendResultData("Transmit: \"end of file\" encountered on reading from socket\n");
   }
   else if (bytesRead == -1)
   {
	   SendResultData("Transmit: read: error encountered, errno=%d\n", errno);
	   SendStopMsg();
   }
}


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
void NetworkTestTransmit::SendRepeatTransmit(ArgStruct *p, int rpt)
{
	unsigned long lrpt, nrpt;

	lrpt = rpt;
	/* Send repeat count as a long in network order */
	nrpt = htonl(lrpt);
	if (send(p->commfd, (char *) &nrpt, sizeof(unsigned long), 0) < 0)
   {
      SendResultData("Transmit: send failed in SendRepeat: errno=%d\n", errno);
      SendStopMsg();
   }
}


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
int NetworkTestTransmit::EstablishTransmit(ArgStruct *p)
{
   if(connect(p->commfd, (struct sockaddr *) &(p->prot.sin1),
		sizeof(p->prot.sin1)) < 0)
	{
		SendResultData("Transmit: Cannot Connect! errno=%d\n",errno);
      SendStopMsg();
		return(ERROR);
	}   
}

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
int  NetworkTestTransmit::CleanUpTransmit(ArgStruct *p)
{
   char *quit="QUIT";
   send(p->commfd,quit, 5,0);
   recv(p->commfd,quit,5,0);
   close(p->commfd);
   return(OK);
}


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
char *NetworkTestTransmit::GetDiagnosticHelp(void)
{
   return (
      "NetworkTestTransmit\n\n"
      "Test the integrity and throughput of the network connection.  Must use this"
      "test in conjunction with NetworkTestServer or external client.\n\n"
      "Parameters - host verbose duration verify start end inc bufsize port\n"
      "\thost - host ip address (e.g. 10.1.2.111)\n"
      "\tverbose - If non-zero show individual error occurences.\n"
      "\tduration - Time in seconds to run test (DEFAULT 60)\n"
      "\tverify - verify any errors in the transmitted data (DEFAULT 0)\n"
	   "\tstart - lower bound start value (DEFAULT 1024)\n"
	   "\tend - upper bound stop value (DEFAULT 4096)\n"
	   "\tinc - specify increment step size (DEFAULT 1024)\n"
      "\tbufsize - size of receive and send buffers (DEFAULT OS default)\n"
      "\tport - port number to listen to (DEFAULT 3456)\n"      
      "Returns - Number of error packets\n"
   );
}


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
void  NetworkTestTransmit::SendStopMsg(void)
{
   // Send a message to stop the test.
   DiagnosticMsgType diagMsg;

   // Initialize the diagnostic message.
   diagMsg.func = DIAG_TEST_INTERNAL_STOP_TEST;

   msgQSend(diagQId, 
            (char *)&diagMsg, 
            sizeof(DiagnosticMsgType), 
            WAIT_FOREVER, 
            MSG_PRI_NORMAL);
}



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
int NetworkTestTransmit::ReadFully(int fd, char *obuf, int len)
{
	int bytesLeft = len;
	char *buf = obuf;
	int bytesRead = 0;

	while (bytesLeft > 0 &&
	 (bytesRead = recv(fd, buf, bytesLeft, 0)) > 0)
	{
      bytesLeft -= bytesRead;
      buf += bytesRead;
	}
	if (bytesRead <= 0)
		return bytesRead;
	return len;
}



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
void NetworkTestTransmit::FillTestPattern (char *buf, int bufLen, 
   int patternIndx)
{
   char *p = buf;
   
   int i = 0;
   while (bufLen)
   {
      *p++ = TestPatterns[patternIndx][i++];
      bufLen--;
      if (i >= NUM_PATTERN_BYTES)
         i = 0;
   }
}



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
int NetworkTestTransmit::VerifyData ()
{
   for (int i = 0; i < args.bufflen; i++)
   {
      if (args.buff[i] != (char)(~args.buff1[i])) 
      {
         return 1;
      }
   }
   return 0;
} 
