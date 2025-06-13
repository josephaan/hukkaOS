#include <hardware/acpi/tables/hpet.h>


static volatile hpet_t *hpet;
static volatile hpet_regs_t *hpet_regs;

/* core functions */

// get HPET and set necessary variables
void hpet_initialize(void)
{
    bool rsdt_table = is_rsdt();
    //log(//_LEVEL_INFO, "HPET is rsdt query = %d", rsdt_table);
    bool xsdt_table = is_xsdt();
    //(//_LEVEL_INFO, "HPET is xsdt query = %d", xsdt_table);
    if(rsdt_table == true){
        hpet = (hpet_t *)(uintptr_t)find_version_1_table("HEPT");
    } else if(xsdt_table == true){
        hpet = (hpet_t *)(uintptr_t)find_version_2_up_table("HEPT");
    } else {
        //(//_LEVEL_ERROR, "NO SDT hence No HPET");
        return;
    }
    

    if (hpet == NULL){
        //(//_LEVEL_ERROR, "No HPET Initialized");
        return;
    }
    //find_table("HPET");
    hpet_regs = (hpet_regs_t *)hpet->address;

    hpet_regs->counter_value = 0;
    hpet_regs->general_config = 1;

    //(//_LEVEL_INFO, "HPET initialized");
}

// wait for us nanoseconds
void hpet_usleep(uint64_t us)
{
    uint32_t clock_period = hpet_regs->capabilities >> 32;

    volatile size_t target_value = hpet_regs->counter_value + (us * (1000000000 / clock_period));

    while (hpet_regs->counter_value < target_value)
    {
        asm volatile("pause");
    }
}

// return the current counter value
uint64_t hpet_get_counter_value(void)
{
    return hpet_regs->counter_value;
}