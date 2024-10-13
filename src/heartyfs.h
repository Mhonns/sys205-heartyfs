#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define DISK_FILE_PATH "/tmp/heartyfs"
#define BLOCK_SIZE (1 << 9)
#define DISK_SIZE (1 << 20)
#define NUM_BLOCK (DISK_SIZE / BLOCK_SIZE)

struct heartyfs_dir_entry 
{
    int block_id;           // 4 bytes
    char file_name[28];     // 28 bytes
};  // Overall: 32 bytes 

struct heartyfs_directory 
{
    int type;               // 4 bytes
    char name[28];          // 28 bytes
    int size;               // 4 bytes
    struct heartyfs_dir_entry entries[14]; // 448 bytes
}; // Overall: 484 bytes

struct heartyfs_superblock 
{
    int total_blocks;       // 4 bytes
    int free_blocks;        // 4 bytes
    int block_size;         // 4 bytes
    int type;               // 4 bytes
    char dir_name[28];      // 28 bytes
    int size;               // 4 bytes
    struct heartyfs_dir_entry entries[14]; // 32 bytes
}; // Overall: 80 bytes

struct heartyfs_inode 
{
    int type;               // 4 bytes
    char name[28];          // 28 bytes
    int size;               // 4 bytes
    int data_blocks[119];   // 476 bytes
};  // Overall: 512 bytes

struct heartyfs_data_block 
{
    int size;               // 4 bytes
    char name[508];         // 508 bytes
};  // Overall: 512 bytes

extern uint8_t bitmap[NUM_BLOCK / 8];  // Bitmap with 2048 bits

void free_block(uint8_t *bitmap, int block_id) 
{
    bitmap[block_id / 8] |= (1 << (block_id % 8));
}

void occupy_block(uint8_t *bitmap, int block_id) 
{
    bitmap[block_id / 8] &= ~(1 << (block_id % 8));
}

int status_block(uint8_t *bitmap, int block_id) 
{
    return (bitmap[block_id / 8] >> (block_id % 8)) & 1;
}

int find_free_block(uint8_t *bitmap) 
{
    for (int i = 0; i < NUM_BLOCK; i++) 
    {
        if (status_block(bitmap, i) > 0) 
        {
            return i;
        }
    }
    return -1; // No free block found
}