#include "heartyfs.h"

uint8_t bitmap[NUM_BLOCK / 8];  // Bitmap with 2048 bits

void free_block(int block_id) 
{
    bitmap[block_id / 8] |= (1 << (block_id % 8));
}

void occupy_block(int block_id) 
{
    bitmap[block_id / 8] &= ~(1 << (block_id % 8));
}

int status_block(int block_id) 
{
    return (bitmap[block_id / 8] >> (block_id % 8)) & 1;
}

int find_free_block() 
{
    for (int i = 0; i < NUM_BLOCK; i++) 
    {
        if (status_block(i) > 0) 
        {
            return i;
        }
    }
    return -1; // No free block found
}

int search_file_in_dir(struct heartyfs_directory *parent_dir, char *target_name)
{
    for (int i = 0; i < FILES_PER_DIR; i++)
    {
        printf("Search in entry %s and target name %s\n", parent_dir->entries[i].file_name, target_name);
        if (strcmp(parent_dir->entries[i].file_name, target_name) == 0)
        {
            printf("entry block id %d\n", parent_dir->entries[i].block_id);
            return parent_dir->entries[i].block_id;
        }
    }
    return 0;
}

int create_entry(struct heartyfs_superblock *superblock, struct heartyfs_directory *parent_dir, 
                    char *target_name)
{
    int target_block_id = find_free_block(bitmap);
    if (target_block_id)
    {
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
            parent_dir->size += 1;
            printf("Created entry name %s at %s", parent_dir->entries[size].file_name, parent_dir->name);
            return 1;
        }        
    }
    return -1;
}

int create_directory(struct heartyfs_superblock *superblock, void *buffer, char *target_name)
{
    int target_block_id = find_free_block();
    if (target_block_id)
    {
        struct heartyfs_directory * created_dir = (struct heartyfs_directory *)
                                                (buffer + BLOCK_SIZE * target_block_id);
        snprintf(created_dir->name, sizeof(created_dir->name), "%s", target_name);
        created_dir->type = 1;
        create_entry(superblock, created_dir, ".");
        create_entry(superblock, created_dir, "..");
    }
    return -1;
}