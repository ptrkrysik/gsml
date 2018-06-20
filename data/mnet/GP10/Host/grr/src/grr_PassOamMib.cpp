/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/

#ifndef __GRR_PASSOAMMIB_CPP__
#define __GRR_PASSOAMMIB_CPP__

#include "grr\grr_head.h"

void grr_PassOamDspBtsPackage(unsigned char trx)
{
   int  len;
   unsigned char buf[GRR_MAX_L1MSG_LEN];

   DBG_FUNC("grr_PassOamDspBtsPackage", GRR_LAYER_OAM);
   DBG_ENTER();
  
   //skip the len of 4 bytes in the beginning
   len = 4;
   buf[len++] = GRR_L1MG_OAMMGMT; 
   buf[len++] = GRR_L1MT_BTSPACKAGE_MSB;
   buf[len++] = GRR_L1MT_BTSPACKAGE_LSB;
   buf[len++] = trx;

   //Parameters contained in this message
   buf[len++] = OAMgrr_RADIO_LINK_TIMEOUT;
   buf[len++] = OAMgrr_BS_PA_MFRMS;

   buf[len] = 0;
   if ( ((int)OAMgrr_MS_UPLINK_DTX_STATE==1) ||
        ((int)OAMgrr_MS_UPLINK_DTX_STATE==0)  ) 
        buf[len] |= 1;       //DTX ul ON

   if ( trx && OAMgrr_MS_DNLINK_DTX_STATE)
        buf[len] |= 2;       //DTX dl ON

   len++;

   //store len in little endian
   buf[0] = (unsigned char) len;
   buf[1] = (unsigned char) (len>>8);
   buf[2] = (unsigned char) (len>>16);
   buf[3] = (unsigned char) (len>>24);

   //dump the message for debugging if needed
   if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

   sendDsp(buf, len);

   DBG_LEAVE();
}

void grr_PassOamDspPwrCtrl(unsigned char trx)
{
  int len;
  unsigned char buf[GRR_MAX_L1MSG_LEN];

  DBG_FUNC("grr_PassOamDspPwrCtrl", GRR_LAYER_OAM);
  DBG_ENTER();

  PowerControlPackage_t *pcPktPtr = &grr_OamData.powerControlPackage;
     
  len = 4;
	
  buf[len++] = GRR_L1MG_OAMMGMT; 
  buf[len++] = GRR_L1MT_DPCCFG_MSB;
  buf[len++] = GRR_L1MT_DPCCFG_LSB;
  buf[len++] = trx;

  //the size for the signal sample buffer used for averaging power
  //level[1-32]. one sample at every 480 ms (1 frame) (Hreqave)
  buf[len++] = pcPktPtr->pcAveragingLev->hreqave;
	
  //the size for the signal sample buffer used for averaging quality
  //samples [1-32]. one sample at every 480 ms (1 frame) (hreqave)
  buf[len++] = pcPktPtr->pcAveragingQual->hreqave;
	
  //Threshold for activating uplink dynamic power control
  buf[len++] =  pcPktPtr->pcLowerThresholdLevParam->rxLevelUL;

  //store length in little endian
  buf[0] = (unsigned char) len;
  buf[1] = (unsigned char) (len>>8);
  buf[2] = (unsigned char) (len>>16);
  buf[3] = (unsigned char) (len>>24);

  //dump the message for debugging if needed
  if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );
    
  sendDsp(buf, len);

  DBG_LEAVE();
    
} 

void grr_PassOamDspHoServCell(unsigned char trx)
{
  int len, tmp;
  unsigned char buf[GRR_MAX_L1MSG_LEN];

  DBG_FUNC("grr_PassOamDspHoServCell", GRR_LAYER_OAM);
  DBG_ENTER();

  HandoverControlPackage_t *hoPktPtr = &grr_OamData.handoverControlPackage;
    
  len = 4;
	
  //Msg discriminator and type
  buf[len++] = GRR_L1MG_OAMMGMT;           
  buf[len++] = GRR_L1MT_HOSCELLCFG_LSB;
  buf[len++] = GRR_L1MT_HOSCELLCFG_LSB;

  buf[len++] = trx;          

  //Power parameters
  //
	
  //Rxlev threshold on the uplink for handover process to commence (-103 ~ -73)
  tmp = hoPktPtr->hoThresholdLevParam->rxLevelUL;
  buf[len++] = (unsigned char) tmp; 
	
  //Rxlev threshold on the downlink for handover process to commence (-103 ~ -73)	
  tmp = hoPktPtr->hoThresholdLevParam->rxLevelDL;
  buf[len++] = (unsigned char) tmp; 
	
  //The number of averages (out of total averages) that have to be upper or lower
  //than the threshold, before making a handover decision (P5).
  buf[len++] = hoPktPtr->hoThresholdLevParam->px;

  //The number of averages that have to be taken into account, when making a
  //handover decision (N5).
  buf[len++] = hoPktPtr->hoThresholdLevParam->nx;

  //Quality parameters
  //

  //uplink
  tmp = hoPktPtr->hoThresholdQualParam->rxQualUL;
  buf[len++] = (unsigned char) tmp; 
	
  //downlink
  tmp = hoPktPtr->hoThresholdQualParam->rxQualDL;
  buf[len++] = (unsigned char) tmp; 
	
  buf[len++] = (unsigned char) hoPktPtr->hoThresholdQualParam->px;
  buf[len++] = (unsigned char) hoPktPtr->hoThresholdQualParam->nx;

  //Handover Margin Default
  buf[len++] = (unsigned char) hoPktPtr->hoMarginDef;

  //Maximum mobile transmission power default
  buf[len++] = (unsigned char) hoPktPtr->mxTxPwrMaxCellDef;

  //Minimum mobile receive power default
  buf[len++] = (unsigned char) hoPktPtr->rxLevMinCellDef;

  //store length in little endian
  buf[0] = (unsigned char) len;
  buf[1] = (unsigned char) (len>>8);
  buf[2] = (unsigned char) (len>>16);
  buf[3] = (unsigned char) (len>>24);

  //dump the message for debugging if needed
  if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );
    
  sendDsp(buf, len);

  DBG_LEAVE();
}

void grr_PassOamDspHoNeighCell(unsigned char trx, unsigned char cell, unsigned char nocells)
{
  int len, tmp;
  unsigned char buf[GRR_MAX_L1MSG_LEN];

  DBG_FUNC("grr_PassOamDspHoNeighCell", GRR_LAYER_OAM);
  DBG_ENTER();
 
  if (OAMgrr_HO_ADJCELL_ID_t(cell)==OAMgrr_HO_ADJCELL_ID_NULL) {DBG_LEAVE(); return;}

  //point to the correct index;
  AdjCell_HandoverEntry *entry 
          = (AdjCell_HandoverEntry *) grr_OamData.adjacentCellPackage_t.adjCell_handoverTable;
  entry  += cell;
   
  len = 4;
	
  //Encode DSP message
  buf[len++] = GRR_L1MG_OAMMGMT;
  buf[len++] = GRR_L1MT_HONCELLCFG_MSB;
  buf[len++] = GRR_L1MT_HONCELLCFG_LSB;
  buf[len++] = trx;          
  buf[len++] = cell;
  buf[len++] = nocells;
	
  //ARFCN
  tmp = entry->adjCell_bCCHFrequency;
  buf[len++] = (unsigned char) (tmp >>8); 
  buf[len++] = (unsigned char) tmp; 
	
  //BSIC
  buf[len++] = (unsigned char) ((unsigned char) entry->adjCell_ncc << 3 |
		                    (unsigned char) entry->adjCell_cid); 
  //Handover margin
  buf[len++] = (unsigned char) entry->adjCell_hoMargin;

  //Maximum Mobile Transmit Power for Neighbour Cell
  tmp =  entry->adjCell_msTxPwrMaxCell;
  buf[len++] = (unsigned char) tmp; 
 
  //Minimum Mobile receive Power for Neighbour Cell
  tmp = entry->adjCell_rxLevMinCell;
  buf[len++] = (unsigned char) tmp; 

  //store length in little endian
  buf[0] = (unsigned char) len;
  buf[1] = (unsigned char) (len>>8);
  buf[2] = (unsigned char) (len>>16);
  buf[3] = (unsigned char) (len>>24);

  //dump the message for debugging if needed
  if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

  sendDsp(buf, len);
    
  DBG_LEAVE();
} 

void grr_PassOamDspPwrRedStep(unsigned char trx)
{
  int len;
  unsigned char buf[GRR_MAX_L1MSG_LEN];

  DBG_FUNC("grr_PassOamDspPwrRedStep", GRR_LAYER_OAM);
  DBG_ENTER();

  if (trx >= OAMgrr_MAX_TRXS)
  {
      DBG_WARNING("grr_PassOamDspPwrRedStep: invalid trx(%d)\n", trx);
      return;
  }

  len = 4;
  buf[len++] = GRR_L1MG_OAMMGMT;
  buf[len++] = GRR_L1MT_TXPWMAXRDCTCFG_MSB;
  buf[len++] = GRR_L1MT_TXPWMAXRDCTCFG_LSB;
  buf[len++] = trx;          

  buf[len++] = (unsigned char)
               (grr_OamData.radioCarrierPackage[OAMgrr_TRX_RC(trx)].txPwrMaxReduction);

  //store length in little endian
  buf[0] = (unsigned char) len;
  buf[1] = (unsigned char) (len>>8);
  buf[2] = (unsigned char) (len>>16);
  buf[3] = (unsigned char) (len>>24);

  //dump the message for debugging if needed
  if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

  sendDsp(buf, len);

  DBG_LEAVE();
} 

void grr_PassOamMsSystemInformation2(
                                   unsigned char trx,
                                   unsigned char SI,
                                   T_CNI_RIL3_CELL_BARRED_ACCESS barState
                                 )
{
  int 				len;
  unsigned char 			buf[GRR_MAX_L1MSG_LEN];
  T_CNI_RIL3_RESULT       	ret;	
  T_CNI_LAPDM_L3MessageUnit   l3_data;

  DBG_FUNC("grr_PassOamMsSystemInformation2", GRR_LAYER_OAM);
  DBG_ENTER();
 
  //Zap rm_UmMsg 0-clean first
  GRR_MEMSET(&grr_Msg, sizeof(T_CNI_RIL3RRM_MSG));
        
  //Populate SI2 for use by RRM encoder
  grr_PopulateSi2(barState);
  GRR_MEMCPY(&grr_Msg, &grr_Si2, sizeof(T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE2));

  //Call RR message encoding functionality
  ret = CNI_RIL3RRM_Encode(&grr_Msg, &l3_data);

  //check the encoding result before sending the message
  switch (ret)
  {
  case CNI_RIL3_RESULT_SUCCESS:
       //Wrap the encoded SI in PH_DATA_REQ and pass to DSP
       len = 4;
       buf[len++] = GRR_L1MG_COMCHAN;
       buf[len++] = GRR_L1MT_BCCHINFO_MSB;
       buf[len++] = GRR_L1MT_BCCHINFO_LSB;
       buf[len++] = trx;
       buf[len++] = GRR_L1CH_BCCH_MSB;
       buf[len++] = GRR_L1CH_BCCH_LSB;
       buf[len++] = SI;
       buf[len++] = l3_data.msgLength;
       GRR_MEMCPY(&buf[len], l3_data.buffer, l3_data.msgLength);
       len = len + l3_data.msgLength;
       buf[len++] = 1; //Start time==immediate
       buf[len++] = 0;
       buf[len++] = 0;

       //store len in little endian
       buf[0] = (unsigned char) len;
       buf[1] = (unsigned char) (len>>8);
       buf[2] = (unsigned char) (len>>16);
       buf[3] = (unsigned char) (len>>24);

       //dump the message for debugging if needed
       if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

       sendDsp(buf, len);

       break;

   default:
       DBG_ERROR("grr_PassOamMsSystemInformation2: rrm encoder failed err(%d)\n", ret);
       printf("grr_PassOamMsSystemInformation2: rrm encoder failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_RIL3_CALL_RRM_ENCODER_FAILED);
       assert(0);
       break;
   } 

   DBG_LEAVE();
} 

void grr_PassOamMsSystemInformation3(
                                   unsigned char trx,
                                   unsigned char SI,
                                   T_CNI_RIL3_CELL_BARRED_ACCESS barState
                                 )
{
  int 				len;
  unsigned char 			buf[GRR_MAX_L1MSG_LEN];
  T_CNI_RIL3_RESULT       	ret;	
  T_CNI_LAPDM_L3MessageUnit   l3_data;

  DBG_FUNC("grr_PassOamMsSystemInformation3", GRR_LAYER_OAM);
  DBG_ENTER();
 
  //Zap rm_UmMsg 0-clean first
  GRR_MEMSET(&grr_Msg, sizeof(T_CNI_RIL3RRM_MSG));
        
  //Populate SI3 for use by RRM encoder
  grr_PopulateSi3(barState);
  GRR_MEMCPY(&grr_Msg, &grr_Si3, sizeof(T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE3));

  //Call RR message encoding functionality
  ret = CNI_RIL3RRM_Encode(&grr_Msg, &l3_data);

  //check the encoding result before sending the message
  switch (ret)
  {
  case CNI_RIL3_RESULT_SUCCESS:
       //Wrap the encoded SI in PH_DATA_REQ and pass to DSP
       len = 4;
       buf[len++] = GRR_L1MG_COMCHAN;
       buf[len++] = GRR_L1MT_BCCHINFO_MSB;
       buf[len++] = GRR_L1MT_BCCHINFO_LSB;
       buf[len++] = trx;
       buf[len++] = GRR_L1CH_BCCH_MSB;
       buf[len++] = GRR_L1CH_BCCH_LSB;
       buf[len++] = SI;
       buf[len++] = l3_data.msgLength;
       GRR_MEMCPY(&buf[len], l3_data.buffer, l3_data.msgLength);
       len = len + l3_data.msgLength;
       buf[len++] = 1; //Start time==immediate
       buf[len++] = 0;
       buf[len++] = 0;

       //store len in little endian
       buf[0] = (unsigned char) len;
       buf[1] = (unsigned char) (len>>8);
       buf[2] = (unsigned char) (len>>16);
       buf[3] = (unsigned char) (len>>24);

       //dump the message for debugging if needed
       if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

       sendDsp(buf, len);

       break;

   default:
       DBG_ERROR("grr_PassOamMsSystemInformation3: rrm encoder failed err(%d)\n", ret);
       printf("grr_PassOamMsSystemInformation3: rrm encoder failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_RIL3_CALL_RRM_ENCODER_FAILED);
       assert(0);
       break;
   } 

   DBG_LEAVE();
} 

void grr_PassOamMsSystemInformation4(
                                   unsigned char trx,
                                   unsigned char SI,
                                   T_CNI_RIL3_CELL_BARRED_ACCESS barState
                                 )
{
  int 				len;
  unsigned char 			buf[GRR_MAX_L1MSG_LEN];
  T_CNI_RIL3_RESULT       	ret;	
  T_CNI_LAPDM_L3MessageUnit   l3_data;

  DBG_FUNC("grr_PassOamMsSystemInformation4", GRR_LAYER_OAM);
  DBG_ENTER();
 
  //Zap rm_UmMsg 0-clean first
  GRR_MEMSET(&grr_Msg, sizeof(T_CNI_RIL3RRM_MSG));
        
  //Populate SI4 for use by RRM encoder
  grr_PopulateSi4(barState);
  GRR_MEMCPY(&grr_Msg, &grr_Si4, sizeof(T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE4));

  //Call RR message encoding functionality
  ret = CNI_RIL3RRM_Encode(&grr_Msg, &l3_data);

  //check the encoding result before sending the message
  switch (ret)
  {
  case CNI_RIL3_RESULT_SUCCESS:
       //Wrap the encoded SI in PH_DATA_REQ and pass to DSP
       len = 4;
       buf[len++] = GRR_L1MG_COMCHAN;
       buf[len++] = GRR_L1MT_BCCHINFO_MSB;
       buf[len++] = GRR_L1MT_BCCHINFO_LSB;
       buf[len++] = trx;
       buf[len++] = GRR_L1CH_BCCH_MSB;
       buf[len++] = GRR_L1CH_BCCH_LSB;
       buf[len++] = SI;
       buf[len++] = l3_data.msgLength;
       GRR_MEMCPY(&buf[len], l3_data.buffer, l3_data.msgLength);
       len = len + l3_data.msgLength;
       buf[len++] = 1; //Start time==immediate
       buf[len++] = 0;
       buf[len++] = 0;

       //store len in little endian
       buf[0] = (unsigned char) len;
       buf[1] = (unsigned char) (len>>8);
       buf[2] = (unsigned char) (len>>16);
       buf[3] = (unsigned char) (len>>24);

       //dump the message for debugging if needed
       if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

       sendDsp(buf, len);

       break;

   default:
       DBG_ERROR("grr_PassOamMsSystemInformation4: rrm encoder failed err(%d)\n", ret);
       printf("grr_PassOamMsSystemInformation4: rrm encoder failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_RIL3_CALL_RRM_ENCODER_FAILED);
       assert(0);
       break;
   } 

   DBG_LEAVE();
} 

void grr_PassOamMsSystemInformation5(
                                   unsigned char trx,
                                   unsigned char SI,
                                   T_CNI_RIL3_CELL_BARRED_ACCESS barState
                                 )
{
  int					i,j,len;
  unsigned char			buf[GRR_MAX_L1MSG_LEN];
  T_CNI_RIL3_RESULT       	ret;	
  T_CNI_LAPDM_L3MessageUnit   l3_data;

  DBG_FUNC("grr_PassOamMsSystemInformation5", GRR_LAYER_OAM);
  DBG_ENTER();
 
  //Zap rm_UmMsg 0-clean first
  GRR_MEMSET(&grr_Msg, sizeof(T_CNI_RIL3RRM_MSG));
        
  //Populate SI5 for use by RRM encoder
  grr_PopulateSi5();
  GRR_MEMCPY(&grr_Msg, &grr_Si5, sizeof(T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE5));

  //Call RR message encoding functionality
  ret = CNI_RIL3RRM_Encode(&grr_Msg, &l3_data);

  //RIL3 will reoder the frequencies during encoding. Get the resulted f.list
  GRR_MEMCPY(&grr_Si5, &grr_Msg, sizeof(T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE5));

  //Construct internal adjacent ho cell tables in terms of the resulted order
  for (i=0;i<grr_Si5.bcchFreqList.numRFfreq;i++)
  {
       memset(&adjCell_HandoverTable_t[i],0, sizeof(AdjCell_HandoverEntry));

       for (j=0;j<grr_Si5.bcchFreqList.numRFfreq;j++)
       {

	    if (grr_Si5.bcchFreqList.arfcn[i]==OAMgrr_HO_ADJCELL_BCCH_ARFCN(j))
 	        break;
       }

       //printf("GRR@===f(i#=%d, o#=%d, total=%d), freq=%d\n", i,j,
       //        grr_Si5.bcchFreqList.numRFfreq,
       //        grr_Si5.bcchFreqList.arfcn[i]);
	   
       if (j<grr_Si5.bcchFreqList.numRFfreq)
       {
	   //Put this adjacent-cell configuration in its right place
 	   memcpy(&adjCell_HandoverTable_t[i], &adjCell_HandoverTable[j], sizeof(AdjCell_HandoverEntry));

	   //printf("GRR@===f(#=%d, total=%d), freq=%d, id=%d\n", i,
	   //        grr_Si5.bcchFreqList.numRFfreq,
   	   //        OAMgrr_HO_ADJCELL_BCCH_ARFCN_t(i),
	   //	       OAMgrr_HO_ADJCELL_ID_t(i));
	   
       } else
       {
	   DBG_WARNING("grr_PassOamMsSystemInformation5: alien freq found after RIL3 encoding\n");
       }

  }   

  //check the encoding result before sending the message
  switch (ret)
  {
  case CNI_RIL3_RESULT_SUCCESS:
	 //Wrap the encoded SI in SACCH FILLING sent to L1
       len = 4;
       buf[len++] = GRR_L1MG_TRXMGMT;
       buf[len++] = GRR_L1MT_SACCHFILL_MSB;
       buf[len++] = GRR_L1MT_SACCHFILL_LSB;
	 buf[len++] = trx;
	 buf[len++] = SI;
       buf[len++] = l3_data.msgLength;
	 GRR_MEMCPY(&buf[len], l3_data.buffer, l3_data.msgLength);
       len = len + l3_data.msgLength;
       buf[len++] = 1; 	//Start time==immediate
       buf[len++] = 0;
       buf[len++] = 0;

       //store len in little endian
       buf[0] = (unsigned char) len;
       buf[1] = (unsigned char) (len>>8);
       buf[2] = (unsigned char) (len>>16);
       buf[3] = (unsigned char) (len>>24);

       //dump the message for debugging if needed
       if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

       sendDsp(buf, len);

       break;

   default:
       DBG_ERROR("grr_PassOamMsSystemInformation5: rrm encoder failed err(%d)\n", ret);
       printf("grr_PassOamMsSystemInformation5: rrm encoder failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_RIL3_CALL_RRM_ENCODER_FAILED);
       assert(0);
       break;
   }
} 

void grr_PassOamMsSystemInformation6(
                                   unsigned char trx,
                                   unsigned char SI,
                                   T_CNI_RIL3_CELL_BARRED_ACCESS barState
                                 )
{
  int					i,j,len;
  unsigned char			buf[GRR_MAX_L1MSG_LEN];
  T_CNI_RIL3_RESULT       	ret;	
  T_CNI_LAPDM_L3MessageUnit   l3_data;

  DBG_FUNC("grr_PassOamMsSystemInformation6", GRR_LAYER_OAM);
  DBG_ENTER();
 
  //Zap rm_UmMsg 0-clean first
  GRR_MEMSET(&grr_Msg, sizeof(T_CNI_RIL3RRM_MSG));
        
  //Populate SI6 for use by RRM encoder
  grr_PopulateSi6();
  GRR_MEMCPY(&grr_Msg, &grr_Si6, sizeof(T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE6));

  //Call RR message encoding functionality
  ret = CNI_RIL3RRM_Encode(&grr_Msg, &l3_data);

  //check the encoding result before sending the message
  switch (ret)
  {
  case CNI_RIL3_RESULT_SUCCESS:
	 //Wrap the encoded SI in SACCH FILLING sent to L1
       len = 4;
       buf[len++] = GRR_L1MG_TRXMGMT;
       buf[len++] = GRR_L1MT_SACCHFILL_MSB;
       buf[len++] = GRR_L1MT_SACCHFILL_LSB;
	 buf[len++] = trx;
	 buf[len++] = SI;
       buf[len++] = l3_data.msgLength;
	 GRR_MEMCPY(&buf[len], l3_data.buffer, l3_data.msgLength);
       len = len + l3_data.msgLength;
       buf[len++] = 1; 	//Start time==immediate
       buf[len++] = 0;
       buf[len++] = 0;

       //store len in little endian
       buf[0] = (unsigned char) len;
       buf[1] = (unsigned char) (len>>8);
       buf[2] = (unsigned char) (len>>16);
       buf[3] = (unsigned char) (len>>24);

       //dump the message for debugging if needed
       if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

       sendDsp(buf, len);

       break;

   default:
       DBG_ERROR("grr_PassOamMsSystemInformation6: rrm encoder failed err(%d)\n", ret);
       printf("grr_PassOamMsSystemInformation6: rrm encoder failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_RIL3_CALL_RRM_ENCODER_FAILED);
       assert(0);
       break;
   }
} 

void grr_PassOamMsSystemInformation13(
                                   unsigned char trx,
                                   unsigned char SI,
                                   T_CNI_RIL3_CELL_BARRED_ACCESS barState
                                 )
{
  int					i,j,len;
  unsigned char			buf[GRR_MAX_L1MSG_LEN];
  T_CNI_RIL3_RESULT       	ret;	
  T_CNI_LAPDM_L3MessageUnit   l3_data;

  if (!OAMgrr_GPRS_FEATURE_SETTING) return;

  DBG_FUNC("grr_PassOamMsSystemInformation13", GRR_LAYER_OAM);
  DBG_ENTER();
 
  //Zap rm_UmMsg 0-clean first
  GRR_MEMSET(&grr_Msg, sizeof(T_CNI_RIL3RRM_MSG));
        
  //Populate SI6 for use by RRM encoder
  grr_PopulateSi13();
  GRR_MEMCPY(&grr_Msg, &grr_Si13, sizeof(T_CNI_RIL3RRM_MSG_SYSTEM_INFO_TYPE13));

  //Call RR message encoding functionality
  ret = CNI_RIL3RRM_Encode(&grr_Msg, &l3_data);

  //check the encoding result before sending the message
  switch (ret)
  {
  case CNI_RIL3_RESULT_SUCCESS:
	   //Wrap the encoded SI in SACCH FILLING sent to L1
	   len = 4;
       buf[len++] = GRR_L1MG_COMCHAN;
       buf[len++] = GRR_L1MT_BCCHINFO_MSB;
       buf[len++] = GRR_L1MT_BCCHINFO_LSB;
       buf[len++] = trx;
       buf[len++] = GRR_L1CH_BCCH_MSB;
       buf[len++] = GRR_L1CH_BCCH_LSB;
       buf[len++] = SI;
       buf[len++] = l3_data.msgLength;
       GRR_MEMCPY(&buf[len], l3_data.buffer, l3_data.msgLength);
       len = len + l3_data.msgLength;

       buf[len++] = 1; 	//Start time==immediate
       buf[len++] = 0;
       buf[len++] = 0;

       //store len in little endian
       buf[0] = (unsigned char) len;
       buf[1] = (unsigned char) (len>>8);
       buf[2] = (unsigned char) (len>>16);
       buf[3] = (unsigned char) (len>>24);

       //dump the message for debugging if needed
       if (grr_DbgDump) DBG_HEXDUMP( (unsigned char*)buf, len );

       sendDsp(buf, len);

       break;

   default:
       DBG_ERROR("grr_PassOamMsSystemInformation13: rrm encoder failed err(%d)\n", ret);
       printf("grr_PassOamMsSystemInformation13: rrm encoder failed err(%d)\n", ret);
       grr_SubmitAlarm(EC_GRR_RIL3_CALL_RRM_ENCODER_FAILED);
       assert(0);
       break;
   }
} 

void grr_PassOamMsParams(unsigned char trx)
{
  DBG_FUNC("grr_PassOamMsParams", GRR_LAYER_OAM);
  DBG_ENTER();

  grr_PassOamMsSystemInformation2(trx,GRR_L1SI_TYPE_2,OAMgrr_RA_CELL_BARRED_STATE);
  grr_PassOamMsSystemInformation3(trx,GRR_L1SI_TYPE_3,OAMgrr_RA_CELL_BARRED_STATE);
  grr_PassOamMsSystemInformation4(trx,GRR_L1SI_TYPE_4,OAMgrr_RA_CELL_BARRED_STATE);
  grr_PassOamMsSystemInformation5(trx,GRR_L1SI_TYPE_5,OAMgrr_RA_CELL_BARRED_STATE);
  grr_PassOamMsSystemInformation6(trx,GRR_L1SI_TYPE_6,OAMgrr_RA_CELL_BARRED_STATE);
  grr_PassOamMsSystemInformation13(trx,GRR_L1SI_TYPE_13,OAMgrr_RA_CELL_BARRED_STATE);

  DBG_LEAVE();
}

void grr_PassOamDspParams(unsigned char trx)
{
  unsigned char i, nocells;

  DBG_FUNC("grr_PassOamDspParams", GRR_LAYER_OAM);
  DBG_ENTER();

  nocells=grr_GetOamTotalAvailableHoNCells();
  for (i=0;i<OAMgrr_HO_ADJCELL_NO_MAX;i++)
       grr_PassOamDspHoNeighCell(trx,i,nocells);

  grr_PassOamDspPwrCtrl(trx);
  grr_PassOamDspHoServCell(trx);
  grr_PassOamDspPwrRedStep(trx);
  grr_PassOamDspBtsPackage(trx);

  DBG_LEAVE();
}

void grr_PassOamParams(unsigned char trx)
{
  DBG_FUNC("grr_PassOamParams", GRR_LAYER_OAM);
  DBG_ENTER();

  grr_PassOamMsParams(trx);
  grr_PassOamDspParams(trx);

  DBG_LEAVE();
}

#endif //__GRR_PASSOAMMIB_CPP__
