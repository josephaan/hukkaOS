
CFLAGS := -Wall  -Wextra -fpic -ffreestanding -fno-stack-protector -nostdinc -nostdlib -mno-red-zone -DENABLE_LOGGING -I src/kernel/include -I src/kernel/include/fonts -I src/kernel/include/boot -I src/libc/include/ 

# -DENABLE_LOGGING // Add this to enable logging
#        -D_TEST_=1

ASM_FLAGS := -f elf64

FONT_FLAGS :=  -r -b binary

#FONT_FLAGS := -r -b binary 

LDFLAGS =  -nostdlib -n -T  -verbose

PRJ_FOLDERS := src
TEST_FOLDER := tests
