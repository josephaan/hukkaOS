#include <hardware/storage/ata.h>
#include <hardware/io/io.h>
#include <display/framebuffer.h>
#include <x86_64/cpu.h>
#include <hardware/apic/apic.h>
#include <hardware/apic/ioapic.h>
#include <debug/log.h>


// Global array to store information for all possible drives
drive_info_t drives[MAX_DRIVES];
enum color Color = Yellow;

// Utility function to read a register
uint8_t ata_read_register(uint16_t channel, uint8_t reg) {
    if (reg > 0x07 && reg < 0x0C)
        out_b(channel + ATA_REG_CONTROL, 0x80 | (reg - 0x08));
    return in_b(channel + (reg < 0x08 ? reg : (reg - 0x06)));
}

// Utility function to write to a register
void ata_write_register(uint16_t channel, uint8_t reg, uint8_t data) {
    if (reg > 0x07 && reg < 0x0C)
        out_b(channel + ATA_REG_CONTROL, 0x80 | (reg - 0x08));
    out_b(channel + (reg < 0x08 ? reg : (reg - 0x06)), data);
}

// Wait for the drive to be ready
int ata_wait_ready(uint16_t channel) {
    for (int i = 0; i < 1000; i++) {
        uint8_t status = ata_read_register(channel, ATA_REG_STATUS);
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRDY)) return 1;
    }
    return 0;  // Timeout
}

// Select the drive
void ata_select_drive(uint16_t channel, uint8_t drive) {
    ata_write_register(channel, ATA_REG_HDDEVSEL, 0xA0 | (drive << 4));
}

// Send IDENTIFY command and process the response
int ata_identify(uint16_t channel, uint8_t drive, drive_info_t* info) {
    ata_select_drive(channel, drive);
    
    // Set all registers to zero
    for (int i = ATA_REG_FEATURES; i <= ATA_REG_LBA2; i++)
        ata_write_register(channel, i, 0);
    
    // Send IDENTIFY command
    ata_write_register(channel, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    
    // Wait for data to be ready
    if (!ata_wait_ready(channel)) return 0;
    
    uint16_t identify_data[256];
    in_sw(channel + ATA_REG_DATA, identify_data, 256);
    
    // Check if it's a SATA device
    info->is_sata = (identify_data[0] == 0xEB14 || identify_data[0] == 0xC33C);
    
    // Process capacity information
    info->capacity = ((uint64_t)identify_data[103] << 48) |
                     ((uint64_t)identify_data[102] << 32) |
                     ((uint64_t)identify_data[101] << 16) |
                     ((uint64_t)identify_data[100]);
    
    if (info->capacity == 0) {
        info->capacity = ((uint32_t)identify_data[61] << 16) | identify_data[60];
    }
    
    // Extract model number and serial number
    for (int i = 0; i < 40; i += 2) {
        info->model[i] = identify_data[27 + i/2] >> 8;
        info->model[i+1] = identify_data[27 + i/2] & 0xFF;
    }
    info->model[40] = '\0';
    
    for (int i = 0; i < 20; i += 2) {
        info->serial[i] = identify_data[10 + i/2] >> 8;
        info->serial[i+1] = identify_data[10 + i/2] & 0xFF;
    }
    info->serial[20] = '\0';
    
    info->present = true;
    return 1;  // Success
}

void setup_ioapic_for_ata() {
    // Map IRQ 14 (Primary ATA) to vector 0x20 (example) in the APIC
    ioapic_set_irq(14, 0x22, 0);
    ioapic_unmask_irq(14);

    // Map IRQ 15 (Secondary ATA) to vector 0x21 (example) in the APIC
    // Vector 0x21 for IRQ 15
    ioapic_set_irq(15, 0x22, 0);
    ioapic_unmask_irq(15);
}

// Enable ATA interrupts (still required)
void enable_ata_interrupts() {
    out_b(ATA_PRIMARY_CONTROL, 0);   // Clear nIEN bit (enable IRQ for primary ATA)
    out_b(ATA_SECONDARY_CONTROL, 0); // Clear nIEN bit (enable IRQ for secondary ATA)
}

// Function to check if the ATA device is ready
int ata_device_ready(uint16_t io_base) {
    // Read the status register (located at io_base + 0x07)
    uint8_t status = in_b(io_base + 0x07);
    
    // Check if the device is busy
    if (status & ATA_SR_BSY) {
        return 0;  // Device is busy, not ready
    }
    
    // Check if the device is ready
    if (status & ATA_SR_DRDY) {
        return 1;  // Device is ready to accept commands
    }
    
    // Optionally, check for errors in the status register
    if (status & ATA_SR_ERR) {
        return -1;  // Error occurred
    }

    return 0;  // Device is not ready
}

// ISR handler for APIC
void ata_interrupt_handler() {
    if (ata_device_ready(ATA_PRIMARY_DATA)) {
        // Handle primary channel
        //handle_primary_channel();
    }
    if (ata_device_ready(ATA_SECONDARY_DATA)) {
        // Handle secondary channel
        //handle_secondary_channel();
    }
}
// Initialize the ATA/SATA driver
void ata_init() {

    setup_ioapic_for_ata();
    
    uint16_t channels[2] = {ATA_PRIMARY_DATA, ATA_SECONDARY_DATA};
    
    for (int channel = 0; channel < 2; channel++) {
        for (int drive = 0; drive < 2; drive++) {
            int index = channel * 2 + drive;
            if (ata_identify(channels[channel], drive, &drives[index])) {
                print_numstr("\n\nDrive %d detected:\n", Color, index);
                print_numstr("Interface: %s\n", Color, drives[index].is_sata ? "SATA" : "PATA");
                print_numstr("Model: %s\n", Color, drives[index].model);
                print_numstr("Serial: %s\n", Color, drives[index].serial);
                print_numstr("Capacity: %lu sectors (%lu MB)\n", Color, 
                      drives[index].capacity, 
                      drives[index].capacity / 2048);  // 512 bytes per sector, 1024 KB per MB
            } else {
                drives[index].present = false;
                print_numstr("No drive detected at channel %d, drive %d\n", Color, channel, drive);
            }
        }
    }

   
    print_numstr("ATA initialization complete\n", Color);
    
    
}

// Write sectors to a drive
int ata_write_sectors(uint8_t drive_index, uint64_t lba, uint16_t sector_count, void* buffer) {
    if (drive_index >= MAX_DRIVES || !drives[drive_index].present) return 0;
    
    uint16_t channel = (drive_index / 2 == 0) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA;
    uint8_t slave = drive_index % 2;
    
    ata_select_drive(channel, slave);
    
    // Use LBA48 addressing if supported and necessary
    if (drives[drive_index].capacity > 0x0FFFFFFF || lba > 0x0FFFFFFF) {
        ata_write_register(channel, ATA_REG_SECCOUNT1, (sector_count >> 8) & 0xFF);
        ata_write_register(channel, ATA_REG_LBA3, (lba >> 24) & 0xFF);
        ata_write_register(channel, ATA_REG_LBA4, (lba >> 32) & 0xFF);
        ata_write_register(channel, ATA_REG_LBA5, (lba >> 40) & 0xFF);
    }
    
    ata_write_register(channel, ATA_REG_SECCOUNT0, sector_count & 0xFF);
    ata_write_register(channel, ATA_REG_LBA0, lba & 0xFF);
    ata_write_register(channel, ATA_REG_LBA1, (lba >> 8) & 0xFF);
    ata_write_register(channel, ATA_REG_LBA2, (lba >> 16) & 0xFF);
    
    uint8_t cmd = (drives[drive_index].capacity > 0x0FFFFFFF || lba > 0x0FFFFFFF) ?
                  ATA_CMD_WRITE_PIO_EXT : ATA_CMD_WRITE_PIO;
    ata_write_register(channel, ATA_REG_COMMAND, cmd);
    
    for (uint16_t i = 0; i < sector_count; i++) {
        if (!ata_wait_ready(channel)) return 0;
        
        // Wait for the drive to request data
        uint8_t status = ata_read_register(channel, ATA_REG_STATUS);
        if (!(status & ATA_SR_DRQ)) {
            print_numstr("Error: Drive not ready for data\n", Color);
            return 0;
        }
        
        // Write a sector
        out_sw(channel + ATA_REG_DATA, buffer + i * 512, 256);
        
        // Flush the cache if this is the last sector or if we're dealing with a SATA drive
        if (i == sector_count - 1 || drives[drive_index].is_sata) {
            ata_write_register(channel, ATA_REG_COMMAND, 0xE7); // Cache flush command
            ata_wait_ready(channel);
        }
    }
    
    return 1; // Success
}

// Helper function to print error information
void ata_print_error(uint16_t channel) {
    uint8_t error = ata_read_register(channel, ATA_REG_ERROR);
    print_numstr("ATA Error: ", Color);
    if (error & ATA_ER_AMNF) print_numstr("Address mark not found ", Color);
    if (error & ATA_ER_TK0NF) print_numstr("Track 0 not found ", Color);
    if (error & ATA_ER_ABRT) print_numstr("Command aborted ", Color);
    if (error & ATA_ER_MCR) print_numstr("Media change request ", Color);
    if (error & ATA_ER_IDNF) print_numstr("ID not found ", Color);
    if (error & ATA_ER_MC) print_numstr("Media changed ", Color);
    if (error & ATA_ER_UNC) print_numstr("Uncorrectable data error ", Color);
    if (error & ATA_ER_BBK) print_numstr("Bad block detected ", Color);
    print_numstr("\n", Color);
}

// Read sectors from a drive
int ata_read_sectors(uint8_t drive_index, uint64_t lba, uint16_t sector_count, void* buffer) {
    if (drive_index >= MAX_DRIVES || !drives[drive_index].present) return 0;
    
    uint16_t channel = (drive_index / 2 == 0) ? ATA_PRIMARY_DATA : ATA_SECONDARY_DATA;
    uint8_t slave = drive_index % 2;
    
    ata_select_drive(channel, slave);
    
    // Use LBA48 addressing if supported and necessary
    if (drives[drive_index].capacity > 0x0FFFFFFF || lba > 0x0FFFFFFF) {
        ata_write_register(channel, ATA_REG_SECCOUNT1, (sector_count >> 8) & 0xFF);
        ata_write_register(channel, ATA_REG_LBA3, (lba >> 24) & 0xFF);
        ata_write_register(channel, ATA_REG_LBA4, (lba >> 32) & 0xFF);
        ata_write_register(channel, ATA_REG_LBA5, (lba >> 40) & 0xFF);
    }
    
    ata_write_register(channel, ATA_REG_SECCOUNT0, sector_count & 0xFF);
    ata_write_register(channel, ATA_REG_LBA0, lba & 0xFF);
    ata_write_register(channel, ATA_REG_LBA1, (lba >> 8) & 0xFF);
    ata_write_register(channel, ATA_REG_LBA2, (lba >> 16) & 0xFF);
    
    uint8_t cmd = (drives[drive_index].capacity > 0x0FFFFFFF || lba > 0x0FFFFFFF) ? ATA_CMD_READ_PIO_EXT : ATA_CMD_READ_PIO;
    ata_write_register(channel, ATA_REG_COMMAND, cmd);

    for (uint16_t i = 0; i < sector_count; i++) {
        if (!ata_wait_ready(channel)) {
            print_numstr("Error: Drive not ready\n", Color);
            ata_print_error(channel);
            return 0;
        }
        
        uint8_t status = ata_read_register(channel, ATA_REG_STATUS);
        if (status & ATA_SR_ERR) {
            print_numstr("Error occurred during read operation\n", Color);
            ata_print_error(channel);
            return 0;
        }
        
        in_sw(channel + ATA_REG_DATA, buffer + i * 512, 256);
    }
    
    return 1; // Success
}