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

int dir_string_check(char *input_str, char *dir_name, void* buffer,
                        struct heartyfs_directory **parent_dir, uint8_t *bitmap)
{
    char delimiter[2] = "/";
    char* token = strtok(input_str, delimiter);
    int depth = 0;
    int matched_depth = 0;
    while (token != NULL) 
    {
        if (depth == matched_depth)
        {
            sscanf(token, "%s", dir_name);
            int parent_block_id = search_entry_in_dir(*parent_dir, dir_name);
            if (parent_block_id > 0)
            {
                struct heartyfs_directory *temp_dir = (struct heartyfs_directory *) (buffer + BLOCK_SIZE * parent_block_id);
                if (temp_dir->type == 1)    // check whether it is a directory or not
                {
                    *parent_dir = (struct heartyfs_directory *) (buffer + BLOCK_SIZE * parent_block_id);
                }
                matched_depth++;
            }
            else if (parent_block_id == 0)
            {
                struct heartyfs_superblock *superblock = (struct heartyfs_superblock *) buffer;
                *parent_dir = superblock->root_dir;
                matched_depth++;
            }
        }
        token = strtok(NULL, delimiter);
        depth++;
    }
    printf("Debug: Matched vs depth: %d vs %d\n", matched_depth, depth);
    int diff = depth - matched_depth;
    return diff;
}

int search_entry_in_dir(struct heartyfs_directory *parent_dir, char *target_name)
{
    for (int i = 0; i < parent_dir->size; i++)
    {
        if (strcmp(parent_dir->entries[i].file_name, target_name) == 0)
        {
            return parent_dir->entries[i].block_id;
        }
    }
    return -1;
}

int create_entry(struct heartyfs_superblock *superblock, struct heartyfs_directory *parent_dir, 
                    char *target_name, int target_block_id, uint8_t *bitmap)
{
    if (parent_dir->size > FILES_PER_DIR - 1)
    {
        printf("Error: The directory is full\n");
        return -1;
    }
    else
    {
        int size = parent_dir->size;
        snprintf(parent_dir->entries[size].file_name, sizeof(parent_dir->entries[size].file_name),
                    "%s", target_name);
        parent_dir->entries[size].block_id = target_block_id;
        parent_dir->size++;
        printf("Success: Created entry %s at %s with id %d\n", parent_dir->entries[size].file_name, 
                    parent_dir->name, parent_dir->entries[size].block_id);
        return 1;
    }
}

int remove_entry(struct heartyfs_superblock *superblock, void* buffer, 
                    int parent_block_id, char *target_name)
{
    struct heartyfs_directory *parent_dir = NULL;
    if (parent_block_id == 0) parent_dir = superblock->root_dir;
    else parent_dir = (struct heartyfs_directory *) (buffer + BLOCK_SIZE * parent_block_id);
    if (strcmp(target_name, ".") != 0 && strcmp(target_name, "..") != 0)
    {
        for (int i = 0; i < parent_dir->size; i++)
        {
            if (strcmp(parent_dir->entries[i].file_name, target_name) == 0)
            {
                // Move the last entry to the removed entry
                parent_dir->entries[i] = parent_dir->entries[parent_dir->size - 1];
                parent_dir->size--;
                printf("Success: Removed entry %s\n", target_name);
                return 1;
            }
        }
        printf("Error: Can not find the entry %s\n", target_name);
        return -1;
    }
    else
    {
        printf("Denied: Refuse to remove this entry %s\n", target_name);
        return -1;
    }
}

void cleanup(void *buffer, int fd) 
{
    if (buffer != NULL) {
        msync(buffer, DISK_SIZE, MS_SYNC); // Sync changes to the file
        munmap(buffer, DISK_SIZE);         // Unmap the memory
    }
    if (fd >= 0) {
        close(fd);                    // Close the file descriptor
    }
}