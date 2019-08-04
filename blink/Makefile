PROJECT_NAME     := blinky_pca10028_s130
TARGETS          := nrf51422_xxac
OUTPUT_DIRECTORY := _build

SDK_ROOT := ../sdk/nRF5_SDK_12.3.0_d7731ad
PROJ_DIR := .

$(OUTPUT_DIRECTORY)/nrf51422_xxac.out: \
  LINKER_SCRIPT  := sensor_pro.ld

-include sources.mk
-include objects.mk

# Linker flags
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m0
# let linker to dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs -lc -lnosys


.PHONY: $(TARGETS) default all clean help flash flash_softdevice

# Default target - first one defined
default: nrf51422_xxac

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo 	nrf51422_xxac

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc

include $(TEMPLATE_PATH)/Makefile.common

# # use system gcc tools
# GNU_INSTALL_ROOT := /usr
# GNU_PREFIX := arm-none-eabi
# CC      := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-gcc"
# CXX     := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-c++"
# AS      := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-as"
# AR      := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ar" -r
# LD      := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-ld"
# NM      := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-nm"
# OBJDUMP := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objdump"
# OBJCOPY := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-objcopy"
# SIZE    := "$(GNU_INSTALL_ROOT)/bin/$(GNU_PREFIX)-size"

$(foreach target, $(TARGETS), $(call define_target, $(target)))

# Flash the program
flash: $(OUTPUT_DIRECTORY)/nrf51422_xxac.hex
	@echo Flashing: $<
	nrfjprog --program $< -f nrf51 --sectorerase
	nrfjprog --reset -f nrf51

# Flash softdevice
flash_softdevice:
	@echo Flashing: s130_nrf51_2.0.1_softdevice.hex
	nrfjprog --program $(SDK_ROOT)/components/softdevice/s130/hex/s130_nrf51_2.0.1_softdevice.hex -f nrf51 --sectorerase 
	nrfjprog --reset -f nrf51

erase:
	nrfjprog --eraseall -f nrf52
