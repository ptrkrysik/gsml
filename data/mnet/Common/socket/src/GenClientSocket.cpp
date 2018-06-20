// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : GenClientSocket.cpp
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : implementation of the GenClientSocket class
//
// *******************************************************************


#include "socket\GenClientSocket.h"

//////////////////////////////////////////////////////////////////////
// Static data members
//////////////////////////////////////////////////////////////////////
SSL_CTX *GenClientSocket::pCtx_ = NULL;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GenClientSocket::GenClientSocket()
    : GenSocketSSL ()
{
    reset ();
}


GenClientSocket::GenClientSocket (int sndSize, int rcvSize)
    : GenSocketSSL ()
{
    reset ();

    sndBuffLen_  = sndSize;
    rcvBuffLen_  = rcvSize;
}


GenClientSocket::GenClientSocket (bool bEnableSSL, bool bEnableEncryption)
    : GenSocketSSL (bEnableSSL, bEnableEncryption)
{
    reset ();
}


GenClientSocket::GenClientSocket (int sndSize, int rcvSize, bool bEnableSSL, bool bEnableEncryption)
    : GenSocketSSL (bEnableSSL, bEnableEncryption)
{
    reset ();

    sndBuffLen_  = sndSize;
    rcvBuffLen_  = rcvSize;
}


//////////////////////////////////////////////////////////////////////
// Primary behaviour methods
//////////////////////////////////////////////////////////////////////

bool GenClientSocket::connectServer (ubyte4 ipAddr, ubyte2 tcpPort)
{
  bool retVal = false;

  struct sockaddr_in remAddr;

  AM_FUNC  ("GenClientSocket::connectServer", SOC_LAYER_ID);
  AM_ENTER ();

    if (isConnected() != SOCKET_CONNECTED)
    {
        if (create ()) // Create a socket
        {
            srvPort_ = tcpPort;
            srvAddr_ = ipAddr ;

            memset (&remAddr, 0, sizeof(remAddr));

            remAddr.sin_family      = AF_INET        ;
            remAddr.sin_addr.s_addr = srvAddr_       ;
            remAddr.sin_port        = htons(srvPort_);

            if ((connect (getSocket(), (struct sockaddr *)&remAddr, sizeof(remAddr))
                        != SOC_STATUS_ERROR) )
            {
                if (isEnableSSL())
                {
                    if (connectSsl ())
                    {
                        retVal = true;
                        AM_TRACE (("SSL Client is connected to %s\n",
                                   getIp (srvAddr_)                 ));
                    }
                    else
                    {
                        AM_WARNING (("SSL Client is unable to connect to %s\n",
                                     getIp (srvAddr_)                         ));
                    }
                }
                else
                {
                    retVal = true;
                    AM_TRACE (("TCP Client is connected to: %s,%u\n", 
                               getIp(srvAddr_), srvPort_            ));
                }
            }
            else
            {
                AM_WARNING (("Socket %u error (%s) on connecting to TCP Server %s\n", 
                             getSocket(), strerror(ERROR_NUM), getIp(srvAddr_)));
            }

            if (!retVal)
            {  // release the socket if not connected
                releaseSocket (true);
            }
        }
    }
    else
    {
        AM_TRACE (("Invalid attempt to reinitialize an active IP connection.\n"));
        retVal = true;
    }

  AM_RETURN (retVal);
}


bool GenClientSocket::reconnect()
{
  bool retVal = false;

  AM_FUNC  ("GenClientSocket::reconnect", SOC_LAYER_ID);
  AM_ENTER ();

    releaseSocket (true);

    retVal = connectServer (srvAddr_, srvPort_);

  AM_RETURN (retVal);
}


bool GenClientSocket::create ()
{
  bool retVal   = false;
  int  buffSize = 4    ;
     
  AM_FUNC  ("GenClientSocket::create", SOC_LAYER_ID);
  AM_ENTER ();

    if (GenSocket::create())
    {
        if (rcvBuffLen_ > 0)
            if (setsockopt (getSocket()         , SOL_SOCKET, SO_RCVBUF, 
                            (char *)&rcvBuffLen_, buffSize             ) == SOC_STATUS_ERROR)
            {
                AM_TRACE (("Unable to set SO_RCVBUF socket option\n"));
            }

        if (rcvBuffLen_ > 0)
            if (setsockopt (getSocket()         , SOL_SOCKET, SO_SNDBUF, 
                            (char *)&sndBuffLen_, buffSize             ) == SOC_STATUS_ERROR)
            {
                AM_TRACE (("Unable to set SO_SNDBUF socket option\n"));
            }

        if (setLinger_)
            setsockopt (getSocket()        , SOL_SOCKET, SO_LINGER,
                        (char *)&lingerVal_, sizeof(linger)       );

        retVal = true;
    }

  AM_RETURN (retVal);
}


bool GenClientSocket::isContext ()
{
    return (pCtx_ != NULL);
}


bool GenClientSocket::initSslContext (LPSTR lpPswd)
{
    return (isSupportSSL() ? GenSocketSSL::initSslContext (pCtx_, false, lpPswd)
                           : true                                               );
}


SSL_CTX *GenClientSocket::getContext ()
{
    return (pCtx_);
}


void GenClientSocket::printInfo()
{
    GenSocketSSL::printInfo();

    if (isConnected())
    {
        printf ("Client Socket Info: socket %u is connected to IP (%s,%d).\n",
                getSocket(), getIp(srvAddr_), srvPort_);
    }
    else
    {
        printf ("Client Socket Info: socket is not connected to the server.\n");
    }
}


//////////////////////////////////////////////////////////////////////
// service methods
//////////////////////////////////////////////////////////////////////


void GenClientSocket::reset()
{
    srvPort_    = 0 ;
    srvAddr_    = 0 ;
    sndBuffLen_ = -1;
    rcvBuffLen_ = -1;
    setLinger_  = false;
}

