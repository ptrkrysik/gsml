#ifndef __GENSECSERVERSOCKET_H__
#define __GENSECSERVERSOCKET_H__

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
// File        : GenSecServerSocket.h
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : interface for the GenSecServerSocket (Secondary Server) 
//               class
//
// *******************************************************************

#include "socket\GenServerSocket.h"

#ifndef SOC_RCV_BUFF_LEN
 #define SOC_RCV_BUFF_LEN        (0x2000)
#endif

#ifndef SOC_SND_BUFF_LEN
 #define SOC_SND_BUFF_LEN        (0x500)
#endif

class GenSecServerSocket : public GenServerSocket  
{
public:
// Constructors
    GenSecServerSocket () : GenServerSocket () { ; }

    GenSecServerSocket (OS_SPECIFIC_SOCKET_HANDLE socket);

    // the following constructor contains a blocking SSL API call,
    // when invoked with enableSSL = true
    GenSecServerSocket (OS_SPECIFIC_SOCKET_HANDLE socket, bool enableSSL,
                        bool enableEncryption = true                    );
// Destractor
    virtual
   ~GenSecServerSocket ();

// Primary methods
    virtual 
    int serve      (const fd_set *, GenSocket *&) { return (false); }

private:

    void reset (OS_SPECIFIC_SOCKET_HANDLE socket);
};


#endif //__GENSECSERVERSOCKET_H__