##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################


Note:  The file sys_const.h is actually a soft link to 
	   jcc\include\sys_const.h

If you need to make another soft link, here is the syntax:

From a DOS window in this directory:
cleartool ln -s <"Path to Top of view"\"directory"\filename  filename

Example:

cleartool ln -s ..\..\..\..\GP10\Host\Oam\include\oam_api.h oam_api.h

Note: The path is case sensitive !!