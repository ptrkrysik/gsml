#ifndef __GENSOCKETSSL_H__
#define __GENSOCKETSSL_H__
// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : GenSocketSSL.h
// Author(s)   : Igal Gutkin
// Create Date : 05/17/01
// Description : interface for the GenSocketSSL class.
//
// *******************************************************************

#include "socket\GenSocket.h"
#include "socket\SslIntf.h"

typedef enum 
{
    ALLSUITES ,
    DESCBC3SHA,
    NOSSL

} CipherSuite_t;

// SSL error codes
typedef enum
{
    SSL_NO_ERROR      = 0,
    SSL_GENERIC_ERROR = 1,
    SSL_BAD_PASSWORD  = 2,
    SSL_BAD_KEY       = 3,
    SSL_BAD_CA_ERROR  = 4,
    SSL_BAD_USER_CERT = 5,
    SSL_BAD_PEER_CERT = 6,

    SSL_GEN_ERROR        ,

    SSL_ERROR_MAX 

} SslError_t;


typedef enum
{
    CertNoError               ,
    CertMiscError             ,
    CertUnspecifiedError      ,
    CertNotYetValid           ,
    CertExpired               ,
    CertNotBeforeError        ,
    CertNotAfterError         ,
    CertCrlLastUpdateError    ,
    CertCrlNextUpdateError    ,
    CertPathLengthExceed      ,
    CertInvalidPurpose        ,
    CertUntrusted             ,
    CertInvalidCA             ,
    CertZeroSelfSigned        , // selfsigned and not trusted
    CertRevoked               ,
    CertOneNotSelfSigned      , // the only certificate in the chain is not selfsigned
    CertNoIssuer              ,
    CertUntrustedNoRoot       , // no local root
    CertUnableDecryptSignature,
    CertUnableDecodeIssuerKey ,
    CertInvalidSignature      ,
    CertNoIssuerCert            // can not find issuer certificate locally

} SslValidationCodes_t;

extern "C"
{
 // Password read\write utilities
 bool encryptPassword (const char *pwd, const char *filename);
 bool decryptPassword (char *pwdout   , const char *filename);

 // Class info
 void SocDataPrint ();

}


class GenSocketSSL : public GenSocket
{
// friends definition
friend int verify_callback (int ok, X509_STORE_CTX *pStoreCtx);

public:

// Constructors & destructor
           GenSocketSSL ()
             : GenSocket         ()     ,
               bEnableSsl_       (false), 
               bEnableEncryption_(false),
               pSsl_             (NULL ),
               pSbio_            (NULL ) { ; }

           GenSocketSSL (bool enableSSL, bool     enableEncryption = true, 
                                         SSL_CTX *pCtx             = NULL)
             : GenSocket         ()     ,
               bEnableSsl_       (isSSL()     ? enableSSL        : false), 
               bEnableEncryption_(bEnableSsl_ ? enableEncryption : false),
               pSsl_             (NULL ),
               pSbio_            (NULL ) { ; }

  virtual ~GenSocketSSL ()               { ; }

  // Primary methods

  static  bool isSupportSSL  () { return (isSSL ()!=0); }
          bool isEnableSSL   () { return (bEnableSsl_); }
          bool isInitSSL     () { return (isValid () && isContext() && pSsl_ && pSbio_); }

  virtual int  tcpReceive    (sbyte *pBuf, int BufSize);

  virtual int  tcpSend       (sbyte *pBuf, int BufSize);

  virtual void releaseSocket (bool   grace = false);
  virtual void printInfo     ();

  static  void classPrintInfo();

  static  SslError_t getError() { return (errorCode_); }

  static void        printVerifyError (long code);

  // password manipulation methods
  static bool   setPassword  (LPCSTR lpPswd);
  static void   resetPassword();
  static LPCSTR getPassword  () { return (szPswd_        ); }
  static bool   isPswdSet    () { return (szPswd_[0] != 0); }

  // configure SSL library
  static  void initSslLib  ();
  static  void cleanSslLib ()   {/*SSL_library_cleanup()*/ ; }

  static  int  getStats    ()   { return (count_)  ; }


protected:

  virtual bool     isContext () = 0;
  virtual SSL_CTX *getContext() = 0;

  static void  setError      (SslError_t code) { errorCode_ = code        ; }
  static void  resetError    (SslError_t code) { errorCode_ = SSL_NO_ERROR; }

  static bool  verifyCert    (long code);
  static LPSTR getCertError  (long code);

  // init\clean SSL context
  // using one shared context per application
  static bool  initSslContext  (SSL_CTX  *& pCtx, bool bServer, LPSTR password = NULL);
  static void  cleanSslContext (SSL_CTX  *& pCtx);

  // init\clean socket-specific SSL resources
  bool    initSsl    ();
  void    cleanSsl   ();

  // handle SSL connection setup
  bool    acceptSsl  ();
  bool    connectSsl ();

  // send\receive data through SSL connection
  int     readSsl    (sbyte *pBuf, int BufSize);
  int     writeSsl   (sbyte *pBuf, int BufSize);

  void    printSession();
  int     getSocNum   ()    { return (count_); }

private:

// data members
protected:

    SSL     *pSsl_  ; // SSL connection
    BIO     *pSbio_ ; // SSL socket wrapper

    static LPCSTR cipherSuite_[];

private:

    static bool       bInitLib_ ;

    static char       szPswd_[SSL_MAX_PASSWORD_LENGTH+2]; // pointer to password string

#ifdef __SSL_USE_TEMP_KEY__
    static RSA      * pRsa_     ;    // temporary key (server only)
#endif //__SSL_USE_TEMP_KEY__

    // keeps the last application error code (application will not reset the code!)
    static SslError_t errorCode_;

    // when false SSL functionality is bypassed
    // otherwise, authentication is enabled and 
    // ciphering is depend on enableEncryption_
    const bool  bEnableSsl_        ;

    // when false, SSL encryption is disabled 
    // but using authentication on connect
    const bool  bEnableEncryption_ ; 

    static char szKeyFile_ [MAX_PATH]; // private key 
    static char szCertFile_[MAX_PATH]; // certificate
    static char szCaFile_  [MAX_PATH]; // trusted CA List

    // Count SSL sockets
    static int  count_;
    
};


#endif //__GENSOCKETSSL_H__