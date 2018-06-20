#ifndef __GENCLIENTSOCKET_H__
#define __GENCLIENTSOCKET_H__

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
// File        : GenClientSocket.h
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : interface for the GenClientSocket class.
//
// *******************************************************************

#include "GenSocketSSL.h"


class GenClientSocket : public GenSocketSSL 
{

public:
    GenClientSocket ();
    GenClientSocket (int sndSize, int rcvSize);

    // allow SSL settings
    GenClientSocket (bool bEnableSSL, bool bEnableEncryption = true);
    GenClientSocket (int  sndSize   , int  rcvSize           ,
                     bool bEnableSSL, bool bEnableEncryption = true);

    virtual 
   ~GenClientSocket () {;}

    virtual 
    int serve          (const fd_set *set, GenSocket *& soc) 
                                      { return (GenSocket::serve(set, soc)) ; }

    void setLinger     (linger& val)  { lingerVal_ = val ; setLinger_ = true; }
    bool connectServer (ubyte4 ipAddr, ubyte2 tcpPort);
    bool reconnect     ();

    virtual void printInfo ();

    static bool initSslContext (LPSTR lpPswd = NULL);
    static void cleanSslContext()                    { GenSocketSSL::cleanSslContext(pCtx_); }

protected:

    virtual bool     create    ();
    virtual bool     isContext ();
    virtual SSL_CTX *getContext();

    void reset ();

private:

// Data members
protected:

    static SSL_CTX  * pCtx_; // SSL context (one per all clients on the host)

    int    rcvBuffLen_,
           sndBuffLen_;
    linger lingerVal_ ;
    bool   setLinger_ ;

    ubyte4 srvAddr_   ;
    ubyte2 srvPort_   ;

};


#endif //__GENCLIENTSOCKET_H__