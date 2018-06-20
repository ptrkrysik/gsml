##########################################################
#
#  (c) Copyright Cisco 2000
#  All Rights Reserved
#
##########################################################


SUBDIRS = Dsp Host

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = .

include $(TOP_OF_VOB)\l3defs.mk

GP10_DIR       = GP10
GP10_CRYPTO_DIR= GP10_CRYPTO
VIPERBASE_DIR  = ViperBase
BIN_DIR        = bin
BIN_CRYPTO_DIR = bin\crypto
VX_DIR         = vx
VIPERWATCH_DIR = GPManager
VBWATCH_DIR    = VBWatch
VIPERWATCH_SRC_DIR = Host\ViperWatch
SCRIPT_DIR     = Host\scripts
MIB_DIR        = Host\mib
FPGA_DIR       = Host\fpga
VXBOOT_DIR     = Host\cdc_bsp
VXBOOT_DST_DIR = vxBoot
ENCRYPTION     = GSM_Encryption

GP10_ROOT        = $(TO)\$(GP10_DIR)
GP10_CRYPTO_ROOT = $(TO)\$(GP10_CRYPTO_DIR)
VIPER_BASE_ROOT  = $(TOP_OF_VOB)\..\$(VIPERBASE_DIR)
HOST_DIR         = Host

DST_BIN_DIR        = $(GP10_ROOT)\$(BIN_DIR)
DST_VX_DIR         = $(GP10_ROOT)\$(VX_DIR)
DST_VIPERWATCH_DIR = $(GP10_ROOT)\$(VIPERWATCH_DIR)
DST_SCRIPT_DIR     = $(GP10_ROOT)\scripts
DST_FPGA_DIR       = $(GP10_ROOT)\fpga
DST_MIB_DIR        = $(GP10_ROOT)\mib
DST_ALARM_DIR      = $(GP10_ROOT)\alarm
DST_VXBOOT_DIR     = $(GP10_ROOT)\$(VXBOOT_DST_DIR)

DST_BIN_CRYPTO_DIR    = $(GP10_CRYPTO_ROOT)\$(BIN_DIR)
DST_SCRIPT_CRYPTO_DIR = $(GP10_CRYPTO_ROOT)\scripts
DST_ENCRYPTION        = $(GP10_CRYPTO_ROOT)\$(ENCRYPTION)
DST_MIB_CRYPTO_DIR    = $(GP10_CRYPTO_ROOT)\mib

SRC_VBWATCH_DIR = $(VIPER_BASE_ROOT)\$(VBWATCH_DIR)
SRC_CERT_DIR    = $(COMMON_VOB_APP_DIR)\cdr\certificate

MV    = move
CP    = copy
XCP   = xcopy
MD    = mkdir
NMAKE = nmake

all: buildcommon

# Add each of the Common VOB directories that produce a .out file
# Example:  vxTemplate

COMMON_DIRS = Os ssh

buildcommon:
	-@for %f in ($(COMMON_DIRS)) do \
		$(MAKE) -C $(COMMON_VOB_APP_DIR)\%f all VOB=GP10

cleanall:
	$(RM) $(BIN_DIR)\*.out
	$(RM) $(BIN_DIR)\*.map
	$(RM) $(BIN_DIR)\vxWorks.*
	$(RM) $(BIN_CRYPTO_DIR)\*.out
	$(RM) $(BIN_CRYPTO_DIR)\*.map

	- @for %f in ($(COMMON_DIRS)) do \
		$(MAKE) -C $(COMMON_VOB_APP_DIR)\%f cleanall VOB=GP10
	- @for %f in ($(SUBDIRS)) do \
		$(MAKE) -C %f cleanall

copyall:
	- @for %f in ($(SUBDIRS)) do \
		$(MAKE) -C %f copyall


pack:
	@echo  Copying binaries
	-$(MD) $(DST_BIN_DIR)
	-$(MD) $(DST_VX_DIR)
	@$(CP) $(BIN_DIR)\*.out $(DST_BIN_DIR)\*.out

	@echo  Copying vxWorks Symbol file
	@$(CP) $(BIN_DIR)\vxWorks.* $(DST_VX_DIR)\

	@echo Copying ViperWatch html files
	-$(MD) $(DST_VIPERWATCH_DIR)
	@$(CP) $(VIPERWATCH_SRC_DIR)\*.*  $(DST_VIPERWATCH_DIR)\

	@echo Copying ViperWatch text files
	-$(MD) $(DST_VIPERWATCH_DIR)\Text
	@$(CP) $(VIPERWATCH_SRC_DIR)\Text\*.*  $(DST_VIPERWATCH_DIR)\Text\

	@echo Copying ViperWatch Images files
	-$(MD) $(DST_VIPERWATCH_DIR)\Images
	@$(CP) $(VIPERWATCH_SRC_DIR)\Images\*.*  $(DST_VIPERWATCH_DIR)\Images\

	@echo Copying ViperWatch js files
	-$(MD) $(DST_VIPERWATCH_DIR)\js
	@$(CP) $(VIPERWATCH_SRC_DIR)\js\*.*  $(DST_VIPERWATCH_DIR)\js\

	@echo Copying ViperWatch Java files
	-$(MD) $(DST_VIPERWATCH_DIR)\Java
	@$(CP) $(VIPERWATCH_SRC_DIR)\Java\*.*  $(DST_VIPERWATCH_DIR)\Java\

	@echo Copying Scripts files
	-$(MD) $(DST_SCRIPT_DIR)
	@$(CP) $(SCRIPT_DIR)\*.*  $(DST_SCRIPT_DIR)\

	@echo Copying Fpga Files
	-$(MD) $(DST_FPGA_DIR)
	@$(CP) $(FPGA_DIR)\*.rbf  $(DST_FPGA_DIR)\

	@echo Copying bootrom Files
	-$(MD) $(DST_VXBOOT_DIR)
	@$(CP) $(VXBOOT_DIR)\*.hex $(DST_VXBOOT_DIR)\*.hex
	@$(CP) $(VXBOOT_DIR)\bootflash.o $(DST_VXBOOT_DIR)\*.o
	@$(CP) $(VXBOOT_DIR)\nvram2.o $(DST_VXBOOT_DIR)\nvram2.*
	@$(CP) $(VXBOOT_DIR)\bootrom_updating.txt $(DST_VXBOOT_DIR)\*.txt

	@echo Copying vipercell.cfg file
	-$(MD) $(GP10_ROOT)
	@$(CP) $(HOST_DIR)\vipercell.cfg $(GP10_ROOT)

	@echo Copying version.txt file and marking as Non-Crypto version
	-$(MD) $(GP10_ROOT)
	@$(CP) $(HOST_DIR)\version.txt $(GP10_ROOT)
	@echo [CRYPTO] >> $(GP10_ROOT)\version.txt
	@echo NON-CRYPTO VERSION >> $(GP10_ROOT)\version.txt


	@echo Copying mib files
	-$(MD) $(DST_MIB_DIR)
	@$(CP) $(HOST_DIR)\mib.txt $(DST_MIB_DIR)
	@$(CP) $(HOST_DIR)\snmp\Asn1\vipercell.mib $(DST_MIB_DIR)\gp10.mib

	@echo Copying default AlarmCode.English file
	-$(MD) $(DST_ALARM_DIR)
	@$(CP) $(HOST_DIR)\Alarm\AlarmCode.English $(DST_ALARM_DIR)

	@echo Preparing encryption enabled option
	-$(MD) $(GP10_CRYPTO_ROOT)
	@$(XCP) /E /C /R /Y $(GP10_ROOT) $(GP10_CRYPTO_ROOT)

	@echo Mark version.txt as CRYPTO version
	@$(CP) $(HOST_DIR)\version.txt $(GP10_CRYPTO_ROOT)
	@echo [CRYPTO] >> $(GP10_CRYPTO_ROOT)\version.txt
	@echo CRYPTO VERSION >> $(GP10_CRYPTO_ROOT)\version.txt

	@echo Update encryption modules
	@$(RM) $(DST_BIN_CRYPTO_DIR)\dsp*.out
	@$(MV) /Y $(DST_BIN_CRYPTO_DIR)\soc.out $(DST_BIN_CRYPTO_DIR)\soc_k8.out
	@$(MV) /Y $(DST_BIN_CRYPTO_DIR)\ssh.out $(DST_BIN_CRYPTO_DIR)\ssh_k8.out

	@$(CP) $(BIN_CRYPTO_DIR)\*.out $(DST_BIN_CRYPTO_DIR)

	@echo Copying DSP Encryption Objects
	-$(MD) $(DST_ENCRYPTION)\encrypt_a51a52
	-$(MD) $(DST_ENCRYPTION)\encrypt_a52only
	@$(CP) .\$(BIN_DIR)\encrypt_a51a52\*    $(DST_ENCRYPTION)\encrypt_a51a52
	@$(CP) $(DST_ENCRYPTION)\encrypt_a51a52 $(DST_BIN_CRYPTO_DIR)
	@$(CP) .\$(BIN_DIR)\encrypt_a52only\*   $(DST_ENCRYPTION)\encrypt_a52only

	@echo Prepare load scripts
	@$(MV) $(DST_SCRIPT_CRYPTO_DIR)\module_list        $(DST_SCRIPT_CRYPTO_DIR)\module_list_noencrypt
	@$(MV) $(DST_SCRIPT_CRYPTO_DIR)\module_list_crypto $(DST_SCRIPT_CRYPTO_DIR)\module_list
	@$(MV) $(DST_SCRIPT_CRYPTO_DIR)\viperstart         $(DST_SCRIPT_CRYPTO_DIR)\viperstart_noencrypt
	@$(MV) $(DST_SCRIPT_CRYPTO_DIR)\viperstart_crypto $(DST_SCRIPT_CRYPTO_DIR)\viperstart
	@$(MV) $(DST_SCRIPT_CRYPTO_DIR)\iosinit.ini         $(DST_SCRIPT_CRYPTO_DIR)\iosinit_noencrypt.ini
	@$(MV) $(DST_SCRIPT_CRYPTO_DIR)\iosinit_crypto.ini $(DST_SCRIPT_CRYPTO_DIR)\iosinit.ini

	@echo Copying certificate files
	@$(CP) $(SRC_CERT_DIR)\*.* $(GP10_CRYPTO_ROOT)
