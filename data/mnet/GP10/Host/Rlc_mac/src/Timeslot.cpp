// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : Timeslot.cpp
// Author(s)   : Tim Olson
// Create Date : 12/5/2000
// Description : 
//
// *******************************************************************


#include "Timeslot.h"

#include "logging\vclogging.h"
#include "logging\vcmodules.h"


void Timeslot::ShowTimeslotInfo()
{
    printf("Channel Comb (%d)   Data RTS count (%d)   Signalling RTS count (%d)   RTI (%d)\n",
        channelComb, dataRTSCount, signallingRTSCount, rti);
    sysTAI.ShowTAITable();
    sysUSF.ShowUSFTable(); 
}





    int                     channelComb;
    int                     dataRTSCount;
    int                     signallingRTSCount;
    TAITable                sysTAI;
    USFTable                sysUSF;
    DLSignallingMsgQueue    sysDLSigMsgQ;
    unsigned char           rti;
