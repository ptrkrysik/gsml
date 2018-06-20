#ifndef __CALLBACKSSL_H__
#define __CALLBACKSSL_H__

//*******************************************************************
//
// (c) Copyright CISCO Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : CallBackSSL.h
// Author(s)   : Igal Gutkin
// Create Date : 5/22/01
// Description : declaration of the service function required by the 
//               SSL library
//
// *******************************************************************

int verify_callback (int   ok , X509_STORE_CTX *ctx);

#if defined (_OPEN_SSL_LIB_)

 int password_cb    (char *buf, int num            ,int rwflag, void *userdata);

#elif defined (_RSA_SSL_LIB_)

 int password_cb    (char *buf, int num            ,int rwflag);

#endif


#endif //__CALLBACKSSL_H__