// *******************************************************************
//
// (c) Copyright Cisco Systems Inc. 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// Version     : 1.0
// Status      : Under development
// File        : CCHndUtil.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 05-01-99
// Description : 
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "CC/CCHalfCall.h"

#include "CC/CCInt.h"

#include "logging/VCLOGGING.h"

#include "jcc/JCCLog.h"

#include "defs.h"
#include "oam_api.h"

//GCELL<tyu:06-01-01> BEGIN
extern GlobalCellId_t MY_GLOBALCELL_ID;
//GCELL<tyu:06-01-01> END

#include "pm/pm_class.h"

#include "csunew/csu_head.h"

#include "CC/hortp.h"

#include "CC/CCHandover.h"


void 
CCHandover::sendVcMsg(JcModuleId_t origModule,
                      JcModuleId_t destModule)
{
  DBG_FUNC("CCHandover::sendVcMsg", CC_HANDOVER_LAYER);
  DBG_ENTER();

  vcOutMsg.origModuleId = origModule;
  vcOutMsg.origSubId = callIndex; 
//GCELL<tyu:06-01-01> BEGIN
  vcOutMsg.origVcId.cellId =  MY_GLOBALCELL_ID.ci;
  vcOutMsg.origVcId.networkId =  MY_GLOBALCELL_ID.lac;
//GCELL<tyu:06-01-01> END
  // lac setting
  vcOutMsg.origVcAddress = MY_VIPERCELL_IP_ADDRESS;

//GCELL<tyu:06-01-01> BEGIN
  vcOutMsg.destVcId.cellId = hoGlobalCellId_.ci;
  vcOutMsg.destVcId.networkId = hoGlobalCellId_.lac;
//GCELL<tyu:06-01-01> END
  vcOutMsg.destVcAddress = hoVcIpAddress_;
  vcOutMsg.destModuleId = destModule;
  vcOutMsg.destSubId = hoVcCallIndex_;

  // send the message.
  if (! JcMsgSendToVipercell(&vcOutMsg) )
    {
      JCCLog1("CC Call Ho Error : sendVcMsg (dest Vipercell Address = %d) error\n ", 
              vcOutMsg.destVcAddress);
      DBG_ERROR("CC Call Ho Error : sendVcMsg (dest Vipercell Address = %d) error\n ", 
                vcOutMsg.destVcAddress);
    }
  else
    {
      // peg here when we are successful
      switch (vcOutMsg.msgType)
        {
        case INTER_VC_CC_PERFORM_HANDOVER:
        case INTER_VC_CC_PERFORM_HANDBACK:
          PM_CCMeasurement.attOutgoingInterMSCHDOs.increment();
          break;

        case INTER_VC_CC_COMPLETE_HANDOVER:
        case INTER_VC_CC_COMPLETE_HANDBACK:
          PM_CCMeasurement.succIncomingInterMSCHDOs.increment();
          break;

        default:
          break;
        }
	

      JCCLog2("CC Call Ho Log: Sent Message to (VC# = %d) with (ip address = %d)\n",  
              vcOutMsg.destVcId.cellId,
              vcOutMsg.destVcAddress);

      DBG_TRACE("CC Call Ho Log: Sent Message to (VC# = %d) with (ip address = %d)\n",  
                vcOutMsg.destVcId.cellId,
                vcOutMsg.destVcAddress);
    }
  DBG_LEAVE();
}  


//HO<xxu:01-24-00>used to send message to MSC-B'
void 
CCHandover::sendVcMsgH3(JcModuleId_t origModule,
                        JcModuleId_t destModule)
{
  DBG_FUNC("CCHandover::sendVcMsgH3", CC_HANDOVER_LAYER);
  DBG_ENTER();

  vcOutMsg.origModuleId = origModule;
  vcOutMsg.origSubId = callIndex; 
//GCELL<tyu:06-01-01> BEGIN
  vcOutMsg.origVcId.cellId = MY_GLOBALCELL_ID.ci;
  vcOutMsg.origVcId.networkId = MY_GLOBALCELL_ID.lac;
//GCELL<tyu:06-01-01> END
  vcOutMsg.origVcAddress = MY_VIPERCELL_IP_ADDRESS;

//GCELL<tyu:06-01-01> BEGIN
  vcOutMsg.destVcId.cellId = h3GlobalCellId_.ci;
  vcOutMsg.destVcId.networkId = h3GlobalCellId_.lac;
//GCELL<tyu:06-01-01> END
  vcOutMsg.destVcAddress = h3VcIpAddress_;
  vcOutMsg.destModuleId = destModule;
  vcOutMsg.destSubId = h3VcCallIndex_;

  // send the message.
  if (! JcMsgSendToVipercell(&vcOutMsg) )
  {
      JCCLog1("CC Error(JCC) : sendVcMsgH3 (dest Vipercell Address = %d) error\n ", 
              vcOutMsg.destVcAddress);
      DBG_ERROR("CC Error(JCC) : sendVcMsgH3 (dest Vipercell Address = %d) error\n ", 
                vcOutMsg.destVcAddress);
  }
  else
  {
      // peg here when we are successful
      switch (vcOutMsg.msgType)
      {
      case INTER_VC_CC_PERFORM_HANDOVER:
      case INTER_VC_CC_PERFORM_HANDBACK:
           PM_CCMeasurement.attOutgoingInterMSCHDOs.increment();
           break;

      default:
           break;
      }
	
      JCCLog2("CC Call Log (JCC): Sent Message to (VC# = %d) with (ip address = %d)\n",  
               vcOutMsg.destVcId.cellId,
               vcOutMsg.destVcAddress);

      DBG_TRACE("CC Call Log (JCC): Sent Message to (VC# = %d) with (ip address = %d)\n",  
                 vcOutMsg.destVcId.cellId,
                 vcOutMsg.destVcAddress);
  }
  DBG_LEAVE();
}  


