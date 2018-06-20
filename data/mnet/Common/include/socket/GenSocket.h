#ifndef __GENSOCKET_H__
#define __GENSOCKET_H__


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
// File        : GenSocket.h
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : interface for the GenSocket class.
//
// *******************************************************************

// system includes
#ifdef __VXWORKS__
 #include <vxworks.h>
 #include <stdio.h>
 #include <ctype.h>

 #include <string.h>
 #include <time.h>

 #include <sockLib.h>
 #include <hostLib.h>
 #include <ioLib.h>

 #include "socdefs.h"

#elif (defined(_WINDOWS) || defined(WIN32) )
 #include <iostream.h>
 #include <winsock2.h>
 #include <wsnetbs.h>
 #include <wsipx.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <stdarg.h>

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <direct.h>

 #include <limits.h>
 #include <io.h>
 #include <fcntl.h>

 #include <time.h>

#include "StdAfx.h"

#endif // system includes

#include "defs.h"
#include "CdrDbg.h"


typedef char IpAddrPool_t [20];

typedef enum {
    SOCKET_NOT_CONNECTED      = 0,
    SOCKET_CONNECTED          = 1,
    SOCKET_CONNECTED_READABLE = 2
} SocketState_t;


class GenSocket  
{
public:
           GenSocket();
  virtual ~GenSocket();

  static  ubyte4            getLocalHostAddress ();
  static  const LPSTR       getIp               (ubyte4 ipAddr);

  virtual bool              isValid    () {return (sock_ != INVALID_SOCKET && sock_);}

  // print socket info
  virtual int       tcpReceive         (sbyte *pBuf, int BufSize);
  virtual int       tcpSend            (sbyte *pBuf, int BufSize);
  virtual void      releaseSocket      (bool grace = false      );
  virtual void      printInfo          ();

    // Socket-specific method will test and serve the socket upon need
  virtual int               serve      (const fd_set *, GenSocket *&) 
                                          {return (false);}

          OS_SPECIFIC_SOCKET_HANDLE 
                        getSocket      () {return (sock_);};
          SocketState_t isConnected    ();

protected:

  static  ubyte4  getHostAddress       (LPSTR);
  virtual bool    create               ();
  // close the socket and reset class variables
  virtual void    cleanup              ();

          bool    setSocket            (OS_SPECIFIC_SOCKET_HANDLE Socket);
          ubyte4  getRemoteHostAddress ();

private:
          void    resetSocket          () {sock_ = INVALID_SOCKET;}


// data members
protected:
  static bool               bMoreInfo_;

private:
  OS_SPECIFIC_SOCKET_HANDLE sock_     ;

  // temporary storage for translated IP address
  // will be overwitten with every call using it
  static IpAddrPool_t   szIpAddr_;

  // Used for select write_fd
  static const struct timeval timeOut_;

};



#endif //__GENSOCKET_H__