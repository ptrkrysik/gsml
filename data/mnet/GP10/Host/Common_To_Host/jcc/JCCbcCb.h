#ifndef JCCbcCb_H
#define JCCbcCb_H
// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************
#include "MnetModuleId.h"
#include "JCCComIE.h"
#include "JcVcToVc.h"

#define INTER_CBC_CB_MAX_DAT_LEN  1320

//PR 1323, SMSCBC schedule message, 6/29
typedef enum {

  INTER_SBSCBC_CB_NULL_MSG = 0,

  // CB <-> CBC Messages --------------------------------------------
  INTER_SMSCBC_CB_START = INTER_SBSCBC_CB_NULL_MSG,
  INTER_SMSCBC_CB_START_ACK,
  INTER_SMSCBC_CB_STOP,
  INTER_SMSCBC_CB_STOP_ACK,
  INTER_SMSCBC_VC_UP, 

} InterCbcCbMsgType_t;

typedef enum {
    INTER_SMSCBC_MSG_CATEGORY_HIGH_PRIORITY = 0,
    INTER_SMSCBC_MSG_CATEGORY_BACKGROUND,
    INTER_SMSCBC_MSG_CATEGORY_NORMAL
} InterCbcCbMsgCategory_t; 

typedef struct {
        unsigned char    module_id;
        unsigned char    message_type;         // InterCbcCbMsgType_t 
        unsigned short   repetition_period;    // 1-1024 each unit = 8*51 multiframes = 1.883s 
        unsigned short   number_of_broadcasts; //  0-65535 
        unsigned char    message_category;     // InterCbcCbMsgCategory_t
        unsigned char    pad;                  // not used
        unsigned short   message_id;           // 0 to 65535
        unsigned short   size;                 // size of actual message
        unsigned char    data[INTER_CBC_CB_MAX_DAT_LEN]; 
} InterCbcCbData_t;
//PR 1323 end

typedef struct {
  // Header
  JcVipercellId_t            origVcId;       // origination ViperCell Id 
  UINT32                     origVcAddress;  // origination ViperCell IP address
  JcModuleId_t               origModuleId;   // origination Module Id
  JcSubId_t                  origSubId;      // optional origination Sub Id
  short                      pad1;           // not used for padding only

  JcVipercellId_t            destVcId;       // destination ViperCell Id 
  UINT32                     destVcAddress;  // destination ViperCell IP address
  JcModuleId_t               destModuleId;   // destination Module Id
  JcSubId_t                  destSubId;      // optional destination Sub Id
  short                      pad2;           // not used for padding only

  int                        Magic;          // This should always be 
  InterCbcCbMsgType_t        msgType;        // Message Type
  InterCbcCbData_t           msgData;        // Actual Message
                                           
} InterRxCbcCbMsg_t;

typedef struct {
  // Header
  JcVipercellId_t            origVcId;       // origination ViperCell Id 
  UINT32                     origVcAddress;  // origination ViperCell IP address
  JcModuleId_t               origModuleId;   // origination Module Id
  JcSubId_t                  origSubId;      // optional origination Sub Id
  short                      pad1;           // not used for padding only

  JcVipercellId_t            destVcId;       // destination ViperCell Id 
  UINT32                     destVcAddress;  // destination ViperCell IP address
  JcModuleId_t               destModuleId;   // destination Module Id
  JcSubId_t                  destSubId;      // optional destination Sub Id
  short                      pad2;           // not used for padding only

  int                        Magic;          // for Msg integrity, set to VIPERCELL_MAGIC_VALUE
  InterCbcCbMsgType_t        msgType;
  unsigned short             message_id;     // 0 to
  short                      pad3;           // not used for padding only
} InterTxCbcCbMsg_t; 

bool JcMsgRcvBySmscbc(InterRxCbcCbMsg_t *msg);
bool JcMsgSendToSmscbc(InterTxCbcCbMsg_t *msg);

#endif // JCCbcCb_H
