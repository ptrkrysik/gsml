// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : JCMsgQDefs.h
// Author(s)   : Tim Olson
// Create Date : 9/18/2000
// Description : 
//
// *******************************************************************
#ifndef _JCMSGQDEFS_H_
#define _JCMSGQDEFS_H_  /* include once only */

#include "Os/JCMsgQueue.h"
#include "MnetModuleId.h"


/* THIS IS FOR MESSAGE QUEUE INTERNAL ONLY!!!! */

typedef struct jc_msg_hdr {
    JCMsgQueue *rplyQ;              /* identifier of queue to which to reply, 
                                       if any */
    unsigned int msgType;           /* module dependent message type */
    MNET_MODULE_ID modId;           /* module ID of sender */
    int bytes;                      /* size of associated data */
    } JC_MSG_HDR;
/* data follows header */


#define JC_MSG_HDR_SIZE     sizeof(JC_MSG_HDR)


#endif
