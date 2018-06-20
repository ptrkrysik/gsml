// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : MsgBase.h
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#ifndef __MSGBASE_H__
#define __MSGBASE_H__

#include "RlcMacResult.h"
#include "RlcMacMsgType.h"
#include "logging\vclogging.h"


// *******************************************************************
// class MsgBase
//
// Description
//    
// *******************************************************************
class MsgBase {
public:
   RlcMacMsgType     msgType;

   // Constructor
   MsgBase(RlcMacMsgType type) : msgType(type) {}
   // Destructor
   ~MsgBase() {};
   
protected:
   static DbgOutput MsgDisplayObj;
   static DbgOutput MsgDisplayDetailObj;
   
   // Default constuctor
   MsgBase() {};
};



// *******************************************************************
// class DownlinkMsgBase
//
// Description
//    
// *******************************************************************
class DownlinkMsgBase : public MsgBase {
public:
   DownlinkMsgBase(RlcMacMsgType type) : MsgBase(type) {}
   ~DownlinkMsgBase() {}
   
   void DisplayMsgShort(BitStreamOut *dataStream);
   virtual void DisplayMsgDetail() = 0;
   virtual RlcMacResult EncodeMsg(BitStreamOut &dataStream) = 0;
   
protected:

   // Default constructor
   DownlinkMsgBase() {}
};



// *******************************************************************
// class UplinkMsgBase
//
// Description
//    
// *******************************************************************
class UplinkMsgBase : public MsgBase {
public:
   UplinkMsgBase(RlcMacMsgType type) : MsgBase(type) {}
   ~UplinkMsgBase() {}
   
   void DisplayMsgShort(BitStreamIn *dataStream);
   
protected:

   // Default contructor
   UplinkMsgBase(){}
};




#endif