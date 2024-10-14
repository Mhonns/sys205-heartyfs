#include "heartyfs.h"

void free_block(int block_id, uint8_t *bitmap) 
{
    bitmap[block_id / 8] |= (1 << (block_id % 8));
}

void occupy_block(int block_id, uint8_t *bitmap) 
{
    bitmap[block_id / 8] &= ~(1 << (block_id % 8));
}

int status_block(int block_id, uint8_t *bitmap) 
{
    return (bitmap[block_id / 8] >> (block_id % 8)) & 1;
}

int find_free_block(uint8_t *bitmap) 
{
    for (int i = 0; i < NUM_BLOCK; i++) 
    {
        if (status_block(i, bitmap) > 0) 
        {
            return i;
        }
    }
    return -1; // No free block found
}

int search_file_in_dir(struct heartyfs_directory *parent_dir, char *target_name, uint8_t *bitmap)
{
    for (int i = 0; i < parent_dir->size; i++)
    {
        printf("Debug: Search in entry %s and target name %s\n", parent_dir->entries[i].file_name, target_name);
        if (strcmp(parent_dir->entries[i].file_name, target_name) == 0)
        {
            printf("Debug: entry block id %d\n", parent_dir->entries[i].block_id);
            return parent_dir->entries[i].block_id;
        }
    }
    return 0;
}

int create_entry(struct heartyfs_superblock *superblock, struct heartyfs_directory *parent_dir, 
                    char *target_name, int target_block_id, uint8_t *bitmap)
{
    if (target_block_id < 0)
    {
        return -1;
    }
    if (parent_dir->size > FILES_PER_DIR)
    {
        return 0;
    }
    else
    {
        int size = parent_dir->size;
        snprintf(parent_dir->entries[size].file_name, sizeof(parent_dir->entries[size].file_name),
                    "%s", target_name);
        parent_dir->entries[size].block_id = target_block_id;
        parent_dir->size++;
        // Mark occupied
        superblock->free_blocks--;
        occupy_block(target_block_id, bitmap);
        printf("Success: Created entry %s at %s with id %d\n", parent_dir->entries[size].file_name, 
                    parent_dir->name, parent_dir->entries[size].block_id);
        return 1;
    }
}

int create_directory(struct heartyfs_superblock *superblock, void *buffer, 
                        char *target_name, uint8_t target_block_id, 
                        uint8_t parent_block_id, uint8_t *bitmap)
{
    if (target_block_id < 1) return -1;
    else 
    {
        struct heartyfs_directory *created_dir = (struct heartyfs_directory *)(buffer + BLOCK_SIZE * target_block_id);
        created_dir->type = 1;
        created_dir->size = 0;
        snprintf(created_dir->name, sizeof(created_dir->name), "%s", target_name);
        create_entry(superblock, created_dir, ".", target_block_id, bitmap);
        create_entry(superblock, created_dir, "..", parent_block_id, bitmap);
        printf("Success: The directory %s was\n", created_dir->name);
        return 0;
    }
}