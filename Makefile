OUTPUT_DIR = $(shell pwd)/output

linux:
	@echo "--- Building for OS GNU/Linux ---"
	@mkdir -p ./build/linux ./output/linux
	@cd ./build/linux && cmake -DCMAKE_INSTALL_PREFIX="${OUTPUT_DIR}/linux" ../../ && make install

win32:
	@echo "--- Building for OS Windows -----"
	@mkdir -p ./build/win32 ./output/win32
	@cd ./build/win32 && cmake -DCMAKE_INSTALL_PREFIX="${OUTPUT_DIR}/win32" ../../ && make install