#include "fsz.h"
#include <string.h>
#include <errno.h>

static FSZ_SuperBlock sb;
static int device_fd;

int fsz_init(const char* device_path) {
    // Open the device
    device_fd = open(device_path, O_RDWR);
    if (device_fd < 0) {
        return -errno;
    }

    // Read the superblock
    if (pread(device_fd, &sb, sizeof(FSZ_SuperBlock), 0) != sizeof(FSZ_SuperBlock)) {
        close(device_fd);
        return -EIO;
    }

    // Verify the magic number
    if (memcmp(sb.magic, FSZ_MAGIC, 4) != 0) {
        close(device_fd);
        return -EINVAL;
    }

    // Initialize free space management structures
    // (This would involve reading in the bitmaps)

    return 0;
}

uint32_t fsz_allocate_block() {
    // Implement block allocation using the free block bitmap
    // For simplicity, let's just do a linear search
    uint32_t bitmap_size = (sb.numsec - sb.data_area_start + 7) / 8;
    uint8_t* bitmap = malloc(bitmap_size);
    
    if (!bitmap) {
        return 0;  // Allocation failed
    }

    // Read the bitmap
    if (pread(device_fd, bitmap, bitmap_size, sb.free_block_bitmap_start * FSZ_SECSIZE) != bitmap_size) {
        free(bitmap);
        return 0;
    }

    // Find a free block
    for (uint32_t i = 0; i < bitmap_size * 8; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            // Mark the block as used
            bitmap[i / 8] |= (1 << (i % 8));
            
            // Write back the updated bitmap
            if (pwrite(device_fd, bitmap, bitmap_size, sb.free_block_bitmap_start * FSZ_SECSIZE) != bitmap_size) {
                free(bitmap);
                return 0;
            }

            free(bitmap);
            return sb.data_area_start + i;
        }
    }

    free(bitmap);
    return 0;  // No free blocks
}

void fsz_free_block(uint32_t block) {
    // Implement block freeing
    uint32_t bitmap_index = block - sb.data_area_start;
    uint32_t byte_offset = bitmap_index / 8;
    uint8_t bit_offset = bitmap_index % 8;

    uint8_t byte;
    
    // Read the byte containing the bit for this block
    if (pread(device_fd, &byte, 1, sb.free_block_bitmap_start * FSZ_SECSIZE + byte_offset) != 1) {
        return;  // Read failed
    }

    // Clear the bit
    byte &= ~(1 << bit_offset);

    // Write the byte back
    if (pwrite(device_fd, &byte, 1, sb.free_block_bitmap_start * FSZ_SECSIZE + byte_offset) != 1) {
        return;  // Write failed
    }
}

int fsz_create(const char* path, mode_t mode) {
    // Implement file creation
    // This involves:
    // 1. Allocating an inode
    // 2. Initializing the inode
    // 3. Adding an entry to the parent directory

    // For brevity, we'll just show inode allocation and initialization
    uint32_t inode_num = fsz_allocate_inode();
    if (inode_num == 0) {
        return -ENOSPC;
    }

    FSZ_Inode inode;
    memset(&inode, 0, sizeof(FSZ_Inode));
    memcpy(inode.magic, "INODE", 4);
    inode.createdate = inode.modifydate = inode.changedate = time(NULL) * 1000000;
    inode.owner.access = mode;
    
    // Write the inode
    if (pwrite(device_fd, &inode, sizeof(FSZ_Inode), 
               sb.inode_table_start * FSZ_SECSIZE + inode_num * sizeof(FSZ_Inode)) != sizeof(FSZ_Inode)) {
        fsz_free_inode(inode_num);
        return -EIO;
    }

    // TODO: Add directory entry to parent directory

    return 0;
}

size_t fsz_read(int fd, void* buf, size_t count) {
    // Implement file reading
    // This involves:
    // 1. Finding the inode for the file
    // 2. Determining which blocks to read
    // 3. Reading the data from those blocks

    // For brevity, we'll just show reading from the first direct block
    FSZ_Inode inode;
    // Assume we have a way to get the inode from the fd
    if (pread(device_fd, &inode, sizeof(FSZ_Inode), 
              sb.inode_table_start * FSZ_SECSIZE + fd * sizeof(FSZ_Inode)) != sizeof(FSZ_Inode)) {
        return -EIO;
    }

    if (count > inode.size) {
        count = inode.size;
    }

    if (inode.size <= sizeof(inode.data.small.inlinedata)) {
        // Small file, data is inline
        memcpy(buf, inode.data.small.inlinedata, count);
    } else {
        // Large file, read from first direct block
        if (pread(device_fd, buf, count, inode.data.large.direct_blocks[0] * FSZ_SECSIZE) != count) {
            return -EIO;
        }
    }

    return count;
}

size_t fsz_write(int fd, const void* buf, size_t count) {
    // Implement file writing
    // This involves:
    // 1. Finding the inode for the file
    // 2. Determining if we need to allocate new blocks
    // 3. Writing the data to the blocks
    // 4. Updating the inode

    FSZ_Inode inode;
    // Assume we have a way to get the inode from the fd
    if (pread(device_fd, &inode, sizeof(FSZ_Inode), 
              sb.inode_table_start * FSZ_SECSIZE + fd * sizeof(FSZ_Inode)) != sizeof(FSZ_Inode)) {
        return -EIO;
    }

    if (inode.size + count <= sizeof(inode.data.small.inlinedata)) {
        // Small file, data is inline
        memcpy(inode.data.small.inlinedata + inode.size, buf, count);
        inode.size += count;
    } else {
        // Large file, need to use blocks
        if (inode.size <= sizeof(inode.data.small.inlinedata)) {
            // Transition from small to large file
            uint32_t block = fsz_allocate_block();
            if (block == 0) {
                return -ENOSPC;
            }
            memcpy(&inode.data.large, &inode.data.small, sizeof(inode.data.small));
            inode.data.large.direct_blocks[0] = block;
        }

        // Write to the first direct block (for simplicity)
        if (pwrite(device_fd, buf, count, inode.data.large.direct_blocks[0] * FSZ_SECSIZE) != count) {
            return -EIO;
        }
        inode.size += count;
    }

    // Update the inode
    inode.modifydate = inode.changedate = time(NULL) * 1000000;
    if (pwrite(device_fd, &inode, sizeof(FSZ_Inode), 
               sb.inode_table_start * FSZ_SECSIZE + fd * sizeof(FSZ_Inode)) != sizeof(FSZ_Inode)) {
        return -EIO;
    }

    return count;
}

// Implement other functions (mkdir, rmdir, rename, etc.) similarly