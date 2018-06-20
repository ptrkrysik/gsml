#ifndef __CDCMODULE_H__
#define __CDRMODULE_H__
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
// File        : CdrModule.h
// Author(s)   : Igal Gutkin
// Create Date : 10/02/00
// Description : CDR module class interface
//               Call Detail Recording (CDR) Client
//               Call Detail Recording is split into two parts. Much
//               of the work is done through the api functions providing
//               access to the record classes. However, a separate task 
//               is used to perform network IO that is required to forward
//               the records to CDR Server which can cause some delay. 
//               Thus, potentially lengthy operations can be done in the 
//               background at a lower priority than call processing.
// *******************************************************************

#include "Os\JCModule.h"
#include "Os\JCMsgQueue.h"
#include "Os\JCTask.h"

#include <usrLib.h>

#include "AlarmCode.h"

#include "CdrShellIntf.h"

#include "CdrInterface.h"
#include "cdrdefs.h"

#include "CdrRefSem.h"
#include "CdrRemClientSocket.h"


// system definitions

#define CDR_MSG_Q_MAX           128                   // max msgs that can be queued
#define CDR_MSG_SIZE_MAX        sizeof (CdrMessage_t) // max bytes in a msg
#define CDR_Q_OPTIONS           JC_MSG_Q_FIFO         // system options


class CdrModule
{
// methods
  public:

// CDR API functions. Used by the CDR record classes
friend void   cdrDataPrint   ();
friend void   cdrStartLog    ();
friend void   cdrStopLog     ();
friend bool   cdrGetStatus   ();
friend bool   cdrSendLog     (CdrMessage_t* msg);
friend ubyte4 cdrAllocateRef ();
friend ubyte4 cdrGetLocalIp  ();
friend int    SysCommand_CDR (T_SYS_CMD     cmd);

    // Main loop entry point
    static bool        mainTaskHandle();

    // Main access interface
    static CdrModule  *getInstance   () {return (pCdr_)     ;}

    // Encrypts & write CDR access password to the file
    static bool        savePwd       (LPCSTR lpPasswd);

           // Alarm handling
    static void        raiseSslAlarm    ();
    static void        cleanSslAlarm    ();

    static void        raiseModuleAlarm (AlarmCode_t code);
    static void        cleanModuleAlarm ();

           // Print operational information
           void        printInfo     ();

           // Test whether the CDR module is properly initialized
           bool        isSet         () {return (pMainQueue_ != NULL && pSoc_ != NULL);}

           // Test whether there is a connection to CDR server
           bool        isConnected   () {return (srvOnline_);}

           ubyte4      getServerIp   () {return (srvrIp_   );}
           ubyte2      getServerPort () {return (srvrPort_ );}

           // use for debug purpose only
           bool        switchServer  (ubyte4 newSrvrIp);

           bool        enableExtraInfo  () { bExtraInfo_ = true ; }
           bool        disableExtraInfo () { bExtraInfo_ = false; }

  private:

    // Constructors & Destructor
     CdrModule ();
    ~CdrModule ();
    

    // Create the module instance
    static void        create        ();

    // System Root control interface
    static int         sysCmdHandler (T_SYS_CMD cmd);

    static bool        readPwd       ();

    static LPSTR       getPswdFile   ();

           // Setup the module task(s) and queue(s)
           bool        start         ();

           // Load init data
           bool        loadData      ();

           // Send log to the CDR module
           bool        submitRec     (CdrMessage_t* msg);

           // receive Msg from the incoming Queue
           bool        getMsg        ();

           // Access the module input queue
           JCMsgQueue *getCdrMainQ   () {return (pMainQueue_);}

           // Allocate new CDR reference number
           ubyte4      getNewRef     () {return (localRef_.allocateRef());}

           // setup socket connection to the CDR server
           bool        socketInit    ();

           // re-establish socket connection to the CDR server
           bool        socketReset   ();

           // Logging control interface
           void        enable        () {logStatus_ = true  ;}
           void        disable       () {logStatus_ = false ;}
           bool        getStatus     () {return (logStatus_);}


// data members
  private:

    // provides information about using SSL
    static bool                bUseSSL_   ,
                               bUseMibSSL_;

    static bool                bExtraInfo_;


    static char                sPasswd_[SSL_MAX_PASSWORD_LENGTH+2];

    // pointer to the module instance
    static CdrModule          *pCdr_      ;

           // module system elements
    static JCMsgQueue         *pMainQueue_;
           JCTask              mainTask_  ;
           JCMsgQueue         *pReplayQ_  ;

           // Alarm code
    static AlarmCode_t         majorAlarmCode_,
                               infoAlarmCode_ ;

           // Client socket to the server
           CdrRemClientSocket *pSoc_     ;

           // CDR server parameters
           ubyte4              srvrIp_   ;
           ubyte2              srvrPort_ ;

           // Local reference allocator
           CdrRefSem           localRef_ ;

           // CDR Logging status
           bool                logStatus_;

           // Cdr connection indicator
           bool                srvOnline_;

           unsigned            sentRecs_ ; 
           unsigned            lostRecs_ ;

           // message handling
           unsigned            inMsgSize_;
           unsigned            inMsgType_;
           CdrMessage_t        inMsgBuf_ ;
           MNET_MODULE_ID      fromModId_;

};


#endif //__CDRMODULE_H__