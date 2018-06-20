// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : ieStartFrameNum.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __IESTARTFRAMENUM_H__
#define __IESTARTFRAMENUM_H__

#include "IeBase.h"
#include "bitstream.h"


// *******************************************************************
// class IeStartFrameNum
//
// Description:
//    Start Frame Number IE -- GSM04.60 12.21
//
// *******************************************************************

#define MAX_K_NUM   8192
#define MAX_T1_VAL  32
#define MAX_T2_VAL  32
#define MAX_T3_VAL  64


class IeStartFrameNum : public IeBase {
public:

    typedef enum
    {
        ABSOLUTE_FRAME_NUM,
        RELATIVE_FRAME_NUM
    } START_FRAME_TYPE;
   
    IeStartFrameNum() : IeBase() {}
   
    RlcMacResult EncodeIe(BitStreamOut &dataStream);
    void DisplayDetails(DbgOutput *outObj);
   
    RlcMacResult SetStartFrameNum(unsigned short kVal)
    {
        if (kVal <= MAX_K_NUM)
        {
            k = kVal;
            isValid = TRUE;
            startFrameType = RELATIVE_FRAME_NUM;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    RlcMacResult SetStartFrameNum(unsigned char t1, unsigned char t2,
        unsigned char t3)
    {
        if ((t1 <= MAX_T1_VAL) && (t2 <= MAX_T2_VAL) && (t2 <= MAX_T2_VAL))
        {
            T1 = t1;
            T2 = t2;
            T3 = t3;
            isValid = TRUE;
            startFrameType = ABSOLUTE_FRAME_NUM;
            return (RLC_MAC_SUCCESS);
        }
        else
        {
            return (RLC_MAC_PARAMETER_RANGE_ERROR);
        }
    }
   
    unsigned short GetStartFrameNumRelative()
    {
        return (k);
    }
    
    void GetStartFrameNumAbsolute (unsigned char *t1, unsigned char *t2,
        unsigned char *t3)
    {
        *t1 = T1;
        *t2 = T2;
        *t3 = T3;
    }
   
    void ClearStartFrameNum() { isValid = FALSE; }
   
private:
    START_FRAME_TYPE startFrameType;
    unsigned short k;
    unsigned char T1;
    unsigned char T2;
    unsigned char T3;
};

#endif