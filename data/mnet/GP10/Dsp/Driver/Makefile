#****************************************************************************
#   (c) Copyright Cisco 2000
#   All Rights Reserved
#****************************************************************************

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..

include $(TOP_OF_VOB)\l3defs.mk
include ../dsp.mk

SOURCE_LIST = Drv6x.c

OBJ_LIST       	=  $(SOURCE_LIST:.c=.obj)
MODULE_OBJS	= $(foreach file, $(OBJ_LIST), $(OBJDIR)/$(file))

all: $(MODULE_OBJS)

# rule for C++ compiles
$(OBJDIR)/%.obj : %.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

