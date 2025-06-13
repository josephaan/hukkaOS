#creates INITRD
mkdir -p tmp/sys
cp ../build/mykernel.x86_64.elf tmp/sys/core
cd tmp
find . | cpio -H hpodc -o | gzip > ../INITRD
cd ..
#creates img
dd if=/dev/zero of=fat.img bs=1k count=1440
mformat -i fat.img -f 1440 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mmd -i fat.img ::/BOOTBOOT
mcopy -i fat.img bootboot.efi ::/EFI/BOOT/BOOTX64.EFI
mcopy -i fat.img INITRD ::/BOOTBOOT/INITRD 

qemu-system-x86_64 fat.img -bios RELEASEX64_OVMF.fd
