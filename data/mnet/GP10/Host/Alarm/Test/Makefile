##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..\..

include $(TOP_OF_VOB)\l3defs.mk

INC_DIR = ../include
MY_OUT = $(OBJDIR)\alarmTest.out
MY_OBJ = $(MY_OUT:.out=.o)
C++FLAGS += -g

all:	$(MY_OUT)
	@echo done

$(MY_OUT): $(MODULE_OBJS)
		$(LD) -r -o  $(MY_OBJ) $(MODULE_OBJS)
		$(NM) $(MY_OBJ)(.out:.o)| munch > _ctdt.c
		$(CC) -traditional $(CC_ARCH_SPEC) -c _ctdt.c
		$(LD) -r -o $(MY_OUT) _ctdt.o $(MY_OBJ)
		$(RM)	_ctdt.c _ctdt.o $(MY_OBJ)
		$(MV) ..\bin\*.o ..\bin\objs\

$(MODULE_OBJS):$(wildcard $(INC_DIR)/*.h) Makefile