# Makefile - makefile skeleton by Wind River
#
# Copyright (C) 1996 Wind River Systems, Inc.
#
# This file was generated via the Tornado Project Facility.
#
CPU    = PPC860
TOOL   = gnu
OUTSUF = .out
MD     = mkdir
MK     = make
CD     = cd
MV     = move

# Enables .o files being used from the common vob to be delivered to this VOB
ifeq ($(APPDIR),)
	OBJDIR = ..\bin
else
	OBJDIR = $(VOB2DIR)\$(APPDIR)\bin
endif

VOBNAME = GP10

VOB_COMMON_DIR   = $(TOP_OF_VOB)\..\$(VOBNAME)\Common_To_Vob
HOST_COMMON_DIR  = $(TOP_OF_VOB)\..\$(VOBNAME)\Host\Common_To_Host

COMMON_VOB_APP_DIR = $(TOP_OF_VOB)\..\Common
COMMON_VOB_DIR     = $(TOP_OF_VOB)\..\Common\include
APP_INCLUDE_DIR    = ../include

ifeq ($(OBJSUF),)
	OBJSUF = .o
endif
export DEP

# following line added by SY 1999-03-15
ADDED_C++FLAGS = -D__PROTOTYPE_5_0

include $(WIND_BASE)/target/h/make/defs.bsp
include $(WIND_BASE)/target/h/make/make.$(CPU)$(TOOL)
include $(WIND_BASE)/target/h/make/defs.$(WIND_HOST_TYPE)
include $(WIND_BASE)/target/h/make/rules.bsp

SOURCE_LIST_CPP = $(wildcard *.cpp)
OBJ_LIST_CPP    =  $(SOURCE_LIST_CPP:.cpp=.o)

SOURCE_LIST_C   = $(wildcard *.c)
OBJ_LIST_C      = $(SOURCE_LIST_C:.c=.o)

SOURCE_LIST     = $(SOURCE_LIST_CPP) $(SOURCE_LIST_C)
OBJ_LIST        = $(OBJ_LIST_CPP) $(OBJ_LIST_C)
MODULE_OBJS	= 	$(foreach file, $(OBJ_LIST), $(OBJDIR)/$(file))

# This is to make everyting relative from this directory

# Included optimization parameters
CC_INCLUDE += -I$(COMMON_VOB_APP_DIR)\$(THIS_APP_DIR)\include
CC_INCLUDE += -I\$(VOBNAME)\Host\$(THIS_APP_DIR)\include -I$(VOB_COMMON_DIR)
CC_INCLUDE += -I$(HOST_COMMON_DIR) -I$(HOST_COMMON_DIR)\include	
CC_INCLUDE += -I$(APP_INCLUDE_DIR)
CC_INCLUDE += -I$(COMMON_VOB_APP_DIR) -I$(COMMON_VOB_DIR) -I$(WIND_BASE)\target\h\snmp
CC_INCLUDE += -DH323_Exist -D__VXWORKS__ -DVX_IGNORE_GNU_LIBS -DVXWORKS
CC_INCLUDE += -O2 -DVIPER_IP -DFAST_RTP_READ -DMNET_GP10

# Can be added in the make command line
C++FLAGS += $(EXTRA)

# to workaround forward slash problem to get the target "clean" to work
RM_MODULE_OBJS = $(subst /,$(DIRCHAR), $(MODULE_OBJS))

# rule for recursive make
SUBDIR_TARGETS = $(foreach dir, $(SUBDIRS), $(dir).sub)
CLEAN_TARGETS  = $(foreach dir, $(SUBDIRS), $(dir).clean)
COPY_TARGETS   = $(foreach dir, $(SUBDIRS), $(dir).copy)
OUTPUT_TARGETS = $(foreach dir, $(SUBDIRS), $(dir).output)

all:	$(SUBDIR_TARGETS)

%.sub:	%
	@ $(MAKE) -C $< all

# rule for C++ compiles
$(OBJDIR)/%.o : %.cpp
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(CXX) $(C++FLAGS) -c -o $@ $<

# rule for C compiles
$(OBJDIR)/%.o : %.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(CXX) $(C++FLAGS) -c -o $@ $<

ifeq ($(TO),)
TO = bin
endif

# Copy All Command
copyall:
override OUTLIST = $(wildcard bin/*$(OUTSUF))
ifneq ($(OUTLIST), )
	@$(CP) bin\*$(OUTSUF) $(TO)
else
ifneq ($(findstring 860, $(wildcard *)), 860)
	echo ($(wildcard bin/*), )
ifneq ($(SUBDIRS),)
	for %f in ($(SUBDIRS)) do \
		$(MAKE) -C %f copyall
endif
endif
endif


# cleanall

cleanall:
ifeq ($(SUBDIRS),)
	-$(RM) $(RM_MODULE_OBJS)
	-$(RM) $(subst /,$(DIRCHAR), $(MY_OUTPUT))
else
	- @for %f in ($(SUBDIRS)) do \
		$(MAKE) -C %f cleanall
endif


# Create Output directories
output:
	- $(MD) bin
	- @for %f in ($(SUBDIRS)) do \
	 $(MD) %f\$(Bin)


depend.$(CPU)$(TOOL):
ifeq ($(SUBDIRS),)
	@ echo Creating depend.$(CPU)$(TOOL)
ifneq ($(SOURCE_LIST),)
		@ for %f in ($(SOURCE_LIST)) do 				\
	    	$(CXX)  -MM $(C++FLAGS) -c %f  \
	    	| $(TCL) $(PROJECT)/depend.tcl $(TGT_DIR) 1>>$@
else
ifneq ($(wildcard *.c),)
			@ for %f in ($(wildcard *.c)) do 				\
	    		$(CXX)  -MM $(C++FLAGS) -c %f  \
	    		| $(TCL) $(PROJECT)/depend.tcl $(TGT_DIR) 1>>$@
endif
ifneq ($(wildcard *.cpp),)
			@ for %f in ($(wildcard *.cpp)) do \
	    		$(CXX) -MM  $(C++FLAGS) %f \
	    		| $(TCL) $(PROJECT)/depend.tcl $(TGT_DIR) 1>>$@
endif
endif
	@echo Created depend file
else
	@for %f in ($(SUBDIRS)) do \
	$(MAKE) -C %f depend
endif

# Force to regenerate dependencies
depend:
	$(RM) depend.$(CPU)$(TOOL)
	$(MAKE) depend.$(CPU)$(TOOL)


# Add include depend.$(CPU)$(TOOL) to make dependencies.
ifeq ($(DEP), YES)
ifeq ($(SUBDIRS),)
-include depend.$(CPU)$(TOOL)
endif
endif


$(OBJDIR):
	mkdir $(subst /,$(DIRCHAR), $@)





