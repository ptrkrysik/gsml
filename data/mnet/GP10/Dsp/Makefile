#****************************************************************************
#   (c) Copyright Cisco 2000
#   All Rights Reserved
#****************************************************************************

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..

SUBDIRS  = asynchctrl bbdataproc Diags peripherals synchctrl shared driver
ENCR_BIN = bin

OBJSUF = .obj
include $(TOP_OF_VOB)\l3defs.mk
include ./dsp.mk

ENCR_A52ONLY = bin\encrypt_a52only
ENCR_A51A52 = bin\encrypt_a51a52

MY_OUTPUT = $(ENCR_A52ONLY)/Dsp \
			$(ENCR_A51A52)/Dsp

all: $(SUBDIRS) $(ENCR_A52ONLY)/Dsp$(OUTSUF) \
				$(ENCR_A51A52)/Dsp$(OUTSUF)

#------ Link with A52 encryption only
$(ENCR_A52ONLY)/Dsp$(OUTSUF): $(wildcard bin/*.obj)
	$(CP) .\encryptA52only.obj .\encryptsubs.obj
	$(DSPLKV2)  -h -m $(ENCR_A52ONLY)/Dsp.map -heap 0x2000 -stack 0x1E00 -c -o \
        $@ -x   -L$(C6XDIRV2)/LIB btsdsp.cmd
	$(CP) $(ENCR_A52ONLY)\Dsp$(OUTSUF) bin\Dsp$(OUTSUF)
	$(CP) $(ENCR_A52ONLY)\Dsp.map bin\Dsp.map

#------ Link with A51 and A52 encryption
$(ENCR_A51A52)/Dsp$(OUTSUF): $(wildcard bin/*.obj)
	$(CP) .\encryptA51A52.obj .\encryptsubs.obj
	$(DSPLKV2)  -h -m $(ENCR_A51A52)/Dsp.map -heap 0x2000 -stack 0x1E00 -c -o \
        $@ -x   -L$(C6XDIRV2)/LIB btsdsp.cmd

copyall:
	$(CP) .\bin\Dsp$(OUTSUF) 				 $(TOP_OF_VOB)\bin\Dsp_k8$(OUTSUF)
	$(CP) .\$(ENCR_A52ONLY)\Dsp$(OUTSUF)	 $(TOP_OF_VOB)\$(ENCR_A52ONLY)\Dsp_k8$(OUTSUF)
	$(CP) .\$(ENCR_A51A52)\Dsp$(OUTSUF)	     $(TOP_OF_VOB)\$(ENCR_A51A52)\Dsp_k9$(OUTSUF)


cleanall:
	- @for %f in ($(SUBDIRS)) do \
		$(MAKE) -C %f cleanall
	-$(RM) bin\Dsp.* \
		$(ENCR_A52ONLY)\Dsp.* \
		$(ENCR_A51A52)\Dsp.* \
        *.obj

