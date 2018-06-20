#ifndef CDR_INTERFACE_H
#define CDR_INTERFACE_H

// *******************************************************************
//
// (c) Copyright Cisco Systems Inc, 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CdrInterface.h
// Author(s)   : Igal Gutkin
// Create Date : 07-25-00
// Description : CDR module interface to the CDR application classes
//
// *******************************************************************

#ifndef __CDRCOMMON_H__
#include "CdrCommon.h" 
#endif //__CDRCOMMON_H__


// CDR message storage
typedef struct
{
    ubyte4       msgType;
    CdrRecPool_t record ;
} CdrMessage_t;


#ifdef __cplusplus
 extern "C" 
 {

 // CDR control functions
 /*-------------------------------------------------------------*/

 // Stop CDR logging
 void   cdrStopLog       ();

 // Start/resume CDR logging
 void   cdrStartLog      ();

 // CDR info functions
 bool   cdrGetStatus     ();

 bool   _cdrSwitchServer (LPSTR ipAddr);

 // CDR user functions
 /*-------------------------------------------------------------*/

 // Return unique CDR reference number
 ubyte4 cdrAllocateRef   ();

 // Provide the local host IP address (network byte order!!!)
 ubyte4 cdrGetLocalIp    ();

 // Send log to CDR subsystem
 bool   cdrSendLog       (CdrMessage_t *);

 // Save CDR password to the file
 bool   cdrSavePwd       (LPCSTR lpPasswd);
 }

#else

 // CDR control functions
 void   cdrStopLog       (void);
 void   cdrStartLog      (int );
 bool   cdrGetStatus     (void);
 bool   _cdrSwitchServer (LPSTR ipAddr);

 // CDR user functions
 ubyte4 cdrAllocateRef   (void);
 ubyte4 cdrGetLocalIp    (void);
 bool   cdrSendLog       (CdrMessage_t *);

 bool   cdrSavePwd       (LPCSTR lpPasswd);

#endif // __cplusplus


#endif // CDR_INTERFACE_H