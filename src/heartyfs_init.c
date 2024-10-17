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

    // Test the block 1 and block 2
    // TODO delete this
    struct heartyfs_superblock *test = (struct heartyfs_superblock *)(buffer);
    printf("Debug: %s\n", test->root_dir->name);
    printf("Debug: %s\n", test->root_dir->entries[0].file_name);
    printf("Debug: %s\n", test->root_dir->entries[1].file_name);
    uint8_t *test2 = (uint8_t *)(buffer + BLOCK_SIZE);
    printf("Debug: %d %d\n", status_block(1, test2), status_block(3, test2));

    // Clean up
    cleanup(buffer, fd);
    
    return 0;
}
