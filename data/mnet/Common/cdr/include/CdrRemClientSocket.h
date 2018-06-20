#ifndef __CDRREMCLIENTSOCKET_H__
#define __CDRREMCLIENTSOCKET_H__

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
// File        : CdrRemClientSocket.h
// Author(s)   : Igal Gutkin
// Create Date : 06/20/00
// Description : interface for the CdrRemClientSocket class.
//
// *******************************************************************

#include "socket\GenClientSocket.h"

#include "msgProc.h"


#define CDR_CLIENT_SOC_RCV_BUFF_LEN    (0x500)
#define CDR_CLIENT_SOC_SND_BUFF_LEN    (0x1024)

class CdrRemClientSocket : public GenClientSocket
{
public:

    CdrRemClientSocket  ()    
      : GenClientSocket (CDR_CLIENT_SOC_SND_BUFF_LEN, CDR_CLIENT_SOC_RCV_BUFF_LEN)
    {;}

    CdrRemClientSocket (bool bEnableSSL, bool bEnableEncryption = true)
      : GenClientSocket (CDR_CLIENT_SOC_SND_BUFF_LEN, 
                         CDR_CLIENT_SOC_RCV_BUFF_LEN,
                         bEnableSSL                 ,
                         bEnableEncryption          ) { ; }

    virtual 
   ~CdrRemClientSocket () { ; }

    int  sendData       (LPSTR  szData);

protected:

private:
    void zapLog ();

// data members
private:

    ClientMsg_t  outMsg_;

};

#endif //__CDRREMCLIENTSOCKET_H__