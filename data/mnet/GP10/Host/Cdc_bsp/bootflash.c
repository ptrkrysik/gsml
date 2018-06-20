/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*
JetCell modification history
----------------------------
99/07/28  to   Initial Version
*/

/* bootFlash.c - Boot Flash Library */

/*
DESCRIPTION
This library contains routines to access the flash memory used for target
bootup.
*/
#include <stdio.h>
#include <fioLib.h>
#include "bootFlash.h"
#include "config.h"
#include "taskLib.h"


/******************************************************************************
*
*  Manufaturers and known parts.
*
*****************************************************************************/

#define FLASH_MFG_AMD                              1
#define AMD_29F016B                                0xad
#define AMD_29F032B                                0x41

/*****************************************************************************
* AMD flash device functions
*****************************************************************************/
int amdFlashGetDevId ( void );
int amdFlashGetMfg ( void );
int amd29F016BGetNumSect ( void );
int amdGetSectSize ( void );
int amd29F032BGetNumSect ( void );
STATUS amdFlashReset ( void );
STATUS amdFlashSectorErase ( unsigned int sector );
STATUS amdFlashChipErase ( void );
STATUS amdFlashProgram ( unsigned int flashAddr, unsigned char *data, int length ); 
STATUS amdFlashProgramNoErase ( unsigned int flashAddr, unsigned char *data, int length ); 

/*****************************************************************************
* Table of known devices
*
*  NOTE:  Make sure FLASH_NUM_DEVICES is the correct count for the number of
*     entries
*****************************************************************************/
#define FLASH_DEFAULT_FLASH_DEV     0
#define FLASH_NUM_DEVICES           2
#define FLASH_IDS_MFG               0
#define FLASH_IDS_DEV               1

/* Index of the currently installed flash device.  This value is setup when */
/* calling the flashInit functions     
                                     */
/* NOTE:  THIS CODE WAS ORIGINALLY WRITTEN TO HANDLE DIFFERENT FLASH */
/*        TYPES. FOR NOW THIS FUNCTION IS DISABLED                   */ 
/*int currFlashDevice;*/
#define currFlashDevice 1

int  FlashIds[FLASH_NUM_DEVICES][2] = 
{
   {FLASH_MFG_AMD, AMD_29F016B},
   {FLASH_MFG_AMD, AMD_29F032B}
};

flashDevice_t FlashDevices[FLASH_NUM_DEVICES] = 
{
   /* AMD 29F016B */
   {amdFlashGetMfg, amdFlashGetDevId, amd29F016BGetNumSect, amdGetSectSize, 
   amdFlashChipErase, amdFlashSectorErase, amdFlashReset, amdFlashProgram,
   amdFlashProgramNoErase},
   /* AMD 29F032B */
   {amdFlashGetMfg, amdFlashGetDevId, amd29F032BGetNumSect, amdGetSectSize, 
   amdFlashChipErase, amdFlashSectorErase, amdFlashReset, amdFlashProgram, 
   amdFlashProgramNoErase}
};



/******************************************************************************
*
*  Routines to access AMD type flash parts.
*
*****************************************************************************/

#define AMD_FLASH_RESET_DATA                       0xf0
#define AMD_FLASH_FIRST_ADDR                       0x555
#define AMD_FLASH_FIRST_DATA                       0xaa
#define AMD_FLASH_SECOND_ADDR                      0x2aa
#define AMD_FLASH_SECOND_DATA                      0x55
#define AMD_FLASH_THIRD_ADDR                       0x555
#define AMD_FLASH_THIRD_AUTOSEL_DATA               0x90
#define AMD_FLASH_THIRD_PROG_DATA                  0xa0
#define AMD_FLASH_THIRD_ERASE_DATA                 0x80
#define AMD_FLASH_FOURTH_MANU_ID_ADDR              0x00
#define AMD_FLASH_FOURTH_DEV_ID_ADDR               0x01
#define AMD_FLASH_FOURTH_ERASE_ADDR                0x555
#define AMD_FLASH_FOURTH_ERASE_DATA                0xaa
#define AMD_FLASH_FIFTH_ERASE_ADDR                 0x2aa
#define AMD_FLASH_FIFTH_ERASE_DATA                 0x55
#define AMD_FLASH_SIXTH_CHIP_ERASE_ADDR            0x555
#define AMD_FLASH_SIXTH_CHIP_ERASE_DATA            0x10
#define AMD_FLASH_SIXTH_SECT_ERASE_DATA            0x30

#define AMD_FLASH_DQ7_MASK                         0x80
#define AMD_29F016B_NUM_SECT                       32
#define AMD_29F032B_NUM_SECT                       64
#define AMD_SECT_SIZE                              65536



/******************************************************************************
*
* amdGetSectSize - get the number of bytes in a sector.
*
* This routine returns the number of bytes in a sector for an AMD flash device.
*
* RETURNS: number of sectors.
*/

int amdGetSectSize ( void )
{
   return (AMD_SECT_SIZE);
}


/******************************************************************************
*
* amd29F016BGetNumSect - get the number of sectors.
*
* This routine returns the number of sectors for a 29F032B AMD flash device.
*
* RETURNS: number of sectors.
*/

int amd29F032BGetNumSect ( void )
{
   return (AMD_29F032B_NUM_SECT);
}


/******************************************************************************
*
* amd29F016BGetNumSect - get the number of sectors.
*
* This routine returns the number of sectors for a 29F016B AMD flash device.
*
* RETURNS: number of sectors.
*/

int amd29F016BGetNumSect ( void )
{
   return (AMD_29F016B_NUM_SECT);
}



/******************************************************************************
*
* amdFlashGetMfg - get the manufacturer id.
*
* This routine returns the manufacturer id for an AMD flash device.
*
* RETURNS: valid id or -1.
*/

int amdFlashGetMfg ( void )
{
   unsigned char id;
   
   id = 0xff;
   
   amdFlashReset();
   
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FIRST_ADDR) = AMD_FLASH_FIRST_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_SECOND_ADDR) = AMD_FLASH_SECOND_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_THIRD_ADDR) = AMD_FLASH_THIRD_AUTOSEL_DATA;
   id = *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FOURTH_MANU_ID_ADDR);
   
   amdFlashReset();
   
   if (id == 0xff)
   {
      return -1;
   }
   else
   {
      return id;
   }
}



/******************************************************************************
*
* amdFlashGetDevId - get the device id.
*
* This routine returns the device id for an AMD flash device.
*
* RETURNS: valid id or -1.
*/

int amdFlashGetDevId ( void )
{
   unsigned char id;
   
   id = 0xff;
   
   amdFlashReset();
   
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FIRST_ADDR) = AMD_FLASH_FIRST_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_SECOND_ADDR) = AMD_FLASH_SECOND_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_THIRD_ADDR) = AMD_FLASH_THIRD_AUTOSEL_DATA;
   id = *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FOURTH_DEV_ID_ADDR);
   
   amdFlashReset();
   
   if (id == 0xff)
   {
      return -1;
   }
   else
   {
      return id;
   }
}


/******************************************************************************
*
* amdFlashReset - reset an AMD flash device.
*
* This routine sends a flash reset command to the AMD flash device.
*
* RETURNS: OK, ERROR.
*/

STATUS amdFlashReset ( void )
{
   *((unsigned char *)ROM_BASE_ADRS) = AMD_FLASH_RESET_DATA;
   return OK;
}



/******************************************************************************
*
* amdFlashChipErase - erase an AMD flash device.
*
* This routine sends a command to erase the entire AMD flash device.
*
* RETURNS: OK, ERROR.
*/

STATUS amdFlashChipErase ( void )
{
   /* Send a chip erase command */
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FIRST_ADDR) = AMD_FLASH_FIRST_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_SECOND_ADDR) = AMD_FLASH_SECOND_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_THIRD_ADDR) = AMD_FLASH_THIRD_ERASE_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FOURTH_ERASE_ADDR) = AMD_FLASH_FOURTH_ERASE_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FIFTH_ERASE_ADDR) = AMD_FLASH_FIFTH_ERASE_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_SIXTH_CHIP_ERASE_ADDR) = AMD_FLASH_SIXTH_CHIP_ERASE_DATA;
   
   /* Spin here until the flash is erased                                  */
   /* DQ7 outputs a zero until the flash is erased.                        */
   printf("Erasing boot flash");
   while (!(*((unsigned char *)ROM_BASE_ADRS) & AMD_FLASH_DQ7_MASK))
   {
      taskDelay(10);
      printf(".");
   }
   printf("Erase Complete!\n");
   return OK;
}


/******************************************************************************
*
* amdFlashSectorErase - erase one sector of an AMD flash device.
*
* This routine sends a command to erase one sector of an AMD flash device.
*
* RETURNS: OK, ERROR.
*/

STATUS amdFlashSectorErase ( unsigned int sector )
{
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FIRST_ADDR) = AMD_FLASH_FIRST_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_SECOND_ADDR) = AMD_FLASH_SECOND_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_THIRD_ADDR) = AMD_FLASH_THIRD_ERASE_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FOURTH_ERASE_ADDR) = AMD_FLASH_FOURTH_ERASE_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FIFTH_ERASE_ADDR) = AMD_FLASH_FIFTH_ERASE_DATA;
   *((unsigned char *)ROM_BASE_ADRS + (sector * flashGetSectSize())) = AMD_FLASH_SIXTH_SECT_ERASE_DATA;
   
   /* Spin here until the sector is erased                                    */
   /* DQ7 outputs a zero until the sector is erased.                          */
   while (!(*((unsigned char *)ROM_BASE_ADRS + (sector * flashGetSectSize())) & 
      AMD_FLASH_DQ7_MASK))
   {
    /*  taskDelay(1);*/
   }
   return OK;
}


/******************************************************************************
*
* amdFlashProgramByte - execute flash programming.
*
* This routine will program the flash memory with the specified data.
*
* RETURNS: OK, ERROR.
*/
STATUS amdFlashProgramByte ( unsigned int flashAddr, unsigned char byte )
{
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_FIRST_ADDR) = AMD_FLASH_FIRST_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_SECOND_ADDR) = AMD_FLASH_SECOND_DATA;
   *((unsigned char *)ROM_BASE_ADRS + AMD_FLASH_THIRD_ADDR) = AMD_FLASH_THIRD_PROG_DATA;
   *((unsigned char *)flashAddr) = byte;
  
   /* Spin here until the byte is programmed                                  */
   /* DQ7 outputs the complement of the data programmed to DQ7 until the byte */
   /* is programmed.                                                          */
   while ((*((unsigned char *)flashAddr) & AMD_FLASH_DQ7_MASK) != 
      (byte & AMD_FLASH_DQ7_MASK))
   {
   /*   taskDelay(1);*/
   }
   return OK; 
}

/******************************************************************************
*
* amdFlashProgram - execute flash programming.
*
* This routine will program the flash memory with the specified data.  Entire
* sectors will be erased as needed to program the data.
*
* RETURNS: OK, ERROR.
*/

STATUS amdFlashProgram ( unsigned int flashAddr, unsigned char *data, 
   int length )
{
   unsigned int sector, startSector, endSector;
   int i;
      
   startSector = (flashAddr & (flashGetSectSize() * flashGetNumSect() - 1)) / 
      flashGetSectSize();
   endSector = ((flashAddr & (flashGetSectSize() * flashGetNumSect() - 1)) + 
      (unsigned int)length - 1) / flashGetSectSize();
   
   /* Erase the sectors that will be programmed */
   for (sector = startSector; sector <= endSector; sector++)
   {
      amdFlashSectorErase(sector);
   }
   
   /* Program the data */
   for (i = 0; i < length; i++)
   {
      amdFlashProgramByte(flashAddr, data[i]);
      flashAddr++;
   }
   
   return OK;
}


/******************************************************************************
*
* amdFlashProgramNoErase - execute flash programming.
*
* This routine will program the flash memory with the specified data.  This 
* function assumes that the flash has been erased before programming.
*
* RETURNS: OK, ERROR.
*/

STATUS amdFlashProgramNoErase ( unsigned int flashAddr, unsigned char *data, 
   int length )
{
   int i;
         
   /* Program the data */
   for (i = 0; i < length; i++)
   {
      amdFlashProgramByte(flashAddr, data[i]);
      flashAddr++;
   }
   
   return OK;
}


/******************************************************************************
*
*  Generic routines to access flash parts.
*
*****************************************************************************/

/******************************************************************************
*
* flashInit - initialize the flash driver.
*
* This routine tries to determine what type of flash device is present.  Once
* the device has been determined the appropriate flash driver is configured.
*
* RETURNS: OK, ERROR.
*/

STATUS flashInit ( void )
{
   int i;
   
   /* Set the default flash type */
   /*currFlashDevice = FLASH_DEFAULT_FLASH_DEV;*/   
   
   /* Try to determine what type of flash is installed. */
   for (i = 0; i < FLASH_NUM_DEVICES; i++)
   {
      if (((FlashDevices[i].flashGetMfgId)() == FlashIds[i][FLASH_IDS_MFG]) && 
         ((FlashDevices[i].flashGetDevId)() == FlashIds[i][FLASH_IDS_DEV]))
      {
         /* NOTE:  THIS CODE WAS ORIGINALLY WRITTEN TO HANDLE DIFFERENT FLASH */
         /*        TYPES. FOR NOW THIS FUNCTION IS DISABLED                   */ 
         /*currFlashDevice = i; */
         break;
      }
   }
   
   return OK;
}


/******************************************************************************
*
* flashGetDevId - get the manufacturer id.
*
* This routine returns the device id for the flash device.
*
* RETURNS: valid id or -1.
*/

int flashGetDevId ( void )
{
   return ((FlashDevices[currFlashDevice].flashGetDevId)());
}


/******************************************************************************
*
* flashGetMfgId - get the manufacturer id.
*
* This routine returns the manufacturer id for the flash device.
*
* RETURNS: valid id or -1.
*/

int flashGetMfgId ( void )
{
   return ((FlashDevices[currFlashDevice].flashGetMfgId)());
}


/******************************************************************************
*
* flashGetNumSect - get the number of sectors.
*
* This routine returns the number of sectors for the flash device.
*
* RETURNS: number of sectors.
*/

int flashGetNumSect ( void )
{
   return ((FlashDevices[currFlashDevice].flashGetNumSect)());
}


/******************************************************************************
*
* flashGetSectSize - get the number of bytes in a sector.
*
* This routine returns the number of bytes in a sector for the flash device.
*
* RETURNS: number of sectors.
*/

int flashGetSectSize ( void )
{
   return ((FlashDevices[currFlashDevice].flashGetSectSize)());
}



/******************************************************************************
*
* flashReset - reset the flash device.
*
* This routine sends a flash reset command to the flash device.
*
* RETURNS: OK, ERROR.
*/

STATUS flashReset ( void )
{
   return ((FlashDevices[currFlashDevice].flashReset)());
}


/******************************************************************************
*
* flashChipErase - erase flash device.
*
* This routine sends a command to erase the entire flash device.
*
* RETURNS: OK, ERROR.
*/

STATUS flashChipErase ( void )
{
   return ((FlashDevices[currFlashDevice].flashErase)());
}


/******************************************************************************
*
* flashSectorErase - erase one sector of a flash device.
*
* This routine sends a command to erase one sector of a flash device.
*
* RETURNS: OK, ERROR.
*/

STATUS flashSectorErase ( unsigned int sector )
{
   return ((FlashDevices[currFlashDevice].flashSectorErase)(sector));
}


/******************************************************************************
*
* flashProgram - execute flash programming.
*
* This routine will program the flash memory with the specified data.  Entire
* sectors will be erased as needed to program the data.
*
* RETURNS: OK, ERROR.
*/

STATUS flashProgram ( unsigned int flashAddr, unsigned char *data,
   int length )
{
   return ((FlashDevices[currFlashDevice].flashProgram)(flashAddr, data, length));
}


/******************************************************************************
*
* flashProgramNoErase - execute flash programming.
*
* This routine will program the flash memory with the specified data.  This 
* function assumes that the flash has been erased before programming.
*
* RETURNS: OK, ERROR.
*/

STATUS flashProgramNoErase ( unsigned int flashAddr, unsigned char *data, 
   int length )
{
   return ((FlashDevices[currFlashDevice].flashProgramNoErase)(flashAddr, data, length));
}


/******************************************************************************
*
* vxBootLoadHex - load boot code into the bootflash.
*
* This routine will program the bootflash with the specified file.  Entire
* sectors will be erased as needed to program the data.
*
* RETURNS: OK, ERROR.
*/
#define MAX_SREC_LENGTH                80
#define SREC_TYPE_SIZE                 2
#define SREC_COUNT_SIZE                2
#define SREC_S3_ADDR_SIZE              8
#define SREC_DATA_SIZE                 64
#define SREC_DATA_ELEMENT_SIZE         2
#define SREC_CHECKSUM_SIZE             2

#define BOOT_VERSION_SIZE 8
STATUS vxBootLoadHex ( char *fileName )
{
	int fd, i, srecCnt, isVerifyOk;
   char sRecord[MAX_SREC_LENGTH];
   char sRecCount[SREC_COUNT_SIZE + 1];
   char sRecAddr[SREC_S3_ADDR_SIZE + 1];
   char sRecData[SREC_DATA_ELEMENT_SIZE + 1];
   char sRecChecksum[SREC_CHECKSUM_SIZE + 1];
   unsigned char data[SREC_DATA_SIZE + 1];
   unsigned int addr; 
   unsigned int count, lastEraseSect, startAddrSect, endAddrSect;
   unsigned int checkSum, calcChkSum;
   
   FILE *f = fopen(fileName, "r");
   if (!f)
   {
      printf("vxBootLoadHex ERROR: unable to open %s\n", fileName);
      return ERROR;
   }
   
   if ((fd = fileno(f)) == -1)
   {
      printf("vxBootLoadHex ERROR: unable to create file desciptor for file %s\n", fileName);
      fclose(f);
      return ERROR;
   }
    
   /* Reset the flash */ 
   if (flashReset() != OK)
   {
      printf("vxBootLoadHex ERROR: unable to reset flash\n");
      fclose(f);
      return ERROR;
   }
      
   printf("Programming flash");
   srecCnt = 0;
   lastEraseSect = 0xffffffff;
   isVerifyOk = TRUE;
   while (fioRdString(fd, sRecord, MAX_SREC_LENGTH) != EOF)
   {
      if (!strncmp("S3", sRecord, SREC_TYPE_SIZE))
      {
         /* Extract the count field of the S-Record */
         strncpy(sRecCount, sRecord + SREC_TYPE_SIZE, SREC_COUNT_SIZE);
         sRecCount[SREC_COUNT_SIZE] = 0; 
         count = (unsigned int) strtoul(sRecCount, 0, 16); 
         
         /* Extract the address stored in the S-Record */
         strncpy(sRecAddr, sRecord + SREC_TYPE_SIZE + SREC_COUNT_SIZE, SREC_S3_ADDR_SIZE);
         sRecAddr[SREC_S3_ADDR_SIZE] = 0; 
         addr = (unsigned int) strtoul(sRecAddr, 0, 16);  
        
         /* Extract the data in the S-Record */
         for (i = 0; i < (count - (SREC_CHECKSUM_SIZE + SREC_S3_ADDR_SIZE)/2); i++)
         {
            strncpy(sRecData, sRecord + SREC_TYPE_SIZE + SREC_COUNT_SIZE + SREC_S3_ADDR_SIZE +
               (i * SREC_DATA_ELEMENT_SIZE) , SREC_DATA_ELEMENT_SIZE);
            sRecData[SREC_DATA_ELEMENT_SIZE] = 0;
            data[i] = (unsigned char) strtoul(sRecData, 0, 16);
         }
        
         /* Extract the checksum field of the S-Record */
         strncpy(sRecChecksum, sRecord + SREC_TYPE_SIZE + SREC_COUNT_SIZE + count*2 - 
            SREC_CHECKSUM_SIZE, SREC_CHECKSUM_SIZE);
         sRecChecksum[SREC_CHECKSUM_SIZE] = 0; 
         checkSum = (unsigned int) strtoul(sRecChecksum, 0, 16); 
        
         /* calculate a check sum */
         calcChkSum = 0;
        
         calcChkSum += count;
         calcChkSum += addr & 0xff;
         calcChkSum += (addr>>8) & 0xff;
         calcChkSum += (addr>>16) & 0xff;
         calcChkSum += (addr>>24) & 0xff;
         for(i = 0; i < count - (SREC_CHECKSUM_SIZE + SREC_S3_ADDR_SIZE)/2; i++)
         {
            calcChkSum += data[i];
         }
        
         calcChkSum = (~calcChkSum) & 0xff;
        
         if (calcChkSum != checkSum)
         {
            printf("vxBootLoadHex WARNING: S-Record contains invalid checksum: addr(0x%x), checksum(0x%x)\n",
               addr, checkSum);
         }
              
         /* If we will cross a sector boundary or this is the first sector */
         /* then erase the sector. */
         startAddrSect = (addr - ROM_BASE_ADRS) / flashGetSectSize();
         endAddrSect = ((addr - ROM_BASE_ADRS) + (count - (SREC_CHECKSUM_SIZE + SREC_S3_ADDR_SIZE)/2)) / flashGetSectSize();
         if ((lastEraseSect != startAddrSect))
         {
            flashSectorErase(startAddrSect);
            lastEraseSect = startAddrSect;
         }  
         
         if ((lastEraseSect != endAddrSect))
         {
            flashSectorErase(endAddrSect);
            lastEraseSect = endAddrSect;
         }  
         
         /* program the flash if a full sector of data has been collected  */
         if (flashProgramNoErase(addr, data, count - (SREC_CHECKSUM_SIZE + SREC_S3_ADDR_SIZE)/2) != OK)
         {
            printf("vxBootLoadHex WARNING: unable to program S-Record: addr(0x%x)\n", addr);
         }
         
         /* verify data was written properly */
         for (i = 0; i < (count - (SREC_CHECKSUM_SIZE + SREC_S3_ADDR_SIZE)/2); i++)
         {
            if (*(((unsigned char *)addr) + i) != data[i])
            {
               isVerifyOk = FALSE;
            }
         }
        
         if (++srecCnt == 1000)
         {
            printf(".");
            taskDelay(1);
            srecCnt = 0;
         }
      }
   }
   
   /* Reset the flash */ 
   if (flashReset() != OK)
   {
      printf("vxBootLoadHex ERROR: unable to reset flash\n");
   }
   
   printf("Boot flash programmed!\n");
   
   if (!isVerifyOk)
   {
      printf("vxBootLoad WARNING!!!! Verify failed!!!!!!!!!!!!!\n");
   }
   
   fclose(f);

	return OK;
}



/******************************************************************************
*
* vxBootLoadBin - load boot code into the bootflash.
*
* This routine will program the bootflash with the specified file.  Entire
* sectors will be erased as needed to program the data.
*
* RETURNS: OK, ERROR.
*/
#define BOOT_VERSION_SIZE 8
#define FLASH_SECTOR_SIZE 65536
STATUS vxBootLoadBin ( char *fileName )
{
   char bootVer[BOOT_VERSION_SIZE+1];
   char *buf;
   unsigned int startAddr, endAddr;
   int size, isVerifyOk, i;
   
   
   FILE *f = fopen(fileName, "rb");
   if (!f)
   {
      printf("vxBootLoadBin ERROR: unable to open %s\n", fileName);
      return ERROR;
   }
   
   /* Reset the flash */
   if (flashReset() != OK)
   {
      printf("vxBootLoadBin ERROR: unable to reset flash\n");
      fclose(f);
      return ERROR;
   }
   
   fread (bootVer, 1, BOOT_VERSION_SIZE, f);
   bootVer[BOOT_VERSION_SIZE] = 0;
   
   fread ((char *)&startAddr, 1, sizeof(startAddr), f);
   fread ((char *)&endAddr, 1, sizeof(endAddr), f);
      
   printf("Programming flash with version %s startAddr(0x%x) endAddr(0x%x)\n", 
      bootVer, startAddr, endAddr);
      
   buf = malloc(flashGetSectSize());
   
   /* Skip a dummy line. */
   fread (buf, 1, 16, f);
   
   isVerifyOk = TRUE;
   while ((size = fread(buf, 1 , FLASH_SECTOR_SIZE, f)) != 0)
   {
      if (flashProgram(startAddr, buf, size) != OK)
      {
         printf ("vxBootLoadBin ERROR: Unable to program sector at 0x%x\n", startAddr);
      }
      
      /* verify tha the sector was programmed ok */
      for (i = 0; i < size; i++)
      {
         if (*(((unsigned char *)startAddr) + i) != buf[i])
         {
            isVerifyOk = FALSE;
         }
      }
      startAddr += size;  
      printf(".");
      taskDelay(1);    
   }
   printf("Boot flash programmed!\n");
   
   if (!isVerifyOk)
   {
      printf("vxBootLoadBin WARNING!!!! Verify failed!!!!!!!!!!!!!\n");
   }
   
   /* Reset the flash */ 
   if (flashReset() != OK)
   {
      printf("vxBootLoadBin ERROR: unable to reset flash\n");
      free(buf);
      fclose(f);
      return ERROR;
   }
   
   free(buf);
   fclose(f);
   
   return OK;
}
