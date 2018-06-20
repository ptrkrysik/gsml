// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : SmsFsm.cpp
// Author(s)   : Igal Gutkin
// Create Date : 01-20-2000
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
//

#include <vxworks.h>

#include "CC/SmsHalfCall.h"

const int SmsOneFsm_ = 0;

/* ================================================================ */
/* SMS FSM tables */

/* MO SMS FSM tables */
const smsEventTableEntry_t smsHandlersMoIdle [] =
{
    {SMS_EVENT_MM_EST_IND     , (SFH)smsHandlerMmEstInd , SMS_STATE_CONNECTED  },

    {SMS_EVENT_TIMEOUT        , smsHandlerDef           , SMS_STATE_NONE       },

    {SMS_EVENT_DEFAULT        , smsHandlerDef           , SMS_STATE_NONE       } 
}; 


// SMS_STATE_MT_CONNECT_MM is valid for MT FSM only
const smsEventTableEntry_t smsHandlersMoInvalid   [] = 
{
    {SMS_EVENT_DEFAULT        , smsHandlerDef           , SMS_STATE_IDLE       } 
};


const smsEventTableEntry_t smsHandlersMoConnected [] =
{
    {SMS_EVENT_MM_REL_IND     , smsHandlerMmRelInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_MD_ERROR_IND   , smsHandlerMdErrInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_CP_DATA        , (SFH)smsHandlerCpData   , SMS_STATE_NONE       },
    {SMS_EVENT_CP_ACK         , (SFH)smsHandlerCpReject , SMS_STATE_NONE       },
    {SMS_EVENT_CP_ERROR       , (SFH)smsHandlerCpError  , SMS_STATE_IDLE       },

    {SMS_EVENT_MNSMS_DATA_REQ , (SFH)smsHandlerMnDataReq, SMS_STATE_WAIT_CP_ACK},
    {SMS_EVENT_MNSMS_REL_REQ  , smsHandlerDef           , SMS_STATE_IDLE       },
    {SMS_EVENT_MNSMS_ABORT_REQ, (SFH)smsHandlerMnAbortReq,SMS_STATE_DISCONNECT },

    {SMS_EVENT_TIMEOUT        , smsHandlerTimeout       , SMS_STATE_IDLE       },

    {SMS_EVENT_DEFAULT        , smsHandlerDef           , SMS_STATE_NONE       } 
};

const smsEventTableEntry_t smsHandlersMoWaitCpAck [] =
{
//    {SMS_EVENT_MM_EST_IND     , smsHandlerMmRelInd      , SMS_STATE_NONE       }, //Concatineted MO Msg notifitation
    {SMS_EVENT_MM_REL_IND     , smsHandlerMmRelInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_MD_ERROR_IND   , smsHandlerMdErrInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_CP_DATA        , (SFH)smsHandlerCpData   , SMS_STATE_NONE       },
    {SMS_EVENT_CP_ACK         , (SFH)smsHandlerCpAck    , SMS_STATE_CONNECTED  },
    {SMS_EVENT_CP_ERROR       , (SFH)smsHandlerCpError  , SMS_STATE_IDLE       },

    {SMS_EVENT_MNSMS_REL_REQ  , smsHandlerDef           , SMS_STATE_DISCONNECT },
    {SMS_EVENT_MNSMS_ABORT_REQ, (SFH)smsHandlerMnAbortReq,SMS_STATE_DISCONNECT },

    {SMS_EVENT_TIMEOUT        , smsHandlerTimeoutAck    , SMS_STATE_NONE       },

    {SMS_EVENT_DEFAULT        , smsHandlerDef           , SMS_STATE_NONE       } 
};

/*==================================================================*/
/* MT SMS FSM tables */

const smsEventTableEntry_t smsHandlersMtIdle [] =
{
    {SMS_EVENT_MNSMS_EST_REQ  , (SFH)smsHandlerMnEstReq , SMS_STATE_MT_CONNECT_MM},

    {SMS_EVENT_TIMEOUT        , smsHandlerDef           , SMS_STATE_NONE       },

    {SMS_EVENT_DEFAULT        , smsHandlerDef           , SMS_STATE_NONE       } 
};


/* Wait for MM (SAPI=0) and L2 (SAPI=3) connection establishment */
const smsEventTableEntry_t smsHandlersMtWaitMm [] =
{
    {SMS_EVENT_MM_EST_CNF     , smsHandlerMmEstCnf      , SMS_STATE_NONE       },
    {SMS_EVENT_MM_EST_REJ     , smsHandlerMmEstRej      , SMS_STATE_IDLE       },
    {SMS_EVENT_MM_REL_IND     , smsHandlerMmRelInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_MD_ERROR_IND   , smsHandlerMdErrInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_MNSMS_REL_REQ  , (SFH)smsHandlerMnRelReq , SMS_STATE_IDLE       },
    {SMS_EVENT_MNSMS_ABORT_REQ, (SFH)smsHandlerMnAbortReq,SMS_STATE_DISCONNECT },

    {SMS_EVENT_TIMEOUT        , smsHandlerTimeout       , SMS_STATE_IDLE       },

    {SMS_EVENT_DEFAULT        , smsHandlerDef           , SMS_STATE_NONE       } 
};


const smsEventTableEntry_t smsHandlersMtConnected [] =
{
    {SMS_EVENT_MM_REL_IND     , smsHandlerMmRelInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_MD_ERROR_IND   , smsHandlerMdErrInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_CP_DATA        , (SFH)smsHandlerCpData   , SMS_STATE_NONE       },
    {SMS_EVENT_CP_ACK         , (SFH)smsHandlerCpReject , SMS_STATE_NONE       },
    {SMS_EVENT_CP_ERROR       , (SFH)smsHandlerCpError  , SMS_STATE_IDLE       },

    {SMS_EVENT_MNSMS_DATA_REQ , (SFH)smsHandlerMnDataReq, SMS_STATE_WAIT_CP_ACK},
    {SMS_EVENT_MNSMS_REL_REQ  , (SFH)smsHandlerMnRelReq , SMS_STATE_DISCONNECT },
    {SMS_EVENT_MNSMS_ABORT_REQ, (SFH)smsHandlerMnAbortReq,SMS_STATE_DISCONNECT },

    {SMS_EVENT_TIMEOUT        , smsHandlerTimeout       , SMS_STATE_IDLE       },

    {SMS_EVENT_DEFAULT        , smsHandlerDef           , SMS_STATE_NONE       } 
};


const smsEventTableEntry_t smsHandlersMtWaitCpAck [] =
{
    {SMS_EVENT_MM_REL_IND     , smsHandlerMmRelInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_MD_ERROR_IND   , smsHandlerMdErrInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_CP_DATA        , (SFH)smsHandlerCpData   , SMS_STATE_CONNECTED  },
    {SMS_EVENT_CP_ACK         , (SFH)smsHandlerCpAck    , SMS_STATE_CONNECTED  },
    {SMS_EVENT_CP_ERROR       , (SFH)smsHandlerCpError  , SMS_STATE_IDLE       },

//    {SMS_EVENT_MNSMS_DATA_REQ , smsHandlerDef           , SMS_STATE_NONE       },
    {SMS_EVENT_MNSMS_REL_REQ  , (SFH)smsHandlerMnRelReq , SMS_STATE_DISCONNECT },
    {SMS_EVENT_MNSMS_ABORT_REQ, (SFH)smsHandlerMnAbortReq,SMS_STATE_DISCONNECT },

    {SMS_EVENT_TIMEOUT        , smsHandlerTimeoutAck    , SMS_STATE_NONE       },

    {SMS_EVENT_DEFAULT        , smsHandlerDef           , SMS_STATE_NONE       } 
};


/*==================================================================*/
/* Common tables */

//SMS_STATE_DISCONNECT
const smsEventTableEntry_t smsHandlersDisconnect [] =
{
    {SMS_EVENT_MM_REL_IND     , smsHandlerMmRelInd      , SMS_STATE_IDLE       },
    
    {SMS_EVENT_MD_ERROR_IND   , smsHandlerMdErrInd      , SMS_STATE_IDLE       },

    {SMS_EVENT_CP_ACK         , (SFH)smsHandlerCpAck    , SMS_STATE_IDLE       },
    {SMS_EVENT_CP_ERROR       , (SFH)smsHandlerCpError  , SMS_STATE_IDLE       },

    {SMS_EVENT_TIMEOUT        , smsHandlerTimerDisc     , SMS_STATE_NONE       },

    {SMS_EVENT_DEFAULT        , smsHandlerDef           , SMS_STATE_NONE       } 

};

/*==================================================================*/
/* State tables */

const smsEventTableEntry_t *smsMoTable [SMS_STATE_MAX] =
{
    smsHandlersMoIdle     ,
    smsHandlersMoInvalid  ,
    smsHandlersMoConnected,
    smsHandlersMoWaitCpAck,
    smsHandlersDisconnect 
};

const smsEventTableEntry_t *smsMtTable [SMS_STATE_MAX] =
{
    smsHandlersMtIdle     ,
    smsHandlersMtWaitMm   ,
    smsHandlersMtConnected,
    smsHandlersMtWaitCpAck,
    smsHandlersDisconnect 
};


/*==================================================================*/
/*
const smsEventTableEntry_t smsHandlersState??? [] =
{
    {SMS_EVENT_MM_EST_IND     , smsHandlerDef, SMS_STATE_NONE },
    {SMS_EVENT_MM_EST_CNF     , smsHandlerDef, SMS_STATE_NONE },
    {SMS_EVENT_MM_EST_REJ     , smsHandlerDef, SMS_STATE_NONE },
    {SMS_EVENT_MM_REL_IND     , smsHandlerDef, SMS_STATE_NONE },

    {SMS_EVENT_MD_ERROR_IND   , smsHandlerDef, SMS_STATE_NONE },

    {SMS_EVENT_CP_DATA        , smsHandlerDef, SMS_STATE_NONE },
    {SMS_EVENT_CP_ACK         , smsHandlerDef, SMS_STATE_NONE },
    {SMS_EVENT_CP_ERROR       , smsHandlerDef, SMS_STATE_NONE },

    {SMS_EVENT_MNSMS_DATA_REQ , smsHandlerDef, SMS_STATE_NONE },
    {SMS_EVENT_MNSMS_REL_REQ  , smsHandlerDef, SMS_STATE_NONE },
    {SMS_EVENT_MNSMS_ABORT_REQ, smsHandlerDef, SMS_STATE_NONE },
    {SMS_EVENT_TIMEOUT        , smsHandlerDef, SMS_STATE_NONE },
    {SMS_EVENT_DEFAULT        , smsHandlerDef, SMS_STATE_NONE }
};
*/