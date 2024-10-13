#include "heartyfs.h"

int main() {
    // Open the disk file
    int fd = open(DISK_FILE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Cannot open the disk file\n");
        exit(1);
    }

    // Map the disk file onto memory
    void *buffer = mmap(NULL, DISK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (buffer == MAP_FAILED) {
        perror("Cannot map the disk file onto memory\n");
        exit(1);
    }

    // TODO:
    // Initialize the superblock
    struct heartyfs_directory *root_dir = (struct heartyfs_directory *)(buffer);
    root_dir->type = 1;
    snprintf(root_dir->name, sizeof(root_dir->name), "%s", "root");
    root_dir->size = 2;

    root_dir->entries[0].block_id = 0;
    snprintf(root_dir->entries[0].file_name, sizeof(root_dir->entries[0].file_name), "%s", ".");

    root_dir->entries[1].block_id = 1;
    snprintf(root_dir->entries[1].file_name, sizeof(root_dir->entries[1].file_name), "%s", "..");

    // Initialize the bitmap
    uint8_t bitmap[NUM_BLOCK / 8];          // bitmap with 2048 bits > 256 bytes
    memset(bitmap, 0xFF, sizeof(bitmap));   // Set all bits to 1

    // Mark occupied
    occupy_block(bitmap, 0);   // Occupied first block for superblock
    occupy_block(bitmap, 1);   // Occupied second block for bitmap
    memcpy((uint8_t *)(buffer + BLOCK_SIZE), bitmap, sizeof(bitmap)); // Put in the second block

    // Test the block 1 and block 2
    // TODO delete this
    // struct heartyfs_directory *test = (struct heartyfs_directory *)(buffer);
    // printf("\n %s", test->name);

    // uint8_t *test2 = (uint8_t *)(buffer + BLOCK_SIZE);
    // printf("\n %d\n", status_block(test2, 0));

    // Clean up
    close(fd);
    munmap(buffer, DISK_SIZE);
    
    return 0;
}
