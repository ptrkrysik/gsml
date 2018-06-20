##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################

SUBDIRS= src

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..

include $(TOP_OF_VOB)\l3defs.mk

CC_INCLUDE += -Isnmp\include

copyall:
	$(CP) bin\*.out $(TOP_OF_VOB)\bin\*.out