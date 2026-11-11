###############################################################################
# ATmega32 layered drivers - build system
#
#   make                      build APP (default motor_control) for the target
#   make APP=smart_home       build another application
#   make flash APP=...        program with avrdude (USBasp by default)
#   make all-apps             build every application for the target
#   make host-check           compile and link every app on the PC (-Werror)
#   make test                 build and run the host unit tests
#   make clean
###############################################################################

APP        ?= motor_control
APPS       := motor_control locker_system car_parking smart_home
MCU        ?= atmega32
F_CPU      ?= 8000000UL
PROGRAMMER ?= usbasp

BUILD      := build

# Every driver folder is on the include path; only the modules an app lists
# in its app.mk are compiled into it.
ALL_MODULE_DIRS := $(sort $(dir $(wildcard MCAL/*/*.h HAL/*/*.h SERVICE/*/*.h)))
MODULE_INCLUDES := $(addprefix -I,$(ALL_MODULE_DIRS))

# Application config folder comes first so it overrides CONFIG/default
app_includes = -IAPP/$(1)/config -ICONFIG/default -ILIB $(MODULE_INCLUDES)
app_modules  = $(shell sed -n 's/^MODULES *:= *//p' APP/$(1)/app.mk)
app_sources  = APP/$(1)/main.c LIB/UTILS.c \
               $(foreach m,$(call app_modules,$(1)),$(wildcard MCAL/$(m)/*.c HAL/$(m)/*.c SERVICE/$(m)/*.c))

WARNINGS := -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Wmissing-prototypes \
            -Wcast-qual -Wundef -Wswitch-default

#--------------------------------- Target ------------------------------------
AVR_CC      := avr-gcc
AVR_OBJCOPY := avr-objcopy
AVR_SIZE    := avr-size
AVR_CFLAGS  := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=c99 -Os $(WARNINGS) \
               -ffunction-sections -fdata-sections -fshort-enums
AVR_LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections

TARGET_ELF := $(BUILD)/$(APP)/$(APP).elf
TARGET_HEX := $(BUILD)/$(APP)/$(APP).hex

.PHONY: all flash all-apps host-check test clean

all: $(TARGET_HEX)

$(TARGET_ELF): $(call app_sources,$(APP)) $(wildcard LIB/*.h CONFIG/default/*.h APP/$(APP)/config/*.h MCAL/*/*.h HAL/*/*.h SERVICE/*/*.h)
	@mkdir -p $(dir $@)
	$(AVR_CC) $(AVR_CFLAGS) $(call app_includes,$(APP)) $(call app_sources,$(APP)) $(AVR_LDFLAGS) -o $@
	$(AVR_SIZE) --format=avr --mcu=$(MCU) $@ 2>/dev/null || $(AVR_SIZE) $@

$(TARGET_HEX): $(TARGET_ELF)
	$(AVR_OBJCOPY) -O ihex -R .eeprom $< $@

flash: $(TARGET_HEX)
	avrdude -c $(PROGRAMMER) -p m32 -U flash:w:$<:i

all-apps:
	@for a in $(APPS); do $(MAKE) --no-print-directory APP=$$a || exit 1; done

#--------------------------------- Host --------------------------------------
HOST_CC     ?= gcc
HOST_CFLAGS := -DHOST_TEST -DF_CPU=$(F_CPU) -std=c99 -O1 -g $(WARNINGS) -Werror

host-check:
	@for a in $(APPS); do $(MAKE) --no-print-directory host-app APP=$$a || exit 1; done
	@echo "All applications compile and link cleanly on the host."

.PHONY: host-app
host-app:
	@mkdir -p $(BUILD)/host
	@echo "HOST  $(APP)"
	@$(HOST_CC) $(HOST_CFLAGS) $(call app_includes,$(APP)) $(call app_sources,$(APP)) \
		test/host_regs.c -o $(BUILD)/host/$(APP)

# Unit tests use the default configuration for every module
TEST_SOURCES := test/test_main.c test/host_regs.c LIB/UTILS.c \
                $(wildcard MCAL/*/*.c HAL/*/*.c SERVICE/*/*.c)

test:
	@mkdir -p $(BUILD)/host
	$(HOST_CC) $(HOST_CFLAGS) -ICONFIG/default -ILIB $(MODULE_INCLUDES) \
		$(TEST_SOURCES) -o $(BUILD)/host/unit_tests
	./$(BUILD)/host/unit_tests

clean:
	rm -rf $(BUILD)
