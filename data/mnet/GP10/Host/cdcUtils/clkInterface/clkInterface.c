/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*********************************************************************

clkInterface  -  Clock Interface

This file consists a set of functions to interface with the Clock Module. 

Before any of the functions can be used the interface needs to be initialized with 
function clkInit().

All the clock interface functions make use of the function clkCmd() to send a 
command to the clock module. clKCmd() sends the given command to the clock module
and reads the response after 320 ms. This function can also be used to send
a command to the clock module from the console.

A function clkNumGet is used by other funcion that need to send a command and read
a numerical value from the response.

All the commands to the clock module are ASCII coded. Each command has an ASCII
coded response. The clock module asssumes that the commands are being sent from
a console and sends a prompt at the end of all responses. The command charaters are
not echoed by the clock module. Each command is terminated with a 
'carraige return' character.
*/

#include <vxWorks.h>
#include <ioLib.h>
#include <semLib.h>
#include "cdc_bsp/nvRam.h"

#define MAX_CMD_LEN 30
#define MAX_RSP_LEN 30
#define NUM_VERSION_STAGES 2
#define SERIAL_NUM_LEN 15
#define EEPROM_SIZE_REVC CLOCK_BOARD_EEPROM_SIZE
#define EEPROM_SIZE_REV1_8  15

/*---------- Globals ----------*/
char clkSerialNum[SERIAL_NUM_LEN + 1] = "xxxxxxxxxxxxxxx";
char clkSoftVersion[4] = "x.x";
int clkVerifyEepromOn = 1;

/*-------- File Locals --------*/
static SEM_ID clkSem;
static int clkFd;
static int clkVerStage;

LOCAL STATUS clkSoftVersionGet();

/**********************************************************************
clkInterfaceInit  -  Interface Initialize

Inititlaizes the clock module interface..
*/

/* This routine opens the serial port for the clock module */

static STATUS clkInterfaceInit()        /* RETURN: OK or ERROR */
{
  STATUS retStat = ERROR;

  /* create a semaphore for mutual exclusion */
  clkSem = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);
  if (clkSem != NULL)
  {
    clkFd = open("/tyCo/1", O_RDWR, 0);     /* open the serial port */
    if (clkFd != ERROR)
    {
      /* The response to first command to clock card is discoverd to be just a prompt.
         Issue a dummy command so that the first real command is not the first command */
      write(clkFd, "\r", 1);          /* send a carraige return */
      taskDelay(2);                   /* wait for the response */
      ioctl(clkFd, FIORFLUSH, 0);     /* clear the 'read' buffer */      
      retStat = OK;
    }
  }
  return retStat;
}


/**********************************************************************
clkInit  -  Initialize

Inititlaizes the clock module interface and reads and saves the clock module
serial number. 
This function should be called  once before any commands can be 
sent to the clock module.
*/

/* This routine opens the serial port for the clock module */

STATUS clkInit()        /* RETURN: OK or ERROR */
{
  STATUS retStat = ERROR;

  clkVerStage = NUM_VERSION_STAGES - 1;  /* assume latest */
  if (clkInterfaceInit() == OK)           /* intitlize the interface */
  {
    if (clkSoftVersionGet() == OK)         /* read the software version number */
    {
      if ((clkSoftVersion[0] >= '0') && (clkSoftVersion[0] <= '9'))
      {            /* valid number */
        if (clkSoftVersion[0] >= '2')
          clkVerStage = 1;      /* version >= 2.x */
        else
          clkVerStage = 0;

        if (clkVerifyEepromOn) clkValidateEeprom();

        if (clkSerialNumGet() == OK) /* read the serial number */
          retStat = OK;
      }
    }
  }

  return retStat;
}


/**********************************************************************
clkSerialNumReturn  -  Serial Number Return

Returns the Serial Number of the clock module.
*/
char* clkSerialNumReturn()
{
  return clkSerialNum;
}


/**********************************************************************
clkSerialNumPrint  -  Serial Number Print

Prints the Serial Number of the clock module.
*/
clkSerialNumPrint()
{
  printf("Clock Module Serial Number: %s\n", clkSerialNum);
}


/**********************************************************************
clkSoftVersionReturn  -  Software Version Return

Returns the Software Version of the clock module.
*/
char* clkSoftVersionReturn()
{
  return clkSoftVersion;
}


/**********************************************************************
clkSoftVersionPrint  -  Software Version Print

Prints the Software Version of the clock module.
*/
clkSoftVersionPrint()
{
  printf("Clock Module Software Version: %s\n", clkSoftVersion);
}


/**********************************************************************
clkCmd  -  Command

This function sends a command to the clock module and returns the response.
It provides interface to clock module for other functions. It can also be 
used for sending a command to the clock module from console.

This function is used by all other functions to send a command to the clock 
module. Normally this function should not called directly. Instead one of 
the specicfic functions that make use of this function should be called.

If the pointer to response is 0, it is assumed that the command was issued 
from the console the response is printed on the standard output.

This functions waits for 320 ms.to check if a response is available. Error is 
returnrd if there is no response.
*/

/*
All the responses from clock module are begin with cr-lf characters and the response ends
with a cr-lf chracters and a prompt character.
*/

STATUS clkCmd(                     /* RETURN:  OK / ERROR */
               const char* cmd,    /* IN : command */
               char* rsp           /* OUT: response */
             )
{
  STATUS retStat = ERROR;
  UINT cmdLen;
  UCHAR rspBuff[MAX_RSP_LEN + 1];   /* + 1 for lf/cr */
  UINT nBytesUnread;
  UINT i;
  
/*  retStat = semTake(clkSem, WAIT_FOREVER);  /* Block the resourse */
  if ((semTake(clkSem, WAIT_FOREVER)) != ERROR)
  {
    cmdLen = strlen(cmd);          
    ioctl(clkFd, FIORFLUSH, 0);         /* clear read buffer */
    write(clkFd, (char*) cmd, cmdLen);  /* send the command */
    write(clkFd, "\r", 1);              /* end with carriage return */
   
    taskDelay(10);                      /* wait for response */
    ioctl(clkFd, FIONREAD, (int)&nBytesUnread);   /* check if anything in the read buffer */
    if (nBytesUnread > 0)
    {                /* Something to read */
      read(clkFd, rspBuff, 2);             /* read CR-LF */
      read(clkFd, rspBuff, MAX_RSP_LEN);   /* read the ascii string */
           /* loop to find the end of the response */
      for (i = 0; i < MAX_RSP_LEN; i++)
      {
        if ((rspBuff[i] == 0x0A) || (rspBuff[i] == 0x0D))   /* CR/LF ? */
        {
          rspBuff[i] = NULL;         /* change to  null */
          if (rsp != 0)         
          {        /* valid rsp pointer */
            memcpy(rsp, rspBuff, i);
            rsp[i] = NULL;      /* null terminate */
          }
          else
          {        /* command must be from console */
            printf("CLK Response: %s\n", rspBuff);
          }
          retStat = OK; 
          break;
        }
      } /* for (i */
    } /* if (nBytes */
  } /* if (retStat */
  semGive(clkSem);        /* release the resource */
  return retStat;
}


/**********************************************************************
clkNumGet - Number Get

Sends a command to clock module and reads the response.
This fuctions is used by other functions requiring to send a command to
the clock module and reading an unsigned numerical value back. 
Returns the number or -1 for error. 
*/

/*
The response consists of a series of chracters followed by the ascii coded digits.
The rspExpected does not include the digits. Number of digits can vary. The maximum 
number of digits is numDigits.
*/

STATUS clkNumGet(                        /* RETURN : number or -1 for ERROR */
               const char* cmd,          /* IN: commnad */
               const char* rspExpected,  /* IN: expected response */
               int   numDigits,          /* IN: maximum number of digits at the end */ 
               int* num                  /* OUT: returned value, valid if STATUS=OK */
             )          
{
  char rsp[MAX_RSP_LEN + 1];          /* + 1 for null */
  int  i;
  int  errorFound;           /* boolean: error found in response */
  const char* pRsp;
  int rspExpectedLen;

  if ((clkCmd(cmd, rsp)) == OK)
  {            /* command successful */
    rspExpectedLen = strlen(rspExpected);
    if (memcmp(rsp, rspExpected, rspExpectedLen) == 0) 
    {            /* response is expected response */  
             /* search for end of command and compute number */
             /* read characters from 'left' */
      for (i = 0, *num = 0, errorFound = FALSE, pRsp = &rsp[rspExpectedLen];
           (i < numDigits) && (*pRsp != 0) && (errorFound == FALSE);
           i++, pRsp++ )
      {
        if ((*pRsp >= '0') && (*pRsp <= '9'))
        {
          *num = *num * 10 + (*pRsp - '0');  /* convert from ASCII */
        }
        else 
          errorFound = TRUE;         /* invalid character */
      }

      if ((i == 0) || (*pRsp != 0))  /* No digits or response too long */
        errorFound = TRUE;   

      if (errorFound == FALSE)
        return OK;     /* return offset */
      
    }
  }
  return ERROR;     /* return ERROR */

}


/**********************************************************************
clkOffsetSet - Offset Set

Sends a command to clock module to set the given offset. 
Return OK or ERROR.
*/

/*
The command sent to the clock module is $PTELS,OFFSET,xxx / $PTELS,OFFS,xxx
The expected response is $PTELA,OFFSET,xxx / $PTELA,OFFS,xxx
*/

STATUS clkOffsetSet(               /* RETURN: OK / ERROR */
                    UINT offset    /* IN: offset */
                   )
{
  const char* cmdSet[] = {"$PTELS,OFFSET,","$PTELS,OFFS,"};
  const char* rspExpectedSet[] = {"$PTELA,OFFSET,","$PTELA,OFFS,"};

  char cmd[MAX_CMD_LEN + 1];            /* + 1 for null */
  char rspExpected[MAX_RSP_LEN + 1];    /* + 1 for null */
  char rsp[MAX_RSP_LEN + 1];            /* + 1 for null */

  if (offset <= 127)
  {
    /* select command and append the offset in ASCII */
    sprintf(cmd, "%s%d", cmdSet[clkVerStage], offset);
    sprintf(rspExpected, "%s%d", rspExpectedSet[clkVerStage], offset);
    
    if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
    {                                       
      if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
        return OK;
    }
  }
  return ERROR;
}


/**********************************************************************
clkOffsetGet - Offset Get

Sends a command to clock module to read the offset. 
Returns the offset or -1 for error. 
*/

/*
The command sent to the clock module is $PTELQ,OFFSET / $PTELQ,OFFS
The expected response is $PTELR,OFFSET,xxx / $PTELR,OFFS,xxx  
*/

int clkOffsetGet()       /* RETURN: offset or -1 for error */
{
  const char* cmdSet[] = {"$PTELQ,OFFSET","$PTELQ,OFFS"};
  const char* rspExpectedSet[] = {"$PTELR,OFFSET,","$PTELR,OFFS,"};
  int num;

  if (clkNumGet(cmdSet[clkVerStage], rspExpectedSet[clkVerStage], 3, &num) != OK)
  {
    num = -1;
  }
  return (num);
}


/**********************************************************************
clkOscillatorSet - Oscillator Type Set

Sends a command to clock module to set the given Oscillator Type. 
Return OK or ERROR.
*/

/*
The command sent to the clock module is $PTELS,OSC,xxx
The expected response is $PTELA,OSC,xxx
*/

STATUS clkOscillatorSet(               /* RETURN: OK / ERROR */
                    UINT type    /* IN: offset */
                   )
{
  char cmd[MAX_CMD_LEN + 1];            /* + 1 for null */
  char rspExpected[MAX_RSP_LEN + 1];    /* + 1 for null */
  char rsp[MAX_RSP_LEN + 1];            /* + 1 for null */

  if (type <= 127)
  {
    /* select command and append the offset in ASCII */
    sprintf(cmd, "$PTELS,OSC,%d", type);
    sprintf(rspExpected, "$PTELA,OSC,%d", type);
    
    if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
    {                                       
      if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
        return OK;
    }
  }
  return ERROR;
}


/**********************************************************************
clkBoardRevSet - Board Revision Set

Sends a command to clock module to set the given Board Revision. 
Return OK or ERROR.
*/

/*
The command sent to the clock module is $PTELS,BOARD,xxx / $PTELS,BRD,xxx
The expected response is $PTELA,BOARD,xxx / $PTELA,BRD,xxx
*/

STATUS clkBoardRevSet(               /* RETURN: OK / ERROR */
                    UINT rev    /* IN: offset */
                   )
{
  const char* cmdSet[] = {"$PTELS,BOARD,","$PTELS,BRD,"};
  const char* rspExpectedSet[] = {"$PTELA,BOARD,","$PTELA,BRD,"};

  char cmd[MAX_CMD_LEN + 1];            /* + 1 for null */
  char rspExpected[MAX_RSP_LEN + 1];    /* + 1 for null */
  char rsp[MAX_RSP_LEN + 1];            /* + 1 for null */

  /* select command and append the offset in ASCII */
  sprintf(cmd, "%s%d", cmdSet[clkVerStage], rev);
  sprintf(rspExpected, "%s%d", rspExpectedSet[clkVerStage], rev);
  
  if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
  {                                       
    if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
      return OK;
  }
  return ERROR;
}


/**********************************************************************
clkChecksumSet - Checksum Set

Sends a command to clock module to set the given checksum value. 
Return OK or ERROR.
*/

/*
The command sent to the clock module is $PTELS,CHECKSUM,xxx / $PTELS,CKSM,xxx
The expected response is $PTELA,CHECKSUM,xxx / $PTELA,CKSM,xxx
*/

STATUS clkChecksumSet(               /* RETURN: OK / ERROR */
                    UINT checksum    /* IN: offset */
                   )
{
  const char* cmdSet[] = {"$PTELS,CHECKSUM","$PTELS,CKSM"};
  const char* rspExpectedSet[] = {"$PTELA,CHECKSUM,","$PTELA,CKSM,"};

  char cmd[MAX_CMD_LEN + 1];            /* + 1 for null */
  char rspExpected[MAX_RSP_LEN + 1];    /* + 1 for null */
  char rsp[MAX_RSP_LEN + 1];            /* + 1 for null */

  /* select command and append the offset in ASCII */
  sprintf(cmd, "%s", cmdSet[clkVerStage]);
  sprintf(rspExpected, "%s%d", rspExpectedSet[clkVerStage], checksum);
  
  if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
  {                                       
    if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
      return OK;
  }
  return ERROR;
}


/**********************************************************************
clkOscillatorTransferFuncSet - Oscillator Transfer Function Set

Sends a command to clock module to set the given Oscillator transfer function. 
Return OK or ERROR.
*/

/*
The command sent to the clock module is $PTELS,POS,0 / $PTELS,NEG,1
The expected response is $PTELA,POS,0 / $PTELA,NEG,1
*/

STATUS clkOscillatorTransferFuncSet(               /* RETURN: OK / ERROR */
                    UINT val           /* IN: value */
                   )
{
    char cmd[MAX_CMD_LEN + 1];            /* + 1 for null */
    char rspExpected[MAX_RSP_LEN + 1];    /* + 1 for null */
    char rsp[MAX_RSP_LEN + 1];            /* + 1 for null */

    if (val == 0)
    {
        /* select command and append the offset in ASCII */
        sprintf(cmd, "$PTELS,POS");
        sprintf(rspExpected, "$PTELA,POS,0");
    }
    else
    {
        /* select command and append the offset in ASCII */
        sprintf(cmd, "$PTELS,NEG");
        sprintf(rspExpected, "$PTELA,NEG,1");
    }
    
    if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
    {                                       
        if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
            return OK;
    }

    return ERROR;
}


/**********************************************************************
clkClockSelectSet - Clock Select Set

Sends a command to clock module to set the given Clock select value. 
Return OK or ERROR.
*/

/*
The command sent to the clock module is $PTELS,E1 / $PTELS,T1
The expected response is $PTELA,E1,1 / $PTELA,T1,0
*/

STATUS clkClockSelectSet(               /* RETURN: OK / ERROR */
                    UINT val           /* IN: value */
                   )
{
    char cmd[MAX_CMD_LEN + 1];            /* + 1 for null */
    char rspExpected[MAX_RSP_LEN + 1];    /* + 1 for null */
    char rsp[MAX_RSP_LEN + 1];            /* + 1 for null */

    if (val == 0)
    {
        /* select command and append the offset in ASCII */
        sprintf(cmd, "$PTELS,T1");
        sprintf(rspExpected, "$PTELA,T1,0");
    }
    else
    {
        /* select command and append the offset in ASCII */
        sprintf(cmd, "$PTELS,E1");
        sprintf(rspExpected, "$PTELA,E1,1");
    }
    
    if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
    {                                       
        if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
            return OK;
    }

    return ERROR;
}


/**********************************************************************
clkSerialNumSet - Serial Number Set

Sends a command to clock module to set the serial number 
Returns the offset OK or ERROR. 
*/

/* No Longer Available */


/**********************************************************************
clkSerialNumGet - Serial Number Get

Sends a command to clock module to read the offset. 
Returns the serial number or -1 for error.
*/

/*
for clkVerStage 0
The command sent to the clock module is $PTELQ,SERIAL
The expected response is $PTELR,SERIAL,xxx  

for other clkVerStage
The commands sent to the clock module are $PTELQ,SA $PTELQ,SB $PTELQ,SC
The expected responses are $PTELR,SA,xxx $PTELR,SB,xxx $PTELR,SC,xxx 
The most significant bits are SA and the least significant bits are SC
*/

STATUS clkSerialNumGet()         /* RETURN: OK/ERROR */
{
  STATUS retStat = ERROR;
  char serialNum[SERIAL_NUM_LEN + 1];
  int num;

  if (clkVerStage == 0)
  {
    retStat = clkNumGet("$PTELQ,SERIAL", "$PTELR,SERIAL,", 5, &num); 
    if (retStat != ERROR) 
    { 
      sprintf(clkSerialNum, "%05d", num);
    }
  }
  else
  {
    retStat = clkNumGet("$PTELQ,SA", "$PTELR,SA,", 5, &num);
    if (retStat != ERROR)
    {
      sprintf(serialNum, "%05d", num);
      retStat = clkNumGet("$PTELQ,SB", "$PTELR,SB,", 5, &num);
      if (retStat != ERROR)
      {
        sprintf(serialNum, "%s%05d", serialNum, num);
        retStat = clkNumGet("$PTELQ,SC", "$PTELR,SC,", 5, &num);
        if (retStat != ERROR)
        {
          sprintf(serialNum, "%s%05d", serialNum, num);
          strcpy(clkSerialNum, serialNum);
        }
      }
    }
  }
  return retStat;
}

/**********************************************************************
clkSerialNumSet - Serial Number Set

Sends a command to clock module to write the offset. 
Returns the serial number or -1 for error.
*/

/*
for clkVerStage 0
The command sent to the clock module is $PTELQ,SERIAL
The expected response is $PTELR,SERIAL,xxx  

for other clkVerStage
The commands sent to the clock module are $PTELQ,SA $PTELQ,SB $PTELQ,SC
The expected responses are $PTELR,SA,xxx $PTELR,SB,xxx $PTELR,SC,xxx 
The most significant bits are SA and the least significant bits are SC
*/

STATUS clkSerialNumSet(unsigned short serialA, unsigned short serialB,
    unsigned short serialC)         /* RETURN: OK/ERROR */
{
    char cmd[MAX_CMD_LEN + 1];            /* + 1 for null */
    char rspExpected[MAX_RSP_LEN + 1];    /* + 1 for null */
    char rsp[MAX_RSP_LEN + 1];            /* + 1 for null */

    if (clkVerStage == 0)
    {
        /* select command and append the offset in ASCII */
        sprintf(cmd, "$PTELS,SERIAL,%d", serialA);
        sprintf(rspExpected, "$PTELA,SERIAL,%d", serialA);
    
        if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
        {                                       
            if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
                return OK;
        }
    }
    else
    {
        /* select command and append the offset in ASCII */
        sprintf(cmd, "$PTELS,SA,%d", serialA);
        sprintf(rspExpected, "$PTELA,SA,%d", serialA);
    
        if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
        {                                       
            if ((strcmp(rsp, rspExpected)) != 0)      /* Is the response expected response? */
                return ERROR;
        }

        sprintf(cmd, "$PTELS,SB,%d", serialB);
        sprintf(rspExpected, "$PTELA,SB,%d", serialB);
    
        if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
        {                                       
            if ((strcmp(rsp, rspExpected)) != 0)      /* Is the response expected response? */
                return ERROR;
        }

        sprintf(cmd, "$PTELS,SC,%d", serialC);
        sprintf(rspExpected, "$PTELA,SC,%d", serialC);
    
        if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
        {                                       
            if ((strcmp(rsp, rspExpected)) != 0)      /* Is the response expected response? */
                return ERROR;
        }
    }

    return OK;
}


/**********************************************************************
clkStatGet - Status Get

Sends a command to clock module to read the status. 
Returns the status or -1 for error. 

The returned status is:

     0 - No Alarm conditions
     1 - Burst Alarm Active
     2 - Frame Alarm Active
     3 - Borth Burst and Fram Alarms active
*/

/*
The command sent to the clock module is $PTELQ,INT
The expected response is $PTELR,INT,x
*/

int clkStatGet()         /* RETURN: status or -1 for error */
{
  char cmd[] = "$PTELQ,INT";    
  char rspExpected[] = "$PTELR,INT,";
  int num;

  if (clkNumGet(cmd, rspExpected, 2, &num) != OK)
  {
    num = -1;
  }
  return (num);  /* Max. 1 digit expected */
}


/**********************************************************************
clkNumDaysTuneReset  -  Number of Days Tune Reset

Sends a command to clock module to reset the 'Number of Days since Tuning' count. 
Returns the status or -1 for error. 
*/

/*
The command sent to the clock module is $PTELS,TUNE
The expected response is $PTELA,TUNE,0  
*/

STATUS clkNumDaysTuneReset()     /* RETURN: OK / ERROR */
{
  char cmd[]  = "$PTELS,TUNE";   
  char rspExpected[] = "$PTELA,TUNE,0";
  char rsp[MAX_RSP_LEN + 1];          /* + 1 for null */

  if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
  {                                       
    if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
      return OK;
  }

  return ERROR;
}


/**********************************************************************
clkNumDaysTuneGet  -  Number of Days Tune Get

Sends a command to clock module to read the number of days since last tuneup. 
Returns the no. of days or -1 for error.
*/

/*
The command sent to the clock module is $PTELQ,TUNE
The expected response is $PTELR,TUNE,xxxx  
*/

int clkNumDaysTuneGet()       /* RETURN: number of days or -1 for error */
{
  char cmd[] = "$PTELQ,TUNE";   
  char rspExpected[] = "$PTELR,TUNE,";
  int num;

  if (clkNumGet(cmd, rspExpected, 4, &num) != OK)
  {
    num = -1;
  }

  return (num);  /* Max. 4 digits expected */
}


/**********************************************************************
clkNumDaysRunReset - Number of Days Run Reset

Sends a command to clock module to reset the 'Number of Days since Running' count. 
Returns the status or -1 for error. 
*/

/*
The command sent to the clock module is $PTELS,RUN
The expected response is $PTELA,RUN,0  
*/

STATUS clkNumDaysRunReset()         /* RETURN: OK / ERROR */
                    
{
  char cmd[]  = "$PTELS,RUN";   
  char rspExpected[] = "$PTELA,RUN,0";
  char rsp[MAX_RSP_LEN + 1];          /* + 1 for null */

  if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
  {                                       
    if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
      return OK;
  }

  return ERROR;
}


/**********************************************************************
clkNumDaysRunGet  -  Number of Days Run Get

Sends a command to clock module to read the number of days since it is running. 
Returns the no. of days or -1 for error. 
*/

/*
The command sent to the clock module is $PTELQ,RUN
The expected response is $PTELR,RUN,xxxx  
*/

int clkNumDaysRunGet()          /* RETURN: number of days or -1 for error */
{
  char cmd[] = "$PTELQ,RUN";   
  char rspExpected[] = "$PTELR,RUN,";
  int num;

  if (clkNumGet(cmd, rspExpected, 4, &num) != OK)
  {
    num = -1;
  }

  return (num);  /* Max. 4 digits expected */
}


/**********************************************************************
clkSoftVersionGet  -  Software Version Get

Sends a command to clock module to get the software version. 
Saves the software version in the file local variable clkSoftVersion.
*/

/*
The command sent to the clock module is $PTELQ,SOFT / $PTELQ,SFT
The expected response is $PTELR,SOFT,x.x / $PTELR,SFT,x.x
*/


LOCAL STATUS clkSoftVersionGet()
{
  const char* cmdSet[] = {"$PTELQ,SOFT","$PTELQ,SFT"};
  const char* rspExpected[] = {"$PTELR,SOFT,","$PTELR,SFT,"};

  char rsp[MAX_RSP_LEN + 1];       /* + 1 for null */
  int verFound;         /* TRUE/ FALSE */
  int clkStat;          /* OK / ERROR */
  int i;
  int rspExpectedLen;

  for (verFound = FALSE, clkStat = OK, i = 0; 
       (verFound == FALSE) && (clkStat == OK) && (i < NUM_VERSION_STAGES); 
       i++)
  {
    clkStat = clkCmd(cmdSet[i], rsp);
    if (clkStat == OK) 
    {
      if (memcmp(rsp, rspExpected[i], strlen(rspExpected[i])) == 0)
      {
        verFound = TRUE; 
        rspExpectedLen = strlen(rspExpected[i]);
        clkSoftVersion[0] = rsp[0 + rspExpectedLen];
        clkSoftVersion[1] = rsp[1 + rspExpectedLen];
        clkSoftVersion[2] = rsp[2 + rspExpectedLen];
      }
    }
  }
  return verFound ? OK : ERROR;
}


/**********************************************************************
clkSetEepromValue - Set a location in EEPROM.
 
*/

STATUS clkSetEepromValue(int location, unsigned char val)
{
    const char* cmdSet[] = {"$PTELS,EEPROM,","$PTELS,EEPRM,"};
    const char* rspExpectedSet[] = {"$PTELA,EEPROM,","$PTELA,EEPRM,"};

    char cmd[MAX_CMD_LEN + 1];            /* + 1 for null */
    char rspExpected[MAX_RSP_LEN + 1];    /* + 1 for null */
    char rsp[MAX_RSP_LEN + 1];            /* + 1 for null */

    /* select command and append the offset in ASCII */
    sprintf(cmd, "%s%d,%d", cmdSet[clkVerStage], location, val);
    sprintf(rspExpected, "%s%d", rspExpectedSet[clkVerStage], val);

    if ((clkCmd(cmd, rsp)) == OK)               /* send the command */
    {                                       
      if ((strcmp(rsp, rspExpected)) == 0)      /* Is the response expected response? */
        return OK;
    }
    return ERROR;
}


/**********************************************************************
clkSetDateOfLastTuning - Set the last clock tuning date.
*/
STATUS clkSetDateOfLastTuning(unsigned char month, unsigned char day, unsigned short year)
{
    if (month > 12) return ERROR;
    if (day > 31) return ERROR;

    if (clkVerStage == 0)
    {
        if (clkSetEepromValue(11, month) != OK) return ERROR;
        if (clkSetEepromValue(12, day) != OK) return ERROR;
        if (clkSetEepromValue(13, (unsigned char)(year>>8)) != OK) return ERROR;
        if (clkSetEepromValue(14, (unsigned char) year) != OK) return ERROR;
    }
    else
    {
        if (clkSetEepromValue(16, month) != OK) return ERROR;
        if (clkSetEepromValue(17, day) != OK) return ERROR;
        if (clkSetEepromValue(18, (unsigned char)(year>>8)) != OK) return ERROR;
        if (clkSetEepromValue(19, (unsigned char) year) != OK) return ERROR;
    }

    return OK;
}


/**********************************************************************
clkGetDateOfLastTuning - Get the last clock tuning date.
*/
STATUS clkGetDateOfLastTuning(unsigned char *month, unsigned char *day, unsigned short *year)
{
    char cmd[MAX_CMD_LEN + 1];
    char rsp[MAX_CMD_LEN + 1];
    int num;
    const char* cmdSet[] = {"$PTELQ,EEPROM,","$PTELQ,EEPRM,"};
    const char* rspSet[] = {"$PTELR,EEPROM,","$PTELR,EEPRM,"};

    if (clkVerStage == 0)
        sprintf(cmd, "%s%d", cmdSet[clkVerStage], 11);
    else
        sprintf(cmd, "%s%d", cmdSet[clkVerStage], 16);
    sprintf(rsp, "%s", rspSet[clkVerStage]);
    if (clkNumGet(cmd, rsp, 3, &num) != OK) return ERROR;
    *month = (unsigned char)num;

    if (clkVerStage == 0)
        sprintf(cmd, "%s%d", cmdSet[clkVerStage], 12);
    else
        sprintf(cmd, "%s%d", cmdSet[clkVerStage], 17);
    sprintf(rsp, "%s", rspSet[clkVerStage]);
    if (clkNumGet(cmd, rsp, 3, &num) != OK) return ERROR;
    *day = (unsigned char)num;

    if (clkVerStage == 0)
        sprintf(cmd, "%s%d", cmdSet[clkVerStage], 13);
    else
        sprintf(cmd, "%s%d", cmdSet[clkVerStage], 18);
    sprintf(rsp, "%s", rspSet[clkVerStage]);
    if (clkNumGet(cmd, rsp, 3, &num) != OK) return ERROR;
    *year = (unsigned short)(num<<8);
    
    if (clkVerStage == 0)
        sprintf(cmd, "%s%d", cmdSet[clkVerStage], 14);
    else
        sprintf(cmd, "%s%d", cmdSet[clkVerStage], 19);
    sprintf(rsp, "%s", rspSet[clkVerStage]);
    if (clkNumGet(cmd, rsp, 3, &num) != OK) return ERROR;
    *year |= (unsigned short)(num);


    if (*month > 12) return ERROR;
    if (*day > 31) return ERROR;

    return OK;
}


/**********************************************************************
clkShowDateOfLastTuning - Print the last clock tuning date.
*/
STATUS clkShowDateOfLastTuning()
{
    unsigned char month;
    unsigned char day;
    unsigned short year;

    if (clkGetDateOfLastTuning(&month, &day, &year) == OK)
    {
        printf("Clock Last Tuned on %d/%d/%d.\n", month, day, year);
    }
    else
    {
        printf("Unable to retrieve a valid clock tuning data!\n");
    }
}


/**********************************************************************
clkReadAllEeprom - Read in all the clocks EEPROM values.
 
*/

STATUS clkReadAllEeprom(unsigned char *buf)
{
    STATUS status;
    char cmd[MAX_CMD_LEN + 1];
    char rsp[MAX_CMD_LEN + 1];
    int numBytes, i, num;
    const char* cmdSet[] = {"$PTELQ,EEPROM,","$PTELQ,EEPRM,"};
    const char* rspSet[] = {"$PTELR,EEPROM,","$PTELR,EEPRM,"};
    
    status = OK;

    if (clkVerStage == 0)
        numBytes = EEPROM_SIZE_REV1_8;
    else
        numBytes = EEPROM_SIZE_REVC;

    for (i = 0; i < numBytes; i++)
    {
        sprintf(cmd, "%s%d", cmdSet[clkVerStage], i);
        sprintf(rsp, "%s", rspSet[clkVerStage]);
        if ((status = clkNumGet(cmd, rsp, 3, &num)) != OK)
        {
            break;
        }
        else
        {
            buf[i] = (unsigned char)num;
        }
    }

    return (status);
}

/**********************************************************************
clkSaveEepromToNvRam - Copy Clock EEPROM to NV Ram

Reads the clock cards EEPROM values and stores them in the NV Ram on CDC. 
*/


STATUS clkSaveEepromToNvRam()         /* RETURN: OK / ERROR */
                    
{
    STATUS status;
    unsigned char buf[EEPROM_SIZE_REVC];  

    /* Read EEPROM from clock card */
    status = clkReadAllEeprom(buf);

    /* Write EEPROM data to NV Ram */
    if (status == OK)
        status = sysClockBoardEepromSet((char *)buf);

    return status;
}


/**********************************************************************
clkRestoreEepromFromNvRam - Copy Clock EEPROM saved in NV Ram to clock card

Compares the NV Ram saved values against the clock card's EEPROM values.
If there is a mismatch then the values are copied from NV Ram to EEPROM. 
*/


STATUS clkRestoreEepromFromNvRam()         /* RETURN: OK / ERROR */
                    
{
    unsigned char nvRamEeprom[EEPROM_SIZE_REVC];  
    unsigned char clockBoardEeprom[EEPROM_SIZE_REVC]; 
    STATUS status;
    int i, j, numBytes;
    unsigned short serialA, serialB, serialC;

    /* Read NV Ram copy of EEPROM */
    status = sysClockBoardEepromGet((char *)nvRamEeprom);

    /* Read EEPROM from clock card */
    if (status ==  OK)
    {
        status = clkReadAllEeprom(clockBoardEeprom);

        if (status == OK)
        {
            if (clkVerStage == 0)
                numBytes = EEPROM_SIZE_REV1_8;
            else
                numBytes = EEPROM_SIZE_REVC;

            for (i=0; i < numBytes; i++)
            {
                /* skip all the Num Days values.... */
                if (clkVerStage == 0)
                    if (i == 6 || i == 7 || i == 8 || i == 9) continue;
                else
                    if (i == 4 || i == 5 || i == 6 || i == 7) continue;

                if (nvRamEeprom[i] != clockBoardEeprom[i])
                {
                    printf("Clock Board EEPROM does not match NV Ram!\n"
                        "Clock Board EEPROM will be updated to match NV Ram!\n");
                    /* Write EEPROM from NV Ram */

                    /* Set Digital Pot value */
                    if (clkOffsetSet(nvRamEeprom[0]) != OK)
                        printf("Clock Board Update: Unable to restore Digital Pot value!\n");

                    /* Set Oscillator transfer function */
                    if (clkOscillatorTransferFuncSet(nvRamEeprom[1]) != OK)
                        printf("Clock Board Update: Unable to restore Oscillator Transfer function value!\n");
                      
                    /* Set Clock Select value */
                    if (clkClockSelectSet(nvRamEeprom[2]) != OK)
                        printf("Clock Board Update: Unable to restore Clock Select value!\n");
                      
                    /* Set Board Revision */
                    if (clkBoardRevSet(nvRamEeprom[3]) != OK)
                        printf("Clock Board Update: Unable to restore Board Revision!\n");
                      
                    if (clkVerStage == 0)
                    {
                        /* Set Serial Number */
                        serialA = nvRamEeprom[4] | (nvRamEeprom[5] << 8);
                        if (clkSerialNumSet(serialA,0,0) != OK)
                            printf("Clock Board Update: Unable to restore Serial Number!\n");

                        /* Restore any remaining bytes in EEPROM */
                        for (j=11; j < EEPROM_SIZE_REV1_8; j++)
                        {
                            if (clkSetEepromValue(j, nvRamEeprom[j]) != OK)
                                printf("Clock Board Update: Unable to restore EEPROM loc %d to %d!\n", 
                                    j, nvRamEeprom[j]);
                        }

                        /* Set Checksum */
                        if (clkChecksumSet(nvRamEeprom[10]) != OK)
                            printf("Clock Board Update: Checksum does not match stored value!\n");
                    }
                    else
                    {
                        /* Set Oscillator Type */
                        if (clkOscillatorSet(nvRamEeprom[9]) != OK)
                            printf("Clock Board Update: Unable to restore Oscillator Type!\n");

                        /* Set Serial Number */
                        serialA = nvRamEeprom[10] | (nvRamEeprom[11] << 8);
                        serialB = nvRamEeprom[12] | (nvRamEeprom[13] << 8);
                        serialC = nvRamEeprom[14] | (nvRamEeprom[15] << 8);
                        if (clkSerialNumSet(serialA,serialB,serialC) != OK)
                            printf("Clock Board Update: Unable to restore Serial Number!\n");

                        /* Restore any remaining bytes in EEPROM */
                        for (j=16; j < EEPROM_SIZE_REVC; j++)
                        {
                            if (clkSetEepromValue(j, nvRamEeprom[j]) != OK)
                                printf("Clock Board Update: Unable to restore EEPROM loc %d to %d!\n", 
                                    j, nvRamEeprom[j]);
                        }

                        /* Set Checksum */
                        if (clkChecksumSet(nvRamEeprom[8]) != OK)
                            printf("Clock Board Update: Checksum does not match stored value!\n");
                    }
                    
                    break;
                }
            }
        }
    }
    return status;
}


/**********************************************************************
clkValidateEeprom - Make sure the EEPROM is correct

 
*/


STATUS clkValidateEeprom()         /* RETURN: OK / ERROR */
{
    STATUS status = OK;
    /* If NV Ram has a saved EEPROM copy then use it to verify the EEPROM */
    printf("Validating Clock Board EEPROM\n");
    if (sysIsClockBoardEepromSet())
    {
        if ((status = clkRestoreEepromFromNvRam()) != OK)
        {
            printf("WARNING - Unable to restore Clock Board EEPROM!\n");
        }
    }
    /* Otherwise assume the clock board EEPROM is good and save it to Nv Ram */
    else
    {
        printf("Saving Clock Board EEPROM into NV Ram!\n");
        if ((status = clkSaveEepromToNvRam()) != OK)
        {
            printf("WARNING - Unable to save Clock Board EEPROM!\n");
        }
    }

    return (status);
}
