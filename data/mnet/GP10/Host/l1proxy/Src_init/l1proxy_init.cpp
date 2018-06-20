/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

#include "vxWorks.h"
#include "memLib.h"
#include "string.h"
#include "intLib.h"
#include "logLib.h"
#include "taskLib.h"
#include "msgQLib.h"
#include "vxLib.h"
#include "errnoLib.h"
#include "stdio.h"
#include "dsp\dsptypes.h"
#include "dsp\dsphmsg.h"

// L1 Interface Message Queue for receiving messages from outside
MSG_Q_ID msgDspDn[2];
 
int l1pTrxSwapState = FALSE;


/*
 * sendDsp is the layer 1 API for the upper layers. It just copies
 * the message to a downstream message queue for later processing
 * by the downstream message task.
 */

void sendDsp
    (
    unsigned char *buffer,
    int len
    )
{
    STATUS status;
    unsigned char port;

    port = (((msgStruc*)buffer)->trx) & 0x1;  /* message for this port(dsp)  */

    status = msgQSend(msgDspDn[port], (char *) buffer, len, WAIT_FOREVER, MSG_PRI_NORMAL);
    if (status == ERROR)
        {
        printf("L1 proxy cannot queue a message for DSP %d\n",port);
        }
}


/*******************************************************************************
l1pTrxSwap

Swaps trx - dsp mapping
*/

STATUS l1pTrxSwap(UCHAR trxA, UCHAR trxB)
{
  l1pTrxSwapState = !l1pTrxSwapState;
  return OK;
}


