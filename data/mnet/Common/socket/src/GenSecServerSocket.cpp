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
// File        : GenSecServerSocket.cpp
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : implementation of the GenSecServerSocket (secondary
//               server socket) class
//
// *******************************************************************

#include "socket\GenSecServerSocket.h"

//////////////////////////////////////////////////////////////////////
// Constructors/Destructor
//////////////////////////////////////////////////////////////////////

/*
 This class should be used ONLY to create "secondary" socket objects:
 Those sockets are created when a server socket accepts a connection
 fromt the client and assigns a new "secondary" socket to handle it. 
 Socket parameter should be a valid socket.
*/

GenSecServerSocket::GenSecServerSocket (OS_SPECIFIC_SOCKET_HANDLE socket)
    : GenServerSocket ()
{
    reset (socket);
}


GenSecServerSocket::GenSecServerSocket (OS_SPECIFIC_SOCKET_HANDLE socket   ,
                                        bool                      enableSSL,
                                        bool                      enableEncryption)
    : GenServerSocket (enableSSL, enableEncryption)
{
  AM_FUNC ("GenSecServerSocket::GenSecServerSocket(SSL)", SOC_LAYER_ID);
  AM_ENTER();

    reset (socket);

    if (isEnableSSL())
    {
        // blocking call to SSL API. USE only for dynamic object creation!!!
        // 1. All the necessary validity checks are performed inside "acceptSsl"
        // 2. will release a socket on error !!!
        if (acceptSsl () == false)
        {
            AM_WARNING (("Server is unable to establish SSL connection with the client on %s\n",
                         getIp(getRemoteHostAddress()) ));
            releaseSocket (true);
        }
    }

    if (bMoreInfo_)
        printInfo ();

  AM_LEAVE();
}


GenSecServerSocket::~GenSecServerSocket()
{
  AM_FUNC ("GenSecServerSocket::~GenSecServerSocket", SOC_LAYER_ID);
  AM_ENTER();

    if (isValid())
    {
        AM_TRACE (("Disconnecting socket (%u) from the client IP(%s) \n", 
                   getSocket(), getIp(getRemoteHostAddress())           ));
    }

  AM_LEAVE();
}


void GenSecServerSocket::reset (OS_SPECIFIC_SOCKET_HANDLE socket)
{
  AM_FUNC ("GenSecServerSocket::reset", SOC_LAYER_ID);
  AM_ENTER();

    if (setSocket (socket))
    {
          //Optional timer to keep alive the connection
        char optVal      = 1               ;
        int  rcvBuffLen  = SOC_RCV_BUFF_LEN, 
             sndBuffLen  = SOC_SND_BUFF_LEN;
        // hard socket close mode
        struct linger lingerVal = {1,0};

        setsockopt (getSocket(), SOL_SOCKET, SO_KEEPALIVE, &optVal            , sizeof(optVal)    );
        setsockopt (getSocket(), SOL_SOCKET, SO_RCVBUF   , (char *)&rcvBuffLen, sizeof(rcvBuffLen));
        setsockopt (getSocket(), SOL_SOCKET, SO_SNDBUF   , (char *)&sndBuffLen, sizeof(sndBuffLen));
        setsockopt (getSocket(), SOL_SOCKET, SO_LINGER   , (char *)&lingerVal , sizeof(linger    ));

        AM_TRACE (("Open new socket %d to IP (%s)\n", 
                   getSocket(), getIp(getRemoteHostAddress()) ));
    }

  AM_LEAVE();
}

