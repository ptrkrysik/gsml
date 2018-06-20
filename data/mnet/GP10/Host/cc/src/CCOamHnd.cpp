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
// File        : CCOamHnd.cpp
// Author(s)   : Bhava Nelakanti
// Create Date : 07-01-99
// Description : CC Oam handlers trap registration, trap handling,
//               initialization, and restarts
//
// *******************************************************************

// *******************************************************************
// Include Files.
// *******************************************************************

#include "stdio.h"
#include "JCC/JCCLog.h"
#include "logging/VCLOGGING.h"


// declare the CC task spawning function
extern int ccInit();

// CC Types
#include "CC/CCTypes.h"
#include "CC/CCInt.h"

#include "CC/CCconfig.h"
#include "CC/CallConfig.h"

#include "logging/VCLOGGING.h"

#include "oam_api.h"

#include "RIL3/ril3irt_db.h"

#include "csunew/csu_head.h"

extern CNI_RIL3IRT_DB                   CNI_IRT_database;

extern void ccCallDataInit(void);

//GCELL<tyu:06-01-01> BEGIN
GlobalCellId_t MY_GLOBALCELL_ID;
const GlobalCellId_t MY_GLOBALCELL_ID_DEF_VAL = {0, {'0','0','0'}, {'0','0','0'}, 1, 1};
//GCELL<tyu:06-01-01> END

UINT32 MY_VIPERCELL_IP_ADDRESS;

//CDR <xxu:08-22-00> BEGIN
BtsBasicPackage        ccBtsBasicPackage;
AdjCell_HandoverEntry  ccAdjCellHoTable[16];
AdjacentCellPackage    ccAdjacentCellPackage;
void                   ccGetBtsBasicPackage(void);
void                   ccGetAdjacentCellPackage(void);

//CDR <xxu:08-22-00> END

// Number ticks in 10 milliseconds
#define TICKS_PER_10MS  (sysClkRateGet()) / 100 

int CALL_MM_CONN_EST_T999;    //         MM Connection Established - Network (MO)
int CALL_ALERTING_T301;       //         Alerting received Timer - Network (MT)
int CALL_SETUP_T303;          //         Setup sent - Network (MT)
int CALL_CALL_CONFIRMED_T310; //         Call Confirmed received Timer - Network (MT)
int CALL_CONNECT_T313;        //         Connect sent Timer - Network (MO)
int CALL_DISCONNECT_T305;     //         Disconnect sent Timer - Network (MO and MT)
int CALL_RELEASE_T308;        //         Release sent Timer - Network (MO and MT)

int SMS_WAIT_CP_ACK_TC1;      //         Wait for CP-ACK
int SMS_WAIT_DISCONNECT;      //         Allows to send the last CP-ACK on SACCH/TCH 

//CH<xxu:11-24-99>
int CALL_HOLD_RETV_TIMER;	//	     Time control for call swapping

//RETRY<xxu:04-28-00> BEGIN
int CALL_HAND_SRC_TVB;		//	     Time control for shake-hand btw anchor and VB
//RETRY<xxu:04-28-00> END

//BCT <xxu:08-11-00> BEGIN
int CALL_BCT_NUMBER_TVB;	//	     Time control for shake-hand btw VC&VB for BCT
//RETRY<xxu:04-28-00> END

//BCT <xxu:07-06-00> BEGIN
char CALL_BCT_PREFIX[10];	//	     Prefix defined for blind call transfer invocation 
char CALL_BCT_PREFIX_LEN;	//	     
//BCT <xxu:07-06-00> END

//CDR <xxu:08-24-00>
int CALL_CDR_10M;			//	     10m call duration timer for CDR

int CALL_SAFETY_TIMER;        //         Call Task running too long

// Handover Config
int CALL_HAND_SRC_THO;        //         Handover - waiting for Target Perform Ho Req Ack (ANCH)
int CALL_HAND_SRC_T103;       //         Handover - waiting for Target Ho Complt (ANCH)
int CALL_HAND_TRG_T204;       //         Handover - waiting for Mobile Handover Complt (TRG)

int CALL_HAND_SRC_TSHO;       //         Handover - waiting for Target Perform Hb Req Ack (TRG)
int CALL_HAND_SRC_T203;       //         Handover - waiting for Target Hb Complt (TRG)
int CALL_HAND_TRG_T104;       //         Handover - waiting for Mobile Handover Complt (ANCH)


//ext-HO <xxu:07-01-01>
int CALL_HAND_TRG_T211;       //         Handover - waiting for Mobile Handover Complt or Failure (TRG)
int CALL_HAND_TRG_T202;       //         Handover - waiting for Mobile Handover Complt btw BSSs under MSC-B

SpeechChann_t currChannDef;


void irtCCDataInit(void)
{
  short i;
  for(i=0; i< MAX_RIL3IRT_DB_ENTRIES ; i++)
    {
      CNI_IRT_database.m_entry[i].cc_id = cc_id;
    }
}

void ccDataInit(void)
{
  ccCallDataInit();
  irtCCDataInit();
  //csu_ClearAllTable();
}

// OAM Trap Registration
void callOamTrapRegister(void)
{
  DBG_FUNC("callOamTrapRegister", CC_LAYER);
  DBG_ENTER();

  STATUS oamStatus;

  // Reset any trap setting to get a clean start
  if ((oamStatus = oam_unsetTrapByModule(MODULE_CC)) 
      != STATUS_OK) 
    {
      DBG_ERROR("OAM->Call ERROR: Failed(status = %d) for unsetTrapByModule\n",
                oamStatus);
    }
  
  // Register to trap callConfigData for timers
  if ((oamStatus = oam_setTrap(MODULE_CC, ccMsgQId, MIB_callConfigData))
      != STATUS_OK)
    {
      DBG_ERROR("OAM->Call ERROR: Failed(status = %d) in setTrap for callConfigData\n",
                oamStatus);
    }

  //CDR <xxu:08-22-00> BEGIN
  if ((oamStatus = oam_setTrap(MODULE_CC, ccMsgQId, MIB_btsBasicPackage))
      != STATUS_OK)
    {
      DBG_ERROR("OAM->Call ERROR: Failed(status = %d) in setTrap for BtsBasicPackage\n",
                 oamStatus);
    }

  if ((oamStatus = oam_setTrap(MODULE_CC, ccMsgQId, MIB_adjacentCellPackage))
      != STATUS_OK)
    {
      DBG_ERROR("OAM->Call ERROR: Failed(status = %d) in setTrap for adjCellPackage\n",
                 oamStatus);
    }

  if ((oamStatus = oam_setTrap(MODULE_CC, ccMsgQId, MIB_handoverTimerMib))
      != STATUS_OK)
    {
      DBG_ERROR("OAM->Call ERROR: Failed(status = %d) in setTrap for handoverTimerMib\n",
                 oamStatus);
    }
  //CDR <xxu:08-22-00> END

  DBG_LEAVE();
}               

//BCT xxu<07-17-00> BEGIN
void setBctPrefix(char *prefix)
{
  printf(" OLD: prefix=%s, length=%d; NEW: prefix=%s, length=%d\n",
           CALL_BCT_PREFIX, CALL_BCT_PREFIX_LEN,
           prefix, strlen(prefix));
  memset(CALL_BCT_PREFIX,0,10);
  strcpy(CALL_BCT_PREFIX,prefix);
  CALL_BCT_PREFIX_LEN = strlen(CALL_BCT_PREFIX);
}
//BCT xxu<07-17-00> END

// OAM Call Data Initialization
void callOamDataInit(void)
{
  DBG_FUNC("callOamDataInit", CC_LAYER);
  DBG_ENTER();

  CallConfigData callConfigVal;
  HandoverTimerMib handoverTimerVal;
  STATUS oamStatus;

  // Temporarily assign default values here.
  CALL_MM_CONN_EST_T999 = CALL_MM_CONN_EST_T999_DEF_VAL * TICKS_PER_10MS;
  CALL_ALERTING_T301 = CALL_ALERTING_T301_DEF_VAL * TICKS_PER_10MS;
  CALL_SETUP_T303 = CALL_SETUP_T303_DEF_VAL * TICKS_PER_10MS;
  CALL_CALL_CONFIRMED_T310 = CALL_CALL_CONFIRMED_T310_DEF_VAL * TICKS_PER_10MS;
  CALL_CONNECT_T313 = CALL_CONNECT_T313_DEF_VAL * TICKS_PER_10MS;
  CALL_DISCONNECT_T305 = CALL_DISCONNECT_T305_DEF_VAL * TICKS_PER_10MS;
  CALL_RELEASE_T308 = CALL_RELEASE_T308_DEF_VAL * TICKS_PER_10MS;
  CALL_SAFETY_TIMER = CALL_SAFETY_TIMER_DEF_VAL * TICKS_PER_10MS;

  //CDR <xxu:08-24-00>
  CALL_CDR_10M = CALL_CDR_10M_DEF_VAL;

  //CH<xxu:11-24-99>
  CALL_HOLD_RETV_TIMER = CALL_HOLD_RETV_TIMER_DEF_VAL;

  //RETRY<xxu:05-01-00> BEGIN
  CALL_HAND_SRC_TVB = CALL_HAND_SRC_TVB_DEF_VAL;
  //RETRY<xxu:05-01-00> END

  //BCT <xxu:07-06-00> BEGIN
  CALL_BCT_NUMBER_TVB = CALL_HAND_SRC_TVB_DEF_VAL;

   strcpy(CALL_BCT_PREFIX, "44*");
  CALL_BCT_PREFIX_LEN = strlen(CALL_BCT_PREFIX);
  for (int i=0;i<CALL_BCT_PREFIX_LEN;i++)
  {
       switch (CALL_BCT_PREFIX[i])
       {
       case 0x2A: //'*'
            CALL_BCT_PREFIX[i] = 0x0A;
            break;
            
       case 0x23: //'#'
            CALL_BCT_PREFIX[i] = 0x0B;
            break;
       default:   //'0'-'9'
            if ( CALL_BCT_PREFIX[i]  >= 0x30 && CALL_BCT_PREFIX[i] < 0x40 )
                 CALL_BCT_PREFIX[i] -= 0x30;
            else
                 DBG_ERROR("OAM->CC@callOamDataInit: bctPrefix=%s", CALL_BCT_PREFIX);
            break;
       } 
  }
  //BCT<xxu:07-06-00> END

  // Temporary assignment of Handover Timers
  CALL_HAND_SRC_THO = CALL_HAND_SRC_THO_DEF_VAL * TICKS_PER_10MS;
  CALL_HAND_SRC_T103 = CALL_HAND_SRC_T103_DEF_VAL * TICKS_PER_10MS;
  CALL_HAND_TRG_T204 = CALL_HAND_TRG_T204_DEF_VAL * TICKS_PER_10MS;

  CALL_HAND_SRC_TSHO = CALL_HAND_SRC_TSHO_DEF_VAL * TICKS_PER_10MS;
  CALL_HAND_SRC_T203 = CALL_HAND_SRC_T203_DEF_VAL * TICKS_PER_10MS;
  CALL_HAND_TRG_T104 = CALL_HAND_TRG_T104_DEF_VAL * TICKS_PER_10MS;

  //ext-HO <xxu:07-01-01>
  CALL_HAND_TRG_T211 = CALL_HAND_TRG_T211_DEF_VAL * TICKS_PER_10MS;
  CALL_HAND_TRG_T202 = CALL_HAND_TRG_T202_DEF_VAL * TICKS_PER_10MS;

  // SMS timers
  SMS_WAIT_CP_ACK_TC1= SMS_WAIT_CP_ACK_TC1_DEF_VAL * TICKS_PER_10MS;
  SMS_WAIT_DISCONNECT= SMS_WAIT_DISCONNECT_DEF_VAL * TICKS_PER_10MS;

  if  ((oamStatus = getMibStruct(MIB_callConfigData, 
                                 (OCTET_T *)&callConfigVal, sizeof(callConfigVal)))
       != STATUS_OK)
    {
      // failed to get OAM Config. data.
      DBG_ERROR("OAM->Call FATAL ERROR: failed(status = %d) to get Call Config.\n",
                oamStatus);
      printf("OAM->Call FATAL ERROR: failed(status = %d) to get Call Config.\n",
             oamStatus);
    }
  else
    {
      printf("CC Call oam data: (T301=%d), (T303=%d), (T310=%d)\n",
             callConfigVal.cc_alerting_T301,
             callConfigVal.cc_setup_T303,
             callConfigVal.cc_call_confirmed_T310);

      printf("                  (T313=%d), (T305=%d), (T308=%d).\n\n",
             callConfigVal.cc_connect_T313,
             callConfigVal.cc_disconnect_T305,
             callConfigVal.cc_release_T308);

      // make the conversion between the MIB value (in 10 ms) and the
      // actual Timer value used in the VxWorks API call (1 VxWorks ticks = 16.69ms)
      CALL_ALERTING_T301 = (callConfigVal.cc_alerting_T301) * TICKS_PER_10MS;
      CALL_SETUP_T303 = (callConfigVal.cc_setup_T303) * TICKS_PER_10MS;
      CALL_CALL_CONFIRMED_T310 = (callConfigVal.cc_call_confirmed_T310) * TICKS_PER_10MS;
      CALL_CONNECT_T313 = (callConfigVal.cc_connect_T313) * TICKS_PER_10MS;
      CALL_DISCONNECT_T305 = (callConfigVal.cc_disconnect_T305) * TICKS_PER_10MS;
      CALL_RELEASE_T308 = (callConfigVal.cc_release_T308) * TICKS_PER_10MS;
    }

  if  ((oamStatus = getMibStruct(MIB_handoverTimerMib, 
                                 (OCTET_T *)&handoverTimerVal, sizeof(HandoverTimerMib)))
       != STATUS_OK)
    {
      // failed to get OAM Config. data.
      DBG_ERROR("OAM->Call FATAL ERROR: failed(status = %d) to get Handover Timer.\n",
                oamStatus);
      printf("OAM->Call FATAL ERROR: failed(status = %d) to get Handover Timer.\n",
             oamStatus);
    }
  else
    {
        printf("CC Call oam data: (T101=%d), (T102=%d), (T103=%d), (T104=%d)\n",
                 handoverTimerVal.t101,
                 handoverTimerVal.t102,
                 handoverTimerVal.t103,
                 handoverTimerVal.t104);

        printf("                  (T201=%d), (T202=%d), (T204=%d), (T210=%d), (T211=%d).\n\n",
                 handoverTimerVal.t201,
                 handoverTimerVal.t202,
                 handoverTimerVal.t204,
                 handoverTimerVal.t210,
                 handoverTimerVal.t211);

        // make the conversion between the MIB value (in 10 ms) and the
        // actual Timer value used in the VxWorks API call (1 VxWorks ticks = 16.69ms)
        CALL_HAND_SRC_T103 = (handoverTimerVal.t103) * TICKS_PER_10MS;
        CALL_HAND_TRG_T104 = (handoverTimerVal.t104) * TICKS_PER_10MS;
        CALL_HAND_TRG_T202 = (handoverTimerVal.t202) * TICKS_PER_10MS;
        CALL_HAND_TRG_T204 = (handoverTimerVal.t204) * TICKS_PER_10MS;
        CALL_HAND_TRG_T211 = (handoverTimerVal.t211) * TICKS_PER_10MS;
    }




  //CDR <xxu:08-22-00> BEGIN
  ccGetBtsBasicPackage();
  ccGetAdjacentCellPackage();
  //CDR <xxu:08-22-00> END

  currChannDef.version = CNI_RIL3_GSM_FULL_RATE_SPEECH_VERSION_2;
 
  DBG_LEAVE();
}

// OAM CC Data Initialization
void ccOamDataInit(void)
{
  DBG_FUNC("ccOamDataInit", CC_LAYER);
  DBG_ENTER();

  INT_32_T mibVal;
  STATUS oamStatus;

  if  ((oamStatus = getMibIntVar(MIB_bts_operationalState, &mibVal))
       != STATUS_OK)
    {
      // failed to get OAM Config. data.
      printf("OAM->CC FATAL ERROR: failed(status = %d) to get CC Config - BTS Op State.\n",
             oamStatus);
      printf("CC Log: FATAL - CC Non functional - cannot get BTS Op. State.\n");
      DBG_ERROR("OAM->CC FATAL ERROR: failed(status = %d) to get CC Config - BTS Op State.\n",
                oamStatus);
      DBG_ERROR("CC Log: FATAL - CC Non functional - cannot get BTS Op. State.\n");
    }
  else
    {
      ccOldBTSState = mibVal;
    }

  // Temporarily assign a default value for cc calls here.
  CC_MAX_CC_CALLS = CC_MAX_CC_CALLS_DEF_VAL;

  //    if  ((oamStatus = getMibIntVar(MIB_cc_max_calls, &mibVal))
  //       != STATUS_OK)
  //    {
  //      // failed to get OAM Config. data.
  //      DBG_ERROR("OAM->CC FATAL ERROR: failed(status = %d) to get CC Config - Max Calls.\n",
  //                            oamStatus);
  //      printf("OAM->CC FATAL ERROR: failed(status = %d) to get CC Config - Max Calls.\n",
  //                      oamStatus);
  //    }
  //    else 
  //    {
  //      CC_MAX_CC_CALLS = mibVal;
  //    }
         
  //GCELL<tyu:06-01-01> REMOVED

  MY_VIPERCELL_IP_ADDRESS = inet_addr(GetViperCellIpAddress());
  
  // Get config. data that is used by the individual calls - including the timers.
  callOamDataInit();

  DBG_LEAVE();
}

void ccOamHandleBTSOpStateChange(INT_32_T opSt)
{
  DBG_FUNC("ccOamHandleBTSOpStateChange", CC_LAYER);
  DBG_ENTER();

  // Handle (Disabled->Enabled) and (Enabled->Disabled) changes.
  DBG_TRACE("OAM->CC Log: Trap received for BTS (Oper. State = %d)\n",
            opSt);

  if (ccOldBTSState == VAL_bts_operationalState_enabled)
    {
      if ((ccOldBTSState = opSt) == VAL_bts_operationalState_disabled)
        {
          ccDataInit();
        }
    }
  else
    {
      // No special action needed.
      ccOldBTSState = opSt;
    }
  DBG_LEAVE();
}


// OAM Trap Handling
void callOamProcessTrapMsg(TrapMsg* oamInMsg)
{
  DBG_FUNC("callOamProcessTrapMsg", CC_LAYER);
  DBG_ENTER();

  //    DBG_TRACE("OAM->CC Log: Trap received. (Mib Tag = %d), (MsgType = %d)\n",
  //                oamInMsg->mibTag,
  //                oamInMsg->msgType);

  switch(oamInMsg->msgType) {

  case MT_TRAP:
    {
        //Got a trap notification
        switch (MIB_TBL(oamInMsg->mibTag))
        {
	  case MTBL_btsBasicPackage:
    	       ccGetBtsBasicPackage();
             if (oamInMsg->mibTag == MIB_bts_operationalState)
                 ccOamHandleBTSOpStateChange(oamInMsg->val.varVal);         
		 break;
        case MTBL_adjacentCellPackage:
		 ccGetAdjacentCellPackage();
             break;

        default:
          {
             switch(oamInMsg->mibTag) {
             case MIB_cc_alerting_T301:
                  CALL_ALERTING_T301 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;

             case MIB_cc_setup_T303:
                  CALL_SETUP_T303 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;
 
             case MIB_cc_call_confirmed_T310:
                  CALL_CALL_CONFIRMED_T310 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;

             case MIB_cc_connect_T313:
                  CALL_CONNECT_T313 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;

             case MIB_cc_disconnect_T305:
                  CALL_DISCONNECT_T305 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;

             case MIB_cc_release_T308:
                  CALL_RELEASE_T308 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;

             case MIB_t103:
                  CALL_HAND_SRC_T103 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;
                  
             case MIB_t104:
                  CALL_HAND_TRG_T104 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;
                  
             case MIB_t202:
                  CALL_HAND_TRG_T202 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;
                  
             case MIB_t204:
                  CALL_HAND_TRG_T204 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;
                  
             case MIB_t211:
                  CALL_HAND_TRG_T211 = (oamInMsg->val.varVal) * TICKS_PER_10MS;
                  break;

             case MIB_t101:
             case MIB_t102:
             case MIB_t201:
             case MIB_t210:
             case MIB_cc_mm_conn_est_T999:
             case MIB_cc_safety_timer:
             case MIB_cc_max_L3_msgs:
             case MIB_cc_l3_msg_q_priority:
             case MIB_cc_task_priority:
             case MIB_cc_task_stack_size:
             case MIB_cc_max_calls:
                  // ignore
                  break;

             //CDR <xxu:08-22-00> BEGIN                
             //case MIB_bts_operationalState:
             //     ccOamHandleBTSOpStateChange(oamInMsg->val.varVal);
             //     break;
             //CDR <xxu:08-22-00> END

             default:
                  DBG_ERROR("OAM->CC Call Error: Unexpected (Mib Tag = %d) for Trap. \n",  
                             oamInMsg->mibTag);
                  break;
             }
         }
         break;
      }
    }
    break;

  default:
    DBG_ERROR("OAM->CC Call Error: Unexpected (Msg Type = %d).\n",  
              oamInMsg->msgType);
    break;
  }
  DBG_LEAVE();
}
  
void ccRestart(void)
{
  short i;
  for(i=0; i< CC_MAX_CC_CALLS;i++)
    {
      semTake(ccSession[i].semId, WAIT_FOREVER);
      semDelete(ccSession[i].semId);
    }
}

// cc reboot action
int  SysCommand_CC (T_SYS_CMD action)
{
  DBG_FUNC("SysCommand_CC", CC_LAYER);
  DBG_ENTER();

  switch(action){

  case SYS_SHUTDOWN:
    printf("[CC] shutdown notification received\n");
    DBG_TRACE("[CC] shutdown notification received\n");
    break;

  case SYS_REBOOT:
    printf("[CC] Reboot notification received. No action needed.\n");
    DBG_TRACE("[CC] Reboot notification received. No action needed.\n");
    break;

  case SYS_START:
    // Also print out a Starting marker for logs
    DBG_TRACE("******************************************************************************\n");
          
    printf("[CC] start notification received\n");
    DBG_TRACE("[CC] start notification received\n");

    // create message queue and put the message queue ID in a global
    // variable so that all the Layer-3 Interface API functions can send
    // message to this queue

    ccMsgQId = msgQCreate(CC_MAX_L3_MSGS,                       // number of messages
                          CC_MAX_MSG_LENGTH,            // size of each message
                          CC_L3_MSG_Q_PRIORITY          // priority of the queue
                          );


    // check message queue creation result
    if(ccMsgQId == NULL)
      {
        // message queue creation failure. No point to continue. 
        printf("CC FATAL ERROR: failed to create message queue\n" );
        //Abort the task.
        DBG_LEAVE();
        return (-1);
      }

    //Initialize for RRM to enter into IRT
    cc_id.msgq_id = ccMsgQId;     

    short i;
    // Initialize all sub tasks message queues.
    for(i=0; i< CC_MAX_CC_CALLS_DEF_VAL;i++)
      {
              
        // create message queue and put the message queue ID in a global
        // variable so that all the Layer-3 Interface API functions can send
        // message to this queue
      
        ccSession[i].msgQId = 
          msgQCreate(CALL_MAX_L3_MSGS,     // number of messages
                     CC_MAX_MSG_LENGTH,    // size of each message
                     CALL_L3_MSG_Q_PRIORITY        // priority of the queue
                     );
              
      
        // check message queue creation result
        if(ccSession[i].msgQId == NULL)
          {
            // message queue creation failure. No point to continue. 
            //Abort the task.
            // message queue creation failure. No point to continue. 
            printf("CC CALL FATAL ERROR: failed to create message queue\n" );
                  
            DBG_LEAVE();
            return (-1);
          }
         
      }
          
    // module initialization etc.
    ccInit();
    break;
          
  default:
    DBG_ERROR("Root->CC Call Error: Unexpected (action = %d).\n",  
              action);
    DBG_LEAVE();
    return(-1);
      
  }
  DBG_LEAVE();
  return 0;
}

//CDR <xxu:08-22-00> BEGIN
void  ccGetBtsBasicPackage(void)
{
  DBG_FUNC("ccGetBtsBasicPackage", CC_LAYER);
  DBG_ENTER();

  STATUS oamStatus;

  oamStatus = getMibStruct(MIB_btsBasicPackage, (unsigned char*)&ccBtsBasicPackage,
	 	               sizeof(BtsBasicPackage));
  if (oamStatus != STATUS_OK)  
  {
      // failed to get btsBasicPackage data.
      DBG_ERROR("OAM->Call FATAL ERROR: failed(status = %d) to get ccbtsBasicPackage\n",
                 oamStatus);
      printf("OAM->Call FATAL ERROR: failed(status = %d) to get ccbtsBasicPackage\n",
                 oamStatus);
      // Temporarily assign a default value here.
//GCELL<tyu:06-01-01> BEGIN
      MY_GLOBALCELL_ID = MY_GLOBALCELL_ID_DEF_VAL;
  }  

   // Convert mcc, mnc to byte string
   getByteAryByInt(ccBtsBasicPackage.bts_mcc, (char *) &ccBtsBasicPackage.bts_mcc,3);
   getByteAryByInt(ccBtsBasicPackage.bts_mnc, (char *) &ccBtsBasicPackage.bts_mnc, 2);

   // Initialize MY_GLOBALCELL_ID here
   MY_GLOBALCELL_ID.mcc[0] = ((char*)&(ccBtsBasicPackage.bts_mcc))[0];
   MY_GLOBALCELL_ID.mcc[1] = ((char*)&(ccBtsBasicPackage.bts_mcc))[1];
   MY_GLOBALCELL_ID.mcc[2] = ((char*)&(ccBtsBasicPackage.bts_mcc))[2];

   MY_GLOBALCELL_ID.mnc[0] = ((char*)&(ccBtsBasicPackage.bts_mnc))[0];
   MY_GLOBALCELL_ID.mnc[1] = ((char*)&(ccBtsBasicPackage.bts_mnc))[1];
   MY_GLOBALCELL_ID.mnc[2] = 0xf;


   MY_GLOBALCELL_ID.lac = ccBtsBasicPackage.bts_lac;
   MY_GLOBALCELL_ID.ci = ccBtsBasicPackage.bts_ci;
//GCELL<tyu:06-01-01> END

   DBG_TRACE("MCC(%d,%d,%d) MNC(%d,%d)\n",
		((char*)&(ccBtsBasicPackage.bts_mcc))[0],
		((char*)&(ccBtsBasicPackage.bts_mcc))[1],
		((char*)&(ccBtsBasicPackage.bts_mcc))[2],
		((char*)&(ccBtsBasicPackage.bts_mnc))[0],
		((char*)&(ccBtsBasicPackage.bts_mnc))[1]);

}

void ccGetAdjacentCellPackage(void)
{
  DBG_FUNC("ccGetAdjacentCellPackage", CC_LAYER);
  DBG_ENTER();

  STATUS oamStatus;

  if (STATUS_OK != (oamStatus = getMibStruct(MIB_adjacentCellPackage,
                                (OCTET_T *)&ccAdjacentCellPackage, sizeof(AdjacentCellPackage))))
    {
      // failed to get adjacentCellPackage.
      DBG_ERROR("OAM->Call FATAL ERROR: failed(status = %d) to get adjacentCellPackage\n",
                 oamStatus);
      printf("OAM->Call FATAL ERROR: failed(status = %d) in to get adjacentCellPackage\n",
                 oamStatus);
      return;
    }  

  //Get handover table list
  if (STATUS_OK != (oamStatus = getMibTbl(MIBT_adjCell_handoverEntry,
                                (OCTET_T *)&ccAdjCellHoTable, sizeof(ccAdjCellHoTable))))
    {
      // failed to get btsBasicPackage data.
      DBG_ERROR("OAM->Call FATAL ERROR: failed(status = %d) to get ccAdjCellHoTable\n",
                 oamStatus);
      printf("OAM->Call FATAL ERROR: failed(status = %d) to get ccAdjCellHoTable\n",
                 oamStatus);
      return;
    }  

       
    ccAdjacentCellPackage.adjCell_handoverTable= (Table *)&ccAdjCellHoTable;

    for ( int i=0; i<16; i++)
    {

        if (((unsigned int)(((AdjCell_HandoverEntry*)
            (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_handoverCellID)))
        { 

         getByteAryByInt( ((AdjCell_HandoverEntry*)(&ccAdjCellHoTable))[i].adjCell_mcc,
                          (char*)&(((AdjCell_HandoverEntry*)(&ccAdjCellHoTable))[i].adjCell_mcc),3);
         getByteAryByInt( ((AdjCell_HandoverEntry*)(&ccAdjCellHoTable))[i].adjCell_mnc,
                          (char*)&(((AdjCell_HandoverEntry*)(&ccAdjCellHoTable))[i].adjCell_mnc),2);

         DBG_TRACE("%dth ADJ-CELL MCC(%d,%d,%d) MNC(%d,%d) LAC(%d) CI(%d)\n", i,
                    ((unsigned char*)&(((AdjCell_HandoverEntry*)
                                      (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc))[0],
                    ((unsigned char*)&(((AdjCell_HandoverEntry*)
                                      (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc))[1],
                    ((unsigned char*)&(((AdjCell_HandoverEntry*)
                                      (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mcc))[2],
                    ((unsigned char*)&(((AdjCell_HandoverEntry*)
                                      (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mnc))[0],
                    ((unsigned char*)&(((AdjCell_HandoverEntry*)
                                      (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_mnc))[1],
                    ((short)((((AdjCell_HandoverEntry*)
                                     (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_lac))),
                    ((short)((((AdjCell_HandoverEntry*)
                                     (ccAdjacentCellPackage.adjCell_handoverTable))[i].adjCell_ci))) 
                    );
        }
    }

}

void ccSetCdrTimer(int val)
{
     long t;
     t = (long)((CALL_CDR_10M * 16.7)/1000)/60;

     printf("Old ccCdr Timer value(tick %d, % min): %d\n", CALL_CDR_10M, t);
     CALL_CDR_10M = (int)((val*60000)/16.7);
     printf("New ccCdr Timer value(tick %d, % min): %d\n", CALL_CDR_10M, val);

}
//CDR <xxu:08-22-00> END


