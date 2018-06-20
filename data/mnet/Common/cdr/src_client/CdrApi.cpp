// *******************************************************************
//
// (c) Copyright CISCO Systems, Inc 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.0
// File        : CdrApi.cpp
// Author(s)   : Igal Gutkin
// Create Date : 11/08/00
// Description : Application interface for the CDR subsystem
//
// *******************************************************************

#include "stdafx.h"

#include "CdrModule.h"

/*******************************************************************************
 *
 *    cdrStartLog, cdrStopLog, cdrGetStatus
 *
 *        These functions are used to turn CDR logging On/Off and check current
 *        logging status
 *
 ********************************************************************************/

void cdrStartLog ()
{
    if (CdrModule::getInstance())
        CdrModule::getInstance()->enable();
}


void cdrStopLog ()
{
    if (CdrModule::getInstance())
        CdrModule::getInstance()->disable();
}


bool cdrGetStatus ()
{
    if (CdrModule::getInstance())
        return (CdrModule::getInstance()->getStatus());
}


/*******************************************************************************
 *
 *    cdrAllocateRef    Allocate a Call Detail Record
 *
 *       This function is used to allocate a CDR record. Since CDR records are
 *       stored in the application context only new CDR Reference number will
 *       be issued.
 *
 *       Call: newRefNum = cdrAllocateRef()
 *
 *        newRefNum - This is a unique 16 bit number that can be used
 *              to identify the record by.
 *              Note: After reaching max value (65,536), the number
 *              will wrap and we start re-using the reference numbers.
 *
 ******************************************************************************/
ubyte4 cdrAllocateRef ()
{
  ubyte4 newRef = 0;

  AM_FUNC ("cdrAllocateRef", LAYER_ID);
  AM_ENTER();

    if (CdrModule::getInstance())
        newRef = CdrModule::getInstance()->getNewRef();

  AM_LEAVE ();
  return (newRef);
}


/*******************************************************************************
 *
 *    cdrGetLocalIp    Provides the local host IP address to the CDR classes
 *
 *       This function is using cached value initialized during the init.
 *
 *       Call        : uIpAddrr = cdrGetLocalIp()
 *
 *       Return value: NULL on error;
 *
 ******************************************************************************/
ubyte4 cdrGetLocalIp ()
{
    return (htonl(GenSocket::getLocalHostAddress()));
}


/*******************************************************************************
 *
 *    cdrSendLog    API for sending prepared CDR log to the CDR subsystem
 *
 *       This function is used to send CDR log from the application to CDR subsystem.
 *       Should be invoked only by the CDR base class
 *       being allocated.
 *
 *       Call: result =  cdrSendLog (&recordPool)
 *
 *        Return Value: true if success, otherwise - false
 *
 ******************************************************************************/
bool cdrSendLog (CdrMessage_t* msg)
{
    if (CdrModule::getInstance())
        return (CdrModule::getInstance()->submitRec (msg));
    else
        return (false);
}


/*******************************************************************************
 *
 *    _cdrSwitchServer    API for changing CDR Server IP address 
 *
 *       This function should be used for changing CDR Server IP address 
 *       for DEBUG purposes ONLY!!!
 *       Should be invoked only by the CDR base class being allocated.
 *
 *       Call: result =  _cdrSwitchServer (char* ipAddress)
 *
 *        Input Parameter: string containing an (Ipv4) Internet Protocol dotted address
 *        Return Value   : true if success, otherwise - false
 *
 ******************************************************************************/
bool _cdrSwitchServer (LPSTR ipAddr)
{
  bool     retVal = false         ;
  unsigned len    = strlen(ipAddr);
  ubyte4   srvrIp ;

    if (len < 7 || len > 15)
    {
        printf ("Invalid IP address length.\n");
    }
    else if ((srvrIp=inet_addr(ipAddr)) == INADDR_NONE)
    {
        printf ("Invalid IP address (%s) is provided\n", ipAddr);
    }
    else
    {
        if (CdrModule::getInstance())
            retVal = CdrModule::getInstance()->switchServer(srvrIp);
        else
            printf ("Cdr Client module is not allocated properly.\n");
    }

  return (retVal);
}