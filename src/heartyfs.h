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
#define FILES_PER_DIR 14
#define CHAR_SIZE 28

struct heartyfs_dir_entry 
{
    int block_id;               // 4 bytes
    char file_name[CHAR_SIZE];  // 28 bytes
};  // Overall: 32 bytes 

struct heartyfs_directory 
{
    int type;               // 4 bytes
    char name[CHAR_SIZE];   // 28 bytes
    int size;               // 4 bytes
    struct heartyfs_dir_entry entries[FILES_PER_DIR]; // 448 bytes
}; // Overall: 484 bytes

struct heartyfs_superblock 
{
    int total_blocks;       // 4 bytes
    int free_blocks;        // 4 bytes
    int block_size;         // 4 bytes
    int type;               // 4 bytes
    struct heartyfs_directory root_dir[1]; // 484 bytes
}; // Overall: 504 bytes

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

#ifndef HEARTYFS_H
#define HEARTYFS_H

// Bitmap operations
void free_block(int block_id, uint8_t *bitmap);
void occupy_block(int block_id, uint8_t *bitmap);
int find_free_block(uint8_t *bitmap);
int status_block(int block_id, uint8_t *bitmap);

// Entry and Directory operations
int search_file_in_dir(struct heartyfs_directory *parent_dir, char *target_name, uint8_t *bitmap);
int create_entry(struct heartyfs_superblock *superblock, struct heartyfs_directory *parent_dir, 
                    char *target_name, int target_block_id, uint8_t *bitmap);
int create_directory(struct heartyfs_superblock *superblock, void *buffer, 
                        char *target_name, uint8_t target_block_id, 
                        uint8_t parent_block_id, uint8_t *bitmap);

#endif