#ifndef __GenServerSocket_H__
#define __GenServerSocket_H__


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
// File        : GenServerSocket.h
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : GenServerSocket.h: interface for the GenServerSocket class.
//
// *******************************************************************

#include "socket\GenSocketSSL.h"

// Defaults
#define kTCP_LISTEN_BACKLOG      10

// Socket receive buffer size
#define SOC_MRCV_BUFF_LEN       (0x2000)
// Socket send buffer size
#define SOC_MSND_BUFF_LEN       (0x500)


class GenServerSocket : public GenSocketSSL
{
public:
  GenServerSocket ()
    : GenSocketSSL (), port_(0)                              { ; }

  GenServerSocket (bool bEnableSSL, bool bEnableEncryption = true);

  virtual 
 ~GenServerSocket ();

  virtual
  int serve  (const fd_set *, GenSocket *&)
                              { return false; }
  bool open  (ubyte2 Port);

  static bool initSslContext (LPSTR lpPswd = NULL);
  static void cleanSslContext()                   { GenSocketSSL::cleanSslContext(pCtx_); }

protected:
  virtual bool     isContext ();
  virtual SSL_CTX *getContext();

  bool acceptConnection (OS_SPECIFIC_SOCKET_HANDLE& accSoc);
  bool bind2Address     (ubyte2 Port);

private:
  void setPort (ubyte2 Port)  { if (Port) port_ = Port;}

// Data members
protected:

  static SSL_CTX  * pCtx_     ; // SSL context (one per all servers on the host)

  ubyte2 port_;

};



#endif //__GenServerSocket_H__