include config/Config.mk
include config/Common.mk

VERSION := 0.0.0

# arch dependant
ARCH_PREFIX := x86_64-elf
ASM_COMPILER := nasm
QEMU_SYSTEM := qemu-system-x86_64

# Directories
SRC_DIR =src
ASM_DIR =$(SRC_DIR)/asm
KERNEL_DIR =$(SRC_DIR)/kernel
USER_DIR =$(SRC_DIR)/user
TLIBC_DIR =$(SRC_DIR)/libc
FONT_DIR =$(SRC_DIR)/fonts

# linker flags

LDFLAGS := -nostdlib -n -T $(SRC_DIR)/link.ld 

# Output

OUT_DIR = build
KERNEL_ELF = $(OUT_DIR)/mykernel.x86_64.elf

IS_WORKFLOW = 0

ASM_COMPILER = nasm
X_OBJCOPY = objcopy

ifeq ($(TOOLCHAIN), gcc)
	X_CC = $(ARCH_PREFIX)-gcc
	X_LD = $(ARCH_PREFIX)-ld
else ifeq ($(TOOLCHAIN), clang)
	X_CC = clang
	X_LD = ld.lld
else
	$(error "Unknown compiler toolchain")
endif

# Source files
ASM_SOURCE_FILES = $(shell find $(ASM_DIR) -name '*.s')
FONT_SOURCE_FILES = $(shell find $(FONT_DIR) -name '*.psf')
KERNEL_SOURCE_FILES = $(shell find $(KERNEL_DIR) -name '*.c')
USER_SOURCE_FILES = $(shell find $(USER_DIR) -name '*.c')
TLIBC_SOURCE_FILES = $(shell find $(TLIBC_DIR) -name '*.c')

# Object files
ASM_OBJECT_FILES = $(patsubst $(ASM_DIR)/%.s, $(OUT_DIR)/asm/%.o, $(ASM_SOURCE_FILES))
FONT_OBJECT_FILES = $(patsubst $(FONT_DIR)/%.psf, $(OUT_DIR)/fonts/%.o, $(FONT_SOURCE_FILES))
KERNEL_OBJECT_FILES = $(patsubst $(KERNEL_DIR)/%.c, $(OUT_DIR)/kernel/%.o, $(KERNEL_SOURCE_FILES))
USER_OBJECT_FILES = $(patsubst $(USER_DIR)/%.c, $(OUT_DIR)/user/%.o, $(USER_SOURCE_FILES))
TLIBC_OBJECT_FILES = $(patsubst $(TLIBC_DIR)/%.c, $(OUT_DIR)/tlibc/%.o, $(TLIBC_SOURCE_FILES))

# All object files
OBJECT_FILES = $(ASM_OBJECT_FILES) $(FONT_OBJECT_FILES) $(KERNEL_OBJECT_FILES) $(TLIBC_OBJECT_FILES) $(USER_OBJECT_FILES)

#	Default target
all: $(KERNEL_ELF)

# Compile assembler object files
$(OUT_DIR)/asm/%.o: $(ASM_DIR)/%.s
	@mkdir -p $(dir $@)
	$(ASM_COMPILER) ${ASM_FLAGS} $< -o $@

# Compile kernel object files
$(OUT_DIR)/kernel/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(dir $@)
	$(X_CC) ${CFLAGS} -c $< -o $@

# Compile user object files
$(OUT_DIR)/user/%.o: $(USER_DIR)/%.c
	@mkdir -p $(dir $@)
	$(X_CC) ${CFLAGS} -c $< -o $@

# Compile tlibc object files
$(OUT_DIR)/tlibc/%.o: $(TLIBC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(X_CC) ${CFLAGS} -c $< -o $@

# Compile font object file
$(OUT_DIR)/fonts/%.o: $(FONT_DIR)/%.psf
	@mkdir -p $(dir $@)
	$(X_LD) ${FONT_FLAGS} $< -o $@

# Link kernel binary
$(KERNEL_ELF): $(OBJECT_FILES)
	@mkdir -p $(dir $@)
	$(X_LD) $(LDFLAGS) -o $@ $(OBJECT_FILES)
	x86_64-elf-readelf -hls $(OUT_DIR)/mykernel.x86_64.elf > $(OUT_DIR)/mykernel.x86_64.txt

#Clean build directory
clean:
	rm -r build

.PHONY: all clean

