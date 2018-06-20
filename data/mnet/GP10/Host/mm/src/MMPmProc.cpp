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
// File        : MMPmProc.cpp
// Author(s)   : Kevin Lim
// Create Date : 07-27-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// function prototypes
// *******************************************************************

#include "stdio.h"
#include "defs.h"
#include "oam_api.h"
#include "PM/pm_class.h"
#include "MMPmProc.h"

bool DEBUG_MM_PMSET = false;

void mmIncAttTrnPaging()
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report added attTransOfPagingMessagesThePCH\n");
	PM_CellMeasurement.attTransOfPagingMessagesThePCH.increment();
}

void mmIncUnsuccTrnPaging()
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report added unsuccTransOfPagingMessagesThePCH\n");
	PM_CellMeasurement.unsuccTransOfPagingMessagesThePCH.increment();
}

void mmUpdMeanSuccPaging(int val)
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report updated meanDurationOfSuccPagingProcs\n");
	PM_CellMeasurement.meanDurationOfSuccPagingProcs.addRecord(val);
}

void mmIncAttMOCalls()
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report added attMobileOriginatingCalls\n");
	PM_CCMeasurement.attMobileOriginatingCalls.increment();
}

void mmIncAttMOEmergencyCalls()
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report added attMobileOriginatingEmergencyCalls\n");
	PM_CCMeasurement.attMobileEmergencyCalls.increment();
}

void mmIncAttCipher()
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report added attCipheringModeControlProcs\n");
	PM_CCMeasurement.attCipheringModeControlProcs.increment();
}

void mmIncSuccCipher()
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report added succCipheringModeControlProcs\n");
	PM_CCMeasurement.succCipheringModeControlProcs.increment();
}

void mmUpdMeanLocUpTime(int val)
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report updated meanTimeToLocationUpdateService\n");
	PM_CCMeasurement.meanTimeToLocationUpdateService.addRecord(val);
}

void mmIncTrnSubIdIMSI()
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report added transSubIdentifiedWithIMSI\n");
	PM_CCMeasurement.transSubIdentifiedWithIMSI.increment();
}

void mmIncIMSIDetach()
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report added imsiDetachProcs\n");
	PM_CCMeasurement.imsiDetachProcs.increment();
}

void mmIncIMSIAttach()
{
	if(DEBUG_MM_PMSET) 
		printf("[MM] PM report added imsiAttachProcs\n");
	PM_CCMeasurement.imsiAttachProcs.increment();
}


