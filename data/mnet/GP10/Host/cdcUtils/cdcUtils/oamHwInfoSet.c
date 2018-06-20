/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/********************************************************************
*                          
*  FILE NAME: oamHwInfoSet.c    
*                      
*  DESCRIPTION: This file contains routine to populate hardware
*               information to the MIB.
*      COMPONENTS:                      
*                              
*                                  
*  NOTES:                              
*                                  
*  REVISION HISTORY                            
*__________________________________________________________________
*----------+--------+----------------------------------------------
* Name     |  Date  |  Reason                
*----------+--------+----------------------------------------------
* Bhawani  |02/15/00| Initial Draft
* Bhawani  |05/11/00| Added ViperCell Serial Number initializaion
* Bhawani  |08/09/00| Added Default Router Initialization part
* Bhawani  |03/09/01| Change MIB tag for the default Gateway
*----------+--------+----------------------------------------------
********************************************************************
*/

#include "stdio.h"
#include "vxWorks.h"
#include "cdcUtils/auxPort.h"

#include "oam_api.h"
#include "cdcUtils/cdcUtils.h"
#include "cdcUtils/drfInterface.h"
#include "clkInterface.h"
#include "cdc_bsp/nvRam.h"

/* Used temporarily to circumvent OAM API restriction */
#define MY_MODULE_ID MODULE_OAM

STATUS cdcSetMibStr(MibTag tag, char *pszValue)
{
    STATUS status = ERROR;


    if (pszValue)
    {
        int nLen = strlen(pszValue) + 1;
#ifdef _DEBUG
        printf("Setting %s = %s\n",  snmp_tagName(tag), pszValue);
#endif

        if( oam_setMibByteAry(MY_MODULE_ID, tag, pszValue, nLen) != STATUS_OK)
        {
            printf("oam_setMibByteAry failure (tag = %s, value = %s)\n", snmp_tagName(tag), pszValue);
        } else {
            status = OK;
        }
    }
    return status;
}


STATUS cdcSetMibInt(MibTag tag, int iValue)
{
    STATUS status = ERROR; 
    
#ifdef _DEBUG
        printf("Setting %s = %d\n",  snmp_tagName(tag), iValue);
#endif

    if( oam_setMibIntVar(MY_MODULE_ID, tag, iValue) != STATUS_OK)
    {
        printf("oam_setMibIntVal failure (tag = %s, value = %d)\n", snmp_tagName(tag), iValue);
    } else {
        status = OK;
    }
    return status;
}


char *cdcCvtIp2Str(unsigned long ip)
{
    static char buf[16];
    unsigned char *pIp = (unsigned char *) &ip;
    sprintf(buf, "%u.%u.%u.%u", pIp[0], pIp[1], pIp[2], pIp[3]);
    return buf;
}
    

void oamHwInfoPopulate()
{
    int dulaTrxStatus;
    STATUS status;
    int drfType = drfTypeGet();
    int pwrClass = drfType+1;
    char pszSerialNumber[MAX_VC_SN+1]; 
    unsigned long ip;
    char *pszIP;

    /* initialize 1780 */
    DS1780_Init();

    /* Set GSM DCS indicator */
    cdcSetMibInt(MIB_gsmdcsIndicator, drfType);
    cdcSetMibInt(MIB_powerClass_0, pwrClass);
    cdcSetMibInt(MIB_powerClass_1, pwrClass);

    /* Cdc board inititializaiton */

    /* Serial Number */
    cdcSetMibStr(MIB_cdcBoardSerialNumber, cdcSerialNumReturn());

    /* Mac Address */
    cdcSetMibStr(MIB_cdcBoardMACAddress, cdcMacAddrGet());


    /* Dual Trx */

   /* Serial Number */
    cdcSetMibStr(MIB_trxSerialNumber, drfSerialNumReturn());

    /*Version Number */
    cdcSetMibStr(MIB_trxSoftwareVersion, drfVersionReturn());

    /* Get Status */

    if ((status = drfStatGet(&dulaTrxStatus)) != OK)
    {
        printf("drfStatusGet failure (status = %d)\n", status);
    } else {

        /* Set the PLL Lock Status */
        cdcSetMibInt(MIB_monitorReceiverPLL_LockDetectStatus,  (dulaTrxStatus & 0x00000004)==0?0:1);
        cdcSetMibInt(MIB_referenceFrequencyPLL_LockDetectStatus, (dulaTrxStatus & 0x00000008)==0?0:1);
        cdcSetMibInt(MIB_transceiver_1_PLL_LockDetectStatus, (dulaTrxStatus & 0x00000010)==0?0:1);
        cdcSetMibInt(MIB_transceiver_2A_PLL_LockDetectStatus, (dulaTrxStatus & 0x00000020)==0?0:1);
        cdcSetMibInt(MIB_transceiver_2B_PLL_LockDetectStatus, (dulaTrxStatus & 0x00000040)==0?0:1);
        cdcSetMibInt(MIB_intermediateFrequency_LockDetectStatus, (dulaTrxStatus & 0x00000080)==0?0:1);
        
        /*  Memory Status  */   
        cdcSetMibInt(MIB_trxLoopBackMode, (dulaTrxStatus &0x00004000));
        
        /* FPGA Syncronization */
        cdcSetMibInt(MIB_gpsCardStatus, (dulaTrxStatus & 0x00002000));
    }

    /* Clock card */

     /* Serial Number */
    cdcSetMibStr(MIB_clockCardCrystalUpTime, clkSerialNumReturn());

    /* Version Number */
    cdcSetMibStr(MIB_clockCardSoftwareVersion, clkSoftVersionReturn());

    /* Alarm Status */
    cdcSetMibInt(MIB_clockCardStatus, clkStatGet());


    /* Days Since Tuneup:  */
       cdcSetMibInt(MIB_clockCardType, clkNumDaysTuneGet());

    /* Days Since Power On: */
       cdcSetMibInt(MIB_clockCardDAC, clkNumDaysRunGet());

	/* Initialize VieprCell Serial Number */
	   pszSerialNumber[MAX_VC_SN] = 0;
	   if (sysSerialNumGet (pszSerialNumber) == OK)
	   {
			cdcSetMibStr(MIB_viperCellSerialNumber, pszSerialNumber);
	   } else {
		   printf("sysSerialNumGet failure; could not set ViperCell serial number\n");
	   }
	   
	/* Add the default gateway */
	
	if (oam_getMibIntVar(MIB_viperCellDefGateway, &ip))
	{
	     printf("oam_getMibIntVal failure (tag = %s)\n", snmp_tagName(MIB_viperCellDefGateway));
    } else {
        pszIP = cdcCvtIp2Str(ip);
        routeAdd("0", pszIP);
        printf("Default gateway %s has been added\n", pszIP);
    }
}
