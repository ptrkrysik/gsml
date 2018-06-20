/*
JetCell modification history
----------------------------
99/09/09  ck   Include ataDrv.c to include ata driver
               Do not include sysHpi.c
               Add compactFlash in sysPhysMemDesc
               Add constants for I/O memory addresse
               Add sysMemMapInit
99/06/01  ck   Add i2c and nvram support. Change sysFecEnetAddrGet.
99/03/22  ck   Add JetCell modification history
99/03/16  ck   Changed ethernet adress to JetCell specific. 
               Changed sysModel to JetCell CDC. 
99/03/15  ck   Add HPI support. Modify sysPhysMemDesc
99/03/15  ck   Fix conditional compile
99/02/18  ck   Changes for CDCIM
99/02/18  ck   Initial revision. Copied from ads860 bsp.
*/

/* sysLib.c - Motorola 860ads board system-dependent library */

/* Copyright 1984-1997 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/*
modification history
--------------------
01k,12nov98,cn   changed sysFecEnetEnable(), sysFecEnetDisable() to
                 include enable/disable of the Ethernet device.
01j,09nov98,cn   added support for FADS860T boards and FEC.
01i,08may98,gls  added clear of all cpm interrups in sysToMonitor
01h,08mar98,gls	 added checks to allow BSP to work with the PPC850FADS
01g,18feb98,gls  added code to allow the BSP to work with the PPC823FADS
01f,12nov97,map  updated to use ppc860Timer.c [SPR# 9366]
01e,21feb97,mas  removed NOMANUAL, made sysHwInit2() visible to 'man' (SPR 7879)
01d,11nov96,tam  added power management mode initialization.
01c,06nov96,tpr  reworked sysHwInit().
01b,28may96,dzb  added Ethernet driver support routines.
		 added port initialization, and increased system to 24Mhz.
01a,19apr96,tpr  written.
*/

/*
DESCRIPTION
This library provides board-specific routines.  The chip drivers included are:

    ppc860Timer.c	- PowerPC/860 Timer library

INCLUDE FILES: sysLib.h

SEE ALSO:
.pG "Configuration"
*/

/* includes */

#include "vxWorks.h"
#include "vme.h"
#include "memLib.h"
#include "cacheLib.h"
#include "sysLib.h"
#include "config.h"
#include "string.h"
#include "intLib.h"
#include "logLib.h"
#include "stdio.h"
#include "taskLib.h"
#include "vxLib.h"
#include "tyLib.h"
#include "arch/ppc/vxPpcLib.h"
#include "private/vmLibP.h"

#include "drv/multi/ppc860Siu.h"
/* CK */
/* #include "ads860.h" */
#include  "cdcim.h"

#ifdef	INCLUDE_CPM
#include "drv/netif/if_cpm.h"
#endif

#include "ds2401.c"
#include "i2c.c"
#include "bootFlash.c"
#ifdef INCLUDE_ATA
#include "ataDrv.c"
#endif

PHYS_MEM_DESC sysPhysMemDesc [] =
{
    {
    (void *) LOCAL_MEM_LOCAL_ADRS,
    (void *) LOCAL_MEM_LOCAL_ADRS,
    LOCAL_MEM_SIZE ,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE
    },

#ifndef CDCIM
    {
    (void *) BCSR0,
    (void *) BCSR0,
    0x00001000,				/* 4 k - Board Control and Status */	
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    },
#endif

    {
    (void *) INTERNAL_MEM_MAP_ADDR,
    (void *) INTERNAL_MEM_MAP_ADDR,
    INTERNAL_MEM_MAP_SIZE,		/* 64 k - Internal Memory Map */	
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    },

    {
    (void *) ROM_BASE_ADRS,
    (void *) ROM_BASE_ADRS,
    ROM_SIZE,				/* Flach memory */	
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    },

    {
    (void *) DSP0_HPI_BASE_ADDR,
    (void *) DSP0_HPI_BASE_ADDR,
    0x00001000,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    },

    {
    (void *) DSP1_HPI_BASE_ADDR,
    (void *) DSP1_HPI_BASE_ADDR,
    0x00001000,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    },

    {
    (void *) AUX_PORT_ADDR,
    (void *) AUX_PORT_ADDR,
    0x00001000,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    },

    {
    (void *) REV_DATA_ADDR,
    (void *) REV_DATA_ADDR,
    0x00001000,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    },

    {
    (void *) CF_BASE_ADDR,
    (void *) CF_BASE_ADDR,
    0x01000000,
    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
    VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_NOT
    }
};

int sysPhysMemDescNumEnt = NELEMENTS (sysPhysMemDesc);

int   sysBus      = BUS;                /* system bus type (VME_BUS, etc) */
int   sysCpu      = CPU;                /* system CPU type (PPC860) */
char *sysBootLine = BOOT_LINE_ADRS;	/* address of boot line */
char *sysExcMsg   = EXC_MSG_ADRS;	/* catastrophic message area */
int   sysProcNum;			/* processor number of this CPU */
int   sysFlags;				/* boot flags */
char  sysBootHost [BOOT_FIELD_LEN];	/* name of host from which we booted */
char  sysBootFile [BOOT_FIELD_LEN];	/* name of file from which we booted */
BOOL  sysVmeEnable = FALSE;		/* by default no VME */

const int sysCfBaseAddr = CF_BASE_ADDR;            /* Compact flash base Addr */
const int sysDsp0HpiBaseAddr = DSP0_HPI_BASE_ADDR; /* DSP0 HPI base Address */
const int sysDsp1HpiBaseAddr = DSP1_HPI_BASE_ADDR; /* DSP1 HPI base address */
const int sysAuxPortAddr = AUX_PORT_ADDR;          /* Aux. Port address */ 




#ifdef	INCLUDE_CPM
/* XXX set the following array to a unique Ethernet hardware address XXX */

/* last 5 nibbles are board specific, initialized in sysHwInit */

unsigned char sysCpmEnetAddr [6] = {0x08, 0x00, 0x3e, 0x03, 0x02, 0x01};

extern STATUS cpmattach();

#endif	/* INCLUDE_CPM */

#ifdef INCLUDE_ATA

/* global data structures used by the EIDE/ATA driver */

ATA_RESOURCE    ataResources[ATA_MAX_CTRLS];

/* 
 * This structure needs to be initialized for each EIDE/ATA device present
 * in the system.  It has the following layout:
 *	int cylinders;
 *	int heads;
 *	int sectorsTrack;
 *	int bytesSector;
 *	int precomp;
 */

ATA_TYPE        ataTypes [ATA_MAX_CTRLS][ATA_MAX_DRIVES] =
    {
        {
    	    {/* ATA_DEV0_CYL*/ 1, 0, 0, 512, 0xff},  /* controller 0, drive 0 */
    	    {/* ATA_DEV1_CYL*/ 0, 0, 0, 512, 0xff},  /* controller 0, drive 1 */
    	},
        {
    	    {/* ATA_DEV2_CYL*/ 0, 0, 0, 512, 0xff},  /* controller 1, drive 0 */
    	    {/* ATA_DEV3_CYL*/ 0, 0, 0, 512, 0xff},  /* controller 1, drive 1 */
    	}
    };
#endif /* INCLUDE_ATA */


/* locals */

void sysCpmEnetDisable (int unit);
void sysCpmEnetIntDisable (int unit);
LOCAL void sysAtaInit();
LOCAL void sysMemMapInit();

#ifdef INCLUDE_MOT_FEC

/* set the following array to a unique Ethernet hardware address */
/* set to JetCell specific address */

unsigned char sysFecEnetAddr [6] = {0x00, 0xd0, 0x2b, 0x00, 0x00, 0x00};


IMPORT STATUS sysFecEnetDisable (UINT32 motCpmaddr);

#endif /* INCLUDE_MOT_FEC */


#include "sysSerial.c"
#include "intrCtl/ppc860Intr.c"
#include "nvRam.c"
#include "timer/ppc860Timer.c"		/* PPC860 & 821 have on chip timers */

/******************************************************************************
*
* sysModel - return the model name of the CPU board
*
* This routine returns the model name of the CPU board.
*
* RETURNS: A pointer to the string.
*/

char * sysModel (void)
    {
    return ("JetCell CDC - PowerPC 860T");
    }

/******************************************************************************
*
* sysBspRev - return the bsp version with the revision eg 1.0/<x>
*
* This function returns a pointer to a bsp version with the revision.
* for eg. 1.0/<x>. BSP_REV defined in config.h is concatanated to
* BSP_VERSION and returned.
*
* RETURNS: A pointer to the BSP version/revision string.
*/

char * sysBspRev (void)
    {
    return (BSP_VERSION BSP_REV);
    }

/******************************************************************************
*
* sysHwInit - initialize the system hardware
*
* This routine initializes various feature of the MPC860ADS boards. It sets up
* the control registers, initializes various devices if they are present.
*
* NOTE: This routine should not be called directly by the user.
*
* RETURNS: N/A
*/

void sysHwInit (void)
    {
    int	immrVal = vxImmrGet();

    sysMemMapInit();

    sysAtaInit();

  /*  initHpi(); */

    /* set the SPLL to the value requested */

    * PLPRCR(immrVal) = (*PLPRCR(immrVal) & ~PLPRCR_MF_MSK) | 
				(SPLL_MUL_FACTOR << PLPRCR_MF_SHIFT);

    /* set the BRGCLK division factor */

    * SCCR(immrVal) = (* SCCR(immrVal) & ~SCCR_DFBRG_MSK) |
				(BRGCLK_DIV_FACTOR << SCCR_DFBRG_SHIFT);
    /* set the Periodic Timer A value */
     
    * MAMR(immrVal) = (* MAMR(immrVal) & ~MAMR_PTA_MSK) | 
				(PTA_VALUE << MAMR_PTA_SHIFT);	

    /* set the Periodic Timer PreScale */

    * MPTPR(immrVal) = PTP_VALUE;

    /* reset the port A */

    *PAPAR(immrVal) = 0x0000;
    *PADIR(immrVal) = 0x0000;
    *PAODR(immrVal) = 0x0000;

    /* reset the port B */

    *PBPAR(immrVal) = 0x0000000;
    *PBDIR(immrVal) = 0x0000000;
    *PBODR(immrVal) = 0x0000000;

    /* reset the port C */

    *PCPAR(immrVal) = 0x0000;
    *PCDIR(immrVal) = 0x0000;
    *PCSO(immrVal)  = 0x0000;

    /* reset the port D */

    *PDPAR(immrVal) = 0x0000;
    *PDDIR(immrVal) = 0x0000;

    *SICR(immrVal) = 0x0;		/* initialize SI/NMSI connections */

    /* Initialize interrupts */

    ppc860IntrInit(IV_LEVEL4);	/* default vector level */

    /* Reset serial channels */

    sysSerialHwInit();

    /* make sure Ethernet is disabled */

    sysCpmEnetDisable (0);
    sysCpmEnetIntDisable (0);

#ifdef INCLUDE_MOT_FEC

    /* make sure the FEC is disabled */

    sysFecEnetDisable (immrVal);

#endif /* INCLUDE_MOT_FEC */

    /*
     * The power management mode is initialized here. Reduced power mode
     * is activated only when the kernel is iddle (cf vxPowerDown).
     * Power management mode is selected via vxPowerModeSet().
     * DEFAULT_POWER_MGT_MODE is defined in config.h.
     */

    vxPowerModeSet (DEFAULT_POWER_MGT_MODE);
    }

/*******************************************************************************
*
* sysPhysMemTop - get the address of the top of physical memory
*
* This routine returns the address of the first missing byte of memory,
* which indicates the top of memory.
*
* RETURNS: The address of the top of physical memory.
*
* SEE ALSO: sysMemTop()
*/

char * sysPhysMemTop (void)
    {
    static char * physTop = NULL;

    if (physTop == NULL)
	{
	physTop = (char *)(LOCAL_MEM_LOCAL_ADRS + LOCAL_MEM_SIZE);
	}

    return (physTop) ;
    }

/*******************************************************************************
*
* sysMemTop - get the address of the top of VxWorks memory
*
* This routine returns a pointer to the first byte of memory not
* controlled or used by VxWorks.
*
* The user can reserve memory space by defining the macro USER_RESERVED_MEM
* in config.h.  This routine returns the address of the reserved memory
* area.  The value of USER_RESERVED_MEM is in bytes.
*
* RETURNS: The address of the top of VxWorks memory.
*/

char * sysMemTop (void)
    {
    static char * memTop = NULL;

    if (memTop == NULL)
	{
	memTop = sysPhysMemTop () - USER_RESERVED_MEM;
	}

    return memTop;
    }


/******************************************************************************
*
* sysToMonitor - transfer control to the ROM monitor
*
* This routine transfers control to the ROM monitor.  Normally, it is called
* only by reboot()--which services ^X--and bus errors at interrupt level.
* However, in some circumstances, the user may wish to introduce a
* <startType> to enable special boot ROM facilities.
*
* RETURNS: Does not return.
*/

STATUS sysToMonitor
    (
     int startType	/* parameter passed to ROM to tell it how to boot */
    )
    {
    FUNCPTR pRom = (FUNCPTR) (ROM_TEXT_ADRS + 4);	/* Warm reboot */

    *CIMR(vxImmrGet()) = 0;     /* disable all cpm interupts */
    
    sysCpmEnetDisable (0);	/* disable the ethernet device */
 
#ifdef INCLUDE_MOT_FEC

    /* disable the FEC */

    sysFecEnetDisable (vxImmrGet ());

#endif /* INCLUDE_MOT_FEC */

    sysSerialReset();		/* reset the serail device */

    (*pRom) (startType);	/* jump to bootrom entry point */

    return (OK);	/* in case we ever continue from ROM monitor */
    }

/******************************************************************************
*
* sysHwInit2 - additional system configuration and initialization
*
* This routine connects system interrupts and does any additional
* configuration necessary.
*
* RETURNS: N/A
*/

void sysHwInit2 (void)
    {
    static BOOL configured = FALSE;
    int		immrVal;

    if (!configured)
	{

	immrVal = vxImmrGet();

	/* initialize serial interrupts */

	sysSerialHwInit2();

	* SCCR(immrVal) &= ~SCCR_TBS;

	/* un-freeze the Time Base clock */

	* TBSCR(immrVal) = TBSCR_TBE ;

        I2CvxInit();
        readDs2401();

	configured = TRUE;
	}
    }

/******************************************************************************
*
* sysProcNumGet - get the processor number
*
* This routine returns the processor number for the CPU board, which is
* set with sysProcNumSet().
* 
* RETURNS: The processor number for the CPU board.
*
* SEE ALSO: sysProcNumSet()
*/

int sysProcNumGet (void)
    {
    return (sysProcNum);
    }

/******************************************************************************
*
* sysProcNumSet - set the processor number
*
* This routine sets the processor number for the CPU board.  Processor numbers
* should be unique on a single backplane.
*
* Not applicable for the busless 860Ads.
*
* RETURNS: N/A
*
* SEE ALSO: sysProcNumGet()
*
*/

void sysProcNumSet
    (
    int 	procNum			/* processor number */
    )
    {
    sysProcNum = procNum;
    }

/******************************************************************************
*
* sysLocalToBusAdrs - convert a local address to a bus address
*
* This routine gets the VMEbus address that accesses a specified local
* memory address.
*
* Not applicable for the 860Ads
*
* RETURNS: ERROR, always.
*
* SEE ALSO: sysBusToLocalAdrs()
*/
 
STATUS sysLocalToBusAdrs
    (
    int 	adrsSpace,	/* bus address space where busAdrs resides */
    char *	localAdrs,	/* local address to convert */ 
    char **	pBusAdrs	/* where to return bus address */ 
    )
    {
    return (ERROR);
    }

/******************************************************************************
*
* sysBusToLocalAdrs - convert a bus address to a local address
*
* This routine gets the local address that accesses a specified VMEbus
* physical memory address.
*
* Not applicable for the 860Ads
*
* RETURNS: ERROR, always.
*
* SEE ALSO: sysLocalToBusAdrs()
*/

STATUS sysBusToLocalAdrs
    (
     int  	adrsSpace, 	/* bus address space where busAdrs resides */
     char *	busAdrs,   	/* bus address to convert */
     char **	pLocalAdrs 	/* where to return local address */
    )
    {
    return (ERROR);
    }

/******************************************************************************
*
* sysBusTas - test and set a location across the bus
*
* This routine does an atomic test-and-set operation across the backplane.
*
* Not applicable for the 860Ads.
*
* RETURNS: FALSE, always.
*
* SEE ALSO: vxTas()
*/

BOOL sysBusTas
    (
     char *	adrs		/* address to be tested-and-set */
    )
    {
    return (FALSE);
    }

/******************************************************************************
*
* sysBusClearTas - test and clear 
*
* This routine is a null function.
*
* RETURNS: N/A
*/

void sysBusClearTas
    (
     volatile char * address	/* address to be tested-and-cleared */
    )
    {
    } 

/*******************************************************************************
*
* sysCpmEnetDisable - disable the Ethernet controller
*
* This routine is expected to perform any target specific functions required
* to disable the Ethernet controller.  This usually involves disabling the
* Transmit Enable (TENA) signal.
*
* RETURNS: N/A
*/

void sysCpmEnetDisable
    (
    int         unit    /* not used - only slave SCC1 is wired to port */
    )
    {
#ifndef CDCIM
    *BCSR1 |= BCSR1_ETHN_EN_L;		/* disable Ethernet device */

    /* If running an 823, use SCC2 */
    if (((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_823DB_MASK) ||
	((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_850DB_MASK))
	{
	*PBPAR(vxImmrGet()) &= ~(0x2000);    /* set port B -> RTS2 = *TENA */
	}

    else
	*PBPAR(vxImmrGet()) &= ~(0x1000);    /* set port B -> RTS1 = *TENA */
#endif /* CDCIM */   
}

/*******************************************************************************
*
* sysCpmEnetIntDisable - disable the Ethernet interface interrupt
*
* This routine disable the interrupt for the Ethernet interface specified
* by <unit>.
*
* RETURNS: N/A.
*/

void sysCpmEnetIntDisable
    (
    int		unit
    )
    {
#ifndef CDCIM
    /* If running an 823, use SCC2 */
    if (((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_823DB_MASK) ||
	((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_850DB_MASK))
	{
	*CIMR(vxImmrGet()) &= ~CIMR_SCC2;
	}
    else
	*CIMR(vxImmrGet()) &= ~CIMR_SCC1;
#endif /* CDCIM */
    }
#ifdef	INCLUDE_CPM
/*******************************************************************************
*
* sysCpmEnetEnable - enable the Ethernet controller
*
* This routine is expected to perform any target specific functions required
* to enable the Ethernet controller.  These functions typically include
* enabling the Transmit Enable signal (TENA) and connecting the transmit
* and receive clocks to the SCC.
*
* RETURNS: OK, or ERROR if the Ethernet controller cannot be enabled.
*/

STATUS sysCpmEnetEnable
    (
    int		unit    /* not used - only slave SCC1 is wired to port */
    )
    {
    int immrVal = vxImmrGet();

    /* If running an 823, use SCC2 */ 
    if (((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_823DB_MASK) ||
	((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_850DB_MASK))
	{
	*PAPAR(immrVal) |= 0x030c;
	*PADIR(immrVal) &= ~(0x030c);

	*PCPAR(immrVal) &= ~(0x00c0);
	*PCDIR(immrVal) &= ~(0x00c0);
	*PCSO(immrVal)  |= 0x00c0;

	*BCSR4 &= ~(BCSR4_ETHLOOP | BCSR4_TFPLDL_L | BCSR4_TPSQEL_L | 
		    BCSR4_MODEM_EN_L | BCSR4_DATA_VOICE_L);

	*SICR(immrVal)  |= 0x2c00;	/* connect SCC2 clocks */
	*SICR(immrVal)  &= ~(0x4000);	/* NMSI mode */

	*BCSR1 &= ~(BCSR1_ETHN_EN_L);	

	*PBPAR(immrVal) |= 0x00002000;  /* set port B -> *RTS2 = TENA */
	*PBDIR(immrVal) |= 0x00002000; 
	}

    else
	{
	*PAPAR(immrVal) |= 0x0303;
	*PADIR(immrVal) &= ~(0x0303);

	*PCPAR(immrVal) &= ~(0x0030);
	*PCDIR(immrVal) &= ~(0x0030);
	*PCSO(immrVal)  |= 0x0030;

	*PCDIR(immrVal) |= 0x0f00;
	*PCDAT(immrVal) |= 0x0600;
	*PCDAT(immrVal) &= ~(0x0900);

	*SICR(immrVal)  |= 0x2c;            /* connect SCC1 clocks */
	*SICR(immrVal)  &= ~(0x40);         /* NMSI mode */

	*BCSR1 &= ~(BCSR1_ETHN_EN_L);       /* enable Ethernet */

	*PBPAR(immrVal) |= 0x00001000;      /* set port B -> *RTS1 = TENA */
	*PBDIR(immrVal) |= 0x00001000;
	}


    return (OK);
    }

/*******************************************************************************
*
* sysCpmEnetAddrGet - get the hardware Ethernet address
*
* This routine provides the six byte Ethernet hardware address that will be
* used by each individual Ethernet device unit.  This routine must copy
* the six byte address to the space provided by <addr>.
*
* RETURNS: OK, or ERROR if the Ethernet address cannot be returned.
*/

STATUS sysCpmEnetAddrGet
    (
    int		unit,   /* not used - only slave SCC1 is wired to port */
    UINT8 *	addr
    )
    {
    bcopy ((char *) sysCpmEnetAddr, (char *) addr, sizeof (sysCpmEnetAddr));

    return (OK);
    }

/*******************************************************************************
*
* sysCpmEnetCommand - issue a command to the Ethernet interface controller
*
* RETURNS: OK, or ERROR if the Ethernet controller could not be restarted.
*/

STATUS sysCpmEnetCommand
    (
    int		unit,
    UINT16	command
    )
    {
    int	immrVal = vxImmrGet();

    /* If running an 823, use SCC2 */
    if (((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_823DB_MASK) ||
	((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_850DB_MASK))
	{
	while (*CPCR(immrVal) & CPM_CR_FLG); 
	*CPCR(immrVal) = CPM_CR_CHANNEL_SCC2 | command | CPM_CR_FLG;
	while (*CPCR(immrVal) & CPM_CR_FLG);
	}

    else
	{
	while (*CPCR(immrVal) & CPM_CR_FLG);
	*CPCR(immrVal) = CPM_CR_CHANNEL_SCC1 | command | CPM_CR_FLG;
	while (*CPCR(immrVal) & CPM_CR_FLG);
	}

    return (OK);
    }

/*******************************************************************************
*
* sysCpmEnetIntEnable - enable the Ethernet interface interrupt
*
* This routine enable the interrupt for the Ethernet interface specified
* by <unit>.
*
* RETURNS: N/A.
*/

void sysCpmEnetIntEnable
    (
    int		unit
    )
    {

    /* If running an 823, use SCC2 */
    if (((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_823DB_MASK) ||
	((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_850DB_MASK))
	{
	*CIMR(vxImmrGet()) |= CIMR_SCC2;
	}

    else
	*CIMR(vxImmrGet()) |= CIMR_SCC1;

    }

/*******************************************************************************
*
* sysCpmEnetIntClear - clear the Ethernet interface interrupt
*
* This routine clears the interrupt for the Ethernet interface specified
* by <unit>.
*
* RETURNS: N/A.
*/

void sysCpmEnetIntClear
    (
    int		unit
    )
    {

    /* If running an 823, use SCC2 */
    if (((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_823DB_MASK) ||
	((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_850DB_MASK))
	{
	*CISR(vxImmrGet()) = CISR_SCC2;
	}

    else
	*CISR(vxImmrGet()) = CISR_SCC1;
    }

/*******************************************************************************
*
* sysCpmAttach - attach wrapper
*
*/

STATUS sysCpmAttach
    (
    int         unit,           /* unit number */
    SCC *       pScc,           /* address of SCC parameter RAM */
    SCC_REG *   pSccReg,        /* address of SCC registers */
    VOIDFUNCPTR * ivec,         /* interrupt vector offset */
    SCC_BUF *   txBdBase,       /* transmit buffer descriptor base address */
    SCC_BUF *   rxBdBase,       /* receive buffer descriptor base address */
    int         txBdNum,        /* number of transmit buffer descriptors */
    int         rxBdNum,        /* number of receive buffer descriptors */
    UINT8 *     bufBase         /* address of memory pool; NONE = malloc it */
    )
    {

    /* If running an 823, use SCC2 */
    if (((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_823DB_MASK) ||
	((*BCSR3 & BCSR3_DBID_MASK) == BCSR3_850DB_MASK))
	{
	return (cpmattach(unit, IF_USR_823ARG1, IF_USR_823ARG2, IF_USR_823ARG3,
			  txBdBase, rxBdBase, txBdNum, rxBdNum, bufBase));
	}

    /* else use SCC1 */
    else
	return (cpmattach(unit, pScc, pSccReg, ivec, 
			  txBdBase, rxBdBase, txBdNum, rxBdNum, bufBase));
    }


#endif	/* INCLUDE_CPM */

#ifdef INCLUDE_MOT_FEC

/*******************************************************************************
*
* sysFecEnetEnable - enable the MII interface of the Fast Ethernet controller
*
* This routine is expected to perform any target specific functions required
* to enable the Ethernet device and the MII interface of the Fast Ethernet 
* controller. These functions include setting the MII-compliant signals on 
* Port D and disabling the IRQ7 signal.
*
* This routine does not enable the 7-wire serial interface.
*
* RETURNS: OK, or ERROR if the Fast Ethernet controller cannot be enabled.
*/

STATUS sysFecEnetEnable
    (
    UINT32	motCpmAddr	/* base address of the on-chip RAM */
    )
    {
    int intLevel = intLock();

    /* enable the Ethernet device for the FEC */

#ifndef CDCIM
    *BCSR4 |= (BCSR4_FETH_RST | BCSR4_FETH_CFG0 
	       | BCSR4_FETH_CFG1 | BCSR4_FETH_FDE); 
    *BCSR4 &= ~BCSR4_UUFDIS;		
#endif /* CDCIM */

    /* mask IRQ7 off, as it is shared with MII_TX_CLK */

    *SIMASK (motCpmAddr) &= ~SIMASK_IRM7;

    /* also clear any pending interrupt */

    *SIPEND (motCpmAddr) |= SIPEND_IRQ7;

    /* 
     * set the arbitration level for the FEC. Do not enable
     * FEC aggressive mode.
     */

    *SDCR (motCpmAddr) |= SDCR_FAID_BR6;

    /* set Port D to use MII signals */

    *PDPAR (motCpmAddr) = 0x1fff;
    *PDDIR (motCpmAddr) = 0x1c58;

    intUnlock (intLevel);

    return (OK);
    }

/*******************************************************************************
*
* sysFecEnetDisable - disable MII interface to the Fast Ethernet controller
*
* This routine is expected to perform any target specific functions required
* to disable the Ethernet device and the MII interface to the Fast Ethernet 
* controller.  This involves restoring the default values for all the Port 
* D signals.
*
* RETURNS: OK, always.
*/

STATUS sysFecEnetDisable
    (
    UINT32	motCpmAddr	/* base address of the on-chip RAM */
    )
    {
    int intLevel = intLock();

    /* disable the Ethernet device for the FEC */
#ifndef CDCIM
    *BCSR4 |= BCSR4_UUFDIS;
    *BCSR4 &= ~BCSR4_FETH_RST;		
#endif /* CDCIM */
    /* configure all Port D pins as general purpose input pins */

    *PDPAR (motCpmAddr) = 0x0;
    *PDDIR (motCpmAddr) = 0x0;

    intUnlock (intLevel);

    return (OK);
    }

/*******************************************************************************
*
* sysFecEnetAddrGet - get the hardware Ethernet address
*
* This routine provides the six byte Ethernet hardware address that will be
* used by each individual Fast Ethernet device unit.  This routine must copy
* the six byte address to the space provided by <addr>.
*
* RETURNS: OK, or ERROR if the Ethernet address cannot be returned.
*/

STATUS sysFecEnetAddrGet
    (
    UINT32	motCpmAddr,	/* base address of the on-chip RAM */
    UCHAR *	addr		/* where to copy the Ethernet address */
    )
    {
#if 0
    bcopy ((char *) sysFecEnetAddr, (char *) addr, sizeof (sysFecEnetAddr));
#endif
    if (sysEnetAddrGet(0, sysFecEnetAddr) != OK) return ERROR;
    addr[0] = sysFecEnetAddr[5];
    addr[1] = sysFecEnetAddr[4];
    addr[2] = sysFecEnetAddr[3];
    addr[3] = sysFecEnetAddr[2];
    addr[4] = sysFecEnetAddr[1];
    addr[5] = sysFecEnetAddr[0];
    return (OK);
    }

#endif /* INCLUDE_MOT_FEC */


/******************************************************************************
*
* sysAtaInit - initialize the EIDE/ATA interface
*
* Perform the necessary initialization required before starting up the
* ATA/EIDE driver.
*/

LOCAL void sysAtaInit
    (
#ifndef CDCIM
    int         pciBusNo,		/* PCI bus number */
    int         pciDevNo,		/* PCI device number */
    int         pciFuncNo		/* PCI function number */
#endif /* CDCIM */
    )
    {
#ifdef INCLUDE_ATA

#ifndef CDCIM
    UINT16      readValue;
    int         baseAddr;

    /* read the WINBOND IDE Device Control register */

    pciConfigInWord (pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_COMMAND,
                        &readValue);

    /* enable the IDE I/O decodes */

    readValue |= PCI_CMD_IO_ENABLE;

    /* disable bus master mode, memory write and invalidate bits */

    readValue &= ~(PCI_CMD_MASTER_ENABLE | PCI_CMD_WI_ENABLE);
    pciConfigOutWord (pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_COMMAND,
					readValue );

    /* read the Port0 and Port1 Primary/Auxillary Register addresses */

    pciConfigInLong (pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_BASE_ADDRESS_0,
                        &baseAddr);
    ataResources[0].resource.ioStart[0] = 
		(CPU_PCI_IO_BA | (baseAddr & PCI_IOBASE_MASK));
    pciConfigInLong (pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_BASE_ADDRESS_1,
                        &baseAddr);

    /* Base Address Reg 1 only decodes byte lane 2 */

    ataResources[0].resource.ioStart[1] = 
		(CPU_PCI_IO_BA | ((baseAddr & PCI_IOBASE_MASK) + 2));
    pciConfigInLong (pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_BASE_ADDRESS_2,
                        &baseAddr);
    ataResources[1].resource.ioStart[0] = 
		(CPU_PCI_IO_BA | (baseAddr & PCI_IOBASE_MASK));
    pciConfigInLong (pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_BASE_ADDRESS_3,
                        &baseAddr);

    /* Base Address Reg 3 only decodes byte lane 2 */

    ataResources[1].resource.ioStart[1] = 
		(CPU_PCI_IO_BA | ((baseAddr & PCI_IOBASE_MASK) + 2));
    pciConfigInLong (pciBusNo, pciDevNo, pciFuncNo, PCI_CFG_BASE_ADDRESS_3,
                        &baseAddr);
#endif /* CDCIM */

    /*
     * initialize the remainder of the ataRsources structure
     *
     * initialize the Controller 0 data structure
     */

    ataResources[0].ctrlType   = IDE_LOCAL;
#ifndef CDCIM
    ataResources[0].drives     = 2;
    ataResources[0].intVector  = IVEC_TO_INUM (IV_IDE0);  /* Interrupt number */
    ataResources[0].intLevel   = IVEC_TO_ILVL (IV_IDE0);  /* Interrupt level */
    ataResources[0].configType = (ATA_PIO_AUTO | ATA_GEO_PHYSICAL);
#else if
    ataResources[0].drives     = 1;
    ataResources[0].intVector  = 0;  /* Interrupt number */
    ataResources[0].intLevel   = 0;  /* Interrupt level */
    ataResources[0].configType = (ATA_PIO_AUTO | ATA_GEO_PHYSICAL | ATA_BITS_16);
#endif /* CDCIM */
    ataResources[0].semTimeout = 0;
    ataResources[0].wdgTimeout = 0;

    /* initialize the Controller 1 data structure */

    ataResources[1].ctrlType   = IDE_LOCAL;
#ifndef CDCIM
    ataResources[1].drives     = 2;
    ataResources[1].intVector  = IVEC_TO_INUM (IV_IDE1);  /* Interrupt Number */
    ataResources[1].intLevel   = IVEC_TO_ILVL (IV_IDE1);  /* Interrupt Level */
    ataResources[1].configType = (ATA_PIO_AUTO | ATA_GEO_PHYSICAL);
#else if
    ataResources[1].drives     = 1;
    ataResources[1].intVector  = 0;  /* Interrupt Number */
    ataResources[1].intLevel   = 0;  /* Interrupt Level */
    ataResources[1].configType = (ATA_PIO_AUTO | ATA_GEO_PHYSICAL | ATA_BITS_16);
#endif /* CDCIM */
    ataResources[1].semTimeout = 0;
    ataResources[1].wdgTimeout = 0;

#endif /* INCLUDE_ATA */
    }


/*******************************************************************************
sysMemMapInit

set memory controller registers to set memory map (except RAM and boot flash)

*/

LOCAL void sysMemMapInit()
{
  /* chip select 1 */ /* Compact Flash */
  *OR1(INTERNAL_MEM_MAP_ADDR)
       = ( CF_ADDR_MASK | OR_CSNT_SAM | OR_ACS_DIV2 | OR_BI | OR_SCY_15_CLK | OR_TRLX);
  *BR1(INTERNAL_MEM_MAP_ADDR)
       = ( (CF_BASE_ADDR & BR_BA_MSK) | BR_MS_GPCM | BR_PS_16 | BR_V);

  /* chip select 3 */ /* DSP 0 */
  *BR3(INTERNAL_MEM_MAP_ADDR)
       = ( (DSP0_HPI_BASE_ADDR & BR_BA_MSK) | BR_MS_GPCM | BR_PS_16 | BR_V);
  *OR3(INTERNAL_MEM_MAP_ADDR)
        = ( DSP0_HPI_ADDR_MASK | OR_BI | OR_ACS_DIV2 | OR_SETA);

  /* chip select 4 */ /* DSP 1 */
  *BR4(INTERNAL_MEM_MAP_ADDR)
       = ( (DSP1_HPI_BASE_ADDR & BR_BA_MSK) | BR_MS_GPCM | BR_PS_16 | BR_V);
  *OR4(INTERNAL_MEM_MAP_ADDR)
        = ( DSP1_HPI_ADDR_MASK | OR_BI | OR_ACS_DIV2 | OR_SETA);
  
  /* chip select 6 */ /* Aux Port */
  *BR6(INTERNAL_MEM_MAP_ADDR)
       = ( (AUX_PORT_ADDR & BR_BA_MSK) | BR_MS_GPCM | BR_PS_32 | BR_V);
  *OR6(INTERNAL_MEM_MAP_ADDR)
        = ( AUX_PORT_ADDR_MASK_V2 | OR_CSNT_SAM | OR_BI | OR_SCY_1_CLK | OR_TRLX);
  
}


