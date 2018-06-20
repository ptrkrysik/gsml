/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_SENDDSPMSG_CPP__
#define __GRR_SENDDSPMSG_CPP__

#include "grr\grr_head.h"

void grr_SendDspSynInf(unsigned char trx)
{
   int  len;
   unsigned char buf[GRR_MAX_L1MSG_LEN];

   DBG_FUNC("grr_SendDspSynInf", GRR_LAYER_DSP);
   DBG_ENTER();

   DBG_TRACE("grr_SendDspSynInf: for trx(%d)\n", trx);


   //Fill Synchronization Information
   len = 4;
   buf[len++] = GRR_L1MG_TRXMGMT;
   buf[len++] = GRR_L1MT_SYNCH_MSB;
   buf[len++] = GRR_L1MT_SYNCH_LSB;
   buf[len++] = trx;
   buf[len++] = 0; 
   buf[len++] = 0; 
   buf[len++] = 0; 
   buf[len++] = 0; 

   //store len in little endian
   buf[0] = (unsigned char)len;
   buf[1] = (unsigned char)(len>>8);
   buf[2] = (unsigned char)(len>>16);
   buf[3] = (unsigned char)(len>>24);

   //dump the message for debug if needed
   if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

   sendDsp(buf, len);

   DBG_LEAVE();
}

void grr_SendDspArfcn(unsigned char trx, int swap)
{
   int  len;
   unsigned char buf[GRR_MAX_L1MSG_LEN];
   unsigned short arfcn;

   DBG_FUNC("grr_SendDspArfcn", GRR_LAYER_DSP);
   DBG_ENTER();
   
   //apply sanitary check
   if (trx>=OAMgrr_MAX_TRXS)
   {
       DBG_WARNING("grr_SendDspArfcn: invalid trx(%d)\n", trx);
       DBG_LEAVE();
       return;
   }  

   switch(trx)
   {
   case 0:
        arfcn = OAMgrr_ARFCN(0);
        break;
   case 1:
        arfcn = OAMgrr_ARFCN(1);
        break;
   }
   
   DBG_TRACE("grr_SendDspArfcn: send arfn(%d) to trx(%d) swap(%d)\n",
              arfcn, trx, swap);

   //Send arfcn via TUNE SYNTH message
   len = 4;
   buf[len++] = GRR_L1MG_TRXMGMT;
   buf[len++] = GRR_L1MT_ARFCN_MSB;
   buf[len++] = GRR_L1MT_ARFCN_LSB;
   buf[len++] = trx;
   buf[len++] = (unsigned char)(arfcn>>8);
   buf[len++] = (unsigned char)(arfcn);
   //store len in little endian
   buf[0] = (unsigned char)len;
   buf[1] = (unsigned char)(len>>8);
   buf[2] = (unsigned char)(len>>16);
   buf[3] = (unsigned char)(len>>24);

   //dump the message for debug if needed
   if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

   sendDsp(buf, len);

   DBG_LEAVE();
}

//Send ping message to dsp
void grr_SendDspPing(int dsp)
{
   int  len;
   unsigned char buf[GRR_MAX_L1MSG_LEN];

   DBG_FUNC("grr_SendDspPing", GRR_LAYER_DSP);
   DBG_ENTER();

   //Form ping message
   len = 4;
   buf[len++] = GRR_L1MG_TRXMGMT;
   buf[len++] = GRR_L1MT_PING_MSB;
   buf[len++] = GRR_L1MT_PING_LSB;
   buf[len++] = (unsigned char)dsp;

   //store len in little endian
   buf[0] = (unsigned char)len;
   buf[1] = (unsigned char)(len>>8);
   buf[2] = (unsigned char)(len>>16);
   buf[3] = (unsigned char)(len>>24);

   //dump the message for debug if needed
   if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

   sendDsp(buf, len);

   DBG_LEAVE();
}

void grr_SendDspSlotActivate(unsigned char trx, unsigned char slot)
{
   int len;
   unsigned char buf[GRR_MAX_L1MSG_LEN];

   DBG_FUNC("grr_SendDspSlotActivate", GRR_LAYER_DSP);
   DBG_ENTER();

   //ATTN:
   //slot's bit 8-5 i.e. 5 MSB bits must be masked by caller of this
   //function. Caller is also responsibile for validity of SLOT COMB
   //value associated with this slot. This function just passes COMB
   //to DSP without guard check of the input values' validity.
	
   len = 4;
   buf[len++] = GRR_L1MG_TRXMGMT;		//0x10
   buf[len++] = GRR_L1MT_SLOTACTIV_MSB;	//0x17
   buf[len++] = GRR_L1MT_SLOTACTIV_LSB;	//0x00
   buf[len++] = trx;
   buf[len++] = slot;
   buf[len++] = OAMgrr_TRX_SLOT_COMB(trx,slot);

   if ( (OAMgrr_TRX_SLOT_COMB(trx,slot) != OAMgrr_SLOT_COMB_1) &&
        (OAMgrr_TRX_SLOT_COMB(trx,slot) != OAMgrr_SLOT_COMB_5) &&
        (OAMgrr_TRX_SLOT_COMB(trx,slot) != OAMgrr_SLOT_COMB_13) )
   {
         DBG_ERROR("grr_SendDspSlotActivate: invalid config (%d) for (trx=%d,slot=%d)\n",
			        OAMgrr_TRX_SLOT_COMB(trx,slot), trx, slot);
         printf("grr_SendDspSlotActivate: invalid config (%d) for (trx=%d,slot=%d)\n",
			        OAMgrr_TRX_SLOT_COMB(trx,slot), trx, slot);

		 assert(0);
   }	 

   //store len in little endian
   buf[0] = (unsigned char) len;
   buf[1] = (unsigned char) (len>>8);
   buf[2] = (unsigned char) (len>>16);
   buf[3] = (unsigned char) (len>>24);

   //dump the message for debug if needed
   if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

   sendDsp(buf, len);

   DBG_LEAVE();
}

void grr_SendDspTrxConfig(unsigned char trx)
{
   int  len;
   unsigned char buf[GRR_MAX_L1MSG_LEN];

   DBG_FUNC("grr_SendDspTrxConfig", GRR_LAYER_DSP);
   DBG_ENTER();
  
   //skip the len of 4 bytes in the beginning
   len = 4;
   buf[len++] = GRR_L1MG_TRXMGMT;	     //0x10
   buf[len++] = GRR_L1MT_TRXCONFIG_MSB;  //0x15
   buf[len++] = GRR_L1MT_TRXCONFIG_LSB;  //0x00
   buf[len++] = trx;                  
   buf[len++] = OAMgrr_AIRINTERFACE;
   buf[len++] = OAMgrr_POCKETBTS_BSIC;
   buf[len++] = OAMgrr_NETWORKIFCONFIG;  //0x00

   //skip unused fields
   len += 14;
   buf[len++] = (unsigned char) OAMgrr_IQ_SWAP;

   //store len in little endian
   buf[0] = (unsigned char) len;
   buf[1] = (unsigned char) (len>>8);
   buf[2] = (unsigned char) (len>>16);
   buf[3] = (unsigned char) (len>>24);

   //dump the message for debug if needed
   if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

   sendDsp(buf, len);

   DBG_LEAVE();
}

void grr_SendDspTuneSynth( unsigned char phTrx, unsigned char synth, unsigned char hopping,
                           unsigned char state, unsigned short arfcn) 
{
   int  len;
   unsigned char buf[GRR_MAX_L1MSG_LEN];
   unsigned short setting;

   DBG_FUNC("grr_SendDspTuneSynth", GRR_LAYER_DSP);
   DBG_ENTER();

   DBG_TRACE("grr_SendDspTuneSynth: swapInd=%d, hopping=%d, tuneSetting:(%d,%d,%d,%d)\n",
              OAMgrr_BCCH_CARRIER_phTRX, hopping, phTrx, synth, state, arfcn);

   //Fill TUNE SYNTH message's header portion
   len = 4;
   buf[len++] = GRR_L1MG_TRXMGMT;
   buf[len++] = GRR_L1MT_TUNESYNTH_MSB;	
   buf[len++] = GRR_L1MT_TUNESYNTH_LSB;	
   buf[len++] = 0;   //Always sent to BCCH DSP
   
   setting = 0;
   setting = ( OAMgrr_phTRX_SELECT(phTrx)    | OAMgrr_phTRX_SYNTH_SELECT(synth) |
               OAMgrr_phTRX_HOPPING(hopping) | OAMgrr_phTRX_PWR_SELECT(state)   |
               (0x3FF&arfcn) );

   buf[len++] = (unsigned char)(setting>>8);
   buf[len++] = (unsigned char)(setting);
  
   //store len in little endian
   buf[0] = (unsigned char)len;
   buf[1] = (unsigned char)(len>>8);
   buf[2] = (unsigned char)(len>>16);
   buf[3] = (unsigned char)(len>>24);

   //dump the message for debug if needed
   if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

   sendDsp(buf, len);

   DBG_LEAVE();
}

#endif //__GRR_SENDDSPMSG_CPP__
