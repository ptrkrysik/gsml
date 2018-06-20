/******************************************************************            

 (c) Copyright Cisco 2000
 All Rights Reserved

******************************************************************/

/*
JetCell modification history
----------------------------
99/07/29  to   Initial Version
*/

/*
 * File name     : bootFlash.h
 * Description   : Constants and Definitions for bootFlash.h
 * History:
 *
 */

#ifndef _BOOTFLASH_H
#define _BOOTFLASH_H

#include "vxWorks.h"




/******************************************************************************
*
* vxBootLoadHex - load a hex (Motorola S-Records) bootfile into the bootflash.
*
* This routine will program the bootflash with the specified file.  Entire
* sectors will be erased as needed to program the data.
*
* RETURNS: OK, ERROR.
*/
extern STATUS vxBootLoadHex ( char *fileName );

/******************************************************************************
*
* vxBootLoadBin - load a binary bootfile into the bootflash.
*
* This routine will program the bootflash with the specified file.  Entire
* sectors will be erased as needed to program the data.
*
* RETURNS: OK, ERROR.
*/
extern STATUS vxBootLoadBin ( char *fileName );


extern STATUS flashInit(void);


extern int flashGetMfgId (void);
extern int flashGetDevId (void);
extern int flashGetNumSect (void);
extern int flashGetSectSize (void);
extern STATUS flashErase (void);
extern STATUS flashSectorErase (unsigned int sector);
extern STATUS flashReset (void); 
extern STATUS flashProgram (unsigned int flashAddr, unsigned char *data, int length);
extern STATUS flashProgramNoErase (unsigned int flashAddr, unsigned char *data, int length);






typedef struct {
   int (*flashGetMfgId) (void);
   int (*flashGetDevId) (void);
   int (*flashGetNumSect) (void);
   int (*flashGetSectSize) (void);
   STATUS (*flashErase) (void);
   STATUS (*flashSectorErase) (unsigned int sector);
   STATUS (*flashReset) (void); 
   STATUS (*flashProgram) (unsigned int flashAddr, unsigned char *data, int length);
   STATUS (*flashProgramNoErase) (unsigned int flashAddr, unsigned char *data, int length);
} flashDevice_t;
            
 

#endif /* _BOOTFLASH_H */
