/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RML1_INTF_H__
#define __RML1_INTF_H__

#include "vxWorks.h"
#include "msgQlib.h"
#include "string.h"
#include "MnetModuleId.h"
#include "logging\vclogging.h"
#include "debug.h"

#define MPH_INFO_REQ					0
#define MPH_INFO_IND					1
#define RML1_MAXMSG_LEN					100
#define L1RM_MAXMSG_LEN					100

extern  DbgOutput					rm_Dbg;
extern  MSG_Q_ID					rm_MsgQId;

typedef struct {
		unsigned char				module_id;
		unsigned char				entry_id;
		unsigned char				primitive_type;
		unsigned char				filler;
		struct  {
							unsigned short	msgLength;
							unsigned char	buffer[RML1_MAXMSG_LEN];

			}l3_data;

	}rml1_Msg_t;

void api_SendMsgFromRmToL1				(unsigned short length, unsigned char *msg);
void api_SendMsgFromL1ToRm				(unsigned short, unsigned char*);

#endif // __RML1_INTF_H__


