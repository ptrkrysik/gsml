/*
JetCell modification history
----------------------------
99/03/22  ck   Add JetCell modification history
99/03/15  ck   Fix conditional compile
99/02/18  ck   Changes for CDCIM
99/02/18  ck   Initial revision. Copied from ads860 bsp.
*/

/* sysALib.s - Motorola 860ads system-dependent assembly routines */

/* Copyright 1984-1998 Wind River Systems, Inc. */
        .data
	.globl	copyright_wind_river
	.long	copyright_wind_river

/*
modification history
--------------------
01e,09nov98,cn   added support for FADS860T boards.
01d,12jan98,dat  SPR 20104, correct use of HI and HIADJ macros
01c,04nov96,tpr  clean up + fix SPR # 7173.
01b,24may96,tpr  added MMU initialization.
01a,19apr96,tpr  written.
*/

/*
DESCRIPTION
This module contains system-dependent routines written in assembly
language.

This module must be the first specified in the \f3ld\f1 command used to
build the system.  The sysInit() routine is the system start-up code.
*/

#define _ASMLANGUAGE
#include "vxWorks.h"
#include "asm.h"
#include "cacheLib.h"
#include "config.h"
#include "regs.h"	
#include "sysLib.h"

	/* globals */

	.globl	_sysInit		/* start of system code */

	/* externals */

	.extern usrInit
	
	.text

/*******************************************************************************
*
* sysInit - start after boot
*
* This is the system start-up entry point for VxWorks in RAM, the
* first code executed after booting.  It disables interrupts, sets up
* the stack, and jumps to the C routine usrInit() in usrConfig.c.
*
* The initial stack is set to grow down from the address of sysInit().  This
* stack is used only by usrInit() and is never used again.  Memory for the
* stack must be accounted for when determining the system load address.
*
* NOTE: This routine should not be called by the user.
*
* RETURNS: N/A

* sysInit (void)              /@ THIS IS NOT A CALLABLE ROUTINE @/

*/

_sysInit:
	/*
	 * disable external interrupts and Instruction/Data MMU, set
	 * the exception prefix 
	 */

        mfmsr   p0                      /* p0 = msr    */
        INT_MASK(p0, p1)                /* mask EE bit */
	rlwinm	p1, p1, 0, _PPC_MSR_BIT_DR + 1, _PPC_MSR_BIT_IR - 1
	rlwinm  p1, p1, 0, _PPC_MSR_BIT_IP + 1, _PPC_MSR_BIT_IP - 1
        mtmsr   p1                      /* msr = p1    */
	isync				/* ISYNC */

	/* disable instruction and data caches */

	lis	p1, HIADJ ( CACHE_CMD_DISABLE)		/* load disable cmd */
	addi	p1, p1, LO (CACHE_CMD_DISABLE)
	mtspr	IC_CST, p1				/* Disable I cache */
	mtspr	DC_CST, p1				/* Disable D cache */

	/* unlock instruction and data caches */

	lis     p1, HIADJ ( CACHE_CMD_UNLOCK_ALL)	/* load unlock cmd */
	addi	p1, p1, LO (CACHE_CMD_UNLOCK_ALL)
        mtspr   IC_CST, p1				/* Unlock I cache */
        mtspr   DC_CST, p1				/* Unlock D cache */

	/* invalidate instruction and data caches */

        lis     p1, HIADJ ( CACHE_CMD_INVALIDATE)	/* load invalidate cmd*/
	addi	p1, p1, LO (CACHE_CMD_INVALIDATE)
        mtspr   IC_CST, p1				/* Invalidate I cache */
        mtspr   DC_CST, p1				/* Invalidate D cache */

	/* invalidate entries within both TLBs */

	tlbia

#ifndef CDCIM
        /* Get the board revision number */

	lis	r4, HIADJ(BCSR3)	/* load r4 with the BCSR3 address */
	lwz	r5, LO(BCSR3)(r4)	/* load r5 with the BCSR3 value */

	lis	r4, HI(BCSR3_BREVN)
	and	r5, r5, r4		/* extract board revision number */

	/* disable all devices (serial, ethernet, ...) */

	cmpwi	r5, 0			/* if board revision number is 0 (ENG)*/
	bne	noEngBoard

	lis	r4, HI(BCSR1_RESET_VAL_ENG)	/* then use the ENG reset val */
	bl	bscr1Init

noEngBoard:
	lis	r4, HI(BCSR1_RESET_VAL)		/* otherwise normal value */

bscr1Init:
	lis	r5, HIADJ (BCSR1)
	stw	r4, LO(BCSR1)(r5)		/* reset the BCSR1 register */

#ifdef  FADS_860T
        lis     r4, HIADJ(BCSR4)                /* load r4 with BCSR4 address */
        lwz     r5, LO(BCSR4)(r4)               /* load r5 with BCSR4 value */
        lis     r4, HI(BCSR4_UUFDIS)		/* load r4 with disable value */
        or     	r5, r5, r4			/* load r5 with BCSR4 value */
        lis     r4, HI(~BCSR4_FETH_RST)		/* load r4 with reset value */
        and     r5, r5, r4			/* load r5 with BCSR4 value */
        lis     r4, HIADJ(BCSR4)                /* load r4 with BCSR4 address */
        stw     r5, LO(BCSR4)(r4)               /* reset the FEC tranceiver */
#endif
#endif  /* CDCIM */ 
	/* initialize Small Data Area (SDA) start address */

#if	FALSE				/* XXX TPR SDA not supported yet */
	lis     r2, HIADJ( _SDA2_BASE_)
	addi    r2, r2, LO(_SDA2_BASE_)

	lis     r13, HIADJ ( _SDA_BASE_)
	addi    r13, r13, LO(_SDA_BASE_)
#endif

	/* initialize the stack pointer */
	
	lis     sp, HIADJ( RAM_LOW_ADRS)
	addi    sp, sp, LO(RAM_LOW_ADRS)

	/* set the default boot code */
	
	lis	r3, HIADJ( BOOT_WARM_AUTOBOOT)
	addi	r3, r3, LO(BOOT_WARM_AUTOBOOT)

	/* jump to usrInit */

	addi	sp, sp, -FRAMEBASESZ	/* get frame stack */
	b	usrInit			/* never returns - starts up kernel */
