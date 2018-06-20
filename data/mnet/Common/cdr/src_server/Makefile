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
THIS_APP_DIR   = cdrserver

VOB2DIR    = $(TOP_OF_VOB)\..\$(VOB)
BINDIR     = ..\bin

ifeq ($(APPDIR),)
	MY_OUTPUT = $(VOB2DIR)\bin\cdr_common_src_server.out
else
	MY_OUTPUT = $(OBJDIR)\cdr_common_src_server.out
endif

include $(VOB2DIR)\l3defs.mk
CC_INCLUDE += -I$(COMMON_VOB_DIR)/os

all: $(MY_OUTPUT)

$(MY_OUTPUT): $(MODULE_OBJS)
	$(LD) -r -o $@.tmp $(MODULE_OBJS)
	$(NM) $@.tmp | munch > _ctdt.c
	$(CC) -traditional $(CC_ARCH_SPEC) -c _ctdt.c
	$(LD) -r -o $@ _ctdt.o $@.tmp
	$(RM)$(subst /,$(DIRCHAR), _ctdt.c _ctdt.o $@.tmp)
	

cleanall:
	@for %f in ($(notdir $(MODULE_OBJS))) do \
		$(RM) ..\bin\%f

	$(RM) $(MY_OUTPUT)


