// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacShow.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : Provides access to all RLC/MAC show routines.
//
// *******************************************************************


#include "RlcMacTask.h"



void ShowTAITableTrx(int trx)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowTAITable(trx);
}

void ShowTAITableTrxTs(int trx, int ts)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowTAITable(trx, ts);
}



void ShowUSFTableTrx(int trx)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowUSFTable(trx);
}

void ShowUSFTableTrxTs(int trx, int ts)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowUSFTable(trx, ts);
}



void ShowULTFITableTrx(int trx)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowULTFITable(trx);
}

void ShowDLTFITableTrx(int trx)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowDLTFITable(trx);
}


void ShowChannelCombTrx(int trx)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowChannelComb(trx);
}

void ShowChannelCombTrxTs(int trx, int ts)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowChannelComb(trx, ts);
}


void ShowTimeslotInfoTrx(int trx)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowTimeslotInfo(trx);
}

void ShowTimeslotInfoTrxTs(int trx, int ts)
{
    RlcMacTask::theRlcMacTask->tsPool.ShowTimeslotInfo(trx, ts);
}


void ShowAllocatedUplinkTBF()
{
    RlcMacTask::theRlcMacTask->tbfPool.ShowULAllocatedTBF();
}

void ShowAllocatedDownlinkTBF()
{
    RlcMacTask::theRlcMacTask->tbfPool.ShowDLAllocatedTBF();
}

void ShowFreeUplinkTBF()
{
    RlcMacTask::theRlcMacTask->tbfPool.ShowULFreeTBF();
}

void ShowFreeDownlinkTBF()
{
    RlcMacTask::theRlcMacTask->tbfPool.ShowDLFreeTBF();
}

void ShowAllUplinkTBF()
{
    RlcMacTask::theRlcMacTask->tbfPool.ShowULAllTBF();
}

void ShowAllDownlinkTBF()
{
    RlcMacTask::theRlcMacTask->tbfPool.ShowDLAllTBF();
}
