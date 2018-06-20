##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################

SUBDIRS = src
SSLLIBDIR = Common\rsa\lib
SSLLIBFILE = sslc_vxppc.out

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..

include $(TOP_OF_VOB)\l3defs.mk

copyall:
	$(CP) bin\*.out $(TOP_OF_VOB)\bin\*.out
	$(CP) $(TOP_OF_VOB)\..\$(SSLLIBDIR)\$(SSLLIBFILE) $(TOP_OF_VOB)\bin\crypto\sslc_k9.out
