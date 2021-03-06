'\" t
.so wrs.an
.\" ads860/target.nr - Motorola MPC8xx(F)ADS target specific documentation
.\"
.\" Copyright 1984-1996 Wind River Systems, Inc.
.\"
.\" modification history
.\" --------------------
.\" 01d,25nov97,map  added info on 860 devices used.
.\" 01c,12sep97,map  info about problems with the "go" command
.\" 01b,24jul96,tpr  added Aux Clock Doc + corrected typo.
.\" 01a,16jun96,tpr  created from mv1604.
.\"
.\"
.TH "ads860" T "Motorola MPC821/860ADS and MPC8xxFADS" "Rev: 17 Jun 96" "VXWORKS REFERENCE MANUAL"

.SH "NAME"
.aX "Motorola MPC821/860ADS & MPC8xxFADS"

.SH "INTRODUCTION"
This note provides board-specific information necessary to run
VxWorks.  Before running VxWorks, verify that the board runs in the
factory configuration.
The ads860 BSP produces a VxWorks image running either on the PowerPC 8xx processors. From the VxWorks perspective these processors are equivalent.

.SH "BOOT ROMS"
No VxWorks Boot ROM is provided with this BSP release. Nevertheless VxWorks
boot code is working if downloaded into the SIMM Flash ROM.

To program the SIMM Flash ROM, the ADI card from Motorola is required.
This debug card is available for Sun workstation or PC. Connect this ADI card
to the MPC821/860ADS board via the ADI port. Check that a 12V potential is
provided to the MPC8xx(F)ADS board; otherwise the SIMM Flash will not be erased
and/or re-programmed. On the host, start the mpc8bug executable (mpc8bug.exe on
PC). Once the prompt appears, reset the board with the following command:
.CS
	821Bug> reset:h
.CE

Then download the bootrom_uncmp.hex file to the Flash:

	821Bug> loadf /<PATH_TO_THE_ADS860_BSP>/bootrom_uncmp.hex 100000

The 100000 value is the address in DRAM memory where the bootrom_uncmp.hex
file is temporarily downloaded before it is moved to Flash. A message
like the following copy should appear:

.CS
	loadf: Loading Srecords file . . .
	Loading flash mapped sections to ram memory buffer:
	Loading block : at 00100000 
		      : 0004efd0 bytes loaded
	Programming flash : 0004efd0 bytes  at 02800100-0284f0cf 
	Flash programming completed 

	Loading ram mapped sections to ram memory:
	Entry point (IP) is not set 
	Heap start address set to 0284f0d0
	821Bug> 
.CE

Now the SIMM Flash memory is programmed with the new boot program. To execute
this new boot program turn the board off and on. If the board is still 
connected to the ADI card then the processor is stopped at the first instruction
to execute. From the mpc8bug prompt call the "go" command to resume FLASH
code execution.
.PP


This board doesn't have non-volatile RAM; thus boot parameters
are not preserved when the system is powered off.
.PP

To load VxWorks, and for more information, follow the instructions in the
"Getting Started"
chapter of the
.I "VxWorks Programmer's Guide."

.PP
An alternative way to load vxWorks is through the ADI debug card. Plug the ADI
connector in the ADI port of the board, power on the board and start the
mpc8bug executable on the host. Execute the reset command to initialize the
board:

.CS
	821Bug> reset:h
.CE

Then download VxWorks:

.CS
	821Bug> load /<ADS860_BSP_PATH>/vxWorks
	Loading ELF file . . .
	Entry point set to 00100000
	Loading section 1 (.text) : 0005fcec bytes at 00100000 
	Loading section 2 (.rodata) : 0000263c bytes at 0015fcec 
	Loading section 4 (.data) : 00001058 bytes at 00162328 
	Loading section 5 (.got) : 00000010 bytes at 00163380 
	Loading section 7 (.sbss) : 00000298 bytes at 00163390 (not loaded)
	Loading section 8 (.bss) : 000079e8 bytes at 00163628 (not loaded)
	Heap start address set to 0016b010
	Loaded 000008da symbols into the symbol table
	Duplicated symbols (-103) expanded with enumerated suffixes 
	r3 and r5 are set to 0
.CE

Then reset the DER register 

.CS
	821Bug> rms DER 0
.CE

And finally start VxWorks execution:

.CS
	821Bug> go 100000
.CE

On some ADI configurations, resuming execution with the "go" could
result in exceptions or other problems. In such cases, unplug the ADI
connector, and reset the target board to start VxWorks.
	
.PP


.SH "DEVICES"
.PP
The chip drivers included are:
    ppc860Timer.c	- Timer library for PPC decrementer, CPM timers 2,3,4
    ppc860Sio.c	- Serial Communications library for SMC UART
    ppc860Intr.c	- Programmable Interrupt Controller Library
    if_cpm.c	- Ethernet Communication library for SCC
.PP

The timer driver, ppc860Timer, implements a system clock using the PPC
decrementer timer, an auxiliary clock using CPM timer 2, and a 32-bit
timestamp facilty by cascading CPM timers 3 & 4. The BSP uses SMC1 as
a UART to implement a console device, while the CPM SCC1 is used as an
ethernet port.

.PP

.SH "SPECIAL CONSIDERATIONS"
.PP
The DRAM controller setup is only performed by the boot program. VxWorks
doesn't re-initialize the DRAM controller when it is executed.
The DRAM memory controller initialization code does not recognized either
the size of the DRAM plugged or the access time or the processor clock
frequency. The only configuration supported is a 4 megabytes DRAM with
a 70 nanosecond access time and with a 25 megahertz processor clock. To
support a different configuration the UpmTable table located in romInit.s
file of the ads860 BSP must be changed.

.PP
The MPC8xx(F)ADS boards do not have a unique Ethernet hardware
address assigned to each board.  A unique address is absolutely necessary if
the user wishes to connect the board to a network.  Thus, the user
must provide a suitable 6 byte Ethernet address for each board used
on a network.  The address is programmed by changing the sysCpmEnetAddr
character array in the file sysLib.c.  The first three bytes (0x08, 0x00, 0x3e)
are a Motorola-specific prefix that should be kept as-is.  The user must
change the last three bytes from 0x03, 0x02, 0x01 to three unique bytes
(i.e., bytes not used by any other Motorola Ethernet connection on your net).
Check with your system administrator if you do not know this information.
If these bytes need changing (they often will not), a new boot ROM
must be burned, and a new image must be built.
.PP
The MC68160 EEST part is very sensitive to its input power voltage (VDD).
VDD to the chip must be between 4.75 and 5.25; values outside this
range may diminish functionality.  Therefore, the power supplied to the
target board and the chip should be checked carefully.  Note that there
may be a significant voltage drop between the power supply connectors
and the chip (the fuses and connectors alone cause approximately a
0.1 volts drop).
In order to get the Ethernet device to work properly the processor clock (PLL)
should be at least 24 megahertz. Both the boot program and VxWorks set
the processor clock to 24 megahertz. Configurations with processors running
at a frequency higher than 24 megahertz or lower than 20 megahertz were not
tested. 
.PP

.SH "SEE ALSO"
.pG "Getting Started, Configuration"


