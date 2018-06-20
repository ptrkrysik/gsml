##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..\..

# These Must be Properly Defined
THIS_APP_DIR   = Mch
THIS_DIRECTORY = post
MY_OUTPUT = $(OBJDIR)\post.out

# Name(s) of Common VOB directories to include
COMMON_BLD_DIR = 

POST_C_SRCS  = 	
POST_C++SRCS = pontest.cpp	
POST_A_SRCS  =	
POST_LIST    = $(POST_C_SRCS:.c=.o)    \
               $(POST_C++SRCS:.cpp=.o) \
               $(POST_A_SRCS:.s=.o)
               
POST_OBJS     = $(foreach file, $(POST_LIST), $(OBJDIR)/$(file))               

include $(TOP_OF_VOB)\l3defs.mk

all: makeCommonObjs $(MY_OUTPUT)

# Adds the .o file(s) list needed from the Common VOB
makeCommonObjs:	
ifneq ($(COMMON_BLD_DIR),)
	@for %f in ($(COMMON_BLD_DIR)) do \
	make -C $(COMMON_VOB_APP_DIR)\%f \
	all VOB=$(VOBNAME) APPDIR=Host\$(THIS_APP_DIR)\$(THIS_DIRECTORY)
endif

# If Common VOB directories to include get the .o files from bin
$(MY_OUTPUT): $(POST_OBJS)
ifneq ($(COMMON_BLD_DIR),)
	$(LD) -r -o $@.tmp $() $(POST_OBJS) $(wildcard ./bin/*.o)
else
	$(LD) -r -o $@.tmp $() $(POST_OBJS)
endif
	$(NM) $@.tmp | munch > _ctdt.c
	$(CC) -traditional $(CC_ARCH_SPEC) -c _ctdt.c
	$(LD) -r -o $@ _ctdt.o $@.tmp 
	$(RM)$(subst /,$(DIRCHAR), _ctdt.c _ctdt.o $@.tmp)

cleanall:
	@for %f in ($(notdir $(MODULE_OBJS))) do \
		$(RM) ..\bin\%f

	$(RM) $(MY_OUTPUT)

ifneq ($(COMMON_BLD_DIR),)
	$(RM) bin\*.o 
	$(RM) bin\*.out
endif