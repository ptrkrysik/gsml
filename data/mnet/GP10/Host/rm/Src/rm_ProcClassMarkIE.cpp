/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __RM_PROCCLASSMARKIE_CPP__
#define __RM_PROCCLASSMARKIE_CPP__

#include "rm\rm_head.h"

void rm_ProcClassMarkIE(void)
{
     rm_EdRet_t		umEdRet;
     T_CNI_RIL3MM_MSG   mmMsg;
     T_CNI_RIL3RRM_MSG  rmMsg;

     RDEBUG__(("ENTER--rm_ProcClassMarkIE: entryID=%d\n",
		rm_ItcRxEntryId));

     //Decode messages in terms of msgType of l3_data

     switch (rm_pItcRxMdMsg->l3_data.buffer[0] & 0x0F)
     {
     case RM_PD:
          //Decode Paging Response message
          //umEdRet = CNI_RIL3RRM_Decode( 
	    //			&(rm_pItcRxMdMsg->l3_data),
          //                      &rmMsg
	    //		   	    );
          //switch (umEdRet)
          //{
          //case RM_UMED_SUCCESS:
	    //     break;

          //default:
               //Um-decoding failed
               //EDEBUG__(("ERROR-rm_ProcClassMarkIE: Um decode err:%d\n", umEdRet));
	    //     break;
          //}

          break;          	            

     case MM_PD:
          //umEdRet = CNI_RIL3MM_Decode( 
	    //			&(rm_pItcRxMdMsg->l3_data),
          //                      &mmMsg
	    //		   	    );
          //switch (umEdRet)
          //{
          //case RM_UMED_SUCCESS:
	         //rm_UmMmMsgTestPrint(&mmMsg);
          //     break;

          //default:
               //Um-decoding failed
               //EDEBUG__(("ERROR-rm_ProcClassMarkIE: Um decode err:%d\n",
		   //	  umEdRet));
	    //     break;
          //}

          break;

     default:
	    EDEBUG__(("ERROR-rm_ProcClassMarkIE: errno=%d\n", errno));
          break;
     }
} //rm_ProcClassMarkIE()

#endif //__RM_PROCCLASSMARKIE_CPP__
