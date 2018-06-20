##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..\..

# These Must be Properly Defined
THIS_APP_DIR   = socket
THIS_DIRECTORY = src
MY_OUTPUT = $(OBJDIR)\soc.out

# Name(s) of Common VOB directories to include
COMMON_BLD_DIR = $(THIS_APP_DIR)

REPLACE_LIST="src src_ssl"

include $(TOP_OF_VOB)\l3defs.mk

all: makeCommonObjs $(MY_OUTPUT)

# Adds the .o file(s) list needed from the Common VOB
makeCommonObjs:
ifneq ($(COMMON_BLD_DIR),)
	@for %f in ($(COMMON_BLD_DIR)) do \
	make -C $(COMMON_VOB_APP_DIR)\%f \
	all VOB=$(VOBNAME) APPDIR=Host\$(THIS_APP_DIR)\$(THIS_DIRECTORY) REPLACE=$(REPLACE_LIST)
endif

# If Common VOB directories to include get the .o files from bin
$(MY_OUTPUT): $(MODULE_OBJS)
ifneq ($(COMMON_BLD_DIR),)
	$(LD) -r -o $@.tmp $() $(MODULE_OBJS) $(wildcard ./bin/*.o)
else
	$(LD) -r -o $@.tmp $() $(MODULE_OBJS)
endif
	$(NM) $@.tmp | munch > _ctdt.c
	$(CC) -traditional $(CC_ARCH_SPEC) -c _ctdt.c
	$(LD) -r -o $@ _ctdt.o $@.tmp
	$(RM)$(subst /,$(DIRCHAR), _ctdt.c _ctdt.o $@.tmp)

cleanall:
	$(RM) ..\bin\*.o
	$(RM) ..\bin\*.out

ifneq ($(COMMON_BLD_DIR),)
	$(RM) .\bin\*.o
	$(RM) .\bin\*.out
endif