/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
*/

#ifndef __RM_S1RMRUNNING_CPP__
#define __RM_S1RMRUNNING_CPP__

#include "rm\rm_head.h"

void rm_S1RmRunning( void )
{
   //Monitoring entrance to a function
   RDEBUG__(("ENTER@rm_S1RmRunning\n"));
   switch(rm_ItcRxOrigin)
   {
   case MODULE_L1:
        // Got a message from L1, process it if it is for CBCH
        if (rm_pItcRxL1Msg->l3_data.buffer[0]==RM_L1MG_DEDMGMT)
        {
	      // Check if having got Ack to the Channel Activation sent earlier
            if (rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_CHANACTIVACK_MSB &&
                rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_CHANACTIVACK_LSB )
            {
                //Got a positive ack to the Channel Activation

                //SMS-SAPI3-Begin
                //Call LAPDm local release to clean up the channel for safety only
                //rm_SendDlRelReq(rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);

                rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI0,RM_LOCAL_RELEASE);
                rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI3,RM_LOCAL_RELEASE);
                rm_SendDlRelReq(rm_pSmCtxt->aOId,RM_SAPI3,RM_LOCAL_RELEASE);

                //SMS-SAPI3-End

                //Further check if this Ack is expected 
                if (rm_pItcRxL1Msg->l3_data.buffer[3]!=rm_pCbchChan->trxNumber     ||
                    rm_pItcRxL1Msg->l3_data.buffer[4]!=rm_pCbchChan->chanNumberMSB ||
                    rm_pItcRxL1Msg->l3_data.buffer[5]!=rm_pCbchChan->chanNumberLSB )
                {
		        //IDEBUG__(("WARNING@rm_S1RmRunning: ack to unexpect chan (%d,%d,%d)\n",
			  //	       rm_pItcRxL1Msg->l3_data.buffer[3],
			  //		 rm_pItcRxL1Msg->l3_data.buffer[4],
			  //		 rm_pItcRxL1Msg->l3_data.buffer[5]));

                    return;
                }

                //It is an expected ack msg
                if (rm_pCbchChan->usable==RM_PHYCHAN_CBCH_USABLE)
                {
                    rm_BroadcastNilCbMsg();
                    rm_ChanCbchActive = RM_TRUE;
                    //GP2
                    //rm_SendSystemInfo4ToBcchTrx(0, RM_L1SYSINFO_TYPE_4,OAMrm_RA_CELL_BARRED_STATE); 
                }
            }
            // Check if having got NAck to the Channel Activation msg sent earlier
	      else if ( rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_CHANACTIVNACK_MSB &&
	                rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_CHANACTIVNACK_LSB )
	      {
		    //Got a negative Ack to Channel Activation sent out eariler

                //SMS-SAPI3-Begin

                //Call LAPDm local release to clean up the channel for safety only
                //rm_SendDlRelReq(rm_pSmCtxt->mOId,rm_pSmCtxt->sapi,RM_LOCAL_RELEASE);

                rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI0,RM_LOCAL_RELEASE);
                rm_SendDlRelReq(rm_pSmCtxt->mOId,RM_SAPI3,RM_LOCAL_RELEASE);
                rm_SendDlRelReq(rm_pSmCtxt->aOId,RM_SAPI3,RM_LOCAL_RELEASE);

                //SMS-SAPI3-End

                //Further check if this NAck is expected 
                if (rm_pItcRxL1Msg->l3_data.buffer[3]!=rm_pCbchChan->trxNumber     ||
                    rm_pItcRxL1Msg->l3_data.buffer[4]!=rm_pCbchChan->chanNumberMSB ||
                    rm_pItcRxL1Msg->l3_data.buffer[5]!=rm_pCbchChan->chanNumberLSB )
                {
		        IDEBUG__(("WARNING@rm_S1RmRunning: nack to unexpect chan (%d,%d,%d)\n",
				       rm_pItcRxL1Msg->l3_data.buffer[3],
					 rm_pItcRxL1Msg->l3_data.buffer[4],
					 rm_pItcRxL1Msg->l3_data.buffer[5]));
                    return;
                }

                //It is an expected nack msg
                IDEBUG__(("WARNING@rm_S1RmRunning: got expected nack (%d,%d,%d)\n",
	  		         rm_pItcRxL1Msg->l3_data.buffer[3],
				   rm_pItcRxL1Msg->l3_data.buffer[4],
				   rm_pItcRxL1Msg->l3_data.buffer[5]));
	     }
           // Check if got RF channel release acknowledgment
           else if(rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_RFCHANRELACK_MSB &&
                   rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_RFCHANRELACK_LSB  )
	     {
	          // Further check if the RF channel release coming for CBCH channel
                if (rm_pItcRxL1Msg->l3_data.buffer[3]!=rm_pCbchChan->trxNumber     ||
                    rm_pItcRxL1Msg->l3_data.buffer[4]!=rm_pCbchChan->chanNumberMSB ||
                    rm_pItcRxL1Msg->l3_data.buffer[5]!=rm_pCbchChan->chanNumberLSB )
                {
		        //IDEBUG__(("WARNING@rm_S1RmRunning: rf ack to unexpect chan(%d,%d,%d)\n",
			  //	       rm_pItcRxL1Msg->l3_data.buffer[3],
			  //		 rm_pItcRxL1Msg->l3_data.buffer[4],
			  //		 rm_pItcRxL1Msg->l3_data.buffer[5]));
                    return;
                }

                // Got expected RF chan release ack, return CBCH to SDCCH for normal use
                if (rm_pCbchChan->usable != RM_PHYCHAN_UNUSABLE)
                    rm_pCbchChan->usable = RM_PHYCHAN_USABLE;
           }
           // Check if got CBCH ready to send message
           else if(rm_pItcRxL1Msg->l3_data.buffer[1]==RM_L1MT_CBCHRTS_MSB &&
                   rm_pItcRxL1Msg->l3_data.buffer[2]==RM_L1MT_CBCHRTS_LSB  )
           {
 	          // Further check if the RTS message is coming for CBCH channel
                if (rm_pItcRxL1Msg->l3_data.buffer[3]!=rm_pCbchChan->trxNumber      ||
                    rm_pItcRxL1Msg->l3_data.buffer[4]!=rm_pCbchChan->chanNumberMSB  ||
                    rm_pItcRxL1Msg->l3_data.buffer[5]!=rm_pCbchChan->chanNumberLSB  )

                    //rm_pItcRxL1Msg->l3_data.buffer[4]!=RM_L1CBCH_CHANNUMBER_MSB  ||
                    //rm_pItcRxL1Msg->l3_data.buffer[5]!=RM_L1CBCH_CHANNUMBER_MSB  )
                {
		        IDEBUG__(("WARNING@rm_S1RmRunning: rts not for cbch(%d,%d,%d)\n",
				       rm_pItcRxL1Msg->l3_data.buffer[3],
					 rm_pItcRxL1Msg->l3_data.buffer[4],
					 rm_pItcRxL1Msg->l3_data.buffer[5]));
                    return;
                }
		   //PR 1323 Shedule message broadcast
               if ( !OAMrm_CBCH_OPSTATE )
               {
                     IDEBUG__(("CBC is not turn on\n"));
                     return;
               }
		  
                //Really got an RTS for CBCH channel, pass down another page if there is
                //if (rm_SmscbcMsg.morPage && rm_SmscbcMsg.totPage>1)
                //{
                //    rm_BroadcastNewCbMsg(rm_SmscbcMsg.curPage);
                //    if ((++rm_SmscbcMsg.curPage)>rm_SmscbcMsg.totPage)
                //         rm_SmscbcMsg.curPage = 1;        
                //} else
                //   PDEBUG__(("INFO@rm_S1RmRunning: useless cbch RTS. more=%d,tPage=%d\n",
                //              rm_SmscbcMsg.morPage, rm_SmscbcMsg.totPage));
           	    //}
		  
			  u8 BroadCast_over = 0;
                    u8 oldCurPage = rm_CbMsgTable.curMsg;
			  do{
                        u8 totPage = rm_CbMsgTable.table[rm_CbMsgTable.curMsg].totPage;

				if( rm_CbMsgTable.table[rm_CbMsgTable.curMsg].state == 1 ) //active
				{
					if( rm_CbMsgTable.table[rm_CbMsgTable.curMsg].curPageInCurMsg < totPage )//more page
					{	
						rm_BroadcastNewCbMsg( rm_CbMsgTable.curMsg );
						rm_CbMsgTable.table[rm_CbMsgTable.curMsg].curPageInCurMsg++;
						BroadCast_over = 1;
					}
					//one page or last page
					else if( totPage != 0 && 
						rm_CbMsgTable.table[rm_CbMsgTable.curMsg].curPageInCurMsg == totPage)
					{	
						rm_BroadcastNewCbMsg( rm_CbMsgTable.curMsg );
						BroadCast_over = 1;
						rm_CbMsgTable.table[rm_CbMsgTable.curMsg].curPageInCurMsg = 1;
						++rm_CbMsgTable.curMsg;
						if(rm_CbMsgTable.curMsg == SmsCbc_TableRow_MAX)
							 rm_CbMsgTable.curMsg = 0;
					}

					else if(totPage == 0 )//null message
					{
 					     rm_CbMsgTable.table[rm_CbMsgTable.curMsg].state = -1;
					     ++rm_CbMsgTable.curMsg;
					     if(rm_CbMsgTable.curMsg == SmsCbc_TableRow_MAX)
					        rm_CbMsgTable.curMsg = 0;
					}
		   		}
				else//inactive
				{
					++rm_CbMsgTable.curMsg;
					if(rm_CbMsgTable.curMsg == SmsCbc_TableRow_MAX)
						rm_CbMsgTable.curMsg = 0;
				}

                        if (rm_CbMsgTable.curMsg==oldCurPage && BroadCast_over ==0)
                        {
                             rm_BroadcastNilCbMsg();
                             BroadCast_over = 1;
                        }
			}while(BroadCast_over == 0);
		   } 
		//PR 1323 End

           // Got a message of unexpected message type landed at this SM state
	     else
		    IDEBUG__(("WARNING@rm_S2ActivatingSigTrafficChan: unexpect L1 MT(%d,%d,%d)\n",
		  	        rm_pItcRxL1Msg->l3_data.buffer[0],
			        rm_pItcRxL1Msg->l3_data.buffer[1],
			        rm_pItcRxL1Msg->l3_data.buffer[2]));
	  } 
	
        // Got a message of unexpected message group landed at this SM state
        else
	     IDEBUG__(("WARNING@rm_S1RmRunning: unexpect L1 MG(%d,%d,%d)\n",
	     	          rm_pItcRxL1Msg->l3_data.buffer[0],
			    rm_pItcRxL1Msg->l3_data.buffer[1],
			    rm_pItcRxL1Msg->l3_data.buffer[2]));
	  break;

   default:
        //SMS-SAPI3-Begin

	  //Unexpected message orgination
        //IDEBUG__(("WARNING@rm_S1RmRunning: wrong msg origination:%d\n",
	  //	       rm_ItcRxOrigin));

        //SMS-SAPI3-End

	  break;

   } //End of switch()
}

#endif //__RM_S1RMRUNNING_CPP__
