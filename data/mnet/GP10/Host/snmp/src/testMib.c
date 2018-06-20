#include <vxWorks.h>
#include <mib.h>
#include <snmpdefs.h>
#include <snmpdLib.h>
#include <ioLib.h>
#include <fioLib.h>
#include <taskLib.h>
#include <sysLib.h>
#include <stdio.h>

#include "vipermib.h"		/* Get the MIB definitions. */
#include "vipertbl.h"		/* Control Table definitions.	*/
#include "version.h"

/* forward declaration */
extern void snmp_printPDU(SNMP_PKT_T *, char *);

void snmp_testMethod(
                     OIDC_T      lastmatch,
                     int         compc,
                     OIDC_T     *compl,
                     SNMP_PKT_T *pktp,
                     VB_T       *vbp)
{
   extern Boolean mibInitialized;
   
   int     idx, tag, ttag, min, max, val;
   Table   *tbl;
   MnetMibDataType    *dtype;
   
   
   /* Although I have never seen it happen, the varbind list may have
   * multiple data items to process. Do them all.
   */
   for (snmpdGroupByGetprocAndInstance(pktp, vbp, compc, compl);
   vbp; vbp = vbp->vb_link) {
      tag = VB_TAG(vbp);
      DMSG("test method", printf("snmp_testMethod: Processing varbind for item %s\n", snmp_tagName(tag)));
      
      if (mibInitialized == False)
      {
         testproc_error(pktp, vbp, RESOURCE_UNAVAILABLE);
         DMSG("test method", printf("snmp_getMethod: 1: MIB not initialized!!! Setting testproc_error(..., RESOURCE_UNAVAILABLE)\n"));
         continue;
      }
      if (compc)
      {
         if (MTYPE_IS_SCALAR(VB_TYPE(vbp)))
         {  
            /* compc != 0, Scalar entry ? */
            if (*compl) 
            {
               testproc_error(pktp, vbp, NO_SUCH_NAME);    /* Yes, only single instances */
               DMSG("test method", printf("snmp_testMethod: 2: setting testtproc_error(..., NO_SUCH_NAME)\n"));
               continue;                   /* are in the MIB. */
            }
         }
         else
         { 
            /* compc != 0, Tabular entry ? */
            idx = *compl;
            ttag = MIB_ITEM(VB_TBL(vbp), VB_ITM(vbp));  /* Set the table pointer. */
            tbl = (Table *)snmp_getValue(ttag);     /* So that we can tell if */
            if (!VALID_TABLE(tbl))
               DMSG("test method", printf("WARNING!! Invalid table pointer!!\n"));
            if (idx >= table_end(tbl)) 
            {         /* Is entry there? */
               DMSG("test method", printf("snmp_testMethod: 3: Instance %d too big. Table has %d entries.\n", idx, table_end(tbl)));
               testproc_error(pktp, vbp, NO_SUCH_NAME);        /* nope. */
               DMSG("test method", printf("snmp_testMethod: 4: Setting testproc_error(..., NO_SUCH_NAME)\n"));
               continue;
            }
         }
      }
      else
      {   /* compc = 0 */
         testproc_error(pktp, vbp, NO_SUCH_NAME);
         DMSG("test method", printf("snmp_testMethod: 5: Setting testproc_error(..., NO_SUCH_NAME)\n"));
         continue;
      }
      if (snmp_validTag(tag) == False)
      {
         testproc_error(pktp, vbp, NO_SUCH_NAME);
         DMSG("test method", printf("snmp_testMethod: 6: Bad tag!!! Setting testproc_error(..., NON_SUCH_NAME)\n"));
         continue;
      }
      val = VB_GET_INT32(vbp);
      
      /* Find the type for this varbind. Later this will change to a
      * more efficient lookup routine like a binary search.
      */
      
      if (!(dtype = (MnetMibDataType *)table_find(mnetMibDataTypeTable, snmp_findTag, (void *)&tag))) {
         testproc_error(pktp, vbp, NO_SUCH_NAME);
         DMSG("test method", printf("snmp_testMethod: 7: Unable to locate TypeData for '%s'!! Setting testproc_error(..., NON_SUCH_NAME)\n", snmp_tagName(tag)));
         continue;
      }
      
      min = max = 0;
      switch (dtype->type) {
         
         /* ViperCell MIB Variables */
      case TYPE_CauseCode:
      case TYPE_FrequencyUsage:
      case TYPE_GSMGeneralObjectID:
      case TYPE_L2Timer:
      case TYPE_L3Timer:
      case TYPE_MSRangeMax:
      case TYPE_PlmnPermitted:
      case TYPE_QueueTimeLimit:
      case TYPE_Tsc:
      case TYPE_TxPower:
      default:
         break;
      case TYPE_Ny1:
		  max = 7; break;
         /* String data types. */
      case TYPE_TBCD_STRING:          /* STRING */
      case TYPE_MobileCountryCode:        /* STRING SIZE(2) */
      case TYPE_MobileNetworkCode:        /* STRING SIZE(1) */
         break;
      case TYPE_PowerRedStepSize:         /* 0 - 1 */
      case TYPE_Boolean:
      case TYPE_EnableHoType:
      case TYPE_OperationalState:
         max = 1;  break;
      case TYPE_PowerIncrStepSize:        /* 0 - 2 */
      case TYPE_AdministrativeState:
      case TYPE_DtxUplink:
      case TYPE_GsmdcsIndicator:
         max = 2;  break;
      case TYPE_Weighting:            /* 0 - 3 */
         max = 3;  break;
      case TYPE_AlarmStatus:          /* 0 - 4 */
         max = 4;  break;
      case TYPE_CellReselectHysteresis:       /* 0 - 7 */
      case TYPE_ChannelID:
      case TYPE_HoPriorityLevel:
      case TYPE_NetworkColourCode:
      case TYPE_NoOfBlocksForAccessGrant:
      case TYPE_RxQual:
         max = 7;  break;
      case TYPE_ChannelCombination:       /* 0 - 13 */
         max = 13;  break;
      case TYPE_PowerClass:           /* 1 - 3 CSCdv08311*/
         min = 1;  max = 3;  break;
      case TYPE_NoOfMultiframesBetweenPaging: /* 2 - 9 */
         min = 2;  max = 9;  break;
      case TYPE_ClassNumber:          /* 0 - 15 */
      case TYPE_RadioLinkTimeout:
      case TYPE_TxInteger:
		  max = 15; break;		/* fixing PR CSCdu33331 */
      case TYPE_TxPwrMaxReduction:
         max = 6;  break;
      case TYPE_HoMargin:             /* 0 - 24 */
         max = 24;  break;
      case TYPE_AveragingNumber:          /* 0 - 31 */
      case TYPE_PowerControlInterval:
         max = 31;  break;
      case TYPE_HoppingSequenceNumber:        /* 0 - 63 */
      case TYPE_RxLev:
         max = 63;  break;
      case TYPE_MaxQueueLength:           /* 0 - 100 */
         max = 100;  break;
      case TYPE_TimerPeriodicUpdateMS:        /* 0 - 255 */
         max = 255;  break;
      case TYPE_AbsoluteRFChannelNo:      /* 0 - 1023 */
         max = 1023;  break;
      case TYPE_CellIdentity:         /* 0 - 65535 */
      case TYPE_LocationAreaCode:
         max = 65535;  break;
      case TYPE_MaxRetrans:           /* 1, 2, 4, 7 */
         if (! ( (val == 1) || (val == 2) || (val == 4) || (val == 7))) {
            DMSG("test method", printf("snmp_testMethod: 8: Invalid value=%d, set={1, 2, 4, 7}. Setting testproc_error(..., WRONG_VALUE)\n", val));
            testproc_error(pktp, vbp, WRONG_VALUE);
            continue;
         }
         break;
	  case TYPE_Si13Position:		/* 0 - 1 */
		  max = 1; break;
		  break;
	  case TYPE_CbchTrxSlot:		/* 0 - 7 */
		  max = 7; break;			
		  break;
	  case TYPE_CbchTrx:			/* 0 - 1 */
		  max = 1; break;			
		  break;
	  case TYPE_BcchChangeMark:		/* 0 - 3 */
		  max = 3; break;			
		  break;
	  case TYPE_SiChangeField:		/* 0 - 15 */
		  max = 15; break;			
		  break;
      }
      if (max && (! ((val >= min) && (val <= max)))) {
         DMSG("test method", printf("snmp_testMethod: 8: Invalid value=%d, min=%d, max=%d Setting testproc_error(..., WRONG_VALUE)\n", val, min, max));
         testproc_error(pktp, vbp, WRONG_VALUE);
         continue;
      }
      /* add any other tests here */
      /*      if (ptr = table_find(validate_list, tag)); */
      DMSG("test method", printf("snmp_testMethod: 9: Valid paramater. Setting testproc_good(...)\n"));
      
      testproc_good(pktp, vbp);
    }
}

int
snmp_verifyMib()
{
    int     i = -1;
    int     tag, addr, value;
    
    while ((tag = mnetMibIndex[++i].tag)) {
        addr = (int) snmp_getAddress(tag);
        value = (int) snmp_getValue(tag);
        DMSG("verify", printf("verifyMib(): '%s' base=0x%x addr=0x%x, *addr=0x%x, value=0x%x\n",
            snmp_tagName(tag), (int) mnetMibIndex[i].addr, addr, *(int *)addr, value));
    }
    DMSG("verify", printf("verifyMib(): 'MIB_T200': base=0x%x, addr=0x%d, value=0x%x\n",
        (int) &mnetMib.t200,
        (int) &mnetMib.btsTimerPackage.t200,
        (int) mnetMib.btsTimerPackage.t200));
    DMSG("verify", printf("verifyMib(): 'MIB_T31XX': base=0x%x, addr=0x%d, value=0x%x\n",
        (int) &mnetMib.t31xx,
        (int) &mnetMib.btsTimerPackage.t31xx,
        (int) mnetMib.btsTimerPackage.t31xx));
    
    DMSG("verify", printf("verifyMib(): 'MIB_T31XX': base=0x%x, addr=0x%d, value=0x%x\n",
        (int) &mnetMib.t31xx, (int) &mnetMib.btsTimerPackage.t31xx, (int) mnetMib.btsTimerPackage.t31xx));
    
    
    DMSG("verify", printf("verifyMib(): 'MIB_hoAveragingAdjCellParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.hoAveragingAdjCellParam,
        (int) &mnetMib.handoverControlPackage.hoAveragingAdjCellParam,
        (int) mnetMib.handoverControlPackage.hoAveragingAdjCellParam));
    DMSG("verify", printf("verifyMib(): 'MIB_hoAveragingDistParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.hoAveragingDistParam,
        (int) &mnetMib.handoverControlPackage.hoAveragingDistParam,
        (int) mnetMib.handoverControlPackage.hoAveragingDistParam));
    DMSG("verify", printf("verifyMib(): 'MIB_hoAveragingLevParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.hoAveragingLevParam,
        (int) &mnetMib.handoverControlPackage.hoAveragingLevParam,
        (int) mnetMib.handoverControlPackage.hoAveragingLevParam));
    DMSG("verify", printf("verifyMib(): 'MIB_hoAveragingQualParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.hoAveragingQualParam,
        (int) &mnetMib.handoverControlPackage.hoAveragingQualParam,
        (int) mnetMib.handoverControlPackage.hoAveragingQualParam));
    DMSG("verify", printf("verifyMib(): 'MIB_hoThresholdDistParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.hoThresholdDistParam,
        (int) &mnetMib.handoverControlPackage.hoThresholdDistParam,
        (int) mnetMib.handoverControlPackage.hoThresholdDistParam));
    DMSG("verify", printf("verifyMib(): 'MIB_hoThresholdInterfaceParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.hoThresholdInterfaceParam,
        (int) &mnetMib.handoverControlPackage.hoThresholdInterfaceParam,
        (int) mnetMib.handoverControlPackage.hoThresholdInterfaceParam));
    DMSG("verify", printf("verifyMib(): 'MIB_hoThresholdLevParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.hoThresholdLevParam,
        (int) &mnetMib.handoverControlPackage.hoThresholdLevParam,
        (int) mnetMib.handoverControlPackage.hoThresholdLevParam));
    DMSG("verify", printf("verifyMib(): 'MIB_hoThresholdQualParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.hoThresholdQualParam,
        (int) &mnetMib.handoverControlPackage.hoThresholdQualParam,
        (int) mnetMib.handoverControlPackage.hoThresholdQualParam));
    DMSG("verify", printf("verifyMib(): 'MIB_interferenceAveragingParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.interferenceAveragingParam,
        (int) &mnetMib.handoverControlPackage.interferenceAveragingParam,
        (int) mnetMib.handoverControlPackage.interferenceAveragingParam));
    
    DMSG("verify", printf("verifyMib(): 'MIB_pcAveragingLev': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.pcAveragingLev,
        (int) &mnetMib.powerControlPackage.pcAveragingLev,
        (int) mnetMib.powerControlPackage.pcAveragingLev));
    
    DMSG("verify", printf("verifyMib(): 'MIB_pcAveragingQual': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.pcAveragingQual,
        (int) &mnetMib.powerControlPackage.pcAveragingQual,
        (int) mnetMib.powerControlPackage.pcAveragingQual));
    
    DMSG("verify", printf("verifyMib(): 'MIB_pcLowerThresholdLevParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.pcLowerThresholdLevParam,
        (int) &mnetMib.powerControlPackage.pcLowerThresholdLevParam,
        (int) mnetMib.powerControlPackage.pcLowerThresholdLevParam));
    
    DMSG("verify", printf("verifyMib(): 'MIB_pcLowerThresholdQualParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.pcLowerThresholdQualParam,
        (int) &mnetMib.powerControlPackage.pcLowerThresholdQualParam,
        (int) mnetMib.powerControlPackage.pcLowerThresholdQualParam));
    
    DMSG("verify", printf("verifyMib(): 'MIB_pcUpperThresholdLevParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.pcUpperThresholdLevParam,
        (int) &mnetMib.powerControlPackage.pcUpperThresholdLevParam,
        (int) mnetMib.powerControlPackage.pcUpperThresholdLevParam));
    
    DMSG("verify", printf("verifyMib(): 'MIB_pcUpperThresholdQualParam': base=0x%x, addr=0x%x. value=0x%x\n",
        (int) &mnetMib.pcUpperThresholdQualParam,
        (int) &mnetMib.powerControlPackage.pcUpperThresholdQualParam,
        (int) mnetMib.powerControlPackage.pcUpperThresholdQualParam));
    
    return 0;
}
