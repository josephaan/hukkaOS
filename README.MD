


Paging Mapping

== Kernel Space:

Kernel Address starts at 0xFFFF800000000000

APIC and IOAPIC remapped to address  0xFFFFFFFFF7001000 and 0xFFFFFFFFF7002000 respectively.

Kernel heap starts at 0xFFFF800000000000

32 bit linear frame buffer, mapped at the negative address defined by the fb symbol at -64M or 0xFFFFFFFF_FC000000 
MMIO areas at -128M or 0xFFFFFFFF_F8000000 
Bootboot structure is mapped at bootboot symbol, at -2M or 0xFFFFFFFF_FFE00000.

The environment configuration string (or command line if you like) is mapped at environment symbol, at -2M + 1 page or 0xFFFFFFFF_FFE01000.
Kernel's combined code and data segment is mapped at -2M + 2 pages or 0xFFFFFFFF_FFE02000.



== Userspace

Userspace starts at 0x0000000000000000

Program should be loaded at address: 0x00000004_00000000 

Program stack starts at 0x00007FFFFFFFFFFF and grows downward.

The User space ends with address:    0x00007FFFFFFFFFFF