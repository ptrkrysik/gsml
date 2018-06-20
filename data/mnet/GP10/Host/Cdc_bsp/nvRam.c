/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*
JetCell modification history
----------------------------
99/06/01  ck   Initial Version
*/

/* nvRam.c - non-volatile RAM library */

/*
DESCRIPTION
This library contains non-volatile RAM manipulation routines for targets
with non-volatile RAM.  Read and write routines that return ERROR
are included.
*/

#include <vxWorks.h> 
#include <logLib.h> 
#include <taskLib.h> 
#include <tickLib.h>
#include "nvRam.h"
#include "bootFlash.h"

/******************************************************************************
*
* The boot flash is broken down into the following sections:
*
*
*               +----------------------+  flash start - 0x20000000
*               |                      |
*               |                      |
*               |   vxWorks Boot Code  |
*               |                      |
*               |                      |
*               |                      |
*               |                      |
*               |                      |
*               |                      |
*               |                      |
*               |                      |
*               |                      |
*             / +----------------------+  BOOT_PARAM_SECT_OFFSET 
*            /  |  Boot Parameters     |
*           |   |                      |
*           |   +----------------------+  ETHERNET_SECT_OFFSET
*           |   |  Ethernet address    |     
*           |   |                      |
*           |   +----------------------+  VC_SN_SECT_OFFSET
*            \  |  VC / CDC            |  
*            /  |  serial/ Part number |
*   NV_RAM   \  +----------------------+  PON_TEST_CONFIG_OFFSET
*            /  | Power on test config |
*           |   |                      |
*           |   +----------------------+  RADIO_BOARD_EEPROM
*           |   | Radio Board Eeprom   |
*           |   |                      |
*           |   +----------------------+  CLOCK_BOARD_EEPROM
*           |   | Clock Board Eeprom   |
*           |   |                      |
*           |   +----------------------+ 
*           |   |                      |
*           |   |       Unused         |
*           |   |       Sectors        |
*           \   |                      |
*             \ +----------------------+  flash end - 0x203fffff
*
*
*  The section of flash is dedicated to be used for the vxWorks boot code.  The
*  next section is non-volatile RAM.  NVRam is used to store various pieces
*  of information unique to a ViperCell.
*/

/* Sector offsets in nvRam */
#define BOOT_PARAM_SECT_OFFSET      0
#define ETHERNET_SECT_OFFSET        1
#define VC_SN_SECT_OFFSET           2
#define PON_TEST_CONFIG_OFFSET      3
#define RADIO_BOARD_EEPROM          4
#define CLOCK_BOARD_EEPROM          5

/* Power on test offsets withing the power on test configuration sector */
#define POST_RESULT_OFFSET          0
#define POST_MASK_OFFSET            4

/* Total size in bytes used in the power on test configuration secotr */
#define POST_SIZE                   8

/* Definitions for serial / Part number sector */
#define SER_PART_DATA_TYPES         4    /* 4: VC serial num, VC part num, CDC serial num, CDC part num */
#define SER_PART_DATA_SIZE        100
#define SER_PART_TOTAL_SIZE       (SER_PART_DATA_TYPES * SER_PART_DATA_SIZE)

enum SerialPartDataType
{
  ViperCellSerialNum = 0,
  ViperCellPartNum = 1,
  CdcSerialNum = 2,
  CdcPartNum = 3
};


/******************************************************************************
*
* sysNvRamGet - get the contents of non-volatile RAM
*
* This routine copies the contents of non-volatile memory into a specified
* string.  The string is terminated with an EOS.
*
* RETURNS: OK, ERROR.
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysNvRamGet
(
   char *string,    /* where to copy non-volatile RAM    */
   int strLen,      /* maximum number of bytes to copy   */
   int offset       /* byte offset into non-volatile RAM */
)
{
	int n;
	for (n = 0; n < strLen; n++)
	{
      string[n] = *((unsigned char *)ROM_BASE_ADRS + 
         (flashGetNumSect() - NV_RAM_SECT_FROM_END) * flashGetSectSize() + 
         offset + n);
   }
   return (OK);
}

/*******************************************************************************
*
* sysNvRamSet - write to non-volatile RAM
*
* This routine copies a specified string into non-volatile RAM.
*
*  NOTE: Entire sectors will be erased as needed to write the data.
*
* RETURNS: OK, ERROR.
*
* SEE ALSO: sysNvRamGet()
*/

STATUS sysNvRamSet
(
   char *string,     /* string to be copied into non-volatile RAM */
   int strLen,       /* maximum number of bytes to copy           */
   int offset        /* byte offset into non-volatile RAM         */
)
{
   return (flashProgram(ROM_BASE_ADRS + (flashGetNumSect() - NV_RAM_SECT_FROM_END) * 
      flashGetSectSize() + offset, string, strLen));
}

/*******************************************************************************
*
* sysEnetAddrGet
*
* This routine gets the Ethernet address from  non-volatile RAM.
*
* RETURNS: OK, ERROR.
*
* SEE ALSO: sysNvRamGet()
*/

STATUS sysEnetAddrGet
(
   int unitNum,
   char *p
)
{
	return sysNvRamGet(p, 6, flashGetSectSize() * ETHERNET_SECT_OFFSET);
}

/*******************************************************************************
*
* sysEnetAddrSet
*
* This routine sets the Ethernet address into  non-volatile RAM.
*
* RETURNS: OK, ERROR, always.
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysEnetAddrSet
(
   char a0,
   char a1,
   char a2,
   char a3,
   char a4,
   char a5
)
{
	char p[] = {a5, a4, a3, a2, a1, a0};
	return sysNvRamSet(p, 6,  flashGetSectSize() * ETHERNET_SECT_OFFSET);
}



/*******************************************************************************
*
* sysIpAddrSet
*
* This routine sets the ip address in the boot string stored in non-volatile RAM.
* It also sets the ip address in the vipercell.cfg file if one is found.
*
* RETURNS: OK, ERROR, always.
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysIpAddrSet
(
   char *ipAddr
)
{
   BOOT_PARAMS bootParams;
   int fd;
   char fileName[256];
   STATUS stat;
   
   stat = OK;
   
   /* Get the current boot parameters */
   bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
   
   /* Save ip address in non-volitile RAM. */
   if (ipAddr)
   {
      strcpy (bootParams.ead, ipAddr);
      bootStructToString (BOOT_LINE_ADRS, &bootParams);
      if ((stat = sysNvRamSet (BOOT_LINE_ADRS, strlen (BOOT_LINE_ADRS) + 1, 0) == OK))
      {
         /* If we booted from flash and vipercell.cfg exists then change the ip */
         /* address there as well.                                              */
         if (strncmp (bootParams.bootDev, "ata", 3) == 0)
         {
            strcpy(fileName, getenv("VIPERCALL_BASE"));
            strcat(fileName, "/vipercell.cfg");
      
            if ((fd = open (fileName, O_RDWR, 0)) != ERROR)
            {
               close(fd);
               unlink(fileName);
               if ((fd = creat(fileName, O_WRONLY)) != ERROR)
               {
                  fdprintf(fd, "[inet_on_ethernet]\n");
                  fdprintf(fd, "%s\n", ipAddr);
                  close(fd);
               }
            }
         }
      }
   }
   
   
	return (stat);
}


/*******************************************************************************
*
* sysIpAddrGet
*
* This routine gets the ip address from the boot string stored in non-volatile RAM.
*
* RETURNS: OK, ERROR, always.
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysIpAddrGet
(
   char *ipAddr
)
{
   BOOT_PARAMS bootParams;
   STATUS stat;
   
   stat = OK;
   
   /* Get the current boot parameters */
   bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
   
   /* Save ip address in non-volitile RAM. */
   strncpy (ipAddr, bootParams.ead, 32);
   
   return (stat);
}


/*******************************************************************************
sysSerialPartDataSet

Common function to set ViperCell/CDC serial numbers and part numbers
*/

static STATUS sysSerialPartDataSet
  (
    enum SerialPartDataType dataType,
    char* data
  )
{
  STATUS retStat = ERROR;

  char buff[SER_PART_TOTAL_SIZE]; 

  if (sysNvRamGet(buff, SER_PART_TOTAL_SIZE, flashGetSectSize() * VC_SN_SECT_OFFSET) == OK)
  {
    memcpy(&buff[SER_PART_DATA_SIZE * dataType], data, min(strlen(data) + 1, SER_PART_DATA_SIZE));      
    retStat = sysNvRamSet(buff, SER_PART_TOTAL_SIZE, flashGetSectSize() * VC_SN_SECT_OFFSET);
  }
  return retStat;
}

/*******************************************************************************
sysSerialPartDataGet

Common function to get ViperCell/CDC serial numbers and part numbers
*/
static STATUS sysSerialPartDataGet
  (
    enum SerialPartDataType dataType,  /* type of data to get*/
    char* data,                        /* where to put the data */
    int len                            /* no. of bytes to read */
  )
{
  STATUS retStat;

  int offset = SER_PART_DATA_SIZE * dataType;

  retStat = sysNvRamGet(data, len, 
                        (flashGetSectSize() * VC_SN_SECT_OFFSET) + offset);
  return retStat;
}

/*******************************************************************************
*
* sysSerialNumGet
*
* This routine gets the ViperCell serial number from  non-volatile RAM.  The pointer 'p'
* should be of atleast size MAX_VC_SN.
*
* RETURNS: OK, ERROR.
*
* SEE ALSO: sysNvRamGet()
*/

STATUS sysSerialNumGet
(
   char *p
)
{
  return sysSerialPartDataGet(ViperCellSerialNum, p, MAX_VC_SN);
}


/*******************************************************************************
*
* sysSerialNumShow
*
* This routine gets ViperCell the serial number from  non-volatile RAM and displays it.
*
* RETURNS: OK, ERROR.
*
* SEE ALSO: sysNvRamGet()
*/

void sysSerialNumShow()
{
   char p[MAX_VC_SN];

   sysSerialNumGet(p);
   p[MAX_VC_SN-1] = 0;
   printf ("System Serial Number: %s\n", p);
}


/*******************************************************************************
*
* sysSerialNumSet
*
* This routine sets the ViperCell serial number into  non-volatile RAM.
*
* RETURNS: OK, ERROR, always.
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysSerialNumSet
(
   char *sn
)
{
  return sysSerialPartDataSet(ViperCellSerialNum, sn);
}


/*******************************************************************************
sysVCPartNumSet

Sets the ViperCell part number into the non-volatile memory
*/

STATUS sysVCPartNumSet(char* partNum)
{
  return sysSerialPartDataSet(ViperCellPartNum, partNum);
}

/*******************************************************************************
sysVCPartNumGet

Gets the ViperCell part number from the non-volatile memory
*/
STATUS sysVCPartNumGet(char* partNum)
{
  return sysSerialPartDataGet(ViperCellPartNum, partNum, MAX_VC_PN);
}


/*******************************************************************************
sysVCPartNumShow

Prints the ViperCell part number
*/
void sysVCPartNumShow()
{
  char data[SER_PART_DATA_SIZE];
  
  sysVCPartNumGet(data);
  data[SER_PART_DATA_SIZE -1] = 0;
  printf("System Part Number: %s\n", data);
}

/*******************************************************************************
sysCDCSerialNumSet

Sets the CDC serial number into the non-volatile memory
*/

STATUS sysCDCSerialNumSet(char* serNum)
{
  return sysSerialPartDataSet(CdcSerialNum, serNum);
}

/*******************************************************************************
sysCDCSerialNumGet

Gets the CDC serial number from the non-volatile memory
*/
STATUS sysCDCSerialNumGet(char* serNum)
{
  return sysSerialPartDataGet(CdcSerialNum, serNum, MAX_CDC_SN);
}


/*******************************************************************************
sysCDCSerialNumShow

Prints the CDC serial number
*/
void sysCDCSerialNumShow()
{
  char data[SER_PART_DATA_SIZE];
  
  sysCDCSerialNumGet(data);
  data[SER_PART_DATA_SIZE -1] = 0;
  printf("CDC Serial Number: %s\n", data);
}


/*******************************************************************************
sysCDCPartNumSet

Sets the CDC part number into the non-volatile memory
*/

STATUS sysCDCPartNumSet(char* partNum)
{
  return sysSerialPartDataSet(CdcPartNum, partNum);
}

/*******************************************************************************
sysCDCPartNumGet

Gets the CDC part number from the non-volatile memory
*/
STATUS sysCDCPartNumGet(char* partNum)
{
  return sysSerialPartDataGet(CdcPartNum, partNum, MAX_CDC_PN);
}


/*******************************************************************************
sysCDCPartNumShow

Prints the CDC part number
*/
void sysCDCPartNumShow()
{
  char data[SER_PART_DATA_SIZE];
  
  sysCDCPartNumGet(data);
  data[SER_PART_DATA_SIZE -1] = 0;
  printf("CDC Part Number: %s\n", data);
}


/*******************************************************************************
*
* postSetTestResult
*
* This routine sets a power on test result into non-volatile RAM.
*
* RETURNS: OK, ERROR, always.
*/

STATUS postSetTestResult (unsigned int testBit, unsigned int result)
{
   char saveBuf[POST_SIZE];
   
   sysNvRamGet (saveBuf, POST_SIZE, flashGetSectSize() * PON_TEST_CONFIG_OFFSET);
   
   if (result == POST_PASS)
   {
      (*(unsigned int *)(saveBuf + POST_RESULT_OFFSET)) |= (testBit);
   }
   else
   {
      (*(unsigned int *)(saveBuf + POST_RESULT_OFFSET)) &= (~testBit);
   }
   
   return sysNvRamSet (saveBuf, POST_SIZE, flashGetSectSize() * PON_TEST_CONFIG_OFFSET);
}


/*******************************************************************************
*
* postGetTestResult
*
* This routine gets a power on test result from non-volatile RAM.
*
* RETURNS: POST_PASS or POST_FAIL.
*/

unsigned int postGetTestResult (unsigned int testBit)
{
   char saveBuf[POST_SIZE];
   
   sysNvRamGet (saveBuf, POST_SIZE, flashGetSectSize() * PON_TEST_CONFIG_OFFSET);
   
   return ((*(unsigned int *)(saveBuf + POST_RESULT_OFFSET)) & testBit);
}



/*******************************************************************************
*
* postSetTestMask
*
* This routine sets a power on test mask into non-volatile RAM.
*
* RETURNS: OK, ERROR, always.
*/

STATUS postSetTestMask (unsigned int testBit, unsigned int mask)
{
   char saveBuf[POST_SIZE];
   
   sysNvRamGet (saveBuf, POST_SIZE, flashGetSectSize() * PON_TEST_CONFIG_OFFSET);
   
   if (mask == POST_ENABLE_TEST)
   {
      *(unsigned int *)(saveBuf + POST_MASK_OFFSET) |= (testBit);
   }
   else
   {
      *(unsigned int *)(saveBuf + POST_MASK_OFFSET) &= (~testBit);
   }
   
   return sysNvRamSet (saveBuf, POST_SIZE, flashGetSectSize() * PON_TEST_CONFIG_OFFSET);
}


/*******************************************************************************
*
* postGetTestMask
*
* This routine gets a power on test mask from non-volatile RAM.
*
* RETURNS: POST_PASS or POST_FAIL.
*/

unsigned int postGetTestMask (unsigned int testBit)
{
   char saveBuf[POST_SIZE];
   
   sysNvRamGet (saveBuf, POST_SIZE, flashGetSectSize() * PON_TEST_CONFIG_OFFSET);
   
   return ((*(unsigned int *)(saveBuf + POST_MASK_OFFSET) & testBit));
}



/*******************************************************************************
*
* sysRadioBoardEepromGet
*
* This routine gets the saved EEPROM data for the radio board.
*
* RETURNS: OK, ERROR.
*
* SEE ALSO: sysNvRamGet()
*/

STATUS sysRadioBoardEepromGet
(
   char *p
)
{
	return sysNvRamGet(p, RADIO_BOARD_EEPROM_SIZE, flashGetSectSize() * RADIO_BOARD_EEPROM + 4);
}

/*******************************************************************************
*
* sysRadioBoardEepromSet
*
* This routine sets the saved EEPROM data for the radio board.
*
* RETURNS: OK, ERROR, always.
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysRadioBoardEepromSet
(
   char *p
)
{
	int i;
	char buf[RADIO_BOARD_EEPROM_SIZE + 4];

	buf[0] = 0x12;
	buf[1] = 0x34;
	buf[2] = 0x45;
	buf[3] = 0x67;

	for (i = 0; i < RADIO_BOARD_EEPROM_SIZE; i++)
		buf[i+4] = p[i];

	return sysNvRamSet(buf, RADIO_BOARD_EEPROM_SIZE+4,  flashGetSectSize() * RADIO_BOARD_EEPROM);
}


/*******************************************************************************
*
* sysRadioBoardEepromSet
*
* This routine sets the saved EEPROM data for the radio board.
*
* RETURNS: OK, ERROR, always.
*
* SEE ALSO: sysNvRamSet()
*/

int sysIsRadioBoardEepromSet()
{
	char tag[4];

	sysNvRamGet(tag, 4, flashGetSectSize() * RADIO_BOARD_EEPROM);
	return (tag[0] == 0x12 &&  tag[1] == 0x34 && tag[2] == 0x45 && tag[3] == 0x67);
}


/*******************************************************************************
*
* sysClockBoardEepromGet
*
* This routine gets the saved EEPROM data for the clock board.
*
* RETURNS: OK, ERROR.
*
* SEE ALSO: sysNvRamGet()
*/

STATUS sysClockBoardEepromGet
(
   char *p
)
{
	return sysNvRamGet(p, CLOCK_BOARD_EEPROM_SIZE, flashGetSectSize() * CLOCK_BOARD_EEPROM + 4);
}

/*******************************************************************************
*
* sysClockBoardEepromSet
*
* This routine sets the saved EEPROM data for the clock board.
*
* RETURNS: OK, ERROR, always.
*
* SEE ALSO: sysNvRamSet()
*/

STATUS sysClockBoardEepromSet
(
   char *p
)
{
	int i;
	char buf[CLOCK_BOARD_EEPROM_SIZE + 4];

	buf[0] = 0x89;
	buf[1] = 0xab;
	buf[2] = 0xcd;
	buf[3] = 0xef;

	for (i=0; i < CLOCK_BOARD_EEPROM_SIZE; i++)
		buf[i+4] = p[i];

	return sysNvRamSet(buf, CLOCK_BOARD_EEPROM_SIZE+4,  flashGetSectSize() * CLOCK_BOARD_EEPROM);
}


/*******************************************************************************
*
* sysIsClockBoardEepromSet
*
* This routine sets the saved EEPROM data for the clock board.
*
* RETURNS: OK, ERROR, always.
*
* SEE ALSO: sysNvRamSet()
*/

int sysIsClockBoardEepromSet()
{
	char tag[4];

	sysNvRamGet(tag, 4, flashGetSectSize() * CLOCK_BOARD_EEPROM);
	return (tag[0] == 0x89 &&  tag[1] == 0xab && tag[2] == 0xcd && tag[3] == 0xef);
}
