#ifndef __IOAPIC__H
#define __IOAPIC__H

#include <stdint.h>

void ioapic_init();
void ioapic_set_irq(uint8_t irq, uint8_t vector, uint8_t apic_id);
void ioapic_unmask_irq(uint8_t irq);
void ioapic_send_eoi(uint8_t vector);
void ioapic_write(uint8_t reg, uint32_t value) ;
uint32_t ioapic_read(uint8_t reg);
#endif // !