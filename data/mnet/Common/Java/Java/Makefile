####################################################################
#                          
#  FILE NAME: Makefile    
#                      
#  DESCRIPTION: This makefile creates the common class files and the
#               common jar file.
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
# Bhawani  |11/28/99| Initial Draft
# Kevin Lim|03/28/00| add AlarmStatus.jar
#----------+--------+----------------------------------------------
####################################################################
#/

CODEBASE = ..
include $(CODEBASE)/java.mk

MIBWMPATH = $(CODEBASE)/com/jetcell/MibWM
VIPERCELL = $(MIBWMPATH)
VIPERBASE = $(MIBWMPATH)/ViperBase
ALARMPATH = $(CODEBASE)/AlarmApplet

MIBWM = ./MibWM.jar 
ALARMARCH = ./AlarmStatus.jar


VIPERCELL_JAVA 	= $(wildcard $(VIPERCELL)/*.java)
VIPERCELL_CLASS	= $(subst /,/, $(VIPERCELL_JAVA:.java=.class))

VIPERBASE_JAVA  = GudbData.java EncodeMessage.java \
		  Configuration.java RoamingNumberTable.java 

VIPERBASE_TMP =  $(VIPERBASE_JAVA:.java=.class)
VIPERBASE_CLASS = $(foreach file, $(VIPERBASE_TMP), $(VIPERBASE)/$(file)) 

all: $(MIBWM) $(CLASS_LIST) $(ALARMARCH)

 
$(MIBWM): $(VIPERCELL_CLASS) $(VIPERBASE_CLASS)
	@$(RM)  $@
	$(MAKE) -C $(CODEBASE) mibwm

$(ALARMARCH): $(ALARMPATH)/*.class
	@$(RM)	$@
	$(MAKE)	-C $(CODEBASE) alarm

viperCell:
	$(MAKE) -C $(VIPERCELL) all

viperBase:
	$(MAKE) -C $(VIPERBASE) all

cleanall:
	-$(RM) $(subst /,\, $(VIPERBASE)/*.class)
	-$(RM) $(subst /,\, $(MIBWM) $(CLASS_LIST))



