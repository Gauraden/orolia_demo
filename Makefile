include Makefile.toolchain

ROOT_DIR   = $(shell pwd)
OUTPUT_DIR = $(ROOT_DIR)/output/$(*)
BUILD_DIR  = $(ROOT_DIR)/build/$(*)

CMAKE = $(ROOT_PATH)/usr/bin/$(TOOLS_PREFIX)cmake ../../ -DCMAKE_INSTALL_PREFIX="${OUTPUT_DIR}"

all: target_linux_x86
	
target_%:
	@echo "--- Сборка ($(*)) ---------------------------"
	@mkdir -p $(BUILD_DIR) $(OUTPUT_DIR)
	@cd $(BUILD_DIR) && $(CMAKE) && make install