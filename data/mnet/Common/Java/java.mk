####################################################################
#                          
#  FILE NAME: Java.mk    
#                      
#  DESCRIPTION: This file needs to be included by every makefile that
#               compiles Java source files.
#
#      COMPONENTS:                      
#                              
#                                  
#  NOTES:                              
#                                  
#  (C) Copyright 1999 JetCell Inc., CA, USA http://www.jetcell.com/
#                                 
#  REVISION HISTORY                            
#__________________________________________________________________
#----------+--------+----------------------------------------------
# Name     |  Date  |  Reason                
#----------+--------+----------------------------------------------
# Bhawani  |11/28/99| Iniitial Draft
#----------+--------+----------------------------------------------
####################################################################
#/

# Hard-coded path should point to the network drive
RLROOT    =	..\..\..\THIRD_PARTY\OamTools
RLROOT2   =	..\..\THIRD_PARTY\OamTools
RLIROOT   =	$(RLROOT)\rl
RLIROOT2  =	$(RLROOT2)\rl
JDKROOT = $(CODEBASE)\..\..\THIRD_PARTY\OamTools\jdk1.3

RM	= del
CP	= copy

MIBWMPATH = $(CODEBASE)\com\jetcell\MibWM
VIPERCELL = $(MIBWMPATH)
VIPERBASE = $(MIBWMPATH)\ViperBase
COMMON    = $(CODEBASE)\Java
OUTPUTDIR    = $(CODEBASE)\..\..\GMC\GMCManager
ALARMAPPLETDIR = $(CODEBASE)\AlarmApplet

ifeq ($(SOURCE_LIST),)
	 SOURCE_LIST     	= $(wildcard *.java)
	
endif

CLASS_LIST      =  $(SOURCE_LIST:.java=.class)

# java compiler and archive maker
JAR	    = $(JDKROOT)\bin\jar.exe
JAVAC   = $(JDKROOT)\bin\javac.exe

CLASSPATH = .;$(ALARMAPPLETDIR);$(CODEBASE);$(JDKROOT)/lib;$(JDKROOT)/lib/classes.zip;$(RLROOT);/$(RLIROOT)/jcit/symbeans.jar;$(RLIROOT)/jcit/Jars/Gauges.jar;$(RLIROOT)/jcit/Jars/RLBeans.jar


# rule to compile Java source files
%.class : %.java
#	- @ $(RM) $(subst /,$(DIRCHAR),$@)
	$(JAVAC)  -classpath $(CLASSPATH) $(subst /,\, $<)

