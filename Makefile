XBE_TITLE = XNetProfiles
GEN_XISO = $(XBE_TITLE).iso
SRCS += $(CURDIR)/main.c $(CURDIR)/backend.c $(CURDIR)/ui.c
NXDK_DIR ?= /home/USER/nxdk
NXDK_SDL = y
NXDK_SDL_IMAGE = y
NXDK_SDL_TTF = y

include $(NXDK_DIR)/Makefile

# Include resources in the ISO
WALLPAPER = background.png
FONT = vegur-regular.ttf

TARGET += $(OUTPUT_DIR)/$(WALLPAPER) $(OUTPUT_DIR)/$(FONT)
$(GEN_XISO): $(OUTPUT_DIR)/$(WALLPAPER) $(OUTPUT_DIR)/$(FONT)

$(OUTPUT_DIR)/$(WALLPAPER): $(CURDIR)/$(WALLPAPER) $(OUTPUT_DIR)
	$(VE)cp '$<' '$@'

$(OUTPUT_DIR)/$(FONT): $(CURDIR)/$(FONT) $(OUTPUT_DIR)
	$(VE)cp '$<' '$@'
