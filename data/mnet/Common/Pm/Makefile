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
#	   (OPTIONAL) Append REPLACE - Replaces the List of directories
#			      to be used in the SUBDIRS variable
#
#	   Example: make all VOB=GP10 APPDIR=Host\vxTemplate\src REPLACE="src_api src_debug"
#
#  Note: This make file must reference a VOB that
#	     has a defs.mk in the top level directory.
#
##########################################################

SUBDIRS = src

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..

VOBDIR =  $(TOP_OF_VOB)/../$(VOB)

# Allows a sub-set of the source code directories to be used

ifneq ($(REPLACE),)
	SUBDIRS = $(REPLACE)
endif

include $(VOBDIR)\l3defs.mk


