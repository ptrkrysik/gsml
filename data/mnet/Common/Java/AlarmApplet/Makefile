####################################################################
#                          
#  FILE NAME: Makefile    
#                      
#  DESCRIPTION: This makefile creates the vipercell related class
#               files
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
# Kevin Lim|03/28/00| Initial Draft
#----------+--------+----------------------------------------------
####################################################################
#/

CODEBASE = ..
ALARMARCH = $(CODEBASE)\Java\AlarmStatus.jar

include $(CODEBASE)\java.mk


all:	$(CLASS_LIST) $(ALARMARCH)

$(ALARMARCH):
	$(JAR) cvf $@  *.class com\jetcell\MibWM\*.class

clean:
	-$(RM) *.class

cleanall:
	-$(RM) *.class
	-$(RM) $(ALARMARCH)

