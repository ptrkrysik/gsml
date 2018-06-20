/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_MSGQRECEIVE_CPP__
#define __RM_MSGQRECEIVE_CPP__

#include "rm\rm_head.h"

int rm_msgQReceive(MSG_Q_ID QId, s8 *pRxMsg, unsigned int len, int flag)
{
    int result;
    T_CNI_RIL3MD_RRM_MSG *pRmMsg;

    result = msgQReceive(QId, pRxMsg, len, flag);
 
    return result;
}

#endif /* __RM_MSGQRECEIVE_CPP__ */
