/*
 * heartyfs_init.c
 * 
 * Brief
 * - This program initializes the filesystem. It sets up the superblock, initializes
 *   the bitmap to track free and occupied blocks, and creates the root directory along with 
 *   the "." and ".." entries to establish the directory structure.
 * 
 * Data Structures:
 * - Superblock: The central structure storing metadata about the filesystem, such as the total 
 *   number of blocks, free blocks, and the block size.
 * - Bitmap: A bit array used to track the status of blocks (free or occupied) in the filesystem.
 * - Directory: Represents the root directory and contains entries that point to files or subdirectories.
 * 
 * Design Decisions:
 * - Memory mapping (`mmap`) is used to map the disk file into memory, allowing efficient access
 *   and modification of the filesystem image.
 * - The bitmap is initialized with all bits set to 1, indicating that all blocks are initially free.
 * - Special entries (".", "..") are created for the root directory to facilitate navigation and consistency.
 * 
 *                                          Created by Nathadon Samairat 18 Oct 2024
 */
#include "heartyfs.h"

int main() 
{
    // Open the disk file
    int fd = open(DISK_FILE_PATH, O_RDWR);
    if (fd < 0) 
    {
        perror("Cannot open the disk file\n");
        exit(1);
    }

    // Map the disk file onto memory
    void *buffer = mmap(NULL, DISK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) 
    {
        perror("Cannot map the disk file onto memory\n");
        exit(1);
    }

    // TODO:
    // Initialize the superblock
    struct heartyfs_superblock *superblock = (struct heartyfs_superblock *) buffer;
    superblock->total_blocks = NUM_BLOCK;
    superblock->free_blocks = NUM_BLOCK - 2;
    superblock->block_size = BLOCK_SIZE;

    // Initialize the bitmap
    uint8_t *bitmap = (uint8_t *)(buffer + BLOCK_SIZE);
    memset(bitmap, 0xFF, sizeof(bitmap));   // Set all bits to 1

    // Add root, ., and .. directories
    struct heartyfs_directory *root_dir = superblock->root_dir;
    root_dir->type = 1;
    root_dir->size = 0;
    snprintf(root_dir->name, sizeof(root_dir->name), "%s", "/");
    create_entry(superblock, root_dir, ".", 0, bitmap);
    create_entry(superblock, root_dir, "..", 0, bitmap);

    // Mark occupied
    occupy_block(0, bitmap);   // Occupied first block for superblock
    occupy_block(1, bitmap);   // Occupied second block for bitmap
    memcpy((uint8_t *)(buffer + BLOCK_SIZE), bitmap, sizeof(bitmap)); // Put in the second block

    // Clean up
    cleanup(buffer, fd);
    
    return 0;
}
