SRC_DIR ?= src
LIB_DIR ?= lib
OBJ_DIR ?= build
SOURCES ?= $(shell find $(SRC_DIR) $(LIB_DIR) -name '*.c' -or -name '*.S')
OBJECTS ?= $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
LINKER ?= dtekv-script.lds

VPATH := $(SRC_DIR):$(LIB_DIR)

TOOLCHAIN ?= riscv32-unknown-elf-
CFLAGS ?= -Wall -nostdlib -O3 -mabi=ilp32 -march=rv32imzicsr -fno-builtin -Iinclude -Ilib

.PHONY: build clean run
build: $(OBJ_DIR)/main.bin

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(TOOLCHAIN)gcc -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.S
	@mkdir -p $(OBJ_DIR)
	$(TOOLCHAIN)gcc -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/main.elf: $(OBJECTS) $(LINKER) $(LIB_DIR)/softfloat.a
	cd $(OBJ_DIR) && $(TOOLCHAIN)ld -o main.elf -T ../$(LINKER) $(filter-out boot.o, $(notdir $(OBJECTS))) ../$(LIB_DIR)/softfloat.a

$(OBJ_DIR)/main.bin: $(OBJ_DIR)/main.elf
	$(TOOLCHAIN)objcopy --output-target binary $< $@
	$(TOOLCHAIN)objdump -D $< > $<.txt

clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.elf $(OBJ_DIR)/*.bin $(OBJ_DIR)/*.txt

TOOL_DIR ?= ./tools
run: $(OBJ_DIR)/main.bin
	$(MAKE) -C $(TOOL_DIR) "FILE_TO_RUN=$(CURDIR)/$<"
