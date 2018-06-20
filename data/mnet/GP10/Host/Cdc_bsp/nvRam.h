/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*
JetCell modification history
----------------------------
99/07/19  to   Initial Version
*/

/*
 * File name     : nvRam.h
 * Description   : Constants and Definitions for NV Ram
 * History:
 *
 */

#ifndef _NVRAM_H
#define _NVRAM_H

#include "bootFlash.h"

/* Definitions for power on tests */
#define POST_PASS                      1
#define POST_FAIL                      0
#define POST_ENABLE_TEST               1
#define POST_DISABLE_TEST              0

/* Amount of stack memory tested at initial startup */
#define STACK_MEM_TEST_SIZE             0x10000

#define MPC860_SDRAM_TEST               0x1
#define DSP_0_INTERNAL_DATA_RAM_TEST    0x2
#define DSP_0_INTERNAL_PROG_RAM_TEST    0x4
#define DSP_0_EXTERNAL_RAM_TEST         0x8
#define DSP_1_INTERNAL_DATA_RAM_TEST    0x10
#define DSP_1_INTERNAL_PROG_RAM_TEST    0x20
#define DSP_1_EXTERNAL_RAM_TEST         0x40
#define FPGA_0_TEST                     0x80
#define FPGA_1_TEST                     0x100
#define I2C_TEST                        0x200
#define ALL_POWER_ON_TESTS             (MPC860_SDRAM_TEST | \
                                        DSP_0_INTERNAL_DATA_RAM_TEST | \
                                        DSP_0_INTERNAL_PROG_RAM_TEST | \
                                        DSP_0_EXTERNAL_RAM_TEST | \
                                        DSP_1_INTERNAL_DATA_RAM_TEST | \
                                        DSP_1_INTERNAL_PROG_RAM_TEST | \
                                        DSP_1_EXTERNAL_RAM_TEST | \
                                        FPGA_0_TEST | \
                                        FPGA_1_TEST | \
                                        I2C_TEST )   

#define NV_RAM_SECT_FROM_END        8
#define NV_RAM_SIZE  (NV_RAM_SECT_FROM_END * flashGetSectSize())


#define MAX_VC_SN       64     /* Max length of the ViperCell serial number */
#define MAX_VC_PN       64     /* Max length of the ViperCell part number */
#define MAX_CDC_SN      64     /* Max length of the CDC serial number */
#define MAX_CDC_PN      64     /* Max length of the CDC part number */

#define RADIO_BOARD_EEPROM_SIZE     512
#define CLOCK_BOARD_EEPROM_SIZE     20

extern STATUS postSetTestResult (unsigned int testBit, unsigned int result);
extern unsigned int postGetTestResult (unsigned int testBit);
extern STATUS postSetTestMask (unsigned int testBit, unsigned int mask);
extern unsigned int postGetTestMask (unsigned int testBit);

extern STATUS sysIpAddrSet (char *ipAddr);
extern STATUS sysIpAddrGet (char *ipAddr);

extern STATUS sysSerialNumSet (char *sn);
extern STATUS sysSerialNumGet (char *p);
extern STATUS sysVCPartNumSet (char *partNum);
extern STATUS sysVCPartNumGet (char *partNum);
extern STATUS sysCDCSerialNumSet (char *serNum);
extern STATUS sysCDCSerialNumGet (char *serNum);
extern STATUS sysCDCPartNumSet (char *partNum);
extern STATUS sysCDCPartNumGet (char *partNum);

extern STATUS sysRadioBoardEepromGet(char *pEeprom);
extern STATUS sysRadioBoardEepromSet(char *pEeprom);
extern int sysIsRadioBoardEepromSet();

extern STATUS sysClockBoardEepromGet(char *pEeprom);
extern STATUS sysClockBoardEepromSet(char *pEeprom);
extern int sysIsClockBoardEepromSet();

extern int sysNvRamGet (char *, int, int);
extern int sysNvRamSet (char *, int, int);


#endif /* _NVRAM_H */
