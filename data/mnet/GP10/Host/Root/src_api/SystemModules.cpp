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
// File        : SystemModules.cpp
// Author(s)   : Tim Olson
// Create Date : 10/18/2000
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "Os/JCModule.h"
#include "MnetModuleId.h"

JCModule *JCModule::systemModules[MNET_MAX_MODULE_IDS];


char MnetModuleNames [][MNET_MAX_MODULE_ID_NAME] = 
{
    "L1Proxy" ,
    "Message Delivery",
    "RR",
    "MM",
    "CC" ,
    "Lapdm",
    "Ril3-SMS",
    "OAM",
    "H323",
    "LUDB",
    "External GP10",
    "Logger",
    "MCH",
    "Alarm",
    "SysInit",
    "PM",
    "SMS-CBC",
    "VBLink",
    "CDR",
    "CISS",
    "TCP Server",
    "HOA",
    "BSSGP",
    "RLC_MAC",
    "GRR",
    "GSLink"
};

