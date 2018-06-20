/*
JetCell modification history
----------------------------
99/06/01  ck   Add a mcro to load 32 bit constant into a register.
99/03/22  ck   Add JetCell modification history
99/03/15  ck   Use conditional compiles. Modify SIUMCR register value, keep DPC bit 0.
99/02/18  ck   Changes for CDCIM
99/02/18  ck   Initial revision. Copied from ads860 bsp.

*/

/* romInit.s - Motorola 860ads ROM initialization module */

/* Copyright 1984-1996 Wind River Systems, Inc. */
	.data
	.globl  copyright_wind_river
	.long   copyright_wind_river

/*
modification history
--------------------
01l,09nov98,cn   added support for FADS860T boards.
01k,30jul98,gls  fixed to initialize CS0/CS1 earlier to allow access to BCSR3
01j,18feb98,gls	 added support for PPC823FADS
01i,23jun97,map  fixed MAMR init for 16/32 DRAM [SPR# 8753]
01h,17dec96,tpr  fixed MAMR init. bug with 16 and 32 Mbytes dram (SPR #7661)
01g,10nov96,tpr  updated UPM table. 
		 removed RTCSC initialization.
01f,08nov96,tpr  modified UPM table for EDO Dram @ 50 Mhz.
01e,06nov96,tpr  replaced PLPRCR_MF_SFT by PLPRCR_MF_SHIFT.
01d,06nov96,tpr  Clean up the code (SPR #7336).
		 added DRAM speed selection (SPR #7335).
01c,24jun96,tpr  Reworked all initialization code.
01b,04jun96,cah  Add mem controller minimal init sequence
01a,19apr96,tpr  written.
*/

/*
DESCRIPTION
This module contains the entry code for the VxWorks bootrom.
The entry point romInit, is the first code executed on power-up.
It sets the BOOT_COLD parameter to be passed to the generic
romStart() routine.

The routine sysToMonitor() jumps to the location 4 bytes
past the beginning of romInit, to perform a "warm boot".
This entry point allows a parameter to be passed to romStart().

*/

#define	_ASMLANGUAGE
#include "vxWorks.h"
#include "asm.h"
#include "cacheLib.h"
#include "config.h"
#include "regs.h"	
#include "sysLib.h"
#include "drv/multi/ppc860Siu.h"
	

         /* load a 32-bit constant into a register using the shortest sequence */
         .macro li32    reg, const32bit
         .if     (\const32bit & 0xffff0000)
                 .if     ((\const32bit & 0xffff8000) - 0xffff8000)
                         lis     \reg,(((\const32bit)>>16) & 0xffff)
                         .if     (\const32bit & 0xffff)
                                 addi    \reg,\reg,((\const32bit) & 0xffff)
                         .endif
                 .else
                         li      \reg,(\const32bit & -1)
                 .endif
         .else
                 .if     (\const32bit & 0x8000)
                         li      \reg,0
                         ori     \reg,\reg,(\const32bit)
                 .else
                         li      \reg,((\const32bit) & 0xffff)
                 .endif
         .endif
         .endm

	
	/* internals */

	.globl	_romInit	/* start of system code */
	.globl	romInit		/* start of system code */
	
	/* externals */

	.extern romStart	/* system initialization routine */

	.text
	.align 2

/******************************************************************************
*
* romInit - entry point for VxWorks in ROM
*

* romInit
*     (
*     int startType	/@ only used by 2nd entry point @/
*     )

*/

_romInit:
romInit:
	bl	cold		/* jump to the cold boot initialization */
	
	bl	start		/* jump to the warm boot initialization */

	/* copyright notice appears at beginning of ROM (in TEXT segment) */

	.ascii   "Copyright 1984-1996 Wind River Systems, Inc."
	.align 2

cold:
	li	r3, BOOT_COLD	/* set cold boot as start type */

	/*
	 * When the PowerPC 860 is powered on, the processor fletch the
	 * instructions located at the address 0x100. We need to jump
	 * from the address 0x100 to the Flash space.
	 */

	lis	r4, HIADJ(start)		/* load r4 with the address */
	addi	r4, r4, LO(start)		/* of start */

	lis	r5, HIADJ(romInit)		/* load r5 with the address */
	addi	r5, r5, LO(romInit)		/* of romInit() */

	lis	r6, HIADJ(ROM_TEXT_ADRS)	/* load r6 with the address */
	addi	r6, r6, LO(ROM_TEXT_ADRS)	/* of ROM_TEXT_ADRS */

	sub	r4, r4, r5			/*  */
	add	r4, r4, r6 

	mtspr	LR, r4				/* save destination address*/
						/* into LR register */
	blr					/* jump to flash mem address */
			
start:
	/* set the MSR register to a known state */

	xor	r4, r4, r4		/* clear register R4 */
	mtmsr 	r4			/* cleat the MSR register */

	/* DER - clear the Debug Enable Register */

	mtspr	DER, r4

	/* ICR - clear the Interrupt Cause Register */

	mtspr	ICR, r4

	/* 
	 * ICTRL - initialize the Intstruction Support Control register
	 *	   
	 */

	lis	r5, HIADJ(0x00000007)
	addi	r5, r5, LO(0x00000007)
	mtspr	ICTRL, r5

	/* disable the instruction/data cache */
	
	lis	r4, HIADJ ( CACHE_CMD_DISABLE)		/* load disable cmd */
	addi	r4, r4, LO (CACHE_CMD_DISABLE)
	mtspr	IC_CST, r4				/* disable I cache */
	mtspr	DC_CST, r4				/* disable D cache */

	/* unlock the instruction/data cache */

	lis	r4, HIADJ ( CACHE_CMD_UNLOCK_ALL)	/* load unlock cmd */
	addi	r4, r4, LO (CACHE_CMD_UNLOCK_ALL)
	mtspr	IC_CST, r4			/* unlock all I cache lines */
	mtspr	DC_CST, r4			/* unlock all D cache lines */

	/* invalidate the instruction/data cache */

	lis	r4, HIADJ (CACHE_CMD_INVALIDATE)   /* load invalidate cmd*/
	addi	r4, r4, LO (CACHE_CMD_INVALIDATE)
	mtspr	IC_CST, r4		/* invalidate all I cache lines */
	mtspr	DC_CST, r4		/* invalidate all D cache lines */

	/*
	 * initialize the IMMR register before any non-core registers
	 * modification.
	 */

	lis	r4, HIADJ( INTERNAL_MEM_MAP_ADDR)	
	addi	r4, r4, LO(INTERNAL_MEM_MAP_ADDR)
	mtspr	IMMR, r4		/* initialize the IMMR register */

	mfspr	r4, IMMR		/* read it back, to be sure */
	rlwinm  r4, r4, 0, 0, 15	/* only high 16 bits count */

	/* SYPCR - turn off the system protection stuff */
	
#if 0
	lis	r5, HIADJ( SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF)
	addi	r5, r5, LO(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF)
#endif
	li32	r5, ( SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF)
	stw	r5, SYPCR(0)(r4)

	/*
	 * Map the bank 0 to the flash area - On the ADS board at reset time
	 * the bank 0 is already used to map the flash.
	 */

	lis	r5, HIADJ( (ROM_BASE_ADRS & BR_BA_MSK) | (BR_PS_8 & BR_PS_MSK) | BR_V)
	addi	r5, r5, LO((ROM_BASE_ADRS & BR_BA_MSK) | (BR_PS_8 & BR_PS_MSK) | BR_V)
	stw	r5, BR0(0)(r4)
   

	lis	r5, HIADJ( 0xffc00000 | OR_CSNT_SAM | OR_BI | OR_SCY_5_CLK | \
			   OR_TRLX)
	addi	r5, r5, LO(0xffc00000 | OR_CSNT_SAM | OR_BI | OR_SCY_5_CLK | \
			   OR_TRLX)
	stw	r5, OR0(0)(r4)
	
#ifdef CDCIM
   /* Setup access to the AUXPORT */
	lis	r5, HIADJ( (AUX_PORT_ADDR & BR_BA_MSK) | BR_MS_GPCM | BR_PS_32 | BR_V)
	addi	r5, r5, LO((AUX_PORT_ADDR & BR_BA_MSK) | BR_MS_GPCM | BR_PS_32 | BR_V)
	stw	r5, BR6(0)(r4) 
   

	lis	r5, HIADJ( AUX_PORT_ADDR_MASK_V2 | OR_CSNT_SAM | OR_BI | OR_SCY_1_CLK | \
			   OR_TRLX)
	addi	r5, r5, LO(AUX_PORT_ADDR_MASK_V2 | OR_CSNT_SAM | OR_BI | OR_SCY_1_CLK | \
			   OR_TRLX)
	stw	r5, OR6(0)(r4)  
#endif
   
#ifndef CDCIM
	/*
	 * Map the bank 1 to the Board Status and Control Registers
	 */

	lis	r5, HIADJ( (BCSR0 & BR_BA_MSK) | BR_V)
	addi	r5, r5, LO((BCSR0 & BR_BA_MSK) | BR_V)
	stw	r5, BR1(0)(r4)

	lis	r5, HIADJ( (0xffff8000 & OR_AM_MSK) | OR_BI | OR_SCY_1_CLK)
	addi	r5, r5, LO((0xffff8000 & OR_AM_MSK) | OR_BI | OR_SCY_1_CLK)
	stw	r5, OR1(0)(r4)
#endif	
	
#ifndef CDCIM
	/* Check for FADS,  set the SIUMCR accordingly */

	
	lis	r5, HIADJ(BCSR3)	/* load r5 with the BCSR3 address */
	lwz	r6, LO(BCSR3)(r5)	/* load r6 with the BCSR3 value */
	
	andis.	r5, r6, 0x3f00		/* Check for FADS board */
	bgt	fads

	/* set the SIUMCR register for important debug port, etc... stuff */

	lis	r6, HIADJ( SIUMCR_FRC	       | SIUMCR_DLK  | SIUMCR_DPC | \
			   SIUMCR_MLRC_3STATES | SIUMCR_AEME | SIUMCR_GB5E)
	addi	r6, r6, LO(SIUMCR_FRC	       | SIUMCR_DLK  | SIUMCR_DPC | \
			   SIUMCR_MLRC_3STATES | SIUMCR_AEME | SIUMCR_GB5E)
	b	siu_done

fads:
	lis	r6, HIADJ(SIUMCR_DLK  | SIUMCR_DPC | \
			SIUMCR_MLRC_3STATES | SIUMCR_GB5E)
	addi	r6, r6, LO( SIUMCR_DLK  | SIUMCR_DPC | \
			SIUMCR_MLRC_3STATES | SIUMCR_GB5E)
#endif

#ifdef CDCIM
	lis	r6, HIADJ(SIUMCR_DLK  | \
			SIUMCR_MLRC_3STATES | SIUMCR_GB5E)
	addi	r6, r6, LO( SIUMCR_DLK  | \
			SIUMCR_MLRC_3STATES | SIUMCR_GB5E)
#endif
	
siu_done:	

	lwz	r5, SIUMCR(0)(r4)
	or	r5, r5, r6
	stw	r5, SIUMCR(0)(r4)

	/* TBSCR - initialize the Time Base Status and Control register */

	lis	r5, HIADJ( TBSCR_REFA | TBSCR_REFB)
	addi	r5, r5, LO(TBSCR_REFA | TBSCR_REFB)
	sth	r5, TBSCR(0)(r4)

	/* set PIT status and control init value */

	li	r5, PISCR_PS | PISCR_PITF
	sth	r5, PISCR(0)(r4)

#ifndef CDCIM
	/* set the SPLL frequency to 20 Mhz */

	lis	r5, HIADJ( (SPLL_MUL_FACTOR_20MHZ << PLPRCR_MF_SHIFT) | \
								PLPRCR_TEXPS)
	addi	r5, r5, LO((SPLL_MUL_FACTOR_20MHZ << PLPRCR_MF_SHIFT) | \
								PLPRCR_TEXPS)
	stw	r5, PLPRCR(0)(r4)

	/* divide by 16 */

	li	r5, MPTPR_PTP_DIV16 
	sth	r5, MPTPR(0)(r4)
#endif

#ifdef CDCIM
	lis	r5, HIADJ((0x13 << MAMR_PTA_SHIFT) | MAMR_PTAE | \
			MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | \
			MAMR_G0CLA_A10 | \
			MAMR_RLFA_1X | MAMR_WLFA_1X | MAMR_TLFA_4X)

	addi	r5, r5, LO((0x13 << MAMR_PTA_SHIFT) | MAMR_PTAE | \
			MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL |MAMR_G0CLA_A10 | \
			MAMR_RLFA_1X | MAMR_WLFA_1X	| \
			MAMR_TLFA_4X)
#else
	lis	r5, HIADJ((0x13 << MAMR_PTA_SHIFT) | MAMR_PTAE | \
			MAMR_AMA_TYPE_2 | MAMR_DSA_1_CYCL | \
			MAMR_G0CLA_A12 | MAMR_GPL_A4DIS | \
			MAMR_RLFA_1X | MAMR_WLFA_1X | MAMR_TLFA_4X)

	addi	r5, r5, LO((0x13 << MAMR_PTA_SHIFT) | MAMR_PTAE | \
			MAMR_AMA_TYPE_2 | MAMR_DSA_1_CYCL |MAMR_G0CLA_A12 | \
			MAMR_GPL_A4DIS | MAMR_RLFA_1X | MAMR_WLFA_1X	| \
			MAMR_TLFA_4X) 
#endif	

    stw	r5, MAMR(0)(r4)


   /* Initialize SDRAM   CK */

	/*
	 * load r6/r7 with the start/end address of the UPM table. 
	 */

	lis	r6, HIADJ( UpmTable)
	addi	r6, r6, LO(UpmTable)

	lis	r7, HIADJ( UpmTableEnd)
	addi	r7, r7, LO(UpmTableEnd)
		

#ifndef CDCIM
	/* get the DRAM speed and if EDO capability supported */

	lis	r5, HIADJ(BCSR2)	/* load r5 with the BCSR2 address */
	lwz	r6, LO(BCSR2)(r5)	/* load r6 with the BCSR2 value */
	lis	r5, HI(BCSR2_DRAM_NO_EDO_L | BCSR2_DRAM_PD_SPEED_MASK)
	and	r6, r6, r5

	lis	r5, HI(BCSR2_DRAM_NO_EDO_L | BCSR2_DRAM_PD_60NS_SIMM)
	cmpw	r6, r5
	beq	dram60ns

	lis	r5, HI(BCSR2_DRAM_NO_EDO_L | BCSR2_DRAM_PD_70NS_SIMM)
	cmpw	r6, r5
	beq	dram70ns

	lis	r5, HI(BCSR2_DRAM_EDO_L | BCSR2_DRAM_PD_60NS_SIMM)
	cmpw	r6, r5
	beq	dramEdo60ns

	lis	r5, HI(BCSR2_DRAM_EDO_L | BCSR2_DRAM_PD_70NS_SIMM)
	cmpw	r6, r5
	beq	dramEdo70ns

dram60ns:
#ifndef	EDO_DRAM
	/*
	 * load r6/r7 with the start/end address of the UPM table for a
	 * none EDO 60ns Dram.
	 */

	lis	r6, HIADJ( UpmTable60)
	addi	r6, r6, LO(UpmTable60)

	lis	r7, HIADJ( UpmTable60End)
	addi	r7, r7, LO(UpmTable60End)
	b	upmInit	
#endif

dramEdo60ns:
	/*
	 * load r6/r7 with the start/end address of the UPM table for an
	 * EDO 60ns Dram.
	 */

	lis	r6, HIADJ( UpmTableEdo60)
	addi	r6, r6, LO(UpmTableEdo60)

	lis	r7, HIADJ( UpmTableEdo60End)
	addi	r7, r7, LO(UpmTableEdo60End)
	b	upmInit	

dram70ns:
#ifndef	EDO_DRAM
	/*
	 * load r6/r7 with the start/end address of the UPM table for a
	 * none EDO 70ns Dram.
	 */

	lis	r6, HIADJ( UpmTable70)
	addi	r6, r6, LO(UpmTable70)

	lis	r7, HIADJ( UpmTable70End)
	addi	r7, r7, LO(UpmTable70End)
	b	upmInit	
#endif

dramEdo70ns:
	/*
	 * load r6/r7 with the start/end address of the UPM table for an
	 * EDO 70ns Dram.
	 */

	lis	r6, HIADJ( UpmTableEdo70)
	addi	r6, r6, LO(UpmTableEdo70)

	lis	r7, HIADJ( UpmTableEdo70End)
	addi	r7, r7, LO(UpmTableEdo70End)
#endif /* ifndef CDCIM */

upmInit:
	/* init UPMA for memory access */

	sub	r5, r7, r6		/* compute table size */
	srawi	r5, r5, 2		/* in integer size */

	/* convert UpmTable to ROM based addressing */

	lis	r7, HIADJ(romInit)	
	addi	r7, r7, LO(romInit)

	lis	r8, HIADJ(ROM_TEXT_ADRS)
	addi	r8, r8, LO(ROM_TEXT_ADRS)

	sub	r6, r6, r7		/* subtract romInit base address */
	add	r6, r6, r8 		/* add in ROM_TEXT_ADRS address */

					/* Command: OP=Write, UPMA, MAD=0 */
	lis	r9, HIADJ (MCR_OP_WRITE | MCR_UM_UPMA | MCR_MB_CS0)
	addi	r9, r9, LO(MCR_OP_WRITE | MCR_UM_UPMA | MCR_MB_CS0)

UpmWriteLoop:	
	/* write the UPM table in the UPM */

	lwz	r10, 0(r6)		/* get data from table */
	stw	r10, MDR(0)(r4)		/* store the data to MD register */

	stw	r9, MCR(0)(r4)		/* issue command to MCR register */

	addi	r6, r6, 4		/* next entry in the table */
	addi	r9, r9, 1		/* next MAD address */
	cmpw	r9, r5			/* done yet ? */
	blt	UpmWriteLoop

#ifndef CDCIM	

      /* get the DRAM size, and Map the bank 2 & 3 to the Dram area */

	lis	r5, HIADJ(BCSR2)	/* load r5 with the BCSR2 address */
	lwz	r6, LO(BCSR2)(r5)	/* load r6 with the BCSR2 value */
	lis	r5, HI(BCSR2_DRAM_PD_SIZE_MASK)
	and	r6, r6, r5

	lis	r5, HI(BCSR2_DRAM_PD_4MEG_SIMM)
	cmpw	r6, r5
	beq	dram4meg

	lis	r5, HI(BCSR2_DRAM_PD_8MEG_SIMM)
	cmpw	r6, r5
	beq	dram8meg

	lis	r5, HI(BCSR2_DRAM_PD_16MEG_SIMM)
	cmpw	r6, r5
	beq	dram16meg

	lis	r5, HI(BCSR2_DRAM_PD_32MEG_SIMM)
	cmpw	r6, r5
	beq	dram32meg

dram32meg:
	/* program BR3 */

	lis	r5, HIADJ( ((0x01000000 + LOCAL_MEM_LOCAL_ADRS) & \
				BR_BA_MSK) | BR_MS_UPMA | BR_V)
	addi	r5, r5, LO(((0x01000000 + LOCAL_MEM_LOCAL_ADRS) & \
				BR_BA_MSK) | BR_MS_UPMA | BR_V)
	stw	r5, BR3(0)(r4)

dram16meg:	
	/* compute the OR3/OR2 value for a 16M block size */

	lis	r5, HIADJ(0xff000000 | OR_CSNT_SAM)
	addi	r5, r5, LO(0xff000000 | OR_CSNT_SAM)
	stw	r5, OR3(0)(r4)	/* Set OR3. Used only when BR3 is valid */

	/* change the Address Multiplexing in MAMR */

	lwz	r6, MAMR(0)(r4)
	lis	r9, HIADJ(~MAMR_AMA_MSK)	
	addi	r9, r9, LO(~MAMR_AMA_MSK)	
	and	r6, r6, r9		/* clear the AMA bits in MAMR */
	lis	r9, HIADJ(MAMR_AMA_TYPE_3)
	addi	r9, r9, LO(MAMR_AMA_TYPE_3)
	or	r6, r6, r9		/* set the AMA bits */
	stw	r6, MAMR(0)(r4)

	b	dramInit

dram8meg:
	/* program BR3 */

	lis	r5, HIADJ( ((0x00400000 + LOCAL_MEM_LOCAL_ADRS) & \
				BR_BA_MSK) | BR_MS_UPMA | BR_V)
	addi	r5, r5, LO(((0x00400000 + LOCAL_MEM_LOCAL_ADRS) & \
				BR_BA_MSK) | BR_MS_UPMA | BR_V)
	stw	r5, BR3(0)(r4)

dram4meg:	
	/* compute the OR3/OR2 value for a 4M block size */

	lis	r5, HIADJ( 0xffc00000 | OR_CSNT_SAM)
	addi	r5, r5, LO(0xffc00000 | OR_CSNT_SAM)
	stw	r5, OR3(0)(r4)	/* Set OR3. Used only when BR3 is valid */
	b	dramInit

#endif /* ifndef CDCIM */ 
		
dramInit:	
	/* Map the bank 2 to the Dram area */

#ifndef CDCIM
	stw	r5, OR2(0)(r4)	/* set OR2 to the previously computed value */
#endif
	lis	r5, HIADJ( (LOCAL_MEM_LOCAL_ADRS & BR_BA_MSK) | BR_MS_UPMA | \
			    BR_V)
	addi	r5, r5, LO((LOCAL_MEM_LOCAL_ADRS & BR_BA_MSK) | BR_MS_UPMA | \
			    BR_V)
	stw	r5, BR2(0)(r4)
      
#ifdef CDCIM
	lis	r5, HIADJ(0xf0000000 | OR_CSNT_SAM | OR_ACS_DIV2 | OR_TRLX)
	addi	r5, r5, LO(0xf0000000 | OR_CSNT_SAM | OR_ACS_DIV2 | OR_TRLX)
	stw	r5, OR2(0)(r4)	
#endif

	/* Sdram intializtion */

	lis	r5, HIADJ( 0xffff)
        addi    r5, r5, LO (0xffff)
	lis	r6, ( 0x0000)

sdramDelayLoop:
	addi	r6, r6, 1
	cmpw	r6, r5			/* done yet ? */
	blt	sdramDelayLoop


	lis	r5, HIADJ( MCR_OP_RUN | MCR_MB_CS2 | MCR_MCLF_1X |  5)
	ori	r5, r5, LO( MCR_OP_RUN | MCR_MB_CS2 | MCR_MCLF_1X |  5)
	stw	r5, MCR(0)(r4)
	lis	r5, HIADJ( MCR_OP_RUN | MCR_MB_CS2 | MCR_MCLF_8X | 30)
	ori	r5, r5, LO( MCR_OP_RUN | MCR_MB_CS2 | MCR_MCLF_8X | 30)
	stw	r5, MCR(0)(r4)
	li	r5, ( 0x88)
	stw	r5, MAR(0)(r4)
	lis	r5, HIADJ( MCR_OP_RUN | MCR_MB_CS2 | MCR_MCLF_1X |  6)
	ori	r5, r5, LO( MCR_OP_RUN | MCR_MB_CS2 | MCR_MCLF_1X |  6)
	stw	r5, MCR(0)(r4)

      
#ifndef CDCIM   /* skip BSCR stuff  CK */
   
	/* Get the board revision number */

	lis	r4, HIADJ(BCSR3)	/* load r4 with the BCSR3 address */
	lwz	r5, LO(BCSR3)(r4)	/* load r5 with the BCSR3 value */

	andis.	r6, r5, 0x3f00		/* 8xxFADS DRAM is active low */
	bgt	noEngBoard
	
	lis	r4, HI(BCSR3_BREVN)
	and	r5, r5, r4		/* extract board revision number */

        /* disable all devices (serial, ethernet, ...) */

	cmpwi	r5, 0			/* if board revision number is 0 */
	bne	noEngBoard 
	
	lis	r4, HI(BCSR1_RESET_VAL_ENG) /* then use the ENG reset val */
	bl	bscr1Init

noEngBoard:
	lis	r4, HI(BCSR1_RESET_VAL)		/* otherwise normal value */

bscr1Init:
	lis	r5, HIADJ(BCSR1)
	stw	r4, LO(BCSR1)(r5)		/* reset the BCSR1 register */

#ifdef  FADS_860T
        lis     r4, HIADJ(BCSR4)                /* load r4 with BCSR4 address */
        lwz     r5, LO(BCSR4)(r4)               /* load r5 with BCSR4 value */
        lis     r4, HI(BCSR4_UUFDIS)            /* load r4 with disable value */
        or      r5, r5, r4                      /* load r5 with BCSR4 value */
        lis     r4, HI(~BCSR4_FETH_RST)         /* load r4 with reset value */
        and     r5, r5, r4                      /* load r5 with BCSR4 value */
        lis     r4, HIADJ(BCSR4)                /* load r4 with BCSR4 address */
        stw     r5, LO(BCSR4)(r4)               /* reset the FEC tranceiver */
#endif

#endif  /* ifndef CDCIM */
 

        /* initialize the stack pointer */

	lis	sp, HIADJ(STACK_ADRS)
	addi	sp, sp, LO(STACK_ADRS)
	
        /* initialize r2 and r13 according to EABI standard */
#if	FALSE					/* SDA Not supported yet */

	lis	r2, HIADJ(_SDA2_BASE_)
	addi	r2, r2, LO(_SDA2_BASE_)

	lis	r13, HIADJ(_SDA_BASE_)
	addi	r13, r13, LO(_SDA_BASE_)
#endif

	/* go to C entry point */

	addi	sp, sp, -FRAMEBASESZ		/* get frame stack */

	/* 
	 * calculate C entry point: routine - entry point + ROM base 
	 * routine	= romStart
	 * entry point	= romInit	= R7
	 * ROM base	= ROM_TEXT_ADRS = R8
	 * C entry point: romStart - R7 + R8 
	 */

        lis	r6, HIADJ(romStart)	
        addi	r6, r6, LO(romStart)	/* load R6 with C entry point */

	sub	r6, r6, r7		/* routine - entry point */
	add	r6, r6, r8 		/* + ROM base */

	mtlr	r6			/* move C entry point to LR */
	blr				/* jump to the C entry point */

#ifndef CDCIM 

#if	(SPLL_FREQ != FREQ_25_MHZ)
/* UPM initialization table, 60ns, 50MHz */

UpmTable60:
# 
# /* DRAM 60ns - single read. (offset 0 in upm RAM) */
        .long	0x8fffec24, 0x0fffec04, 0x0cffec04, 0x00ffec04
	.long	0x00ffec00, 0x37ffec47
# /* offsets 6-7 not used */
        .long	0xffffffff, 0xffffffff
# /* DRAM 60ns - burst read. (offset 8 in upm RAM) */
        .long	0x8fffec24, 0x0fffec04, 0x08ffec04, 0x00ffec0c
        .long	0x03ffec00, 0x00ffec44, 0x00ffcc08, 0x0cffcc44
        .long	0x00ffec0c, 0x03ffec00, 0x00ffec44, 0x00ffcc00
        .long	0x3fffc847
# /* offsets 15-17 not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff 
# /* DRAM 60ns - single write. (offset 18 in upm RAM) */
        .long	0x8fafcc24, 0x0fafcc04, 0x0cafcc00, 0x11bfcc47
# /* offsets 1c-1f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - burst write. (offset 20 in upm RAM) */
        .long	0x8fafcc24, 0x0fafcc04, 0x0cafcc00, 0x03afcc4c
        .long	0x0cafcc00, 0x03afcc4c, 0x0cafcc00, 0x03afcc4c
        .long	0x0cafcc00, 0x33bfcc4f
# /* offsets 2a-2f not used */
        .long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - refresh. (offset 30 in upm RAM) */
        .long	0xc0ffcc84, 0x00ffcc04, 0x07ffcc04, 0x3fffcc06
        .long	0xffffcc85, 0xffffcc05
# /* offsets 36-3b not used */
	.long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - exception. (offset 3c in upm RAM) */
        .long	0x33ffcc07
# /* offset 3d-3f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
UpmTable60End:

/* UPM initialization table, 70ns, 50MHz */

UpmTable70:
# 
# /* DRAM 70ns - single read. (offset 0 in upm RAM) */
        .long	0x8fffcc24, 0x0fffcc04, 0x0cffcc04, 0x00ffcc04
        .long	0x00ffcc00, 0x37ffcc47
# /* offsets 6-7 not used */
        .long	0xffffffff, 0xffffffff
# /* DRAM 70ns - burst read. (offset 8 in upm RAM) */
        .long	0x8fffcc24, 0x0fffcc04, 0x0cffcc04, 0x00ffcc04
        .long	0x00ffcc08, 0x0cffcc44, 0x00ffec0c, 0x03ffec00
        .long	0x00ffec44, 0x00ffcc08, 0x0cffcc44, 0x00ffec04
        .long	0x00ffec00, 0x3fffec47 
# /* offsets 16-17 not used */
        .long	0xffffffff, 0xffffffff
# /* DRAM 70ns - single write. (offset 18 in upm RAM) */
        .long	0x8fafcc24, 0x0fafcc04, 0x0cafcc00, 0x11bfcc47
# /* offsets 1c-1f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - burst write. (offset 20 in upm RAM) */
        .long	0x8fafcc24, 0x0fafcc04, 0x0cafcc00, 0x03afcc4c
        .long	0x0cafcc00, 0x03afcc4c, 0x0cafcc00, 0x03afcc4c
        .long	0x0cafcc00, 0x33bfcc4f
# /* offsets 2a-2f not used */
        .long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - refresh. (offset 30 in upm RAM) */
        .long	0xe0ffcc84, 0x00ffcc04, 0x00ffcc04,  0x0fffcc04
        .long	0x7fffcc06, 0xffffcc85, 0xffffcc05
# /* offsets 37-3b not used */
	.long	0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - exception. (offset 3c in upm RAM) */
        .long	0x33ffcc07
# /* offset 3d-3f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
UpmTable70End:

/* UPM initialization table, EDO, 60ns, 50MHz */

UpmTableEdo60:
# 
# /* DRAM 60ns - single read. (offset 0 in upm RAM) */
        .long	0x8ffbec24, 0x0ff3ec04, 0x0cf3ec04, 0x00f3ec04
	.long	0x00f3ec00, 0x37f7ec47
# /* offsets 6-7 not used */
        .long	0xffffffff, 0xffffffff
# /* DRAM 60ns - burst read. (offset 8 in upm RAM) */
        .long	0x8fffec24, 0x0ffbec04, 0x0cf3ec04, 0x00f3ec0c
        .long	0x0cf3ec00, 0x00f3ec4c, 0x0cf3ec00, 0x00f3ec4c
        .long	0x0cf3ec00, 0x00f3ec44, 0x03f3ec00, 0x3ff7ec47
# /* offsets 14-17 not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff 
# /* DRAM 60ns - single write. (offset 18 in upm RAM) */
        .long	0x8fffcc24, 0x0fefcc04, 0x0cafcc00, 0x11bfcc47
# /* offsets 1c-1f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - burst write. (offset 20 in upm RAM) */
        .long	0x8fffcc24, 0x0fefcc04, 0x0cafcc00, 0x03afcc4c
        .long	0x0cafcc00, 0x03afcc4c, 0x0cafcc00, 0x03afcc4c
        .long	0x0cafcc00, 0x33bfcc4f
# /* offsets 2a-2f not used */
        .long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - refresh. (offset 30 in upm RAM) */
        .long	0xc0ffcc84, 0x00ffcc04, 0x07ffcc04, 0x3fffcc06
        .long	0xffffcc85, 0xffffcc05
# /* offsets 36-3b not used */
	.long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - exception. (offset 3c in upm RAM) */
        .long	0x33ffcc07
# /* offset 3d-3f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
UpmTableEdo60End:

/* UPM initialization table, EDO, 70ns, 50MHz */

UpmTableEdo70:
# 
# /* DRAM 70ns - single read. (offset 0 in upm RAM) */
        .long	0x8ffbcc24, 0x0ff3cc04, 0x0cf3cc04, 0x00f3cc04
        .long	0x00f3cc00, 0x37f7cc47
# /* offsets 6-7 not used */
        .long	0xffffffff, 0xffffffff
# /* DRAM 70ns - burst read. (offset 8 in upm RAM) */
        .long	0x8fffcc24, 0x0ffbcc04, 0x0cf3cc04, 0x00f3cc0c
        .long	0x03f3cc00, 0x00f3cc44, 0x00f3ec0c, 0x0cf3ec00
        .long	0x00f3ec4c, 0x03f3ec00, 0x00f3ec44, 0x00f3cc00
        .long	0x33f7cc47 
# /* offsets 15-17 not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - single write. (offset 18 in upm RAM) */
        .long	0x8fffcc24, 0x0fefcc04, 0x0cafcc00, 0x11bfcc47
# /* offsets 1c-1f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - burst write. (offset 20 in upm RAM) */
        .long	0x8fffcc24, 0x0fefcc04, 0x0cafcc00, 0x03afcc4c
        .long	0x0cafcc00, 0x03afcc4c, 0x0cafcc00, 0x03afcc4c
        .long	0x0cafcc00, 0x33bfcc47
# /* offsets 2a-2f not used */
        .long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - refresh. (offset 30 in upm RAM) */
        .long	0xe0ffcc84, 0x00ffcc04, 0x00ffcc04,  0x0fffcc04
        .long	0x7fffcc04, 0xffffcc86, 0xffffcc05
# /* offsets 37-3b not used */
	.long	0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - exception. (offset 3c in upm RAM) */
        .long	0x33ffcc07
# /* offset 3d-3f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
UpmTableEdo70End:

#else	/* FALSE/TRUE */

/* UPM initialization table, 60ns, 25MHz */

UpmTable60:
# 
# /* DRAM 60ns - single read. (offset 0 in upm RAM) */
        .long	0x0fffcc04, 0x08ffcc00, 0x33ffcc47
# /* offsets 3-7 not used */
        .long	0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - burst read. (offset 8 in upm RAM) */
        .long	0x0fffcc24, 0x0fffcc04, 0x08ffcc00, 0x03ffcc4c
        .long	0x08ffcc00, 0x03ffcc4c, 0x08ffcc00, 0x03ffcc4c
        .long	0x08ffcc00, 0x33ffcc47
# /* offsets 11-17 not used */
        .long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - single write. (offset 18 in upm RAM) */
        .long	0x0fafcc04, 0x08afcc00, 0x3fbfcc47
# /* offsets 1b-1f not used */
        .long	0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - burst write. (offset 20 in upm RAM) */
        .long	0x0fafcc04, 0x0cafcc00, 0x01afcc4c, 0x0cafcc00
        .long	0x01afcc4c, 0x0cafcc00, 0x01afcc4c, 0x0cafcc00
        .long	0x31bfcc43
# /* offsets 29-2f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - refresh. (offset 30 in upm RAM) */
        .long	0x80ffcc84, 0x13ffcc04, 0xffffcc87, 0xffffcc05
# /* offsets 34-3b not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 60ns - exception. (offset 3c in upm RAM) */
        .long	0x33ffcc07
# /* offset 3d-3f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
UpmTable60End:

/* UPM initialization table, 70ns, 25MHz */

UpmTable70:
# 
# /* DRAM 70ns - single read. (offset 0 in upm RAM) */
        .long	0x0fffec04, 0x08ffec04, 0x00ffec00, 0x3fffcc47
# /* offsets 4-7 not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - burst read. (offset 8 in upm RAM) */
        .long	0x0fffcc24, 0x0fffcc04, 0x08ffcc00, 0x03ffcc4c
        .long	0x08ffcc00, 0x03ffcc4c, 0x08ffcc00, 0x03ffcc4c
        .long	0x08ffcc00, 0x33ffcc47
# /* offsets 12-17 not used */
        .long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - single write. (offset 18 in upm RAM) */
        .long	0x0fafcc04, 0x08afcc00, 0x3fbfcc47
# /* offsets 1b-1f not used */
        .long	0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - burst write. (offset 20 in upm RAM) */
        .long	0x0fafcc04, 0x0cafcc00, 0x01afcc4c, 0x0cafcc00
        .long	0x01afcc4c, 0x0cafcc00, 0x01afcc4c, 0x0cafcc00
        .long	0x31bfcc43
# /* offsets 29-2f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - refresh. (offset 30 in upm RAM) */
        .long	0xc0ffcc84, 0x01ffcc04, 0x7fffcc86, 0xffffcc05
# /* offsets 34-3b not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - exception. (offset 3c in upm RAM) */
        .long	0x33ffcc07
# /* offset 3d-3f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
UpmTable70End:

/* UPM initialization table, 60ns EDO DRAMs, 25MHz */

UpmTableEdo60:
/* DRAM 60ns - single read. (offset 0 in upm RAM) */
        .long	0x0ffbcc04, 0x0cf3cc04, 0x00f3cc00, 0x33f7cc47
/* offsets 6-7 not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 60ns - burst read. (offset 8 in upm RAM) */
        .long	0x0ffbcc04, 0x09f3cc0c, 0x09f3cc0c, 0x09f3cc0c
        .long	0x08f3cc00, 0x3ff7cc47
/* offsets 0e-17 not used */
        .long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 60ns - single write. (offset 18 in upm RAM) */
        .long	0x0fefcc04, 0x08afcc04, 0x00afcc00, 0x3fbfcc47
/* offsets 1c-1f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 60ns - burst write. (offset 20 in upm RAM) */
        .long	0x0fefcc04, 0x08afcc00, 0x07afcc48, 0x08afcc48
        .long	0x08afcc48, 0x39bfcc47
/* offsets 26-2f not used */
        .long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 60ns - refresh. (offset 30 in upm RAM) */
        .long	0x80ffcc84, 0x13ffcc04, 0xffffcc87, 0xffffcc05
/* offsets 34-3b not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 60ns - exception. (offset 3c in upm RAM) */
        .long	0x33ffcc07
/* offset 3d-3f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
UpmTableEdo60End:

/* UPM initialization table, EDO, 70ns DRAM, 25MHz */

UpmTableEdo70:
/* DRAM 70ns - single read. (offset 0 in upm RAM) */
        .long	0x0ffbcc04, 0x0cf3cc04, 0x00f3cc00, 0x33f7cc47
/* offsets 6-7 not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 70ns - burst read. (offset 8 in upm RAM) */
        .long	0x0ffbec04, 0x08f3ec04, 0x03f3ec48, 0x08f3cc00
        .long	0x0ff3cc4c, 0x08f3cc00, 0x0ff3cc4c, 0x08f3cc00
	.long	0x3ff7cc47
/* offsets 11-17 not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 70ns - single write. (offset 18 in upm RAM) */
        .long	0x0fefcc04, 0x08afcc04, 0x00afcc00, 0x3fbfcc47
/* offsets 1c-1f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 70ns - burst write. (offset 20 in upm RAM) */
        .long	0x0fefcc04, 0x08afcc00, 0x07afcc4c, 0x08afcc00
        .long	0x07afcc4c, 0x08afcc00, 0x07afcc4c, 0x08afcc00
	.long	0x37bfcc47
/* offsets 29-2f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 70ns - refresh. (offset 30 in upm RAM) */
        .long	0xc0ffcc84, 0x01ffcc04, 0x7fffcc86, 0xffffcc05
/* offsets 34-3b not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
/* DRAM 70ns - exception. (offset 3c in upm RAM) */
        .long	0x33ffcc07
/* offset 3d-3f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
UpmTableEdo70End:

#endif

#endif  /* ifndef CDCIM */

/* CK */
UpmTable:
# 
# /* DRAM  - single read. (offset 0 in upm RAM) */
        .long	0x1f07fc04, 0xeeaefc04, 0x11adfc04, 0xefbbbc00
        .long	0x1ff77c47, 0x1ff77c35, 0xefeabc34, 0x1fb57c35
# /* DRAM 70ns - burst read. (offset 8 in upm RAM) */
        .long	0x1f07fc04, 0xeeaefc04, 0x10adfc04, 0xf0affc00
        .long	0xf0affc00, 0xf1affc00, 0xefbbbc00, 0x1ff77c47
        .long	0xffffffff, 0xffffffff
# /* offsets 12-17 not used */
        .long	0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM  - single write. (offset 18 in upm RAM) */
        .long	0x1f27fc04, 0xeeaebc00, 0x01b93c04
# /* offsets 1b-1f not used */
        .long	0x1ff77c47
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM  - burst write. (offset 20 in upm RAM) */
        .long	0x1f07fc04, 0xeeaebc00, 0x10ad7c00, 0xf0affc00
        .long	0xf0affc00, 0xe1bbbc04, 0x1ff77c47, 0xffffffff
        .long	0xffffffff
# /* offsets 29-2f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM  - refresh. (offset 30 in upm RAM) */
        .long	0x1ff5fc84, 0xfffffc04, 0xfffffc04, 0xfffffc04
# /* offsets 34-3b not used */
        .long	0xfffffc84, 0xfffffc07, 0xffffffff, 0xffffffff
        .long	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
# /* DRAM 70ns - exception. (offset 3c in upm RAM) */
        .long	0x7ffffc07
# /* offset 3d-3f not used */
        .long	0xffffffff, 0xffffffff, 0xffffffff
UpmTableEnd:
