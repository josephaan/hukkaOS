#ifndef __KERNEL_H
#define __KERNEL_H



// bootboot
#include <bootboot.h>

// Architecture header files
#include <x86_64/idt.h>
#include <x86_64/gdt.h>
#include <x86_64/cpu.h>

// Interface related header files
#include <display/framebuffer.h>
#include <logos/hukaos_logo.h>
#include <logos/snake.h>
#include <gui/gui.h>
#include <terminal/terminal.h>

// ACPI & APIC related header files
#include <hardware/acpi/acpi.h>
#include <hardware/acpi/tables/madt.h>
#include <hardware/apic/apic.h>
#include <hardware/apic/ioapic.h>

// IO header files
#include <hardware/ps2/devices/keyboard.h>

// Logging header files
#include <debug/log.h>
// Memory management header files
#include <memory/pmm.h>
#include <memory/phys.h>
#include <memory/paging.h>
#include <memory/vmm.h>

// Storage management
#include <hardware/storage/ata.h>





#endif // !__KERNEL_H
