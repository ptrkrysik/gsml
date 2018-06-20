/* ataDrv.c - ATA/IDE (LOCAL and PCMCIA) disk device driver */

/* Copyright 1989-1998 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
modification history
--------------------
01j,31mar98,map  removed INCLUDE_ATA, redefined sys* prototypes.
01i,23mar98,map  renamed macros, made endian safe, added docs.
01h,30oct97,db   added cmd to reinitialize controller with params read. fixed
                 bug reported in SPR #9139. used PCI macros for input/output.
01g,21apr97,hdn  fixed a semaphore timeout problem(SPR 8394).
01f,28feb97,dat  fixed SPRs 8084, 3273 from ideDrv.
01e,06nov96,dgp  doc: final formatting
01d,01nov96,hdn  added support for PCMCIA.
01c,25sep96,hdn  added support for ATA-2.
01b,01mar96,hdn  cleaned up.
01a,02mar95,hdn  written based on ideDrv.c.
*/

/*
DESCRIPTION

This is a driver for ATA/IDE devices on PCMCIA, ISA, and other buses. The
driver can be customized via various macros to run on a variety of boards and
both big-endian, and little endian CPUs.

USER-CALLABLE ROUTINES

Most of the routines in this driver are accessible only through the I/O
system.  However, two routines must be called directly:  ataDrv() to
initialize the driver and ataDevCreate() to create devices.

Before the driver can be used, it must be initialized by calling ataDrv().
This routine must be called exactly once, before any reads, writes, or
calls to ataDevCreate().  Normally, it is called from usrRoot() in
usrConfig.c.

The routine ataRawio() supports physical I/O access. The first
argument is a drive number, 0 or 1; the second argument is a pointer
to an ATA_RAW structure.

NOTE
Format is not supported, because ATA/IDE disks are already formatted, and bad
sectors are mapped.

PARAMETERS
The ataDrv() function requires a configuration flag as a parameter.
The configuration flag is one of the following:

.TS
tab(|);
l l .
Transfer mode 

ATA_PIO_DEF_0    | PIO default mode
ATA_PIO_DEF_1    | PIO default mode, no IORDY
ATA_PIO_0        | PIO mode 0
ATA_PIO_1        | PIO mode 1
ATA_PIO_2        | PIO mode 2
ATA_PIO_3        | PIO mode 3
ATA_PIO_4        | PIO mode 4
ATA_PIO_AUTO     | PIO max supported mode
ATA_DMA_0        | DMA mode 0
ATA_DMA_1        | DMA mode 1
ATA_DMA_2        | DMA mode 2
ATA_DMA_AUTO     | DMA max supported mode

Transfer bits

ATA_BITS_16      | RW bits size, 16 bits
ATA_BITS_32      | RW bits size, 32 bits

Transfer unit

ATA_PIO_SINGLE   | RW PIO single sector
ATA_PIO_MULTI    | RW PIO multi sector
ATA_DMA_SINGLE   | RW DMA single word
ATA_DMA_MULTI    | RW DMA multi word

Geometry parameters

ATA_GEO_FORCE    | set geometry in the table
ATA_GEO_PHYSICAL | set physical geometry
ATA_GEO_CURRENT  | set current geometry
.TE

DMA transfer is not supported in this release.  If ATA_PIO_AUTO or ATA_DMA_AUTO
is specified, the driver automatically chooses the maximum mode supported by the
device.  If ATA_PIO_MULTI or ATA_DMA_MULTI is specified, and the device does
not support it, the driver automatically chooses single sector or word mode.
If ATA_BITS_32 is specified, the driver uses 32-bit transfer mode regardless of
the capability of the drive.  

If ATA_GEO_PHYSICAL is specified, the driver uses the physical geometry 
parameters stored in the drive.  If ATA_GEO_CURRENT is specified,
the driver uses current geometry parameters initialized by BIOS.  
If ATA_GEO_FORCE is specified, the driver uses geometry parameters stored 
in sysLib.c.

The geometry parameters are stored in the structure table
`ataTypes[]' in sysLib.c. That table has two entries, the first for
drive 0, the second for drive 1. The members of the structure
are:
.CS
    int cylinders;              /@ number of cylinders @/
    int heads;                  /@ number of heads @/
    int sectors;                /@ number of sectors per track @/
    int bytes;                  /@ number of bytes per sector @/
    int precomp;                /@ precompensation cylinder @/
.CE

This driver does not access the PCI-chip-set IDE interface, but rather takes
advantage of BIOS initialization.  Thus, the BIOS setting should match
the modes specified by the configuration flag.

INTERNAL

Delays should be implemented using sysMsDelay() routine rather than BSP/CPU
dependant sysDelay(). 

SEE ALSO:
.pG "I/O System"
*/

#include "vxWorks.h"
#include "taskLib.h"
#include "ioLib.h"
#include "memLib.h"
#include "stdlib.h"
#include "errnoLib.h"
#include "stdio.h"
#include "string.h"
#include "private/semLibP.h"
#include "intLib.h"
#include "iv.h"
#include "wdLib.h"
#include "sysLib.h"
#include "sys/fcntlcom.h"
#include "drv/pcmcia/pcmciaLib.h"

#include "drv/hdisk/ataDrv.h"

#define CF_DATA_BASE_ADDR  0x80800000
#define CF_REG_BASE_ADDR  0x80C00000

#define CF_DATA(base0)      (base0 + 0x10) /* (RW) data register (16 bits) */
#define CF_ERROR(base0)         (base0 + 2) /* (R)  error register      */
#define CF_FEATURE(base0)   (base0 + 2) /* (W)  feature/precompensation */
#define CF_SECCNT(base0)    (base0 + 4) /* (RW) sector count        */
#define CF_SECTOR(base0)    (base0 + 6) /* (RW) first sector number     */
#define CF_CYL_LO(base0)    (base0 + 8) /* (RW) cylinder low byte       */
#define CF_CYL_HI(base0)    (base0 + 10) /* (RW) cylinder high byte     */
#define CF_SDH(base0)       (base0 + 12) /* (RW) sector size/drive/head  */
#define CF_COMMAND(base0)   (base0 + 14) /* (W)  command register       */
#define CF_STATUS(base0)    (base0 + 14) /* (R)  immediate status       */
#define CF_A_STATUS(base1)  (base1 + 28) /* (R)  alternate status       */
#define CF_D_CONTROL(base1) (base1 + 28) /* (W)  disk controller control */
#define CF_D_ADDRESS(base1) (base1 + 30) /* (R)  disk controller address */

#define VXDOS                           "VXDOS"
#define VXEXT                           "VXEXT"

/* imports */

IMPORT ATA_TYPE     ataTypes [ATA_MAX_CTRLS][ATA_MAX_DRIVES];
IMPORT ATA_RESOURCE ataResources [ATA_MAX_CTRLS];
IMPORT void sysOutByte (int port, char data);
IMPORT UCHAR    sysInByte (int port);
IMPORT void sysInWordString (int port, short *pData, int count);
IMPORT void sysInWordStringRev (int port, short *pData, int count);
IMPORT void sysOutWordString (int port, short *pData, int count);
IMPORT void sysInLongString (int port, long *pData, int count);
IMPORT void sysOutLongString (int port, long *pData, int count);
IMPORT STATUS   sysIntEnablePIC (int intNum);
IMPORT void sysMsDelay (UINT uSecs);
IMPORT void sysDelay (void);

/* defines */

#if 0  
/* Read a BYTE from IO port, `ioAdrs' */

#ifndef ATA_IO_BYTE_READ
#define ATA_IO_BYTE_READ(ioAdrs)    sysInByte (ioAdrs)
#endif  /* ATA_IO_BYTE_READ */

/* Write a BYTE `byte' to IO port, `ioAdrs' */

#ifndef ATA_IO_BYTE_WRITE
#define ATA_IO_BYTE_WRITE(ioAdrs, byte) sysOutByte (ioAdrs, byte)
#endif  /* ATA_IO_BYTE_WRITE */

/* Read 16-bit little-endian `nWords' into `pData' from IO port, `ioAdrs' */

#ifndef ATA_IO_NWORD_READ
#define ATA_IO_NWORD_READ(ioAdrs, pData, nWords)                        \
    sysInWordString (ioAdrs, pData, nWords)
#endif  /* ATA_IO_NWORD_READ */

/* Write 16-bit little-endian `nWords' from `pData' into IO port, `ioAdrs' */

#ifndef ATA_IO_NWORD_WRITE
#define ATA_IO_NWORD_WRITE(ioAdrs, pData, nWords)                       \
    sysOutWordString (ioAdrs, pData, nWords)
#endif /* ATA_IO_NWORD_WRITE */
    
/* Read 32-bit little-endian `nLongs' into `pData' from IO port, `ioAdrs' */

#ifndef ATA_IO_NLONG_READ
#define ATA_IO_NLONG_READ(ioAdrs, pData, nLongs)                        \
    sysInLongString (ioAdrs, pData, nLongs)
#endif  /* ATA_IO_NLONG_READ */

/* Write 32-bit little-endian `nLongs' from `pData' into IO port, `ioAdrs' */

#ifndef ATA_IO_NLONG_WRITE
#define ATA_IO_NLONG_WRITE(ioAdrs, pData, nLongs)                       \
    sysOutLongString (ioAdrs, pData, nLongs)
#endif  /* ATA_IO_NLONG_WRITE */

/* Read 32-bit CPU-endian `nWords' into `pData' from IO port, `ioAdrs' */

#ifndef ATA_IO_NWORD_READ_SWAP
#  if (_BYTE_ORDER == _BIG_ENDIAN)
#  define ATA_IO_NWORD_READ_SWAP(ioAdrs, pData, nWords)                 \
    sysInWordStringRev (ioAdrs, pData, nWords)
#  else /* (_BYTE_ORDER == _BIG_ENDIAN) */
#  define ATA_IO_NWORD_READ_SWAP(ioAdrs, pData, nWords)                 \
    ATA_IO_NWORD_READ (ioAdrs, pData, nWords)
#  endif /* (_BYTE_ORDER == _BIG_ENDIAN) */
#endif  /* ATA_IO_NLONG_READ_SWAP */

#endif /* 0 */

#define ATA_IO_BYTE_READ  ataInByte
#define ATA_IO_BYTE_WRITE  ataOutByte
#define ATA_IO_NWORD_READ  ataInWordStringRev
#define ATA_IO_NWORD_WRITE  ataOutWordStringRev
#define ATA_IO_NWORD_READ_SWAP  ataInWordString


/* globals */

BOOL      ataDrvInstalled = FALSE;  /* TRUE if installed */
ATA_CTRL  ataCtrl [ATA_MAX_CTRLS];

int     ataNumErrs = 0;
int     ataCFStatErrs = 0;

BOOL    ataWriteVerifyBufSet = FALSE;
int     ataWriteVerifyBufSize;
int     ataWriteVerifyNumErrs = 0;
char    *ataWriteVerifyBuf;


/* locals */

LOCAL int       ataRetry = 3;       /* max retry count */


/* function prototypes */

LOCAL STATUS ataBlkRd   (ATA_DEV *pDev, int startBlk, int nBlks, char *p);
LOCAL STATUS ataBlkWrt  (ATA_DEV *pDev, int startBlk, int nBlks, char *p);
LOCAL STATUS ataReset   (ATA_DEV *pDev);
LOCAL STATUS ataStatus  (ATA_DEV *pDev);
LOCAL STATUS ataIoctl   (ATA_DEV *pDev, int function, int arg);
LOCAL STATUS ataBlkRW   (ATA_DEV *pDev, int startBlk, int nBlks, char *p,
             int direction);
LOCAL void   ataWdog    (int ctrl);
/* LOCAL void   ataIntr (int ctrl); */
LOCAL STATUS ataInit    (int ctrl);
LOCAL void   ataWait    (int ctrl, int request);
LOCAL STATUS ataCmd (int ctrl, int drive, int cmd, int arg0, int arg1);
LOCAL STATUS ataPread   (int ctrl, int drive, void *p);
LOCAL STATUS ataRW  (int ctrl, int drive, int cylinder, int head, int sec, 
             void *p, int nSecs, int direction);
LOCAL STATUS ataWVerify (int ctrl, int drive, int cylinder, int head, int sec, 
             void *p, int nSecs, int direction);


/*******************************************************************************
ataInByte

Read a BYTE from IO port, `ioAdrs' 
*/
UCHAR ataInByte(int port)
{
  __asm__ volatile ("   eieio;  sync");
  return *(short*)port; 
}

/*******************************************************************************
ataOutByte

Write a BYTE `byte' to IO port, `ioAdrs' 

*/
void ataOutByte(int port, char data)
{
  __asm__ volatile ("   eieio;  sync");
  *(short*)port = data;
}

/*******************************************************************************
ataInWordStringRev

Read 16-bit little-endian `nWords' into `pData' from IO port, `ioAdrs' 
*/
void ataInWordStringRev(int port, volatile short* pData, int nWords)
{
  int i;
  unsigned short temp;

  for (i = 0; i < nWords; i++)
  {
    temp = *(short*)port;
  	__asm__ volatile ("   eieio;  sync");
    *pData++ = (temp >> 8) | (temp << 8);
  }
}

/*******************************************************************************
ataOutWordStringRev

Write 16-bit little-endian `nWords' from `pData' into IO port, `ioAdrs' 
*/
void ataOutWordStringRev(int port, volatile short* pData, int nWords)
{
  int i;
  unsigned short temp;

  for (i = 0; i < nWords; i++)
  {
    temp = *pData++;
  	__asm__ volatile ("   eieio;  sync");
    *(short*)port = (temp >> 8) | (temp << 8);
  }
}
 
/*******************************************************************************
ataInWordString

Read 16-bit CPU-endian `nWords' into `pData' from IO port, `ioAdrs' 

*/
void ataInWordString(int port, volatile short* pData, int nWords)
{
  int i;

  for (i = 0; i < nWords; i++)
  {
  	__asm__ volatile ("   eieio;  sync");
    *pData++ = *(short*)port;
  }
}


/*******************************************************************************
*
* ataDrv - initialize the ATA driver
*
* This routine initializes the ATA/IDE driver, sets up interrupt vectors,
* and performs hardware initialization of the ATA/IDE chip.
*
* This routine must be called exactly once, before any reads, writes,
* or calls to ataDevCreate().  Normally, it is called by usrRoot()
* in usrConfig.c.
*
* RETURNS: OK, or ERROR if initialization fails.
*
* SEE ALSO: ataDevCreate()
*/

STATUS ataDrv
(
    int  ctrl,          /* controller no. */
    int  drives,        /* number of drives */
    int  vector,        /* interrupt vector */
    int  level,         /* interrupt level */
    BOOL configType,        /* configuration type */
    int  semTimeout,        /* timeout seconds for sync semaphore */
    int  wdgTimeout     /* timeout seconds for watch dog */
)
{
    ATA_CTRL *pCtrl     = &ataCtrl[ctrl];
/*
    ATA_RESOURCE *pAta      = &ataResources[ctrl];
    PCCARD_RESOURCE *pResource  = &pAta->resource;
*/
    ATA_DRIVE *pDrive;
    ATA_PARAM *pParam;
    ATA_TYPE *pType;
    int drive;
    int ix;


    if ((ctrl >= ATA_MAX_CTRLS) || (drives > ATA_MAX_DRIVES))
        return (ERROR);

    if (!ataDrvInstalled)
    {
        for (ix = 0; ix < ATA_MAX_CTRLS; ix++)
            ataCtrl[ix].wdgId = wdCreate ();
        ataDrvInstalled = TRUE;
    }

    if (!pCtrl->installed)
    {
        if (semTimeout == 0)
            pCtrl->semTimeout = ATA_SEM_TIMEOUT_DEF;
        else
            pCtrl->semTimeout = semTimeout;

        if (wdgTimeout == 0)
            pCtrl->wdgTimeout = ATA_WDG_TIMEOUT_DEF;
        else
            pCtrl->wdgTimeout = wdgTimeout;

/*        semBInit (&pCtrl->syncSem, SEM_Q_FIFO, SEM_EMPTY);
*/
        semMInit (&pCtrl->muteSem, SEM_Q_PRIORITY | SEM_DELETE_SAFE |
              SEM_INVERSION_SAFE);
/*

    pCtrl->data = ATA_DATA  (pResource->ioStart[0]);
    pCtrl->error    = ATA_ERROR (pResource->ioStart[0]);
    pCtrl->feature  = ATA_FEATURE   (pResource->ioStart[0]);
    pCtrl->seccnt   = ATA_SECCNT    (pResource->ioStart[0]);
    pCtrl->sector   = ATA_SECTOR    (pResource->ioStart[0]);
    pCtrl->cylLo    = ATA_CYL_LO    (pResource->ioStart[0]);
    pCtrl->cylHi    = ATA_CYL_HI    (pResource->ioStart[0]);
    pCtrl->sdh  = ATA_SDH   (pResource->ioStart[0]);
    pCtrl->command  = ATA_COMMAND   (pResource->ioStart[0]);
    pCtrl->status   = ATA_STATUS    (pResource->ioStart[0]);
    pCtrl->aStatus  = ATA_A_STATUS  (pResource->ioStart[1]);
    pCtrl->dControl = ATA_D_CONTROL (pResource->ioStart[1]);
    pCtrl->dAddress = ATA_D_ADDRESS (pResource->ioStart[1]);
*/
        pCtrl->data = CF_DATA   (CF_DATA_BASE_ADDR);
        pCtrl->error    = CF_ERROR  (CF_REG_BASE_ADDR);
        pCtrl->feature  = CF_FEATURE    (CF_REG_BASE_ADDR);
        pCtrl->seccnt   = CF_SECCNT (CF_REG_BASE_ADDR);
        pCtrl->sector   = CF_SECTOR (CF_REG_BASE_ADDR);
        pCtrl->cylLo    = CF_CYL_LO (CF_REG_BASE_ADDR);
        pCtrl->cylHi    = CF_CYL_HI (CF_REG_BASE_ADDR);
        pCtrl->sdh  = CF_SDH    (CF_REG_BASE_ADDR);
        pCtrl->command  = CF_COMMAND    (CF_REG_BASE_ADDR);
        pCtrl->status   = CF_STATUS (CF_REG_BASE_ADDR);
        pCtrl->aStatus  = CF_A_STATUS   (CF_REG_BASE_ADDR);
        pCtrl->dControl = CF_D_CONTROL  (CF_REG_BASE_ADDR);
        pCtrl->dAddress = CF_D_ADDRESS  (CF_REG_BASE_ADDR);

#if 0
        (void) intConnect ((VOIDFUNCPTR *)INUM_TO_IVEC (vector),
                   (VOIDFUNCPTR)ataIntr, ctrl);
        sysIntEnablePIC (level);    /* unmask the interrupt level */
#endif
        pCtrl->intLevel = level;
        pCtrl->wdgOkay  = TRUE;

        semTake (&pCtrl->muteSem, WAIT_FOREVER);

        if (ataInit (ctrl) != OK)
        {
            semGive (&pCtrl->muteSem);
            return (ERROR);
        }

        for (drive = 0; drive < drives; drive++)
        {
            pType  = &ataTypes[ctrl][drive];
            pDrive = &pCtrl->drive[drive];
            pParam = &pDrive->param;
            if (pType->cylinders == 0)
                break;

            if ((pCtrl->ctrlType == ATA_PCMCIA) ||
            ((pCtrl->ctrlType != ATA_PCMCIA) && (drive == 0)))
            {
                if (ataCmd (ctrl, drive, ATA_CMD_DIAGNOSE, NULL, NULL) != OK)
                {
                    semGive (&pCtrl->muteSem);
                    return (ERROR);
                }
            }

            /* find out geometry */

            if ((configType & ATA_GEO_MASK) == ATA_GEO_FORCE)
            {
                (void) ataCmd (ctrl, drive, ATA_CMD_INITP, NULL, NULL);
                (void) ataPread (ctrl, drive, (char *)pParam);
            }
            else if ((configType & ATA_GEO_MASK) == ATA_GEO_PHYSICAL)
            {
                (void) ataPread (ctrl, drive, (char *)pParam);
                pType->cylinders = pParam->cylinders - 1;
                pType->heads     = pParam->heads;
                pType->sectors   = pParam->sectors;
            }
            else if ((configType & ATA_GEO_MASK) == ATA_GEO_CURRENT)
            {
                (void) ataPread (ctrl, drive, (char *)pParam);
                if ((pParam->currentCylinders != 0) &&
                    (pParam->currentHeads != 0) &&
                    (pParam->currentSectors != 0))
                {
                    pType->cylinders = pParam->currentCylinders - 1;
                    pType->heads     = pParam->currentHeads;
                    pType->sectors   = pParam->currentSectors;
                }
                else
                {
                    pType->cylinders = pParam->cylinders - 1;
                    pType->heads     = pParam->heads;
                    pType->sectors   = pParam->sectors;
                }
            }
            /*
            * reinitialize the controller with parameters read from the
            * controller.
            */

            (void) ataCmd (ctrl, drive, ATA_CMD_INITP, NULL, NULL);

            /* recalibrate */

            (void) ataCmd (ctrl, drive, ATA_CMD_RECALIB, NULL, NULL);

            /* find out supported capabilities of the drive */

            pDrive->multiSecs = pParam->multiSecs & 0x00ff;
            pDrive->okMulti = (pDrive->multiSecs != 0) ? TRUE : FALSE;
            pDrive->okIordy = (pParam->capabilities & 0x0800) ? TRUE : FALSE;
            pDrive->okLba   = (pParam->capabilities & 0x0200) ? TRUE : FALSE;
            pDrive->okDma   = (pParam->capabilities & 0x0100) ? TRUE : FALSE;

            /* find out supported max PIO mode */

            pDrive->pioMode = (pParam->pioMode >> 8) & 0x03;    /* PIO 0,1,2 */
            if (pDrive->pioMode > 2)
                pDrive->pioMode = 0;

            if ((pDrive->okIordy) && (pParam->valid & 0x02))    /* PIO 3,4 */
            {
                if (pParam->advancedPio & 0x01)
                    pDrive->pioMode = 3;
                if (pParam->advancedPio & 0x02)
                    pDrive->pioMode = 4;
            }

            /* find out supported max DMA mode */

            if ((pDrive->okDma) && (pParam->valid & 0x02))
            {
                pDrive->singleDmaMode = (pParam->dmaMode >> 8) & 0x03;
                if (pDrive->singleDmaMode >= 2)
                        pDrive->singleDmaMode = 0;
                pDrive->multiDmaMode  = 0;

                if (pParam->singleDma & 0x04)
                    pDrive->singleDmaMode = 2;
                else if (pParam->singleDma & 0x02)
                    pDrive->singleDmaMode = 1;
                else if (pParam->singleDma & 0x01)
                    pDrive->singleDmaMode = 0;

                if (pParam->multiDma & 0x04)
                    pDrive->multiDmaMode = 2;
                else if (pParam->multiDma & 0x02)
                    pDrive->multiDmaMode = 1;
                else if (pParam->multiDma & 0x01)
                    pDrive->multiDmaMode = 0;
            }

            /* find out transfer mode to use */

            pDrive->rwBits = configType & ATA_BITS_MASK;
            pDrive->rwPio  = configType & ATA_PIO_MASK;
            pDrive->rwDma  = configType & ATA_DMA_MASK;
            pDrive->rwMode = ATA_PIO_DEF_W;

            switch (configType & ATA_MODE_MASK)
            {
            case ATA_PIO_0:
            case ATA_PIO_1:
            case ATA_PIO_2:
            case ATA_PIO_3:
            case ATA_PIO_4:
            case ATA_PIO_DEF_0:
            case ATA_PIO_DEF_1:
                pDrive->rwMode = configType & ATA_MODE_MASK;
                break;
            case ATA_PIO_AUTO:
                 pDrive->rwMode = ATA_PIO_W_0 + pDrive->pioMode;
                break;
            case ATA_DMA_0:
            case ATA_DMA_1:
            case ATA_DMA_2:
                if (pDrive->okDma)
                {
                    if (pDrive->rwDma == ATA_DMA_SINGLE)
                        pDrive->rwMode |= ATA_DMA_SINGLE_0;
                    if (pDrive->rwDma == ATA_DMA_MULTI)
                        pDrive->rwMode |= ATA_DMA_MULTI_0;
                }
                break;
            case ATA_DMA_AUTO:
                if (pDrive->okDma)
                {
                    if (pDrive->rwDma == ATA_DMA_SINGLE)
                        pDrive->rwMode = ATA_DMA_SINGLE_0 + 
                            pDrive->singleDmaMode;
                    if (pDrive->rwDma == ATA_DMA_MULTI)
                        pDrive->rwMode = ATA_DMA_MULTI_0 + 
                            pDrive->multiDmaMode;
                }
                break;
            default:
                break;
            }
            
            /* set the transfer mode */

            (void) ataCmd (ctrl, drive, ATA_CMD_SET_FEATURE, ATA_SUB_SET_RWMODE,
                   pDrive->rwMode);

            if (pDrive->rwPio == ATA_PIO_MULTI)
            {
                if (pDrive->okMulti)
                    (void) ataCmd (ctrl, drive, ATA_CMD_SET_MULTI,
                           pDrive->multiSecs, NULL);
                else
                    pDrive->rwPio = ATA_PIO_SINGLE;
            }
        }

        pCtrl->installed = TRUE;

        semGive (&pCtrl->muteSem);
    }

    return (OK);
}

/*******************************************************************************
*
* ataDevCreate - create a device for a ATA/IDE disk
*
* This routine creates a device for a specified ATA/IDE disk.
*
* <drive> is a drive number for the hard drive; it must be 0 or 1.
*
* The <nBlocks> parameter specifies the size of the device in blocks.
* If <nBlocks> is zero, the whole disk is used.
*
* The <blkOffset> parameter specifies an offset, in blocks, from the start
* of the device to be used when writing or reading the hard disk.  This
* offset is added to the block numbers passed by the file system during
* disk accesses.  (VxWorks file systems always use block numbers beginning
* at zero for the start of a device.)
*
*
* RETURNS:
* A pointer to a block device structure (BLK_DEV) or NULL if memory cannot
* be allocated for the device structure.
*
* SEE ALSO: dosFsMkfs(), dosFsDevInit(), rt11FsDevInit(), rt11FsMkfs(),
* rawFsDevInit()
*/

BLK_DEV *ataDevCreate
(
    int ctrl,
    int drive,
    int nBlocks,
    int blkOffset
)
{
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];
    ATA_TYPE *pType = &ataTypes[ctrl][drive];
    ATA_DEV *pDev;
    BLK_DEV *pBlkdev;
    int     maxBlks;

    if ((ctrl >= ATA_MAX_CTRLS) || (drive >= ATA_MAX_DRIVES) ||
        !ataDrvInstalled || !pCtrl->installed)
        return (NULL);

    if ((pDev = (ATA_DEV *)malloc(sizeof (ATA_DEV))) == NULL)
        return (NULL);

    pBlkdev = &pDev->blkDev;

    maxBlks = (pType->cylinders * pType->heads * pType->sectors) - blkOffset;

    if (nBlocks == 0)
        nBlocks = maxBlks;
    
    if (nBlocks > maxBlks)
        nBlocks = maxBlks;

    pBlkdev->bd_nBlocks     = nBlocks;
    pBlkdev->bd_bytesPerBlk = pType->bytes;
    pBlkdev->bd_blksPerTrack    = pType->sectors;
    pBlkdev->bd_nHeads      = pType->heads;
    pBlkdev->bd_removable   = TRUE;
    pBlkdev->bd_retry       = 1;
    pBlkdev->bd_mode        = O_RDWR;
    pBlkdev->bd_readyChanged    = TRUE;
    pBlkdev->bd_blkRd       = ataBlkRd;
    pBlkdev->bd_blkWrt      = ataBlkWrt;
    pBlkdev->bd_ioctl       = ataIoctl;
    pBlkdev->bd_reset       = ataReset;
    pBlkdev->bd_statusChk   = ataStatus;
    pBlkdev->bd_reset       = NULL;
    pBlkdev->bd_statusChk   = NULL;

    pDev->ctrl          = ctrl;
    pDev->drive         = drive;
    pDev->blkOffset     = blkOffset;

    if (ataWriteVerifyBufSet)
    {
        if (ATA_MAX_RW_SECTORS * pType->bytes > ataWriteVerifyBufSize)
        {
            free (ataWriteVerifyBuf);
            ataWriteVerifyBuf = malloc(ATA_MAX_RW_SECTORS * pType->bytes);
            ataWriteVerifyBufSize = ATA_MAX_RW_SECTORS * pType->bytes;
        }
    }
    else
    {
        ataWriteVerifyBuf = malloc(ATA_MAX_RW_SECTORS * pType->bytes);
        ataWriteVerifyBufSize = ATA_MAX_RW_SECTORS * pType->bytes;
        ataWriteVerifyBufSet = TRUE;
    }

    return (&pDev->blkDev);
}

/*******************************************************************************
*
* ataRawio - do raw I/O access
*
* This routine is called to perform raw I/O access.
*
* <drive> is a drive number for the hard drive: it must be 0 or 1.
*
* The <pAtaRaw> is a pointer to the structure ATA_RAW which is defined in 
* ataDrv.h.
*
* RETURNS:
* OK, or ERROR if the parameters are not valid.
*
*/

STATUS ataRawio
(
    int ctrl,
    int drive,
    ATA_RAW *pAtaRaw
)
{
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];
    ATA_TYPE *pType = &ataTypes[ctrl][drive];
    ATA_DEV ataDev;
    BLK_DEV *pBlkdev    = &ataDev.blkDev;
    UINT startBlk;

    if ((ctrl >= ATA_MAX_CTRLS) || (drive >= ATA_MAX_DRIVES) ||
        !ataDrvInstalled || !pCtrl->installed)
    return (ERROR);

    if ((pAtaRaw->cylinder  >= pType->cylinders)    ||
        (pAtaRaw->head      >= pType->heads)    ||
        (pAtaRaw->sector    >  pType->sectors)  ||
        (pAtaRaw->sector    == 0))
    return (ERROR);

    pBlkdev->bd_nBlocks     = pType->cylinders * pType->heads * 
                  pType->sectors;
    pBlkdev->bd_bytesPerBlk = pType->bytes;
    pBlkdev->bd_blksPerTrack    = pType->sectors;
    pBlkdev->bd_nHeads      = pType->heads;
    pBlkdev->bd_removable   = FALSE;
    pBlkdev->bd_retry       = 1;
    pBlkdev->bd_mode        = O_RDWR;
    pBlkdev->bd_readyChanged    = TRUE;
    pBlkdev->bd_blkRd       = ataBlkRd;
    pBlkdev->bd_blkWrt      = ataBlkWrt;
    pBlkdev->bd_ioctl       = ataIoctl;
    pBlkdev->bd_reset       = ataReset;
    pBlkdev->bd_statusChk   = ataStatus;

    ataDev.ctrl         = ctrl;
    ataDev.drive        = drive;
    ataDev.blkOffset        = 0;

    startBlk = pAtaRaw->cylinder * (pType->sectors * pType->heads) +
           pAtaRaw->head * pType->sectors + pAtaRaw->sector - 1;

    return (ataBlkRW (&ataDev, startBlk, pAtaRaw->nSecs, pAtaRaw->pBuf,
             pAtaRaw->direction));
}

/*******************************************************************************
*
* ataBlkRd - read one or more blocks from a ATA/IDE disk
*
* This routine reads one or more blocks from the specified device,
* starting with the specified block number.
*
* If any block offset was specified during ataDevCreate(), it is added
* to <startBlk> before the transfer takes place.
*
* RETURNS: OK, ERROR if the read command didn't succeed.
*/

LOCAL STATUS ataBlkRd
(
    ATA_DEV *pDev,
    int startBlk,
    int nBlks,
    char *pBuf
)
{
    return (ataBlkRW (pDev, startBlk, nBlks, pBuf, O_RDONLY));
}

/*******************************************************************************
*
* ataBlkWrt - write one or more blocks to a ATA/IDE disk
*
* This routine writes one or more blocks to the specified device,
* starting with the specified block number.
*
* If any block offset was specified during ataDevCreate(), it is added
* to <startBlk> before the transfer takes place.
*
* RETURNS: OK, ERROR if the write command didn't succeed.
*/

LOCAL STATUS ataBlkWrt
(
    ATA_DEV *pDev,
    int startBlk,
    int nBlks,
    char *pBuf
)
{
    return (ataBlkRW (pDev, startBlk, nBlks, pBuf, O_WRONLY));
}

/*******************************************************************************
*
* ataReset - reset a ATA/IDE disk controller
*
* This routine resets a ATA/IDE disk controller.
*
* RETURNS: OK, always.
*/

LOCAL STATUS ataReset
(
    ATA_DEV *pDev
)
{
    ATA_CTRL *pCtrl = &ataCtrl[pDev->ctrl];
    
    if (!pCtrl->installed)
        return (ERROR);

    semTake (&pCtrl->muteSem, WAIT_FOREVER);

    (void) ataInit (pDev->ctrl);

    semGive (&pCtrl->muteSem);

    return (OK);
}

/*******************************************************************************
*
* ataStatus - check status of a ATA/IDE disk controller
*
* This routine check status of a ATA/IDE disk controller.
*
* RETURNS: OK, ERROR if the card is removed.
*/

LOCAL STATUS ataStatus
(
    ATA_DEV *pDev
)
{
    ATA_CTRL *pCtrl = &ataCtrl[pDev->ctrl];
    BLK_DEV *pBlkdev    = &pDev->blkDev;
    
    if (!pCtrl->installed)
        return (ERROR);

    if (pCtrl->changed)
    {
        pBlkdev->bd_readyChanged = TRUE;
        pCtrl->changed       = FALSE;
    }

    return (OK);
}

/*******************************************************************************
*
* ataIoctl - do device specific control function
*
* This routine is called when the file system cannot handle an ioctl()
* function.
*
* RETURNS:  OK or ERROR.
*/

LOCAL STATUS ataIoctl
(
    ATA_DEV *pDev,
    int function,
    int arg
)
{
    ATA_CTRL *pCtrl = &ataCtrl[pDev->ctrl];
    FAST int status = ERROR;

    if (!pCtrl->installed)
        return (ERROR);

    semTake (&pCtrl->muteSem, WAIT_FOREVER);

    switch (function)
    {
    case FIODISKFORMAT:
        (void) errnoSet (S_ioLib_UNKNOWN_REQUEST);
        break;

    default:
        (void) errnoSet (S_ioLib_UNKNOWN_REQUEST);
    }

    semGive (&pCtrl->muteSem);
    return (status);
}

/*******************************************************************************
*
* ataBlkRW - read or write sectors to a ATA/IDE disk.
*
* Read or write sectors to a ATA/IDE disk.
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

LOCAL STATUS ataBlkRW
(
    ATA_DEV *pDev,
    int startBlk,
    int nBlks,
    char *pBuf,
    int direction
)
{
    ATA_CTRL *pCtrl = &ataCtrl[pDev->ctrl];
    ATA_DRIVE *pDrive   = &pCtrl->drive[pDev->drive];
    BLK_DEV *pBlkdev    = &pDev->blkDev;
    ATA_TYPE *pType = &ataTypes[pDev->ctrl][pDev->drive];
    int status      = ERROR;
    int retryRW0    = 0;
    int retryRW1    = 0;
    int retrySeek   = 0;
    int cylinder;
    int head;
    int sector;
    int nSecs;
    int ix;

    /* sanity check */

    if (!pCtrl->installed)
        return (ERROR);

    nSecs = pBlkdev->bd_nBlocks;
    if ((startBlk + nBlks) > nSecs)
    {
#ifdef  ATA_DEBUG
        printErr ("startBlk=%d nBlks=%d: 0 - %d\n", startBlk, nBlks, nSecs);
#endif  /* ATA_DEBUG */
        return (ERROR);
    }

    startBlk += pDev->blkOffset;

    semTake (&pCtrl->muteSem, WAIT_FOREVER);

    for (ix = 0; ix < nBlks; ix += nSecs)
    {
        if (pDrive->okLba)
        {
            head     = (startBlk & 0x0f000000) >> 24;
            cylinder = (startBlk & 0x00ffff00) >> 8;
            sector   = (startBlk & 0x000000ff);
        }
        else
        {
            cylinder = startBlk / (pType->sectors * pType->heads);
            sector   = startBlk % (pType->sectors * pType->heads);
            head     = sector / pType->sectors;
            sector   = sector % pType->sectors + 1;
        }
        nSecs    = min (nBlks - ix, ATA_MAX_RW_SECTORS);

        retryRW1 = 0;
        retryRW0 = 0;
        while ((ataRW(pDev->ctrl, pDev->drive, cylinder, head, sector, 
           pBuf, nSecs, direction) != OK) || (ataWVerify(pDev->ctrl, 
           pDev->drive, cylinder, head, sector, pBuf, nSecs, direction) != OK))
        {
            if (++retryRW0 > ataRetry)
            {
                ataNumErrs++;
                (void)ataCmd (pDev->ctrl, pDev->drive, ATA_CMD_RECALIB, NULL,
                    NULL);
                if (++retryRW1 > ataRetry)
                    goto done;
                retrySeek = 0;
                while (ataCmd (pDev->ctrl, pDev->drive, ATA_CMD_SEEK, cylinder,
                    head) != OK)
                    if (++retrySeek > ataRetry)
                        goto done;
                retryRW0 = 0;
            }
        }

        startBlk += nSecs;
        pBuf += pBlkdev->bd_bytesPerBlk * nSecs;
    }

    status = OK;

done:
    if (status == ERROR)
        (void)errnoSet (S_ioLib_DEVICE_ERROR);
    semGive (&pCtrl->muteSem);
    return (status);
}

/*******************************************************************************
*
* ataIntr - ATA/IDE controller interrupt handler.
*
* RETURNS: N/A
*/
#if 0
LOCAL void ataIntr
(
    int ctrl
)
{
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];

    pCtrl->intCount++;
    pCtrl->intStatus = ATA_IO_BYTE_READ (pCtrl->status);
    semGive (&pCtrl->syncSem);
}
#endif /* 0 */
/*******************************************************************************
*
* ataWdog - ATA/IDE controller watchdog handler.
*
* RETURNS: N/A
*/

LOCAL void ataWdog
(
    int ctrl
)
{
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];

    pCtrl->wdgOkay = FALSE;
}

/*******************************************************************************
*
* ataWait - wait the drive ready
*
* Wait the drive ready
*
* RETURNS: OK, ERROR if the drive didn't become ready in certain period of time.
*/

LOCAL void ataWait
(
    int ctrl,
    int request
)
{
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];

#ifdef  ATA_DEBUG
    printErr ("ataWait: ctrl=%d  request=0x%x\n", ctrl, request);
#endif  /* ATA_DEBUG */

    switch (request)
    {
    case ATA_STAT_READY:
        wdStart (pCtrl->wdgId, (sysClkRateGet() * pCtrl->wdgTimeout), 
             (FUNCPTR)ataWdog, ctrl);
        while ((ATA_IO_BYTE_READ (pCtrl->aStatus) & ATA_STAT_BUSY) && 
            (pCtrl->wdgOkay));
        while (((ATA_IO_BYTE_READ (pCtrl->aStatus) & ATA_STAT_READY) == 0)
               && (pCtrl->wdgOkay));
        wdCancel (pCtrl->wdgId);

        if (!pCtrl->wdgOkay)
        {
            pCtrl->wdgOkay = TRUE;
            (void) ataInit (ctrl);
        }
        break;

    case ATA_STAT_BUSY:
        while (ATA_IO_BYTE_READ (pCtrl->aStatus) & ATA_STAT_BUSY);
        break;

    case ATA_STAT_DRQ:
        while ((ATA_IO_BYTE_READ (pCtrl->aStatus) & ATA_STAT_DRQ) == 0);
        break;

    case ATA_STAT_SEEKCMPLT:
        while ((ATA_IO_BYTE_READ (pCtrl->aStatus) & ATA_STAT_SEEKCMPLT) == 0);
        break;
    }

#ifdef  ATA_DEBUG
    printErr ("ataWait end:\n");
#endif  /* ATA_DEBUG */
}

/*******************************************************************************
*
* ataInit - init a ATA/IDE disk controller
*
* This routine initializes a ATA/IDE disk controller.
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

LOCAL STATUS ataInit
(
    int ctrl
)
{
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];
    int ix;
    int iy;

#ifdef  ATA_DEBUG
    printErr ("ataInit: ctrl=%d\n", ctrl);
#endif  /* ATA_DEBUG */

    ATA_IO_BYTE_WRITE (pCtrl->dControl,
                       ATA_CTL_4BIT | ATA_CTL_RST | ATA_CTL_IDS);
    for (ix = 0; ix < 100; ix++)
      /*  sysDelay ();
      */
         for (iy = 0; iy < 50000; iy++);

    ATA_IO_BYTE_WRITE (pCtrl->dControl, ATA_CTL_4BIT | ATA_CTL_IDS);
    for (ix = 0; ix < 100; ix++)
      /*  sysDelay ();
      */
         for (iy = 0; iy < 50000; iy++);
 
    pCtrl->wdgOkay = TRUE;

    /* start the ata  watchdog */

    wdStart (pCtrl->wdgId, (sysClkRateGet() * pCtrl->wdgTimeout),
         (FUNCPTR)ataWdog, ctrl);

    while ((ATA_IO_BYTE_READ (pCtrl->aStatus) & ATA_STAT_BUSY) &&
           (pCtrl->wdgOkay))
    ;

    ATA_IO_BYTE_WRITE (pCtrl->dControl, ATA_CTL_4BIT);
    for (ix = 0; ix < 100; ix++) 
     /*   sysDelay ();
     */
        for (iy = 0; iy < 50000; iy++);

    while (((ATA_IO_BYTE_READ (pCtrl->aStatus) & ATA_STAT_READY) == 0) &&
       (pCtrl->wdgOkay))
    ;

    wdCancel (pCtrl->wdgId);

    if (!pCtrl->wdgOkay)
    {
#ifdef  ATA_DEBUG
        printErr ("ataInit error:\n");
#endif  /* ATA_DEBUG */
        pCtrl->wdgOkay = TRUE;
        return (ERROR);
    }

#ifdef  ATA_DEBUG
    printErr ("ataInit end:\n");
#endif  /* ATA_DEBUG */

    return (OK);
}

/*******************************************************************************
*
* ataCmd - issue non data command
*
* Issue a non data command.  Non data commands have the same protocol.
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

LOCAL STATUS ataCmd
(
    int ctrl,
    int drive,
    int cmd,
    int arg0,
    int arg1
)
{
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];
    ATA_TYPE *pType = &ataTypes[ctrl][drive];
    BOOL retry      = TRUE;
    int retryCount  = 0;
    int semStatus       = OK;

#ifdef  ATA_DEBUG
    printErr ("ataCmd: ctrl=%d drive=%d cmd=0x%x\n", ctrl, drive, cmd);
#endif  /* ATA_DEBUG */

    while (retry)
    {
        ataWait (ctrl, ATA_STAT_READY);

        switch (cmd)
        {
            case ATA_CMD_DIAGNOSE:
                ATA_IO_BYTE_WRITE (pCtrl->sdh, ATA_SDH_IBM | (drive << 4));
            break;
            case ATA_CMD_INITP:
                ATA_IO_BYTE_WRITE (pCtrl->cylLo, pType->cylinders);
                ATA_IO_BYTE_WRITE (pCtrl->cylHi, pType->cylinders >> 8);
                ATA_IO_BYTE_WRITE (pCtrl->seccnt, pType->sectors);
                ATA_IO_BYTE_WRITE (pCtrl->sdh, ATA_SDH_IBM | (drive << 4) | 
                    ((pType->heads - 1) & 0x0f));
            break;
            case ATA_CMD_RECALIB:
            ATA_IO_BYTE_WRITE (pCtrl->sdh, ATA_SDH_IBM | (drive << 4));
            break;
            case ATA_CMD_SEEK:
                ATA_IO_BYTE_WRITE (pCtrl->cylLo, arg0);
                ATA_IO_BYTE_WRITE (pCtrl->cylHi, arg0>>8);
                ATA_IO_BYTE_WRITE (pCtrl->sdh, ATA_SDH_IBM | (drive << 4) |
                     (arg1 & 0xf));
            break;
            case ATA_CMD_SET_FEATURE:
                ATA_IO_BYTE_WRITE (pCtrl->seccnt, arg1);
                ATA_IO_BYTE_WRITE (pCtrl->feature, arg0);
                ATA_IO_BYTE_WRITE (pCtrl->sdh, ATA_SDH_IBM | (drive << 4));
            break;
            case ATA_CMD_SET_MULTI:
                ATA_IO_BYTE_WRITE (pCtrl->seccnt, arg0);
                ATA_IO_BYTE_WRITE (pCtrl->sdh, ATA_SDH_IBM | (drive << 4));
            break;
        }

        ATA_IO_BYTE_WRITE (pCtrl->command, cmd);
/*        semStatus = semTake (&pCtrl->syncSem, 
                 sysClkRateGet() * pCtrl->semTimeout);
*/
        ataWait (ctrl, ATA_STAT_BUSY);
        pCtrl->intStatus = ATA_IO_BYTE_READ (pCtrl->status);


        if ((pCtrl->intStatus & ATA_STAT_ERR) || (semStatus == ERROR))
        {
#ifdef  ATA_DEBUG
            int error = ATA_IO_BYTE_READ(pCtrl->error);
            printErr ("ataCmd err: status=0x%x semStatus=%d err=0x%x\n", 
                pCtrl->intStatus, semStatus, error);
#endif  /* ATA_DEBUG */
            if (++retryCount > ataRetry)
                return (ERROR);
        }
        else
            retry = FALSE;
    }

    switch (cmd)
    {
        case ATA_CMD_SEEK:
            ataWait (ctrl, ATA_STAT_SEEKCMPLT);
        break;
    }

#ifdef  ATA_DEBUG
    printErr ("ataCmd end:\n");
#endif  /* ATA_DEBUG */

    return (OK);
}

/*******************************************************************************
*
* ataPread - Read drive parameters
*
* Read drive parameters.
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

LOCAL STATUS ataPread
(
    int ctrl,
    int drive,
    void *buffer
)
{
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];
    BOOL retry      = TRUE;
    int retryCount  = 0;
    int semStatus       = OK;

#ifdef  ATA_DEBUG
    printErr ("ataPread: ctrl=%d drive=%d\n", ctrl, drive);
#endif  /* ATA_DEBUG */

    while (retry)
    {
        ataWait (ctrl, ATA_STAT_READY);

        ATA_IO_BYTE_WRITE (pCtrl->sdh, ATA_SDH_IBM | (drive << 4));
        ATA_IO_BYTE_WRITE (pCtrl->command, ATA_CMD_READP);
/*        semStatus = semTake (&pCtrl->syncSem, 
                 sysClkRateGet() * pCtrl->semTimeout);
*/
        ataWait (ctrl, ATA_STAT_BUSY);
        pCtrl->intStatus = ATA_IO_BYTE_READ (pCtrl->status);


        if ((pCtrl->intStatus & ATA_STAT_ERR) || (semStatus == ERROR))
        {
#ifdef  ATA_DEBUG
            int error   = ATA_IO_BYTE_READ (pCtrl->error);
            int status  = ATA_IO_BYTE_READ (pCtrl->aStatus);
            printErr ("ataPread err: stat=0x%x 0x%x semStatus=%d err=0x%x\n",
                  pCtrl->intStatus, status, semStatus, error);
#endif  /* ATA_DEBUG */
            if (++retryCount > ataRetry)
                return (ERROR);
        }
        else
            retry = FALSE;
    }

    ataWait (ctrl, ATA_STAT_DRQ);

    ATA_IO_NWORD_READ_SWAP (pCtrl->data, (short *)buffer, 256);

#ifdef  ATA_DEBUG
    printErr ("ataPread end:\n");
#endif  /* ATA_DEBUG */

    return (OK);
}

/*******************************************************************************
*
* ataWVerify - read back data and verify
*
* RETURNS: OK, ERROR if the data does not match.
*/

LOCAL STATUS ataWVerify
(
    int ctrl,
    int drive,
    int cylinder,
    int head,
    int sector,
    void *buffer,
    int nSecs,
    int direction
)
{
    STATUS stat = OK;
    ATA_TYPE *pType = &ataTypes[ctrl][drive];
    int curByte;

    if (direction == O_WRONLY)
    {
        if ((stat = ataRW(ctrl, drive, cylinder, head, sector, 
           ataWriteVerifyBuf, nSecs, O_RDONLY)) == OK)
        {
            for (curByte=0; curByte < nSecs * pType->bytes; curByte++)
            {
                if (ataWriteVerifyBuf[curByte] != ((char *)buffer)[curByte])
                {
                    stat = ERROR;
                    ataWriteVerifyNumErrs++;
                    break;
                }
            }
        }
    }

    return (stat);
}

/*******************************************************************************
*
* ataRW - read/write a number of sectors on the current track
*
* Read/write a number of sectors on the current track
*
* RETURNS: OK, ERROR if the command didn't succeed.
*/

LOCAL STATUS ataRW
(
    int ctrl,
    int drive,
    int cylinder,
    int head,
    int sector,
    void *buffer,
    int nSecs,
    int direction
)
{
    ATA_CTRL *pCtrl = &ataCtrl[ctrl];
    ATA_DRIVE *pDrive   = &pCtrl->drive[drive];
    ATA_TYPE *pType = &ataTypes[ctrl][drive];
    int retryCount  = 0;
    int block       = 1;
    int nSectors    = nSecs;
    int nWords;
    int semStatus       = OK;
    short *pBuf;

#ifdef  ATA_DEBUG
    printErr ("ataRW: ctrl=%d drive=%d c=%d h=%d s=%d buf=0x%x n=%d dir=%d\n",
              ctrl, drive, cylinder, head, sector, 
          (int)buffer, nSecs, direction);
#endif  /* ATA_DEBUG */

retryRW:
    ataWait (ctrl, ATA_STAT_READY);

    pBuf = (short *)buffer;
    ATA_IO_BYTE_WRITE (pCtrl->feature, pType->precomp);
    ATA_IO_BYTE_WRITE (pCtrl->seccnt, nSecs);
    ATA_IO_BYTE_WRITE (pCtrl->sector, sector);
    ATA_IO_BYTE_WRITE (pCtrl->cylLo, cylinder);
    ATA_IO_BYTE_WRITE (pCtrl->cylHi, cylinder>>8);
    if (pDrive->okLba)
        ATA_IO_BYTE_WRITE (pCtrl->sdh,
                           ATA_SDH_LBA | (drive << 4) | (head & 0xf));
    else
        ATA_IO_BYTE_WRITE (pCtrl->sdh,
                           ATA_SDH_IBM | (drive << 4) | (head & 0xf));

    if (pDrive->rwPio == ATA_PIO_MULTI)
        block = pDrive->multiSecs;

    nWords = (pType->bytes * block) >> 1;

    if (direction == O_WRONLY)
    {
        if (pDrive->rwPio == ATA_PIO_MULTI)
            ATA_IO_BYTE_WRITE (pCtrl->command, ATA_CMD_WRITE_MULTI);
        else  
            ATA_IO_BYTE_WRITE (pCtrl->command, ATA_CMD_WRITE);

        while (nSectors > 0)
        {
            if ((pDrive->rwPio == ATA_PIO_MULTI) && (nSectors < block))
            {
                block = nSectors;
                nWords = (pType->bytes * block) >> 1;
            }

            ataWait (ctrl, ATA_STAT_BUSY);
            ataWait (ctrl, ATA_STAT_DRQ);
/*
            if (pDrive->rwBits == ATA_BITS_16)
                ATA_IO_NWORD_WRITE (pCtrl->data, pBuf, nWords);
            else
                ATA_IO_NLONG_WRITE (pCtrl->data, (long *)pBuf, nWords >> 1);
*/
            ATA_IO_NWORD_WRITE (pCtrl->data, pBuf, nWords);

/*            semStatus = semTake (&pCtrl->syncSem, 
                 sysClkRateGet() * pCtrl->semTimeout);
*/
            ataWait (ctrl, ATA_STAT_BUSY);
            pCtrl->intStatus = ATA_IO_BYTE_READ (pCtrl->status);

            if ((pCtrl->intStatus & ATA_STAT_ERR) || (semStatus == ERROR))
                goto errorRW;

            pBuf     += nWords;
            nSectors -= block;
        }
    }
    else
    {
        if (pDrive->rwPio == ATA_PIO_MULTI)
            ATA_IO_BYTE_WRITE (pCtrl->command, ATA_CMD_READ_MULTI);
        else 
            ATA_IO_BYTE_WRITE (pCtrl->command, ATA_CMD_READ);

        while (nSectors > 0)
        {
            if ((pDrive->rwPio == ATA_PIO_MULTI) && (nSectors < block))
            {
                block = nSectors;
                nWords = (pType->bytes * block) >> 1;
            }

/*            semStatus = semTake (&pCtrl->syncSem, 
                 sysClkRateGet() * pCtrl->semTimeout);
*/
            ataWait (ctrl, ATA_STAT_BUSY);
            pCtrl->intStatus = ATA_IO_BYTE_READ (pCtrl->status);

            if ((pCtrl->intStatus & ATA_STAT_ERR) || (semStatus == ERROR))
                goto errorRW;

            ataWait (ctrl, ATA_STAT_DRQ);
/*
            if (pDrive->rwBits == ATA_BITS_16)
                ATA_IO_NWORD_READ (pCtrl->data, pBuf, nWords);
            else
                ATA_IO_NLONG_READ (pCtrl->data, (long *)pBuf, nWords >> 1);
*/
            ATA_IO_NWORD_READ (pCtrl->data, pBuf, nWords);

            pBuf     += nWords;
            nSectors -= block;
        }
    }

#ifdef  ATA_DEBUG
    printErr ("ataRW: end\n");
#endif  /* ATA_DEBUG */

    return (OK);

errorRW:
#ifdef  ATA_DEBUG
    {
        int error   = ATA_IO_BYTE_READ (pCtrl->error);
        int status  = ATA_IO_BYTE_READ (pCtrl->aStatus);
        printErr ("ataRW err: stat=0x%x 0x%x semStatus=%d error=0x%x\n",
          pCtrl->intStatus, status, semStatus, error);
    }
#endif  /* ATA_DEBUG */
    ataCFStatErrs++;
    if (++retryCount < ataRetry)
        goto retryRW;
    return (ERROR);
}
