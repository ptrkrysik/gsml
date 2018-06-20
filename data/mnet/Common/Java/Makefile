####################################################################
#                          
#  FILE NAME: Makefile    
#                      
#  DESCRIPTION: This is root make file that genetates java classes
#               and jar file.
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
# Kevin Lim|03/28/00| added AlarmStatusApplet
#----------+--------+----------------------------------------------
####################################################################

CODEBASE = .
include $(CODEBASE)/java.mk
VIPERCELL = $(MIBWMPATH)
VIPERBASE = $(MIBWMPATH)/ViperBase
PACKPATH = $(ALARMAPPLETDIR)\com\jetcell\MibWM

MIBWM = ./Java/MibWM.jar 
ALARMARCH = ./Java/AlarmStatus.jar

all: 
	$(CP) $(MIBWMPATH)\*.java $(PACKPATH)

	$(MAKE) -C $(ALARMAPPLETDIR) all

alarm: $(ALARMARCH)

$(ALARMARCH):	
	$(CP) AlarmApplet\AlarmStatus.jar Java\AlarmStatus.jar 

# mibwm: $(MIBWM)

$(MIBWM): ./Java/AlarmCommThread.class
#	$(JAR) cvf $@  $(VIPERCELL)/*.class $(VIPERBASE)/*.class ./Java/AlarmCommThread.class
	$(JAR) cvf $@  $(VIPERCELL)/*.class  ./Java/AlarmCommThread.class


cleanall:
	$(MAKE) -C $(ALARMAPPLETDIR) cleanall
	$(MAKE) -C $(PACKPATH) cleanall
	$(MAKE) -C $(COMMON) cleanall

	


