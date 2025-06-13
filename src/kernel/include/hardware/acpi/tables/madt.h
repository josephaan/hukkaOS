#ifndef __MADT__H
#define __MADT__H

#include <stdint.h>
#include <hardware/acpi/acpi.h>

void parse_madt(madt_t * madt);

uint32_t get_lapic_address();
uint32_t get_ioapic_addresss();

#endif //