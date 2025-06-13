#ifndef FSZ_H
#define FSZ_H

#include <stdint.h>
#include <stddef.h>

#define FSZ_SECSIZE 4096
#define FSZ_MAGIC "FSZ1"

// Extend the superblock to include free space management
typedef struct {
    char magic[4];
    uint16_t version_major;
    uint16_t version_minor;
    uint8_t logsec;
    uint8_t maxmounts;
    uint8_t currmounts;
    uint8_t reserved1;
    uint64_t createdate;
    uint64_t lastmountdate;
    uint64_t lastumountdate;
    uint8_t uuid[16];
    uint64_t numsec;
    uint64_t freesec;
    uint32_t free_inode_bitmap_start;  // Sector number where free inode bitmap starts
    uint32_t free_block_bitmap_start;  // Sector number where free block bitmap starts
    uint32_t inode_table_start;        // Sector number where inode table starts
    uint32_t data_area_start;          // Sector number where data blocks start
    uint32_t rootdirfid;
    uint32_t checksum;
    char magic2[4];
} FSZ_SuperBlock;

// Keep the inode structure, but add a few fields
typedef struct {
    char magic[4];
    uint32_t sec;
    uint32_t size;
    uint16_t numlinks;
    uint16_t numblocks;
    uint32_t flags;
    uint64_t createdate;
    uint64_t modifydate;
    uint64_t changedate;
    char filetype[4];
    char mimetype[36];
    struct {
        char name[28];
        uint32_t access;
    } owner;
    union {
        struct {
            uint32_t direct_blocks[10];  // Direct block pointers
            uint32_t indirect_block;     // Single indirect block pointer
            uint32_t double_indirect;    // Double indirect block pointer
            char inlinedata[916];
        } large;
        struct {
            char inlinedata[1000];
        } small;
    } data;
    uint32_t checksum;
} FSZ_Inode;

// Function prototypes for the extended functionality
int fsz_init(const char* device_path);
int fsz_mount(const char* device_path);
int fsz_unmount();

// File operations
int fsz_create(const char* path, mode_t mode);
int fsz_open(const char* path, int flags);
size_t fsz_read(int fd, void* buf, size_t count);
size_t fsz_write(int fd, const void* buf, size_t count);
int fsz_close(int fd);
int fsz_unlink(const char* path);

// Directory operations
int fsz_mkdir(const char* path, mode_t mode);
int fsz_rmdir(const char* path);
int fsz_readdir(const char* path, void* buf, fuse_fill_dir_t filler);

// Other operations
int fsz_rename(const char* from, const char* to);
int fsz_truncate(const char* path, off_t size);
int fsz_chmod(const char* path, mode_t mode);

// Utility functions
uint32_t fsz_allocate_block();
void fsz_free_block(uint32_t block);
uint32_t fsz_allocate_inode();
void fsz_free_inode(uint32_t inode);

#endif // FSZ_H