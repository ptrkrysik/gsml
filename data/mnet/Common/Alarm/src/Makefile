
##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
#  Use Examples:
#
#	Case 1:
#      make all VOB=GP10 -
#	   Places .out in VOB/bin directory
#
#   Case 2:
#      make all VOB=GP10 APPDIR=Host\<Application Name>\<Source Directory> -
#	   Places .o file(s) in VOB\$(APPDIR)\bin directory.
#
#      <Application Name> = Name of Application directory
#	   <Source Directory> = application sub directory where the calling
#                           Makefile resides.
#
#      Example: make all VOB=GP10 APPDIR=Host\vxTemplate\src
#
#
#  Note: This make file must reference a VOB that
#	     has a defs.mk in the top level directory.
#
##########################################################

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..

# Name of this App's Directory
THIS_APP_DIR   = Alarm

VOB2DIR    = $(TOP_OF_VOB)\..\$(VOB)
BINDIR     = ..\bin

include $(VOB2DIR)\l3defs.mk

all: $(MODULE_OBJS)

cleanall:
	@for %f in ($(notdir $(MODULE_OBJS))) do \
		$(RM) ..\bin\%f
