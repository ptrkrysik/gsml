// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.0
// Status      : Under development
// File        : CdrRemClientSocket.cpp
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : Implementation of the CdrRemClient class.
//
// *******************************************************************

#include "StdAfx.h"
#include "CdrCommon.h"

#include "CdrRemClientSocket.h"


//////////////////////////////////////////////////////////////////////
// Primary behaviour methods
//////////////////////////////////////////////////////////////////////

int CdrRemClientSocket::sendData (LPSTR szData)
{
  AM_FUNC  ("CdrRemClientSocket::sendData", LAYER_ID);
  AM_ENTER ();

  int    retVal      = CDR_STATUS_FATAL_SOC_ERR;
  size_t inDataLen   = strlen (szData) + 1; // correction for terminating '0'
  int    sentDataLen , socError;

    if (inDataLen <= CDR_MAX_DATA_LENGTH)
    {
        // Prepare the message structure
        outMsg_.hdr.msgType = htonl  (CDR_GENERIC_MSG_TYPE);
        outMsg_.hdr.dataLen = htonl  (inDataLen);
        memcpy (outMsg_.msg.logData, szData, inDataLen);

        inDataLen +=clientMsgHdrLen;

        retVal = tcpSend ((sbyte *)&outMsg_, inDataLen);
    }
    else
    {
        retVal = CDR_STATUS_MSG_TOO_LONG;
        
        AM_TRACE (("CDR data length %u exciding maximum. Discard the record.\n",
                   inDataLen));
    }

  AM_RETURN (retVal);
}


//////////////////////////////////////////////////////////////////////
// Service methods
//////////////////////////////////////////////////////////////////////


void CdrRemClientSocket::zapLog()
{
    memset ((char *)&outMsg_, 0, sizeof outMsg_);
}


