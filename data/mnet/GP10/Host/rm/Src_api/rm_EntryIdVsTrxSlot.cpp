/*
*******************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
******************************************************************
  */

#ifndef __RM_ENTRYIDVSTRXSLOT_CPP__
#define __RM_ENTRYIDVSTRXSLOT_CPP__

#include "rm\rm_head.h"

u8 rm_EntryIdToTrxSlot(T_CNI_IRT_ID entryId, u16 *gsmHandler)
{
   u8 trx, slot;


//   if (rm_SmCtxt[entryId].pPhyChan->usable != RM_PHYCHAN_USABLE)
//   {   
//       EDEBUG__(("ERROR-rm_EntryIdToTchTrxSlot:entryId=%d related chan unusable\n",
//			entryId));
//       return RM_FALSE;
//   } else
     if (rm_SmCtxt[entryId].pPhyChan->chanType == RM_PHYCHAN_TCHF_S)
     { 
         trx  = rm_SmCtxt[entryId].pPhyChan->trxNumber;
         slot = (rm_SmCtxt[entryId].pPhyChan->chanNumberMSB)&0x07;

         if ( trx>2 || (trx==0&&slot==0) || slot>7 )
         {
              EDEBUG__(("ERROR-rm_EntryIdToTchTrxSlot:
                         entryId=%d not related to a usable chan\n", entryId));
              return RM_FALSE;
         }
         *gsmHandler = (u16)((trx<<8)|slot);
     } else
     {
         EDEBUG__(("ERROR-rm_EntryIdToTchTrxSlot: entryId=%d is not TCH/F\n",
  		        entryId));
         return RM_FALSE;
     }

     return RM_TRUE;
}

u8 rm_TrxSlotToEntryId(u16 gsmHandler, T_CNI_IRT_ID *entryId)
{
   u8 trx, slot, idx;
   
   trx  = (u8)(gsmHandler>>8);
   slot = (u8)(gsmHandler);

   //TCHF chan assumed here
   idx  = RM_TCHF_ENTRYIDX(trx, slot);
   *entryId = rm_TchfEntryId[idx];

   if (*entryId==0)
       printf("{\nRM WARNING!!!\n: failed to get entryId for (trx=%d,slot=%d,gsmhandler=%x,index=%d)\n}\n",
                trx,slot,gsmHandler,idx);

   return RM_TRUE;
}

u8 rm_CodecMatch(u16 gsmHandler, u8 codec)
{
   u8 spchVer;

   T_CNI_IRT_ID entryId;

   rm_TrxSlotToEntryId(gsmHandler, &entryId);
 
   if (rm_SmCtxt[entryId].smType != RM_MAIN_SM)
   {
       //Speech frame is always associated main SM
       EDEBUG__(("ERROR@rm_CodecMatch: entryId '%d' is not assoc with main SM\n",
                  entryId));
       return RM_FALSE;
   }

   //Convert codec to internal speech codec value
   codec = 0xF0&codec;
   spchVer = 0xff;

   if (codec == 0xD0)
       spchVer = 1; 
   else if (codec == 0xC0)
       spchVer = 0x21;
   else
   {
       //Speech frame is always associated main SM
       EDEBUG__(("ERROR@rm_CodecMatch: invalid codec value '%d'\n", codec));
       return RM_FALSE;
   }
  
   if (rm_SmCtxt[entryId].codAlgo != spchVer)
   {
       IDEBUG__(("WARNING@rm_CodecMatch: codec mismatch (dsp %x, rm %d) (entryId %d, port %x)\n",
                  codec, rm_SmCtxt[entryId].codAlgo, entryId, gsmHandler));
       return RM_FALSE;
   }

   return RM_TRUE;
}

//Check if given trx blocked
u8 rm_TrxIsNotBlocked(u8 trx)
{
   return ((u8)(OAMrm_TRX_AMSTATE(trx)));
}


T_CNI_LAPDM_OID rm_GetOid (T_CNI_IRT_ID entryId, T_CNI_LAPDM_SAPI sapi)
{
   PDEBUG__(("INFO@rm_GetOid: entryId %d, sapi %d, mEntryId %d, aEntryId %d, chanType %d\n",
              entryId, sapi, rm_SmCtxt[entryId].mEntryId, rm_SmCtxt[entryId].aEntryId,
              rm_SmCtxt[entryId].chanType));
          
   switch(sapi)
   {
   case RM_SAPI0:
        if ( entryId==rm_SmCtxt[entryId].mEntryId )
             return (rm_SmCtxt[entryId].mOId);
        break;

   case RM_SAPI3:
        if (entryId==rm_SmCtxt[entryId].mEntryId)
        {
            if (rm_SmCtxt[entryId].chanType==RM_CTFACCH_F)
                return (rm_SmCtxt[entryId].aOId);
            else if (rm_SmCtxt[entryId].chanType==RM_PHYCHAN_SDCCH4)
                return (rm_SmCtxt[entryId].mOId);
            else 
                return CNI_LAPDM_NULL_OID;
        }
        break;
    }

    return CNI_LAPDM_NULL_OID;
}

T_CNI_IRT_ID rm_GetSacchEntryId (T_CNI_IRT_ID entryId)
{
    if (entryId==rm_SmCtxt[entryId].mEntryId)
       return (rm_SmCtxt[entryId].aEntryId);
    else
       return CNI_IRT_NULL_ID;
}

int rm_IsChanAvailable(T_CNI_RIL3_CHANNEL_TYPE chan)
{
     switch(chan)
     {
     case CNI_RIL3_TCH_ACCH_FULL:
          if (RM_SMCTXT_NULL==rm_PreAllocSmCtxt(RM_PHYCHAN_TCHF_S))
             return RM_FALSE;
          break;

     case CNI_RIL3_SDCCH_ACCH_4orCBCH:
          if (RM_SMCTXT_NULL==rm_PreAllocSmCtxt(RM_PHYCHAN_SDCCH4))
             return RM_FALSE;
          break;

     case CNI_RIL3_SDCCH_ACCH_8orCBCH:
     default:
          IDEBUG__(("WARNING@rm_IsChanAvailable: chanType requested %d\n",
                     chan));
          return RM_FALSE;
     }

     return RM_TRUE;
}

rm_SmCtxt_t *rm_PreAllocSmCtxt(u8 chanType)
{
     int i;

     switch (chanType)
     {
     case RM_PHYCHAN_TCHF_S:

          for (i=0; i<OAMrm_MAX_TCHFS; i++)
          {
               //Check if the concerned radio TS is blocked/shutting down
               if ( rm_PhyChanTchf[i].amState == unlocked         &&
                    rm_PhyChanTchf[i].opState == opStateEnabled   &&
	              rm_PhyChanTchf[i].usable == RM_PHYCHAN_USABLE &&
	              rm_PhyChanTchf[i].state  == RM_PHYCHAN_FREE    )
                    break;
          } 

          if ( i >= OAMrm_MAX_TCHFS )
               return RM_SMCTXT_NULL;
          else
               return rm_PhyChanTchf[i].pMSmCtxt;

          break;
 
     case RM_PHYCHAN_SDCCH4:

          for (i=0; i<OAMrm_MAX_SDCCH4S; i++)
          {
               if ( rm_PhyChanSdcch4[i].amState == unlocked          &&
                    rm_PhyChanSdcch4[i].opState == opStateEnabled    &&
                    rm_PhyChanSdcch4[i].usable  == RM_PHYCHAN_USABLE &&
	              rm_PhyChanSdcch4[i].state   == RM_PHYCHAN_FREE   )
                    break;
          }

          if ( i >= OAMrm_MAX_SDCCH4S )
               return RM_SMCTXT_NULL;
          else
               return rm_PhyChanSdcch4[i].pMSmCtxt;
            
          break;

      default:
	 
          EDEBUG__(( "ERROR-rm_PreAllocSmCtxt: unsupported chan type:%d\n",
		          chanType ));

          return RM_SMCTXT_NULL;

	    break;
           
      }
}


short rm_DecideCipherAlgo( 
      T_CNI_RIL3_IE_CIPHER_MODE_SETTING *setting,
      T_CNI_RIL3_IE_MS_CLASSMARK_2 *classmark2)
{
     short algo = 0;
     setting->ciphering = CNI_RIl3_CIPHER_START_CIPHERING;

	 PDEBUG__(("INFO@rm_DecideCipherAlgodspCiphCap(%x)\n", rm_DspCiphCap));

     if ( !classmark2->a51 && RM_DSPA51(rm_DspCiphCap) )
     {
          algo = 1;
          setting->algorithm = CNI_RIL3_CIPHER_ALGORITHM_A51;
     } else if (classmark2->a52 && RM_DSPA52(rm_DspCiphCap) )
     {
          algo = 2;
          setting->algorithm = CNI_RIL3_CIPHER_ALGORITHM_A52;
     } else
     {
          algo = 0;
          setting->ciphering =CNI_RIl3_CIPHER_NO_CIPHERING; 
     }

     return (algo);
}

BOOL rm_bssgp_msg_callback(BSSGP_API_MSG msg)
{
   IntraL3Msg_t msgOut;

   if (msg.msg_type != BSSGP_API_MSG_PAGING_PS)
   {
       IDEBUG__(("WARNING@rm_bssgp_msg_callback: unexpected msgType(%d)\n", msg.msg_type));
       return false;
   }

   msgOut.module_id = MODULE_BSSGP;
   msgOut.primitive_type = INTRA_L3_RR_EST_REQ;
   msgOut.message_type = INTRA_L3_RR_PAGE_REQ;
   msgOut.entry_id = CNI_IRT_NULL_ID;

   msgOut.l3_data.pageReq.imsi.numDigits = msg.msg.paging_ps.imsi.numDigits;
   memcpy(msgOut.l3_data.pageReq.imsi.digits, msg.msg.paging_ps.imsi.digits,
	    msg.msg.paging_ps.imsi.numDigits);

   //send the message. 
   if (ERROR == msgQSend( rm_MsgQId, 
 			       (char *) &msgOut, 
			        sizeof(struct  IntraL3Msg_t), 
			        NO_WAIT,
			        MSG_PRI_NORMAL
					    ))
   {
	 EDEBUG__(("ERROR@ rm_bssgp_msg_callback: msgQSend (QID=%d) errno(%d)\n ",
                 (int)rm_MsgQId, errno));
       assert(0);
   } else
   {
       PDEBUG__(("TRACE@rm_bssgp_msg_callback: (prim=%d) (msgType=%d)\n", 
                  msgOut.primitive_type, msgOut.message_type));
   }

   return true;
}

#endif //__RM_ENTRYIDVSTRXSLOT_CPP__
