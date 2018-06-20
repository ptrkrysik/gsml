#****************************************************************************
#   (c) Copyright Cisco 2000
#   All Rights Reserved
#****************************************************************************

# TOP_OF_VOB must be defined before including l3defs.mk
TOP_OF_VOB = ..\..

include $(TOP_OF_VOB)\l3defs.mk
include ../dsp.mk
.SUFFIXES:
.SUFFIXES: .c .asm .obj

SOURCE_LIST = accum.c agc.c bcch.c burstisr.c DYNPWRCTRL.c encrypt.c \
encryptA52only.asm encryptA51A52.asm frameisr.c frmsync.c groupini.c \
grouptbl.c hoProc.c PAGCH.c RTSCHECK.c SACCH.c SDCCH.c SYNCMSG.c  tch.c

MODULE_OBJS = $(OBJDIR)/accum.obj \
 $(OBJDIR)/agc.obj \
 $(OBJDIR)/bcch.obj \
 $(OBJDIR)/burstisr.obj \
 $(OBJDIR)/DYNPWRCTRL.obj \
 $(OBJDIR)/encrypt.obj \
 ../encryptA52only.obj \
 ../encryptA51A52.obj \
 ../decryptopt.obj \
 $(OBJDIR)/frameisr.obj \
 $(OBJDIR)/frmsync.obj \
 $(OBJDIR)/groupini.obj \
 $(OBJDIR)/grouptbl.obj \
 $(OBJDIR)/hoProc.obj \
 $(OBJDIR)/PAGCH.obj \
 $(OBJDIR)/RTSCHECK.obj \
 $(OBJDIR)/SACCH.obj \
 $(OBJDIR)/SDCCH.obj \
 $(OBJDIR)/SYNCMSG.obj \
 $(OBJDIR)/tch.obj \
 $(OBJDIR)/pdch.obj

all: $(MODULE_OBJS)

# rule for C++ compiles
$(OBJDIR)/%.obj : %.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

../%.obj : %.asm 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS) $<

$(OBJDIR)/agc.obj : agc.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/bcch.obj : bcch.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/burstisr.obj : burstisr.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/DYNPWRCTRL.obj : DYNPWRCTRL.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/encrypt.obj : encrypt.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

../encryptA52only.obj : encryptA52only.ASM 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS)  $<

../encryptA51A52.obj : encryptA51A52.ASM 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS)  $<

../decryptopt.obj : decryptopt.ASM 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS)  $<

$(OBJDIR)/frameisr.obj : frameisr.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/frmsync.obj : frmsync.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/groupini.obj : groupini.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/grouptbl.obj : grouptbl.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/PAGCH.obj : PAGCH.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/RTSCHECK.obj : RTSCHECK.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/SACCH.obj : SACCH.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/SDCCH.obj : SDCCH.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/SYNCMSG.obj : SYNCMSG.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/tch.obj : tch.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/hoProc.obj : hoProc.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/pdch.obj : pdch.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<
