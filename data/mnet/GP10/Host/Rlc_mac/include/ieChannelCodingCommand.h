// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieChannelCodingCommand.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IECHANNELCODINGCOMMAND_H__
#define __IECHANNELCODINGCOMMAND_H__

#include "IeBase.h"
#include "bitstream.h"

// *******************************************************************
// class IeChannelCodingCommand
//
// Description:
//    PR Mode IE -- GSM04.60 11.2.29
// *******************************************************************


class IeChannelCodingCommand : public IeBase {
public:

    typedef enum
    {
        CS_1,
        CS_2,
        CS_3,
        CS_4
    } CHANNEL_CODING_COMMAND;

    IeChannelCodingCommand() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetChannelCodingCommand(CHANNEL_CODING_COMMAND val)
    {
        coding = val;
        isValid = TRUE;
        return (RLC_MAC_SUCCESS);
    }
   
    CHANNEL_CODING_COMMAND GetChannelCodingCommand()
    {
        return (coding);
    }
    
    int GetNumFillBytes()
    {
        switch(coding)
        {
            case CS_1 : return (20);    // 160 / 8
            case CS_2 : return (30);    // 240 / 8
            case CS_3 : return (36);    // 288 / 8
            case CS_4 : return (50);    // 400 / 8
        }
    }
   
    int GetRLCDataBlockSize()
    {
        switch(coding)
        {
            case CS_1 : return (23);
            case CS_2 : return (33);
            case CS_3 : return (39);
            case CS_4 : return (53);
        }
    }
   
    void ClearChannelCodingCommand() { isValid = FALSE; }
   
private:
    CHANNEL_CODING_COMMAND coding;
};

#endif