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
    superblock->type = 1;
    superblock->size = 0;

    snprintf(superblock->dir_name, sizeof(superblock->dir_name), "%s", "/");
    superblock->entries[1].block_id = 0;
    snprintf(superblock->entries[0].file_name, sizeof(superblock->entries[0].file_name), "%s", ".");
    superblock->entries[2].block_id = 0;
    snprintf(superblock->entries[1].file_name, sizeof(superblock->entries[1].file_name), "%s", "..");

    // Initialize the bitmap
    uint8_t bitmap[NUM_BLOCK / 8];          // bitmap with 2048 bits > 256 bytes
    memset(bitmap, 0xFF, sizeof(bitmap));   // Set all bits to 1

    // Mark occupied
    occupy_block(bitmap, 0);   // Occupied first block for superblock
    occupy_block(bitmap, 1);   // Occupied second block for bitmap
    memcpy((uint8_t *)(buffer + BLOCK_SIZE), bitmap, sizeof(bitmap)); // Put in the second block

    // Test the block 1 and block 2
    // TODO delete this
    // struct heartyfs_superblock *test = (struct heartyfs_superblock *)(buffer);
    // printf("\n %s", test->dir_name);
    // printf("\n %s", test->entries[0].file_name);
    // printf("\n %s", test->entries[1].file_name);

    // uint8_t *test2 = (uint8_t *)(buffer + BLOCK_SIZE);
    // printf("\n %d\n", status_block(test2, 0));

    // Clean up
    msync(buffer, DISK_SIZE, MS_SYNC);
    munmap(buffer, DISK_SIZE);
    close(fd);
    
    return 0;
}
