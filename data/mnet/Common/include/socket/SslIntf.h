#ifndef __SSLINTF_H__
#define __SSLINTF_H__
// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : SslIntf.h
// Author(s)   : Igal Gutkin
// Create Date : 05/17/01
// Description : generic interface between SSL Socket library and SSL 
//               implementations
//
// *******************************************************************

#if defined (_OPEN_SSL_LIB_)
 #include <openssl/rand.h>
 #include <openssl/ssl.h>
 #include <openssl/bio.h>
 #include <openssl/err.h>

#elif defined (_RSA_SSL_LIB_)
 #include "rsa/ssl_rsa.h"
 
#endif

#define SSL_MIN_PASSWORD_LENGTH 4
#define SSL_MAX_PASSWORD_LENGTH 32  // must be a multiple of LENGTH_OF_CBLOCK
#define LENGTH_OF_CBLOCK 8

// specifies whether SSL is supported by the library (false for the stub)

extern "C" { int isSSL (); }


#endif //__SSLINTF_H__