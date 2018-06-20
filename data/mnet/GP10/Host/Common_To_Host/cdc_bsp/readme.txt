

Note:  	All of the files in this directory are soft links. To check out 
		any of these files go to the GP10/Host/Cdc_bsp directory.


If you need to make another soft link, here is the syntax:

From a DOS window in this directory:
cleartool ln -s <"Path to Top of view"\"directory"\filename  filename

Example:

cleartool ln -s ..\..\..\..\GP10\Host\Oam\include\oam_api.h oam_api.h

Note: The path is case sensitive !!