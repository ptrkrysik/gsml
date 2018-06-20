#ifndef JCCVCMSG_H
#define JCCVCMSG_H

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
// File        : JCCVcMsg.h
// Author(s)   : Igal Gutkin
// Create Date : 12-08-99
// Description : message definition for the VipeCell Update sub-system
//
// *******************************************************************

#include "JCCComIE.h"
#include "voip\vblink.h"

// ViperCell update Info 
typedef struct 
{
	JcVipercellId_t	    btsID ;
	VBLINK_VC_STATUS_INFO status;

} IntraL3InfoVC_t;

#endif // JCCVCMSG_H

