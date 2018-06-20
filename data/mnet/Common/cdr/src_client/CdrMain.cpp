// *******************************************************************
//
// (c) Copyright CISCO Systems Inc., 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 2.0
// Status      : Under development
// File        : CdrMain.cpp
// Author(s)   : Igal Gutkin
// Create Date : 09-09-00
// Description : CDR client module main task function implementation
//
// *******************************************************************


// *******************************************************************
// Include Files.
// *******************************************************************

#include "StdAfx.h"

#include <msgQLib.h>

#include "AlarmCode.h"

#include "CdrModule.h"


//////////////////////////////////////////////////////////////////////
// Static data members
//////////////////////////////////////////////////////////////////////
bool CdrModule::bExtraInfo_ = 
#ifdef _AM_DEBUG_
 true;
#else
 false;
#endif // _AM_DEBUG_

//////////////////////////////////////////////////////////////////////
// Cdr task main loop functions
//////////////////////////////////////////////////////////////////////

bool CdrModule::mainTaskHandle ()
{
  int         errorCode;
  bool        retVal   = true;

  CdrModule * module   = CdrModule::getInstance();

  AM_FUNC ("CdrModule::mainTaskHandle", LAYER_ID);
  AM_ENTER();

    if (module && pMainQueue_)
    {
        // Load system data
        if ((retVal = module->loadData()) == false)
        {
            raiseModuleAlarm (EC_CDR_CLIENT_INIT_FAILED);
            module->disable  ();

            // keep going in order to allow the user to update a MIB variable when MIB is not
            // initialized properly
            // AM_RETURN (retVal);
        }
        else
        {
            cleanModuleAlarm ();
            cleanSslAlarm    ();

            // Setup CDR Server connection
            if (!module->socketInit())
            {
                AM_TRACE (("CDR Client: Unable to establish link to CDR Server.\n"));
                // Continue. Will try to establish connection later
            }

            AM_TRACE (("CDR Server IP address is set to %#08lx (%s)\n", module->srvrIp_,
                       GenSocket::getIp(module->srvrIp_)                               ));

            if (bExtraInfo_)
                module->printInfo ();
        }

        module->mainTask_.JCTaskEnterLoop();

        while (true)
        {
            // Allow other tasks with the same priority to run before retriving the next 
            // message from the Queue.
            taskDelay (NO_WAIT);

            if (module->getMsg () == true)
            {
                AM_DEBUG (("CDR Client receivced a new message\n"));

                if (!module->getStatus())
                {
                    ++(module->lostRecs_);
                    AM_TRACE (  ("CDR Client: Logging is disabled. Discard the record %d.\n %s",
                              (module->sentRecs_+module->lostRecs_), 
                              module->inMsgBuf_.record             )  );
                    continue;
                }

                if (!module->isConnected() && !module->socketReset())
                {
                    ++(module->lostRecs_);

                    raiseSslAlarm    ();
                    raiseModuleAlarm (EC_CDR_CLIENT_UNABLE_CONNECT_SERVER);

                    AM_TRACE (("CDR Client: Failed connect to server. Discard the record %d:\n %s\n",
                              (module->sentRecs_+module->lostRecs_),
                              module->inMsgBuf_.record             ));
                    continue;
                }

                cleanModuleAlarm ();
                cleanSslAlarm    ();

                if (module->inMsgBuf_.msgType == CDR_GENERIC_MSG_TYPE)
                {
                    AM_DEBUG (("CDR Client: Processing new log:\n%100s\n", module->inMsgBuf_.record));

                    errorCode = module->pSoc_->sendData (module->inMsgBuf_.record);

                    switch (errorCode)
                    {
                    case CDR_STATUS_OK:
                        ++(module->sentRecs_);
                        AM_TRACE (("CDR Client: Record %d sent successully\n", 
                                   (module->sentRecs_+module->lostRecs_)));
                        break;

                    case CDR_STATUS_MSG_TOO_LONG:
                        // message can't be sent over the network
                        ++(module->lostRecs_);
                        AM_WARNING (("CDR Client: Record is too long. Discard record %d.\n",
                                     (module->sentRecs_+module->lostRecs_)                 ));
                        break;

                    case CDR_STATUS_FATAL_SOC_ERR:
                        // fatal socket error
                        // attempt to reconnect to the CDR server
                        AM_TRACE (("CDR Client: Server connection is broken. Attempt to reconnect.\n"));
                        if (!module->socketReset())
                        {
                            ++(module->lostRecs_);

                            cleanModuleAlarm ();
                            raiseModuleAlarm (EC_CDR_CLIENT_UNABLE_CONNECT_SERVER);

                            AM_TRACE (("CDR Client: Failed reconnect to server. Discard record %d.\n",
                                       (module->sentRecs_+module->lostRecs_)));
                            break;
                        }
                        // break is omitted intentionally, continue to resend

                    case CDR_STATUS_TEMP_SOC_ERR: 
                        // non-fatal error
                        // buffer overloading. It's possible to retry
                        if (module->pSoc_->sendData (module->inMsgBuf_.record) == CDR_STATUS_OK)
                        {
                            ++(module->sentRecs_);
                            AM_TRACE (("CDR Client: Record %d resent successully\n", 
                                       (module->sentRecs_+module->lostRecs_)) );
                        }
                        else
                        {
                            ++(module->lostRecs_);
                            AM_TRACE (("CDR Client: Record sending has failed. Discard record %d.\n",
                                       (module->sentRecs_+module->lostRecs_)) ) ;
                        }
                        break;

                    default:
                        AM_TRACE (("CDR Client: Unknown return code %d from CdrRemClientSocket::sendData.\n",
                                   errorCode));
                        break;
                    }
                }
                else
                {
                    ++(module->lostRecs_);
                    AM_TRACE (("CDR Client: Unknown CDR Message type %lx. Discard record %d.\n",
                               module->inMsgBuf_.msgType, (module->sentRecs_+module->lostRecs_)));
                }
            }
            else if (pMainQueue_)
            { // Error on the Queue
                AM_ERROR (("CDR Client: Error on CDR task queue read attempt.\n"));
            }
            else
            { // Queue is deleted. Exit
                break;
            }
        } // while

    }
    else
    {
        AM_ERROR (("CDR Client module is not initialised.\n"));
        retVal = false;
    }

  AM_RETURN (retVal);
}

