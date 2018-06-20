// *******************************************************************
//
// (c) Copyright CISCO Systems, 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CdrInit.cpp
// Author(s)   : Igal Gutkin
// Create Date : 10/02/00
// Description : CDR client module class target-dependant methods
//               GP10 application.
//
// *******************************************************************

#include "StdAfx.h"

#include "CdrModule.h"

#include "mibtags.h"
#include "oam_api.h"


// CDR server is co-located with GMC.
// Retrive GMC IP address from the MIB on GP10.
// Always use GMC IP as an address for the primary CDR server.
bool CdrModule::loadData ()
{
  bool   retVal     = true;
  ubyte4 tempIpAddr = 0    ,
         tempStatus = 0    ;

  AM_FUNC ("CdrModule::loadData", CDR_LAYER);
  AM_ENTER();

    // load parameters
    // load CDR server info (IP, username, password) from the MIB

    if (oam_getMibIntVar (MIB_h323_GKIPAddress, (sbyte4 *)&tempIpAddr) == STATUS_OK)
    {
        // For debug purpose ONLY, redirect CDR Server address here
        // tempIpAddr = inet_addr ("...");

        if (retVal = (tempIpAddr != 0))
        {
            srvrIp_ = tempIpAddr;

            AM_TRACE (("CDR Server IP address is set to %s (%#08lx)\n",
                       GenSocket::getIp(srvrIp_), tempIpAddr          ));
        }
    }
    else
    {
        retVal = false;
        AM_ERROR (("CDR Client: Unable to retrieve serving CDR Server (GMC) IP address from the MIB.\n"));
    }

    if (oam_getMibIntVar (MIB_cdrClientSSLEnable, (sbyte4 *)&tempStatus) == STATUS_OK)
    {
        bUseMibSSL_ = (tempStatus != 0) ? true : false;
    }
    else
    {
        retVal = false;
        AM_ERROR (("CDR Client: Unable to retrieve SSL status from the MIB.\n"));
    }

    // Read password from the password file
    readPwd ();

    // retrive CDR MIB error code
    if (oam_getMibIntVar (MIB_errorCode_CDR, (sbyte4 *)&tempStatus) == STATUS_OK)
        majorAlarmCode_ = (AlarmCode_t)tempStatus;

  AM_RETURN (retVal);
}

