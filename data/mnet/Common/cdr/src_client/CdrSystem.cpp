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
// File        : CdrSystem.cpp
// Author(s)   : Igal Gutkin
// Create Date : 10/02/00
// Description : System interface for CDR module
//
// *******************************************************************


#include "stdafx.h"

#include "CdrModule.h"


int SysCommand_CDR (T_SYS_CMD cmd)
{
    return (CdrModule::sysCmdHandler (cmd));
}


void CdrDataPrint ()
{
  CdrModule * pInst = CdrModule::getInstance();

    if (pInst)
        pInst->printInfo();
    else
        printf ("CDR module is not started.\n");
}


bool cdrSavePwd (LPCSTR lpPasswd)
{
  bool        retVal = false;
  CdrModule * pInst  = CdrModule::getInstance();

    if (pInst)
        retVal = pInst->savePwd (lpPasswd);
    else
        printf ("CDR module is not started.\n");

  return (retVal);
}


void CdrEnableExtraInfo ()
{
    CdrModule::getInstance()->enableExtraInfo();
}


void CdrDisableExtraInfo ()
{
    CdrModule::getInstance()->disableExtraInfo();
}


// CDR debug input
void _CdrSendDebug ()
{
  static CdrMessage_t msg                  ;
         sbyte        szHostName[MAX_PATH] ;
         ubyte4       uHostIp = 0;
         int          ch      ;

  struct tm          *currTime;
         time_t       lTime   ;

    // get a local host name
    if (gethostname (szHostName, sizeof(szHostName)) != SOC_STATUS_ERROR)
    {
        uHostIp = htonl (cdrGetLocalIp());
    }
    else
        *szHostName = '\0';

    msg.msgType = CDR_GENERIC_MSG_TYPE;

    /* Obtain coordinated universal time: */
    time (&lTime );
    currTime = gmtime (&lTime);
    sprintf ((char *)msg.record, "Test CDR log number %d from virtual client on host %s (%s), %s",
             cdrAllocateRef(), szHostName, GenSocket::getIp(uHostIp), asctime(currTime));

    printf ("Sending CDR log: %s\n", msg.record);

    cdrSendLog (&msg);
}
