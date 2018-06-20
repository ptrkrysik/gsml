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
// File        : SmsLeg.cpp
// Author(s)   : Igal Gutkin
// Create Date : 26-05-2000
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
//

#include <vxworks.h>

#include "cc/ccsessionhandler.h"
#include "cc/smqueue.h"




/*=========================================================================*/

void SmQueue::flash ()
{
  unsigned index = 0;

  DBG_FUNC ("SmQueue::flash", CC_SESSION_LAYER);
  DBG_ENTER();

    lock();

    while (!isEmpty ())
    {
        DBG_TRACE ("SmQueue: Send PDU %d to LAPDm\n", (index+1));
        parent_.sendL2Msg (storage_ + index++);
        --queueCounter_;
    }

    initData();

  DBG_LEAVE();
}


bool SmQueue::add (const T_CNI_LAPDM_L3MessageUnit * inMsg)
{
  bool retVal = false;

  DBG_FUNC ("SmQueue::add", CC_SESSION_LAYER);
  DBG_ENTER();

    if (!(isLocked() || isFull()))
    {
        storage_[queueCounter_++] = *inMsg;
        retVal                    =  true ;
    }
    else
        DBG_ERROR ("SmQueue: attempt to add message to the full queue. lock=%d, full=%d\n",
                   isLocked(), isFull() );

  DBG_LEAVE();
  return   (retVal);
}


/*=========================================================================*/

void SmQueueMm::flash ()
{
  unsigned index = 0;

  DBG_FUNC ("SmQueueMm::flash", CC_SESSION_LAYER);
  DBG_ENTER();

    lock();

    while (!isEmpty ())
    {
        DBG_TRACE ("SmQueueMm: Send MM Message %d\n", (index+1));
        parent_.parent->sendMMMsg (storage_ + index++);
        --queueCounter_;
    }

    initData();

  DBG_LEAVE();
}


bool SmQueueMm::add (const IntraL3Msg_t * inMsg)
{
  bool retVal = false;

  DBG_FUNC ("SmQueueMm::add", CC_SESSION_LAYER);
  DBG_ENTER();

    if (!(isLocked() || isFull()))
    {
        storage_[queueCounter_++] = *inMsg;
        retVal                    =  true ;
    }
    else
        DBG_ERROR ("SmQueueMm: attempt to add message to the full queue. lock=%d, full=%d\n",
                   isLocked(), isFull() );

  DBG_LEAVE();
  return   (retVal);
}

