# Thin `make` wrapper around the CMake/Ninja build that pico-sdk requires
# (board config headers, boot2 checksum generation, and the RP2040 linker
# script all come from pico-sdk's own CMake logic). Day-to-day use is just
# `make` / `make clean` / `make flash` -- CMake still runs underneath, but
# never needs to be typed directly.

BUILD_DIR      := build
PICO_SDK_PATH  ?= $(abspath ../pico-sdk)

# Needed because pico-sdk 2.0.0 vendors an old mbedtls whose
# cmake_minimum_required() predates policies removed by CMake >= 4.0, and
# picotool is fetched/built from source the first time.
export CMAKE_POLICY_VERSION_MINIMUM := 3.5

UF2 := $(BUILD_DIR)/blink.uf2

.PHONY: all configure build clean flash

all: build

configure: $(BUILD_DIR)/build.ninja

$(BUILD_DIR)/build.ninja:
	cmake -S lib -B $(BUILD_DIR) -G Ninja -DPICO_SDK_PATH=$(PICO_SDK_PATH)

build: configure
	cmake --build $(BUILD_DIR)

$(UF2): build

clean:
	rm -rf $(BUILD_DIR)

# Copies the UF2 onto a Pico in BOOTSEL mode, if its mass-storage drive is
# mounted at one of the usual locations. Adjust MOUNT= if yours differs, e.g.
#   make flash MOUNT=/run/media/$$USER/RPI-RP2
MOUNT := $(firstword $(wildcard /run/media/$(USER)/RPI-RP2 /media/$(USER)/RPI-RP2))

flash: build
	@if [ -z "$(MOUNT)" ]; then \
		echo "RPI-RP2 drive not found. Hold BOOTSEL while plugging in the Pico, then re-run 'make flash'."; \
		exit 1; \
	fi
	cp $(UF2) "$(MOUNT)/"
	@echo "Copied $(UF2) -> $(MOUNT)/"
