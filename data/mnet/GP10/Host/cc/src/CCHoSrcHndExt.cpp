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
// File        : CCHoSrcHndExt.cpp
// Author(s)   : Joe Chen
// Create Date : 05-30-01
// Description : External Handover Event handlers for Anchor Session
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************


#include "CC/CCHandover.h"

#include "CC/CCSessionHandler.h"

#include "CC/CCUtil.h"

#include "CC/CallConfig.h"
#include "CC/CCInt.h"
#include "CC/CCH323Util.h"

#include "jcc/JCCLog.h"
#include "logging/VCLOGGING.h"

#include "stdio.h"

// Temporarily use csu_head instead of csu_intf
#include "csunew/csu_head.h"

#include "CC/hortp.h"

#include "lapdm/cni_debug.h"

#include "logging/VCLOGGING.h"

#include "strLib.h"

#include "string.h"

#include "pm/pm_class.h"

#include "voip/vblink.h"

#include "voip/exchangeho.h"
#include "CC/A_Handover_Request_Msg.h"
#include "CC/A_Handover_Request_Ack_Msg.h"
#include "jcc/LUDBApi.h"

#include "ril3/ril3md.h"

//CDR <xxu:08-22-00> BEGIN
#include "CDR/CdrVoiceCall.h"

#include "CC/A_Handover_Common.h"

extern bool forcedHandover;
extern bool useViperBase;
extern bool forcedHandmsc3;
extern GlobalCellId_t MY_GLOBALCELL_ID;
extern GlobalCellId_t ccHoTrgGlobalCellId;
extern UINT32 ccHoTrgCellIpAddress;
extern BtsBasicPackage ccBtsBasicPackage;

// Logging scripts
extern void ViperLogSetModuleFilters(int module_id,short severity_level);
extern void ccDataPrint();
extern void mmDataPrint();
extern void irtDataPrint();
extern void csu_ShowAllSrcTable();
extern void intg_ShowRmChans();
extern void VoipShowAllCalls();
extern void handoverRtpDataPrint();


// *******************************************************************
// Testing scripts
// *******************************************************************

void
hologs()
{
   ViperLogSetModuleFilters(  0, 0x1ff); // CC Layer
   ViperLogSetModuleFilters(  1, 0x1ff); // CC Session
   ViperLogSetModuleFilters(  2, 0x1ff); // CC CallLeg
   ViperLogSetModuleFilters(  4, 0x1ff); // CC HalfCall
   ViperLogSetModuleFilters(  5, 0x1ff); // CC Handover
   ViperLogSetModuleFilters(  7, 0x1ff); // CC EI
   ViperLogSetModuleFilters(  8, 0x1ff); // CC EO
   ViperLogSetModuleFilters(  9, 0x1ff); // CC HO
   ViperLogSetModuleFilters( 10, 0x1ff); // CC ETRG
   ViperLogSetModuleFilters( 75, 0x1ff); // H323
   ViperLogSetModuleFilters( 76, 0x1ff); // H323 Protocol
   ViperLogSetModuleFilters( 80, 0x1ff); // VBLINK
   ViperLogSetModuleFilters(225, 0x1ff); // RM Layer
}

void
hologsoff()
{
   ViperLogSetModuleFilters(  1000, 0); // Turn off logging 
}

void
showalltxt()
{
   ccDataPrint();
// mmDataPrint();
// irtDataPrint();
   csu_ShowAllSrcTable();
// intg_ShowRmChans();
// VoipShowAllCalls();
   handoverRtpDataPrint();
}


// *******************************************************************
// External Handover functions.
// *******************************************************************

   JCCEvent_t 
   CCAnchorHandover::ExtPerformHO(void)
   {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::ExtPerformHO): entering......\n}\n");

      TwoPartyCallLeg                     *whichLeg;
      NonStdRasMessagePerformHandoverRQ_t  Msg;
      T_AIF_MessageUnit                    a_msg;
      A_Result_t                           A_result;


      DBG_TRACEho("   Got an external handover candidate!\n");

      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
        {
           // abort handover
           DBG_TRACEho("   WARNING: No active leg (hoType %d, hoState %d)\n}\n", hoType_, hoState_);
           DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::ExtPerformHO): leaving.\n}\n");
           return (JCC_NULL_EVENT);
        }

      // Set aifMsg's HANDOVER REQUEST A INTERFACE values
      populateAHandoverRequest( &MY_GLOBALCELL_ID, &hoGlobalCellId_ );

      if ((A_result = AIF_Encode( &aifMsg, &a_msg )) != A_RESULT_SUCCESS)
        {
          // Couldn't encode Perform Handover A-Interface msg. 
          DBG_ERRORho ("   A Interface encoding Error: Perform Handver Request. Result (%d)\n",
                           (int) A_result);
          initData();
          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::ExtPerformHO): leaving.\n}\n");
          return (JCC_NULL_EVENT);
        }

      memset( (char*)&Msg, 0, sizeof(NonStdRasMessagePerformHandoverRQ_t));

      Msg.A_HANDOVER_REQUEST.msglength = a_msg.msgLength;
      memcpy(Msg.A_HANDOVER_REQUEST.A_INTERFACE_DATA, 
             (unsigned char*)a_msg.buffer, MAX_A_INTERFACE_LEN);

      Msg.header.origSubId          = callIndex;
      Msg.header.destSubId          = -1;              // NULL (unknown hoAgId - GMC will assign)
      Msg.globalCellID              = hoGlobalCellId_;
      Msg.hoCause                   = rrInMsg->l3_data.handReqd.hoCause;
      Msg.hoNumberReqd              = true;            // Always TRUE
      Msg.handoverNumber.ie_present = false;           // NULL (unknown handoverNumber - GMC will assign)
      Msg.mscNumber.ie_present      = false;           // NULL (unknown mscNumber - GMC will find out)

      DBG_TRACEho("   MESSAGE DATA:\n");
      DBG_TRACEho("       : hoAgId_=%d, callId=%d\n",hoAgId_, callIndex);
      DBG_TRACEho("       : hoGlobalCellId_ (type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
                            hoGlobalCellId_.ci_disc,hoGlobalCellId_.mcc[0],hoGlobalCellId_.mcc[1],hoGlobalCellId_.mcc[2],
                            hoGlobalCellId_.mnc[0],hoGlobalCellId_.mnc[1],hoGlobalCellId_.mnc[2],hoGlobalCellId_.lac,hoGlobalCellId_.ci);
      DBG_TRACEho("       : hoCause=%d\n",rrInMsg->l3_data.handReqd.hoCause);


      DBG_TRACEho("   a_msg.buffer POST-AIF PROCESSING MESSAGE DATA: A_HANDOVER_REQUEST_TYPE\n");
      DBG_TRACEho("      {\n");
      DBG_TRACEho("        \na_msg.buffer hexdump: ");
      DBG_HEXDUMPho((unsigned char*) a_msg.buffer, a_msg.msgLength);
      DBG_TRACEho("\n      }\n");


      DBG_TRACEho("   MSG POST-AIF PROCESSING MESSAGE DATA: A_HANDOVER_REQUEST_TYPE\n");
      DBG_TRACEho("      {\n");
      DBG_TRACEho("        \nMsg hexdump: ");
      DBG_HEXDUMPho((unsigned char*) &Msg, sizeof (NonStdRasMessagePerformHandoverRQ_t) );
      DBG_TRACEho("\n      }\n");


      if (VBLinkPerformHandoverRequest(&Msg)
          == false)
        {
          // Ignore the message 
          DBG_ERRORho ("    CC->VB Link ERROR: Failed on Perform Handver Request Message\n}\n");

          initData();
          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::ExtPerformHO): leaving.\n}\n");
          return (JCC_NULL_EVENT);
        }

      hoState_ = HAND_ST_ANCH_PERF_EHO_REQ;

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::ExtPerformHO): leaving.\n}\n");
      return (CC_MESSAGE_PROCESSING_COMPLT);
   }


   JCCEvent_t 
   CCAnchorHandover::handleExtPerformHOAck(void)
   {
      A_Result_t aif_result;

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): entering......\n}\n");

      // cancel timer
      parent->sessionTimer->cancelTimer();

      // save HO agent ID for later transactions
      switch( hoType_ )
        {
           case HO_TY_EXT_HANDOVER:
             {
                hoAgId_ = hoaInMsg->origSubId;
                break;
             }

           case HO_TY_EXT_HANDMSC3:
           case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
             {
                h3AgId_ = hoaInMsg->origSubId;
                break;
             }

           default:
             {
               //Deadly problem, need trouble shooting first before running ahead !
               DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                hoType_, hoState_);
               initData();
               DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
               return (CC_RELEASE_CALL);
             }
        };

      TwoPartyCallLeg *whichLeg;
    
      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
        {
          DBG_TRACEho("   No active leg (hoType %d, hoState %d)\n}\n", hoType_, hoState_);

          hoEndCause_ = MC_HO_SUCCESS;  // Normal takedown
          sendEndHandover();
          initData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
          return (CC_RELEASE_ALLHOCALLLEG);
        }

      //Decode A-interface message
      memset( (char*)&aifMsg, 0, sizeof(T_AIF_MSG));

      aif_result = AIF_Decode((T_AIF_MessageUnit*)&(hoaInMsg->msgData.perfExtHandoverAck.A_INTERFACE_MSG),
                               &aifMsg);
      if ( aif_result != A_RESULT_SUCCESS)
        {
           DBG_ERRORho("{\n   A-MSG decoding ERROR (err=%d, hoAgId_=%d)\nA-MSG:\n",
                              aif_result, hoAgId_);
           DBG_HEXDUMPho(
              (unsigned char*)&hoaInMsg->msgData.perfExtHandoverAck.A_INTERFACE_MSG.A_INTERFACE_DATA,
              (int)hoaInMsg->msgData.perfExtHandoverAck.A_INTERFACE_MSG.msglength
           );
           DBG_ERRORho("\n}\n");

          hoEndCause_ = MC_HO_UNEXPECTED_DATA_VALUE;
          sendEndHandover();
          initData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
          return (CC_RELEASE_ALLHOCALLLEG);
        }
  
      if (aifMsg.msgType != A_HANDOVER_REQUEST_ACK_TYPE)
        {
           //Unexpected A-MSG received
           DBG_ERRORho("   UNEXPECTED A-MSG (msgType=%d,hoState_=%d)\n",
                           aifMsg.msgType, hoState_);

           hoEndCause_ = MC_HO_UNEXPECTED_DATA_VALUE;
           sendEndHandover();
           initData();

           DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
           return (CC_RELEASE_ALLHOCALLLEG);
        }

      hoNumber_ = hoaInMsg->msgData.perfExtHandoverAck.handoverNumber;

      //Received PERFORM HANDOVER REQUEST ACK message
      DBG_TRACEho("   Received PERFORM_HANDOVER_ACK from HOA (hoType_=%d, hoAgId_=%d, callIndex=%d)\n",
                      hoType_, hoAgId_, callIndex);

      switch( hoType_ )
        {
           case HO_TY_EXT_HANDOVER:
             {
                // Ready HOCallLeg and send SETUP to VoIP (PLMN) to handover number
                if (!parent->setupOrigExtHo())
                  {
                    // SETUP to VoIP subsystem failed
                    DBG_ERRORho("   ERROR in SETUP to VoIP system failed. Handover number (ie_present=%d),(numberType=%d),(plan=%d),(numDigits=%d)\n}\n",
                                    hoNumber_.ie_present, hoNumber_.numberType, hoNumber_.numberingPlan, hoNumber_.numDigits);
                    DBG_ERRORho("                : handover number digits (%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x)\n}\n",
                                    hoNumber_.digits[0],hoNumber_.digits[1],hoNumber_.digits[2], hoNumber_.digits[3],hoNumber_.digits[4],
                                    hoNumber_.digits[5],hoNumber_.digits[6],hoNumber_.digits[7],hoNumber_.digits[8],hoNumber_.digits[9],hoNumber_.digits[10]);

                    hoEndCause_ = MC_HO_SUBHO_FAILURE;
                    sendEndHandover();
                    initData();

                    DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
                    return (CC_RELEASE_HOCALLLEG);
                  }

                // Send internal RR msg
               rrOutMsg.l3_data.handCmd.handCmd.msgLength = (unsigned short) aifMsg.handoverRequestAck.layer3Information.length;
               memcpy( (unsigned char*) rrOutMsg.l3_data.handCmd.handCmd.buffer,
                       (unsigned char*) aifMsg.handoverRequestAck.layer3Information.layer3Info,
                       aifMsg.handoverRequestAck.layer3Information.length );

               DBG_TRACEho("   Sending RR INTRA_L3_RR_HANDOVER_COMMAND.\n");
               DBG_TRACEho("      rrOutMsg.l3_data.handCmd.handCmd hexdump: ");
               DBG_TRACEho("\n        ");
               DBG_HEXDUMPho((unsigned char*) &rrOutMsg.l3_data.handCmd.handCmd, rrOutMsg.l3_data.handCmd.handCmd.msgLength + 2);

               sendRR(INTRA_L3_DATA, INTRA_L3_RR_HANDOVER_COMMAND, parent->entryId, &rrOutMsg);

               hoState_ = HAND_ST_ANCH_RCV_EHO_REQ_ACK;

               break;
             }

           case HO_TY_EXT_HANDMSC3:
             {
                // Ready H3CallLeg and send SETUP to VoIP (PLMN) to handover number
                if (!parent->setupOrigExtHo3())
                  {
                    // SETUP to VoIP subsystem failed
                    DBG_ERRORho("   ERROR in SETUP to VoIP system failed. Handover number (ie_present=%d),(numberType=%d),(plan=%d),(numDigits=%d)\n}\n",
                                    hoNumber_.ie_present, hoNumber_.numberType, hoNumber_.numberingPlan, hoNumber_.numDigits);
                    DBG_ERRORho("                : handover number digits (%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x)\n}\n",
                                    hoNumber_.digits[0],hoNumber_.digits[1],hoNumber_.digits[2], hoNumber_.digits[3],hoNumber_.digits[4],
                                    hoNumber_.digits[5],hoNumber_.digits[6],hoNumber_.digits[7],hoNumber_.digits[8],hoNumber_.digits[9],hoNumber_.digits[10]);

                    hoEndCause_ = MC_HO_SUBHO_FAILURE;
                    sendEndHandover();
                    initData();

                    DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
                    return (CC_RELEASE_ALLHOCALLLEG);
                  }

                // Send Handback ACK to the MSC which originated the Perform Handover.
                sendHandbackAck();

                hoState_ = HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK;

                break;
             }

           case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
             {
                // Ready H3CallLeg and send SETUP to VoIP (PLMN) to handover number
                if (!parent->setupOrigExtHo3())
                  {
                    // SETUP to VoIP subsystem failed
                    DBG_ERRORho("   ERROR in SETUP to VoIP system failed. Handover number (ie_present=%d),(numberType=%d),(plan=%d),(numDigits=%d)\n}\n",
                                    hoNumber_.ie_present, hoNumber_.numberType, hoNumber_.numberingPlan, hoNumber_.numDigits);
                    DBG_ERRORho("                : handover number digits (%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x)\n}\n",
                                    hoNumber_.digits[0],hoNumber_.digits[1],hoNumber_.digits[2], hoNumber_.digits[3],hoNumber_.digits[4],
                                    hoNumber_.digits[5],hoNumber_.digits[6],hoNumber_.digits[7],hoNumber_.digits[8],hoNumber_.digits[9],hoNumber_.digits[10]);

                    hoEndCause_ = MC_HO_SUBHO_FAILURE;
                    sendEndHandover();
                    initData();

                    DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
                    return (CC_RELEASE_H3CALLLEG);
                  }

                // Send Handback ACK to the MNET GP10 which originated the Perform Handover.
                vcOutMsg.msgType = INTER_VC_CC_PERFORM_HANDBACK_ACK;

                vcOutMsg.msgData.perfHandbackAck.handCmd.msgLength = (unsigned short) aifMsg.handoverRequestAck.layer3Information.length;
                memcpy((char*)vcOutMsg.msgData.perfHandbackAck.handCmd.buffer, 
                       (char*)aifMsg.handoverRequestAck.layer3Information.layer3Info,
                       (int)  aifMsg.handoverRequestAck.layer3Information.length);

                sendVcMsg(MODULE_CC, MODULE_CC);

                hoState_ = HAND_ST_ANCH_RCV_EHO_H3_REQ_ACK;

                break;
             }

           default:
             {
                //Deadly problem, need trouble shooting first before running ahead !
                DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                 hoType_, hoState_);
                DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
                return (CC_RELEASE_HOCALLLEG);
             }
        };

      // start the T103 timer 
      parent->sessionTimer->setTimer(CALL_HAND_SRC_T103);
    
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
      return (CC_MESSAGE_PROCESSING_COMPLT);
   }


   JCCEvent_t 
   CCAnchorHandover::handleExtPerformHONack(void)
   {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHONack): entering......\n}\n");

      DBG_TRACEho("   Received Perform Handover NACK instead of ACK (hoType=%d, reason=%d, hoAgId_=%d)\n}\n",
                      hoType_, hoaInMsg->msgData.perfExtHandoverAck.reason, hoAgId_);

      // cancel timer
      parent->sessionTimer->cancelTimer();

      switch( hoType_ )
        {
           case HO_TY_EXT_HANDOVER:
             {
                // set hoAgId_ to -1 since Handover NACK should have ended the HOA session.
                hoAgId_ = -1;
                initData();
                break;
             }

           case HO_TY_EXT_HANDMSC3:
             {
                // set h3AgId_ to -1 since Handover NACK should have ended the HOA session.
                h3AgId_ = -1;
                sendHandbackNack();
                initExtHandmsc3Data();

                DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHONack): leaving.\n}\n");
                return (CC_MESSAGE_PROCESSING_COMPLT);
             }

           case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
             {
                // set h3AgId_ to -1 since Handover NACK should have ended the HOA session.
                h3AgId_ = -1;
                initHandmsc3Data(); // Put hoState_ back to HAND_ST_ANCH_HO_COMPLT 
                return (handleAnchHoReqNackWiCos(JC_FAILURE_TARGETn_PERFORM_HANDOVER_REQ_NACK));
             }

           default:
             {
               //Deadly problem, need trouble shooting first before running ahead !
               DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                hoType_, hoState_);

               hoEndCause_ = MC_HO_SYSTEM_FAILURE;
               sendEndHandover();

               DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
               return (CC_RELEASE_CALL);
             }
        };

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHONack): leaving.\n}\n");
      return (CC_RELEASE_HOCALLLEG);
   }


    JCCEvent_t 
    CCAnchorHandover::handleTrgToAnchEndHandover(void)
    {
      JCCEvent_t  return_val = CC_MESSAGE_PROCESSING_COMPLT;
      JcSubId_t   fromHoAgId = hoaInMsg->origSubId;
      bool        late_endhandover = FALSE;

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleTrgToAnchEndHandover): entering......\n}\n");
    
      DBG_TRACEho("   Received END HANDOVER. Info:\n");
      DBG_TRACEho("     (hoType '%d', hoState '%d', origSubId '%d', destSubId '%d', reason '%d')\n", 
                         hoType_, hoState_,
                         hoaInMsg->origSubId,
                         hoaInMsg->destSubId,
                         hoaInMsg->msgData.extEndHandover.reason);

      // Check for late END HANDOVER messages
      // One time this happens is on completion of handover to thirds
      // the old HOA sends an END HANDOVER when old callleg is released.
      if ( (hoAgId_ != fromHoAgId) &&
           (h3AgId_ != fromHoAgId) )
        {
          // When PERFORM handover is being sent and PERFORM ACK has
          // not been received, then hoAgId_ or h3AgId_ has not been assigned
          // yet, so don't mark it as a late endhandover!
          if ( ( hoState_ != HAND_ST_ANCH_PERF_EHO_REQ ) &&
               ( hoState_ != HAND_ST_ANCH_PERF_EHO_H3_REQ ) &&
               ( hoState_ != HAND_ST_ANCH_EHO_SND_H3_REQ) )
            {
              late_endhandover = TRUE;
            }
        }

      if ( late_endhandover )
        {
           // This is a LATE END HANDOVER message.  Discard..
           DBG_TRACEho("   Late HOA END HANDOVER message received. (hoAgId_=%d, h3AgId_=%d, hoaInMsg->origSubId=%d)\n",
                           hoAgId_, h3AgId_, hoaInMsg->origSubId);
           return_val = CC_MESSAGE_PROCESSING_COMPLT;
        }
      else
        {
           switch (hoType_)
             {
               case HO_TY_EXT_HANDOVER:
                  {
                    parent->sessionTimer->cancelTimer();
                    // set hoAgId_ to -1 since END HANDOVER received should have ended the HOA session.
                    hoAgId_ = -1;
                    initData();
                    return_val = CC_RELEASE_HOCALLLEG;
                    break;
                  }

               case HO_TY_EXT_HANDBACK:
                  {
                    parent->sessionTimer->cancelTimer();
                    releaseMobAndConnRes();
                    // set hoAgId_ to -1 since END HANDOVER received should have ended the HOA session.
                    hoAgId_ = -1;
                    initData();
                    return_val = CC_RELEASE_HOCALLLEG;
                    break;
                  }

               case HO_TY_EXT_HANDMSC3:
               case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
               case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
                  {
                    if (hoAgId_ == fromHoAgId)
                      {
                         // Received END HANDOVER from 1st HO call leg.
                         hoAgId_ = -1;

                         // Send End Handover to THIRD MSC if required
                         if (h3AgId_ != -1)
                           {
                              sendEndHandover();
                           }
                         initData();
                         return_val = CC_RELEASE_HOCALLLEG;
                      }
                    else
                      {
                         // Received END HANDOVER from 2nd HO call leg.
                         h3AgId_ = -1;

                         if (hoType_ == HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3)
                           {
                              initHandmsc3Data(); // Reset values (internal handover)
                           }
                         else
                           {
                              initExtHandmsc3Data(); // Reset values (external handover)
                           }

                         return_val = CC_RELEASE_H3CALLLEG;
                      }
                    break;
                  }

               default:
                  {
                     //Deadly problem, need shoubleshooting first before running ahead !
                     DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                      hoType_, hoState_);
                     initData();
                     return_val = CC_RELEASE_ALLHOCALLLEG;
                  }
             }

            emptyRRQueue      ();
            emptyDownlinkQueue();
            emptyMMQueue      ();
        }

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleTrgToAnchEndHandover): leaving.\n}\n");

      return (return_val);
    }

    JCCEvent_t 
    CCAnchorHandover::handleAnchPostExtHoRls(void)
    {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoRls): entering......\n}\n");

      // Send End Handover to the Target and release call
      DBG_TRACEho("   CC HO INFO @anch-handleAnchPostExtHoRls: got RLS during HO (hoType %d) (hoState %d)\n",
                      hoType_, hoState_);
    
      // cancel any timers running
      parent->sessionTimer->cancelTimer();
    
      switch(hoType_)
        {
          case HO_TY_EXT_HANDOVER:
          case HO_TY_EXT_HANDBACK:
            {
              hoEndCause_ = MC_HO_SUCCESS;
              sendEndHandover();

              // JOE: need to map right cause value
              parent->hoCallLeg->cleanup(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);

              DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoRls): leaving.\n}\n");
              return (CC_RELEASE_CALL);

              break;
            }

          case HO_TY_EXT_HANDMSC3:
            {
              hoEndCause_ = MC_HO_SUCCESS;
              sendEndHandover();

              // JOE: need to map right cause value
              parent->hoCallLeg->cleanup(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);
              parent->h3CallLeg->cleanup(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);

              DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoRls): leaving.\n}\n");
              return (CC_RELEASE_CALL);

              break;
            }


          case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
            {
              vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDOVER;
              vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_REMOTE_END_HANDOVER;

              sendVcMsg(MODULE_CC, MODULE_CC);

              hoEndCause_ = MC_HO_SUCCESS;
              sendEndHandover();

              parent->hoCallLeg->cleanup(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);

              DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoRls): leaving.\n}\n");
              return (CC_RELEASE_CALL);

              break;
            }

          case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
            {
              vcOutMsg.msgType = INTER_VC_CC_ABORT_HANDBACK;
              vcOutMsg.msgData.abortHandover.cause = JC_FAILURE_REMOTE_END_HANDOVER;

              sendVcMsg(MODULE_CC, MODULE_CC);

              hoEndCause_ = MC_HO_SUCCESS;
              sendEndHandover();

              parent->h3CallLeg->cleanup(CNI_RIL3_CAUSE_NORMAL_CALL_CLEARING);

              DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoRls): leaving.\n}\n");
              return (CC_RELEASE_CALL);

              break;
            }

         default:
            {
              DBG_ERRORho("   CC Error @anch-handleAnchPostExtHoRls: messed up (hoType: %d) (hoState: %d)\n", 
                              hoType_, hoState_);
              break;
            }
        }

     DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoRls): leaving.\n}\n");
     return (CC_MESSAGE_PROCESSING_COMPLT);
   }


    JCCEvent_t 
    CCAnchorHandover::cleanExtHoCallWiCause(InterEHOMsgType_t msg, JcFailureCause_t cause)
    {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::cleanExtHoCallWiCause): entering......\n}\n");

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::cleanExtHoCallWiCause): leaving.\n}\n");
      return (CC_MESSAGE_PROCESSING_COMPLT);
    
    }

    void
    CCAnchorHandover::sendL2MsgToTargetPLMN(T_CNI_LAPDM_L3MessageUnit *msEncodedMsg)
    {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendL2MsgToTargetPLMN): entering......\n}\n");

      //Relay the MS message received to HOA
      NonStdRasMessagePostHandoverMobEvent_t relaymsg;

      memset( (char*)&relaymsg, 0, sizeof(NonStdRasMessagePostHandoverMobEvent_t));

      relaymsg.header.origSubId = callIndex;
      relaymsg.header.destSubId = hoAgId_;

      relaymsg.LAYER3PDU.LAYER3DATA[0] = 0x01;  //DTAP 1st byte of Distribution Data Unit (Discrimination)
      relaymsg.LAYER3PDU.LAYER3DATA[1] = 0x80;  //DTAP 2nd byte of Distribution Data Unit (FACCH or SDCCH, SAPI0)

      relaymsg.LAYER3PDU.LAYER3DATA[2] = (unsigned char) msEncodedMsg->msgLength; //length of L3 Msg followed

      relaymsg.LAYER3PDU.msglength= (unsigned short) msEncodedMsg->msgLength + 3;

      memcpy((char*) &(relaymsg.LAYER3PDU.LAYER3DATA[3]),
             (char*)   msEncodedMsg->buffer,  //body of L3 Msg
             (int)     relaymsg.LAYER3PDU.msglength);

      DBG_TRACEho("   POST MOB EVENT [==>HOA MS MESSAGE] (hoAgId=%d,callId=%d, msEncodedMsg->msgLength=%d)\n",   
                      hoAgId_, callIndex, msEncodedMsg->msgLength);
      DBG_TRACEho("     {");
      DBG_HEXDUMPho((unsigned char*) &(relaymsg.LAYER3PDU.LAYER3DATA[0]), relaymsg.LAYER3PDU.msglength + 20 );
      DBG_TRACEho("}\n");

      if (!VBLinkPostHandoverMobEvent(&relaymsg))
        {
          DBG_ERRORho("{\nMNEThoERROR(CCAnchorHandover::sendRRMsgToTargetPLMN): VBLinkPostHandoverMobEvent API failed !\n}\n");
          DBG_HEXDUMPho((unsigned char*) &relaymsg, relaymsg.LAYER3PDU.msglength + 3);
          DBG_TRACEho("}\n");

          hoEndCause_ = MC_HO_SYSTEM_FAILURE;  //EHOcause

          return;
        }

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendL2MsgToTargetCC): leaving.\n}\n");
    }

    void
    CCAnchorHandover::sendRRMsgToTargetPLMN(IntraL3Msg_t      *rrMsg)
    {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendRRMsgToTargetPLMN): entering......\n}\n");

      DBG_TRACEho("   MNET does NOT forward RR Msgs to target PLMN! rrMsg message_type (%d)\n",
                      (int) rrMsg->message_type );

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendRRMsgToTargetPLMN): leaving.\n}\n");
    }

    void 
    CCAnchorHandover::sendMMMsgToRemotePLMN (IntraL3Msg_t *mmMsg)
    { 
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendMMMsgToRemotePLMN): entering......\n}\n");

      DBG_TRACEho("   MNET does NOT forward MM Msgs to target PLMN! mmMsg message_type (%d)\n",
                      (int) mmMsg->message_type );

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendMMMsgToRemotePLMN): leaving.\n}\n");
    }


    JCCEvent_t 
    CCAnchorHandover::handleAnchExtHoTimerExpiry(void)
    {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoTimerExpiry): entering......\n}\n");

      DBG_ERRORho("   CC Timeout: External Handover expiry (hoSt %d, hoTyp %d)\n",
                      hoState_, hoType_);

      switch (hoType_)
        {
          case HO_TY_EXT_HANDOVER:
             {
                hoEndCause_ = MC_HO_T204_TIMEOUT;
                sendEndHandover();
                initData();

                DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoTimerExpiry): leaving.\n}\n");
                return ( CC_RELEASE_HOCALLLEG );
             }

          case HO_TY_EXT_HANDBACK:
             {
                releaseMobAndConnRes();
                hoEndCause_ = MC_HO_T204_TIMEOUT;
                sendHandbackNack();
                initExtHandbackData();

                DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoTimerExpiry): leaving.\n}\n");
                return ( CC_MESSAGE_PROCESSING_COMPLT );
             }

          case HO_TY_EXT_HANDMSC3:
            {
                hoEndCause_ = MC_HO_T204_TIMEOUT;
                sendHandbackNack();
                initExtHandmsc3Data();

                DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoTimerExpiry): leaving.\n}\n");
                return ( CC_MESSAGE_PROCESSING_COMPLT );
            }

          case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
             {
                hoEndCause_ = MC_HO_T204_TIMEOUT;
                sendEndHandover();
                initHandmsc3Data();  // Set state back to HAND_ST_ANCH_HO_COMPLT

                DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoTimerExpiry): leaving.\n}\n");
                return ( CC_RELEASE_H3CALLLEG );
             }

          default:
            {
               //Deadly problem, release call and troubleshoot asap!
               DBG_ERRORho("    CC Ho Error: messed up at (hoSt %d, hoTyp %d, rc %d)\n",
                                hoState_, hoType_, hoRetry_);
               initData();
               DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoTimerExpiry): leaving.\n}\n");
               return(CC_RELEASE_ALLHOCALLLEG);
            }
         }

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoTimerExpiry): leaving.\n}\n");
      return (CC_RELEASE_HOCALLLEG);
    }


    JCCEvent_t 
    CCAnchorHandover::handleAnchPostExtHoMobEvent(void)
    {
      TwoPartyCallLeg  *whichLeg;
      short             discrimination_val;
    
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoMobEvent): entering......\n}\n");

      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
        {
           // abort handover
          DBG_TRACEho("   No active leg (hoType %d, hoState %d)\n", hoType_, hoState_);

          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendEndHandover();
          initData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoMobEvent): leaving.\n}\n");
          return (CC_RELEASE_HOCALLLEG);
        }

      discrimination_val = hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA[0] & 0x1;

      switch (discrimination_val)
        {
           case 0:  // For BSSMAP msgs
             {
                DBG_TRACEho("   Received BSSMAP msg.  Doing nothing.\n");
                break;
             }

           case 1:  // For DTAP msgs
             {
                T_CNI_RIL3MD_CCMM_MSG msMsg;

                msMsg.module_id = MODULE_MD;
                msMsg.entry_id  = parent->entryId;
                msMsg.lapdm_oid = parent->oid;
//              msMsg.l3_id
//              msMsg.sapi
                msMsg.primitive_type = L23PT_DL_DATA_IND;

                msMsg.l3_data.msgLength = hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA[2];
                if (msMsg.l3_data.msgLength <= CNI_LAPDM_MAX_L3MSG_LENGTH)
                  {
                    memcpy( msMsg.l3_data.buffer,
                            &(hoaInMsg->msgData.postExtHoMobEventData.LAYER3PDU.LAYER3DATA[3]),
                            msMsg.l3_data.msgLength );

                    DBG_TRACEho("   Handling the following Mobile Event.\n");
                    DBG_TRACEho("      - module_id (%d) \n", msMsg.module_id);
                    DBG_TRACEho("      - entry_id (%d) \n", msMsg.entry_id);
                    DBG_TRACEho("      - lapdm_oid (%d) \n", msMsg.lapdm_oid);
                    DBG_TRACEho("      - primitive_type (%d) \n", msMsg.primitive_type);
                    DBG_TRACEho("      - l3_id: unknown\n");
                    DBG_TRACEho("      - sapi: unknown\n");

                    DBG_TRACEho("      - l3_data HEXDUMP:\n" );
                    DBG_HEXDUMPho((unsigned char*) msMsg.l3_data.buffer, msMsg.l3_data.msgLength );
                    DBG_TRACEho("\n");

                    parent->handleMobMsg(&msMsg);
                  }
                else
                  {
                     DBG_ERRORho("   Layer3 PDU TOO LONG (msgLength %d)\n", msMsg.l3_data.msgLength);
                  }

                break;
             }

           default:
             {
                DBG_ERRORho("   UNKNOWN discrimation value (discrimination_val %d)\n", discrimination_val);
             }
        }

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoMobEvent): leaving.\n}\n");
      return(CC_MESSAGE_PROCESSING_COMPLT);
    
    }


    JCCEvent_t 
    CCAnchorHandover::handleAnchExtHoReqAck(void)
    {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoReqAck): entering......\n}\n");

      // cancel timer
      parent->sessionTimer->cancelTimer();

      TwoPartyCallLeg *whichLeg;
    
      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
        {
           DBG_TRACEho("   No active leg (hoType %d, hoState %d)\n}\n", hoType_, hoState_);

           releaseMobAndConnRes();
           hoEndCause_ = MC_HO_SUBHO_FAILURE;
           sendEndHandover();
           initData();

           DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoReqAck): leaving.\n}\n");
           return (CC_RELEASE_HOCALLLEG);
        }
    
      T_CNI_L3_ID newId;
      newId.msgq_id =  msgQId;
      newId.sub_id = 0;  // No sub id needed here

      if (parent->setIrt (mmInMsg->entry_id, newId) == false)
      {
          DBG_ERRORho("   Error: Not able to setIrt\n");

          releaseMobAndConnRes();
          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendHandbackNack();
          initExtHandbackData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoReqAck): leaving.\n}\n");
          return   (CC_MESSAGE_PROCESSING_COMPLT);
      }
    
      // Setup the entry id, oid, and sapi
      parent->entryId = mmInMsg->entry_id;
    
      // GET OID AND SAPI
      T_CNI_RIL3_IRT_ENTRY	irtEntry;
      irtEntry.entry_id = mmInMsg->entry_id;
      
      if  ( (CNI_RIL3_IRT_Get_Entry_by_Entry_ID(&irtEntry))
          == false )
        {
          DBG_ERRORho("   CC Call Error : IRT Table problem for (entry id = %d)\n ",
                          mmInMsg->entry_id);

          releaseMobAndConnRes();
          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendHandbackNack();
          initExtHandbackData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoReqAck): leaving.\n}\n");
          return (CC_MESSAGE_PROCESSING_COMPLT);
        }
    
      parent->oid = irtEntry.lapdm_oid;
    
      // Send the Perform Ack message.

      sendHandbackAck();

      // start the T104 timer 
      parent->sessionTimer->setTimer(CALL_HAND_TRG_T104);

      DBG_TRACEho("   [<==CC START TIMER T104] (hoAgId=%d, callId=%d, T104=%d)\n",
                      hoAgId_, callIndex, (int)CALL_HAND_TRG_T104);

      hoState_ = HAND_ST_ANCH_SND_EHB_REQ_ACK;
    
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoReqAck): leaving.\n}\n");
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }
    


   JCCEvent_t 
   CCAnchorHandover::sendHandbackAck(void)
   {
     SpeechChann_t currSpeechCh;
     int choosenEncryption;
     T_AIF_MSG     temp_aifMsg = aifMsg;  // copy for ext handover to third MSC purpose
     //Populate data for encoding A-HANDOVER-ACKNOWLEDGE message
     A_Handover_Request_Ack_t  *pRequestAck = (A_Handover_Request_Ack_t*) &aifMsg.handoverRequestAck;

     //Mandatory IEs
     pRequestAck->msgType = A_HANDOVER_REQUEST_ACK_TYPE;
     pRequestAck->layer3Information.ie_present = true;
     pRequestAck->layer3Information.elementIdentifier = A_LAYER3_INFORMATION_TYPE;

     switch( hoType_ )
       {
          case HO_TY_EXT_HANDBACK:
            {
               pRequestAck->layer3Information.length = (unsigned char) mmInMsg->l3_data.handReqAck.handCmd.msgLength;
               memcpy((char*)pRequestAck->layer3Information.layer3Info, 
                      (char*)mmInMsg->l3_data.handReqAck.handCmd.buffer,
                      (int)  mmInMsg->l3_data.handReqAck.handCmd.msgLength);
               break;
            }

          case HO_TY_EXT_HANDMSC3:
            {
               pRequestAck->layer3Information.length = (unsigned char) temp_aifMsg.handoverRequestAck.layer3Information.length;
               memcpy((char*)pRequestAck->layer3Information.layer3Info, 
                      (char*)temp_aifMsg.handoverRequestAck.layer3Information.layer3Info,
                      (int)  temp_aifMsg.handoverRequestAck.layer3Information.length);
               break;
            }

          case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
            {
               pRequestAck->layer3Information.length = (unsigned char) vcInMsg->msgData.perfHandoverAck.handCmd.msgLength;
               memcpy((char*)pRequestAck->layer3Information.layer3Info, 
                      (char*)vcInMsg->msgData.perfHandoverAck.handCmd.buffer,
                      (int)  vcInMsg->msgData.perfHandoverAck.handCmd.msgLength);
               break;
            }

          default:
            {
              //Deadly problem, need trouble shooting first before running ahead !
              DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                               hoType_, hoState_);
              initData();
              DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
              return (CC_RELEASE_HOCALLLEG);
            }
       };

     //Optional IEs
     pRequestAck->chosenChannel.ie_present = true;
     pRequestAck->chosenChannel.elementIdentifier = A_CHOSEN_CHANNEL_TYPE;
     pRequestAck->chosenChannel.channelMode = 9;  //speech (full/half)
     pRequestAck->chosenChannel.channel     = 4;  //Eight TCH full

     pRequestAck->speechVersionChosen.ie_present = true;
     pRequestAck->speechVersionChosen.elementIdentifier= A_SPEECH_VERSION_TYPE;
     currSpeechCh = parent->currSpeechCh();
     if (currSpeechCh.version == CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_1)  
         pRequestAck->speechVersionChosen.speechVersionId = 0x01;
     else if (currSpeechCh.version == CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_2) 
         pRequestAck->speechVersionChosen.speechVersionId = 0x21;
     else if (currSpeechCh.version == CNI_RIL3_GSM_HALF_RATE_SPEECH_VERSION_1) 
         pRequestAck->speechVersionChosen.speechVersionId = 0x05;
     else
       {
          DBG_ERRORho("   CC Internal error (speechVer=%d)\n",
                          currSpeechCh_.version);

          releaseMobAndConnRes();
          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendHandbackNack();
          initExtHandbackData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendHandbackAck): leaving.\n}\n");
          return (CC_MESSAGE_PROCESSING_COMPLT);
       }

     choosenEncryption = selectEncryption();

     pRequestAck->chosenEncryptAlgorithm.ie_present = true;
     pRequestAck->chosenEncryptAlgorithm.elementIdentifier = A_CHOSEN_ENCRYPTION_ALGORITHM_TYPE;
     if (choosenEncryption == -1)
       {
          pRequestAck->chosenEncryptAlgorithm.algorithmId = 0x01; //no encryption used
       } else if (choosenEncryption == CNI_RIL3_CIPHER_ALGORITHM_A51)
       {
          pRequestAck->chosenEncryptAlgorithm.algorithmId = 0x02; //A51 used
       } else if (choosenEncryption == CNI_RIL3_CIPHER_ALGORITHM_A52)
       {
          pRequestAck->chosenEncryptAlgorithm.algorithmId = 0x03; //A52 used
       } else
       {
          DBG_ERRORho("   CC Internal error (choosenEncryption=%d)\n",
                          choosenEncryption);

          releaseMobAndConnRes();
          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendHandbackNack();
          initExtHandbackData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendHandbackAck): leaving.\n}\n");
          return (CC_MESSAGE_PROCESSING_COMPLT);	  
       }

     //Encode A-MSG: HANDOVER REQUEST ACKNOWLEDGE
     A_Result_t result;
     NonStdRasMessagePerformHandbackAck_t requestAck;

     memset( (char*) &requestAck, 0,  sizeof(NonStdRasMessagePerformHandbackAck_t) );
     if ( A_RESULT_SUCCESS != (result=
                               AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&requestAck.A_INTERFACE_MSG) ) )
       {
          DBG_ERRORho("   A-HANDOVER-REQUEST-ACKNOWLEDGE encoding failed (result=%d)\n",
                          result);

          releaseMobAndConnRes();
          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendHandbackNack();
          initExtHandbackData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendHandbackAck): leaving.\n}\n");
          return (CC_MESSAGE_PROCESSING_COMPLT);	  
       }

      //Send PERFORM-HANDOVER-ACK (positive) to HOA
      //requestAck.reason = 0;
      requestAck.header.origSubId = callIndex;
      requestAck.header.destSubId = hoAgId_;
      requestAck.reason           = MC_HO_SUCCESS;

      if (!VBLinkPerformHandbackAck(&requestAck))
        {
           DBG_ERRORho("   VBLinkPerformHandbackAck API failed !\n",
                           result);
           DBG_HEXDUMPho( (unsigned char*) &requestAck,
                          (int) (requestAck.A_INTERFACE_MSG.msglength+6));
           DBG_ERRORho("}\n");

           releaseMobAndConnRes();
           hoEndCause_ = MC_HO_SUBHO_FAILURE;
           sendHandbackNack();
           initExtHandbackData();

           DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendHandbackAck): leaving.\n}\n");
           return (CC_MESSAGE_PROCESSING_COMPLT);
        }

      DBG_TRACEho("   [==>HOA PERFORM_HANDBACK_ACK(p)] (hoAgId=%d,callIndex=%d,entryId=%d)\n}\n",
                      hoAgId_, callIndex, mmInMsg->entry_id);
   }

   JCCEvent_t
   CCAnchorHandover::sendHandbackNack(void)
   {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendHandbackNack): entering......\n}\n");

      //Send HANDBACK_NACK (which is really a HANDBACK ACK with a reason code other than SUCCESS)
      NonStdRasMessagePerformHandbackAck_t  handbackAck;

      memset( (char*)&handbackAck, 0, sizeof(NonStdRasMessagePerformHandbackAck_t));
      // Note: handbackAck's A_INTERFACE_MSG will be all zeros for a NACK.
      handbackAck.header.origSubId = callIndex;
      handbackAck.header.destSubId = hoAgId_;
      handbackAck.reason           = hoEndCause_;

      if (!VBLinkPerformHandbackAck(&handbackAck))
        {
          DBG_ERRORho("   VBLinkPerformHandbackAck API FAILED\n");
        } else
        {
          DBG_TRACEho("   Sent to HOA HANDBACK NACK (hoAgId=%d,callId=%d,entryId=%d,reason=%d)\n",
                          hoAgId_, callIndex, parent->entryId, hoEndCause_);
        }

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::sendHandbackNack): leaving.\n}\n");

      return (CC_MESSAGE_PROCESSING_COMPLT);
   }


    JCCEvent_t 
    CCAnchorHandover::handleAnchExtHOT104TimerExpiry(void)
    {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHOT104TimerExpiry): entering......\n}\n");
    
      releaseMobAndConnRes();
      hoEndCause_ = MC_HO_SUBHO_FAILURE;
      sendEndHandover();
      initData();

      // JOE: check 03.09 spec as to whether should release entire call (take it all down) or send END HANDOVER

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHOT104TimerExpiry): leaving.\n}\n");
      return (CC_RELEASE_HOCALLLEG);
    }


    JCCEvent_t 
    CCAnchorHandover::handleAnchExtHoComplt(void)
    {
      TwoPartyCallLeg *whichLeg;
      bool             sig_and_voice_complete = false;

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoComplt): entering......\n}\n");

      switch( hoType_ )
        {
           case HO_TY_EXT_HANDOVER:
             {
                // cancel T103 timer only if VoIP CONNECT was received.
                if (parent->hoCallLeg->voiceConnected)
                  {
                    parent->sessionTimer->cancelTimer();

                    DBG_TRACEho("   [==>HOA HANDOVER-COMPLETE both signaling and voice] (hoType=%d, hoAgId=%d,callId=%d,entryId=%d)\n",
                                    hoType_, hoAgId_, callIndex, parent->entryId);

                    sig_and_voice_complete = true;
                  }
                else
                  {
                    DBG_TRACEho("   [==>HOA HANDOVER-COMPLETE signaling only] (hoType=%d, hoAgId=%d,callId=%d,entryId=%d)\n",
                                    hoType_, hoAgId_, callIndex, parent->entryId);

                    sig_and_voice_complete = false;
                  }

                break;
             }

           case HO_TY_EXT_HANDBACK:
           case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
             {
                // cancel T103 timer always
                parent->sessionTimer->cancelTimer();

                DBG_TRACEho("   [==>HOA HANDBACK-COMPLETE] (hoAgId=%d,callId=%d,entryId=%d)\n",
                                hoAgId_, callIndex, parent->entryId);

                sig_and_voice_complete = true;
                break;
             }

           case HO_TY_EXT_HANDMSC3:
           case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
             {
                // cancel T103 timer only if VoIP CONNECT was received.
                if (parent->h3CallLeg->voiceConnected)
                  {
                    parent->sessionTimer->cancelTimer();

                    DBG_TRACEho("   [==>HOA HANDOVER-COMPLETE both signaling and voice] (hoType=%d, hoAgId=%d, h3AgId=%d, callId=%d,entryId=%d)\n",
                                    hoType_, hoAgId_, h3AgId_, callIndex, parent->entryId);

                    sig_and_voice_complete = true;
                  }
                else
                  {
                    DBG_TRACEho("   [==>HOA HANDOVER-COMPLETE signaling only] (hoType=%d, hoAgId=%d, h3AgId=%d, callId=%d, entryId=%d)\n",
                                    hoType_, hoAgId_, h3AgId_, callIndex, parent->entryId);

                    sig_and_voice_complete = false;
                  }

                break;
             }

           default:
             {
               //Deadly problem, need trouble shooting first before running ahead !
               DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                hoType_, hoState_);
               initData();
               DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
               return (CC_RELEASE_ALLHOCALLLEG);
             }
        }

      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
        {
          DBG_TRACEho("   No active leg (hoType %d, hoState %d)\n}\n", hoType_, hoState_);

          releaseMobAndConnRes();
          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendEndHandover();
          initData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
          return (CC_RELEASE_ALLHOCALLLEG);
        }

      //Do CDR
      T_CNI_RIL3_IE_LOCATION_AREA_ID lai;
      T_CNI_RIL3_IE_CELL_ID ci;

      switch( hoType_ )
        {
           case HO_TY_EXT_HANDOVER:
             {
                ci.ie_present = true;
                ci.value = ((short)(hoGlobalCellId_.ci));
                lai.ie_present = true;
                lai.mcc[0] = ((unsigned char*)&(hoGlobalCellId_.mcc))[0];
                lai.mcc[1] = ((unsigned char*)&(hoGlobalCellId_.mcc))[1];
                lai.mcc[2] = ((unsigned char*)&(hoGlobalCellId_.mcc))[2];
                lai.mnc[0] = ((unsigned char*)&(hoGlobalCellId_.mnc))[0];
                lai.mnc[1] = ((unsigned char*)&(hoGlobalCellId_.mnc))[1];
                lai.mnc[2] = ((unsigned char*)&(hoGlobalCellId_.mnc))[2];
                lai.lac    = ((short)(hoGlobalCellId_.lac));
                break;
             }

           case HO_TY_EXT_HANDBACK:
             {
                ci.ie_present = true;
                ci.value = ((short)(ccBtsBasicPackage.bts_ci));
                lai.ie_present = true;
                lai.mcc[0] = ((unsigned char*)&(ccBtsBasicPackage.bts_mcc))[0];
                lai.mcc[1] = ((unsigned char*)&(ccBtsBasicPackage.bts_mcc))[1];
                lai.mcc[2] = ((unsigned char*)&(ccBtsBasicPackage.bts_mcc))[2];
                lai.mnc[0] = ((unsigned char*)&(ccBtsBasicPackage.bts_mnc))[0];
                lai.mnc[1] = ((unsigned char*)&(ccBtsBasicPackage.bts_mnc))[1];
                lai.mnc[2] = ((unsigned char*)&(ccBtsBasicPackage.bts_mnc))[2];
                lai.lac    = ((short)(ccBtsBasicPackage.bts_lac));
                break;
             }

           case HO_TY_EXT_HANDMSC3:
           case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
           case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
             {
                ci.ie_present = true;
                ci.value = ((short)(h3GlobalCellId_.ci));
                lai.ie_present = true;
                lai.mcc[0] = ((unsigned char*)&(h3GlobalCellId_.mcc))[0];
                lai.mcc[1] = ((unsigned char*)&(h3GlobalCellId_.mcc))[1];
                lai.mcc[2] = ((unsigned char*)&(h3GlobalCellId_.mcc))[2];
                lai.mnc[0] = ((unsigned char*)&(h3GlobalCellId_.mnc))[0];
                lai.mnc[1] = ((unsigned char*)&(h3GlobalCellId_.mnc))[1];
                lai.mnc[2] = ((unsigned char*)&(h3GlobalCellId_.mnc))[2];
                lai.lac    = ((short)(h3GlobalCellId_.lac));
                break;
             }

           default:
             {
               //Deadly problem, need trouble shooting first before running ahead !
               DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                hoType_, hoState_);
               initData();
               DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
               return (CC_RELEASE_ALLHOCALLLEG);
             }
        };

      if ( (parent->callLeg1->msSide != NULL) &&
           (parent->callLeg1->msSide->callState_ == HC_ST_ACTIVE) )
            parent->callLeg1->ccCdr.handover(&lai, &ci);
      if ( (parent->callLeg2->msSide != NULL) &&
           (parent->callLeg2->msSide->callState_ == HC_ST_ACTIVE) )
            parent->callLeg2->ccCdr.handover(&lai, &ci);
      //CDR END

      // Relay HANDOVER SUCCESS
      switch( hoType_ )
        {
           case HO_TY_EXT_HANDOVER:
             {
                // Release rf and RTP connections
                hoSimplexConnected_ = true;  // JOE: so GSM RTP connections will be 
                                             //      released in releaseMobAndConnRes()
                releaseMobAndConnRes();

                hoSimplexConnected_ = false;
                hoSrcLinkLost_      = false;

                break;
             }

           case HO_TY_EXT_HANDBACK:
           case HO_TY_EXT_HANDMSC3:
           case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
             {
                // Send HANDOVER SUCCESS to PLMN
                // Populate data for encoding A-HANDOVER-COMPLETE message
                memset(&aifMsg, 0, sizeof(A_Handover_Complete_t));

                A_Handover_Complete_t  *pComplete = (A_Handover_Complete_t*) &aifMsg.handoverComplete;

                //Mandatory IEs
                pComplete->msgType = A_HANDOVER_COMPLETE_TYPE;

                //Optional IEs
                pComplete->rrCause.ie_present = true;
                pComplete->rrCause.elementIdentifier = A_RR_CAUSE_TYPE;
                pComplete->rrCause.rrCause = 0;  //normal event

                //Encode A-MSG: HANDOVER REQUEST ACKNOWLEDGE
                A_Result_t result;
                NonStdRasMessageHandoverSuccess_t hoSuccess;

                memset( (char*) &hoSuccess, 0,  sizeof(NonStdRasMessageHandoverSuccess_t) );
                if ( A_RESULT_SUCCESS != (result=
                                          AIF_Encode(&aifMsg, (T_AIF_MessageUnit*)&hoSuccess.A_INTERFACE_MSG) ) )
                  {
                     DBG_ERRORho("   A-HANDOVER-COMPLETE encoding FAILED (result=%d)\n",
                                     result);

                     // Really caught in a bad place.  Mobile has already handed back and
                     // an internal error occured.  So, just continue on and hope for the best!
                     DBG_TRACEho("   Continue processing HANDOVER SUCCESS anyways.\n");
                  }

                // Send HANDOVER-SUCCESS to HOA
                hoSuccess.header.origSubId = callIndex;
                hoSuccess.header.destSubId = hoAgId_;

                if (!VBLinkHandoverSuccess(&hoSuccess))
                  {
                    DBG_ERRORho("   VBLinkHandoverSuccess API failed!\n");
                    DBG_HEXDUMPho( (unsigned char*) &hoSuccess,
                                   (int) (hoSuccess.A_INTERFACE_MSG.msglength+4));
                    DBG_ERRORho("}\n");

                   // Really caught in a bad place.  Mobile has already handed back and
                   // an internal error occured.  So, just continue on and hope for the best!
                    DBG_TRACEho("   Continue processing HANDOVER SUCCESS anyways.\n");
                  }

                break;
             }

           case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
             {
                // Non-anchor MNET GP10 to external PLMN handback
                // Send HANDOVER SUCCESS (Complete Handback) to MNET GP10

                vcOutMsg.msgType = INTER_VC_CC_COMPLETE_HANDBACK;
                sendVcMsg(MODULE_CC, MODULE_CC);

                break;
             }

           default:
             {
               //Deadly problem, need trouble shooting first before running ahead !
               DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                hoType_, hoState_);
               initData();
               DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
               return (CC_RELEASE_ALLHOCALLLEG);
             }
        };


      if (whichLeg->voiceConnected)
        {
          // Call is not held at this point for some reason.

          // First take down the earlier connection
          
          // Break the connections with the RTP Port
          T_CSU_PORT_ID hoRtpSourcePort;
          T_CSU_RESULT_CON csuResult;

          switch( hoType_ )
            {
               case HO_TY_EXT_HANDOVER:
                 {
                    // do nothing (no connections to break)
                    hoRtpSourcePort.portType = CSU_NIL_PORT;
                    break;
                 }

               case HO_TY_EXT_HANDBACK:
               case HO_TY_EXT_HANDMSC3:
               case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
                 {
                    hoRtpSourcePort.portType = CSU_RTP_PORT;
                    hoRtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(parent->hoCallLeg->h323CallHandle);
                    break;
                 }

              case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
                 {
                    hoRtpSourcePort.portType = CSU_RTP_PORT;
                    hoRtpSourcePort.portId.rtpHandler = hoRtpSession_;
                    break;
                 }

              default:
                {
                   //Deadly problem, need trouble shooting first before running ahead !
                   DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                    hoType_, hoState_);
                   initData();
                   DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
                   return (CC_RELEASE_ALLHOCALLLEG);
                 }
            };

          if (hoRtpSourcePort.portType != CSU_NIL_PORT)
            {
              if ((csuResult = csu_OrigTermBreakAll(&hoRtpSourcePort))
                 != CSU_RESULT_CON_SUCCESS)
                {
                  DBG_ERRORho("   CSU Error : Ho Disconn. Failure, (Result = %d) for (rtp port = %x)\n",  
                                  csuResult,
                                  (int)hoRtpSourcePort.portId.rtpHandler);

                  // Release the Ho RTP channel
                  ReleaseHandoverRtpChannel(hoRtpSourcePort.portId.rtpHandler);
                  parent->hoCallLeg->h323CallHandle = VOIP_NULL_CALL_HANDLE;

                  // Really caught in a bad place.  Mobile has already handed back and
                  // an internal error occured.  So, just continue on and hope for the best!
                  DBG_TRACEho("   Continue processing HANDOVER SUCCESS anyways.\n");
                }

              // voice is broken
              whichLeg->voiceConnected = false;
            }

          // Complete the connection
          // Make a duplex connection between the remote rtp channel and the rf channel
          T_CSU_PORT_ID rfSinkPort, rtpSourcePort;

          switch( hoType_ )
            {
               case HO_TY_EXT_HANDOVER:
                 {
                    rfSinkPort.portType = CSU_RTP_PORT;
                    rtpSourcePort.portType = CSU_RTP_PORT;

                    rfSinkPort.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->h323CallHandle);
                    rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(parent->hoCallLeg->h323CallHandle);
                    break;
                 }

               case HO_TY_EXT_HANDBACK:
                 {
                    rfSinkPort.portType = CSU_GSM_PORT;
                    rtpSourcePort.portType = CSU_RTP_PORT;

                    rfSinkPort.portId.gsmHandler = parent->entryId;
                    rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->h323CallHandle);
                    break;
                 }

               case HO_TY_EXT_HANDMSC3:
               case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
               case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
                 {
                    rfSinkPort.portType = CSU_RTP_PORT;
                    rtpSourcePort.portType = CSU_RTP_PORT;

                    rfSinkPort.portId.rtpHandler = VoipCallGetRTPHandle(whichLeg->h323CallHandle);
                    rtpSourcePort.portId.rtpHandler = VoipCallGetRTPHandle(parent->h3CallLeg->h323CallHandle);
                    break;
                 }

               default:
                 {
                   //Deadly problem, need trouble shooting first before running ahead !
                   DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                    hoType_, hoState_);
                   initData();
                   DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
                   return (CC_RELEASE_HOCALLLEG);
                 }
            };


          if ((csuResult = csu_DuplexConnect(&rfSinkPort, &rtpSourcePort))
              != CSU_RESULT_CON_SUCCESS)
            {
              
              DBG_ERRORho("CSU Error : Ho Duplex Conn. Failure, (Result = %d) for (rtp port = %x) and (entryId = %d)\n",  
                          csuResult,
                          (int)rtpSourcePort.portId.rtpHandler,
                          parent->entryId);
              
              // Really caught in a bad place.  Mobile has already handed back and
              // an internal error occured.  So, just continue on and hope for the best!
              DBG_TRACEho("   Continue processing HANDOVER SUCCESS anyways.\n");
              
            }
          whichLeg->voiceConnected = true;
          
        }

      // empty the queues now that the handover is successful
      // Do this only at the end.
      emptyRRQueue      ();
      emptyDownlinkQueue();  
      emptyMMQueue      ();

      switch( hoType_ )
        {
           case HO_TY_EXT_HANDOVER:
             {
                hoState_ = HAND_ST_ANCH_EHO_COMPLT;
                break;
             }
           case HO_TY_EXT_HANDBACK:
             {
                // Send DISCONNECT (RELEASE COMPLETE) to VoIP (PLMN)
                parent->hoCallLeg->disconnectOrigExtHo();

                initData();
                break;
             }

           case HO_TY_EXT_HANDMSC3:
             {
                // Send DISCONNECT (RELEASE COMPLETE) to VoIP (PLMN)
                parent->hoCallLeg->disconnectOrigExtHo();

                // Need to swap HO with H3 data only if signalling AND voice are completed
                if (sig_and_voice_complete)
                  {
                    swapH3Data();
                    hoType_  = HO_TY_EXT_HANDOVER;
                  }

                // Note: if signalling and voice is NOT completed, leave hoType_ as
                //       it is (don't reset it to HO_TY_EXT_HANDOVER) since when
                //       voice gets completed, the hoType will signify if swapH3Data is needed.

                hoState_ = HAND_ST_ANCH_EHO_COMPLT;
                break;
             }

           case HO_TY_MNET_NONANCHOR_HANDBACK_TO_MSC3:
             {
                // Release Inter-GP10 RTP ports.
                if ( hoRtpSession_ != NULL )
                  {
                     ReleaseHandoverRtpChannel(hoRtpSession_);
                  }

                // Need to swap HO with H3 data only if signalling AND voice are completed
                if (sig_and_voice_complete)
                  {
                    swapH3Data();
                    hoType_  = HO_TY_EXT_HANDOVER;
                  }

                // Note: if signalling and voice is NOT completed, leave hoType_ as
                //       it is (don't reset it to HO_TY_EXT_HANDOVER) since when
                //       voice gets completed, the hoType will signify if swapH3Data is needed.

                hoState_ = HAND_ST_ANCH_EHO_COMPLT;
                break;
             }

           case HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR:
             {
                // clear up external handover data
                hoAgId_ = -1;
                h3AgId_ = -1;

                // JOE: clear everything to make it look like a normal GP10 to GP10 handover took place.
                hoType_  = HO_TY_HANDOVER;
                hoState_ = HAND_ST_ANCH_HO_COMPLT;  // Leave in state after normal inter-GP10 handover
                DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoComplt): leaving.\n}\n");
                return (CC_RELEASE_ALLHOCALLLEG);
             }

           default:
             {
               //Deadly problem, need trouble shooting first before running ahead !
               DBG_ERRORho ("   CC Error: messed up (hoType: %d) (hoState: %d)\n", 
                                hoType_, hoState_);
               DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
               return (CC_RELEASE_ALLHOCALLLEG);
             }
        };

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoComplt): leaving.\n}\n");
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }


   void
   CCAnchorHandover::swapH3Data(void)
   {
     HOCallLeg                   * tempCallLeg;
     DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::swapH3Data): entering......\n}\n");

     DBG_TRACEho("   Swapping the following handover data.\n");
     DBG_TRACEho("      - hoAgId_ (%d) with h3AgId_ (%d)\n", hoAgId_, h3AgId_);
     DBG_TRACEho("      - hoCallLeg (%d) with h3CallLeg (%d)\n", parent->hoCallLeg, parent->h3CallLeg);

     hoAgId_ = h3AgId_;
     h3AgId_ = -1;

     tempCallLeg = parent->hoCallLeg;
     parent->hoCallLeg = parent->h3CallLeg;
     parent->h3CallLeg = tempCallLeg;
     // parent->h3CallLeg->initData();  //JOE: some sort of cleanup for h3CallLeg should be done

     DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::swapH3Data): leaving.\n}\n");
   }

   JCCEvent_t 
   CCAnchorHandover::handleAnchExtHoFail(void)
   {
     DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoFail): entering......\n}\n");

     DBG_TRACEho("   [<==RR HANDOVER FAILURE](callId=%d,entryId=%d)\n",
                     callIndex, parent->entryId);

     //cancel T204
     parent->sessionTimer->cancelTimer();

     //Send EndHandover to Target (as opposed to HANDOVER FAILURE) specified by 03.09
     hoEndCause_ = MC_HO_SUBHO_FAILURE;
     sendEndHandover();
     hoAgId_ = -1;  // since EndHandover was sent.


     //Retry next candidate if there is candidate
     int i;
     IntraL3HandoverRequired_t *inMsg = &handReqd;

     if ( (i=getNextHoCandidate()) == (-1) )
       {
          DBG_TRACEho("   No more candidate to retry (#ofCands=%d)\n",
                          inMsg->numCandidates);

          initData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoFail): leaving.\n}\n");
          return (CC_RELEASE_HOCALLLEG);
       } else
       {
          candGlobalCellId_t candidate;
          candidate = inMsg->candGlobalCellId[i];

          DBG_TRACEho("   Trying next cand cell! %d-th of Cands(%d) candCell(%x,%x%x%x,%x%x%x,%x,%x) hoType(%d)  hoState(%d)\n",
                          i,inMsg->numCandidates, 
                          candidate.candGlobalCellId.mcc[0],candidate.candGlobalCellId.mcc[1],candidate.candGlobalCellId.mcc[2],
                          candidate.candGlobalCellId.mnc[0],candidate.candGlobalCellId.mnc[1],candidate.candGlobalCellId.mnc[2],
                          candidate.candGlobalCellId.lac, candidate.candGlobalCellId.ci,
                          hoType_, hoState_);

          if (candidate.candIsExternal)
            {
               A_Result_t         A_result;
               T_AIF_MessageUnit  a_msg;

               NonStdRasMessagePerformHandoverRQ_t Msg;

               //External handover from anchor GP to PLMN case
               populateAHandoverRequest( &MY_GLOBALCELL_ID, &candidate.candGlobalCellId );

               if ((A_result = AIF_Encode( &aifMsg, &a_msg )) != A_RESULT_SUCCESS)
                 {
                   // Couldn't encode Perform Handover A-Interface msg. 
                   DBG_ERRORho ("   A Interface encoding Error: Perform Handover Request. Result (%d)\n",
                                   (int) A_result);
                   initData();
                   DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoFail): leaving.\n}\n");
                   return (CC_RELEASE_HOCALLLEG);
                 }

               memset( (char*)&Msg, 0, sizeof(NonStdRasMessagePerformHandoverRQ_t));

               Msg.A_HANDOVER_REQUEST.msglength = a_msg.msgLength;
               memcpy(Msg.A_HANDOVER_REQUEST.A_INTERFACE_DATA, 
                     (unsigned char*)a_msg.buffer, MAX_A_INTERFACE_LEN);

               Msg.header.origSubId          = callIndex;
               // Note: destSubId (hoAgId) needs to be -1 to make GMC assign a NEW HOA.
               Msg.header.destSubId          = -1;              // NULL (unknown hoAgId - GMC will assign)
               Msg.globalCellID              = hoGlobalCellId_;
               Msg.hoCause                   = rrInMsg->l3_data.handReqd.hoCause;
               Msg.hoNumberReqd              = true;            // Always TRUE
               Msg.handoverNumber.ie_present = false;           // NULL (unknown handoverNumber - GMC will assign)
               Msg.mscNumber.ie_present      = false;           // NULL (unknown mscNumber - GMC will find out)

               DBG_TRACEho("   MESSAGE DATA:\n");
               DBG_TRACEho("       : hoAgId_=%d, callId=%d\n",hoAgId_, callIndex);
               DBG_TRACEho("       : hoGlobalCellId_ (type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
                                     hoGlobalCellId_.ci_disc,hoGlobalCellId_.mcc[0],hoGlobalCellId_.mcc[1],hoGlobalCellId_.mcc[2],
                                     hoGlobalCellId_.mnc[0],hoGlobalCellId_.mnc[1],hoGlobalCellId_.mnc[2],hoGlobalCellId_.lac,hoGlobalCellId_.ci);
               DBG_TRACEho("       : hoCause=%d\n",rrInMsg->l3_data.handReqd.hoCause);


               DBG_TRACEho("   a_msg.buffer POST-AIF PROCESSING MESSAGE DATA: A_HANDOVER_REQUEST_TYPE\n");
               DBG_TRACEho("      {\n");
               DBG_TRACEho("        \na_msg.buffer hexdump: ");
               DBG_HEXDUMPho((unsigned char*) a_msg.buffer, a_msg.msgLength);
               DBG_TRACEho("\n      }\n");


               DBG_TRACEho("   MSG POST-AIF PROCESSING MESSAGE DATA: A_HANDOVER_REQUEST_TYPE\n");
               DBG_TRACEho("      {\n");
               DBG_TRACEho("        \nMsg hexdump: ");
               DBG_HEXDUMPho((unsigned char*) &Msg, sizeof (NonStdRasMessagePerformHandoverRQ_t) );
               DBG_TRACEho("\n      }\n");

               if (VBLinkPerformHandoverRequest(&Msg)
                   == false)
                 {
                   // Ignore the message 
                   DBG_ERRORho ("    CC->VB Link ERROR: Failed on Perform Handver Request Message\n}\n");

                   initData();
                   DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::ExtPerformHO): leaving.\n}\n");
                   return (CC_RELEASE_HOCALLLEG);
                 }

               parent->sessionTimer->setTimer(CALL_HAND_SRC_THO);
               hoState_ = HAND_ST_ANCH_PERF_EHO_REQ;

           } else
           {

              // JOE: Do handover to this scenerio later!

              DBG_TRACEho("   S/W not ready yet for handover retry to VIPERCELL!\n");
              initData();

              DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoFail): leaving.\n}\n");
              return (CC_RELEASE_HOCALLLEG);

//              //Handover from anchor GP to another GP case
//              sprintf(&(cellLocationRequest.ViperCellId[0]), "%d:%d", 
//                       candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
//
//              cellLocationRequest.TxnId = callIndex;
//    
//              if (false == VBLinkLocationRequest(cellLocationRequest) )
//                {
//                   // Ignore the message 
//                  DBG_ERROR("{\nMNETeiTRACE(etrg::handleTrgHoFail): VBLinkLocationRequest failed!\n");
//                  DBG_ERROR("                                      (hoAgId=%d callId=%d entryId=%d,lac=%d,ci=%d)\n}\n",
//                              hoAgId_, callIndex, parent->entryId, 
//                              candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
//                  DBG_LEAVE();
//                  return (CC_RELEASE_EH_HB);
//                }

//              DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgHoFail): [==>HOA IP_ADDRESS_REQ](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
//                             callIndex, parent->entryId,
//                             candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
//         
//              DBG_CALL_TRACE(hoAgId_, "{\nMNETeiTRACE(etrg::handleTrgHoFail): [<==CC START TIMER Tvb](callId=%d,entryId=%d,lac=%d,ci=%d)\n}\n",
//                             callIndex, parent->entryId,
//                             candidate.candGlobalCellId.lac,candidate.candGlobalCellId.ci);
//
//              parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB)
//
//              hoState_ = HAND_ST_ANCH_VC_IP_ADDRESS_REQ
           }
      }

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHoFail): leaving.\n}\n");
      return (CC_MESSAGE_PROCESSING_COMPLT);
    }


   JCCEvent_t 
   CCAnchorHandover::handleAnchExtHandbackFail(void)
   {
     DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHandbackFail): entering......\n}\n");

     DBG_TRACEho("   [<==RR HANDBACK FAILURE](callId=%d,entryId=%d)\n",
                     callIndex, parent->entryId);

     // cancel T204
     parent->sessionTimer->cancelTimer();

     // JOE ----- What are we supposed to do on HANDBACK FAILURES?? ------
     releaseMobAndConnRes();
     hoEndCause_ = MC_HO_SUBHO_FAILURE;
     sendEndHandover();
     initData();

     DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchExtHandbackFail): leaving.\n}\n");
     return (CC_RELEASE_HOCALLLEG);
   }


   JCCEvent_t 
   CCAnchorHandover::handleAnchPostExtHoHandbackEvent(void)
   {
      A_Result_t                 aif_result;

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoHandbackEvent): entering......\n}\n");

      TwoPartyCallLeg *whichLeg;

      // as long as handover is allowed, active leg is mandatory here.
      if ((whichLeg = parent->activeLeg()) == NULL)
        {
          DBG_TRACEho("   No active leg (hoType %d, hoState %d)\n}\n", hoType_, hoState_);

          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendEndHandover();
          initData();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoHandbackEvent): leaving.\n}\n");
          return (CC_RELEASE_HOCALLLEG);
        }

      //Decode A-interface message
      memset( (char*)&aifMsg, 0, sizeof(T_AIF_MSG));

      aif_result = AIF_Decode((T_AIF_MessageUnit*)&(hoaInMsg->msgData.perfExtHandbackReq.A_HANDOVER_REQUEST),
                               &aifMsg);
      if ( aif_result != A_RESULT_SUCCESS)
        {
          DBG_ERRORho("{\n   A-MSG decoding ERROR (err=%d, hoAgId_=%d)\nA-MSG:\n",
                             aif_result, hoAgId_);
          DBG_HEXDUMPho(
              (unsigned char*)&hoaInMsg->msgData.perfExtHandbackReq.A_HANDOVER_REQUEST.A_INTERFACE_DATA,
              (int)hoaInMsg->msgData.perfExtHandbackReq.A_HANDOVER_REQUEST.msglength
          );
          DBG_ERRORho("\n}\n");

          hoEndCause_ = MC_HO_UNEXPECTED_DATA_VALUE;
          sendHandbackNack();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoHandbackEvent): leaving.\n}\n");
          return (CC_MESSAGE_PROCESSING_COMPLT);
        }
  
      if (aifMsg.msgType != A_HANDOVER_REQUEST_TYPE)
        {
           //Unexpected A-MSG received
           DBG_ERRORho("   UNEXPECTED A-MSG (msgType=%d,hoState_=%d)\n",
                           aifMsg.msgType, hoState_);

           hoEndCause_ = MC_HO_UNEXPECTED_DATA_VALUE;
           sendHandbackNack();

           DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleExtPerformHOAck): leaving.\n}\n");
           return (CC_MESSAGE_PROCESSING_COMPLT);
        }

      //Received PERFORM HANDOVER REQUEST ACK message
      DBG_TRACEho("   Received PERFORM_HANDBACK REQUEST from HOA (hoAgId_=%d, callIndex=%d)\n",
                      hoAgId_, callIndex);

      // Use the same criteria as when triggered on the anchor side.
      if (!parent->isHandoverAllowed())
      {
          // Need to fix the cause value for the nack.
          DBG_ERRORho("   External HB or H3 failed on handover disallowed.\n");

          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendHandbackNack();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoHandbackEvent): leaving.\n}\n");
          return(CC_MESSAGE_PROCESSING_COMPLT);
      }

      // Store target MSC number for Perform Handover to THIRD MSC number
      // so HOA will be able to address MAP message.
      targetMscNumber_ = hoaInMsg->msgData.perfExtHandbackReq.mscNumber;

      // Store requested encryption and channel type info for negotiation
      reqEncrypt_         = aifMsg.handoverRequest.encryptionInformation;
      optCurrEncrypt_     = aifMsg.handoverRequest.chosenEncryptAlgorithmServing;
      reqChannelType_     = aifMsg.handoverRequest.channelType;
      optCurrChannelType_ = aifMsg.handoverRequest.currentChannelType1;
    
      DBG_TRACEho("   CC External HO Info @anch-PerfHbReq: Mcc(%x%x%x), Mnc(%x%x%x), lac(%d), ci(%d), state %d)\n",
                      hoaInMsg->msgData.perfExtHandbackReq.globalCellID.mcc[0],
                      hoaInMsg->msgData.perfExtHandbackReq.globalCellID.mcc[1],
                      hoaInMsg->msgData.perfExtHandbackReq.globalCellID.mcc[2],
                      hoaInMsg->msgData.perfExtHandbackReq.globalCellID.mnc[0],
                      hoaInMsg->msgData.perfExtHandbackReq.globalCellID.mnc[1],
                      hoaInMsg->msgData.perfExtHandbackReq.globalCellID.mnc[2],
                      hoaInMsg->msgData.perfExtHandbackReq.globalCellID.lac,
                      hoaInMsg->msgData.perfExtHandbackReq.globalCellID.ci,
                      hoState_);

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoHandbackEvent): leaving.\n}\n");

      if (hoaInMsg->msgData.perfExtHandbackReq.externalCell)
        {
           return(do_Handover_To_Third_PLMN_scenario());
        }
      else
        {
          if ( isEqualGlobalCellId(&(hoaInMsg->msgData.perfExtHandbackReq.globalCellID), &MY_GLOBALCELL_ID) )
            { 
               return(do_Handback_scenario());
            }
          else
            {
               return(do_Handback_To_Third_MNET_scenario());
            }
        }
   }


   JCCEvent_t
   CCAnchorHandover::do_Handover_To_Third_PLMN_scenario(void)
   {
      A_Result_t                           A_result;
      NonStdRasMessagePerformHandoverRQ_t  Msg;
      T_AIF_MessageUnit                    a_msg;
      GlobalCellId_t                       serving_cell;
      T_AIF_MSG                            temp_aifMsg = aifMsg;  // copy for ext handover to third MSC purpose

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_Handover_To_Third_PLMN_scenario): entering......\n}\n");

      initExtHandmsc3Data();

      DBG_TRACEho("   External HANDBACK to THIRD PLMN is requested (hoType:%d, hoState:%d)\n",
                      hoType_, hoState_);

      h3GlobalCellId_ = hoaInMsg->msgData.perfExtHandbackReq.globalCellID;

      // Set aifMsg's HANDOVER REQUEST A INTERFACE values
      serving_cell.ci_disc = 0;
      memcpy( (char *)serving_cell.mcc, 
              (char *)aifMsg.handoverRequest.cellIdentifierServing.mcc, 
              NUM_MCC_DIGITS );
      memcpy( (char *)serving_cell.mnc, 
              (char *)aifMsg.handoverRequest.cellIdentifierServing.mnc, 
              NUM_MNC_DIGITS );
      serving_cell.lac = aifMsg.handoverRequest.cellIdentifierServing.lac;
      serving_cell.ci = aifMsg.handoverRequest.cellIdentifierServing.ci;

      populateAHandoverRequest( &serving_cell, &h3GlobalCellId_);

      if ((A_result = AIF_Encode( &aifMsg, &a_msg )) != A_RESULT_SUCCESS)
        {
          // Couldn't encode Perform Handover A-Interface msg. 
          DBG_ERRORho ("   A Interface encoding Error: Perform Handver Request. Result (%d)\n",
                           (int) A_result);

          hoEndCause_ = MC_HO_SUBHO_FAILURE;
          sendHandbackNack();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_Handover_To_Third_PLMN_scenario): leaving.\n}\n");
          return (CC_MESSAGE_PROCESSING_COMPLT);
        }

      memset( (char*)&Msg, 0, sizeof(NonStdRasMessagePerformHandoverRQ_t));

      Msg.A_HANDOVER_REQUEST.msglength = a_msg.msgLength;
      memcpy(Msg.A_HANDOVER_REQUEST.A_INTERFACE_DATA, 
            (unsigned char*)a_msg.buffer, MAX_A_INTERFACE_LEN);

      Msg.header.origSubId          = callIndex;
      // Note: need to put -1 as hoAgId since this requires GMC to create a new HOA
      Msg.header.destSubId          = -1;              // NULL (unknown hoAgId - GMC will assign)
      Msg.globalCellID              = h3GlobalCellId_;
      Msg.hoCause                   = 0;  // JOE: Do this cause later (extract from A-interface msg)
      Msg.hoNumberReqd              = true;            // Always TRUE
      Msg.handoverNumber.ie_present = false;           // NULL (unknown handoverNumber - GMC will assign)
      Msg.mscNumber.ie_present      = true;
      Msg.mscNumber                 = targetMscNumber_; // Send MSC number which was received on HANDBACK

      DBG_TRACEho("   MESSAGE DATA:\n");
      DBG_TRACEho("       : hoAgId_=%d, h3AgId_=%d, callId=%d\n",hoAgId_, h3AgId_, callIndex);
      DBG_TRACEho("       : h3GlobalCellId_ (type=%d, mcc[1-3]:%x,%x,%x; mnc[1-3]:%x,%x,%x;lac=%x,ci=%x)\n",
                            h3GlobalCellId_.ci_disc,hoGlobalCellId_.mcc[0],h3GlobalCellId_.mcc[1],h3GlobalCellId_.mcc[2],
                            h3GlobalCellId_.mnc[0],h3GlobalCellId_.mnc[1],h3GlobalCellId_.mnc[2],h3GlobalCellId_.lac,hoGlobalCellId_.ci);
      DBG_TRACEho("       : hoCause=SET TO 0 FOR NOW\n");
      DBG_TRACEho("       : targetMscNumber_ (present=%d,numberType=%d,numberingPlan=%d,numDigits=%d)\n",
                            targetMscNumber_.ie_present,targetMscNumber_.numberType,targetMscNumber_.numberingPlan,targetMscNumber_.numDigits);
      DBG_TRACEho("            (digits[1-10]:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x)\n",
                               targetMscNumber_.digits[0],targetMscNumber_.digits[1],targetMscNumber_.digits[2],
                               targetMscNumber_.digits[3],targetMscNumber_.digits[4],targetMscNumber_.digits[5],
                               targetMscNumber_.digits[6],targetMscNumber_.digits[7],targetMscNumber_.digits[8],
                               targetMscNumber_.digits[9],targetMscNumber_.digits[10]);


      DBG_TRACEho("   a_msg.buffer POST-AIF PROCESSING MESSAGE DATA: A_HANDOVER_REQUEST_TYPE\n");
      DBG_TRACEho("      {\n");
      DBG_TRACEho("        \na_msg.buffer hexdump: ");
      DBG_HEXDUMPho((unsigned char*) a_msg.buffer, a_msg.msgLength);
      DBG_TRACEho("\n      }\n");


      DBG_TRACEho("   MSG POST-AIF PROCESSING MESSAGE DATA: A_HANDOVER_REQUEST_TYPE\n");
      DBG_TRACEho("      {\n");
      DBG_TRACEho("        \nMsg hexdump: ");
      DBG_HEXDUMPho((unsigned char*) &Msg, sizeof (NonStdRasMessagePerformHandoverRQ_t) );
      DBG_TRACEho("\n      }\n");


      if (VBLinkPerformHandoverRequest(&Msg)
          == false)
        {
          // Ignore the message 
          DBG_ERRORho ("    CC->VB Link ERROR: Failed on Perform Handver Request Message\n}\n");

          hoEndCause_ = MC_HO_SUBHO_FAILURE;  // JOE: May not be best error cause.
          sendHandbackNack();

          DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_Handover_To_Third_PLMN_scenario): leaving.\n}\n");
          return (CC_MESSAGE_PROCESSING_COMPLT);
        }

      parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);

      hoType_  = HO_TY_EXT_HANDMSC3;
      hoState_ = HAND_ST_ANCH_PERF_EHO_H3_REQ;

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_Handover_To_Third_PLMN_scenario): leaving.\n}\n");
   }


   JCCEvent_t
   CCAnchorHandover::do_Handback_scenario(void)
   {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_Handback_scenario): entering......\n}\n");
      // External Handback scenario
      initExtHandbackData();

      DBG_TRACEho("   External HANDBACK is requested (hoState:%d)\n",
                      hoState_);

      // Build a message for MM for Handover Req.
      // Fill the data for MM message
      mmOutMsg.l3_data.handReq.channelType.bearerType = BEARER_SPEECH;
      // Since this is a handback, the anchor's current speech version is
      // preferred.  If the current speech version is not supported, more has
      // to be done in terms of changing the gateway's speech mode on the fly.
      mmOutMsg.l3_data.handReq.channelType.speechChann = parent->currSpeechCh();

      IntraL3CipherModeCommand_t  cipherCmd;
      if (ludbGetCipherAlgo(parent->ludbIndex, &cipherCmd))
        {
           //handling error return
           DBG_ERRORho("   CC->Handover Error: ludbGetCipherAlgo() failed, (ludbIndex = %d)\n",
                           parent->ludbIndex);
           cipherCmd.cmd.cipherModeSetting.ciphering = CNI_RIl3_CIPHER_NO_CIPHERING;
        }

      DBG_TRACEho("   CC->@anch:PerfHb(a2r): ludbIndex %d, bearer %d, permittedIndicator[0-7] %x,%x,%x,%x,%x,%x,%x,%x, ciphSet %d, algo %d, kc(%x,%x)\n",
                      parent->ludbIndex,
                      aifMsg.handoverRequest.channelType.speechDataIndicator,
                      aifMsg.handoverRequest.channelType.permittedIndicator[0],
                      aifMsg.handoverRequest.channelType.permittedIndicator[1],
                      aifMsg.handoverRequest.channelType.permittedIndicator[2],
                      aifMsg.handoverRequest.channelType.permittedIndicator[3],
                      aifMsg.handoverRequest.channelType.permittedIndicator[4],
                      aifMsg.handoverRequest.channelType.permittedIndicator[5],
                      aifMsg.handoverRequest.channelType.permittedIndicator[6],
                      aifMsg.handoverRequest.channelType.permittedIndicator[7],
                      cipherCmd.cmd.cipherModeSetting.ciphering,
                      cipherCmd.cmd.cipherModeSetting.algorithm,
                      cipherCmd.Kc[0],cipherCmd.Kc[1],
                      cipherCmd.Kc[0],cipherCmd.Kc[2]);

      memcpy( (unsigned char*) &(mmOutMsg.l3_data.handReq.cipherCmd),
              (unsigned char*) &(cipherCmd),
              sizeof (IntraL3CipherModeCommand_t) );

      mmOutMsg.l3_data.handReq.qid = msgQId;

      // Send internal MM msg - 
      sendMM(INTRA_L3_MM_EST_REQ, INTRA_L3_RR_HANDOVER_REQ, parent->entryId, &mmOutMsg);

      parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);
      hoType_  = HO_TY_EXT_HANDBACK;
      hoState_ = HAND_ST_ANCH_RCV_EHO_HB_REQ;
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_Handback_scenario): leaving.\n}\n");
   }



   JCCEvent_t
   CCAnchorHandover::do_Handback_To_Third_MNET_scenario(void)
   {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_Handback_To_Third_MNET_scenario): entering......\n}\n");
      // External handback to third (but target is within MNET) scenario
      initExtHandmsc3Data();

      h3GlobalCellId_ = hoaInMsg->msgData.perfExtHandbackReq.globalCellID;

      DBG_TRACEho("   External HANDBACK to THIRD within MNET is requested (hoType:%d, hoState:%d) (mcc %x%x%x, mnc %x%x%x, lac %d; ci %d)\n",
                      hoType_, hoState_,
                      h3GlobalCellId_.mcc[0],h3GlobalCellId_.mcc[1],h3GlobalCellId_.mcc[2],
                      h3GlobalCellId_.mnc[0],h3GlobalCellId_.mnc[1],h3GlobalCellId_.mnc[2],
                      h3GlobalCellId_.lac, h3GlobalCellId_.ci);

      VBLINK_API_CELL_LOCATION_REQUEST_MSG t_cellLocationRequest;

      //H3 scenario. Go ahead to inquire IP address of mscB', later check
      //if the mscB' is known or not.

      sprintf(&(t_cellLocationRequest.ViperCellId[0]), "%d:%d", h3GlobalCellId_.lac,h3GlobalCellId_.ci);

      t_cellLocationRequest.TxnId = callIndex;

      if (VBLinkLocationRequest(t_cellLocationRequest)
          == false)
        {
           // Ignore the message 
           DBG_ERRORho("   VB Link Error: h3 failed on calling VBLinkLocationRequest()(%d,%d)\n",
                           hoType_, hoState_);

           hoEndCause_ = MC_HO_SUBHO_FAILURE;
           sendHandbackNack();

           DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::handleAnchPostExtHoHandbackEvent): leaving.\n}\n");
           return(CC_MESSAGE_PROCESSING_COMPLT);
        }

      parent->sessionTimer->setTimer(CALL_HAND_SRC_TVB);
      hoType_  = HO_TY_EXT_HANDBACK_TO_MNET_NONANCHOR;
      hoState_ = HAND_ST_ANCH_EHO_H3_VC_IP_ADDRESS_REQ;
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::do_Handback_To_Third_MNET_scenario): leaving.\n}\n");
   }


   void
   CCAnchorHandover::sendEndHandover(void)
   {
      DBG_TRACEho("{\nMNEThoTRACE(ho::sendEndHandover): entering......\n}\n");

      if (hoAgId_ != -1)
        {
           //Send END_HANDOVER to HOA
           NonStdRasMessageEndHandover_t  endHandover;

           memset( (char*)&endHandover, 0, sizeof(NonStdRasMessageEndHandover_t));
           endHandover.header.origSubId = callIndex;
           endHandover.header.destSubId = hoAgId_;
           endHandover.reason           = hoEndCause_;

           if (!VBLinkEndHandover(&endHandover))
             {
                DBG_ERRORho("   VBLinkEndHandover API FAILED\n");
             } else
             {
                DBG_TRACEho("   Sent to HOA END_HANDOVER (hoAgId=%d,callId=%d,entryId=%d)\n",
                                hoAgId_, callIndex, parent->entryId);
             }
        }

      if (h3AgId_ != -1)
        {
           //Send END_HANDOVER to HOA
           NonStdRasMessageEndHandover_t  endHandover;

           memset( (char*)&endHandover, 0, sizeof(NonStdRasMessageEndHandover_t));
           endHandover.header.origSubId = callIndex;
           endHandover.header.destSubId = h3AgId_;
           endHandover.reason           = hoEndCause_;

           if (!VBLinkEndHandover(&endHandover))
             {
                DBG_ERRORho("   VBLinkEndHandover API FAILED\n");
             } else
             {
                DBG_TRACEho("   Sent to HOA END_HANDOVER (h3AgId=%d,callId=%d,entryId=%d)\n",
                                h3AgId_, callIndex, parent->entryId);
             }
        }

      DBG_TRACEho("{\nMNEThoTRACE(ho::sendEndHandover): leaving.\n}\n");
   }

   void
   CCAnchorHandover::initExtHandbackData(void)
   {
     DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::initExtHandbackData): entering......\n}\n");

     hoState_ = HAND_ST_ANCH_EHO_COMPLT;

     DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::initExtHandbackData): leaving.\n}\n");
   }


    void
    CCAnchorHandover::initExtHandmsc3Data(void)
    {
      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::initExtHandmsc3Data): entering......\n}\n");

      if (h3SimplexConnected_)
      {
          // Break connections with the RF Port
          T_CSU_PORT_ID dstRtpPort;
          T_CSU_RESULT_CON csuResult;

          dstRtpPort.portType = CSU_RTP_PORT;
          dstRtpPort.portId.rtpHandler = h3RtpSession_;

          if ((csuResult = csu_OrigTermBreakAll(&dstRtpPort))
              != CSU_RESULT_CON_SUCCESS)
          {
              DBG_ERRORho("   CSU Error : Disconn. Failure, (hoType %d;hoState_ %d)(Result = %d)(rtpHndler %x) \n",  
                              hoType_, hoState_, csuResult, h3RtpSession_);

              // Can only generate OA&M log. 
          }
      }

      if (h3RtpSession_ != NULL)
      {
          // Release the H3 RTP channel
          ReleaseHandoverRtpChannel(h3RtpSession_);
      }
    
      msMsgsQueued_           = 0;
      rrMsgsQueued_           = false;
    
      h3SimplexConnected_     = false;
      h3VcCallIndex_          = CC_MAX_CC_CALLS_DEF_VAL;
      memset(&h3GlobalCellId_, 0, sizeof(GlobalCellId_t));
      h3VcIpAddress_          = 0;
      otherH3IncomingRtpPort_ = 0;
      h3RtpSession_           = NULL;
    
      h3SrcLinkLost_          = false;
      hoType_                 = HO_TY_EXT_HANDOVER;
      hoState_                = HAND_ST_ANCH_EHO_COMPLT;

      DBG_TRACEho("{\nMNEThoTRACE(CCAnchorHandover::initExtHandmsc3Data): leaving.\n}\n");
    }