##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################
# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..

#EXT  = arppc

# These Must be Properly Defined
THIS_APP_DIR   = vxsh
THIS_DIRECTORY = Src

ifeq ($(APPDIR),)
	MY_OUTPUT = $(VOB2DIR)\bin\vxsh.out
else
	MY_OUTPUT = $(OBJDIR)\vxsh.out
endif


VOB2DIR    = $(TOP_OF_VOB)\..\$(VOB)
BINDIR     = ..\bin

#LIBNAME		= libppc860gnuvx.a
LIBNAME		= lib$(CPU)$(TOOL)vx.a

# Name(s) of Common VOB directories to include
COMMON_BLD_DIR =

include $(VOB2DIR)\l3defs.mk
CC_INCLUDE += -I$(VOB2DIR)\Host

all:  makeCommonObjs $(MY_OUTPUT)

# Adds the .o file(s) list needed from the Common VOB
makeCommonObjs:	
ifneq ($(COMMON_BLD_DIR),)
	@for %f in ($(COMMON_BLD_DIR)) do \
	make -C $(COMMON_VOB_APP_DIR)\%f \
	all VOB=$(VOBNAME) APPDIR=Host\$(THIS_APP_DIR)\$(THIS_DIRECTORY)
endif

# If Common VOB directories to include get the .o files from bin
$(MY_OUTPUT): $(MODULE_OBJS)

	@ echo Copying $(LIBNAME).copy to $(LIBNAME)
	$(CP) $(WIND_BASE)\target\lib\$(LIBNAME).copy  $(OBJDIR)\$(LIBNAME)
	attrib -R $(OBJDIR)\$(LIBNAME)
 
	$(AR)  -x $(OBJDIR)\$(LIBNAME) shell.o
	$(AR)  -x $(OBJDIR)\$(LIBNAME) shellLib.o
	$(AR)  -x $(OBJDIR)\$(LIBNAME) ledLib.o
	
	$(AR) -dv $(WIND_BASE)\$(LIBNAME) shellLib.o
	$(AR) -dv $(WIND_BASE)\$(LIBNAME) shell.o
	$(AR) -dv $(WIND_BASE)\$(LIBNAME) ledLib.o

ifneq ($(COMMON_BLD_DIR),)
	$(LD) -r -o $@.tmp $() $(MODULE_OBJS) $(wildcard ./bin/*.o) shell.o shellLib.o ledLib.o
else
	$(LD) -r -o $@.tmp $() $(MODULE_OBJS) shell.o shellLib.o ledLib.o
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
	$(RM) src\*.o
	$(RM) bin\*.o 
	$(RM) bin\*.out
endif

# Create libPENTIUM
library: $(MODULE_OBJS)
	@ echo Copying $(LIBNAME).copy to $(LIBNAME)
	$(CP) $(WIND_BASE)\target\lib\$(LIBNAME).copy  $(WIND_BASE)\target\lib\$(LIBNAME)
	$(CP) $(WIND_BASE)\target\lib\$(LIBNAME)  $(OBJDIR)\$(LIBNAME)
	$(AR) -dv $(WIND_BASE)\target\lib\$(LIBNAME) shellLib.o
	$(AR) -dv $(WIND_BASE)\target\lib\$(LIBNAME) shell.o
	$(AR) -dv $(WIND_BASE)\target\lib\$(LIBNAME) ledLib.o
	attrib -R $(OBJDIR)\$(LIBNAME)



extract : $(MODULE_OBJS)
	$(AR)  -x $(OBJDIR)\$(LIBNAME) shell.o
	$(AR)  -x $(OBJDIR)\$(LIBNAME) shellLib.o
	$(AR)  -x $(OBJDIR)\$(LIBNAME) ledLib.o


    
	

