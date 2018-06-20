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
// File        : GenSocketSSL_lib.cpp
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : Implementation GenSocketSSL class methods 
//               
//
// *******************************************************************

#include "socket\GenSocketSSL.h"
#include "CallBackSSL.h"

// maximum depth of the certificates chain to explore
#define MAX_ALLOWED_CERTIFICATE_DEPTH   10

// the length of the generated RSA key
#define RSA_KEY_LENGTH                  1024


// Base names fot the SSL files
#define SSL_KEY_FILE    "key.pem"
#define SSL_CERT_FILE   "cert.pem"
#define SSL_CA_FILE     "ca.pem"


//////////////////////////////////////////////////////////////////////
// Static data
//////////////////////////////////////////////////////////////////////

#if defined(__VXWORKS__) && defined (__USE_SSL_LOCK__)
 #include "Os\JCMutex.h"
 static JCMutex socMutex;
#endif


///////////////////////////////////////////////////////////
// Constructors\destractor
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// SSL library setup functions
///////////////////////////////////////////////////////////
void GenSocketSSL::initSslLib ()
{
  AM_FUNC  ("GenSocketSSL::initSslLib", SOC_LAYER_ID);
  AM_ENTER ();

    //  Global system initialization one-per-application
    if (isSupportSSL() && !bInitLib_)
    {
      char rand_seed [] = "this is to make him think that there is some randomness";
      //int  randStat;
      LPSTR  pRoot  = NULL;
      size_t length = 0   ;

        SSL_library_init ();
        
        RAND_seed ((unsigned char *)rand_seed, sizeof(rand_seed));

        //randStat = RAND_status ();

        // Construct full pathname for the SSL required files
        pRoot = getenv ("MNET_BASE");

        if (!pRoot || !*pRoot || (strlen (pRoot) > MAX_PATH - 10))
        {
            pRoot = DEF_ROOT_DIR;
            AM_WARNING (("MNET root directory \"MNET_BASE\" is not specified or too long. Use default value.\n"));
        }

        if (pRoot)
            length = strlen(pRoot);

        sprintf (szKeyFile_ , "%s%c%s", pRoot, ((length) ? DIR_DELIMITER : '\0'), SSL_KEY_FILE );
        sprintf (szCertFile_, "%s%c%s", pRoot, ((length) ? DIR_DELIMITER : '\0'), SSL_CERT_FILE);
        sprintf (szCaFile_  , "%s%c%s", pRoot, ((length) ? DIR_DELIMITER : '\0'), SSL_CA_FILE  );

        bInitLib_ = true;
    }

  AM_LEAVE();
}


///////////////////////////////////////////////////////////
// SSL context setup functions (one per server\client)
///////////////////////////////////////////////////////////
bool GenSocketSSL::initSslContext (SSL_CTX *& pCtx, bool bServer, LPSTR lpPswd)
{
  AM_FUNC  ("GenSocketSSL::initSslContext", SOC_LAYER_ID);
  AM_ENTER ();

    if (pCtx)
    { // already initialized
        AM_RETURN (true);
    }

    if (!isSupportSSL())
    {
        AM_RETURN (true);
    }

  bool        retVal    = true;
  int         IDContext = 1   ;
  int         iRetVal         ;
  SSL_METHOD *meth            ;

    initSslLib ();

#if defined(__VXWORKS__) && defined (__USE_SSL_LOCK__)
    socMutex.take();
#endif

    if (lpPswd)
        setPassword (lpPswd);

    // Check that the password is set and of correct value
    if (!isPswdSet())
    {
        setError   (SSL_BAD_PASSWORD);
        AM_WARNING (("initSslContext: bad or missing SSL password\n"));
        retVal = false;
    }

    if (retVal)
    {
        //meth  = (bServer) ? SSLv3_server_method () : SSLv3_client_method ();
        meth  = SSLv3_method ();

        pCtx = SSL_CTX_new       (meth);
        SSL_CTX_set_verify_depth (pCtx, MAX_ALLOWED_CERTIFICATE_DEPTH);

        /* Load the keys and certificates */
        if (!(SSL_CTX_use_certificate_file (pCtx, (LPSTR)szCertFile_, SSL_FILETYPE_PEM)))
        {
            setError  (SSL_BAD_USER_CERT);
            AM_ERROR  (("Unable to read certificate file %s\n", szCertFile_));
            retVal = false;
        }
    }

    if (retVal)
    {
        SSL_CTX_set_default_passwd_cb (pCtx, 
#ifdef _RSA_SSL_LIB_
                                       (FUNCPTR)
#endif // _RSA_SSL_LIB_
                                       password_cb);

        /* load the private key from the file */
        if (SSL_CTX_use_PrivateKey_file (pCtx, (LPSTR)szKeyFile_, SSL_FILETYPE_PEM))
        {
            /* check that the certificate and private key match */
            if (!SSL_CTX_check_private_key (pCtx))
            {
                setError  (SSL_BAD_KEY);
                AM_ERROR  (("Private key does not match the user certificate\n"));
                retVal = false;
            }
        }
        else
        {
            setError  (SSL_BAD_KEY);
            AM_ERROR  (("Unable to read SSL key file %s\n", szKeyFile_));
            retVal = false;
        }
    }

    if (retVal)
    {
        /* Load the CAs we trust*/
        if (!(SSL_CTX_load_verify_locations (pCtx, (LPSTR)szCaFile_, 0)))
        {
            setError  (SSL_BAD_CA_ERROR);
            AM_ERROR  (("Unable to read CA list from %s file\n", szCaFile_));
            retVal = false;
        }
    }

    if (retVal)
    {
#ifdef _OPEN_SSL_LIB_
        // very useful
        SSL_CTX_set_mode (pCtx, SSL_MODE_AUTO_RETRY);
#endif _OPEN_SSL_LIB_

        if (!(iRetVal = SSL_CTX_set_cipher_list (pCtx,(LPSTR)cipherSuite_[ALLSUITES])) )
        {
            setError   (SSL_GENERIC_ERROR);
            AM_ERROR   (("Cipher setting failed for the context\n"));
            retVal = false;
        }
    }

#ifdef __SSL_USE_TEMP_KEY__
    if (retVal && bServer)
    {
        // generate context key
        pRsa_ = RSA_generate_key (RSA_KEY_LENGTH, RSA_F4, NULL, NULL);

        if (pRsa_)
        {
            if (!SSL_CTX_set_tmp_rsa (pCtx, pRsa_))
            {
                AM_TRACE (("Unable to set temporary key\n"));
                retVal = false;
            }
        }
        else
        {
            AM_TRACE (("Unable to generate temporary key\n"));
            retVal = false;
        }

        if (!retVal)
        {
            setError (SSL_GEN_ERROR);
            retVal = false;
        }
    }
#endif // __SSL_USE_TEMP_KEY__

    if (retVal)
    {
        SSL_CTX_set_session_id_context (pCtx, (const unsigned char*)&IDContext,
                                        sizeof(IDContext)                     );
        retVal = true;

        AM_TRACE (("SSL context is initialized\n"));
    }

    if (!retVal)
        cleanSslContext (pCtx);

#if defined(__VXWORKS__) && defined (__USE_SSL_LOCK__)
    socMutex.give();
#endif

  AM_RETURN (retVal);
}


void GenSocketSSL::cleanSslContext (SSL_CTX *& pCtx)
{
  AM_FUNC  ("GenSocketSSL::cleanSslContext", SOC_LAYER_ID);
  AM_ENTER ();

    if (isSupportSSL())
    {
        if (pCtx)
        {
#if defined(__VXWORKS__) && defined (__USE_SSL_LOCK__)
            socMutex.take();
#endif
            SSL_CTX_free (pCtx);
            pCtx = NULL;
#if defined(__VXWORKS__) && defined (__USE_SSL_LOCK__)
            socMutex.give();
#endif
        }
    
#ifdef __SSL_USE_TEMP_KEY__
        if (pRsa_)
            RSA_free (pRsa_);
#endif // __SSL_USE_TEMP_KEY__
    }
}


///////////////////////////////////////////////////////////
// SSL connection setup functions
///////////////////////////////////////////////////////////

bool GenSocketSSL::initSsl ()
{
  bool retVal = false;

  AM_FUNC  ("GenSocketSSL::initSsl", SOC_LAYER_ID);
  AM_ENTER ();

    if (!isEnableSSL())
    {
        retVal = true;
    }
    else if (isValid() && isContext())
    { // Start it for a valid socket under initialized context
        if (!pSbio_)
        { // create new socket wrapper
            AM_DEBUG (("Creating BIO for a socket\n"));
            pSbio_ = BIO_new_socket (getSocket(), BIO_NOCLOSE);
        }

        if (pSbio_ && !pSsl_)
        {
            AM_DEBUG (("Creating new SSL connection\n"));

#if defined(__VXWORKS__) && defined (__USE_SSL_LOCK__)
    socMutex.take();
#endif
            if ((pSsl_ = SSL_new(getContext())) != NULL)
            {
                AM_DEBUG (("Set SSL session\n"));

                if (SSL_set_session (pSsl_, NULL))
                {
                    SSL_set_bio     (pSsl_, pSbio_, pSbio_);
                    ++count_;
                }
                else
                {
                    AM_TRACE (("Unable to set SSL session\n"));
                    cleanSsl ();
                }
            }

#if defined(__VXWORKS__) && defined (__USE_SSL_LOCK__)
    socMutex.give();
#endif

        }

        if (pSsl_ && pSbio_)
            retVal = true;
        else
        {
            cleanSsl   ();
            AM_WARNING (("Unable to create SSL session\n"));
        }
    }
    else
        AM_WARNING (("Unable to start SSL handshake.\n"));

  AM_RETURN (retVal);
}


void GenSocketSSL::cleanSsl ()
{
  AM_FUNC  ("GenSocketSSL::cleanSsl", SOC_LAYER_ID);
  AM_ENTER ();

    if (isEnableSSL())
    {
        if (pSsl_)
        {
            SSL_shutdown (pSsl_); 
            SSL_clear    (pSsl_);
            SSL_free     (pSsl_);
            pSsl_  = NULL;
            pSbio_ = NULL; // Released by the SSL_free() ???
            --count_;
        }

        if (!pSbio_)
        {
            BIO_free (pSbio_);
            pSbio_ = NULL;
        }
    }
}



///////////////////////////////////////////////////////////
// SSL connect function
///////////////////////////////////////////////////////////

// SSL Client 
bool GenSocketSSL::connectSsl ()
{
  AM_FUNC  ("GenSocketSSL::connectSsl", SOC_LAYER_ID);
  AM_ENTER ();

  bool          retVal    = false;
  int           stat      ;
  CipherSuite_t suiteType = (bEnableEncryption_ ? DESCBC3SHA : NOSSL);

    if (isEnableSSL())
    {
        if (initSsl ())
        {
#ifdef _RSA_SSL_LIB_
            // RSA lib requires to set state explicitly
            SSL_set_connect_state (pSsl_);
#endif // _RSA_SSL_LIB_

            if (!SSL_set_cipher_list(pSsl_,(LPSTR)cipherSuite_[suiteType]) )
            {
                AM_ERROR  (("Set cipher list for socket connection failed\n"));
                AM_RETURN (retVal);
            }

            /*  Check peer certificate against the local list here.
                The chain length is automatically checked by SSL when we
                set the verify depth in the ctx. 
                Check is stoped when trusted certificate is found or 
                when reachingthe maximum specified chain depth.           */
            SSL_CTX_set_verify   (getContext()                                     ,
                                  SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
#ifdef _RSA_SSL_LIB_
                                  (FUNCPTR)
#endif // _RSA_SSL_LIB_
                                  verify_callback                                  );

            //This is where the handshake really occurs
            if ((stat = SSL_connect(pSsl_)) > 0)
            {   // normal case
                // sometimes certificate verificxation error does not prevent TSL session establishment
                if (verifyCert (SSL_get_verify_result(pSsl_)) == false)
                {
                    AM_ERROR (("Server [%s] certificate verification has failed\n Cause: %s\n", 
                               getIp(getRemoteHostAddress()),
                               getCertError(SSL_get_verify_result(pSsl_))            ));
                    setError (SSL_BAD_PEER_CERT);
                }
                else
                {
                    // connection is extablished, peer's certificate is verified
                    AM_TRACE (("SSL Connection is established to [%s] using ciphering %s\n",
                        getIp(getRemoteHostAddress())                    ,
                        SSL_CIPHER_get_name(SSL_get_current_cipher(pSsl_)) ));
                    retVal = true;
                }
            }
            else if (stat == 0)
            {
                AM_ERROR (("Server on [%s] closed connection on the socket %d\n",
                           getIp(getRemoteHostAddress()), getSocket()           ));
            }
            else
            {   // (stat < 0)
                // connection error. Can't continue anyway
                switch (SSL_get_error(pSsl_,stat))
                {
                case SSL_ERROR_NONE:
                    AM_TRACE (("SSL connection is established on socket %u\n", getSocket() ));
                    retVal = true;
                    break;

                case SSL_ERROR_ZERO_RETURN:
                    // peer shutdown connection
                    AM_ERROR (("Server on [%s] closed connection on the socket %d\n",
                               getIp(getRemoteHostAddress()), getSocket()      ));
                    break;

                case SSL_ERROR_WANT_READ:
                    AM_ERROR (("connectSsl: Data needs to be read, problem due to renegotiation\n"));
                    break;

                case SSL_ERROR_WANT_WRITE:
                    AM_ERROR (("connectSsl: Data needs to be written, problem due to renegotiation\n"));
                    break;

                case SSL_ERROR_WANT_X509_LOOKUP:
                    AM_TRACE (("connectSsl: The operation did not complete because an \
application callback set by SSL_CTX_set_client_cert_cb() \
has askedto be called again. The TLS/SSL I/O function \
should becalled again later\n"));
                    break;

                case SSL_ERROR_SYSCALL:
                    AM_TRACE (("SSL system call error on SSL_connect\n"));
                    break;

                case SSL_ERROR_SSL:
                    // General error
                    // check if the peer's certificate is OK
                    // print verification error
                    if (verifyCert(SSL_get_verify_result(pSsl_)) == false)
                    {
                        AM_ERROR (("Server [%s] certificate verification has failed\n Cause: %s\n",
                                   getIp(getRemoteHostAddress()),
                                   getCertError(SSL_get_verify_result(pSsl_))            ));
                        setError (SSL_BAD_PEER_CERT);
                    }
                    else
                    {
                        AM_TRACE (("General SSL error during on SSL connect\n"));
                    }
                    break;

                default:
                    AM_TRACE (("Unspecified SSL connect error %d\n", stat));
                    break;
                }
            }
        }
    }
    else
    { // SSL is disabled or not supported
        retVal = true;
    }

  AM_RETURN (retVal);
}


// SSL server 
bool GenSocketSSL::acceptSsl ()
{
  int stat;
  bool retVal = false;

  AM_FUNC  ("GenSocketSSL::acceptSsl", SOC_LAYER_ID);
  AM_ENTER ();

    if (!isEnableSSL())
    {
        retVal = true;
    }
    else if (initSsl ())
    {
#ifdef _RSA_SSL_LIB_
       // SSL_set_accept_state (pSsl_);
#endif // _RSA_SSL_LIB_

        // accept state is the default one

        /*  Check peer certificate against the local list here.
            The chain length is automatically checked by SSL when we
            set the verify depth in the ctx. 
            Check is stoped when trusted certificate is found or 
            when reachingthe maximum specified chain depth.           */
        SSL_CTX_set_verify (getContext(), SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
#ifdef _RSA_SSL_LIB_
                            (FUNCPTR)
#endif // _RSA_SSL_LIB_
                            verify_callback);

        //This is where the handshake really occurs
        if ((stat = SSL_accept(pSsl_)) > 0)
        {
            if (verifyCert(SSL_get_verify_result(pSsl_)) == false)
            {
                AM_ERROR (("Client [%s] certificate verification has failed.\nCause: %s\n",
                    getIp        (getRemoteHostAddress()),
                    getCertError (SSL_get_verify_result(pSsl_))                 ));
                setError (SSL_BAD_PEER_CERT);
            }
            else
            {
                // connection is extablished, peer's certificate is OK
                AM_TRACE (("SSL Connection is accepted from [%s] using ciphering %s\n",
                           getIp(getRemoteHostAddress())                    , 
                           SSL_CIPHER_get_name(SSL_get_current_cipher(pSsl_)) ));
                retVal = true;
            }
        }
        else if (stat == 0)
        {
            AM_ERROR (("Client on [%s] closed connection on the socket %d\n",
                       getIp(getRemoteHostAddress()), getSocket()           ));
        }
        else if (stat < 0)
        {
            switch (SSL_get_error (pSsl_, stat) )
            {
            case SSL_ERROR_NONE:
                AM_TRACE (("SSL connection is accepted on socket %u\n", getSocket() ));
                retVal = true;
                break;

            case SSL_ERROR_ZERO_RETURN:
                    AM_ERROR (("Client on [%s] closed connection on the socket %d\n",
                               getIp(getRemoteHostAddress()), getSocket()      ));
                    break;
                break;

            case SSL_ERROR_WANT_READ:
                AM_ERROR (("acceptSsl: Data needs to be read, problem due to renegotiation\n"));
                break;

            case SSL_ERROR_WANT_WRITE:
                AM_ERROR (("acceptSsl: Data needs to be written, problem due to renegotiation\n"));
                break;

            case SSL_ERROR_WANT_X509_LOOKUP:
                AM_ERROR (("acceptSsl: The operation did not complete because an \
application callback set by SSL_CTX_set_client_cert_cb() \
has asked to be called again. The TLS/SSL I/O function \
should be called again later\n"));
                break;

            case SSL_ERROR_SYSCALL:
                AM_ERROR (("SSL system call error in SSL_accept\n"));
                break;

            case SSL_ERROR_SSL:
                // General error
                // check if the peer's certificate is OK
                // print verification error
                if (verifyCert(SSL_get_verify_result(pSsl_)) == false)
                {
                    AM_ERROR (("Client [%s] certificate verification has failed.\nCause: %s\n", 
                        getIp        (getRemoteHostAddress())     ,
                        getCertError (SSL_get_verify_result(pSsl_)) ));
                        setError     (SSL_BAD_PEER_CERT           );
                }
                else
                    AM_ERROR (("General SSL error on SSL accept connection\n")); 
                break;

            default:
                AM_ERROR (("Unspecified SSL accept error %d\n", stat));
            }
        }
    }

  AM_RETURN (retVal);
}


///////////////////////////////////////////////////////////
// SSL read\write functions
///////////////////////////////////////////////////////////

int GenSocketSSL::readSsl (sbyte *pBuf, int BufSize)
{
  AM_FUNC  ("GenSocketSSL::readSsl", SOC_LAYER_ID);
  AM_ENTER ();

  int msgLen = SOC_STATUS_ERROR;
  int errCode;

    if (isInitSSL ())
    {
        msgLen = SSL_read (pSsl_, pBuf, BufSize);

        if (msgLen <= 0 && (errCode = SSL_get_error(pSsl_,msgLen)) != SSL_ERROR_NONE )
        {
            msgLen = SOC_STATUS_ERROR;

            switch (errCode)
            {
                case SSL_ERROR_NONE:
                    AM_DEBUG (("readSsl: message received - total length (%d)\n", msgLen));
                    msgLen = SOC_STATUS_OK;
                    break;

                case SSL_ERROR_ZERO_RETURN:
                    AM_TRACE (("readSsl: Client closed connection on socket (%d)\n",
                               getSocket()                                         ));
                    break;

                case SSL_ERROR_SYSCALL:
                    AM_ERROR (("readSsl: Problem in SSL_read,system call error\n"));
                    break;

                case SSL_ERROR_SSL:
                    AM_ERROR (("readSsl: general SSL library error\n"));
                    break;

                case SSL_ERROR_WANT_X509_LOOKUP:
                    AM_ERROR(("readSsl: The operation did not complete because an \
application callback set by SSL_CTX_set_client_cert_cb() \
has asked to be called again. The TLS/SSL I/O function \
should be called again later\n"));
                    break;

                case SSL_ERROR_WANT_READ:
                    AM_ERROR(("readSsl: Data needs to be read, problem due to renegotiation\n"));
                    break;

                default:
                    AM_ERROR (("readSsl: recv error %d on socket (%d)\n",
                               errCode, getSocket() ));
            }
        }
        else
        {
            AM_DEBUG   (("readSsl: received message [length=%d] on the socket %u\n", 
                         msgLen, getSocket() ));
            AM_HEXDUMP ((ubyte *)pBuf, msgLen);
        }
    }
    else
        AM_ERROR (("Attempt to read read from uninitialised SSL socket %u\n",
                   getSocket() ));

  AM_RETURN (msgLen);
}


int GenSocketSSL::writeSsl (sbyte *pBuf, int BufSize)
{
  AM_FUNC  ("GenSocketSSL::writeSsl", SOC_LAYER_ID);
  AM_ENTER ();

  int msgLen = SOCKET_STATUS_FATAL_SOC_ERR;
  int errCode;

    if (isInitSSL ())
    {
        msgLen = SSL_write (pSsl_, pBuf, BufSize);

        if (msgLen <= 0 && (errCode = SSL_get_error(pSsl_,msgLen)) != SSL_ERROR_NONE) 
        {
            msgLen = SOCKET_STATUS_FATAL_SOC_ERR;

            switch (errCode)
            {
            case SSL_ERROR_NONE:
                AM_DEBUG (("writeSsl: Sent %d bytes to socket %d\n", msgLen, getSocket() ));
                msgLen = SOCKET_STATUS_OK;
                break;

            case SSL_ERROR_ZERO_RETURN:
                AM_ERROR (("writeSsl: Peer closed connection on socket (%d)\n",
                           getSocket()));
                break;

            case SSL_ERROR_SYSCALL:
                AM_ERROR (("writeSsl: System call error\n"));
                ERR_print_errors_fp (stdout);
                break;

            case SSL_ERROR_SSL:
                AM_ERROR (("writeSsl: general SSL library error\n"));
                break;

            case SSL_ERROR_WANT_X509_LOOKUP:
                AM_ERROR (("writeSsl: The operation did not complete because an \
application callback set by SSL_CTX_set_client_cert_cb() \
has asked to be called again. The TLS/SSL I/O function \
should be called again later\n"));
                break;

            case SSL_ERROR_WANT_WRITE:
                AM_ERROR(("writeSsl: Data needs to be written, problem due to renegotiation\n"));
                break;

            default:
                AM_TRACE (("writeSsl: error %d on socket (%d)\n",
                           errCode, getSocket() ));
                break;
            }
        }
        else
        {
            AM_DEBUG (("writeSsl: Sent %d bytes to socket %u\n", msgLen, getSocket() ));
            msgLen = SOCKET_STATUS_OK;
        }
    }
    else
        AM_ERROR (("Attempt to read read from uninitialised SSL socket %u\n",
                   getSocket() ));

    if (msgLen == SOCKET_STATUS_FATAL_SOC_ERR)
        releaseSocket(true);

  AM_RETURN (msgLen);
}


// Define here certificate verification policy
bool GenSocketSSL::verifyCert (long code)
{
  bool retVal = false;

    switch (code)
    {
    // no error
    case X509_V_OK                                :
    // minor verification errors
    case X509_V_ERR_CERT_NOT_YET_VALID            :
    case X509_V_ERR_CERT_HAS_EXPIRED              :
    case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
    case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD :
    case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
    case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
        retVal = true;
        break;

#ifdef _OPEN_SSL_LIB_

    // minor verification errors
    case X509_V_ERR_INVALID_PURPOSE            :
    case X509_V_ERR_SUBJECT_ISSUER_MISMATCH    :
    case X509_V_ERR_AKID_SKID_MISMATCH         :
    case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
    case X509_V_ERR_KEYUSAGE_NO_CERTSIGN       :
        retVal = true;
        break;

    // The following errors considered as critical and will cause a link termination 
    case X509_V_ERR_PATH_LENGTH_EXCEEDED       :
    case X509_V_ERR_INVALID_CA                 :
    case X509_V_ERR_CERT_UNTRUSTED             :
        break;

#endif // _OPEN_SSL_LIB_

    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT       :
    case X509_V_ERR_CERT_REVOKED                      :
    case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE   :
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT         :
    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN         :
    case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE  :
    case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
    case X509_V_ERR_CERT_SIGNATURE_FAILURE            :
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY :
    default                                           :
        break;
    }

  return (retVal);
}

// Define here certificate verification policy
LPSTR GenSocketSSL::getCertError (long code)
{

  static LPSTR verificationErrorList [] = 
  {
    "Certificate is OK"                            , // no error
    "Minor certificate verification error"         , // OPENSSL specific errors
    "Unspecified verification error"               ,

    "Certificate is not valid yet"                 ,
    "Certificate has expired"                      ,
    "Certificate \"Not before\" field is not valid",
    "Certificate \"Not After\" field is not valid" ,
    "Certificate Last CRL update field is wrong"   ,
    "Certificate CRL next update field is wrong"   ,
    "Certificate the basic Constraints pathlength parameter has been exceeded",
    "Certificate purpose invalid"                  ,
    "The certificate is untrusted"                 ,
    "Invalid CA detected"                          ,
    "Certificate is self signed and cannot be found in the list of trusted certificates",
    "The certificate has been revoked"             ,
    "No signatures could be verified because the chain contains only one certificate\n\
and it is not self signed"                         ,
    "Certificate: Unable to get issuer certificate",
    "The certificate chain could be built up using the untrusted\n \
certificates but the root could not be found locally",
    "Unable to decrypt certificate\'s signature     ",
    "Unable to decode issuer public key             ",
    "Invalid signature found                        ",
    "Unable to get issuer certificate locally       ",
  };

  LPSTR retVal = NULL;

    switch (code)
    {
    case X509_V_OK:
        // no error
        retVal = verificationErrorList[CertNoError];
        break;

    case X509_V_ERR_CERT_NOT_YET_VALID:
        retVal = verificationErrorList[CertNotYetValid];
        break;

    case X509_V_ERR_CERT_HAS_EXPIRED:
        retVal = verificationErrorList[CertExpired];
        break;

    case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
        retVal = verificationErrorList[CertNotBeforeError];
        break;

    case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
        retVal = verificationErrorList[CertNotAfterError];
        break;

    case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
        retVal = verificationErrorList[CertCrlLastUpdateError];
        break;

    case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
        retVal = verificationErrorList[CertCrlNextUpdateError];
        break;

#ifdef _OPEN_SSL_LIB_

    case X509_V_ERR_PATH_LENGTH_EXCEEDED:
        retVal = verificationErrorList[CertPathLengthExceed];
        break;

    case X509_V_ERR_INVALID_PURPOSE:    
        retVal = verificationErrorList[CertInvalidPurpose];
        break;

    case X509_V_ERR_CERT_UNTRUSTED:    
        retVal = verificationErrorList[CertUntrusted];
        break;

    case X509_V_ERR_SUBJECT_ISSUER_MISMATCH    :
    case X509_V_ERR_AKID_SKID_MISMATCH         :
    case X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH:
    case X509_V_ERR_KEYUSAGE_NO_CERTSIGN       :
        // minor verification errors
        retVal = verificationErrorList[CertMiscError];
        break;

// The following errors considered as critical and will cause a link termination 

    case X509_V_ERR_INVALID_CA:
        retVal = verificationErrorList[CertInvalidCA];
        break;

#endif // _OPEN_SSL_LIB_

    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        retVal = verificationErrorList[CertZeroSelfSigned];
        break;

    case X509_V_ERR_CERT_REVOKED:
        retVal = verificationErrorList[CertRevoked];
        break;

    case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
        retVal = verificationErrorList[CertOneNotSelfSigned];
        break;

    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
        retVal = verificationErrorList[CertNoIssuer];
        break;

    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
        retVal = verificationErrorList[CertUntrustedNoRoot];
        break;

    case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
        retVal = verificationErrorList[CertUnableDecryptSignature];
        break;

    case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
        retVal = verificationErrorList[CertUnableDecodeIssuerKey];
        break;

    case X509_V_ERR_CERT_SIGNATURE_FAILURE:    
        retVal = verificationErrorList[CertInvalidSignature];
        break;

    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
        retVal = verificationErrorList[CertNoIssuerCert];
        break;

    default:
        retVal = verificationErrorList[CertUnspecifiedError];
        break;
    }

  return (retVal);
}


void GenSocketSSL::printSession ()
{
    SSL_SESSION *pSession =  SSL_get_session(pSsl_);

    if (pSession)
        SSL_SESSION_print_fp (stderr, pSession);
}

