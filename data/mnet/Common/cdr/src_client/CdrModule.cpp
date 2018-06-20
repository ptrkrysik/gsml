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
// File        : CdrModule.cpp
// Author(s)   : Igal Gutkin
// Create Date : 10/02/00
// Description : CDR client module class implementation
//
// *******************************************************************

#include "StdAfx.h"

#include "CdrModule.h"


//////////////////////////////////////////////////////////////////////
// Static Data members declarations
//////////////////////////////////////////////////////////////////////

CdrModule  *CdrModule::pCdr_      = NULL ;
JCMsgQueue *CdrModule::pMainQueue_= NULL ;

// SSL related variables
bool        CdrModule::bUseSSL_   = false;
bool        CdrModule::bUseMibSSL_= false;
char        CdrModule::sPasswd_[SSL_MAX_PASSWORD_LENGTH+2] = "";

AlarmCode_t CdrModule::majorAlarmCode_ = EC_NOERROR;
AlarmCode_t CdrModule::infoAlarmCode_  = EC_NOERROR;


//////////////////////////////////////////////////////////////////////
// Constructors/Destructors
//////////////////////////////////////////////////////////////////////

CdrModule::CdrModule ()
    : mainTask_      ("tCdrClient"       ),
      fromModId_     (MNET_MAX_MODULE_IDS),
      srvrPort_      (AM_MAIN_SERVER_PORT),
      pReplayQ_      (NULL ),
      pSoc_          (NULL ),
      logStatus_     (true ),
      srvOnline_     (false),
      srvrIp_        (0),
      sentRecs_      (0),
      lostRecs_      (0),
      inMsgType_     (0),
      inMsgSize_     (0),
      localRef_      ( )
{
    pCdr_ = this;
}


CdrModule::~CdrModule () 
{
  int tId = mainTask_.GetTaskId();

    // Suspend CDR task
    if (tId)
        taskSuspend (tId);

    mainTask_.JCTaskNormExit();

    // Close opened files, sockets and etc. before the shutdown

    if (pMainQueue_)
        delete pMainQueue_;

    if (pSoc_)
        delete pSoc_;

    pMainQueue_ = NULL;
    pCdr_       = NULL;
    pSoc_       = NULL;

    bUseSSL_    = false;
    *sPasswd_   = '\0';
}


//////////////////////////////////////////////////////////////////////
// Static methods
//////////////////////////////////////////////////////////////////////

void CdrModule::create ()
{
    if (!pCdr_)
    { // Create a single CDR client module instance
        new CdrModule ();
    }
}


int CdrModule::sysCmdHandler (T_SYS_CMD sysCmd)
{
    int retVal = true;

  AM_FUNC ("CdrModule::sysCmdHandler", LAYER_ID);
  AM_ENTER();

    switch (sysCmd)
    {
        case SYS_START:
            printf ("[CDR client] Received start notification.\n");

            create ();

            if (pCdr_)
            {
                if ((retVal = pCdr_->start()) == false)
                    delete pCdr_;
            }
            break;

        case SYS_SHUTDOWN:
            printf ("[CDR client] Received system shutdown notification.\n");

            if (pCdr_)
                delete pCdr_;

            break;

        case SYS_REBOOT:

            printf ("[CDR client] Reboot ready.\n");

            if (pCdr_)
                delete pCdr_;

            break;

        default:
            printf ("[CDR client] Ignore unknown system command %d.\n", sysCmd);
            break;
    }

  AM_RETURN (retVal);
}



//////////////////////////////////////////////////////////////////////
// Public methods
//////////////////////////////////////////////////////////////////////
void CdrModule::printInfo ()
{
  SslError_t tmpError;

    printf ("=====================================\n");
    printf ("CDR Client logging is %s\n", (getStatus() ? "On" : "Off"));

    printf ("CDR Client Security is%s supported, %s \n", 
            (pSoc_->isSupportSSL() ? ""        : " not"    ),
            (pSoc_->isEnableSSL () ? "enabled" : "disabled"));

    if (bExtraInfo_ && isSSL())
    {
        if ((tmpError = pSoc_->getError()) != SSL_NO_ERROR)
            printf ("Last SSL error detected = %u\n", tmpError);
    }

    if (isSet())
    {
        if (bExtraInfo_)
        {
            printf ("CDR Client is initialized properly.\nCDR Server socket Info:\n");
            pSoc_->printInfo();
        }
        else
        {
            printf ("CDR Client is initialized properly.\nCDR Server is set to %s\n",
                    GenSocket::getIp(getServerIp())                                  );
        }

        printf ("CDR stats: records sent %lu, records lost %lu\n",
                sentRecs_, lostRecs_                             );
    }
    else
    {
        printf ("CDR Socket is not initialized properly.\n");

        printf ("CDR server address is set to (%s)\n",
                GenSocket::getIp(getServerIp())      );
    }

    printf ("=====================================\n");
}



//////////////////////////////////////////////////////////////////////
// Private methods
//////////////////////////////////////////////////////////////////////

bool CdrModule::start ()
{
  bool retVal = false;

  AM_FUNC ("CdrModule::start", LAYER_ID);
  AM_ENTER();

    pMainQueue_ = new JCMsgQueue (CDR_MSG_Q_MAX   ,  // max msgs that can be queued
                                  CDR_MSG_SIZE_MAX,  // max bytes in a msg
                                  CDR_Q_OPTIONS   ); // message queue options

    // spawn CDR task(s)
    if (pMainQueue_)
    {
        retVal = (mainTask_.JCTaskSpawn (CDR_TASK_PRIORITY      ,
                                         CDR_TASK_OPTION        ,
                                         CDR_TASK_STACK_SIZE    ,
                                         (FUNCPTR)mainTaskHandle,
                                         0, 0, 0, 0, 0, 0, 0 , 0, 0, 0,
                                         CDR_MODULE_ID          , 
                                         JC_CRITICAL_TASK       ) != 0);
    }

  AM_RETURN (retVal);
}


bool CdrModule::submitRec (CdrMessage_t* msg)
{
  bool   retVal = false;
  size_t recLen = strlen (msg->record);

  AM_FUNC  ("CdrModule::submitRec", LAYER_ID);
  AM_ENTER ();

    if (recLen >= CDR_HEADER_SIZE     && 
        recLen <  CDR_MAX_DATA_LENGTH   ) 
    {
        msg->msgType  = CDR_GENERIC_MSG_TYPE    ;
        recLen       += sizeof(msg->msgType) + 1;

        retVal = (pMainQueue_->JCMsgQSend (pMainQueue_      , 
                                           msg->msgType     , 
                                           CDR_MODULE_ID    ,
                                           (char *)msg      ,
                                           recLen           ,
                                           JC_WAIT_FOREVER  ,
                                           JC_MSG_PRI_NORMAL) == OK);
    }
    else
    {
        AM_TRACE (("Invalid record length %d. Discard message:\n %100s\n",
                   recLen, msg->record));
    }

  AM_RETURN (retVal);
}


bool CdrModule::getMsg ()
{
    // receive messages from the queue
    return (pMainQueue_->JCMsgQReceive (&pReplayQ_             ,
                                        (unsigned *)&inMsgType_,
                                        &fromModId_            ,
                                        (char * )&inMsgBuf_    ,
                                        &inMsgSize_            , 
                                        WAIT_FOREVER           ) != ERROR);
}


bool CdrModule::socketInit ()
{
  bool retVal = false;
  AM_FUNC ("CdrModule::socketInit", LAYER_ID);
  AM_ENTER();

    if (!pSoc_) // Create a socket
    {
        // Use SSL function only when both SSL MIB variable is true 
        // and socket library supports SSL
        // bUseMibSSL_ is preset with the MIB value
        bUseSSL_ = bUseMibSSL_ && GenSocketSSL::isSupportSSL();

        AM_DEBUG (("Cdr Client%s using SSL\n", (bUseSSL_ ? "" : " is not") ));

        // set password and initalize SSL Client context
        if (bUseSSL_)
            GenClientSocket::initSslContext (strlen(sPasswd_) ? sPasswd_ : NULL);

        pSoc_ = new CdrRemClientSocket (bUseSSL_);
    }

    if (pSoc_)
    {
        retVal = pSoc_->connectServer(srvrIp_, srvrPort_);
        srvOnline_ = retVal;

        if (!srvOnline_)
            raiseSslAlarm();

#ifdef _AM_DEBUG_
        pSoc_->printInfo ();
#endif // _AM_DEBUG
    }
    else
    {
        AM_ERROR (("CDR Client: unable to initiate link to CDR Server on %s:%u\n",
                   GenSocket::getIp(srvrIp_), srvrPort_ ));
    }

  AM_RETURN (retVal);
}


bool CdrModule::socketReset ()
{
  bool retVal = false;

  AM_FUNC ("CdrModule::socketReset", LAYER_ID);
  AM_ENTER();

    if (!pSoc_) // Create it upon need
        retVal = socketInit ();
    else
    { // reset the server conection
        retVal = pSoc_->reconnect();
    }

    srvOnline_ = retVal;

    AM_TRACE (("CDR server reconnect result %d\n", retVal));

  AM_RETURN (retVal);
}


// Raise alarm for the recognized SSL problems (only first time)
void CdrModule::raiseSslAlarm ()
{
  AM_FUNC ("CdrModule::raiseSslAlarm", LAYER_ID);
  AM_ENTER();

    if (infoAlarmCode_ == EC_NOERROR)
    {
        switch (GenSocketSSL::getError())
        {
        case SSL_BAD_PASSWORD:
            infoAlarmCode_ = EC_CDR_CLIENT_BAD_SSL_PASSWORD_FILE;
            break;
        case SSL_BAD_KEY:
            infoAlarmCode_ = EC_CDR_CLIENT_BAD_SSL_KEY_FILE     ;
            break;
        case SSL_BAD_CA_ERROR:
        case SSL_BAD_USER_CERT:
            infoAlarmCode_ = EC_CDR_CLIENT_BAD_LOCAL_CERTIFICATE;
            break;
        case SSL_BAD_PEER_CERT:
            infoAlarmCode_ = EC_CDR_CLIENT_BAD_PEER_CERTIFICATE ;
            break;
        default:
            // either non-SSL or unrecognized error
            // will open general communication problem alarm only
            break;
        }

        if (infoAlarmCode_ != EC_NOERROR)
        {
            AM_DEBUG (("Raise CDR SSL alarm %u\n", infoAlarmCode_));
            alarm_raise (CDR_MODULE_ID, CDR_CLIENT_ALARM_ERRCODE, infoAlarmCode_);
        }
    }

  AM_LEAVE();
}


// Clean SSL alarm if any
void CdrModule::cleanSslAlarm ()
{
    if (infoAlarmCode_ != EC_NOERROR)
    {
        alarm_clear (CDR_MODULE_ID, CDR_CLIENT_ALARM_ERRCODE, infoAlarmCode_);
        infoAlarmCode_ = EC_NOERROR;
    }
}


void CdrModule::raiseModuleAlarm (AlarmCode_t code)
{
    // raise new CDR alarm
    if (majorAlarmCode_ == EC_NOERROR)
    {
        majorAlarmCode_ = code;
        alarm_raise (CDR_MODULE_ID, CDR_CLIENT_ALARM_ERRCODE, majorAlarmCode_);
    }
}


void CdrModule::cleanModuleAlarm ()
{
    if (majorAlarmCode_ != EC_NOERROR)
    {
        alarm_clear (CDR_MODULE_ID, CDR_CLIENT_ALARM_ERRCODE, majorAlarmCode_);
        majorAlarmCode_ = EC_NOERROR;
    }
}



// This function should be used for debug purpose only
bool CdrModule::switchServer (ubyte4 newSrvrIp)
{
  bool retVal = false;

  AM_FUNC ("CdrModule::switchServer", LAYER_ID);
  AM_ENTER();

    srvrIp_ = newSrvrIp;

    if (pSoc_) // Release connection to the old server
    {
        delete pSoc_;
        pSoc_ = NULL;
    }

    if (retVal = socketInit())
    {
        AM_TRACE (("Connected to the new CDR server %s \n", GenSocket::getIp(srvrIp_)));
    }
    else
    {
        AM_TRACE (("Failed connect to the new CDR server %s \n", GenSocket::getIp(srvrIp_)));
    }

  AM_RETURN (retVal);
}


bool CdrModule::savePwd (LPCSTR lpPasswd)
{
  bool retVal = false;

  AM_FUNC ("CdrModule::savePwd", LAYER_ID);
  AM_ENTER();

    if (GenSocketSSL::isSupportSSL())
    {
        retVal = encryptPassword (lpPasswd, getPswdFile());
    }
    else
    {
        AM_ERROR (("SSL is not supported.\n"));
    }

  AM_RETURN (retVal);
}


bool CdrModule::readPwd ()
{
  bool retVal = false;

  AM_FUNC ("CdrModule::readPwd", LAYER_ID);
  AM_ENTER();

    // Read password from the password file
    if (GenSocketSSL::isSupportSSL() && !GenSocketSSL::isPswdSet())
    {
        retVal = decryptPassword (sPasswd_, getPswdFile());
        
#ifdef _AM_DEBUG_
        AM_DEBUG (("SSL password is \"%s\" of length %u\n",
                   sPasswd_, strlen (sPasswd_)            ));
#endif // _AM_DEBUG
    }

  AM_RETURN (retVal);
}


LPSTR CdrModule::getPswdFile ()
{
  static char szFile [MAX_PATH] = ""   ;
  static bool bSet              = false;

    if (!bSet)
    {
      LPSTR  pRoot  = NULL;
      size_t length = 0   ;

        // Construct full pathname for the SSL password file
        pRoot = getenv ("MNET_BASE");

        if (!pRoot || !*pRoot || (strlen (pRoot) > MAX_PATH - 10))
        {
            pRoot = DEF_ROOT_DIR;
        }

        if (pRoot)
            length = strlen(pRoot);

        sprintf (szFile, "%s%c%s", pRoot, ((length) ? DIR_DELIMITER : '\0'), CDR_PWD_FILE_NAME);

        bSet = true;
    }

  return (szFile);
}


// Interface for the HTTP module
int SavePwdCdr (LPCSTR lpPasswd)
{
    return ((int)CdrModule::savePwd(lpPasswd));
}


