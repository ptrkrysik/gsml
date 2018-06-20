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

SOURCE_LIST = 	./BURST/BURST.C \
		./BURST/MOD2ASM.ASM \
		./BURST/MOD2SPB.C \
		./CHCODEC/BFI.C \
		./CHCODEC/CODECS.C \
		./CHCODEC/INTLEAVS.C \
		./CHCODEC/SBJCODECS.C \
		./CHCODEC/TOPCODEC.C \
		./CHCODEC/VITGSM.ASM \
		./CHCODEC/CHCODESUBS.ASM \
		./CHCODEC/SIDDETECT.C \
		./CHCODEC/INTLEAV.ASM \
		./DEMOD/DEMSUBS.ASM \
		./DEMOD/DEMTOP.C \
		./DEMOD/DEMODC.C \
		./DEMOD/VITEQ.ASM

MODULE_OBJS    	= $(OBJDIR)/BURST.obj \
		../MOD2ASM.obj \
		$(OBJDIR)/MOD2SPB.obj \
		$(OBJDIR)/BFI.obj \
		$(OBJDIR)/CODECS.obj \
		$(OBJDIR)/INTLEAVS.obj \
		$(OBJDIR)/SBJCODECS.obj \
		$(OBJDIR)/TOPCODEC.obj \
		$(OBJDIR)/SIDDETECT.obj \
		../VITGSM.obj \
		../CHCODESUBS.obj \
		../DEMSUBS.obj \
		../INTLEAV.obj \
		$(OBJDIR)/DEMTOP.obj \
		$(OBJDIR)/DEMODC.obj \
		../VITEQ.obj

all: $(MODULE_OBJS)

# rule for C++ compiles
$(OBJDIR)/%.obj : BURST/%.c
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

../%.obj : ./BURST/%.ASM 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS) $<

$(OBJDIR)/MOD2SPB.obj : ./BURST/MOD2SPB.C 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/BFI.obj : ./CHCODEC/BFI.C 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/CODECS.obj : ./CHCODEC/CODECS.C 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/INTLEAVS.obj : ./CHCODEC/INTLEAVS.C 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/SBJCODECS.obj : ./CHCODEC/SBJCODECS.C 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<
    
$(OBJDIR)/SIDDETECT.obj : ./CHCODEC/SIDDETECT.C 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/TOPCODEC.obj : ./CHCODEC/TOPCODEC.C 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

../VITGSM.obj : ./CHCODEC/VITGSM.ASM 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS)  $<

../CHCODESUBS.obj : ./CHCODEC/CHCODESUBS.ASM 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS)  $<

../INTLEAV.obj : ./CHCODEC/INTLEAV.ASM 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS)  $<

../DEMSUBS.obj : ./DEMOD/DEMSUBS.ASM 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS)  $<

$(OBJDIR)/DEMTOP.obj : ./DEMOD/DEMTOP.C 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

$(OBJDIR)/DEMODC.obj : ./DEMOD/DEMODC.C 
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(CC_FLAGS) $(CC_INCLUDE) $<

../VITEQ.obj : ./DEMOD/VITEQ.ASM
	@ $(RM) $(subst /,$(DIRCHAR),$@)
	$(DSPCC) $(ASM_FLAGS)  $<

