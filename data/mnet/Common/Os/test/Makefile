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
TOP_OF_VOB = ..\..\..

# Name of this App's Directory
COMMON_APP_DIR = Os

VOB2DIR    = $(TOP_OF_VOB)\$(VOB)
BINDIR     = ..\bin

ifeq ($(APPDIR),)
	MY_OUTPUT = $(VOB2DIR)\bin\OsTest.out	
else
	MY_OUTPUT = $(OBJDIR)\OsTest.out
endif

include $(VOB2DIR)\l3defs.mk


all: $(MY_OUTPUT)

$(MY_OUTPUT): $(MODULE_OBJS)
	$(LD) -r -o $@.tmp $(MODULE_OBJS)
	$(NM) $@.tmp | munch > _ctdt.c
	$(CC) -traditional $(CC_ARCH_SPEC) -c _ctdt.c
	$(LD) -r -o $@ _ctdt.o $@.tmp 
	$(RM)$(subst /,$(DIRCHAR), _ctdt.c _ctdt.o $@.tmp)	
ifneq ($(APPDIR),)  
	$(CP) $(BINDIR)\*.o $(VOB2DIR)\$(APPDIR)\bin\*.o
endif


cleanall:
	@for %f in ($(notdir $(MODULE_OBJS))) do \
		$(RM) ..\bin\%f

	$(RM) $(MY_OUTPUT)
	

