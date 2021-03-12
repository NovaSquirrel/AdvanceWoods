# Adopted from $DEVKTIARM/gba_rules but uses libtonc instead of libgba

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro)
endif

include $(DEVKITARM)/base_rules

PORTLIBS	:=	$(PORTLIBS_PATH)/gba $(PORTLIBS_PATH)/armv4

LIBTONC	:=	$(DEVKITPRO)/libtonc

#---------------------------------------------------------------------------------
%.gba: %.elf
	@$(OBJCOPY) -O binary $< $@
	@echo built ... $(notdir $@)
	@gbafix $@

#---------------------------------------------------------------------------------
%_mb.elf:
	@echo linking multiboot
	@$(LD) -specs=gba_mb.specs $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
%.elf:
	@echo linking cartridge
	@$(LD)  $(LDFLAGS) -specs=gba.specs $(OFILES) $(LIBPATHS) $(LIBS) -o $@