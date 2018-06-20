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
// File        : MMapiH323.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 02-01-98
// Description :  
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

// included L3 Message header for messages from other Layer 3 modules (CC, RR)
#include "oam_api.h"
#include "ril3/ie_location_area_id.h"
#include "MM/MMInt.h"
#include "MM/MMConfig.h"
#include "MMTypes.h"


// OAM configuration table defined in vipermib.h as
//	INTEGER			mm_max_mm_connections;	
//	INTEGER			mm_max_L3_msgs;			
//	INTEGER			mm_L3_msg_Q_priority;	
//	INTEGER			task_priority;			
//	INTEGER			task_stack_size;		
//	INTEGER			mm_paging_t3113;		
//	Boolean			mm_authenticationRequired;	
//	Boolean			mm_cipheringRequired;		
//	Boolean			mm_IMEICheckRequired;		

MobilityConfigData				mmCfg;
T_CNI_RIL3_IE_LOCATION_AREA_ID	mmCfgLocId;
int								mmOpStatus = 1;
int								mmCfgCipherMO = 0;
int								mmCfgCipherMT = 0;
int								mmCfgIMSIrequired = 0;

// Global Variable for current MM Connections
MMEntry_t mmEntries[MM_MAXNUM_ENTRY];


