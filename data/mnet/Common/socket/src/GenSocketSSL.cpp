// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// File        : GenSocketSSL.cpp
// Author(s)   : Igal Gutkin
// Create Date : 05/17/01
// Description : platform independent implementation of the GenSocketSSL class.
//
// *******************************************************************

#include "socket\GenSocketSSL.h"


//////////////////////////////////////////////////////////////////////
// Static data members
//////////////////////////////////////////////////////////////////////

bool       GenSocketSSL::bInitLib_ = false       ;
SslError_t GenSocketSSL::errorCode_= SSL_NO_ERROR;
int        GenSocketSSL::count_    = 0           ;
char       GenSocketSSL::szPswd_   [SSL_MAX_PASSWORD_LENGTH+2];

LPCSTR     GenSocketSSL::cipherSuite_ [] =
{
    "eNULL:DES-CBC3-SHA:RC4-SHA",
    "DES-CBC3-SHA"              ,
    "eNULL"                     
};

// Full pathname storege of the SSL files
char GenSocketSSL::szKeyFile_ [MAX_PATH]; // private key 
char GenSocketSSL::szCertFile_[MAX_PATH]; // certificate
char GenSocketSSL::szCaFile_  [MAX_PATH]; // trusted CA List

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Primary methods
//////////////////////////////////////////////////////////////////////

int GenSocketSSL::tcpReceive (sbyte *pBuf, int BufSize)
{
    return ((bEnableSsl_ ? readSsl                (pBuf, BufSize)
                         :  GenSocket::tcpReceive (pBuf, BufSize))); 
}


int GenSocketSSL::tcpSend (sbyte *pBuf, int BufSize)
{
    return ((bEnableSsl_ ? writeSsl           (pBuf, BufSize)
                         : GenSocket::tcpSend (pBuf, BufSize))); 
}


void GenSocketSSL::releaseSocket (bool grace)
{
  AM_FUNC  ("GenSocketSSL::releaseSocket", SOC_LAYER_ID);
  AM_ENTER ();

    cleanSsl ();
    // Context is not deleted upon release of the single socket
    // High level application should take case

    // for SSL socket: forcibly false so that shutdown is not called
    // GenSocket::releaseSocket ((bEnableSsl_? false : grace));
    GenSocket::releaseSocket (grace);

  AM_LEAVE ();
}


void GenSocketSSL::printInfo ()
{
    GenSocket::printInfo ();

    if (isSSL())
    {
        if (bEnableSsl_)
        {
            printf ("SSL is enabled on the socket, encryption is %s.\n", 
                    (bEnableEncryption_ ? "enabled" : "disabled") );

            if (bMoreInfo_)
            {
                printf ("SSL info: Context %p, BIO %p, connection %p\n",
                        getContext(), pSbio_, pSsl_);

                if (pSsl_)
                    printSession ();
                else
                    printf ("SSL Connection is not initialized.\n");
            }
        }
        else
            printf ("SSL is disabled on the socket.\n");
    }
}


void GenSocketSSL::classPrintInfo ()
{
    printf ("SSL Socket class info:\n");
    printf ("  SSL is%s supported.\n", (isSSL()) ? "" : " not");

    if (isSSL())
    {
        printf ("  Total number of active SSL-enabled sockets = %u\n", count_);

        if (errorCode_ != SSL_NO_ERROR)
            printf ("Last SSL error recorded %d\n", errorCode_);

#ifdef _AM_DEBUG_
        if (bMoreInfo_)
            printf ("  Password \"%s\"\n", 
                    (szPswd_ && strlen(szPswd_) <= SSL_MAX_PASSWORD_LENGTH)
                    ? szPswd_ : "INVALID");
#endif // _AM_DEBUG_
    }
}


// Set password only when it's NOT set and a new password is of valid length
bool GenSocketSSL::setPassword (LPCSTR lpPswd)
{
  int  length = (lpPswd) ? strlen (lpPswd) : 0;
  bool retVal = false;

    if (!isPswdSet() && length > 0 && length <= SSL_MAX_PASSWORD_LENGTH)
    {
        strcpy (szPswd_, lpPswd);
        retVal = true;
    }

  return (retVal);
}


void GenSocketSSL::resetPassword ()
{
    memset (szPswd_, 0, sizeof(szPswd_)) ; 
}


//////////////////////////////////////////////////////////////////////
// Service methods
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Shell Interface
//////////////////////////////////////////////////////////////////////

void SocDataPrint ()
{
    GenSocketSSL::classPrintInfo();
}

