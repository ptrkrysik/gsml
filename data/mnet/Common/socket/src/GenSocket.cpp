// *******************************************************************
//
// (c) Copyright Cisco Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.0
// Status      : Under development
// File        : GenSocket.cpp
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : implementation of the GenSocket class.
//
// *******************************************************************

#include "socdefs.h"
#include "socket\GenSocket.h"



//////////////////////////////////////////////////////////////////////
// static data members
//////////////////////////////////////////////////////////////////////
IpAddrPool_t GenSocket::szIpAddr_;

const struct timeval GenSocket::timeOut_ = {0, 0};

bool GenSocket::bMoreInfo_ = 
#ifdef _AM_DEBUG_
        true ;
#else
        false;
#endif // _AM_DEBUG_

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GenSocket::GenSocket() 
    : sock_ (INVALID_SOCKET)
{
    ;
}


GenSocket::~GenSocket()
{
    cleanup ();
}



//////////////////////////////////////////////////////////////////////
// static methods 
//////////////////////////////////////////////////////////////////////

// Retrives local host IP address
ubyte4 GenSocket::getLocalHostAddress()
{
  ubyte4  ulHostAddr = 0;
  sbyte   szHostName[MAX_PATH];

  AM_FUNC  ("GenSocket::getLocalHostAddress", SOC_LAYER_ID);
  AM_ENTER ();

    // get local host name
    if (gethostname (szHostName, sizeof(szHostName)) != SOC_STATUS_ERROR)
    {
        ulHostAddr = getHostAddress (szHostName);
    } 
    else 
    {
        AM_ERROR(("%s: %s\n", "GenSocket::getLocalHostAddress", strerror(ERROR_NUM) ));
    }

  AM_LEAVE();
  return  (ulHostAddr);
}


// Converts IP from HEX to dotted form
const LPSTR GenSocket::getIp (ubyte4 ipAddr)
{
    memset (szIpAddr_, 0, sizeof szIpAddr_);

    sprintf (szIpAddr_, "%d.%d.%d.%d", IP_BYTES (ipAddr));

  return (szIpAddr_);
}



//////////////////////////////////////////////////////////////////////
// service methods 
//////////////////////////////////////////////////////////////////////

SocketState_t GenSocket::isConnected ()
{
  SocketState_t retVal  = SOCKET_NOT_CONNECTED;
  fd_set        readFds ;
  int           tempVal ;
  char          tempBuff [32];

  AM_FUNC  ("GenSocket::isConnected", SOC_LAYER_ID);
  AM_ENTER ();

    if (isValid())
    {
        FD_ZERO (&readFds);
        FD_SET  (getSocket(), &readFds);

        /*
        Readability means that queued data is available for reading such 
        that a call to recv, WSARecv, WSARecvFrom, or recvfrom is 
        guaranteed not to block.
        For connection-oriented sockets, readability can also indicate that
        a request to close the socket has been received from the peer. If 
        the virtual circuit was closed gracefully, and all data was received,
        then a recv will return immediately with zero bytes read. If the 
        virtual circuit was reset, then a recv will complete immediately with
        an error code such as WSAECONNRESET. The presence of out-of-band data
        will be checked if the socket option SO_OOBINLINE has been enabled. 

        1. select returns  -1, no connection
       +2. select returns   0, socket isn't ready to read - connection is OK
        3. select returns > 0, socket is ready to read - check it with recv
            1. recv return 0  OR
               recv return SOCKET_ERROR and errno != WSAEMSGSIZE 
                    socket is closed.
            2. recv return SOCKET_ERROR and errno == WSAEMSGSIZE OR 
               recv return > 1, there is a data on the socket 
                    If the data is unexpected, reset the socket to avoid 
                    potential security violation in the child class.
        */
        tempVal = select (1, &readFds, NULL, NULL, (struct timeval *)&timeOut_);


        switch (tempVal)
        {
        case SOC_STATUS_ERROR:
            retVal = SOCKET_NOT_CONNECTED;
            break;

        case 0:
            retVal = SOCKET_CONNECTED;
            break;

        default:
            tempVal = recv (getSocket(), tempBuff, sizeof (tempBuff), MSG_PEEK);

            retVal = (!tempVal || (tempVal == SOC_STATUS_ERROR && ERROR_NUM != WSAEMSGSIZE))
                     ? SOCKET_NOT_CONNECTED
                     : SOCKET_CONNECTED_READABLE;
            break;
        }
    }

    // AM_DEBUG (("Socket %u is %s connected.\n", getSocket(), retVal ? "" : " not"));

  AM_LEAVE();
  return  (retVal);
}


bool GenSocket::create()
{
  bool                      retVal   = false;
  OS_SPECIFIC_SOCKET_HANDLE newSocket;

  AM_FUNC  ("GenSocket::create", SOC_LAYER_ID);
  AM_ENTER ();

    if (!isValid ())
    {
        if ((newSocket = socket(AF_INET, SOCK_STREAM, 0)) != INVALID_SOCKET)
        {
            retVal = setSocket (newSocket);
            AM_DEBUG (("Open new socket %u\n", getSocket()));
        }
        else
        {
            AM_ERROR(("GenSocket::create: %s\n", strerror(ERROR_NUM) ));
        }
    }
    else
    {
        retVal = true;
        AM_WARNING (("Attempt to reinitialize socket.\n"));
    }

  AM_LEAVE();
  return  (retVal);
}


// Returns a host address by the host name
ubyte4 GenSocket::getHostAddress (LPSTR HostName)
{
  ubyte4 ipAddr;

#ifdef _WINDOWS_
    struct hostent *pHostAddr;
    pHostAddr = gethostbyname (HostName);
    ipAddr    = (pHostAddr) ? (*((ubyte4 *)pHostAddr->h_addr)) : 0;
#else
    ipAddr = hostGetByName (HostName);
#endif

    return (ipAddr);
}


// Returns remote (peer) host address
ubyte4 GenSocket::getRemoteHostAddress ()
{
  sockaddr_in  ClientAddr   ;
  ubyte4       remIpAddr = 0;
  int          iAddrLen  = sizeof(ClientAddr);

  AM_FUNC  ("GenSocket::getRemoteHostAddress", SOC_LAYER_ID);
  AM_ENTER ();

    if (isValid ())
    {
        if (SOC_STATUS_OK == getpeername (getSocket(), (struct sockaddr *)&ClientAddr, &iAddrLen))
            remIpAddr = ClientAddr.sin_addr.s_addr;
        else
        {
            remIpAddr = 0;

            AM_TRACE (("getpeername() error %s on socket %d\n", 
                       strerror(ERROR_NUM), getSocket() ));
        }
    }

  AM_LEAVE();
  return  (remIpAddr);
}


bool GenSocket::setSocket (OS_SPECIFIC_SOCKET_HANDLE Socket)
{
  bool retVal = false;

    if (!isValid() && Socket != INVALID_SOCKET && Socket)
    {
        sock_  = Socket;
        retVal = true  ;
    }

  return (retVal);
}


int GenSocket::tcpReceive (sbyte *pBuf, int BufSize)
{
  int MsgLen = SOC_STATUS_ERROR;

  AM_FUNC  ("GenSocket::tcpReceive", SOC_LAYER_ID);
  AM_ENTER ();

    if (isValid())
    {
        MsgLen = recv (getSocket(), pBuf, BufSize, 0);

        switch (MsgLen)
        {
        case 0:
            AM_TRACE (("GenSocket::tcpReceive: Client closed connection on socket (%d)\n",
                       getSocket()));
            break;

        case SOC_STATUS_ERROR:
            AM_TRACE (("GenSocket::tcpReceive: recv error %s on socket (%d)\n",
                       strerror(ERROR_NUM), getSocket() ));
            break;

        default:
            AM_DEBUG   (("GenSocket::tcpReceive: message received - total length (%d)\n", MsgLen));
            AM_HEXDUMP ((ubyte *)pBuf, MsgLen);
            break;
        }
    }

  AM_LEAVE();
  return  (MsgLen);
}


int GenSocket::tcpSend (sbyte* pBuff, int inDataLen)
{
  AM_FUNC  ("GenSocket::tcpSend", SOC_LAYER_ID);
  AM_ENTER ();

  int retVal = SOCKET_STATUS_FATAL_SOC_ERR;
  int sentDataLen, socError;
  

    if (isValid ())
    {
        sentDataLen = send (getSocket(), pBuff, inDataLen, 0);
        
        if (sentDataLen != SOC_STATUS_ERROR)
        { 
        /* Checks that ALL the data is sent. 
        If blocking mode is used all the data is sent.
            For non-blocking mode it could be less. */
            retVal = SOCKET_STATUS_OK;

            AM_DEBUG (("tcpSend: Sent %d bytes to socket %d\n", sentDataLen, 
                       getSocket()));
        }
        else
        {
            socError = ERROR_NUM;
            
            switch (socError)
            {
            case WSAENOBUFS:
                retVal = SOCKET_STATUS_TEMP_SOC_ERR;
                break;
                
            case WSAEMSGSIZE:
                retVal = SOCKET_STATUS_MSG_TOO_LONG;
                AM_TRACE (("tcpSend: Data length %u exciding maximum allowed by the network.\n",
                           inDataLen));
                break;
                
            default:
                retVal = SOCKET_STATUS_FATAL_SOC_ERR; // fatal error, the connection is broken
            }
        }
    }
    else
    {
        AM_TRACE (("Socket is not connected. Send has failed.\n"));
    }

  AM_LEAVE();
  return  (retVal);
}


void GenSocket::releaseSocket (bool grace)
{
  AM_FUNC  ("GenSocket::releaseSocket", SOC_LAYER_ID);
  AM_ENTER ();

    if (isValid())
    {
        if (grace)
        {
            // Optional "grace" closing of the socket will wait 
            // till all pending data is sent
            shutdown (getSocket(), SD_BOTH);
        }

        AM_DEBUG (("Closing socket %u\n", getSocket()));

        SysCloseSocket (getSocket());
        // Put optional diagnostics here

        resetSocket ();
    }

  AM_LEAVE();
}


void GenSocket::cleanup()
{
    releaseSocket (true);
}


void GenSocket::printInfo()
{
    if (isValid())
    {
      sbyte4 rcvBuffLen  = 0, 
             sndBuffLen  = 0;
      int    buffSize    = 4;
     
        printf ("Socket Info: socket %d.\n", getSocket());

        if (bMoreInfo_)
        {
            getsockopt (getSocket(), SOL_SOCKET, SO_RCVBUF, (char *)&rcvBuffLen , &buffSize);
            getsockopt (getSocket(), SOL_SOCKET, SO_SNDBUF, (char *)&sndBuffLen , &buffSize);

            printf ("Socket receive buffer is %ld Byte, send buffer is %ld Byte\n",
                    rcvBuffLen, sndBuffLen  );
        }
    }
    else
    {
        printf ("Socket is not initialized\n");
    }
}

