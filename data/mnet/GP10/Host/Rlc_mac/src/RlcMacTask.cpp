// *******************************************************************
//
// (c) Copyright Cisco 2000
// All Rights Reserved
//
// *******************************************************************

// *******************************************************************
//
// File        : RlcMacTask.cpp
// Author(s)   : Tim Olson
// Create Date : 6/5/2000
// Description : 
//
// *******************************************************************


#include "RlcMacTask.h"
#include "RlcMacConfig.h"
#include "Os/JCMsgQueue.h"
#include "GP10OsTune.h"
#include "MnetModuleId.h"
#include "GP10MsgTypes.h"
#include "dsp/dsphmsg.h"
#include "Grr/grr_intf.h"

// Flag used to indicate that the one and only RlcMac task
// has been created.
bool RlcMacTask::isCreated = FALSE;


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::RlcMacTask
**
**    PURPOSE: RlcMacTask constructor.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
RlcMacTask::RlcMacTask() : RlcMacTaskObj(RLCMAC_TASK_NAME)
{
}

/*----------------------------------------------------------------------------**
**
**    METHOD NAME: SysCommand_RlcMac
**
**    PURPOSE: SysCommand_RlcMac is called anytime the system needs to notify
**       a module about a system event.  
**
**    INPUT PARAMETERS: action - type of system command
**
**    RETURN VALUE(S): OK
**
**----------------------------------------------------------------------------*/
int SysCommand_RlcMac(T_SYS_CMD action)
{
   
   switch(action){
      case SYS_SHUTDOWN:
         printf("[RlcMac] Received system shutdown notification\n");
         break;
      case SYS_START:
         printf("[RlcMac] Received system start notification\n");
         if (RlcMacTask::InitRlcMac() != OK)
         {
            printf("Unable to initialize RlcMac properly\n");
         }
         break;
         
      case SYS_REBOOT:
         printf("[RlcMac] Received system reboot notification\n");
         break;
         
      default:
         printf("[RlcMac] Unknown system command received %d\n", action);
   }
   
   return OK;
}



/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::InitRlcMac
**
**    PURPOSE: InitRlcMac function provides the basic initialization for the
**      RLC/MAC task.  A single instance of RlcMacTask is created.  The single 
**      instance is referenced by static member pointer theRlcMacTask.
**
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/

int RlcMacTask::InitRlcMac(void)
{
   DBG_FUNC("RlcMacTask::InitRlcMac",RLC_MAC);
   DBG_ENTER();

    if (isCreated == FALSE)
    {
        // Create the one and only RlcMacTask object.
        if ((theRlcMacTask = new RlcMacTask()) == 0)
        {
            DBG_ERROR("RlcMac: Unable to allocate memory for RlcMacTask\n");
            DBG_LEAVE();
            return ERROR;
        }
        
        // Create the RlcMac message queue
        RlcMacMsgQ = new JCMsgQueue(RLCMAC_MAX_MSGS, RLCMAC_MAX_MSG_LENGTH, JC_MSG_Q_PRIORITY);
        
        if (!RlcMacMsgQ)
        {
            DBG_ERROR("RlcMac: Unable to create message queue\n");
            DBG_LEAVE();
            return ERROR;  
        }
        
        if (theRlcMacTask->RlcMacTaskObj.JCTaskSpawn(RLCMAC_TASK_PRIORITY,
                    RLCMAC_TASK_OPTION,
                    RLCMAC_TASK_STACK_SIZE, 
                    (FUNCPTR) RlcMacTask::RlcMacLoop,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
                    MODULE_RLCMAC, JC_CRITICAL_TASK ) == ERROR)
        {
            DBG_ERROR("RlcMac: Unable to spawn task\n");
            DBG_LEAVE();
            return ERROR;
        }
        
        isCreated = TRUE;
        
        // TEMP !!!! TEMP !!!!
        // For now set MIB variables
        oam_setMibIntVar(MODULE_RLCMAC,MIB_t3191, 5);
        oam_setMibIntVar(MODULE_RLCMAC,MIB_t3193, 2);
        oam_setMibIntVar(MODULE_RLCMAC,MIB_t3195, 5);
        oam_setMibIntVar(MODULE_RLCMAC,MIB_t3169, 5);
    }

    DBG_LEAVE();
    return OK;
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: CallRlcMacLoop
**
**    PURPOSE: This is a wrapper function used to call RlcMacLoop.  This is needed
**     so that a simple extern declaration is sufficient to call RlcMacLoop.  If
**     RlcMacLoop were call directly the entire MCHTask class would need to be
**     included in the root task scope.  
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int CallRlcMacLoop(void)
{
    return RlcMacTask::RlcMacLoop();
}


/*----------------------------------------------------------------------------**
**
**    METHOD NAME: RlcMacTask::RlcMacLoop
**
**    PURPOSE: RlcMacLoop is the main message processing loop for the RlcMacTask.
**
**    INPUT PARAMETERS: none
**
**    RETURN VALUE(S): none
**
**----------------------------------------------------------------------------*/
int RlcMacTask::RlcMacLoop( void )
{
    DBG_FUNC("RlcMacTask::RlcMacLoop", RLC_MAC);
    DBG_ENTER();
    
    JC_STATUS status;
    JCMsgQueue *replyMsgQ;
    GP10_MESSAGE_TYPES msgType;
    MNET_MODULE_ID modId;
    char rlcMacMsg[sizeof(BSSGP_API_MSG)];
    unsigned int nBytes;
    
    theRlcMacTask->RlcMacTaskObj.JCTaskEnterLoop();
    
    while (1)
    {
        if (status = RlcMacMsgQ->JCMsgQReceive
        (
            &replyMsgQ,
            (unsigned int *)&msgType,
            &modId, 
            rlcMacMsg,
            &nBytes,
            JC_WAIT_FOREVER
        ) == ERROR)
        {
            DBG_WARNING("RlcMacTask::RlcMacLoop: Msg receive error status(%#x)\n",
                status);
            continue;        
        }
        
        switch (msgType)
        {
            case RLCMAC_READY_TO_GO :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_READY_TO_GO msg\n");
                // Setup channel information from GRR
                for (int trx = 0; trx < MAX_TRX; trx++)
                {
                    for (int ts = 0; ts < MAX_TIMESLOTS; ts++)
                    {
                        theRlcMacTask->tsPool.SetChannelComb(trx, ts, 
                            grr_GetTrxSlotChanComb(trx, ts));
                    }
                }
                grr_SendReadyToGoAck(MODULE_RLCMAC, grr_MsgQId);
            }
            break;
            case RLCMAC_TIMESLOT_CONFIG :
            {
                RlcMacTSConfiguration *configMsg = (RlcMacTSConfiguration *)rlcMacMsg;
                DBG_TRACE("RLC/MAC received RLCMAC_TIMESLOT_CONFIG msg\n");
                DBG_TRACE("RLC/MAC Setting trx(%d) ts(%d) chanComb(%d)\n", 
                    configMsg->trx, configMsg->timeslot, configMsg->chanComb);
                theRlcMacTask->tsPool.SetChannelComb(configMsg->trx, configMsg->timeslot,
                    configMsg->chanComb);
            }
            break;
            case RLCMAC_ADMIN_STATE_CHANGE :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_ADMIN_STATE_CHANGE msg\n");
                theRlcMacTask->DecodeOamAdminMsgInd((RlcMacAdminStateChangeType *)rlcMacMsg);
            }
            break;
            case RLCMAC_OP_STATE_CHANGE :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_OP_STATE_CHANGE msg\n");
                theRlcMacTask->DecodeOamOpMsgInd((RlcMacOpStateChangeType *)rlcMacMsg);
            }
            break;
            case RLCMAC_RM_MD_MSG_IND :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_RM_MD_MSG_IND msg\n");
                theRlcMacTask->ProcessRmMdMsgInd((T_CNI_RIL3MD_RRM_MSG *)rlcMacMsg);
            }
            break;
            case RLCMAC_L1_MSG_IND :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_L1_MSG_IND msg\n");
                theRlcMacTask->DecodeL1MsgInd((RlcMacL1MsgIndType *)rlcMacMsg);
            }
            break;
            case RLCMAC_DL_PDU_REQ :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_DL_PDU_REQ msg\n");
                theRlcMacTask->AllocateDLResources((BSSGP_DL_UNITDATA_MSG *)rlcMacMsg);
            }
            break;
            case RLCMAC_L1_RTS :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_L1_RTS msg\n");
                theRlcMacTask->DecodeL1MsgInd((RlcMacL1MsgIndType *)rlcMacMsg);
            }
            break;
            case RLCMAC_T3169_EXPIRY :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_T3169_EXPIRY msg\n");
                int tbfPointer = 0;
                
                for (int i = sizeof(int) - 1; i >= 0; i--)
                {
                    tbfPointer |= (rlcMacMsg[i] << (i * 8));
                }
                
                ULTbf *pUlTbf = (ULTbf *)(tbfPointer);
                if ((pUlTbf) && (pUlTbf->GetTbfState() != Tbf::FREE))
                {
                    pUlTbf->rlcLink.ProcessT3169Expiry();
                }
            }
            break;
            case RLCMAC_T3195_EXPIRY :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_T3195_EXPIRY msg\n");
                int tbfPointer = 0;
                
                for (int i = sizeof(int) - 1; i >= 0; i--)
                {
                    tbfPointer |= (rlcMacMsg[i] << (i * 8));
                }
                
                DLTbf *pDlTbf = (DLTbf *)(tbfPointer);
                if ((pDlTbf) && (pDlTbf->GetTbfState() != Tbf::FREE))
                {
                    pDlTbf->rlcLink.ProcessT3195Expiry();
                }
            }
            break;
            case RLCMAC_T3191_EXPIRY :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_T3191_EXPIRY msg\n");
                int tbfPointer = 0;
                
                for (int i = sizeof(int) - 1; i >= 0; i--)
                {
                    tbfPointer |= (rlcMacMsg[i] << (i * 8));
                }
                
                DLTbf *pDlTbf = (DLTbf *)(tbfPointer);
                if ((pDlTbf) && (pDlTbf->GetTbfState() != Tbf::FREE))
                {
                    pDlTbf->rlcLink.ProcessT3191Expiry();
                }
            }
            break;
            case RLCMAC_T3193_EXPIRY :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_T3193_EXPIRY msg\n");
                int tbfPointer = 0;
                
                for (int i = sizeof(int) - 1; i >= 0; i--)
                {
                    tbfPointer |= (rlcMacMsg[i] << (i * 8));
                }
                
                DLTbf *pDlTbf = (DLTbf *)(tbfPointer);
                if ((pDlTbf) && (pDlTbf->GetTbfState() != Tbf::FREE))
                {
                    pDlTbf->rlcLink.ProcessT3193Expiry();
                }
            }
            break;
            case RLCMAC_DL_ACTIVITY_TIMER_EXPIRY :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_DL_ACTIVITY_TIMER_EXPIRY msg\n");
                int tbfPointer = 0;
                
                for (int i = sizeof(int) - 1; i >= 0; i--)
                {
                    tbfPointer |= (rlcMacMsg[i] << (i * 8));
                }
                
                DLTbf *pDlTbf = (DLTbf *)(tbfPointer);
                if ((pDlTbf) && (pDlTbf->GetTbfState() != Tbf::FREE))
                {
                    pDlTbf->rlcLink.ProcessActivityTimerExpiry();
                }
            }
            break;
            case RLCMAC_UL_ACTIVITY_TIMER_EXPIRY :
            {
                DBG_TRACE("RLC/MAC received RLCMAC_UL_ACTIVITY_TIMER_EXPIRY msg\n");
                int tbfPointer = 0;
                
                for (int i = sizeof(int) - 1; i >= 0; i--)
                {
                    tbfPointer |= (rlcMacMsg[i] << (i * 8));
                }
                
                ULTbf *pUlTbf = (ULTbf *)(tbfPointer);
                if ((pUlTbf) && (pUlTbf->GetTbfState() != Tbf::FREE))
                {
                    pUlTbf->rlcLink.ProcessActivityTimerExpiry();
                }
            }
            break;
            default :
            {
                DBG_WARNING("RlcMacTask::RlcMacLoop: Unkown message type %#x\n",
                    msgType);
            }
        }
    
    }

    DBG_LEAVE();
}













