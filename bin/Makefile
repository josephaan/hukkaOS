# platform, either x86, rpi or icicle
PLATFORM=x86
# the path to OVMF.fd (for testing with EFI)
#OVMF=bios64.bin 
OVMF=RELEASEX64_OVMF.fd

all: mkbootimg initdir disk

# compile the image creator
mkbootimg:
	@make -C ../mkbootimg all

# create an initial ram disk image with the kernel inside
initdir:
	@mkdir initrd initrd/sys 2>/dev/null | true

ifeq ($(PLATFORM),x86)
	cp ../build/mykernel.x86_64.elf initrd/sys/core
else
ifeq ($(PLATFORM),rpi)
	cp ../build/mykernel.aarch64.elf initrd/sys/core
else
	cp ../build/mykernel.riscv64.elf initrd/sys/core
endif
endif

# create hybrid disk / cdrom image or ROM image
disk: ./mkbootimg initdir mkbootimg.json
	./mkbootimg mkbootimg.json disk-$(PLATFORM).img
	@rm -rf initrd


# test the disk image
bios:
	qemu-system-x86_64 -enable-kvm  -hda disk-x86.img -M q35 -m 2G -cpu host  -serial stdio -serial file:log.txt

cdrom:
	qemu-system-x86_64 -cdrom disk-x86.img -serial stdio

grubcdrom: grub.iso
	qemu-system-x86_64 -cdrom grub.iso -serial stdio

grub2: grub.iso
	qemu-system-x86_64 -drive file=disk-x86.img,format=raw -cdrom grub.iso -boot order=d -serial stdio

efi:
	qemu-system-x86_64 -bios $(OVMF)  -enable-kvm -M q35 -m 2G -cpu host  -drive file=disk-x86.img,format=raw -serial stdio
	@printf '\033[0m'

# clean up
clean:
	rm -rf initrd *.bin *.img *.rom *.iso 2>/dev/null || true
