#include "heartyfs.h"



__uint8_t bitmap[NUM_BLOCK / 8];  // bitmap with 2048 bits > 256 bytes

void free_block(int block_id) {
    bitmap[block_id / 8] |= (1 << (block_id % 8));
}

void occupy_block(int block_id) {
    bitmap[block_id / 8] &= ~(1 << (block_id % 8));
}

int status_block(int block_id) {
    return (bitmap[block_id / 8] >> (block_id % 8)) & 1;
}

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
    // TODO Delete this
    printf("%d, %d, %d\n", BLOCK_SIZE, DISK_SIZE, NUM_BLOCK);
    printf("\n buffer %p", buffer);

    // Initialize the superblock
    struct heartyfs_directory root_dir;
    root_dir.type = 1;

    // Initialize the bitmap
    memset(bitmap, 0xFF, sizeof(bitmap)); // Set all bits to 1

    // Mark occupied
    occupy_block(0);   // Occupied first block for super block
    occupy_block(1);   // Occupied second block for bitmap

    return 0;
}
