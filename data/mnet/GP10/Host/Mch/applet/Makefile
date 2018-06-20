##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################

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
#  REVISION HISTORY                            
#__________________________________________________________________
#----------+--------+----------------------------------------------
# Name     |  Date  |  Reason                
#----------+--------+----------------------------------------------
# Bhawani  |08/21/00| Initial Draft
#----------+--------+----------------------------------------------
####################################################################
#/

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..\..

OBJ_DIR   = ../bin

include $(TOP_OF_VOB)\l3defs.mk

SOURCE_LIST = $(wildcard *.java)

OAMROOT =   $(TOP_OF_VOB)\..\THIRD_PARTY\OAMTOOLS
RLIROOT = 	$(OAMROOT)\RLI
JDKROOT = 	$(OAMROOT)\jdk1.1.5
MCHJAR =    ..\..\ViperWatch\Java\mch.jar
JAR	 =    $(JDKROOT)\bin\jar

CABARC = 	$(RLIROOT)\jcit\tools\cabarc\Cabarc.exe
JAVAC = 	$(JDKROOT)\bin\javac.exe

#CLASSPATH = -classpath $(JDKROOT)lib -classpath $(RLIROOT)\jcit

CLASSPATH = .;$(JDKROOT)\lib;$(JDKROOT)\lib\classes.zip;$(RLIROOT)\jcit;$(RLIROOT)\jcit\rlclass.zip;\$(RLIROOT)\jcit\symbeans.jar;$(RLIROOT)\jcit\Jars\Gauges.jar;$(RLIROOT)\jcit\Jars\RLBeans.jar

all: $(MCHJAR)

MCH_SRC= mchControl.java DialogLayout.java ResponseTextArea.java MchCommThread.java
MCH_CLASS = $(MCH_SRC:.java=.class)


$(MCHJAR): $(MCH_CLASS)
	$(JAR) cvf $@  $(MCH_CLASS)

$(MCH_CLASSES):% 

%.class : %.java
	@ $(RM) $(subst \,$(DIRCHAR),$@)
	$(JAVAC) -classpath $(CLASSPATH) $<

clean:
	$(RM) $(subst \,$(DIRCHAR), $(MCH_CLASS))

cleanall:
	$(MAKE) clean
	$(RM) $(subst \,$(DIRCHAR), $(MCHCAB))
