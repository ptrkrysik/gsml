# Makefile - makefile skeleton by Wind River
#
# Copyright (C) 1996 Wind River Systems, Inc.
#
# This file was generated via the Tornado Project Facility.
# It has been modified to work inside the GP10 VOB.
#
############################################################


OBJSUF      = .obj
C6XDIR	    = $(TOP_OF_VOB)\..\THIRD_PARTY\DspTools\c6xtools
C6XDIRV2	= $(TOP_OF_VOB)\..\THIRD_PARTY\DspTools\c6xtools
DSPCC		= $(C6XDIR)\Bin\cl6x
DSPLK		= $(C6XDIR)\Bin\lnk6x 
DSPLKV2	    = $(C6XDIRV2)\Bin\lnk6x

C6XINC	    = $(C6XDIR)\include
MAININC     = ..\include
BBINC		= ..\bbdataproc\include
CODECINC	= ..\bbdataproc\chcodec
DEMINC		= ..\bbdataproc\demod

# This is to make everything relative from this directory

CC_FLAGS = -gqq -pk -pw2 -as -ml3 -o2 -x1 -fr$(OBJDIR) -DCPU=TI_C6201
CC_INCLUDE =  -I$(C6XINC) -I$(MAININC) -I$(BBINC) -I$(CODECINC) -I$(DEMINC)	-I$(TOP_OF_VOB)\Common_To_VOB -I$(TOP_OF_VOB)\..\Common\Include
ASM_FLAGS  = -g -fr..





