##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..\..

include $(TOP_OF_VOB)\l3defs.mk

CC_INCLUDE  += -I$(WIND_BASE)/target/h/snmp -DVXWORKS -DUT

all: 	oam_test

oam_test: $(MODULE_OBJS)
	 $(LD) -r -o $(OBJDIR)/$@.tmp $(MODULE_OBJS)
	 $(NM) $(OBJDIR)/$@.tmp | munch > _ctdt.c
	 $(CC) -traditional $(CC_ARCH_SPEC) -c _ctdt.c
	 $(LD) -r -o $(OBJDIR)/$@.out _ctdt.o $(OBJDIR)/$@.tmp
	 $(RM)	_ctdt.c _ctdt.o $(OBJDIR)\$@.tmp