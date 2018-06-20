/*
********************************************************************
**
** (c) Copyright Cisco 2000
** All Rights Reserved
**
*********************************************************************
*/
#ifndef __INTG_SENDPHCONNINDTOLAPDM_CPP__
#define __INTG_SENDPHCONNINDTOLAPDM_CPP__

#include "rm/rm_head.h"
#include "lapdm/lapdm_l1intf.h"

#define NUM_SDCCH4S	4
#define NUM_TCHFS	2
#define TRX_IDX		7
#define CHN_MSB		8
#define CHN_LSB		9		
#define LNK_IDX		10

void intg_SendPhConnIndToLapdm(void);

void intg_SendPhConnIndToLapdm(void)
{
     int	   i;
     int           intg_PhConnIndLen      = 11;
     unsigned char intg_PhConnIndData[11] =
     {
          0x10,
	  0x00,
	  0x00,
	  0x00,
	  0x02, //1. L2 PD
	  0x01, //2. L2 MT: PH_CONN_IND MSB
	  0x00, //3. L2 MT: PH_CONN_IND LSB
          0x00, //4. Trx
          0x00, //5. Channel Number MSB byte
          0x00, //6. Channel Number LSB byte
          0x00, //7. Link Identifier
     };

     /*
     ** Send PH_CONN_IND for RACH
     */
     intg_PhConnIndData[TRX_IDX] = 0;
     intg_PhConnIndData[CHN_MSB] = rm_PhyChanBCcch[RM_RACH_IDX].chanNumberMSB;
     intg_PhConnIndData[CHN_LSB] = rm_PhyChanBCcch[RM_RACH_IDX].chanNumberLSB;
     intg_PhConnIndData[LNK_IDX] = RM_L1LINK_VOID;

     CNI_LAPDM_Ph_Callback(
		     intg_PhConnIndLen,
		     intg_PhConnIndData
			);
     /*
     ** Send PH_CONN_IND for all the FACCH and SACCH channels
     */
     for (i=0;i<NUM_TCHFS;i++)
     {
         /* PH_CONN_IND for FACCH */
         intg_PhConnIndData[TRX_IDX] = rm_PhyChanTchf[i].trxNumber;
         intg_PhConnIndData[CHN_MSB] = rm_PhyChanTchf[i].chanNumberMSB;
         intg_PhConnIndData[CHN_LSB] = rm_PhyChanTchf[i].chanNumberLSB;
         intg_PhConnIndData[LNK_IDX] = RM_L1LINK_MAIN;

         CNI_LAPDM_Ph_Callback(
		     intg_PhConnIndLen,
		     intg_PhConnIndData
			);

         /* PH_CONN_IND for SACCH */
         intg_PhConnIndData[TRX_IDX] = rm_PhyChanTchf[i].trxNumber;
         intg_PhConnIndData[CHN_MSB] = rm_PhyChanTchf[i].chanNumberMSB;
         intg_PhConnIndData[CHN_LSB] = rm_PhyChanTchf[i].chanNumberLSB;
         intg_PhConnIndData[LNK_IDX] = RM_L1LINK_ACCH;

         CNI_LAPDM_Ph_Callback(
		     intg_PhConnIndLen,
		     intg_PhConnIndData
			);
     }

     /*
     ** Send PH_CONN_IND for all the SDCCH4 and SACCH channels
     */
     for (i=0;i<NUM_SDCCH4S;i++)
     {
         /* PH_CONN_IND for SDCCH4 */
         intg_PhConnIndData[TRX_IDX] = rm_PhyChanSdcch4[i].trxNumber;
         intg_PhConnIndData[CHN_MSB] = rm_PhyChanSdcch4[i].chanNumberMSB;
         intg_PhConnIndData[CHN_LSB] = rm_PhyChanSdcch4[i].chanNumberLSB;
         intg_PhConnIndData[LNK_IDX] = RM_L1LINK_MAIN;

         CNI_LAPDM_Ph_Callback(
		     intg_PhConnIndLen,
		     intg_PhConnIndData
			);

         /* PH_CONN_IND for SACCH */
         intg_PhConnIndData[TRX_IDX] = rm_PhyChanSdcch4[i].trxNumber;
         intg_PhConnIndData[CHN_MSB] = rm_PhyChanSdcch4[i].chanNumberMSB;
         intg_PhConnIndData[CHN_LSB] = rm_PhyChanSdcch4[i].chanNumberLSB;
         intg_PhConnIndData[LNK_IDX] = RM_L1LINK_ACCH;

         CNI_LAPDM_Ph_Callback(
		     intg_PhConnIndLen,
		     intg_PhConnIndData
			);
     }
}

#endif /* __INTG_SENDPHRAINDTOLAPDM_CPP__ */
