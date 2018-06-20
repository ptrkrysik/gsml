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
// Description : stub implementation of the GenSocketSSL class methods
//
// *******************************************************************

#include "socket\GenSocketSSL.h"

//////////////////////////////////////////////////////////////////////
// function stubs
//////////////////////////////////////////////////////////////////////


bool GenSocketSSL::initSslContext (SSL_CTX *& pCtx, bool bServer, LPSTR lpPswd)
{
    return  (false);
}


void GenSocketSSL::cleanSslContext  (SSL_CTX *& pCtx)
{
    return ;
}


bool GenSocketSSL::initSsl ()
{
    return  (false);
}

void GenSocketSSL::cleanSsl ()
{
    pSsl_  = NULL;
    pSbio_ = NULL;
}


bool GenSocketSSL::acceptSsl  ()
{
    return  (false);
}


bool GenSocketSSL::connectSsl ()
{
    return  (false);
}


void GenSocketSSL::initSslLib ()
{
    return ;
}


int GenSocketSSL::readSsl  (sbyte *pBuf, int BufSize)
{
    return (SOC_STATUS_ERROR);
}


int GenSocketSSL::writeSsl (sbyte *pBuf, int BufSize)
{
    return (SOC_STATUS_ERROR);
}


void GenSocketSSL::printSession ()
{
    ;
}


bool encryptPassword (const char *pwd, const char *filename)
{
    return (false);
}


bool decryptPassword (char *pwdout, const char *filename)
{
    return (false);
}

