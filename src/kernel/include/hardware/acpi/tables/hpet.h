#ifndef HPET_H
#define HPET_H

#include <stddef.h>
#include <stdint.h>

#include <hardware/acpi/acpi.h>

typedef struct __attribute__((__packed__))
{
    acpi_sdt_header_t header;
    uint8_t hardware_rev_id;
    uint8_t info; 
    uint16_t pci_id;
    uint8_t address_space_id;
    uint8_t register_width;
    uint8_t register_offset;
    uint8_t reserved;
    uint64_t address;
    uint8_t hpet_num;
    uint16_t minimum_ticks;
    uint8_t page_protection;
} hpet_t;

typedef struct __attribute__((packed))
{
    uint64_t capabilities;
    uint64_t reserved0;
    uint64_t general_config;
    uint64_t reserved1;
    uint64_t int_status;
    uint64_t reserved2;
    uint64_t reserved3[24];
    volatile uint64_t counter_value;
    uint64_t unused4;
} hpet_regs_t;

void hpet_initialize(void);
void hpet_usleep(uint64_t us);
uint64_t hpet_get_counter_value(void);

#endif