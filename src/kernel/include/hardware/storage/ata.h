#ifndef __ATA__H
#define __ATA__H

#include <stdint.h>
#include <stdbool.h>

// ATA controller registers (primary and secondary channels)
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_CONTROL     0x3F6
#define ATA_SECONDARY_DATA      0x170
#define ATA_SECONDARY_CONTROL   0x376

// Register offsets
#define ATA_REG_DATA        0x00
#define ATA_REG_ERROR       0x01
#define ATA_REG_FEATURES    0x01
#define ATA_REG_SECCOUNT0   0x02
#define ATA_REG_LBA0        0x03
#define ATA_REG_LBA1        0x04
#define ATA_REG_LBA2        0x05
#define ATA_REG_HDDEVSEL    0x06
#define ATA_REG_COMMAND     0x07
#define ATA_REG_STATUS      0x07
#define ATA_REG_SECCOUNT1   0x08
#define ATA_REG_LBA3        0x09
#define ATA_REG_LBA4        0x0A
#define ATA_REG_LBA5        0x0B
#define ATA_REG_CONTROL     0x00
#define ATA_REG_ALTSTATUS   0x00

// ATA commands
#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_IDENTIFY        0xEC

// Status register bits
#define ATA_SR_BSY      0x80
#define ATA_SR_DRDY     0x40
#define ATA_SR_DRQ      0x08
#define ATA_SR_ERR      0x01

// Error register bits
#define ATA_ER_AMNF     0x01
#define ATA_ER_TK0NF    0x02
#define ATA_ER_ABRT     0x04
#define ATA_ER_MCR      0x08
#define ATA_ER_IDNF     0x10
#define ATA_ER_MC       0x20
#define ATA_ER_UNC      0x40
#define ATA_ER_BBK      0x80

// Maximum number of drives (2 channels, 2 drives per channel)
#define MAX_DRIVES      4

// Structure to hold drive information
typedef struct {
    bool present;
    bool is_sata;
    uint64_t capacity;  // in sectors
    char model[41];     // 40 chars + null terminator
    char serial[21];    // 20 chars + null terminator
} drive_info_t;

void ata_init();
void ata_interrupt_handler();

#endif // !__ATA__H
