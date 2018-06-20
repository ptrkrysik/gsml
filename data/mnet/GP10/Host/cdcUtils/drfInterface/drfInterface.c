/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*******************************************************************************      
drfInterface.c

This module consists of two set of functions. One to interface with the Dual RF 
module and the other to write to and read from the EEPROM on the Dual RF
module.

*/
/*

All the commands to the rf module are sent by drfCmd(). This function is called 
by other functions. It should not be called directly. It can be used to send 
commands from console.

*/

#include "vxWorks.h"
#include "stdio.h"
#include "string.h"
#include "semLib.h"
#include "taskLib.h"
#include "sysLib.h"

#include "cdc_bsp/i2c.h"
#include "cdcUtils/drfInterface.h"

#define MAX_CMD_LEN 53
#define MAX_CMD_SEG_SIZE 1
#define RSP_LEN 4
#define RF_ADDR 0x77
#define RF_MEM_ADDR 0x52
#define MAX_RF_MEM_SIZE 26
#define VERSION_LEN 4
#define SER_NUM_LEN 12


/*---------- Globals ----------*/
char drfVersion[VERSION_LEN + 1] = "xxxx";    /* Dual RF firmware version */
char drfSerialNum[SER_NUM_LEN + 1] = "xxxxxxxxxxxx";  /* Dual RF serial Number */

/*-------- File Locals --------*/
static char rspDefault[RSP_LEN];
static char drfMem[MAX_RF_MEM_SIZE];   
static SEM_ID drfSem;
static int drfType;
static unsigned int drfVersionStage;
static unsigned int drfMemLayoutVer;
static unsigned int drfMemSize;

/*******************************************************************************      

ascDec2Int - ASCII Decimal to Integer

Converts a string of four ASCII coded decimal chracters to an integer.

*/

static STATUS ascDec2Int  /* RETURN: OK/ERROR in the input */
  (
     const char* str,    /* IN : ASCII chracter string */
     int* pNum           /* OUT : number */
  )
{
  int retVal = OK;
  int pwr[4] = {1000,100,10,1}; 
  int i;

  *pNum = 0;
  for (i = 0; i < 4; i++)
  {
    if ((str[i] >= '0') && (str[i] <= '9'))
    {
      *pNum += (str[i] - '0') * pwr[i];
      printf("num: %d\n",*pNum);
    }
    else
    {
      retVal = ERROR;
      break;
    }
  }
  return retVal;
}

/*******************************************************************************      

ascHex2Int - ASCII Hex to Integer

Converts a string of four ASCII coded hexadecimal chracters to an integer.

*/

static STATUS ascHex2Int   /* RETURN: OK/ERROR in the input */
  (
    const char* str,     /* IN : ASCII chracter string */
    int* pNum             /* OUT : number */
  )
{
  int retVal = OK;
  int pwr[4] = {16*16*16,16*16,16,1}; 
  int i;

  *pNum = 0;
  for (i = 0; i < 4; i++)
  {
    if ((str[i] >= '0') && (str[i] <= '9'))
      *pNum += (str[i] - '0') * pwr[i];
    else if ((str[i] >= 'A') && (str[i] <= 'F'))
      *pNum += (str[i] - 'A' + 10) * pwr[i];
    else if ((str[i] >= 'a') && (str[i] <= 'f'))
      *pNum += (str[i] - 'a' + 10) * pwr[i];
    else
    {
      retVal = ERROR;
      break;
    }
  }
  return retVal;
}

/*******************************************************************************      

byte2AscDec - Byte to ASCII Decimal

Converts a byte to three ascii coded decimal chracters

*/

static void byte2AscDec
  (
    unsigned char num,         /* IN : byte to convert */
    char* ascStr               /* OUT : ASCII chracter string */
  )
{
  ascStr[0] = '0' + num/100;
  num -= (num/100)*100;
  ascStr[1] = '0' + num/10; 
  num -= (num/10)*10;
  ascStr[2] = '0' + num;
}

/*******************************************************************************      

byte2AscHex - Byte to ASCII Hex

Converts a byte to two ascii coded hexadecimal chracters

*/

static void byte2AscHex
  (
    unsigned char num,     /* IN : byte to convert */
    char* ascStr           /* OUT : ASCII chracter string */
  )
{
  unsigned char temp;
  
  temp = num >> 4;
  if (temp < 10)   
    ascStr[0] = '0' + temp;
  else 
    ascStr[0] = 'A' + temp - 10;

  temp = num & 0xF;
  if (temp < 10)   
    ascStr[1] = '0' + temp;
  else 
    ascStr[1] = 'A' + temp - 10;
}

/*******************************************************************************      
rspOk - Response Ok

Checks if the response from drf board is 'Command Successful'
Returns true or false
*/

static int rspOk(char* rsp)
{
  if ((rsp[0] == '0') && (rsp[1] == '0') && (rsp[2] == '0') && (rsp[3] == '0'))
    return TRUE;
  else
    return FALSE;
}

/*******************************************************************************      
rspError - Response Error

Checks if the response from drf board is 'Command Unsuccessful'
Returns true or false
*/

static int rspError(char* rsp)
{
  if ((rsp[0] == 'F') && (rsp[1] == 'F') && (rsp[2] == 'F') && (rsp[3] == 'F'))
    return TRUE;
  else
    return FALSE;
}


/*******************************************************************************      

drfCmd - Command

This rouine sends a command to dual radio board and receives the response back.
It uses I2C interface routine to interface to the dual RF board.

No more than one command is in progress at any given time.

*/

STATUS drfCmd
  (
    char* cmd,  /* command: Null terminated chracter string */
    char* rsp   /* response */
  )
{
  int retVal;    /* Return Value */
  int cmdLen;    /* Command Length */
  int i, n;

  retVal = semTake(drfSem, WAIT_FOREVER);  /* Block the resourse */
  if (retVal == ERROR)
    goto exit1;
  if (rsp == 0)                   
    rsp = rspDefault;             /* if rsp is null set it to default */
  cmdLen = strlen(cmd);           /* find out length of the command */
  if (cmdLen > MAX_CMD_LEN)
  {
    retVal = ERROR;
    goto exit;
  }

  I2Coperation(ON);
/*
  for (i = 0; i < cmdLen; i += MAX_CMD_SEG_SIZE)
  {
    n = ((cmdLen - i) > MAX_CMD_SEG_SIZE) ? MAX_CMD_SEG_SIZE : (cmdLen - i);
    printf("i%d n%d\n",i,n);
    if ((retVal = I2Cwrite2(&cmd[i], RF_ADDR, n)) != OK)
      break;
    taskDelay(10);
  }
  if (retVal == OK)
*/
  for (i = 0, retVal = ERROR; (i < 5) && (retVal == ERROR); i++)
  {    /* if error try muliple times since I2Cread can fail occasionally */
    if ((retVal = I2Cwrite2(cmd, RF_ADDR, cmdLen)) == OK)   /* send the command */
    {
/*    taskDelay((int)(0.1*sysClkRateGet()));   /* sleep for 100 ms. */
      taskDelay(2);
      rsp[0] = '?';
      rsp[1] = '?';
      rsp[2] = '?';
      rsp[3] = '?';
      retVal = I2Cread2(rsp, RF_ADDR, RSP_LEN);    /* read the response */
    }
  }
  I2Coperation(OFF);

  /* print response only for commands from console */
  if ((rsp == rspDefault) && (retVal == OK)) 
    printf("DRF response: %c%c%c%c\n",rsp[0],rsp[1],rsp[2],rsp[3]);

  exit:
  semGive(drfSem);   /* release the resource */
  exit1:
  return retVal;     
}  

/*******************************************************************************      

drfPllRefFreqSet  -  PLL Reference Frequency Get

Set the PLL reference frequency. 

*/

STATUS drfPllRefFreqSet    /* RETURN : OK, ERROR */
  (
    unsigned int freq   /* IN : the freq to set */
  )
{
  int retVal;           /* return value */
  char rsp[RSP_LEN];    /* response */

  if (freq == 0)
    retVal = drfCmd("PR10",rsp); /* set ref freq to 10Mhz */
  else if (freq == 1)
    retVal = drfCmd("PR13",rsp); /* set ref freq to 13Mhz */
  else
    retVal = ERROR;              /* invalid input */
   

  if (retVal == OK)
    retVal = rspError(rsp) ? ERROR : OK;

  return retVal;
}

/*******************************************************************************      

drfPllRefFreqGet - PLL Refrence Frequency Get

Gets the PLL refernce frequency from the Dual RF board

*/

int drfPllRefFreqGet()    /* RETURN : 0 - 10Mhz, 1 - 13Mhz, -1 - Error */
                        
{
  int retVal = -1;            
  char rsp[RSP_LEN];    /* response */

  retVal = drfCmd("PR?",rsp);

  if (retVal == OK)
  {
    if ((rsp[2] == '1') && (rsp[3] == '0'))
    {
      retVal = 0;
    }
    else if ((rsp[2] == '1') && (rsp[3] == '3'))
    {
      retVal = 1;
    }
  } 

  return retVal;
} 

/*******************************************************************************      

drfNullAttenSet - Nulliing Attenuator level Set 

Sets the Nulling attenuator level on the DRF board. 

*/

STATUS drfNullAttenSet  /* RETURN : OK, ERROR */
  (
    int attenLevel      /* IN : attenuation Level * 10 */
  )
{
  int retVal = ERROR;
  char cmdStr[6] = {'N'};     /* N??.? + null */
  char rsp[RSP_LEN];    /* response */
 
  if (attenLevel > 999)
    goto exit;
  byte2AscDec(attenLevel, &cmdStr[1]);

  /* insert decimal point */
  cmdStr[4] = cmdStr[3];
  cmdStr[3] = '.';
  cmdStr[5] = 0;

  if (drfCmd(cmdStr, rsp) == OK)
    retVal = rspOk(rsp) ? OK : ERROR;

  exit:
  return retVal;
}

/*******************************************************************************      

drfNullAttenGet - Null Attenuator level Get

Gets the null attenuator level from the DRF board

*/

STATUS drfNullAttenGet  /* RETURN : OK, ERROR */
  (
    int* level          /* OUT : Attenuator level * 10 */
  )
{
  int retVal = ERROR;   /* return value */
  char rsp[RSP_LEN];    /* response */
  int temp; 
 
  if (drfCmd("N?",rsp) == OK)
  {
    if (!(rspError(rsp)))
    {
      /* remove decimal point from the string */
      rsp[2] = rsp[1];
      rsp[1] = rsp[0];
      rsp[0] = '0';

      if (ascDec2Int(rsp, &temp) == OK)
      {
        *level = temp;
        retVal = OK;
      }
    }
  }
 
  return retVal;
}

/*******************************************************************************      

drfNullAttenMin - Null Attenuation Minimization algotrithm

Commands dual RF board to run Null Attenuation minimization algotrithm.

*/

STATUS drfNullAttenMin()  /* RETUEN : OK, ERROR */
{
  int retVal = ERROR;
  char rsp[RSP_LEN];      /* response */

  if (drfCmd("NM",rsp) == OK)
  {
    if (rspOk(rsp))
      retVal = OK;
  }
  return retVal;
}

/*******************************************************************************      

drfLockStatGet - Lock Status Get

Gets the lock status of dual RF board.

*/
int drfLockStatGet     /* RETUEN : 0 - not locked, 1 - locked, -1 - error */
  (
    PLLType pll        /* IN : pll type */
  )
{
  int retVal = 0;
  char* cmd = "";
  char rsp[RSP_LEN];    /* response */  

  switch (pll)
  {
    case Mon:
      cmd = "LDM";
      break;
    case Ref:
      cmd = "LDR";
      break;
    case T1:
      cmd = "LD1";
      break;
    case T2A:
      cmd = "LD2A";
      break;
    case T2B:
      cmd = "LD2B";
      break;
    case IF:
      cmd = "LDI";
      break;
    default:
      retVal = -1;
      break;
  }

  if (retVal != -1)
  {
    if (drfCmd(cmd, rsp) == OK)
    {
      if (rspError(rsp))
        retVal = -1;
      else if (rsp[0] == 0x00)
        retVal = 0;
      else 
        retVal = 1;
    }
    else
      retVal = -1;
  }

  return retVal;
}

/*******************************************************************************      

drfRSSIGet - RSSI (Receiver Signal Strength Indication) Get

Gets RSSI (Receiver Signal Strength Indication) from the dual RF board

*/

int drfRSSIGet()     /* RETURN : RSSI or -1 for error */
{
  int retVal = -1;
  char rsp[RSP_LEN];    /* response */

  if (drfCmd("R",rsp) == OK)
  {
    if (!(rspError(rsp)))
      if (ascHex2Int((const char*)rsp, &retVal) == ERROR)
        retVal = -1;
  }
  return retVal;  
}

/*******************************************************************************      

drfLoopbackSet - Loopback Set

Sets Loopback signal low/high on the dual RF board 

*/

STATUS drfLoopbackSet  /* RETURN : OK, ERROR */
  (
    int txNum,       /* IN : transmitter, 0 or 1  */
    int lowHigh      /* IN : 0 - set low, !0 - set high */
  )
{
  int retVal = ERROR;
  char* cmd;
  char rsp[RSP_LEN];    /* response */

  if (txNum >= 2)
    goto exit;

  if (txNum == 0)
  {
    if (lowHigh == 0)
      cmd = "LB10";
    else
      cmd = "LB11";
  }
  else 
  {
    if (lowHigh == 0)
      cmd = "LB20";
    else
      cmd = "LB21";
  }  

  if (drfCmd(cmd, rsp) == OK)
    retVal = rspOk(rsp) ? OK : ERROR;

  exit:
  return retVal;
 
}

/*******************************************************************************      

drfTxPwrSet - Tx Power Set

Sets Tx Power control low/high on the dual RF board 

*/

STATUS drfTxPwrSet     /* RETURN : OK, ERROR */
  (
    int txNum,         /* IN : transmitter, 0 or 1  */
    int lowHigh        /* IN : 0 - set low, !0 - set high */
  )
{
  int retVal = ERROR;
  char* cmd;
  char rsp[RSP_LEN];    /* response */

  if (txNum >= 2)
    goto exit;

  if (txNum == 0)
  {
    if (lowHigh == 0)
      cmd = "T10";
    else
      cmd = "T11";
  }
  else
  {
    if (lowHigh == 0)
      cmd = "T20";
    else
      cmd = "T21";
  }  

  if (drfCmd(cmd, rsp) == OK)
    retVal = rspOk(rsp) ? OK : ERROR;

  exit:
  return retVal;
}

/*******************************************************************************      

drfFpgaTxPwrEnable - FPGA Tx Power Enable

Enables FPGA interface Power Control 

*/

STATUS drfFpgaTxPwrEnable     /* RETURN : OK, ERROR */
  (
    int txNum                 /* IN : transmitter, 0 or 1  */
  )
{
  int retVal = ERROR;
  char* cmd;
  char rsp[RSP_LEN];    /* response */

  if (txNum >= 2)
    goto exit;

  if (txNum == 0)
  {
    cmd = "T1E";
  }
  else
  {
    cmd = "T2E";
  }  

  if (drfCmd(cmd, rsp) == OK)
    retVal = rspOk(rsp) ? OK : ERROR;

  exit:
  return retVal;
}
      
/*******************************************************************************      

drfFpgaTxPwrDisable - FPGA Tx Power Enable

Disables FPGA interface Power Control 

*/

STATUS drfFpgaTxPwrDisable     /* RETURN : OK, ERROR */
  (
    int txNum                 /* IN : transmitter, 0 or 1  */
  )
{
  int retVal = ERROR;
  char* cmd;
  char rsp[RSP_LEN];    /* response */

  if (txNum >= 2)
    goto exit;

  if (txNum == 0)
  {
    cmd = "T1D";
  }
  else
  {
    cmd = "T2D";
  }  

  if (drfCmd(cmd, rsp) == OK)
    retVal = rspOk(rsp) ? OK : ERROR;

  exit:
  return retVal;
}

/*******************************************************************************      

drfFreqHopEnable - Frequency Hopping Enable

This command does two things, it allows the FPGAs to manipulate the ARFCNs, 
and it also allows the FPGAs to manipulate the transmitter's power control. 
*/

STATUS drfFreqHopEnable()     /* RETURN : OK, ERROR */
{
  int retVal = ERROR;
  char rsp[RSP_LEN];    /* response */

  if (drfCmd("HE", rsp) == OK)
    retVal = rspOk(rsp) ? OK : ERROR;

  exit:
  return retVal;
}

/*******************************************************************************      

drfFreqHopDisable - Frequency Hopping Disable

This command does two things, it prevents the FPGAs from manipulating the ARFCNs, 
and it also prevents the FPGAs from manipulating the transmitter's power control
*/

STATUS drfFreqHopDisable()     /* RETURN : OK, ERROR */
{
  int retVal = ERROR;
  char rsp[RSP_LEN];    /* response */

  if (drfCmd("HD", rsp) == OK)
    retVal = rspOk(rsp) ? OK : ERROR;

  exit:
  return retVal;
}

/*******************************************************************************      

drfVersionGet - Version Get

Gets the Version Number of dual RF board firmware
*/

LOCAL STATUS drfVersionGet()     /* RETURN : OK, ERROR */
{
  int retVal = ERROR;
  char rsp[RSP_LEN];    /* response */

  if ((drfCmd("V",rsp)) == ERROR)
    goto exit;

  if (!(rspError(rsp)))
  {
    drfVersion[0] = rsp[0];
    drfVersion[1] = rsp[1];
    drfVersion[2] = rsp[2];
    drfVersion[3] = rsp[3];

    retVal = OK;
  }
  exit:
  return retVal;
}


/*******************************************************************************      

drfVersionReturn - Version Retuen

Returns the Version Number of dual RF board firmware.
*/
char* drfVersionReturn()
{
  return drfVersion;
}


/*******************************************************************************      

drfVersionPrint - Version Print

Prints the version no. of dual RF board firmware

*/

void drfVersionPrint()
{
  printf("Dual RF Module Firmware Version: %s\n", drfVersion);
}


/*******************************************************************************      

drfSerialNumGet - SerialNumber Get

Gets the Serial Number of dual RF board
*/

LOCAL STATUS drfSerialNumGet()     /* RETURN : OK, ERROR */
{
  int retVal = ERROR;
  char rsp[RSP_LEN * 3];    /* response */
  int i;

  if (drfVersionStage == 0)
  {
    if ((drfCmd("S?",rsp)) != ERROR)
    {
      if (!(rspError(rsp)))
      {
        drfSerialNum[0] = rsp[0];
        drfSerialNum[1] = rsp[1];
        drfSerialNum[2] = rsp[2];
        drfSerialNum[3] = rsp[3];
        drfSerialNum[4] = 0;      /* null terminate */
        retVal = OK;
      }
    }
  }
  else if (drfVersionStage == 1) 
  {
    if ((drfCmd("SI?", rsp)) != ERROR)
    {
      if (!(rspError(rsp)))
      {
        if ((drfCmd("SN?", &rsp[RSP_LEN])) != ERROR)
        {
          if (!(rspError(&rsp[RSP_LEN])))
          {
            for (i = 0; i < (RSP_LEN * 2); i++)
              drfSerialNum[i] = rsp[i];
            drfSerialNum[i] = 0;  /* null terminate */ 
            retVal = OK;
          }
        }
      }
    }
  }
  else 
  {
    if ((drfCmd("SI?", rsp)) != ERROR)
    {
      if (!(rspError(rsp)))
      {
        if ((drfCmd("SN?", &rsp[RSP_LEN])) != ERROR)
        {
          if (!(rspError(rsp)))
          {
            if ((drfCmd("SX?", &rsp[RSP_LEN * 2])) != ERROR)
            {
              if (!(rspError(&rsp[RSP_LEN])))
              {
                for (i = 0; i < (RSP_LEN * 3); i++)
                  drfSerialNum[i] = rsp[i];
                drfSerialNum[i] = 0;  /* null terminate */ 
                retVal = OK;
              }
            }
          }
        }
      }
    }
  }

  
  return retVal;
}


/*******************************************************************************      

drfSerialNumPrint - Serial Number Print

Prints the Serial Number of dual RF board 

*/

void drfSerialNumPrint()
{
  printf("Dual RF Module Serial Number: %s\n", drfSerialNum);
}


/*******************************************************************************      

drfSerialNumReturn - Serial Number Return

Returns the Serial Number of dual RF board.

*/
char* drfSerialNumReturn()
{
  return drfSerialNum;
}


/*******************************************************************************      

drfInfoSet - Information Set

Detrmines and saves the type of the RF module present in the system.

Sets the drfType to following:
        0 -  900 system
        1 - 1800 system
        2 - 1900 system 
       -1 - Undtermined

Determines the stage of version of firmware on the RF board and sets
drfVersionStage. 

Determines the version of the memory layout on the rf board and sets
drfMemLayoutVersion

drfVersionStage and drfMemLayoutVersion are for the internal use only.
*/

/* 
The type of the system is figured out from the version number of the 
drf firmare. The version of the layout is also figured out from the firmware 
version number. 

If the version no. begins with a letter, the letter indiacates type 
of the board. Px.x is 1900, Dx.x is 1800, Gx.x is 900, and Ex.x is extended gsm
system. If there is no leading letter, the leading digit is 0. Versions 01.x is 
for 1900 systems and versions 02.x is for 1800 systems 
*/

/* 
If version is P1.x, D1.x, G1.x, E1.x, 01.x, or 02.x the drfVersion stage 
should be set to 0. This versions are no longer built. 
If the version id P2.x, D2.x, G2.x, or E2.x the drfVersionStage is set to 1 
*/

LOCAL void drfInfoSet()      
{
  drfType = -1;     /* undertermined type */
  drfVersionStage = 2;   /* assume newest version */

  if (drfVersion[0] == 'P')
    drfType = 2;    /* 1900 */
  else if (drfVersion[0] == 'D')
    drfType = 1;    /* 1800 */
  else if ((drfVersion[0] == 'G') || (drfVersion[0] == 'E'))
    drfType = 0;    /*  900 */
  else if (drfVersion[0] == '0')
  {
    if (drfVersion[1] == '1')
      drfType = 2;   /* 1900 */
    else if (drfVersion[1] == '2') 
      drfType = 1;   /* 1800 */
    drfVersionStage = 0;     /* old version */
  }

  if (drfVersionStage != 0)
  {                  /* version is not 0x.x */
    if (drfVersion[1] == '1')
      drfVersionStage = 0;     /* old version */
    else if (drfVersion[1] == '2')
    {
      if (drfVersion[3] < '2')
        drfVersionStage = 1;   /* old version */
    }
  }
  

  if (drfVersionStage == 0)
  {
    drfMemLayoutVer = 0;
    drfMemSize = 22;
  }
  else if (drfVersionStage == 1)
  {
    drfMemLayoutVer = 1;
    drfMemSize = 24;
  }
  else
  {
    drfMemLayoutVer = 2;
    drfMemSize = 26;
  }
}


/*******************************************************************************      

drfTypeGet - Type Get

Returns type of the RF module present in the system.
Returns
        0 -  900 system
        1 - 1800 system
        2 - 1900 system 
       -1 - Undetermined
*/
int drfTypeGet()     /* RETURN: 0,1,2, or -1 */
{
  return drfType;
}

/*******************************************************************************      

drfIQSwapGet - IQ Swap Get

Returns TRUE if I/Q signals are swapped on the RF board. The swapping is as 
compared to the original 1900 board. 
*/
int drfIQSwapGet()      /* RETURN: TRUE / FALSE */
{
  if ((drfTypeGet() == 2) && (drfVersion[0] == '0'))
    return FALSE;
  else
    return TRUE;
}

/*******************************************************************************      

drfFreqHopGet - Frequency Hop Get

Returns if the TRX is capable of hopping.
*/
int drfFreqHopGet        /* RETURN: TRUE /FALSE */
  (
    UINT  trx            /* IN: trx number */
  )
{
  return FALSE;     /* no hopping as of yet */
}

/*******************************************************************************      

drfMemSend - Memory Send

Sends dual RF board contents of memory on that board

*/

LOCAL STATUS drfMemSend()     /* RETURN : ok/error */
{

  STATUS retVal = ERROR;
  char  cmdStr[1 + MAX_RF_MEM_SIZE *2 + 1]  = {'Q'};
  char  rsp[RSP_LEN];    /* response */
  char* pCmd;
  int i;

  /* build the command string */
  for (i = 0, pCmd = &cmdStr[1];  i < drfMemSize; i++, pCmd+=2)
  {
    byte2AscHex(drfMem[i], pCmd);
  }
  cmdStr[drfMemSize * 2 + 1] = 0; /* null terminate */
  /* send the command */
  if (drfCmd(cmdStr, rsp) == OK)
  {
    retVal = rspOk(rsp) ? OK : ERROR;
  }
  exit:
  return retVal;  

}

/*******************************************************************************      

drfStatGet - Status Get

Gets status of dual RF board.

*/

STATUS drfStatGet                 /* RETURN: OK ro ERROR */
                  ( 
                   int* pStatus   /* OUT: drf status here */
                  )
{
  STATUS retVal = ERROR;
  char rsp[RSP_LEN];    /* response */

  if (pStatus != NULL)
  {
    if (drfCmd("?",rsp) == OK)
    {
      if (ascHex2Int(rsp, pStatus) == OK)
      {
        retVal = OK;
      }
    }
  }
  return retVal;
}




/*******************************************************************************      

drfStatGet - Status Get

Gets status of dual RF board.

*/

long drfSynthStatusGet(void )
{
  long retVal = 0xFFFFFFFF;
  char rsp[RSP_LEN];    /* response */

  if (drfCmd("?",rsp) == OK)
  {
     /*retVal = rsp[0] | (rsp[1]<<8) | (rsp[2]<<16) | (rsp[3]<<24);*/
     ascHex2Int(rsp, & retVal);
  }

  return retVal;
}
/*******************************************************************************

drfInterfaceInit - Interface Initialize

Initializes Dual RF interface

*/
STATUS drfInterfaceInit()
{
  drfSem = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);
  if (drfSem == NULL)
    return ERROR;
  else
    return OK;
}

/*******************************************************************************      

drfInit - Initialize

Initializes dual RF board interface, reads the EEPROM on the RF board, and sends
the values to the dual RF borad. The host reads the memory on behalf of the dual
radio board to avoid multiple masters on the I2C bus. The I2C interface on MPC860 
processor has known issues related to multi-master mode operation.

*/

STATUS drfInit()

{
  STATUS  retVal = ERROR;
  int  status;   /* drf status */
  int i;

  retVal = drfInterfaceInit();
  if (retVal == ERROR)
    goto exit;

  drfVersionGet();    /* get the Dual RF firmware version */
  drfInfoSet();       /* set the RF board firmware version no. etc. */
    
  /* read the dual RF EEPROM */
  I2Coperation(ON);
  /* if error try muliple times since I2Cread can fail occasionally */
  for (i = 0, retVal = ERROR; (i < 5) && (retVal == ERROR); i++)
    retVal = I2Cread(drfMem, RF_MEM_ADDR, 0, drfMemSize); 
  I2Coperation(OFF);
  if (retVal == ERROR)
    goto exit;

  /* send the read values to the drf board */
  retVal = drfMemSend();   

  drfSerialNumGet();  /* get the Dual RF serial number */
  drfStatGet(&status);   /* get the Dual RF board's status */
  if ((status & EEPROM_REQUIRED) != 0)
    printf("Error: Dual RF did not receive EEPROM values\n");
  if ((status & EEPROM_DEFAULT_USED) != 0)
    printf("Error: Dual RF using default EEPROM settings\n");
 
  /* Enable frequency hopping. 
  This enables the FPGA interface to the RF module.
  This command does two things, it allows the FPGAs to manipulate the ARFCNs, 
  and it also allows the FPGAs to manipulate the transmitter's power control.
  */
  if ((drfFreqHopEnable()) == ERROR)
    goto exit;

  /* Enable the PA on transceivers.
  The actual setting of the PA power enable control will be determined by the 
  FPGA interface.
  */
  if ((drfFpgaTxPwrEnable(0)) == ERROR)
    goto exit;
  if ((drfFpgaTxPwrEnable(1)) == ERROR)
    goto exit;

  exit:
  return retVal;
}

/*---------------------------------------------------------------------------*/
/* Following commands read and write data to the EEPROM on the dual RF board */
/*---------------------------------------------------------------------------*/

/* RF EEPROM Layout */
/* To be indexed by drfMemLayoutVersion */

UINT DACI1[]            = {0x00, 0x0B, 0x0B};
UINT DACQ1[]            = {0x01, 0x0C, 0x0C};
UINT DACGREF1[]         = {0x02, 0x0D, 0x0D};
UINT DACDELAY[]         = {0x03, 0x0E, 0x0E};
UINT DACVCXO[]          = {0x04, 0x0F, 0x0F};
UINT DACGREF2[]         = {0x05, 0x10, 0x10};
UINT DACQ2[]            = {0x06, 0x11, 0x11};
UINT DACI2[]            = {0x07, 0x12, 0x12};
UINT NULL_FFEDBACK[]    = {0x08, 0x13, 0x13};
UINT ARFCN1_LSB[]       = {0x09, 0x00, 0x00};
UINT ARFCN1_MSB[]       = {0x0A, 0x01, 0x01};
UINT ARFCN2A_LSB[]      = {0x0B, 0x02, 0x02};
UINT ARFCN2A_MSB[]      = {0x0C, 0x03, 0x03};
UINT ARFCN2B_LSB[]      = {0x0D, 0x04, 0x04};
UINT ARFCN2B_MSB[]      = {0x0E, 0x05, 0x05};
UINT ARFCN_MON_LSB[]    = {0x0F, 0x06, 0x06};
UINT ARFCN_MON_MSB[]    = {0x10, 0x07, 0x06};
UINT RFF_FREQ_INPUT[]   = {0x11, 0x08, 0x08};
UINT REF_FREQ_MODE[]    = {0x12, 0x09, 0x09};
UINT SERIAL_INFO_LSB[]  = {0x13, 0x14, 0x14};
UINT SERIAL_INFO_MSB[]  = {0x14, 0x15, 0x15};
UINT CHECKSUM[]         = {0x15, 0x0A, 0x0A};
UINT SERIAL_NUM_LSB[]   = {0xFF, 0x16, 0x16};   /* Invalid for index 0 */
UINT SERIAL_NUM_MSB[]   = {0xFF, 0x17, 0x17};   /* Invalid for index 0 */
UINT SERIAL_XN_LSB[]    = {0xFF, 0xFF, 0x18};   /* Invalid for index 0 */
UINT SERIAL_XN_MSB[]    = {0xFF, 0xFF, 0x19};   /* Invalid for index 0 */

/*-----------------------------------------------------------------
drfMemUpdate - Memory Update
Updates given address of EEPROM in DRF module with the given value.
In addtion to this updates checksum field in the memory if required.
If the adress of the memory being updated is less than the address of
the chekcsum than the checksum is updates.
*/
LOCAL STATUS drfMemUpdate(char addr, char value)
{
  STATUS retStat = ERROR;
  char oldValue;
  char checksum;

  if ((addr < drfMemSize) && (addr != CHECKSUM[drfMemLayoutVer]))
  {
    I2Coperation(ON);
    if ((I2Cread(&oldValue, RF_MEM_ADDR, addr, 1)) == OK)
    {
      if ((I2Cwrite(&value, RF_MEM_ADDR, addr, 1)) == OK)
      {
        taskDelay(2);
        if (addr < CHECKSUM[drfMemLayoutVer])
        {
          I2Cread(&checksum, RF_MEM_ADDR, CHECKSUM[drfMemLayoutVer], 1);
          checksum = checksum + (oldValue - value);
          I2Cwrite(&checksum, RF_MEM_ADDR, CHECKSUM[drfMemLayoutVer], 1);
          taskDelay(2);
        }
        retStat = OK;
      }
    }
    if (retStat == ERROR)
      printf("Error accessing DRF Memory\n");
    I2Coperation(OFF);
  }
  else
  {
    printf("Error: Illegal DRF memory address specified for update\n"); 
  }

  return retStat;
}

/*-----------------------------------------------------------------
drfMemDacI1Write - Memory DAC I1 Write
Writes DAC I1 trim value in the drf EEPROM
*/
STATUS drfMemDacI1Write(char newI1)
{
  return drfMemUpdate(DACI1[drfMemLayoutVer], newI1);
}

/*-----------------------------------------------------------------
drfMemDacQ1Write - Memory DAC Q1 Write
Writes DAC Q1 trim value in the drf EEPROM
*/
STATUS drfMemDacQ1Write(char newQ1)
{
  return drfMemUpdate(DACQ1[drfMemLayoutVer], newQ1);
}

/*-----------------------------------------------------------------
drfMemDacI2Write - Memory DAC I2 Write
Writes DAC I2 trim value in the drf EEPROM
*/
STATUS drfMemDacI2Write(char newI2)
{
  return drfMemUpdate(DACI2[drfMemLayoutVer], newI2);
}

/*-----------------------------------------------------------------
drfMemDacQ2Write - Memory DAC Q2 Write
Writes DAC Q2 trim value in the drf EEPROM
*/
STATUS drfMemDacQ2Write(char newQ2)
{
  return drfMemUpdate(DACQ2[drfMemLayoutVer], newQ2);
}

/*-----------------------------------------------------------------
drfMemLocRead - Memory Location Read
Reads the given memory location of EEPROM on the DRF module
*/
LOCAL STATUS drfMemLocRead(char addr, char* pValue)
{
  STATUS retStat;

  I2Coperation(ON);
  retStat = I2Cread(pValue, RF_MEM_ADDR, addr,1);
  I2Coperation(OFF);
  if (retStat == ERROR)
    printf("Error reading from DRF memory\n");
  return retStat;
}

/*-----------------------------------------------------------------
drfMemDacI1Print - Memory DAC I1 Print
Prints the DAC I1 trim value in drf EEPROM
*/
void drfMemDacI1Print()
{
  char value;

  if ((drfMemLocRead(DACI1[drfMemLayoutVer], &value)) == OK) 
  {
    printf("DRF DAC I1 in EEPROM: 0x%02x\n",value); 
  }
}

/*-----------------------------------------------------------------
drfMemDacQ1Print - Memory DAC Q1 Print
Prints the DAC Q1 trim value in drf EEPROM
*/
void drfMemDacQ1Print()
{
  char value;

  if ((drfMemLocRead(DACQ1[drfMemLayoutVer], &value)) == OK) 
  {
    printf("DRF DAC Q1 in EEPROM: 0x%02x\n", value); 
  }
}

/*-----------------------------------------------------------------
drfMemDacI2Print - Memory DAC I2 Print
Prints the DAC I2 trim value in drf EEPROM
*/
void drfMemDacI2Print()
{
  char value;

  if ((drfMemLocRead(DACI2[drfMemLayoutVer], &value)) == OK)
  {
    printf("DRF DAC I2 in EEPROM: 0x%02x\n",value); 
  }
}

/*-----------------------------------------------------------------
drfMemDacQ2Print - Memory DAC Q2 Print
Prints the DAC Q2 trim value in drf EEPROM
*/
void drfMemDacQ2Print()
{
  char value;

  if ((drfMemLocRead(DACQ2[drfMemLayoutVer], &value)) == OK)
  {
    printf("DRF DAC Q2 in EEPROM: 0x%02x\n",value);
  }
}

/*-----------------------------------------------------------------
drfMemSerialNumPrint - Memory Serial Number Print
Prints the serial number in the drf EEPROM
*/
void drfMemSerialNumPrint()
{
  char numLsb;
  char numMsb;
  char serialNum[SER_NUM_LEN + 1];

/* 
Serial Number Format:
For memLayaoutVersion 0
16 bits: SERIAL_INFO

For memLayoutVesrion 1
32 bits: SERIAL_INFO and SERIAL_NUM. INFO has MSB and NUM has LSB

other memLayoutVersion
48 bits: SERIAL_INFO, SERIAL_NUM, and SERIAL_SX. INFO has MSB and SX has LSB
*/
  if (((drfMemLocRead(SERIAL_INFO_LSB[drfMemLayoutVer], &numLsb)) == OK) && 
      ((drfMemLocRead(SERIAL_INFO_MSB[drfMemLayoutVer], &numMsb)) == OK) )
  {
    sprintf(serialNum, "%04x", (numMsb << 8) | numLsb);
    if (drfMemLayoutVer == 0)
      printf("DRF Serial Number in EEPROM: 0x%s\n", serialNum);
    else
      if (((drfMemLocRead(SERIAL_NUM_LSB[drfMemLayoutVer], &numLsb)) == OK) && 
          ((drfMemLocRead(SERIAL_NUM_MSB[drfMemLayoutVer], &numMsb)) == OK) )
      {
        sprintf(serialNum, "%s%04x", serialNum, (numMsb << 8) | numLsb);
        if (drfMemLayoutVer == 1) 
          printf("DRF Serial Number in EEPROM: 0x%s\n", serialNum);
        else
          if (((drfMemLocRead(SERIAL_XN_LSB[drfMemLayoutVer], &numLsb)) == OK) && 
             ((drfMemLocRead(SERIAL_XN_MSB[drfMemLayoutVer], &numMsb)) == OK) )
            printf("DRF Serial Number in EEPROM: %s%04x\n", serialNum, 
                    (numMsb << 8) | numLsb);  
      }
  }
  else
  {
    printf("Error reading from DRF memory\n");
  }
}

/*-----------------------------------------------------------------
drfMemSerialNumWrite -  Memory Serial Number Write
Writes the given serial number in the drf EEPROM 
*/

/* No Longer Available */

/*-----------------------------------------------------------------
drfMemChecksumCompute - Memory Checksum Compute
Computes the checksum of the data in drf EEPROM
*/
LOCAL STATUS drfMemChecksumCompute(char* pChecksum)
{
  STATUS retStat;
  char drfEeprom[MAX_RF_MEM_SIZE - 1];
  int i;

  I2Coperation(ON);
  retStat = I2Cread(drfEeprom, RF_MEM_ADDR, 0, CHECKSUM[drfMemLayoutVer]);
  I2Coperation(OFF);
  for (i = 0, *pChecksum =0; i < CHECKSUM[drfMemLayoutVer]; i++)
  {
    *pChecksum += drfEeprom[i];
  }
  *pChecksum = 0 - *pChecksum;
  return retStat;
}

/*-----------------------------------------------------------------
drfMemChecksumCheck - Memory Checksum Check
Checks if the cheksum in the drf EEPROM is correct.
Also prints the cheksum the in drf EEPROM
*/
STATUS drfMemChecksumCheck()
{
  STATUS retStat = ERROR;
  char checksum;
  char checksumMem;

  if (((drfMemChecksumCompute(&checksum)) == OK) &&
      ((drfMemLocRead(CHECKSUM[drfMemLayoutVer], &checksumMem)) == OK))    
  {
    if ((checksum - checksumMem) == 0)
      printf("DRF EEPROM checksum is correct\n");
    else
      printf("DRF EEPROM checksum is incorrect\n");
    printf("DRF EEPROM checksum: 0x%02x\n",checksumMem);
    retStat = OK;
  }
  else
  {
    printf("Error reading from DRF memory\n");
  }
  return retStat;
}


/*-----------------------------------------------------------------
drfMemCheksumFix - Memory Checksum Fix
Fixes cheksum in drf EEPROM by computing and writing the new checksum
*/
STATUS drfMemChecksumFix()
{
  STATUS retStat;
  char checksum;

  retStat = drfMemChecksumCompute(&checksum);
  I2Coperation(ON);
  retStat = I2Cwrite(&checksum, RF_MEM_ADDR, CHECKSUM[drfMemLayoutVer], 1);
  I2Coperation(OFF);
  if (retStat == ERROR)
    printf("Error accessing DRF Memory\n");
  return retStat;
}

