##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
#  Use Examples: 
#
#	Case 1: make all VOB=GP10 - 
#	        Places .out in VOB/bin directory
#
#   Case 2: make all VOB=GP10 APPDIR=Host\vxTemplate\bin - 
#		    Places .o file(s) in VOB/$(APPDIR) directory.
#
#  Note: This make file must reference a VOB that
#	     has a defs.mk in the top level directory.
#
##########################################################

SUBDIRS = src loadmodule

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..

VOBDIR =  $(TOP_OF_VOB)\$(VOB)

include $(VOBDIR)\l3defs.mk


