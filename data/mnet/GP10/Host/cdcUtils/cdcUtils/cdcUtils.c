/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

#include "vxWorks.h"
#include "stdio.h"
#include "cdc_bsp\nvRam.h"
#include "cdcUtils\auxPort.h"
#include "l1proxy\l1proxy.h"
#include "mch\MCHDefs.h"

#define CDC_SERNUM_SIZE  MAX_CDC_SN     /* cdc serial number size  */

/*--------- externals ---------*/
extern char ds2401[8];        /* global 8 byte buffer                      */
extern int ds2401CRC;         /* calculated checksum                       */
void oamHwInfoPopulate();
extern STATUS I2Cwrite(unsigned char*, unsigned char, unsigned char, int);
extern STATUS I2Cread(unsigned char*, unsigned char, unsigned char, int);
extern void   I2Coperation(int);

#define ON  1
#define OFF 0


/*---------- globals ----------*/
char cdcSerialNum[CDC_SERNUM_SIZE + 1] = "????";
char cdcMacAddrStr[12 + 1];
int radioBoardVerifyEepromOn = 1;
 
void  cdcBootromVerInfo(void);
char *cdcGetBootromVersion(void);
int   cdcVerifyBootromCheckSum(void);
char *cdcBootromVerDate(void);
int   cdcGetBootromCheckSum(void);
int   cdcCalCheckSum(unsigned char *loc, int number);

/*******************************************************************************
cdcLed1On - LED 1 On
Turn LED 1 on
*/
void cdcLed1On()
{
  auxPortOutSet(~LED1_OFF, LED1_OFF);
}


/*******************************************************************************
cdcLed1Off - LED 1 Off
Turn LED 1 off
*/
void cdcLed1Off()
{
  auxPortOutSet(LED1_OFF, LED1_OFF);
}


/*******************************************************************************
cdcLed2On - LED 2 On
Turn LED 2 on
*/
void cdcLed2On()
{
  auxPortOutSet(~LED2_OFF, LED2_OFF);
}


/*******************************************************************************
cdcLed2Off - LED 2 Off
Turn LED 2 off
*/
void cdcLed2Off()
{
  auxPortOutSet(LED2_OFF, LED2_OFF);
}


/*******************************************************************************
cdcLed3On - LED 3 On
Turn LED 3 on
*/
void cdcLed3On()
{
  auxPortOutSet(~LED3_OFF, LED3_OFF);
}


/*******************************************************************************
cdcLed3Off - LED 3 Off
Turn LED 3 off
*/
void cdcLed3Off()
{
  auxPortOutSet(LED3_OFF, LED3_OFF);
}


/*******************************************************************************
cdcFaultLedOn - Fault LED On
Turn Fault LED on
*/
void cdcLed4On()
{
  auxPortOutSet(~FAULT_LED_OFF, FAULT_LED_OFF);
}


/*******************************************************************************
cdcFaultLedOff - Fault LED Off
Turn Fault LED off
*/
void cdcLed4Off()
{
  auxPortOutSet(FAULT_LED_OFF, FAULT_LED_OFF);
}


/*******************************************************************************
cdcLed5On - LED 5 On
Turn LED 5 on
*/
void cdcLed5On()
{
  auxPortOutSet(LED5_ON, LED5_ON);
}


/*******************************************************************************
cdcLed5Off - LED 5 Off
Turn LED 5 off
*/
void cdcLed5Off()
{
  auxPortOutSet(~LED5_ON, LED5_ON);
}


/*******************************************************************************
cdcPowerOff - CDC Power Off
Turn off Power to CDC
*/
void cdcPowerOff()
{
  auxPortOutSet(CDC_POWER_OFF, CDC_POWER_OFF);
}


/*******************************************************************************
cdcSerialNumGet - CDC Serial Number Get

*/
void cdcSerialNumGet()
{
  cdcSerialNum[CDC_SERNUM_SIZE] = 0;
  if (sysCDCSerialNumGet(cdcSerialNum) != OK)
  {
    cdcSerialNum[0] = 'x';
    cdcSerialNum[1] = 'x';
    cdcSerialNum[2] = 'x';
    cdcSerialNum[3] = 'x';
    cdcSerialNum[4] = 0;
  }
}

/*******************************************************************************
cdcSerialNumReturn - CDC Serial Number Return

*/
char* cdcSerialNumReturn()
{
  return cdcSerialNum;
}


/*******************************************************************************
cdcSerialNumPrint - CDC Serial Number Print

*/
void cdcSerialNumPrint()
{
  printf("CDC Serial Number: %s\n", cdcSerialNum);
}


/*******************************************************************************
cdcMacAddrGet - CDC MAC Address Get

*/
char* cdcMacAddrGet()
{
  char  macAddr[6];
  int i;

  taskDelay(10);
  sysEnetAddrGet(0, macAddr);
  taskDelay(10);
  for (i = 0; i < 6; i++)
  {
    sprintf(&cdcMacAddrStr[i * 2], "%02X", macAddr[6 - 1 - i]);
  }
  cdcMacAddrStr[12] = 0;

  return cdcMacAddrStr;
}

/*******************************************************************************
dspReset

reset DSPs
*/
void dspReset(void)
{
  /* put DSPs in reset */
  auxPortOutSet(~(DSPA_RST_NOT|DSPB_RST_NOT), DSPA_RST_NOT|DSPB_RST_NOT);
  /* delay 1.0 s */
  taskDelay((int)(1.0 * sysClkRateGet()));
  /* bring DSPs out of reset */
  auxPortOutSet((DSPA_RST_NOT|DSPB_RST_NOT), DSPA_RST_NOT|DSPB_RST_NOT);
}


/*******************************************************************************
cdcInit

Initialize CDC board
*/
STATUS cdcInit(const char* fpgaFile, const char *base_prefix)
{
  char fName[256];
  STATUS  retVal = OK;

  strcpy(fName, base_prefix);
  strcat(fName, "/");
  strcat(fName, fpgaFile);
  
  auxPortInit();                 /* Initilalize auxillary output port */
  cdcSerialNumGet();             /* Get CDC serial Number */
  
  printf("Resetting DSPs\n");
  dspReset();

  printf("Configuring FPGAs\n");
  if ((fpgaConfig(fName)) != 0)
  {
    printf("FPGA configuration failed\n");
    retVal = ERROR;
  }

  if (radioBoardVerifyEepromOn)
  {
    printf("Validating Radio Board EEPROM\n");
    if ((VerifyRadioBoardEeprom() != OK))
    {
      printf("Radio Board EEPROM valication failed!\n");
    }
  }
  else
  {
    printf("Radio Board EEPROM will NOT be validated\n");
  }

  printf("Initializing Dual RF module\n");
  if ((drfInit()) != OK)
  {
    printf("RF module initialization failed\n");
    retVal = ERROR;
  }

  printf("Initializing Clock module\n");
  if ((clkInit()) != OK)
  {
    printf("Clock module initialization failed\n");
    retVal = ERROR; 
  }

  cdcLed5On();  /* Turn on LED 5 */

  /* Initialize MIB hardware parameters */
  oamHwInfoPopulate();

  return retVal;
}


/*******************************************************************************
cdcTestInit

Initialize CDC board for Board Testing
*/
STATUS cdcTestInit(const char* fpgaFile, const char *base_prefix)
{
  char fName[256];
  STATUS  retVal = OK;

  strcpy(fName, base_prefix);
  strcat(fName, "/");
  strcat(fName, fpgaFile);
  
  auxPortInit();                 /* Initilalize auxillary output port */
  cdcSerialNumGet();             /* Get CDC serial Number */

  printf("Configuring FPGAs\n");
  if ((fpgaConfig(fName)) != 0)
  {
    printf("FPGA configuration failed\n");
    retVal = ERROR;
  }

  cdcLed5On();  /* Turn on LED 5 */

  return retVal;
}

/*******************************************************************************
cdcBootromVerInfo

Sends Bootrom version information to standard I/O.
*/
void cdcBootromVerInfo(void)
{
    char    *chPtr;

    chPtr = (char *)0x20000020;
    printf("Bootrom %s\n", chPtr);

    chPtr = (char *)0x20000000;
    printf("Bootrom Date: %s\n", chPtr);
}


/*******************************************************************************
cdcBootromVerInfo

Returns Bootrom version.
*/
char *cdcGetBootromVersion(void)
{
    char    *chPtr;

    chPtr = (char *)0x20000020;
    return(chPtr);
}

/*******************************************************************************
IsCorrectCdcBootromVer

Returns OK if bootrom version info 1st character = 'V'
*/
int IsCorrectCdcBootromVer(void)
{
    char    *chPtr;

    chPtr = cdcGetBootromVersion();

    if ( *chPtr != 'V' )
        return ERROR;
    else 
        return OK;
}

/*******************************************************************************
cdcBootromVerDate

Returns Bootrom version date.
*/
char *cdcBootromVerDate(void)
{
    char    *chPtr;

    chPtr = (char *)0x20000000;
    return(chPtr);
}


/*******************************************************************************
cdcVerifyBootromCheckSum

Verifies checksum for bootrom memory locations.

RETURNS:  OK = Valid Bootrom   ERROR = Bootrom is corrupted or wrong version
*/
int cdcVerifyBootromCheckSum(void)
{
    int              checksum;
    int              fileCheckSum;
    unsigned char   *pfileCheckSum;
    unsigned char    cfilesum[5];
    int              result;

    if ( IsCorrectCdcBootromVer() == OK )
    {
        pfileCheckSum = (unsigned char *)0x20000060;
        strcpy(cfilesum,pfileCheckSum);
        sscanf(cfilesum, "%x", &fileCheckSum);
        checksum = cdcGetBootromCheckSum();

        if ( fileCheckSum == checksum )
        {
            result = OK;
            printf("Bootrom CheckSum is valid\n");
        }
        else
        {   
            printf("Bootrom CheckSum Verify FAILED !!\n");
            result = ERROR;
        }
    }
    else
    {
        printf("Bootrom CheckSum Verify FAILED\n");
        printf("No CheckSum info in header, wrong bootrom version !!\n");
        result = ERROR;
    }

    return(result);
}


/*******************************************************************************
cdcGetBootromCheckSum

Calculates checksum for bootrom memory locations if the bootrom version is valid.
*/
int cdcGetBootromCheckSum(void)
{
    int              checksum  = 0; 
    int              databytes = 0;
    unsigned char   *pDataBytes;

    if ( IsCorrectCdcBootromVer() == OK )
    {
        pDataBytes = (unsigned char *)0x20000064;

        sscanf((char *)pDataBytes, "%d", &databytes);

        checksum = cdcCalCheckSum((unsigned char *)0x20000100, databytes);
    }
    return(checksum);
}


/*******************************************************************************
cdcCalCheckSum

Calculates checksum for consecutive memory locations.
*/
int cdcCalCheckSum(unsigned char *loc, int number)
{
    int i;
    int calcChkSum = 0;

    for (i = 0; i < number; i++)
    {
        calcChkSum += loc[i];
    }
    
    calcChkSum = (~calcChkSum) & 0xff;

    printf("Checksum = %02x\n", calcChkSum);

    return(calcChkSum);
}




/*****************************************************************************
 *
 *  Module Name: SaveEepromInNVRam
 *  
 *  Purpose: Read EEPROM from the radio board and store in NVRam.
 *
 *****************************************************************************/
STATUS   SaveEepromInNVRam(void)
{
   STATUS status;
   unsigned char buf[RADIO_BOARD_EEPROM_SIZE];  
   int i2cRetryCount;
     
   i2cRetryCount = 3;  
   do 
   {
       I2Coperation(ON);
       status = I2Cread(buf, 0x52, 0, RADIO_BOARD_EEPROM_SIZE/2);
       I2Coperation(OFF);   
   } while ((status != OK) && (--i2cRetryCount > 0));
     
   if (status == OK)
   {
       i2cRetryCount = 3;  
       do 
       {
           I2Coperation(ON);
           status = I2Cread(buf+RADIO_BOARD_EEPROM_SIZE/2, 0x53, 0, RADIO_BOARD_EEPROM_SIZE/2);
           I2Coperation(OFF);   
       } while ((status != OK) && (--i2cRetryCount > 0));
   }
   
   if (status == OK)
   {
      status = sysRadioBoardEepromSet((char *)buf);
   }
   return(status);
}


/*****************************************************************************
 *
 *  Module Name: RestoreEepromFromNVRam
 *  
 *  Purpose: Read saved EEPROM from NVRam.  If it does not match the current
 *   EEPROM values then write the NVRam version to the radio board.
 *
 *****************************************************************************/
STATUS   RestoreEepromFromNVRam(void)
{
   STATUS status;
   unsigned char nvRamEeprom[RADIO_BOARD_EEPROM_SIZE];  
   unsigned char radioBoardEeprom[RADIO_BOARD_EEPROM_SIZE];  
   int i2cRetryCount, i;
     
   /* Read in the EEPROM table stored in NNRam */
   status = sysRadioBoardEepromGet((char *)nvRamEeprom);
   
   /* Read in the Radio Board EEPROM */
   i2cRetryCount = 3;  
   do 
   {
       I2Coperation(ON);
       status = I2Cread(radioBoardEeprom, 0x52, 0, RADIO_BOARD_EEPROM_SIZE/2);
       I2Coperation(OFF); 
   } while ((status != OK) && (--i2cRetryCount > 0));
    
   if (status == OK)
   {
       i2cRetryCount = 3;  
       do 
       {
           I2Coperation(ON);
           status = I2Cread(radioBoardEeprom+RADIO_BOARD_EEPROM_SIZE/2, 0x53, 0, RADIO_BOARD_EEPROM_SIZE/2);
           I2Coperation(OFF); 
       } while ((status != OK) && (--i2cRetryCount > 0));
   }
     
   if (status == OK)
   {
        /* Check NVRam version against Radio Board version */
        for (i = 0; i < RADIO_BOARD_EEPROM_SIZE; i++)
        {
            /* If any errors are found then write out the NVRam version */
            if (nvRamEeprom[i] != radioBoardEeprom[i])
            {
                printf("Radio Board EEPROM does not match NV Ram!\n"
                    "Radio Board EEPROM will be updated to match NV Ram!\n");
                I2Coperation(ON);
                for ( i=0; i<RADIO_BOARD_EEPROM_SIZE/2; i++ )
                {
                    i2cRetryCount = 3;  
                    do 
                    {
                        status = I2Cwrite(nvRamEeprom+i, 0x52, (unsigned char)i, 1);
                        taskDelay(2);    /* Limit how fast the EEPROM gets the bytes */
                    } while ((status != OK) && (--i2cRetryCount > 0));
                }
                for ( i=RADIO_BOARD_EEPROM_SIZE/2; i<RADIO_BOARD_EEPROM_SIZE; i++ )
                {
                    i2cRetryCount = 3;  
                    do 
                    {
                        status = I2Cwrite(nvRamEeprom+i, 0x53, (unsigned char)i, 1);
                        taskDelay(2);    /* Limit how fast the EEPROM gets the bytes */
                    } while ((status != OK) && (--i2cRetryCount > 0));
                }
                
                I2Coperation(OFF);
                break;
            }
        }
   }
   
   return(status);
}



/*******************************************************************************
VerifyRadioBoardEeprom

*/

int VerifyRadioBoardEeprom()
{
    int status;
    /* If there is a valid saved Radio EEPROM in NvRam then
       restore that on the radio board. */
    if (sysIsRadioBoardEepromSet())
    {
        if ((status = RestoreEepromFromNVRam()) != OK)
        {
            printf("WARNING - Unable to restore Radio Board EEPROM!\n");
        }
    }
    /* Otherwise we have to assume the EEPROM is ok so we will 
      move it into NvRam. */
    else
    {
        printf("Saving Radio Board EEPROM into NV Ram!\n");
        if ((status = SaveEepromInNVRam()) != OK)
        {
            printf("WARNING - Unable to save Radio Board EEPROM!\n");
        }
    }
    return(status);
}
