#include <hardware/acpi/acpi.h>
#include <hardware/acpi/tables/madt.h>
#include <display/framebuffer.h>


static xsdt_t* xsdt ;
static rsdt_t* rsdt ;


acpi_sdt_header_t* find_rsdp(uint64_t acpi_ptr) {
    return (acpi_sdt_header_t*)acpi_ptr;
}

void print_sdt_type(){
    uintptr_t acpi_ptr = bootboot.arch.x86_64.acpi_ptr;
    acpi_sdt_header_t* sdt = find_rsdp(acpi_ptr);
    if (strncmp(sdt->signature, "RSDT", 4) == 0){
        print_numstr("\nValid RSDT", 0xFFFFFF );
        //print_numstr("\nRSDT Length %d", 0xFFFFFF , rsdp->length);
        if(sdt->revision == 0){
            print_numstr("\nVersion 1.0", 0xFFFFFF );
        } else { print_numstr("\nVersion > 1.0", 0xFFFFFF );}
        madt_t* madt = (madt_t*)find_version_1_table("APIC");
        if (!madt) {
            // MADT not found
            return;
        } else { 
            print_numstr("\nValid APIC found", 0xFFFFFF ); 
            uint8_t* entry = madt->entries;
            int num_entry = *entry;
            print_numstr("\nThe number of entries %d", 0xFFFFFF , num_entry );
            while (entry < (uint8_t*)madt + madt->header.length) {
                madt_entry_header_t* hdr = (madt_entry_header_t*)entry;
                switch (hdr->type) {
                    case 0: // Local APIC
                    {
                        madt_local_apic_t* lapic = (madt_local_apic_t*)entry;
                        print_numstr("\nThe local apic id = %d and processor id = %d and length = %d", 0xFFFFFF , (int)lapic->apic_id, (int)lapic->acpi_processor_id, (int)lapic->header.length);
                        break;
                    }
                    case 1: // I/O APIC
                    {
                        madt_io_apic_t* ioapic = (madt_io_apic_t*)entry;
                        // Process I/O APIC entry
                        print_numstr("\nThe io apic id = %d and address = %p", 0xFFFFFF , (int)ioapic->io_apic_id, (uintptr_t)ioapic->io_apic_address);
                        break;
                    }
                    // Handle other MADT entry types...
                }
                entry += hdr->length;
            }
        }

    } else if (strncmp(sdt->signature, "XSDT", 4) == 0){
        print_numstr("\nValid XSDT", 0xFFFFFF );
        if(sdt->revision == 0){
            print_numstr("\nVersion 1.0", 0xFFFFFF );
        } else { print_numstr("\nVersion %d", 0xFFFFFF , (int)sdt->revision); }
        madt_t* madt = (madt_t*)find_version_2_up_table("APIC");
        if (!madt) {
            // MADT not found
            return;
        } else { 
            print_numstr("\nValid APIC found", 0xFFFFFF ); 
            uint8_t* entry = madt->entries;
            int num_entry = *entry;
            print_numstr("\nThe number of entries %d", 0xFFFFFF , num_entry );
            while (entry < (uint8_t*)madt + madt->header.length) {
                madt_entry_header_t* hdr = (madt_entry_header_t*)entry;
                switch (hdr->type) {
                    case 0: // Local APIC
                    {
                        madt_local_apic_t* lapic = (madt_local_apic_t*)entry;
                        print_numstr("\nThe local apic id = %d and processor id = %d", 0xFFFFFF , (int)lapic->apic_id, (int)lapic->acpi_processor_id);
                        break;
                    }
                    case 1: // I/O APIC
                    {
                        madt_io_apic_t* ioapic = (madt_io_apic_t*)entry;
                        // Process I/O APIC entry
                        print_numstr("\nThe io apic id = %d and address = %p", 0xFFFFFF , (int)ioapic->io_apic_id, (uintptr_t)ioapic->io_apic_address);
                        break;
                    }
                    // Handle other MADT entry types...
                }
                entry += hdr->length;
            }
        }
    }

}

madt_t * get_madt(madt_t* madt) {
    return madt;
}


acpi_sdt_header_t* find_version_1_table(const char* signature) {
    //log(//LOG_LEVEL_INFO, "SEARCHING for %s", signature);

    int entries = (rsdt->header.length - sizeof(acpi_sdt_header_t)) / sizeof(uint32_t);
    //log(//LOG_LEVEL_INFO, "There are %ld Table Entries found", entries);

    for (int i = 0; i < entries; i++) {
        acpi_sdt_header_t* table = (acpi_sdt_header_t*)(uintptr_t)(rsdt->pointer_to_other_sdt[i]);
        if (strncmp(table->signature, signature, 4) == 0) {
            //log(//LOG_LEVEL_INFO, "Valid %s found", signature);
            bool validity = doChecksum(&rsdt->header);
            if(validity == false){
                //log(//LOG_LEVEL_INFO, "Checksum Invalid");
            } else{
                //log(//LOG_LEVEL_INFO, "Checksum Valid");
            }
            return table;
        }
    }
    //log(//LOG_LEVEL_ERROR, "No %s found", signature); 
    
    return NULL;
}

acpi_sdt_header_t* find_version_2_up_table(const char* signature) {
    //log(//LOG_LEVEL_INFO, "SEARCHING for %s", signature);

    uint32_t entries = (xsdt->header.length - sizeof(acpi_sdt_header_t)) / sizeof(uint64_t);
    //log(//LOG_LEVEL_INFO, "There are %ld Table Entries found", entries);

    for (uint32_t i = 0; i < entries; i++) {
        acpi_sdt_header_t* table = (acpi_sdt_header_t*)(uintptr_t)(xsdt->pointer_to_other_sdt[i]);
        if (strncmp(table->signature, signature, 4) == 0) {
            //log(//LOG_LEVEL_INFO, "Valid %s found", signature);
                        bool validity = doChecksum(&rsdt->header);
            if(validity == false){
                //log(//LOG_LEVEL_INFO, "Checksum Invalid");
            } else{
                //log(//LOG_LEVEL_INFO, "Checksum Valid");
            }
            return table;
        } 
    }
    //log(//LOG_LEVEL_ERROR, "No %s found", signature); 
        
    
    return NULL;
}

static bool rsdt_bool = false, xsdt_bool = false;

void parse_acpi(){

    uintptr_t acpi_ptr = bootboot.arch.x86_64.acpi_ptr;
    acpi_sdt_header_t * sdt = find_rsdp(acpi_ptr);
    if (strncmp(sdt->signature, "RSDT", 4) == 0){
        rsdt_bool = true;
        rsdt = (rsdt_t*)sdt;
        madt_t* madt = (madt_t*)find_version_1_table("APIC");
        if (!madt) {
        // MADT not found
            //log(//LOG_LEVEL_ERROR, "No MADT table found");
            return;
        } else { 
            parse_madt(madt); 
            //log(//LOG_LEVEL_INFO, "FOUND MADT table");
        }

    } else if (strncmp(sdt->signature, "XSDT", 4) == 0){
        xsdt_bool = true;
        xsdt = (xsdt_t*)sdt;
        madt_t* madt = (madt_t*)find_version_2_up_table("APIC");
        if (!madt) {
        // MADT not found
            //log(//LOG_LEVEL_ERROR, "No MADT table found");
            return;
        } else { 
            parse_madt(madt); 
            //log(//LOG_LEVEL_INFO, "FOUND MADT table");
        }
    }
}

acpi_sdt_header_t* find_table(rsdt_t* sdt, const char* signature) {
    if (strncmp(sdt->header.signature, "RSDT", 4) == 0) {
        int entries = (sdt->header.length - sizeof(acpi_sdt_header_t)) / sizeof(uint32_t);
        for (int i = 0; i < entries; i++) {
            acpi_sdt_header_t* table = (acpi_sdt_header_t*)(uintptr_t)sdt->pointer_to_other_sdt[i];
            if (strncmp(table->signature, signature, 4) == 0) {
                return table;
            }
        }
    } else if (strncmp(sdt->header.signature, "XSDT", 4) == 0) {
        int entries = (sdt->header.length - sizeof(acpi_sdt_header_t)) / sizeof(uint32_t);
        for (int i = 0; i < entries; i++) {
            acpi_sdt_header_t* table = (acpi_sdt_header_t*)(uintptr_t)sdt->pointer_to_other_sdt[i];
            if (strncmp(table->signature, signature, 4) == 0) {
                return table;
            }
        }
    } 
    
    return NULL;

}

bool is_xsdt(){ return xsdt_bool;}

bool is_rsdt(){ return rsdt_bool;}

bool doChecksum(acpi_sdt_header_t *tableHeader)
{
    unsigned char sum = 0;
 
    for (uint32_t i = 0; i < tableHeader->length; i++)
    {
        sum += ((char *) tableHeader)[i];
    }
 
    return sum == 0;
}

void acpi_interrupt_handler(){
    //log(//LOG_LEVEL_DEBUG, "ACPI Interrupt");
}