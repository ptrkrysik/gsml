# JetCell modification history
# ----------------------------
# 00/08/09  ck   Add susUser.o in MACH_EXTRA
# 99/09/11  ck   Add usrFsLib.o in MACH_EXTRA
# 99/03/22  ck   Add JetCell modification history
# 99/02/18  ck   Changes for CDCIM.
# 99/02/18  ck   Initial revision. Copied from ads860 bsp.

# Makefile - makefile for bsp/config/ads860
#
# Copyright 1984-1996 Wind River Systems, Inc.
#
# modification history
# --------------------
# 01d,06nov96,tpr  replaced RAM_LOW_ADRS from 00100000 to 00010000.
#		   + remove RELEASE.
# 01c,25jun96,tpr  removed vxWorks.st of the release rule.
# 01b,20jun96,tpr  added TGT_DIR and replaced $(WIND_BASE) by $(TGT_DIR).
# 01a,16apr96,tpr  written from bsp/ultra603/Makefile 01c version.
#
# DESCRIPTION
# This file contains rules for building VxWorks for the
# Motorola Application Development Support Board with a PowerPC 860 or 
# PowerPC 821 processor.
#
# INCLUDES
#     makeTarget
#*/
TOP_OF_VOB = ..\..\..

CPU              = PPC860
TOOL             = gnu

TGT_DIR = $(WIND_BASE)/target
CLI_DIR = $(TOP_OF_VOB)\Common\cli\bin
CLI_SRC = $(TOP_OF_VOB)\Common\cli

VXSH_DIR= $(TOP_OF_VOB)\Common\vxsh

include $(TGT_DIR)/h/make/defs.bsp
include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.


TARGET_DIR	= ads860
VENDOR		= Motorola
BOARD		= MPC860ads

## The constants ROM_TEXT_ADRS, ROM_SIZE, and RAM_HIGH_ADRS are defined
## in config.h and Makefile. All definitions for these constants must be
## identical.

ROM_TEXT_ADRS    = 20000100 # ROM entry address
ROM_SIZE         = 00400000 # number of bytes of ROM space

RAM_LOW_ADRS     = 00010000 # RAM text/data address
RAM_HIGH_ADRS    = 00200000 # RAM text/data address
#RAM_HIGH_ADRS    = 00000100 # RAM text/data address

HEX_FLAGS        = -a $(ROM_TEXT_ADRS)

CLI_EXTRA     = $(CLI_DIR)\chain.o \
        $(CLI_DIR)\parser.o \
        $(CLI_DIR)\cli_parser_init.o \
        $(CLI_DIR)\command2.o \
        $(CLI_DIR)\debug.o \
        $(CLI_DIR)\imc.o \
        $(CLI_DIR)\imsg.o \
        $(CLI_DIR)\command_chain.o\
        $(CLI_DIR)\exec.o\
        $(CLI_DIR)\exec_chain.o\
        $(CLI_DIR)\parser_actions.o \
        $(CLI_DIR)\parser_input.o \
        $(CLI_DIR)\parser_modes.o \
        $(CLI_DIR)\parser_print.o \
        $(CLI_DIR)\parser_printf.o \
        $(CLI_DIR)\parser_privilege.o \
        $(CLI_DIR)\parser_util.o \
        $(CLI_DIR)\queue.o \
        $(CLI_DIR)\cfg_chain.o\
        $(CLI_DIR)\basic_exec_chain.o\
        $(CLI_DIR)\show_chain.o\
        $(CLI_DIR)\sys_debug.o \
        $(CLI_DIR)\cli_shellLib.o\
        $(CLI_DIR)\usrScript.o\
        $(CLI_DIR)\util.o\
        $(CLI_DIR)\ip_address.o\
                


ifndef BOOT
MACH_EXTRA       = motFecEnd.o $(CLI_EXTRA) $(TGT_DIR)/lib/objPPC860gnuvx/usrFsLib.o sysUser.o
else
MACH_EXTRA       = motFecEnd.o 
endif

ORGLIB = $(WIND_BASE)/target/lib/libPPC860gnuvx.a

## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.

include $(TGT_DIR)/h/make/rules.bsp
include $(TGT_DIR)/h/make/rules.$(WIND_HOST_TYPE)
CC_INCLUDE += -DVIPER_IP
all:  library parser vxWorks

copyall:
	@$(CP) vxWorks.* ..\..\bin

cleanall:
	$(MAKE) -C ../snmp cleanall
	$(MAKE) -C $(CLI_SRC) VOB=GP10 cleanall
	$(MAKE) clean



# vxWorks depends on library and that needs to be created this way.
library:
	$(MAKE) -C ../snmp all
	
vxWorks: library

vxsh :
	$(MAKE) -C $(VXSH_DIR) VOB=GP10 all

parser :
	$(MAKE) -C $(CLI_SRC) VOB=GP10 all
