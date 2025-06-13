source ~/.bashrc
make clean
make

x86_64-elf-strip -s -K mmio -K fb -K bootboot -K environment -K initstack  build/mykernel.x86_64.elf

./mkbootimg check build/mykernel.x86_64.elf

cd bin

make disk
make bios

