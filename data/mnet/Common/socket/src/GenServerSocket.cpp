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
// File        : GenServerSocket.cpp
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : implementation of the GenServerSocket class.
//
// *******************************************************************

#include "socket\GenServerSocket.h"

//////////////////////////////////////////////////////////////////////
// Static data members
//////////////////////////////////////////////////////////////////////
SSL_CTX *GenServerSocket::pCtx_ = NULL;


//////////////////////////////////////////////////////////////////////
// Constructors/Destructor
//////////////////////////////////////////////////////////////////////
GenServerSocket::GenServerSocket (bool bEnableSSL, bool bEnableEncryption)
    : GenSocketSSL (bEnableSSL, bEnableEncryption), port_(0)
{
    ;
}


GenServerSocket::~GenServerSocket()
{
  AM_FUNC ("GenServerSocket::~GenServerSocket", SOC_LAYER_ID);
  AM_ENTER();

    if (isValid())
    {
        AM_DEBUG (("Closing server socket (%d) on IP (%s,%u)\n",
                   getSocket(), getIp(getLocalHostAddress()), port_));
    }

    // should clean server SSL context once per host

  AM_LEAVE();
}


///////////////////////////////////////////////////////////
// primary methods
///////////////////////////////////////////////////////////

bool GenServerSocket::open (ubyte2 Port) 
{
  AM_FUNC  ("GenServerSocket::open", SOC_LAYER_ID);
  AM_ENTER ();
  bool retVal = true;

  // hard socket close mode
  struct linger lingerVal = {1,0};

    if ((retVal = !isValid()) == false)
    {
        AM_WARNING (("GenServerSocket::open - Attempt to reopen socket (%d).\n",
                     getSocket() ));
    }

    // create socket
    if (retVal)
        retVal = create ();

    // bind socket
    if (retVal)
        retVal = bind2Address (Port);

    // wait for client connection requests
    if (retVal)
    {
        if (listen (getSocket(), kTCP_LISTEN_BACKLOG) == 0)
        {
          int  rcvBuffLen  = SOC_MRCV_BUFF_LEN, 
               sndBuffLen  = SOC_MSND_BUFF_LEN;

            setPort (Port);

            setsockopt (getSocket(), SOL_SOCKET, SO_RCVBUF, (char *)&rcvBuffLen, sizeof(rcvBuffLen));
            setsockopt (getSocket(), SOL_SOCKET, SO_SNDBUF, (char *)&sndBuffLen, sizeof(sndBuffLen));
            setsockopt (getSocket(), SOL_SOCKET, SO_LINGER, (char *)&lingerVal , sizeof(linger    ));

            if (!isEnableSSL () || initSsl ())
            { // initiate SSL session on a valid socket
                AM_TRACE (("Open main server socket (%d) on IP (%s,%u)\n",
                           getSocket(), getIp(getLocalHostAddress()), port_));
            }
            else
            {
                AM_TRACE (("Unable to initiate SSL session on a server socket\n"));
                retVal = false;
            }

#ifdef _AM_DEBUG_
            printInfo  ();
#endif // _AM_DEBUG_
        }
        else
        {
            AM_ERROR (("%s: %sd\n", "listen", strerror(ERROR_NUM)));
            retVal = false;
        }
    }

  AM_RETURN (retVal);
}


SSL_CTX *GenServerSocket::getContext ()
{
    return (pCtx_);
}


bool GenServerSocket::isContext ()
{
    return (pCtx_ != NULL);
}


bool GenServerSocket::initSslContext (LPSTR lpPswd)
{
    return (isSupportSSL() ? GenSocketSSL::initSslContext (pCtx_, false, lpPswd)
                           : true                                               );
}


///////////////////////////////////////////////////////////
// service methods
///////////////////////////////////////////////////////////

bool GenServerSocket::bind2Address (ubyte2 Port) 
{
   AM_FUNC("GenServerSocket::bind2Address", SOC_LAYER_ID);
   AM_ENTER();
      
   struct sockaddr_in  LocalAddr;
   bool                retVal   = false;
   u_long              ulHostAddress;

     if (!isValid())
         AM_RETURN (false);

     /* ask the system to allocate a port and bind to INADDR_ANY */
     memset(&LocalAddr, 0x00, sizeof(LocalAddr));

     if ((ulHostAddress = getLocalHostAddress()) == 0)
         AM_RETURN (false);

     /* get system to allocate a port number by binding a host address */
     LocalAddr.sin_family        = AF_INET;

     /* allows connection on any local interface. Replace it with ulHostAddress 
        for multihomed machine compatibility */
     LocalAddr.sin_addr.s_addr   = htonl(INADDR_ANY);   
     LocalAddr.sin_port          = htons(Port);

     if (bind (getSocket(), (struct sockaddr *)&LocalAddr, sizeof(LocalAddr)) != 0)
     {
         AM_ERROR (("Unable to bind a socket to [%s:%u]\nAnother application using it\n",
                    getIp(getLocalHostAddress()),Port ));
     }
     else
         retVal = true;

   AM_RETURN (retVal);
}


bool GenServerSocket::acceptConnection (OS_SPECIFIC_SOCKET_HANDLE& accSoc)
{
  bool    retVal  = false;
  char    ClientAddr[MAX_PATH];
  int     iAddrLen;

    AM_FUNC("GenServerSocket::acceptConnection", SOC_LAYER_ID);
    AM_ENTER();

    iAddrLen = sizeof(ClientAddr);
    memset(&ClientAddr, 0x00, iAddrLen);

    if ((accSoc = accept (getSocket(), (sockaddr FAR *)&ClientAddr, &iAddrLen))
                != INVALID_SOCKET)
    {
        // Perform 
        retVal = true;
    }
    else
    {
        if (isValid())
        {
            AM_WARNING (("GenServerSocket::acceptConnection: %s\n", strerror(ERROR_NUM) ));
        }
    }

  AM_RETURN (retVal);
}

